#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>

int checkError(int val, const char *msg){
    if (val == -1)
   {
       perror(msg);
       exit(EXIT_FAILURE);
   }
return val;
}

void pSigHndl (int sig){
    int status;
    int readycount = 0;
    if (sig == SIGINT){
        //SIGINT should send a SIGTERM to both children
        printf("kill all children\n");
        checkError(kill(0, SIGTERM), "failed to kill all children");
        exit(EXIT_SUCCESS);
    }
    if (sig == SIGCHLD){
        //reap children
        while (waitpid(-1, &status, WNOHANG) > 0) { //removing child processes
            printf("shot the child :(\n");
        }

        if (waitpid(-1, NULL, WNOHANG) == -1) { //if it returns -1 (error)
            printf("no more children, goodbye\n"); 
            exit(EXIT_SUCCESS);
        }
    }
    //SIGUSR1 and SIGUSR2 represent the handshakes from the respective children
    if (sig == SIGUSR1){
        readycount++;
        if (readycount == 2) {
            printf("Both children are ready. Starting the game!\n");
        }
    }
    if (sig == SIGUSR2){
        readycount++;
        if (readycount == 2) {
            printf("Both children are ready. Starting the game!\n");
        }
    }
}

void cSigHndl (int sig){
    static int low = 0, high = 100; //bounds
    if (sig == SIGUSR1){
        //high guess
        low = (low + high) / 2; //change lower bound
        printf("got SIGUSR1, going higher, range: [%d, %d]\n", low, high);
    }
    if (sig == SIGUSR2){
        //low guess
        high = (low + high) / 2; //change upper bound
        printf("got SIGUSR2, going lower, range: [%d, %d]\n", low, high);
    }
    if (sig == SIGTERM){
        //exit child
        printf("Child: Received SIGTERM. Exiting...\n");
        exit(EXIT_SUCCESS);
    }
    if (sig == SIGINT){
        //SIGINT tells the child to stop guessing and start over
        low = 0;
        high = 100;
        printf("Child: Received SIGINT, resetting game...\n");
    }
}

int player1guess(int low,  int high){
   // Player 1 should compute its guess by taking the average of min and max
    return (high/low) / 2;
}
int player2guess(int low, int high){
    /*
    Player 2 should compute its guess by generating a random number between min and max
    Remember to seed the random number generator using srand
    it is a good idea to the generate a few random numbers to prevent the parent and child 2 from generating the same random numbers every time.
    */
    srand(time(NULL) ^ getpid());
    return low + rand() % (high - low + 1);
}

int childActions(int player){
    struct sigaction csa; //child signal handler
    csa.sa_handler = cSigHndl;
    csa.sa_flags = 0;
    sigemptyset(&csa.sa_mask);
    
    sigaction(SIGTERM, &csa, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL); //masking sigint so children dont explode

    int guess,low,high;
    
    if (player == 1){ //child is ready
        kill(getppid(), SIGUSR1);
    }
    else{
        kill(getppid(), SIGUSR2);
    }
    
    while(1){ //picking guess based on argument
        if (player == 1){
            guess = player1guess(low, high);
        } 
        else {
            guess = player2guess(low, high);
        }

        char buf[10]; 
        int len = snprintf(buf, sizeof(buf), "%d", guess);

        int fd;
        if (player == 1){ //open and writing guesses to file
            fd = checkError(open("guess1", O_WRONLY | O_CREAT | O_TRUNC, 0666), "open guess1");
        } 
        else{
            fd = checkError(open("guess2", O_WRONLY | O_CREAT | O_TRUNC, 0666), "open guess2");
        }

        checkError(write(fd, buf, len), "write guess file");
        checkError(close(fd), "close guess file");

        if (player == 1){ // send signal to parent
            kill(getppid(), SIGUSR1);
        } 
        else{
            kill(getppid(), SIGUSR2);
        }

        pause(); //waiting for response
    }


}

void parentActions(pid_t p1, pid_t p2){
    int goal;
    int guess1, guess2;
    int p1score = 0;
    int p2score = 0;
    char buf[10];
    char buf2[10];

    for (int game = 1; game <= 10; game++) {
        printf("Starting Game %d\n", game);

        srand(time(NULL));
        goal = 1 + rand() % 100;

        while (1) {
            pause();

            int p1guessfd = checkError(open("guess1", O_RDONLY), "open guess1");
            int bytesRead1 = checkError(read(p1guessfd, buf, sizeof(buf) - 1), "read guess1");
            checkError(close(p1guessfd), "close guess1");
            buf[bytesRead1] = '\0';

            if (sscanf(buf, "%d", &guess1) != 1){
                char errorMsg[] = "invalid guess format for p1\n";
                write(STDERR_FILENO, errorMsg, sizeof(errorMsg) - 1);
                exit(EXIT_FAILURE);
            }
            
            int p2guessfd = checkError(open("guess2", O_RDONLY), "open guess2");
            int bytesRead2 = checkError(read(p2guessfd, buf2, sizeof(buf2) - 1), "read guess2");
            checkError(close(p2guessfd), "close guess2");
            buf[bytesRead2] = '\0';

            if (sscanf(buf, "%d", &guess2) != 1){
                char errorMsg[] = "invalid guess format for p2\n";
                write(STDERR_FILENO, errorMsg, sizeof(errorMsg) - 1);
                exit(EXIT_FAILURE);
            }

            if (guess1 == goal || guess2 == goal){ //if player matches 
                if (guess1 == goal){ 
                    p1score++;
                    printf("Game %d Winner: Player 1\n", game);
                }
                if (guess2 == goal){
                    p2score++;
                    printf("Game %d Winner: Player 2\n", game);
                }
                break;
            }

            if (guess1 < goal){//player1
                kill(p1, SIGUSR1); //guess higher
            } 
            else{
                kill(p1, SIGUSR2); //guess lower
            }

            if (guess2 < goal){ //player2
                kill(p2, SIGUSR1); //guess higher
            } 
            else{
                kill(p2, SIGUSR2); //guess lower
            }
        }
        kill(p1, SIGINT);
        kill(p2, SIGINT);
    }
    printf("Final Score - Player 1: %d, Player 2: %d\n", p1score, p2score);
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
}

int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sa.sa_handler = pSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    checkError(sigaction(SIGCHLD, &sa, NULL), "sigaction SIGCHLD");
    checkError(sigaction(SIGUSR1, &sa, NULL), "sigaction SIGUSR1");
    checkError(sigaction(SIGUSR2, &sa, NULL), "sigaction SIGUSR2");

    pid_t pids[2];

    for (int i = 0; i < 2; i++) {
        pids[i] = checkError(fork(), "fork exploded for the second time");
        if (pids[i] == 0) {
            childActions(i + 1); // p1 = 0+1, p2 = 1+1 
            exit(0);
        }
    }

    parentActions(pids[0], pids[1]); //pass over the pids
    return 0;
}

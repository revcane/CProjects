#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

volatile sig_atomic_t game_over = 0;

volatile sig_atomic_t gotsig1 = 0;
volatile sig_atomic_t gotsig2 = 0;
volatile sig_atomic_t gotsigint = 0;

volatile sig_atomic_t parent_received_sigusr1 = 0;
volatile sig_atomic_t parent_received_sigusr2 = 0;

pid_t p1;
pid_t p2;

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void pSigHndl (int sig){
    int status;
    if (sig == SIGUSR1) {
        parent_received_sigusr1 = 1;
    }
    else if (sig == SIGUSR2) {
        parent_received_sigusr2 = 1;
    }
    else if (sig == SIGINT) {
        write(STDOUT_FILENO, "received SIGINT, terminating children...\n", 42);
        kill(p1, SIGTERM);
        kill(p2, SIGTERM);
    }
    else if (sig == SIGCHLD) {
        while (waitpid(-1, &status, WNOHANG) > 0) {
            write(STDOUT_FILENO, "child process terminated\n", 26);
        }
        if (waitpid(-1, NULL, WNOHANG) == -1) {
            write(STDOUT_FILENO, "the children are all dead\n", 27);
            exit(EXIT_SUCCESS);
        }
    }
}

void cSigHndl (int sig){
    if (sig == SIGUSR1) {
        gotsig1 = 1;
    }
    else if (sig == SIGUSR2) {
        gotsig2 = 1;
    }
    else if (sig == SIGINT) { 
        gotsigint = 1;
    }
    else if (sig == SIGTERM) {
        exit(EXIT_SUCCESS);
    }
}

void childSignals(){
    struct sigaction csa;
    csa.sa_handler = cSigHndl;
    sigemptyset(&csa.sa_mask);
    csa.sa_flags = 0;

    checkError(sigaction(SIGUSR1, &csa, NULL), "sigaction SIGUSR1");
   
    csa.sa_handler = cSigHndl;
    checkError(sigaction(SIGUSR2, &csa, NULL), "sigaction SIGUSR2");
   
    csa.sa_handler = cSigHndl;
    checkError(sigaction(SIGTERM, &csa, NULL), "sigaction SIGTERM");
    
    csa.sa_handler = cSigHndl;
    checkError(sigaction(SIGINT, &csa, NULL), "sigaction SIGINT");
}

void parentSignals(){
    struct sigaction sa;
    sa.sa_handler = pSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    checkError(sigaction(SIGCHLD, &sa, NULL), "sigaction SIGCHLD");
    sa.sa_handler = pSigHndl;
    checkError(sigaction(SIGUSR1, &sa, NULL), "sigaction SIGUSR1");
    sa.sa_handler = pSigHndl;
    checkError(sigaction(SIGUSR2, &sa, NULL), "sigaction SIGUSR2");
}
void writeFile(int child,int value){
    char filename[50];

    int fd;
    if (child == 1){
        fd = checkError(open("guess1.dat",  O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR), "open guess1");
    } 
    else {
        fd = checkError(open("guess2.dat",  O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR), "open guess2");
    }
    
    checkError(write(fd, &value, sizeof(value)), "write guess file");

    checkError(close(fd), "close guess file");
}

int readFile(int child){
    int guess = 0;
    int fd;
    if (child == 1){
        fd = checkError(open("guess1.dat", O_RDONLY, S_IRUSR), "opening guess1");
    } 
    else {
        fd = checkError(open("guess2.dat", O_RDONLY, S_IRUSR), "opening guess2");
    }

    checkError(read(fd, &guess, sizeof(int)), "read file");
    
    checkError(close(fd), "close guess1");    

    return guess;
}

void player1Actions(){
    childSignals();
    pause();
    
    while(1){ //game loop
        int guess;
        int low = 0; 
        int high = 101;

        pid_t parent_pid = getppid();
        checkError(kill(parent_pid, SIGUSR1), "error in sending SIGUSR1");

        while(1) { // guess loop
            gotsig1 = 0;
            gotsig2 = 0;
            gotsigint = 0;

            guess = (low + high) / 2;


            writeFile(1,guess);

            sleep(1);

            pid_t parent_pid = getppid();
            checkError(kill(parent_pid, SIGUSR1), "error in sending SIGUSR1");

            pause();

            if (gotsig1){
                high = guess - 1; //low
            }
            else if (gotsig2){
                low = guess + 1; //high
            }
            else if (gotsigint){
                break; //correct
            }
        }
    }
    exit(EXIT_SUCCESS);
}

void player2Actions(){
    childSignals();

    srand(time(NULL));

    pause();
    
    while(1){ //game loop
        int guess;
        int low = 0; 
        int high = 101;

        pid_t parent_pid = getppid();
        checkError(kill(parent_pid, SIGUSR2), "error in sending SIGUSR1");

        while(1) { // guess loop
            gotsig1 = 0;
            gotsig2 = 0;
            gotsigint = 0;

            guess = low + rand() % (high - low + 1);

            writeFile(2,guess);

            sleep(1);
            //signal to parent
            pid_t parent_pid = getppid();
            checkError(kill(parent_pid, SIGUSR2), "error in sending SIGUSR1");

            pause();
            
            if (gotsig1){
                high = guess - 1; //low
            }
            else if (gotsig2){
                low = guess + 1; //high
            }
            else if (gotsigint){
                break; //correct
            }
        }
    }
    exit(EXIT_SUCCESS);
}

void parentActions(){
    struct sigaction sa;
    sa.sa_handler = pSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    checkError(sigaction(SIGINT, &sa, NULL), "sigaction SIGINT");

    int goal;
    int guess1, guess2;
    int p1score = 0;
    int p2score = 0;

    srand(time(NULL));

    write(STDOUT_FILENO, "sleeping for 5 seconds before starting match\n",46);

    sleep(5);

    checkError(kill(p1, SIGUSR1), "P: failed to send SIGUSR1 to P1");
    checkError(kill(p2, SIGUSR2), "P: failed to send SIGUSR2 to P2");

    for (int game = 1; game <= 10; game++) {
        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "starting game %d,\n score: P1 - %d | P2 - %d\n", game, p1score, p2score);
        write(STDOUT_FILENO, buffer, len);

        while (!parent_received_sigusr1 || !parent_received_sigusr2) {
            pause();
        }

        parent_received_sigusr1 = 0;
        parent_received_sigusr2 = 0;

        goal = 1 + rand() % 100;
        
        while (1){
            while (!parent_received_sigusr1 || !parent_received_sigusr2) {
                pause();
            }
            parent_received_sigusr1 = 0;
            parent_received_sigusr2 = 0;

            //reading player 1 guesses
            guess1 = readFile(1);
            
            //reading player 2 guesses
            guess2 = readFile(2);

            printf("player 1: %d, player 2: %d (goal: %d)\n", guess1, guess2, goal); //TODO: WRITE()

            //determining winner
            if (guess1 == goal && guess2 == goal) {
                write(STDOUT_FILENO, "players tied\n", 14);
                p1score++;
                p2score++;
                kill(p1, SIGINT);
                kill(p2, SIGINT);
                break;
            }
            else if (guess1 == goal) {
                write(STDOUT_FILENO, "player 1 wins game\n", 20);
                p1score++;
                kill(p1, SIGINT);
                kill(p2, SIGINT);
                break;
            }
            else if (guess2 == goal) {
                write(STDOUT_FILENO, "player 2 wins game\n", 20);
                p2score++;
                kill(p1, SIGINT);
                kill(p2, SIGINT);
                break;
            }

            //no winner ifs
            if (guess1 < goal) {
                checkError(kill(p1, SIGUSR2), "P: failed to send SIGUSR2 to player 1");
            } else { 
                checkError(kill(p1, SIGUSR1), "P: failed to send SIGUSR1 to player 1");
            }

            if (guess2 < goal) {
                checkError(kill(p2, SIGUSR2), "P: failed to send SIGUSR2 to player 2");
            } else {
                checkError(kill(p2, SIGUSR1), "P: failed to send SIGUSR1 to player 2");
            }

            sleep(2);
        }
        kill(p1, SIGINT);
        kill(p2, SIGINT);

    }
    printf("final score - player 1: %d, player 2: %d\n", p1score, p2score);
    printf("P: game over. terminating children.\n");
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
}

int main(int argc, char const *argv[])
{   
    printf("starting the game. . . . .\n");
    
    p1 = fork();

    checkError(p1, "failed to run player 1 fork");

    if(p1 == 0)
        player1Actions();

    p2 = fork();

    checkError(p2, "failed to run player 2 fork");

    if(p2 == 0)
    player2Actions();

    parentSignals();
    parentActions();
    return 0;
}


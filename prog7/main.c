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

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void pSigHndl (int sig){ //SIGUSR2 triggers here
    printf("(PID: %d): signal %d received in parent \n", getpid(), sig);
    fflush(stdout);
    
    int status;
    if (sig == SIGUSR1) {
        parent_received_sigusr1 = 1;
        printf("parent received signal from player 1\n");
        fflush(stdout);
    }
    if (sig == SIGUSR2) {
        parent_received_sigusr2 = 1;
        printf("parent received signal from player 2\n");
        fflush(stdout);
    }
    if (sig == SIGINT) {
        printf("received SIGINT, terminating children...\n");
        fflush(stdout);
        kill(0, SIGTERM);
    }
    if (sig == SIGCHLD) {
        while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("child process terminated\n");
            fflush(stdout);
        }
        if (waitpid(-1, NULL, WNOHANG) == -1) {
            printf("the children are all dead\n");
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
    }
}

void cSigHndl (int sig){
    printf("(PID %d): Received signal %d\n", getpid(), sig);
    fflush(stdout);
    if (sig == SIGUSR1) {
        gotsig1 = 1;
    }
    if (sig == SIGUSR2) {
        gotsig2 = 1;
    }
    if (sig == SIGINT) { 
        gotsigint = 1;
    }
    if (sig == SIGTERM) {
        printf("(PID %d):  got SIGTERM, exiting\n", getpid());
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }
}

int player1guess(int low,  int high){
    return (low + high) / 2; //player 1 should compute its guess by taking the average of min and max

}
int player2guess(int low, int high){
    srand(time(NULL) ^ getpid());
    return low + rand() % (high - low + 1); //player 2 should compute its guess by generating a random number between min and max

}

int childActions(int player){
    struct sigaction csa;
    csa.sa_handler = cSigHndl;
    sigemptyset(&csa.sa_mask);
    csa.sa_flags = 0;

    checkError(sigaction(SIGUSR1, &csa, NULL), "sigaction SIGUSR1");
    checkError(sigaction(SIGUSR2, &csa, NULL), "sigaction SIGUSR2");
    checkError(sigaction(SIGTERM, &csa, NULL), "sigaction SIGTERM");
    checkError(sigaction(SIGINT, &csa, NULL), "sigaction SIGINT");

    pause();
    
    while(1){ //game loop

        int guess;
        int low = 0; 
        int high = 101;

        pid_t parent_pid = getppid();
        if (player == 1) {
            checkError(kill(parent_pid, SIGUSR1), "error in sending SIGUSR1");
            printf("(PID: %d): Sent initial signal to parent\n", getpid());
        } else {
            checkError(kill(parent_pid, SIGUSR2), "error in sending SIGUSR2");
            printf("(PID: %d): Sent initial signal to parent\n", getpid());
        }
        fflush(stdout);

        printf("(PID: %d): waiting for response from parent before starting guess loop\n",getpid());
        fflush(stdout);
        pause();

        while(1) { // guess loop
            gotsig1 = 0;
            gotsig2 = 0;
            gotsigint = 0;

            if (player == 1) {
                guess = player1guess(low, high);
            } else {
                guess = player2guess(low, high);
            }

            // Write guess to file
            char buf[10]; 
            int len = snprintf(buf, sizeof(buf), "%d", guess);
            int fd;
            if (player == 1) {
                fd = checkError(open("guess1", O_WRONLY | O_CREAT | O_TRUNC, 0666), "open guess1");
            } else {
                fd = checkError(open("guess2", O_WRONLY | O_CREAT | O_TRUNC, 0666), "open guess2");
            }
            checkError(write(fd, buf, len), "write guess file");
            checkError(close(fd), "close guess file");

            sleep(1);
            // Signal parent
            pid_t parent_pid = getppid();
            if (player == 1) {
                checkError(kill(parent_pid, SIGUSR1), "error in sending SIGUSR1");
                printf("(PID: %d): Sent guess %d to parent\n", getpid(), guess);
            } else {
                checkError(kill(parent_pid, SIGUSR2), "error in sending SIGUSR2");
                printf("(PID: %d): Sent guess %d to parent\n", getpid(), guess);
            }
            fflush(stdout);

            // Wait for parent's response
            printf("(PID: %d): waiting for response from parent in guess loop\n",getpid());
            fflush(stdout);
            pause();
            
            if (gotsig1) {
                high = guess;
                printf("(PID %d): got SIGUSR1, going lower, range: [%d, %d]\n", getpid(), low, high);
            } 
            if (gotsig2) {
                low = guess;
                printf("(PID %d): got SIGUSR2, going higher, range: [%d, %d]\n", getpid(), low, high);
            }
            if (gotsigint) {
                printf("(PID %d): Restarting game\n", getpid());
                low = 0;
                high = 101;
                break;
            }
            fflush(stdout);
        }
    }
}

void parentActions(pid_t p1, pid_t p2){
    struct sigaction sa;
    sa.sa_handler = pSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    checkError(sigaction(SIGINT, &sa, NULL), "sigaction SIGINT");

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);

    int goal;
    int guess1, guess2;
    int p1score = 0;
    int p2score = 0;
    char buf[10];
    char buf2[10];

    srand(time(NULL));

    printf("P: sleeping for 5 seconds before starting the match\n");
    fflush(stdout);
    sleep(5);

    checkError(kill(p1, SIGUSR1), "P: failed to send SIGUSR1 to P1");
    checkError(kill(p2, SIGUSR2), "P: failed to send SIGUSR2 to P2");
    printf("P: told children to start match\n");
    fflush(stdout);

    for (int game = 1; game <= 10; game++) {
        printf("Starting Game %d,\n Score: P1 - %d | P2 - %d\n", game,p1score,p2score);
        fflush(stdout);

        //sigprocmask(SIG_BLOCK, &mask, &oldmask);
        while (!parent_received_sigusr1 || !parent_received_sigusr2) {
            pause();
            // sigset_t suspendMask;
            // sigemptyset(&suspendMask);
            // sigsuspend(&suspendMask);
        }
        //sigprocmask(SIG_SETMASK, &oldmask, NULL);
        //sigprocmask(SIG_UNBLOCK, &mask, NULL);
        parent_received_sigusr1 = 1;
        parent_received_sigusr2 = 1;
        printf("P: told children to restart game\n");
        fflush(stdout);

        //pause();

        goal = 1 + rand() % 100;
        game_over = 0;
        
        while (!game_over){
            printf("Parent: Waiting for child guesses\n");
            fflush(stdout);

            while (!parent_received_sigusr1 || !parent_received_sigusr2) {
                pause();
                // sigset_t suspendMask;
                // sigemptyset(&suspendMask);
                // sigsuspend(&suspendMask);
            }
            parent_received_sigusr1 = 0;
            parent_received_sigusr2 = 0;

            sigprocmask(SIG_BLOCK, &mask, &oldmask);

            // reading player 1 guesses
            int p1guessfd = checkError(open("guess1", O_RDONLY), "open guess1");
            int bytesRead1 = checkError(read(p1guessfd, buf, sizeof(buf) - 1), "read guess1");
            checkError(close(p1guessfd), "close guess1");
            buf[bytesRead1] = '\0';

            if (sscanf(buf, "%d", &guess1) != 1) {
                char errorMsg[] = "invalid guess format for p1\n";
                write(STDERR_FILENO, errorMsg, sizeof(errorMsg) - 1);
                exit(EXIT_FAILURE);
            }
            
            // reading player 2 guesses
            int p2guessfd = checkError(open("guess2", O_RDONLY), "open guess2");
            int bytesRead2 = checkError(read(p2guessfd, buf2, sizeof(buf2) - 1), "read guess2");
            checkError(close(p2guessfd), "close guess2");
            buf2[bytesRead2] = '\0';

            if (sscanf(buf2, "%d", &guess2) != 1) {
                char errorMsg[] = "invalid guess format for p2\n";
                write(STDERR_FILENO, errorMsg, sizeof(errorMsg) - 1);
                exit(EXIT_FAILURE);
            }

            printf("Parent: Read guesses -> Player 1: %d, Player 2: %d (Goal: %d)\n", guess1, guess2, goal);
            fflush(stdout);

            // --- determining winner
            if (guess1 == goal && guess2 == goal) {
                printf("Tie!\n");
                fflush(stdout);
                p1score++;
                p2score++;
                game_over = 1;
            }
            else if (guess1 == goal) {
                printf("P1 win\n");
                fflush(stdout);
                p1score++;
                game_over = 1;
            }
            else if (guess2 == goal) {
                printf("P2 win\n");
                fflush(stdout);
                p2score++;
                game_over = 1;
            }
            sigprocmask(SIG_UNBLOCK, &mask, NULL);
            //no winner ifs
            if (guess1 < goal) {
                checkError(kill(p1, SIGUSR2), "Parent: Failed to send SIGUSR2 to Player 1");
                printf("Parent: Telling Player 1 to guess higher\n");
                fflush(stdout);
            } else { 
                checkError(kill(p1, SIGUSR1), "Parent: Failed to send SIGUSR1 to Player 1");
                printf("Parent: Telling Player 1 to guess lower\n");
                fflush(stdout);
            }

            if (guess2 < goal) {
                checkError(kill(p2, SIGUSR2), "Parent: Failed to send SIGUSR2 to Player 2");
                printf("Parent: Telling Player 2 to guess higher\n");
                fflush(stdout);
            } else {
                checkError(kill(p2, SIGUSR1), "Parent: Failed to send SIGUSR1 to Player 2");
                printf("Parent: Telling Player 2 to guess lower\n");
                fflush(stdout);
            }

            if (game_over) {
                checkError(kill(p1, SIGINT), "Parent: Failed to send SIGINT to Player 1");
                checkError(kill(p2, SIGINT), "Parent: Failed to send SIGINT to Player 2");
                printf("Parent: Reset signal sent to both children\n");
                fflush(stdout);

                // X: Wait for children to reset
                sleep(2);
            }


        }
    }
    printf("Final Score - Player 1: %d, Player 2: %d\n", p1score, p2score);
    printf("Parent: Game over. Terminating children.\n");
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
    
    printf("Starting the game!\n");
    
    pid_t pids[2];

    for (int i = 0; i < 2; i++) {
        printf("About to fork child %d\n", i + 1);
        fflush(stdout);

        pids[i] = fork();
        if (pids[i] == -1) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        } else if (pids[i] == 0) {
            printf("Child %d process started. PID: %d\n", i + 1, getpid());
            fflush(stdout);
            childActions(i + 1);
            exit(0);
        }
        printf("Forked child %d, PID: %d\n", i + 1, pids[i]);
        fflush(stdout);
    }

    parentActions(pids[0], pids[1]);
    return 0;
}


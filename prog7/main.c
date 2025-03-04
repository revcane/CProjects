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

volatile sig_atomic_t readycount = 0;
volatile sig_atomic_t game_over = 0; 

int checkError(int val, const char *msg){
    if (val == -1)
   {
       perror(msg);
       exit(EXIT_FAILURE);
   }
return val;
}

void pSigHndl (int sig){
    printf("signal %d received in parent PID: %d\n", sig, getpid());
    fflush(stdout);
    
    int status;
    static int children_ready = 0;
    if (sig == SIGINT) {
        printf("Parent: Received SIGINT, terminating children...\n");
        fflush(stdout);
        kill(0, SIGTERM);
        exit(EXIT_SUCCESS);
    }
    if (sig == SIGCHLD) {
        while (waitpid(-1, &status, WNOHANG) > 0) {
            printf("Parent: Child process terminated\n");
            fflush(stdout);
        }
        if (waitpid(-1, NULL, WNOHANG) == -1) {
            printf("Parent: No more children, exiting...\n");
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }
    }
    if (sig == SIGUSR1 || sig == SIGUSR2) {
        children_ready++;
        readycount = children_ready;
        printf("✅ Parent: Child handshake received. Ready count: %d\n", readycount);
        fflush(stdout);
    }
}

void cSigHndl (int sig){
    static int low = 0, high = 100; // bounds
    printf("Child (PID: %d): Received signal %d\n", getpid(), sig); 
    fflush(stdout);
    if (sig == SIGUSR1) {
        high = (low + high) / 2; // change upper bound
        printf("Child (PID: %d): got SIGUSR1, going lower, range: [%d, %d]\n", getpid(), low, high);
        fflush(stdout);
    }
    if (sig == SIGUSR2) {
        low = (low + high) / 2; // change lower bound
        printf("Child (PID: %d): got SIGUSR2, going higher, range: [%d, %d]\n", getpid(), low, high);
        fflush(stdout);
    }
    if (sig == SIGTERM || sig == SIGINT) {
        printf("Child (PID: %d): Received %s. Exiting...\n", getpid(), (sig == SIGTERM) ? "SIGTERM" : "SIGINT");
        fflush(stdout);
        if (sig == SIGINT) {
            low = 0;
            high = 100;
            kill(getppid(), SIGUSR1);
        } else {
            exit(EXIT_SUCCESS);
        }
    }
}

int player1guess(int low,  int high){
   // Player 1 should compute its guess by taking the average of min and max
    return (low + high) / 2;
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
    sigemptyset(&csa.sa_mask);
    csa.sa_flags = 0;

    checkError(sigaction(SIGUSR1, &csa, NULL), "sigaction SIGUSR1");
    checkError(sigaction(SIGUSR2, &csa, NULL), "sigaction SIGUSR2");
    checkError(sigaction(SIGTERM, &csa, NULL), "sigaction SIGTERM");
    checkError(sigaction(SIGINT, &csa, NULL), "sigaction SIGINT");

    int guess;
    int low = 0; 
    int high = 100;

    pid_t parent_pid = getppid();
    if (player == 1) {
        kill(parent_pid, SIGUSR1);
    } else {
        kill(parent_pid, SIGUSR2);
    }

    while(1) {
        if (player == 1) {
            guess = player1guess(low, high);
        } else {
            guess = player2guess(low, high);
        }

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

        parent_pid = getppid();
        if (parent_pid == 1) {
            printf("Child (PID: %d): Parent has terminated. Exiting...\n", getpid());
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }

        int signal_to_send = (player == 1) ? SIGUSR1 : SIGUSR2; // X: Use consistent signal naming
        printf("Child %d (PID: %d): Sending signal %d to parent (PID: %d)\n", player, getpid(), signal_to_send, parent_pid);
        fflush(stdout);
        if (kill(parent_pid, signal_to_send) == -1) {
            perror("Child: Failed to send signal to parent");
            exit(EXIT_FAILURE);
        }

        sleep(1);
        printf("Child (PID: %d): Sleeping in child action\n", getpid());
        fflush(stdout);

        pause();
    }
}

void parentActions(pid_t p1, pid_t p2){
    int goal;
    int guess1, guess2;
    int p1score = 0;
    int p2score = 0;
    char buf[10];
    char buf2[10];

    srand(time(NULL));  // Called once at the beginning

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);

    for (int game = 1; game <= 10; game++) {
        printf("Starting Game %d\n", game);
        fflush(stdout);

        goal = 1 + rand() % 100;
        game_over = 0; // X: Reset game_over flag
        
        while (!game_over) { // X: Use game_over flag
            printf("Parent: Waiting for child guesses...\n");
            fflush(stdout);

            sigprocmask(SIG_BLOCK, &mask, &oldmask);

            while (readycount < 2) {
                sigsuspend(&oldmask);
            }
            readycount = 0;

            sigprocmask(SIG_UNBLOCK, &mask, NULL);

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

            printf("Parent: Read guesses -> Player 1: %d, Player 2: %d (Goal: %d)\n", guess1, guess2, goal); // X: Added goal to debug output
            fflush(stdout);

            if (guess1 < goal){ // player1
                checkError(kill(p1, SIGUSR2), "Parent: Failed to send SIGUSR2 to Player 1"); // X: Added error checking
                printf("Parent: Telling Player 1 to guess higher\n");
                fflush(stdout);
            } 
            else { 
                checkError(kill(p1, SIGUSR1), "Parent: Failed to send SIGUSR1 to Player 1"); // X: Added error checking
                printf("Parent: Telling Player 1 to guess lower\n");
                fflush(stdout);
            }

            if (guess2 < goal){ // player2
                checkError(kill(p2, SIGUSR2), "Parent: Failed to send SIGUSR2 to Player 2"); // X: Added error checking
                printf("Parent: Telling Player 2 to guess higher\n");
                fflush(stdout);
            } 
            else {
                checkError(kill(p2, SIGUSR1), "Parent: Failed to send SIGUSR1 to Player 2"); // X: Added error checking
                printf("Parent: Telling Player 2 to guess lower\n");
                fflush(stdout);
            }

            // --- determining winner
            if (guess1 == goal && guess2 == goal) {
                printf("It's a tie!\n");
                fflush(stdout);
                p1score++;
                p2score++;
                game_over = 1; // X: Set game_over flag
            }
            else if (guess1 == goal) {
                printf("Player 1 wins!\n");
                fflush(stdout);
                p1score++;
                game_over = 1; // X: Set game_over flag
            }
            else if (guess2 == goal) {
                printf("Player 2 wins!\n");
                fflush(stdout);
                p2score++;
                game_over = 1; // X: Set game_over flag
            }

            if (game_over) {
                // X: Send reset signal only once
                checkError(kill(p1, SIGINT), "Parent: Failed to send SIGINT to Player 1");
                checkError(kill(p2, SIGINT), "Parent: Failed to send SIGINT to Player 2");
                printf("Parent: Reset signal sent to both children\n");
                fflush(stdout);

                // Wait for children to acknowledge reset
                readycount = 0;
                while (readycount < 2) {
                    sigsuspend(&oldmask);
                }
                printf("Parent: Both children have reset\n");
                fflush(stdout);
            }
        }
    }
    printf("Final Score - Player 1: %d, Player 2: %d\n", p1score, p2score);
    fflush(stdout);

    printf("Parent: Game over. Terminating children.\n");
    fflush(stdout);
    kill(p1, SIGTERM);
    kill(p2, SIGTERM);
    
    int status;
    waitpid(p1, &status, 0);
    waitpid(p2, &status, 0);
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
    checkError(sigaction(SIGINT, &sa, NULL), "sigaction SIGINT");
    
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
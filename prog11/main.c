#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

pid_t p1;
pid_t p2;

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void waitPlayer(int fd, const char* label) {
    char buf[32];
    ssize_t n;
    do {
        n = checkError(read(fd, buf, sizeof(buf) - 1), "failed to read READY");
        buf[n] = '\0';
    } while (strncmp(buf, "READY", 5) != 0);
    printf("%s is ready.\n", label);
}


void player1Actions(){
    int fd_PtoC1 = checkError(open("/tmp/PtoC1", O_RDONLY),"failed to open PtoC1 fifo");
    int fd_C1toP = checkError(open("/tmp/C1toP", O_WRONLY),"failed to open C1toP fifo");

    while(1){ //game loop
        int guess;
        int low = 0;
        int high = 101;
        int cont = 0;

        char signal[32];
        ssize_t bytes = checkError(read(fd_PtoC1, signal, sizeof(signal) - 1),"failed to read PtoC1");
        signal[bytes] = '\0';

        printf("1SIGNAL\n");
        if (strncmp(signal, "START", 5) == 0){
            checkError(write(fd_C1toP, "READY\n", 6), "failed to send READY after START");
        } else {
            printf("continuing\n");
            cont = 1;
            continue; 
        }

        bytes = checkError(read(fd_PtoC1, signal, sizeof(signal) - 1),"failed to read PtoC1");
        signal[bytes] = '\0';

        if (strncmp(signal, "GUESS", 5) == 0 || cont == 1){
            while(1) { // guess loop
                guess = (low + high) / 2;

                char msg[64];
                snprintf(msg, sizeof(msg), "GUESS:%d\n", guess);
                checkError(write(fd_C1toP, msg, strlen(msg)), "failed to send guess to parent");

                char feedback[32] = {0};
                bytes = checkError(read(fd_PtoC1, feedback, sizeof(feedback) - 1),"failed to read PtoC1");
                feedback[bytes] = '\0';

                if (strncmp(feedback, "HIGH", 4) == 0){
                    high = guess - 1;
                    //checkError(write(fd_C1toP, "READY\n", 6), "failed to send READY after LOW");
                }
                else if (strncmp(feedback, "LOW", 3) == 0){
                    low = guess + 1;
                    //checkError(write(fd_C1toP, "READY\n", 6), "failed to send READY after HIGH");
                }
                else if (strncmp(feedback, "CORRECT", 7) == 0){
                    break; // correct guess
                }
                else if (strncmp(feedback, "EXIT", 4) == 0){
                    printf("p1: exiting\n");
                    close(fd_PtoC1);
                    close(fd_C1toP);
                    exit(EXIT_SUCCESS);
                }
            }
        }
        else {
        printf("blehh");
        }
    }
    close(fd_PtoC1);
    close(fd_C1toP);
    exit(EXIT_SUCCESS);
}

void player2Actions(){
    srand(time(NULL));

    int fd_PtoC2 = checkError(open("/tmp/PtoC2", O_RDONLY),"failed to open PtoC2 fifo");
    int fd_C2toP = checkError(open("/tmp/C2toP", O_WRONLY),"failed to open C2toP fifo");

    while(1){ //game loop
        int guess;
        int low = 0;
        int high = 101;
        int cont = 0;

        char signal[32];
        ssize_t bytes = checkError(read(fd_PtoC2, signal, sizeof(signal) - 1),"failed to read PtoC2");
        signal[bytes] = '\0';

        printf("2SIGNAL\n");
        if (strncmp(signal, "START", 5) == 0){
            checkError(write(fd_C2toP, "READY\n", 6), "failed to send READY after START");
        } else {
            printf("continuing2\n");
            cont = 1;
            continue; 
        }

        bytes = checkError(read(fd_PtoC2, signal, sizeof(signal) - 1),"failed to read PtoC2");
        signal[bytes] = '\0';

        if (strncmp(signal, "GUESS", 5) == 0 || cont == 1){
            while(1) { // guess loop
                guess = low + rand() % (high - low + 1);

                char guess_msg[64];
                snprintf(guess_msg, sizeof(guess_msg), "GUESS:%d\n", guess);
                checkError(write(fd_C2toP, guess_msg, strlen(guess_msg)), "failed to send guess");

                char feedback[32] = {0};
                bytes = checkError(read(fd_PtoC2, feedback, sizeof(feedback) - 1),"failed to read PtoC2");
                feedback[bytes] = '\0';

                if (strncmp(feedback, "HIGH", 4) == 0){
                    high = guess - 1;
                    //checkError(write(fd_C2toP, "READY\n", 6), "failed to send READY after LOW");
                }
                else if (strncmp(feedback, "LOW", 3) == 0){
                    low = guess + 1;
                    //checkError(write(fd_C2toP, "READY\n", 6), "failed to send READY after HIGH");
                }
                else if (strncmp(feedback, "CORRECT", 7) == 0){
                    break; // correct guess
                }
                else if (strncmp(feedback, "EXIT", 4) == 0){
                    printf("p2: exiting\n");
                    close(fd_PtoC2);
                    close(fd_C2toP);
                    exit(EXIT_SUCCESS);
                }
            }
        }
        else {
            printf("blehh");
        }
    }
    close(fd_PtoC2);
    close(fd_C2toP);
    exit(EXIT_SUCCESS);
}

void parentActions(){
    int goal;
    int guess1, guess2;
    int p1score = 0;
    int p2score = 0;

    int fd_PtoC2 = checkError(open("/tmp/PtoC2", O_WRONLY),"failed to open PtoC2 fifo");
    int fd_PtoC1 = checkError(open("/tmp/PtoC1", O_WRONLY),"failed to open PtoC1 fifo");
    int fd_C2toP = checkError(open("/tmp/C2toP", O_RDONLY),"failed to open C2toP fifo");
    int fd_C1toP = checkError(open("/tmp/C1toP", O_RDONLY),"failed to open C1toP fifo");

    srand(time(NULL));

    write(STDOUT_FILENO, "sleeping for 5 seconds before starting match\n",46);
    sleep(5);

    checkError(write(fd_PtoC1, "START\n", 6),"error sending start signal PtoC1");
    checkError(write(fd_PtoC2, "START\n", 6),"error sending start signal PtoC2");

    waitPlayer(fd_C1toP, "Player 1");
    waitPlayer(fd_C2toP, "Player 2");


    for (int game = 1; game <= 10; game++) {
        goal = 1 + rand() % 100;
        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "starting game %d,\n score: P1 - %d | P2 - %d\n", game, p1score, p2score);
        write(STDOUT_FILENO, buffer, len);
        printf("doididgndngnidigndgd\n");

        //waitPlayer(fd_C1toP, "Player 1");
        //waitPlayer(fd_C2toP, "Player 2");
        
        checkError(write(fd_PtoC1, "GUESS\n", 6),"error sending guess signal PtoC1"); //tell children to guess
        checkError(write(fd_PtoC2, "GUESS\n", 6),"error sending guess signal PtoC2");
        printf("BINGINBIGNIBIGINBGINBIGBING\n");

        while (1){
            char response1[64] = {0}; //read guess
            char response2[64] = {0};
            ssize_t bytes1 = 0, bytes2 = 0;
            char *ptr1 = response1, *ptr2 = response2;

            while (bytes1 < sizeof(response1) - 1){// wait for complete message
                ssize_t n = checkError(read(fd_C1toP, ptr1, sizeof(response1) - 1 - bytes1),"failed to read temp from C1toP");
                if (n <= 0) break;
                bytes1 += n;
                ptr1 += n;
                if (ptr1[-1] == '\n') break;
            }
            response1[bytes1] = '\0';

            while (bytes2 < sizeof(response2) - 1){
                ssize_t n = checkError(read(fd_C2toP, ptr2, sizeof(response2) - 1 - bytes2),"failed to read temp from C2toP");
                if (n <= 0) break;
                bytes2 += n;
                ptr2 += n;
                if (ptr2[-1] == '\n') break;
            }
            response2[bytes2] = '\0';

            printf("Parent read from C1toP: %s\n", response1); //debug delete
            printf("Parent read from C2toP: %s\n", response2);

            if (sscanf(response1, "GUESS:%d", &guess1) != 1){
                write(STDERR_FILENO, "player 1 had invalid guess\n", 28);
                break;
            }
            if (sscanf(response2, "GUESS:%d", &guess2) != 1){
                write(STDERR_FILENO, "player 2 had invalid guess\n", 28);
                break;
            }

            printf("player 1: %d, player 2: %d (goal: %d)\n", guess1, guess2, goal);

            if (guess1 == goal && guess2 == goal){
                write(STDOUT_FILENO, "players tied\n", 14);
                p1score++;
                p2score++;
                checkError(write(fd_PtoC1, "CORRECT\n", 8),"failed to send correct PtoC1");
                checkError(write(fd_PtoC2, "CORRECT\n", 8),"failed to send correct PtoC2");
                break;
            }
            else if (guess1 == goal){
                write(STDOUT_FILENO, "player 1 wins game\n", 20);
                p1score++;
                checkError(write(fd_PtoC1, "CORRECT\n", 8),"failed to send correct PtoC1");
                checkError(write(fd_PtoC2, "CORRECT\n", 8),"failed to send correct PtoC2");
                break;
            }
            else if (guess2 == goal){
                write(STDOUT_FILENO, "player 2 wins game\n", 20);
                p2score++;
                checkError(write(fd_PtoC1, "CORRECT\n", 8),"failed to send correct PtoC1");
                checkError(write(fd_PtoC2, "CORRECT\n", 8),"failed to send correct PtoC2");
                break;
            }

            if (guess1 < goal){
                checkError(write(fd_PtoC1, "LOW\n", 4),"error sending low message PtoC1");
            } else {
                checkError(write(fd_PtoC1, "HIGH\n", 5),"error sending high message PtoC1");
            }

            if (guess2 < goal){
                checkError(write(fd_PtoC2, "LOW\n", 4),"error sending low message PtoC2");
            } else {
                checkError(write(fd_PtoC2, "HIGH\n", 5),"error sending high message PtoC2");
            }

            sleep(1);
        }
        checkError(write(fd_PtoC1, "CORRECT\n", 7),"failed to send correct PtoC1");
        checkError(write(fd_PtoC2, "CORRECT\n", 7),"failed to send correct PtoC2");
    }
    printf("final score - player 1: %d, player 2: %d\n", p1score, p2score);
    printf("P: game over. terminating children.\n");
    checkError(write(fd_PtoC1, "EXIT\n", 6),"failed to send exit PtoC1");
    checkError(write(fd_PtoC2, "EXIT\n", 6),"failed to send exit PtoC2");

    close(fd_PtoC1);
    close(fd_PtoC2);
    close(fd_C1toP);
    close(fd_C2toP);
}

int main(int argc, char const *argv[])
{   
    printf("starting the game. . . . .\n");
    
    mkfifo("/tmp/C1toP", 0666);
    mkfifo("/tmp/C2toP", 0666);
    mkfifo("/tmp/PtoC1", 0666);
    mkfifo("/tmp/PtoC2", 0666);

    p1 = fork();
    checkError(p1, "failed to run player 1 fork");

    if(p1 == 0)
        player1Actions();
    p2 = fork();
    checkError(p2, "failed to run player 2 fork");

    if(p2 == 0)
        player2Actions();
    parentActions();

    unlink("/tmp/C1toP");
    unlink("/tmp/C2toP");
    unlink("/tmp/PtoC1");
    unlink("/tmp/PtoC2");
    return 0;
}
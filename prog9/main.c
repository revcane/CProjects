#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>

int guess[2]; //storing guesses
int cmp[2]; 
int rdy[4];

pthread_mutex_t mtx[4];
pthread_cond_t cnd[4];
volatile int endg = 0;

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

int checkThread(int val, const char *msg){
    if (val != 0) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void *player1Actions(void *args){
    printf("i am 0\n");
    while(!endg){ //game loop
        printf("locking mtx[2] in player 1\n");
        pthread_mutex_lock(&mtx[2]);
        printf("locked mtx[2] in player 1\n");
        while (rdy[2] == 0 && !endg){
            printf("p1 waiting for game start, rdy[2] = %d\n", rdy[2]);
            pthread_cond_wait(&cnd[2], &mtx[2]);
        }
        if (endg){ 
            pthread_mutex_unlock(&mtx[2]);
            break; 
        }

        rdy[2] = 0; //set looping flag to 0
        printf("set player1 to rdy to 0\n");

        pthread_mutex_unlock(&mtx[2]); //unlock mutex3
        
        int low = 0; 
        int high = 100;

        while(1) { // guess loop
            printf("locking mtx[0] in player 1\n");
            pthread_mutex_lock(&mtx[0]); //lock associated mutex (mutex 1)
            printf("locked mtx[0] in player 1\n");
            printf("p2 high: %d low:%d\n",high,low);

            if (high < low) {
                printf("error: out of range | high: %d low: %d\n",high,low);
                exit(EXIT_FAILURE);
            }
            printf("p1 guessing\n"); 
            guess[0] = (low + high) / 2; //calculate guess
            printf("p1 guessed: %d\n", guess[0]); 
            
            rdy[0] = 1;
            pthread_cond_signal(&cnd[0]); 

            while (rdy[0] == 1 && !endg){
                printf("p1 waiting on rdy[0] = %d\n", rdy[0]);
                pthread_cond_wait(&cnd[0], &mtx[0]);
            }

            if (endg){ 
                pthread_mutex_unlock(&mtx[0]);
                break; 
            }

            //printf("comparison1: %d,%d\n",cmp[0],guess[0]);
            if (cmp[0] < 0){
                low = guess[0] + 1; //low
            }
            else if (cmp[0] > 0){
                high = guess[0] - 1; //high
            }
            else {
                pthread_mutex_unlock(&mtx[0]); 
                break; //correct
            }
            pthread_mutex_unlock(&mtx[0]); //unlock mutex
        }
    }
    return NULL;
}

void *player2Actions(void *args){
    printf("i am 1\n");
    while (!endg){ //game loop
        printf("locking mtx[2] in player 2\n");
        pthread_mutex_lock(&mtx[2]); //lock mutex3
        printf("locked mtx[2] in player 2\n");
        while (rdy[2] == 0 && !endg){
            printf("p2 waiting for game start, rdy[2] = %d\n", rdy[2]);
            pthread_cond_wait(&cnd[2], &mtx[2]);
        }
        if (endg){ 
            pthread_mutex_unlock(&mtx[2]);
            break;
        }

        rdy[2] = 0; //set looping flag to 0
        printf("set player2 to rdy to 0\n");

        pthread_mutex_unlock(&mtx[2]);
       
        int low = 0;  //setting min max
        int high = 100;

        while(!endg){ // guess loop
            printf("locking mtx[1] in player 2\n");
            pthread_mutex_lock(&mtx[1]); //lock associated mutex (mutex 2)
            printf("locked mtx[1] in player 2\n");
            printf("p1 high: %d low:%d\n",high,low);

            if (high < low){
                printf("error: out of range | high: %d low: %d\n",high,low);
                pthread_mutex_unlock(&mtx[1]);
                exit(EXIT_FAILURE);
            }
            printf("p2 guessing\n");

            guess[1] = low + rand() % (high - low + 1); //calculate guess
            printf("p2 guessed: %d\n", guess[1]); 

            rdy[1] = 1;
            pthread_cond_signal(&cnd[1]); 

            while (rdy[1] == 1 && !endg) {
                printf("p2 waiting on rdy[1] = %d\n", rdy[1]);
                pthread_cond_wait(&cnd[1], &mtx[1]);
            }

            if (endg) {
                pthread_mutex_unlock(&mtx[1]);
                break;
            }

            //printf("comparison2: %d,%d\n",cmp[1],guess[1]);
            if (cmp[1] < 0){
                low = guess[1] + 1; //low
            }
            else if (cmp[1] > 0){
                high = guess[1] - 1; //high
            }
            else {
                pthread_mutex_unlock(&mtx[1]);
                break; //correct
            }
            pthread_mutex_unlock(&mtx[1]); //unlock mutex
        }
    }
    return NULL;
}

void *parentActions(void *args){
    int goal;
    int p1score = 0;
    int p2score = 0;

    for (int game = 1; game <= 10; game++) {
        printf("locking mtx[2] in parent\n");
        pthread_mutex_lock(&mtx[2]); //lock mutex 3
        printf("locked mtx[2] in parent\n");
        goal = 1 + rand() % 100; //geneerate

        rdy[2] = 1; //set flags to 1
        printf("set parent rdy[2] to 1\n");

        printf("broadcasting cnd 2\n");
        pthread_cond_broadcast(&cnd[2]); //broadcast and lock
        pthread_mutex_unlock(&mtx[2]);

        char buffer[50];
        int len = snprintf(buffer, sizeof(buffer), "starting game %d,\n score: P1 - %d | P2 - %d\n", game, p1score, p2score);
        write(STDOUT_FILENO, buffer, len);
        
        while (1){ 
            sleep(1);

            printf("player 1: %d, player 2: %d (goal: %d)\n", guess[0], guess[1], goal);
            printf("locking mtx[0] in parent\n");
            pthread_mutex_lock(&mtx[0]); //lock mutex 1 and 2
            printf("locked mtx[0] in parent\n");

            printf("locking mtx[1] in parent\n");
            pthread_mutex_lock(&mtx[1]);
            printf("locked mtx[1] in parent\n");
            //determining winner with cmp
            cmp[0] = guess[0] - goal;
            cmp[1] = guess[1] - goal;
            if (cmp[0] == 0 && cmp[1] == 0){
                write(STDOUT_FILENO, "players tied\n", 14);
                p1score++;
                p2score++;
                break;
            }
            else if (cmp[0] == 0){
                write(STDOUT_FILENO, "player 1 wins game\n", 20);
                p1score++;
                break;
            }
            else if (cmp[1] == 0){
                write(STDOUT_FILENO, "player 2 wins game\n", 20);
                p2score++;
                break;
            }

            rdy[0] = 0; //setting flags again
            rdy[1] = 0;
            printf("set parent rdy[0] and rdy[1] to 0\n");
            pthread_cond_signal(&cnd[0]); //broadcast
            pthread_cond_signal(&cnd[1]);

            pthread_mutex_unlock(&mtx[1]);
            pthread_mutex_unlock(&mtx[0]);
        }
        printf("locking mtx[0] in parent\n");
        pthread_mutex_lock(&mtx[1]); //lock mutex 1 and 2
        printf("locked mtx[0] in parent\n");

        printf("locking mtx[1] in parent\n");
        pthread_mutex_lock(&mtx[0]);
        printf("locked mtx[1] in parent\n");

        rdy[0] = 0;
        rdy[1] = 0;
        printf("set parent rdy[0] and rdy[1] to 0 outside of loop\n");
        pthread_cond_signal(&cnd[0]);
        pthread_cond_signal(&cnd[1]);

        pthread_mutex_unlock(&mtx[0]);
        pthread_mutex_unlock(&mtx[1]);

        pthread_mutex_lock(&mtx[3]);
        rdy[2] = 1; //reset game
        printf("set parent rdy[2] to 1, resetting round\n");
        pthread_cond_broadcast(&cnd[2]);
        pthread_mutex_unlock(&mtx[3]);

        
    }
    printf("locking mtx[3] in parent\n");
    pthread_mutex_lock(&mtx[3]);
    printf("locked mtx[3] in parent\n");
    endg = 1;
    pthread_mutex_unlock(&mtx[3]);

    pthread_cond_broadcast(&cnd[0]); //broadcast
    pthread_cond_broadcast(&cnd[1]);
    pthread_cond_broadcast(&cnd[2]);

    printf("final score - player 1: %d, player 2: %d\n", p1score, p2score);
    printf("P: game over. terminating children.\n");
    return NULL;
}

int main(int argc, char const *argv[])
{  
    srand(time(NULL));
    pthread_t thrds[3];
    printf("starting the game. . . . .\n");

    for (int i = 0; i < 4; i++) { //initalizing mutxes and conditionals
        rdy[i] = 0;
    }
    
    for (int i = 0; i < 4; i++) { //initalizing mutxes and conditionals
        checkThread(pthread_mutex_init(&mtx[i], NULL),"error intializing mutex");
        checkThread(pthread_cond_init(&cnd[i], NULL),"error initalizing conditionals");
        printf("%d\n",i);   
   }

    //creating the threads
    checkThread(pthread_create(&thrds[0], NULL, player1Actions, NULL),"error creating p1 thread");
    checkThread(pthread_create(&thrds[1], NULL, player2Actions, NULL),"error creating p2 thread");
    checkThread(pthread_create(&thrds[2], NULL, parentActions, NULL),"error creating p1 thread");

    checkThread(pthread_join(thrds[2], NULL),"error joining referee thread"); //waiting for ref to join

    for (int i = 0; i < 4; i++) {
        checkThread(pthread_mutex_destroy(&mtx[i]),"error destroying mutex");
        checkThread(pthread_cond_destroy(&cnd[i]),"error destroying conditionals");
    }

    return 0;
}


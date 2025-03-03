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
    if (sig == SIGINT) {
        //SIGINT should send a SIGTERM to both children
    }
    if (sig == SIGCHLD) {
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
    if (sig == SIGUSR1) {

    }
    if (sig == SIGUSR2) {

    }
}

void cSigHndl (int sig){
    if (sig == SIGUSR1) {
        //high guess
    }
    if (sig == SIGUSR2) {
        //low guess
    }
    if (sig == SIGTERM) {
        //exit child
    }
    if (sig == SIGINT) {
        //SIGINT tells the child to stop guessing a start over
    }
}

int player1guess(int x,  int y){
   // Player 1 should compute its guess by taking the average of min and max

}
int player2guess(){
    /*
    Player 2 should compute its guess by generating a random number between min and max
    Remember to seed the random number generator using srand
    it is a good idea to the generate a few random numbers to prevent the parent and child 2 from generating the same random numbers every time.
    */
}

int childActions(){
    struct sigaction csa; //child signal handler
    csa.sa_handler = cSigHndl;
    csa.sa_flags = 0;
    sigemptyset(&csa.sa_mask);
    
    sigaction(SIGTERM, &csa, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL); //masking sigint so children dont explode
    //Wait until the parent has sent SIGUSR1 or SIGUSR2
    
    /*
    Loop forever -- game loop
    Set min to 0, max to 101
    Signal the parent
        In my game, child 1 sends SIGUSR1, child 2 sends SIGUSR2
    Loop forever -- guess loop
        Set the flags that store handling SIGUSR1, SIGUS2, and SIGINT to 0
        Each child should guess a value based on its strategy (see below)
        Write the guess into a file. Each child should have its own file.
        Sleep for 1 second
        Signal the parent
        In my game, child 1 sends SIGUSR1, child 2 sends SIGUSR2
        Wait until the parent responds with a signal
        If the parent responds with SIGUSR1 the guess was low, so set min to the guess
        If the parent responds with SIGUSR2 the guess was high, so set max to the guess
        If the parent responds with SIGINT the parent is moving to the next game, exit the guess loop
    */

}

int parentActions(){
    /*
    Set the disposition for SIGINT
    Sleep for 5 seconds
    Tell the children to begin playing the game
        In my game, I do this by sending SIGUSR1 to child 1 and SIGUSR2 to child 2  
    Loop from 0 to 10 (this is the game counter)
        Wait until both children have signaled they are ready to begin
            This basically means you are waiting until the parent handles both SIGUSR1 and SIGUSR2
        Display statistics for the game ... so display game number, total wins per player, etc
        Generate the target for this game -- the target is a random number between 1 and 100
    Loop forever -- referee loop
        Wait until both children have signaled they have made a guess
            This basically means you are waiting until the parent handles both SIGUSR1 and SIGUSR2
        Open the files containing the guesses and read the guesses
        Close the files
        Perform the following for each player
            Compare their guess with the target and store the result
        If either player wins,
            check each player for a win and increment their score by 1
            break the referee loop
        otherwise perform the following for each player
            if guess < target, send SIGUSR1 to the correct child
            if guess > target, send SIGUSR2 to the correct child
        send SIGINT to both children to tell them to reset their game play loop
    Print the final results
    */
}
int main(int argc, char const *argv[])
{
    struct sigaction sa;
    sa.sa_handler = sigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGCHLD, &sa, NULL);
    //sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    //Use fork to spawn child 1 and child 2.
    //Call the parent function

    return 0;
}

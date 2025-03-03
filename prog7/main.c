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
    
    }
    if (sig == SIGCHLD) {

    }
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

    }
    if (sig == SIGINT) {

    }
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



}
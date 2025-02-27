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

void sigHndl(int sig) {
    int status;
    char response[10];

    if (sig == SIGINT) {
        write(STDOUT_FILENO, "\nDo you really want to exit? (y/n): ", 36);
        if (read(STDIN_FILENO, response, sizeof(response)) == -1) {
            perror("Error reading input");
            exit(EXIT_FAILURE);
        }
        if (response[0] == 'y' || response[0] == 'Y') {
            kill(0, SIGTERM);
            exit(EXIT_SUCCESS);
        }
    }
    if (sig == SIGCHLD){
        while (waitpid(-1, &status, WNOHANG) > 0) { //removing child processes
            printf("shot the child :(\n");
        }

        if (waitpid(-1, NULL, WNOHANG) == -1) { //if it returns -1 (error)
            printf("no more children, goodbye\n"); 
            exit(EXIT_SUCCESS);
        }
    }
    if (sig == SIGUSR1){
        printf("Warning! Roll outside of bounds\n");
    }
    if (sig == SIGUSR2)
    {
        printf("Warning! Pitch outside of bounds\n");
    }
    if (sig == SIGTERM){
        printf("child sigterm, exiting\n");
        exit(EXIT_SUCCESS);
    }
    
}

int checkRange(double value1){
    return(value1 <= 20.0 && value1 >= -20.0);
}

int childActions(){
    struct timespec ts = {1, 0};
    pid_t pid = getpid();

    struct sigaction csa; //child signal handler
    csa.sa_handler = sigHndl;
    csa.sa_flags = 0;
    sigemptyset(&csa.sa_mask);
    
    sigaction(SIGTERM, &csa, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigprocmask(SIG_BLOCK, &mask, NULL); //masking sigint so children dont explode

    printf("child pid: %d started\n",pid);

    int fd;

    double pitch;
    double roll;
    double yaw;

    fd = checkError(open("angl.dat",O_RDONLY), "error in opening angl.dat\n");

    while (read(fd, &roll, sizeof(double)) == sizeof(double) && read(fd, &pitch, sizeof(double)) == sizeof(double) && read(fd, &yaw, sizeof(double)) == sizeof(double)) {
        if (checkRange(roll)) { //straight out of program 4, loops reading as long as there is content to read
            printf("Roll: %f (INSIDE), ", roll);
        } 
        else {
            printf("Roll: %f (OUTSIDE), ", roll);
            kill(getppid(), SIGUSR1);
        }
        if (checkRange(pitch)) {
            printf("Pitch: %f (INSIDE)\n", pitch);
        } 
        else {
            printf("Pitch: %f (OUTSIDE)\n", pitch);
            kill(getppid(), SIGUSR2);
        }
        nanosleep(&ts, NULL);
    }
    
    close(fd);
    return 0;
}

int main(){
    struct sigaction sa;
    sa.sa_handler = sigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGCHLD, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    pid_t pid;
    int i = 0;
    
    pid = fork(); //creating the child

    if (pid == -1){
        perror("forking error");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0)
    {
        childActions();
    }
    else { 
        printf("parent pid: %d\n", getpid());
        while (1){ //puts parent on loop so program can listen for signals
            pause();
        } 
    }
    
    return (EXIT_SUCCESS);
}
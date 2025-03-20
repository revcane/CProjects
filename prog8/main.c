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

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void childSigHndl(int sig){
    if (sig == SIGCHLD) {
        printf("got SIGCHLD\n");
    }
}

int main(int argc, char const *argv[]){
    pid_t pid;
    int status;
    int fd;

    struct sigaction sa;
    sa.sa_handler = childSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    checkError(sigaction(SIGCHLD, &sa, NULL), "sigaction SIGCHLD");

    pid = fork();

    checkError(pid, "error creating child");

    if(pid == 0){
        checkError(execlp("./myRand", "myRand", NULL), "error running exec on myRand"); //running myrand program on child
    }
    else {
        wait(&status);
        if (WIFEXITED(status)){
            int values[60]; 
            int sum;
            double avg;

            int filenum = WEXITSTATUS(status);
            char filename[20];
            //printf("program 1 got: %d\n", filenum);
            sprintf(filename, "data%d.dat", filenum);

            fd = checkError(open(filename, O_RDONLY, S_IRUSR), "error opening data.dat");
            checkError(read(fd, values, sizeof(int) * 60), "error reading data.dat");

            for(int i = 0; i < 60; i++) { //storing values in array and iterating them
                printf("number: %d\n", values[i]);
                //printf("number: %d | iteration %d\n", values[i],i);
                sum += values[i];
                //printf("sum: %d\n", sum);
            }

            avg = sum / 60.0;
            printf("average: %lf\n", avg);

            close(fd);

            checkError(unlink(filename), "error unlinking data.dat");

            printf("deleted: %s\n", filename);
        }
    }

    return 0;
}

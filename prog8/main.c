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

void ChildSigHndl(int sig) {
    if (sig == SIGCHLD) {
        printf("got SIGCHLD\n");
    }
}

int main(int argc, char const *argv[]){
    pid_t pid;
    int status;
    int fd;

    struct sigaction sa;
    sa.sa_handler = ChildSigHndl;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    checkError(sigaction(SIGCHILD, &sa, NULL), "sigaction SIGCHILD");

    pid = fork();

    checkError(pid, "error creating child");

    if(pid == 0) {
        checkError(execlp("./myRand", "myRand", NULL), "Failed to run exec on myRand");
    }
    else {
        wait(&status);
        if (WIFEXITED(status)) {
            int values[60];
            int sum;
            double avg;

            int filenum = WEXITSTATUS(status);
            printf("program 1 got: %d\n", filenum);

            char filename[20];
            sprintf(filename, "data%d.dat", filenum);

            fd = checkError(open(filename, O_RDONLY, S_IRUSR), "error opening data.dat");

            checkError(read(fd, values, sizeof(int) * 60), "error reading data.dat");

            for(int i = 0; i < 60; i++) {
                printf("Number from file: %d\n", values[i]);
                sum += values[i];
                printf("Sum: %d\n", sum);
            }

            avg = sum / 60.0;
            printf("Average is: %lf\n", avg);

            close(fd);

            checkError(unlink(filename), "error unlinking data.dat");

            printf("Deleted file: %s\n", filename);
        }
    }

    return 0;
}

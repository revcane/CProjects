#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

void sigHndl(int sig) {
    char response[10];

    if (sig == SIGINT) {
        write(STDOUT_FILENO, "\nDo you really want to exit? (y/n): ", 36);
        if (read(STDIN_FILENO, response, sizeof(response)) == -1) {
            perror("Error reading input");
            exit(EXIT_FAILURE);
        }
        if (response[0] == 'y' || response[0] == 'Y') {
            exit(EXIT_SUCCESS);
        }
    }
}


int main(){

}
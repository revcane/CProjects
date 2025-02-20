#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

#define BUFFERSIZE 128
#define BUFFERLIMIT 2048
volatile sig_atomic_t timerbool = 0; //

int checkError(int val, const char *msg){
    if (val == -1)
   {
       perror(msg);
       exit(EXIT_FAILURE);
   }
return val;
}

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
    if (sig == SIGALRM) {
        timerbool = 1;
        write(STDOUT_FILENO, "\nTime has elapsed!\n", 19);
    }
}

char *readline(int fd) {
    int buffer_size = BUFFERSIZE;
    int pos = 0;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    char c;
    int bytes_read;
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        if (c == '\n') {  //stop reaeding at newline
            buffer[pos] = '\0';
            return buffer;
        }
        buffer[pos++] = c;

        if (pos >= buffer_size - 1) {  //expand buffer if above limit 
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                perror("realloc failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (pos == 0) {  //end of file
        free(buffer);
        return NULL;
    }

    buffer[pos] = '\0';  //null terminate last line
    return buffer;
}

int main() {
   struct sigaction sa;
    sa.sa_handler = sigHndl;
    sa.sa_flags = SA_RESTART;  //restart syscalls if needed
    sigemptyset(&sa.sa_mask);
    
    checkError(sigaction(SIGALRM, &sa, NULL), "signal SIGALRM");
    checkError(sigaction(SIGINT, &sa, NULL), "signal SIGINT");

    int question_fd = checkError(open("quest.txt", O_RDONLY), "Error opening quest.txt");
    int answer_fd = checkError(open("ans.txt", O_RDONLY), "Error opening ans.txt");

    write(STDOUT_FILENO, "Press Ctrl+C to exit. You have 15 seconds per question.\n", 56);

    char *question, *answer;
    char answer_input[BUFFERSIZE];
    int correct = 0;

    while ((question = readline(question_fd)) != NULL && (answer = readline(answer_fd)) != NULL) {
        timerbool = 0; //reset bool
        write(STDOUT_FILENO, "\nQuestion: ", 11); //send question
        write(STDOUT_FILENO, question, strlen(question));
        write(STDOUT_FILENO, "\nYour answer: ", 14);

        struct itimerval timer;
        timer.it_value.tv_sec = 15;
        timer.it_value.tv_usec = 0;
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, &timer, NULL);  //start the timer

        int readInput = read(STDIN_FILENO, answer_input, BUFFERSIZE - 1);
        
        if (timerbool) { // If time elapsed, move to next question
            write(STDOUT_FILENO, "Skipping to next question...\n", 29);
            free(question); 
            free(answer);
            continue;
        }

        setitimer(ITIMER_REAL, &(struct itimerval){0}, NULL);  //disable timer after input

        if (readInput > 0 && answer_input[readInput - 1] == '\n') { //trim newline and or null termination
            answer_input[readInput - 1] = '\0';
        } else {
            answer_input[readInput] = '\0';
        }

        if (strcmp(answer_input, answer) == 0) { //comparision
            correct++;
        }

        free(question); //freeing memory
        free(answer);
    }

    char buffer[256];
    int results = snprintf(buffer, sizeof(buffer), "\nQuiz complete! You got %d question(s) correct.\n", correct);
    write(STDOUT_FILENO, buffer, results);

    close(question_fd);
    close(answer_fd);
    return 0;
}
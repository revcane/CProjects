#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

void child(int fd[2]){
    close(fd[0]);
    
    dup2(fd[1],STDOUT_FILENO); //dup the write side of the pipe onto standard output
    execlp("./rand", "rand", "-c", "50", "-m", "200", "-M", "500", NULL); //run "rand -c 50 -m 200 -M 500" using exec
    
    close(fd[1]);
    
    exit(EXIT_SUCCESS);
}

void parent(int fd[2]){
    int total = 0;
    int sum = 0;
    int average = 0;
    int value = 0;
    
    close(fd[1]); //close write side of pipe
    while(1){
        //read integer from pipe
        ssize_t bytesRead = read(fd[0], &value, sizeof(int));
        if (bytesRead == -1){
            perror("read failed");
            exit(EXIT_FAILURE);
        }
        if (bytesRead == 0){ 
            break;
        }

        char buffer[33];
        int len = snprintf(buffer, sizeof(buffer), "Value #%d: %d \n",total+1, value);
        write(STDOUT_FILENO, buffer, len); //print the value read to standard out
        
        sum += value; //add the value read to the sum
        total++; //increment the number of values read by 1
    }
    close(fd[0]);

    if (total > 0){
        average = (sum / total);
    } 
    else{
        average = 0;
    }    

    printf("The sum is: %d\n",sum); //print the total of the values read
    printf("The average is: %d\n",average); //print the average of the values read
    
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
    int fd[2];
    pid_t pid; 
   
    checkError(pipe(fd),"failed to create pipe\n"); //create a pipe
    
    pid = fork(); //call the fork
    
    checkError(pid, "failed to make fork\n");
   
    if(pid == 0){
        child(fd); //call function child
    }
    else{
        parent(fd); //call function parent
    }
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>



int checkError(int val, const char *msg){
    if (val == -1) {
        perror(msg);
        exit(EXIT_FAILURE);
    }
    return val;
}

int main(int argc, char const *argv[])
{
    char filename[20];
    int fd;
    int num = 0;
    int value;
    srand(time(NULL));

    num = rand() % 256;
    
    sprintf(filename, "data%d.dat", num); //formatting name of file
    
    fd = checkError(open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH), "error opening data.dat");
    
    for(int i = 0; i < 60; i++) { //generating the 60 random values
        value = rand() % 101;
        checkError(write(fd, &value, sizeof(value)), "error writing to data.dat");
    }

    close(fd);
    exit(num);
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>


#define BUF_SIZE 4096

struct timespec ts = {1, 0};

int checkError(int val, const char *msg){
     if (val == -1)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
return val;
}

int checkRange(double value1){
    return(value1 <= 20.0 && value1 >= -20.0);
}

int main(int argc, char *argv[]){
    int fd; //file descriptor

    double pitch; //values are stored as doubles in the angl.dat file
    double roll;
    double yaw;

    fd = checkError(open("angl.dat",O_RDONLY), "error in opening angl.dat\n");

    while (read(fd, &roll, sizeof(double)) == sizeof(double) && read(fd, &pitch, sizeof(double)) == sizeof(double) && read(fd, &yaw, sizeof(double)) == sizeof(double)) {
        if (checkRange(roll)) {
            printf("Roll: %f (INSIDE), ", roll);
        } 
        else {
            printf("Roll: %f (OUTSIDE), ", roll);
        }
        if (checkRange(pitch)) {
            printf("Pitch: %f (INSIDE)\n", pitch);
        } 
        else {
            printf("Pitch: %f (OUTSIDE)\n", pitch);
        }
    
    nanosleep(&ts, NULL);
    }

close(fd);
return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include "packetHelpers.h"

#define BUF_SIZE 2048 //1024 did not work well


int checkError(int val, const char *msg){
     if (val == -1)
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
return val;
}

int main(int argc, char *argv[]){
    int fd;
    int outputFd; 
    int i = 0;
    int numRead = 0;
    unsigned char buffer[BUF_SIZE];

    fd = checkError(open("raw.dat",O_RDONLY), "attempting to open raw file\n");
    outputFd = checkError(open("data.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR), "attempting to open/write output file\n");
   
    numRead = checkError(read(fd, buffer, BUF_SIZE), "reading raw file\n");
    if (numRead == -1) {
        perror("error reading raw file\n");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("bytes read: %d\n", numRead);

    for (int i = 0; i < numRead; i += 20)
    {
        if (i+20 <=numRead)
        {
            processPacket(&buffer[i],outputFd);
        }
        else
        {
           
            perror("error: stopped processing packet\n");
            //printf("reached buffer: %d/%d\n", i,BUF_SIZE);
        }
         
    } 
    
    close(fd);
    close(outputFd);
return 0;
}
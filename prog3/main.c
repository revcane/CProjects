
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#include "packetHelpers2.h"

#define BUF_SIZE 4096 //1024 did not work well


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

    int dir;
    int accelFd;
    int rotaFd;
    int anglFd;

    int i = 0;
    int numRead = 0;
    unsigned char buffer[BUF_SIZE];

    fd = checkError(open("data.dat",O_RDONLY), "error in attempting to open data file\n");

if (mkdir("values", 0755) == -1) {
    if (errno == EEXIST) {
        printf("values already exists.\n");
    } else {
        perror("error in creating directory values");
        exit(EXIT_FAILURE);
    }
}

    accelFd = checkError(open("values/accl.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR), "error inattempting to open/write accel file\n");
    rotaFd = checkError(open("values/rota.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR), "error in attempting to open/write rota file\n");
    anglFd = checkError(open("values/angl.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR), "error in attempting to open/write angl file\n");

    numRead = checkError(read(fd, buffer, BUF_SIZE), "error in reading data file\n");
    if (numRead == -1) {
        perror("error reading raw file\n");
        close(fd);
        close(accelFd);
        close(rotaFd);
        close(anglFd);
        return EXIT_FAILURE;
    }
    printf("bytes read: %d\n", numRead);

    for (int i = 0; i + 8 <= numRead; i += 8){
        processPacket(&buffer[i],accelFd,rotaFd,anglFd);
        printf("[DEBUG] Processing packet at index: %d\n", i); 
    }
    if (numRead % 8 != 0){
            perror("incomplete packet\n");
        //printf("reached buffer: %d/%d\n", i,BUF_SIZE);
    }

    close (fd);
    close(rotaFd);
    close(accelFd);
    close(anglFd);
    return 0;
}
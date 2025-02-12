
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

int checkError(int val, const char *msg){
if (val == -1)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
return val;
}

// void writeArr(int fd, int *out){
//     int numWr;
//     numWr = checkError(write(fd,out,sizeof(out)),write);
// }

int main(){
    int fd;
    int i;
    int numWr;
    int out[1000];
    

    fd = checkError( open("data.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR), "open"); 

    for (i = 1; i < 1001; i++)
    {
        out[i-1] = i;
        //numWr = checkError(write(fd, &i, sizeof(int)), "write");

    }
    numWr = checkError(write(fd, out, 1000*sizeof(int)), "write");
    close(fd);
    return 0;
}
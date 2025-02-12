
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

int main(){
    int fd;
    int i;
    int numRd;
    char out[6];

    fd = checkError( open("data.dat", O_RDONLY), "open"); 
    while ((numRd = checkError(read(fd,&i,sizeof(int)),"read")) > 0){
        //printf("%d ", i);
        //sprintf(out, "%d\n", i);
        write(STDOUT_FILENO, &i, sizeof(int));
    }
    printf("\n");

    close(fd);
    return 0;
}
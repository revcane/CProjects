/******************************************************************************

Welcome to GDB Online.
GDB online is an online compiler and debugger tool for C, C++, Python, Java, PHP, Ruby, Perl,
C#, OCaml, VB, Swift, Pascal, Fortran, Haskell, Objective-C, Assembly, HTML, CSS, JS, SQLite, Prolog.
Code, Compile, Run and Debug online from anywhere in world.

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
// checkError(-1,"This is a test");
int
checkError (int val, const char *msg)
{
  if (val == -1)
    {
      perror (msg);
      exit (EXIT_FAILURE);
    }
  return val;
}

/*
int main()
{
    int fd;
    int i = 0;
    int numWr = 0;
    int buf[1000];
    fd = checkError(open("data.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR), "failed to open for write");
    
    for (i = 1; i < 1000; i++)
    {
        buf[i-1] = i;
    }
    numWr = checkError(write(fd, buf, 999*sizeof(int)), "failed to write");
    
    close(fd);
    
    return 0;
}
*/

int main()
{
    int fd;
    int buf[1000];
    int i = 0;
    int numRd = 0;
    fd = checkError(open("data.bin", O_RDONLY), "failed to open for read");
    
    numRd = checkError(read(fd, buf, 1000*sizeof(int)), "failed to read");
    
    close(fd);
    
    for (int i = 0 ; i < numRd/sizeof(int); i++)
    {
        printf("%d  ",buf[i]);
    }
    printf("\n");
    return 0;
}

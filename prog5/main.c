#include <stdio.h>
#include <stdlib.h>

#include <time.h>
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

int main(int argc, char *argv[]){

}
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

// goal is to prompt the user for total number of widgets
// to enter .. each widget has a name, price, and amount.
// we will loop while entering widgets and save them in
// a widget array. Then we will open a file and output the array
// to the file.


#define BUF_SIZE 1024

struct widget
{
  char name[40];
  float price;
  unsigned int amount;
};


int handleError(int val, const char *str)
{
  if (val == -1)
    {
      perror(str);
      exit(EXIT_FAILURE);
    }
  return val;
}

void printCString(const char *str)
{
  int len = strlen(str);
  int numWrit;
  numWrit = handleError(write(STDOUT_FILENO, str, len * sizeof(char)),"printCString");
}

void readCString(char *str)
{
  int numRead = 0;
  numRead = handleError(read(STDIN_FILENO, str, (BUF_SIZE-1) * sizeof(char)), "readCString");
  str[numRead-1] = 0; // adds the null character to the end character buffer we just read
}

int main(int argc, char *argv[])
{
  char buffer[BUF_SIZE];
  int count = 0;
  int total = 0;
  int amt = 0;
  float price = 0.0;
  int fd, numWrit;
  struct widget *arr = NULL;


  printCString("Please enter the number of widgets to manage: ");
  readCString(buffer);

  if (sscanf(buffer,"%d",&count) <= 0)       // reads the first integer in the buffer
    handleError(-1, "sscanf");

  // allocate some memory

  arr = (struct widget *) calloc(count, sizeof(struct widget));
  if (arr == NULL) handleError(-1, "calloc");
  // loop for each value
  for (total = 0; total < count; total++)
    {
      printCString("Widget name? ");
      readCString(buffer);
      strncpy(arr[total].name, buffer,39);

      printCString("Price? ");
      readCString(buffer);
      if (sscanf(buffer, "%f", &price) <= 0) handleError(-1, "sscanf");
      arr[total].price = price;

      printCString("Amount? ");
      readCString(buffer);
      if (sscanf(buffer, "%d", &amt) <= 0) handleError(-1, "sscanf");
      arr[total].amount = amt;
    }

  // open a file
  fd = handleError(open("data.dat", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR), "open");
  // write to the file
  numWrit = handleError(write(fd, arr, count * sizeof(struct widget)), "write");
  // close the file
  close(fd);
  // free the memory

  free(arr);
  

  exit(EXIT_SUCCESS);

}

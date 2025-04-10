#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

int genRand(int low, int high)
{
  static int frst = 1;
  int rng, rnd;
  double scale;

  if (frst) { srand(time(NULL)); frst = 0;}
  
  rng = high - low + 1;
  scale = (rng+0.0)/INT_MAX;
  rnd = rand() * scale + low;

  return rnd;
}


int getVal(int argc, char *argv[], const char *flag, int *val)
{
  int i = 0;
  for (i = 1; i < argc; i++)
    {
      if (!strcmp(argv[i], flag))
	{
	  if ((i + 1) >= argc) return 0;
	  sscanf(argv[i+1], "%d",val);
	  return 1;
	}
    }
  return 0;
}


int main(int argc, char *argv[])
{

  char str[10];
  int cnt = 60;
  int min = 10;
  int max = 50;

  getVal(argc,argv,"-c",&cnt);
  getVal(argc,argv,"-m",&min);
  getVal(argc,argv,"-M",&max);
  
  for (int i = 0; i < cnt; i++)
    {
      int val = genRand(min,max);
      write(STDOUT_FILENO, &val,sizeof(int));
    }
  return 0;
}
  

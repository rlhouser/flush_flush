#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <stdio.h>
#include "../cacheutils.h"

const int ARRAY_SIZE = 5*1024;
const int CYLES_MAX = 600;
size_t array[ARRAY_SIZE];


size_t hit_histogram[CYLES_MAX];
size_t miss_histogram[CYLES_MAX];

size_t flushandreload(size_t* addr)
{
  size_t time = rdtsc();
  flush(addr); //flush the cacheline containing the given address
  size_t delta = rdtsc() - time; //get the time for the flush;
  maccess(addr); //read the address
  maccess(addr); //read the address again;
  return delta;
}

size_t flushandflush(size_t* addr)
{
  size_t time = rdtsc();
  flush(addr);
  size_t delta = rdtsc() - time;
  flush(addr);
  return delta;
}

int main(int argc, char** argv)
{
  memset(array,-1,ARRAY_SIZE*sizeof(size_t)); //initialize array with -1
  maccess(array + 2*1024); //defined in cacheutiles; writes to a quadword in the middle of the array;
  sched_yield(); //force thread to relinquish the processor and wait its turn https://pubs.opengroup.org/onlinepubs/9699919799/functions/sched_yield.html
  for (int i = 0; i < 1*1024*1024; ++i)
  {
    size_t d = flushandreload(array+2*1024);//get time to flush cacheline containing given address
    hit_histogram[MIN(CYLES_MAX - 1,d)]++;
    for (size_t i = 0; i < 30; ++i)
      sched_yield();
  }
  flush(array+2*1024); //flush target address before trying flush + flush
  for (int i = 0; i < 1*1024*1024; ++i)
  {
    size_t d = flushandflush(array+2*1024);
    miss_histogram[MIN(CYLES_MAX-1,d)]++;
    for (size_t i = 0; i < 30; ++i)
      sched_yield();
  }
  FILE *outfile;
  outfile = fopen ("histogram.csv", "w");
  for (size_t i = 0; i < 600; ++i)
  {
    fprintf(outfile, "%3zu\t%10zu\t%10zu\n",i,hit_histogram[i],miss_histogram[i]);
  }
  fclose(outfile);
  return 0;
}

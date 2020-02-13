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

size_t onlyreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  return delta;
}

size_t flushandreload(void* addr)
{
  size_t time = rdtsc();
  maccess(addr);
  size_t delta = rdtsc() - time;
  flush(addr);
  return delta;
}

int main(int argc, char** argv)
{
  memset(array,-1,ARRAY_SIZE*sizeof(size_t));
  maccess(array + 2*1024);
  sched_yield();
  for (int i = 0; i < 12*1024*1024; ++i)
  {
    size_t d = onlyreload(array+2*1024);
    hit_histogram[MIN(CYLES_MAX-1,d)]++;
  }
  flush(array+1024);
  for (int i = 0; i < 12*1024*1024; ++i)
  {
    size_t d = flushandreload(array+2*1024);
    miss_histogram[MIN(CYLES_MAX-1,d)]++;
  }
  printf(".\n");
  size_t hit_max = 0;
  size_t hit_max_i = 0;
  size_t miss_min_i = 0;
  FILE *outfile;
  outfile = fopen ("histogram.csv", "w");
  for (size_t i = 0; i < CYLES_MAX; ++i)
  {
    fprintf(outfile, "%3zu\t%10zu\t%10zu\n",i,hit_histogram[i],miss_histogram[i]);
    if (hit_max < hit_histogram[i])
    {
      hit_max = hit_histogram[i];
      hit_max_i = i;
    }
    if (miss_histogram[i] > 3 && miss_min_i == 0)
      miss_min_i = i;
  }
  fclose(outfile);
  if (miss_min_i > hit_max_i+4)
    printf("Flush+Reload possible!\n");
  else if (miss_min_i > hit_max_i+2)
    printf("Flush+Reload probably possible!\n");
  else if (miss_min_i < hit_max_i+2)
    printf("Flush+Reload maybe not possible!\n");
  else
    printf("Flush+Reload not possible!\n");
  size_t min = -1UL;
  size_t min_i = 0;
  for (int i = hit_max_i; i < miss_min_i; ++i)
  {
    if (min > (hit_histogram[i] + miss_histogram[i]))
    {
      min = hit_histogram[i] + miss_histogram[i];
      min_i = i;
    }
  }
  printf("The lower the threshold, the lower the number of false positives.\n");
  printf("Suggested cache hit/miss threshold: %zu\n",min_i);
  return min_i * 5;
}

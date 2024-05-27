#include <assert.h>
#include <stdlib.h>

// gives integer between [min, max] where both are +ve values
// works only for max <= RAND_MAX
unsigned int rand_range(unsigned int min, unsigned max) {
  assert(min <= max);
  unsigned int range = max + 1 - min;
  return min + (int)(((double)rand() / RAND_MAX) * range);
}

void shuffle(unsigned int *arr, int length) {
  for (unsigned int i = 0; i < length; i++) {
    arr[i] = i;
  }
  for (int i = length - 1; i > -1; i--) {
    int replacement = rand_range(0, i);
    unsigned int temp = arr[replacement];
    arr[replacement] = arr[i];
    arr[i] = temp;
  }

  return;
}

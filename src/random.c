#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "random.h"
#include "debug.h"



FILE* u_random;


void random_init() {
  u_random = fopen("/dev/urandom", "r");
  if (!u_random) {
    error("Cannot open /dev/urandom");
  }
}

int random_int() {
  int r;
  fread(&r, sizeof(int), 1, u_random);
  return r;
}

int random_int_range(int from, int to) {
  int r = random_int();
  int m = r % (to - from + 1);
  return abs(m) + from;
}

double random_zero_to_one() {
  int precision = 100000;
  int r = random_int_range(0, precision);
  return (double)r / precision;
}

int random_nof_elems(double two, double three, double four) {
  double r = random_zero_to_one();
  three = two + three;
  if (r <= two)
    return 2;
  else if (r <= three)
    return 3;
  else return 4;
}

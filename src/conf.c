#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include "conf.h"

double TWO;
double THREE;
double FOUR;
int NOF_INVITES;
int MAX_REJECT;
int POP_SIZE;
int SIM_TIME;

void config_init() {
  FILE* opt = fopen("opt.conf","r");
  if (!opt) {
    error("Cannot read from opt.conf file.");
  }
  fscanf(opt, "%*s %lf %*s %lf %*s %lf %*s %d %*s %d %*s %d %*s %d", &TWO, &THREE, &FOUR, &NOF_INVITES, &MAX_REJECT, &POP_SIZE, &SIM_TIME);
  fclose(opt);
}

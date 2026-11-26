#ifndef MEASURES_H
#define MEASURES_H

#include "register.h"
#include <stdio.h>

typedef struct Measure {
  int id;
  char *date;
  float *measures[24]; // H01, H02, ..., H24
} Measure;

void read_measures(FILE *dataset, Register *registres, int total_registres);

#endif // MEASURES_H

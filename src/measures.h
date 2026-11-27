#ifndef MEASURES_H
#define MEASURES_H

#include "register.h"
#include <stdio.h>

typedef struct Measure {
  int id;
  char *date;
  double *measures[24]; // H01, H02, ..., H24 (NULL = mesura absent)
} Measure;

/**
 * Processa el fitxer de mesures i actualitza els registres.
 * Retorna 0 si tot va bé, o -1 en cas d'error (ja informat per stderr).
 * No tanca el fitxer ni allibera els registres: ho fa qui els ha creat.
 */
int read_measures(FILE *dataset, Register *registres, int total_registres);

#endif // MEASURES_H

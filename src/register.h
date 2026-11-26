#ifndef REGISTER_H
#define REGISTER_H

#include <stdio.h>

typedef struct Register {
  int id;
  char codi_eoi[32];
  int magnitud;
  char unidad[32];
  float min, max;
  int hora_min, hora_max;
  char *fecha_min, *fecha_max;
  int presents, ausents;
} Register;

void Register_init(Register *p);
Register *read_catalogo(FILE *catalogo, int *total_registres);
void print_results(const Register *registres, int total_registres);
void free_registers(Register *registres, int total_registres);

#endif // REGISTER_H

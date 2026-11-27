#ifndef REGISTER_H
#define REGISTER_H

#include <stdio.h>

#define DATE_LEN 11 // "AAAA-MM-DD" + '\0'

typedef struct Register {
  int id;
  char codi_eoi[32];
  int magnitud;
  char unidad[32];
  double min, max;
  int hora_min, hora_max;
  char fecha_min[DATE_LEN], fecha_max[DATE_LEN]; // Dates dins l'estructura: cap reserva dinàmica
  int presents, ausents;
} Register;

void Register_init(Register *p);

/**
 * Llegeix el catàleg. Comprova que els IDs siguin únics i consecutius (0..K-1).
 * Retorna el vector de registres, o NULL en cas d'error (ja informat per stderr).
 * No tanca el fitxer: ho fa qui l'ha obert.
 */
Register *read_catalogo(FILE *catalogo, int *total_registres);
void print_results(const Register *registres, int total_registres);
void free_registers(Register *registres);

#endif // REGISTER_H

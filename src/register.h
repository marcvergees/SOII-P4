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

/**
 * Inicialitza un registre amb valors per defecte: ID -1, cadenes buides
 * i comptadors de mesures presents i absents a 0.
 */
void Register_init(Register *p);

/**
 * Llegeix el catàleg. Comprova que els IDs siguin únics i consecutius (0..K-1).
 * Retorna el vector de registres, o NULL en cas d'error (ja informat per stderr).
 * No tanca el fitxer: ho fa qui l'ha obert.
 */
Register *read_catalogo(FILE *catalogo, int *total_registres);

/**
 * Imprimeix per stdout la taula de resultats en format TSV: la capçalera i
 * una fila per cada ID amb almenys una fila de mesures, ordenada per ID.
 * MIN i MAX amb "%.3f" i hores amb dues xifres (01 a 24). Si un ID només
 * té mesures absents, els extrems, les dates i les hores es mostren com a NA.
 */
void print_results(const Register *registres, int total_registres);

/**
 * Allibera el vector de registres creat per read_catalogo.
 */
void free_registers(Register *registres);

#endif // REGISTER_H

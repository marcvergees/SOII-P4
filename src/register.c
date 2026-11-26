#include "register.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Register_init(Register *p) {
  p->id = -1;
  p->codi_eoi[0] = '\0';
  p->magnitud = 0;
  p->unidad[0] = '\0';
  p->min = 0.0f;
  p->max = 0.0f;
  p->fecha_min = NULL;
  p->hora_min = 0;
  p->fecha_max = NULL;
  p->hora_max = 0;
  p->presents = 0;
  p->ausents = 0;
}

Register *read_catalogo(FILE *catalogo, int *total_registres) {
  char line[MAX_LINE];

  // Saltem la capçalera
  if (fgets(line, sizeof(line), catalogo) == NULL) {
    fprintf(stderr, "Error a la línia 1: fitxer de catàleg buit\n");
    fclose(catalogo);
    exit(EXIT_FAILURE);
  }
  size_t hlen = strlen(line);
  if (hlen == sizeof(line) - 1 && line[hlen - 1] != '\n') {
    fprintf(stderr, "Error a la línia 1: línia massa llarga (supera 511 bytes)\n");
    fclose(catalogo);
    exit(EXIT_FAILURE);
  }

  int capacity = 128;
  int num_registres = 0;
  Register *registres = malloc(capacity * sizeof(Register));
  if (!registres) {
    perror("Error assignant memòria per al catàleg");
    exit(EXIT_FAILURE);
  }

  int num_linea = 1; // La capçalera és la línia 1
  while (fgets(line, sizeof(line), catalogo) != NULL) {
    num_linea++;
    size_t len = strlen(line);
    if (len == sizeof(line) - 1 && line[len - 1] != '\n') {
      fprintf(stderr, "Error a la línia %d: línia massa llarga (supera 511 bytes)\n", num_linea);
      free_registers(registres, num_registres);
      fclose(catalogo);
      exit(EXIT_FAILURE);
    }
    line[strcspn(line, "\r\n")] = '\0';

    if (num_registres >= capacity) {
      capacity *= 2;
      Register *tmp = realloc(registres, capacity * sizeof(Register));
      if (!tmp) {
        free_registers(registres, num_registres);
        perror("Error reassignant memòria per al catàleg");
        exit(EXIT_FAILURE);
      }
      registres = tmp;
    }

    Register *reg = &registres[num_registres];
    Register_init(reg);

    int count = 0;
    char *start = line;
    char *comma;

    while ((comma = strchr(start, ',')) != NULL) {
      *comma = '\0'; // Tallem la cadena abans de guardar-la o fer atoi()
      if (count < LIMIT_LINE_CATALOGO) {
        if (count == 0) {
          char *endptr;
          long id_val = strtol(start, &endptr, 10);
          if (endptr == start || *endptr != '\0' || id_val < 0) {
            fprintf(stderr, "Error a la línia %d: ID invàlid al catàleg '%s'\n", num_linea, start);
            free_registers(registres, num_registres);
            fclose(catalogo);
            exit(EXIT_FAILURE);
          }
          reg->id = (int)id_val;
        } else if (count == 1) {
          strncpy(reg->codi_eoi, start, sizeof(reg->codi_eoi) - 1);
          reg->codi_eoi[sizeof(reg->codi_eoi) - 1] = '\0';
        } else if (count == 3) {
          char *endptr;
          long mag_val = strtol(start, &endptr, 10);
          if (endptr == start || *endptr != '\0') {
            fprintf(stderr, "Error a la línia %d: magnitud invàlida al catàleg '%s'\n", num_linea, start);
            free_registers(registres, num_registres);
            fclose(catalogo);
            exit(EXIT_FAILURE);
          }
          reg->magnitud = (int)mag_val;
        }
      }
      count++;
      start = comma + 1; // Avancem al següent caràcter
    }

    // Processem l'últim camp (després de l'última coma: UNIDAD, count == 5)
    if (count < LIMIT_LINE_CATALOGO && count == 5) {
      strncpy(reg->unidad, start, sizeof(reg->unidad) - 1);
      reg->unidad[sizeof(reg->unidad) - 1] = '\0';
    }
    count++;

    // Validació que exigeix l'enunciat per a la versió seqüencial:
    if (count != LIMIT_LINE_CATALOGO) {
      fprintf(stderr, "Error a la línia %d: s'esperaven %d camps, trobats %d\n",
              num_linea, LIMIT_LINE_CATALOGO, count);
      free_registers(registres, num_registres);
      fclose(catalogo);
      exit(EXIT_FAILURE);
    }

    num_registres++;
  }

  *total_registres = num_registres;
  return registres;
}

void print_results(const Register *registres, int total_registres) {
  fprintf(
      stdout,
      "ID\tCODI_EOI\tMAGNITUD\tUNIDAD\tMIN\tFECHA_MIN\tHORA_MIN\tMAX\tFECHA_"
      "MAX\tHORA_MAX\tPRESENTES\tAUSENTES\n");
  for (int i = 0; i < total_registres; i++) {
    const Register *reg = &registres[i];
    // Només imprimim els IDs amb almenys una fila de mesures
    if (reg->presents + reg->ausents > 0) {
      if (reg->presents > 0) {
        fprintf(stdout,
                "%d\t%s\t%d\t%s\t%.3f\t%s\t%02d\t%.3f\t%s\t%02d\t%d\t%d\n",
                reg->id, reg->codi_eoi, reg->magnitud, reg->unidad, reg->min,
                reg->fecha_min, reg->hora_min, reg->max, reg->fecha_max,
                reg->hora_max, reg->presents, reg->ausents);
      } else {
        fprintf(stdout, "%d\t%s\t%d\t%s\tNA\tNA\tNA\tNA\tNA\tNA\t%d\t%d\n",
                reg->id, reg->codi_eoi, reg->magnitud, reg->unidad,
                reg->presents, reg->ausents);
      }
    }
  }
}

void free_registers(Register *registres, int total_registres) {
  if (!registres)
    return;
  for (int i = 0; i < total_registres; i++) {
    if (registres[i].fecha_min)
      free(registres[i].fecha_min);
    if (registres[i].fecha_max)
      free(registres[i].fecha_max);
  }
  free(registres);
}

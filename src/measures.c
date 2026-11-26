#include "measures.h"
#include "utils.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_measures(FILE *dataset, Register *registres, int total_registres) {
  char line[MAX_LINE];

  // Saltem la capçalera
  if (fgets(line, sizeof(line), dataset) == NULL) {
    fprintf(stderr, "Error a la línia 1: fitxer de mesures buit\n");
    fclose(dataset);
    exit(EXIT_FAILURE);
  }
  size_t hlen = strlen(line);
  if (hlen == sizeof(line) - 1 && line[hlen - 1] != '\n') {
    fprintf(stderr, "Error a la línia 1: línia massa llarga (supera 511 bytes)\n");
    fclose(dataset);
    exit(EXIT_FAILURE);
  }

  int num_linea = 1; // La capçalera és la línia 1
  while (fgets(line, sizeof(line), dataset) != NULL) {
    num_linea++;
    size_t len = strlen(line);
    if (len == sizeof(line) - 1 && line[len - 1] != '\n') {
      fprintf(stderr, "Error a la línia %d: línia massa llarga (supera 511 bytes)\n", num_linea);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    // Eliminem el salt de línia final (\n o \r\n)
    line[strcspn(line, "\r\n")] = '\0';

    struct Measure measure;
    measure.id = -1;
    measure.date = NULL;
    float values[24];
    for (int i = 0; i < 24; i++) {
      measure.measures[i] = NULL;
    }

    int count = 0;
    char *start = line;
    char *comma;

    while ((comma = strchr(start, ',')) != NULL) {
      *comma = '\0'; // Tallem la cadena abans de guardar-la o parsejar-la
      if (count < LIMIT_LINE_DATASET) {
        if (count == 0) {
          char *endptr;
          long id_val = strtol(start, &endptr, 10);
          if (endptr == start || *endptr != '\0') {
            fprintf(stderr, "Error a la línia %d: ID no enter '%s'\n", num_linea, start);
            fclose(dataset);
            exit(EXIT_FAILURE);
          }
          if (id_val < 0 || id_val >= total_registres) {
            fprintf(stderr, "Error a la línia %d: ID %ld fora de rang del catàleg\n",
                    num_linea, id_val);
            fclose(dataset);
            exit(EXIT_FAILURE);
          }
          measure.id = (int)id_val;
        } else if (count == 1) {
          measure.date = start;
        } else {
          int idx = count - 2;
          if (start[0] != '\0') {
            char *endptr;
            errno = 0;
            float val = strtof(start, &endptr);
            if (endptr == start || *endptr != '\0' || !isfinite(val) || errno == ERANGE) {
              fprintf(stderr, "Error a la línia %d: valor no vàlid '%s'\n", num_linea, start);
              fclose(dataset);
              exit(EXIT_FAILURE);
            }
            values[idx] = val;
            measure.measures[idx] = &values[idx];
          } else {
            measure.measures[idx] = NULL;
          }
        }
      }
      count++;
      start = comma + 1; // Avancem al següent caràcter
    }

    // Processem l'últim camp (després de l'última coma: H24, count == 25)
    if (count < LIMIT_LINE_DATASET && count >= 2) {
      int idx = count - 2;
      if (start[0] != '\0') {
        char *endptr;
        errno = 0;
        float val = strtof(start, &endptr);
        if (endptr == start || *endptr != '\0' || !isfinite(val) || errno == ERANGE) {
          fprintf(stderr, "Error a la línia %d: valor no vàlid '%s'\n", num_linea, start);
          fclose(dataset);
          exit(EXIT_FAILURE);
        }
        values[idx] = val;
        measure.measures[idx] = &values[idx];
      } else {
        measure.measures[idx] = NULL;
      }
    }
    count++;

    // Validació que exigeix l'enunciat per a la versió seqüencial:
    if (count != LIMIT_LINE_DATASET) {
      fprintf(stderr, "Error a la línia %d: s'esperaven %d camps, trobats %d\n",
              num_linea, LIMIT_LINE_DATASET, count);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    if (!is_valid_date(measure.date)) {
      fprintf(stderr,
              "Error a la línia %d: format de data incorrecte '%s' (s'esperava AAAA-MM-DD)\n",
              num_linea, measure.date);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    // Actualitzem les estadístiques del registre corresponent
    Register *reg = &registres[measure.id];

    for (int i = 0; i < 24; i++) {
      if (measure.measures[i] != NULL) {
        float val = *measure.measures[i];
        int hora = i + 1; // Hores de 1 a 24

        if (reg->presents == 0) {
          // Primera mesura present per a aquest ID
          reg->min = val;
          reg->fecha_min = strdup(measure.date);
          reg->hora_min = hora;

          reg->max = val;
          reg->fecha_max = strdup(measure.date);
          reg->hora_max = hora;
        } else {
          // Actualització del MÍNIM:
          // Menor valor, o en empat data més antiga o hora menor
          int cmp_min = strcmp(measure.date, reg->fecha_min);
          if (val < reg->min ||
              (val == reg->min &&
               (cmp_min < 0 || (cmp_min == 0 && hora < reg->hora_min)))) {
            reg->min = val;
            if (reg->fecha_min)
              free(reg->fecha_min);
            reg->fecha_min = strdup(measure.date);
            reg->hora_min = hora;
          }

          // Actualització del MÀXIM:
          // Major valor, o en empat data més antiga o hora menor
          int cmp_max = strcmp(measure.date, reg->fecha_max);
          if (val > reg->max ||
              (val == reg->max &&
               (cmp_max < 0 || (cmp_max == 0 && hora < reg->hora_max)))) {
            reg->max = val;
            if (reg->fecha_max)
              free(reg->fecha_max);
            reg->fecha_max = strdup(measure.date);
            reg->hora_max = hora;
          }
        }
        reg->presents++;
      } else {
        reg->ausents++;
      }
    }
  }
}

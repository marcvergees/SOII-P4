#include "measures.h"
#include "utils.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void read_measures(FILE *dataset, Register *registres, int total_registres) {
  char line[MAX_LINE];

  if (fgets(line, sizeof(line), dataset) == NULL) { // Descarta capçalera o detecta fitxer buit
    fprintf(stderr, "Error a la línia 1: fitxer de mesures buit\n");
    fclose(dataset);
    exit(EXIT_FAILURE);
  }
  size_t hlen = strlen(line);
  if (hlen == sizeof(line) - 1 && line[hlen - 1] != '\n') { // Valida límit de 511 bytes a la capçalera
    fprintf(stderr, "Error a la línia 1: línia massa llarga (supera 511 bytes)\n");
    fclose(dataset);
    exit(EXIT_FAILURE);
  }

  int num_linea = 1; // Comptador de línies (la capçalera és la línia 1)
  while (fgets(line, sizeof(line), dataset) != NULL) {
    num_linea++;
    size_t len = strlen(line);
    if (len == sizeof(line) - 1 && line[len - 1] != '\n') { // Comprova que la línia no superi 511 bytes
      fprintf(stderr, "Error a la línia %d: línia massa llarga (supera 511 bytes)\n", num_linea);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    line[strcspn(line, "\r\n")] = '\0'; // Trunca caràcters de salt de línia

    struct Measure measure;
    measure.id = -1;
    measure.date = NULL;
    float values[24];
    for (int i = 0; i < 24; i++) {
      measure.measures[i] = NULL; // Inicialitza hores com a absents
    }

    int count = 0;
    char *start = line;
    char *comma;

    while ((comma = strchr(start, ',')) != NULL) { // Separa camps delimitats per coma
      *comma = '\0'; // Marca final del camp actual
      if (count < LIMIT_LINE_DATASET) {
        if (count == 0) { // Camp 0: ID
          char *endptr;
          long id_val = strtol(start, &endptr, 10); // Conversió estricta a enter
          if (endptr == start || *endptr != '\0') {
            fprintf(stderr, "Error a la línia %d: ID no enter '%s'\n", num_linea, start);
            fclose(dataset);
            exit(EXIT_FAILURE);
          }
          if (id_val < 0 || id_val >= total_registres) { // Valida rang de l'ID respecte al catàleg
            fprintf(stderr, "Error a la línia %d: ID %ld fora de rang del catàleg\n",
                    num_linea, id_val);
            fclose(dataset);
            exit(EXIT_FAILURE);
          }
          measure.id = (int)id_val;
        } else if (count == 1) { // Camp 1: Data (AAAA-MM-DD)
          measure.date = start;
        } else { // Camps 2 a 24: Hores H01 a H23
          int idx = count - 2;
          if (start[0] != '\0') {
            char *endptr;
            errno = 0;
            float val = strtof(start, &endptr); // Conversió estricta a float finit
            if (endptr == start || *endptr != '\0' || !isfinite(val) || errno == ERANGE) {
              fprintf(stderr, "Error a la línia %d: valor no vàlid '%s'\n", num_linea, start);
              fclose(dataset);
              exit(EXIT_FAILURE);
            }
            values[idx] = val;
            measure.measures[idx] = &values[idx]; // Apunta al valor numèric present
          } else {
            measure.measures[idx] = NULL; // Camp buit: mesura absent
          }
        }
      }
      count++;
      start = comma + 1; // Avança al següent token
    }

    if (count < LIMIT_LINE_DATASET && count >= 2) { // Processa l'últim camp (H24)
      int idx = count - 2;
      if (start[0] != '\0') {
        char *endptr;
        errno = 0;
        float val = strtof(start, &endptr); // Conversió estricta a float finit
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

    if (count != LIMIT_LINE_DATASET) { // Valida que la fila tingui exactament 26 camps
      fprintf(stderr, "Error a la línia %d: s'esperaven %d camps, trobats %d\n",
              num_linea, LIMIT_LINE_DATASET, count);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    if (!is_valid_date(measure.date)) { // Valida calendari i format AAAA-MM-DD
      fprintf(stderr,
              "Error a la línia %d: format de data incorrecte '%s' (s'esperava AAAA-MM-DD)\n",
              num_linea, measure.date);
      fclose(dataset);
      exit(EXIT_FAILURE);
    }

    Register *reg = &registres[measure.id]; // Accés directe al registre corresponent

    for (int i = 0; i < 24; i++) {
      if (measure.measures[i] != NULL) {
        float val = *measure.measures[i];
        int hora = i + 1; // Hora de 1 a 24

        if (reg->presents == 0) { // Inicialitza amb la primera mesura vàlida
          reg->min = val;
          reg->fecha_min = strdup(measure.date);
          reg->hora_min = hora;

          reg->max = val;
          reg->fecha_max = strdup(measure.date);
          reg->hora_max = hora;
        } else {
          int cmp_min = strcmp(measure.date, reg->fecha_min); // Desempat temporal de dates
          if (val < reg->min ||
              (val == reg->min &&
               (cmp_min < 0 || (cmp_min == 0 && hora < reg->hora_min)))) { // Menor valor, data antiga o hora menor
            reg->min = val;
            if (reg->fecha_min)
              free(reg->fecha_min);
            reg->fecha_min = strdup(measure.date);
            reg->hora_min = hora;
          }

          int cmp_max = strcmp(measure.date, reg->fecha_max); // Desempat temporal de dates
          if (val > reg->max ||
              (val == reg->max &&
               (cmp_max < 0 || (cmp_max == 0 && hora < reg->hora_max)))) { // Major valor, data antiga o hora menor
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

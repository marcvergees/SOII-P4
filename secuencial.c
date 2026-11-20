#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512
#define LIMIT_LINE_DATASET 26
#define LIMIT_LINE_CATALOGO 6

typedef struct Measure {
  int id;
  char *date;
  float *measures[24]; // H01, H02, ..., H24
} Measure;

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

void read_measures(FILE *dataset, Register *registres, int total_registres) {
  char line[MAX_LINE];

  // Saltem la capçalera
  if (fgets(line, sizeof(line), dataset) == NULL) {
    fclose(dataset);
    exit(EXIT_FAILURE);
  }

  int num_linea = 1; // La capçalera és la línia 1
  while (fgets(line, sizeof(line), dataset) != NULL) {
    // Eliminem el salt de línia final (\n o \r\n)
    line[strcspn(line, "\r\n")] = '\0';
    num_linea++;

    struct Measure measure;
    float values[24];
    for (int i = 0; i < 24; i++) {
      measure.measures[i] = NULL;
    }

    int count = 0;
    char *start = line;
    char *comma;

    while ((comma = strchr(start, ',')) != NULL) {
      *comma = '\0'; // Tallem la cadena abans de guardar-la o fer atoi()
      if (count < LIMIT_LINE_DATASET) {
        if (count == 0)
          measure.id = atoi(start);
        else if (count == 1)
          measure.date = start;
        else {
          int idx = count - 2;
          if (start[0] != '\0') {
            values[idx] = strtof(start, NULL);
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
        values[idx] = strtof(start, NULL);
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
      exit(EXIT_FAILURE);
    }

    if (measure.id < 0 || measure.id >= total_registres) {
      fprintf(stderr, "Error a la línia %d: ID %d fora de rang del catàleg\n",
              num_linea, measure.id);
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

Register *read_catalogo(FILE *catalogo, int *total_registres) {
  char line[MAX_LINE];

  // Saltem la capçalera
  if (fgets(line, sizeof(line), catalogo) == NULL) {
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
    line[strcspn(line, "\r\n")] = '\0';
    num_linea++;

    if (num_registres >= capacity) {
      capacity *= 2;
      Register *tmp = realloc(registres, capacity * sizeof(Register));
      if (!tmp) {
        free(registres);
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
        if (count == 0)
          reg->id = atoi(start);
        else if (count == 1) {
          strncpy(reg->codi_eoi, start, sizeof(reg->codi_eoi) - 1);
          reg->codi_eoi[sizeof(reg->codi_eoi) - 1] = '\0';
        } else if (count == 3)
          reg->magnitud = atoi(start);
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
      free(registres);
      exit(EXIT_FAILURE);
    }

    num_registres++;
  }

  *total_registres = num_registres;
  return registres;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Ús: %s <catalogo> <dataset>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *catalogo = fopen(argv[1], "r");
  FILE *dataset = fopen(argv[2], "r");
  if (!catalogo || !dataset) {
    perror("Error obrint el fitxer");
    exit(EXIT_FAILURE);
  }

  int total_registres = 0;
  Register *registres = read_catalogo(catalogo, &total_registres);

  read_measures(dataset, registres, total_registres);

  print_results(registres, total_registres);

  for (int i = 0; i < total_registres; i++) {
    if (registres[i].fecha_min)
      free(registres[i].fecha_min);
    if (registres[i].fecha_max)
      free(registres[i].fecha_max);
  }
  free(registres);

  fclose(catalogo);
  fclose(dataset);
  return 0;
}
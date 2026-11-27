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
  p->min = 0.0;
  p->max = 0.0;
  p->fecha_min[0] = '\0';
  p->hora_min = 0;
  p->fecha_max[0] = '\0';
  p->hora_max = 0;
  p->presents = 0;
  p->ausents = 0;
}

Register *read_catalogo(FILE *catalogo, int *total_registres) {
  char line[MAX_LINE];

  if (fgets(line, sizeof(line), catalogo) == NULL) { // Descarta capçalera o detecta fitxer buit
    fprintf(stderr, "Error a la línia 1: fitxer de catàleg buit\n");
    return NULL;
  }
  if (line_too_long(catalogo, line, sizeof(line))) { // Valida límit de 511 bytes a la capçalera
    fprintf(stderr, "Error a la línia 1: línia massa llarga (supera 511 bytes)\n");
    return NULL;
  }

  int capacity = 128; // Capacitat inicial del vector dinàmic
  int num_registres = 0;
  Register *registres = malloc(capacity * sizeof(Register)); // Reserva memòria per als registres
  if (!registres) {
    perror("Error assignant memòria per al catàleg");
    return NULL;
  }

  int num_linea = 1; // Comptador de línies (la capçalera és la línia 1)
  while (fgets(line, sizeof(line), catalogo) != NULL) {
    num_linea++;
    if (line_too_long(catalogo, line, sizeof(line))) { // Comprova que la línia no superi 511 bytes
      fprintf(stderr, "Error a la línia %d: línia massa llarga (supera 511 bytes)\n", num_linea);
      free_registers(registres);
      return NULL;
    }
    line[strcspn(line, "\r\n")] = '\0'; // Trunca caràcters de final de línia

    if (num_registres >= capacity) { // Duplica la capacitat si s'omple el buffer
      capacity *= 2;
      Register *tmp = realloc(registres, capacity * sizeof(Register));
      if (!tmp) {
        perror("Error reassignant memòria per al catàleg");
        free_registers(registres);
        return NULL;
      }
      registres = tmp;
    }

    Register *reg = &registres[num_registres];
    Register_init(reg); // Inicialitza camps a valors per defecte

    int count = 0;
    char *start = line;
    char *comma;

    while ((comma = strchr(start, ',')) != NULL) { // Separa camps per coma
      *comma = '\0'; // Marca final del camp actual
      if (count < LIMIT_LINE_CATALOGO) {
        if (count == 0) { // Camp 0: ID
          char *endptr;
          long id_val = strtol(start, &endptr, 10); // Conversió estricta a enter
          if (!is_plain_number(start) || endptr == start || *endptr != '\0' || id_val < 0) {
            fprintf(stderr, "Error a la línia %d: ID invàlid al catàleg '%s'\n", num_linea, start);
            free_registers(registres);
            return NULL;
          }
          // Els IDs han de ser únics i consecutius des de 0: la fila i-èsima ha de tenir ID i.
          // Això garanteix que registres[id] és el registre amb aquell ID.
          if (id_val != num_registres) {
            fprintf(stderr,
                    "Error a la línia %d: ID %ld no consecutiu o duplicat al catàleg (s'esperava %d)\n",
                    num_linea, id_val, num_registres);
            free_registers(registres);
            return NULL;
          }
          reg->id = (int)id_val;
        } else if (count == 1) { // Camp 1: CODI_EOI
          strncpy(reg->codi_eoi, start, sizeof(reg->codi_eoi) - 1);
          reg->codi_eoi[sizeof(reg->codi_eoi) - 1] = '\0';
        } else if (count == 3) { // Camp 3: MAGNITUD
          char *endptr;
          long mag_val = strtol(start, &endptr, 10); // Conversió estricta de la magnitud
          if (endptr == start || *endptr != '\0') {
            fprintf(stderr, "Error a la línia %d: magnitud invàlida al catàleg '%s'\n", num_linea, start);
            free_registers(registres);
            return NULL;
          }
          reg->magnitud = (int)mag_val;
        }
      }
      count++;
      start = comma + 1; // Avança al següent token
    }

    if (count < LIMIT_LINE_CATALOGO && count == 5) { // Processa l'últim camp (UNIDAD)
      strncpy(reg->unidad, start, sizeof(reg->unidad) - 1);
      reg->unidad[sizeof(reg->unidad) - 1] = '\0';
    }
    count++;

    if (count != LIMIT_LINE_CATALOGO) { // Valida que la fila tingui exactament 6 camps
      fprintf(stderr, "Error a la línia %d: s'esperaven %d camps, trobats %d\n",
              num_linea, LIMIT_LINE_CATALOGO, count);
      free_registers(registres);
      return NULL;
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
      "MAX\tHORA_MAX\tPRESENTES\tAUSENTES\n"); // Imprimeix capçalera TSV
  for (int i = 0; i < total_registres; i++) {
    const Register *reg = &registres[i];
    if (reg->presents + reg->ausents > 0) { // Omet estacions sense mesures
      if (reg->presents > 0) { // Si hi ha mesures vàlides, imprimeix estadístiques
        fprintf(stdout,
                "%d\t%s\t%d\t%s\t%.3f\t%s\t%02d\t%.3f\t%s\t%02d\t%d\t%d\n",
                reg->id, reg->codi_eoi, reg->magnitud, reg->unidad, reg->min,
                reg->fecha_min, reg->hora_min, reg->max, reg->fecha_max,
                reg->hora_max, reg->presents, reg->ausents);
      } else { // Si totes són absents, imprimeix NA
        fprintf(stdout, "%d\t%s\t%d\t%s\tNA\tNA\tNA\tNA\tNA\tNA\t%d\t%d\n",
                reg->id, reg->codi_eoi, reg->magnitud, reg->unidad,
                reg->presents, reg->ausents);
      }
    }
  }
}

void free_registers(Register *registres) {
  free(registres); // Les dates són dins l'estructura: només cal alliberar el vector
}

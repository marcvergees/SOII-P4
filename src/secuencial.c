#include "measures.h"
#include "register.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Versió seqüencial. Ús: ./secuencial <catalogo> <dataset>
 * Carrega el catàleg, recorre el fitxer de mesures una sola vegada i
 * imprimeix la taula de resultats per stdout. Els errors s'informen per
 * stderr i el programa acaba amb EXIT_FAILURE, alliberant abans els recursos.
 */
int main(int argc, char **argv) {
  if (argc != 3) { // Valida exactament 2 arguments (catàleg i mesures)
    fprintf(stderr, "Ús: %s <catalogo> <dataset>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *catalogo = fopen(argv[1], "r"); // Obre fitxer de catàleg
  if (!catalogo) {
    fprintf(stderr, "Error obrint el catàleg '%s': ", argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  int total_registres = 0;
  Register *registres = read_catalogo(catalogo, &total_registres); // Llegeix el catàleg d'estacions
  fclose(catalogo); // El catàleg ja no es necessita: es tanca tant si hi ha error com si no
  if (!registres) {
    exit(EXIT_FAILURE);
  }

  FILE *dataset = fopen(argv[2], "r"); // Obre fitxer de mesures
  if (!dataset) {
    fprintf(stderr, "Error obrint el fitxer de mesures '%s': ", argv[2]);
    perror(NULL);
    free_registers(registres);
    exit(EXIT_FAILURE);
  }

  // Processa les mesures i extreu extrems
  int status = read_measures(dataset, registres, total_registres);
  fclose(dataset); // Tanca fitxer de mesures

  if (status == 0) {
    print_results(registres, total_registres); // Imprimeix el resum TSV per stdout (només si no hi ha error)
  }

  free_registers(registres); // Allibera la memòria dinàmica utilitzada
  return status == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

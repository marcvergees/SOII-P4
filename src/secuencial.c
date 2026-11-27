#include "measures.h"
#include "register.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 3) { // Valida exactament 2 arguments (catàleg i mesures)
    fprintf(stderr, "Ús: %s <catalogo> <dataset>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *catalogo = fopen(argv[1], "r"); // Obre fitxer de catàleg
  FILE *dataset = fopen(argv[2], "r");  // Obre fitxer de mesures
  if (!catalogo || !dataset) {          // Comprova errors en obrir els fitxers
    perror("Error obrint el fitxer");
    exit(EXIT_FAILURE);
  }

  int total_registres = 0;
  Register *registres = read_catalogo(catalogo, &total_registres); // Llegeix el catàleg d'estacions

  read_measures(dataset, registres, total_registres); // Processa les mesures i extreu extrems

  print_results(registres, total_registres); // Imprimeix el resum TSV per stdout

  free_registers(registres, total_registres); // Allibera la memòria dinàmica utilitzada

  fclose(catalogo); // Tanca fitxer de catàleg
  fclose(dataset);  // Tanca fitxer de mesures
  return 0;
}

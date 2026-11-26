#include "measures.h"
#include "register.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 3) {
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

  free_registers(registres, total_registres);

  fclose(catalogo);
  fclose(dataset);
  return 0;
}

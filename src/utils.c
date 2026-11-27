#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_valid_date(const char *date) {
  if (date == NULL || strlen(date) != 10) return 0; // Comprova longitud de 10 caràcters

  if (date[4] != '-' || date[7] != '-') return 0; // Comprova separadors de guió (AAAA-MM-DD)

  for (int i = 0; i < 10; i++) {
    if (i == 4 || i == 7) continue;
    if (!isdigit((unsigned char)date[i])) return 0; // Comprova que la resta siguin dígits
  }

  int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 +
             (date[2] - '0') * 10 + (date[3] - '0'); // Extreu l'any numèric
  int month = (date[5] - '0') * 10 + (date[6] - '0'); // Extreu el mes numèric
  int day = (date[8] - '0') * 10 + (date[9] - '0');   // Extreu el dia numèric

  if (year < 1000 || year > 9999) return 0; // Any de 4 xifres vàlid
  if (month < 1 || month > 12) return 0;    // Mes entre 1 i 12
  if (day < 1 || day > 31) return 0;        // Dia positiu fins a 31

  static const int days_in_month[] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (day > days_in_month[month]) return 0; // Valida màxim de dies segons el mes
  if (month == 2 && day == 29) {
    int is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0); // Comprova any de traspàs
    if (!is_leap) return 0;
  }

  return 1;
}

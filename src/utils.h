#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define MAX_LINE 512
#define LIMIT_LINE_DATASET 26
#define LIMIT_LINE_CATALOGO 6

/**
 * Valida si una cadena de data té el format AAAA-MM-DD
 * i representa una data real de calendari.
 * Retorna 1 si és vàlida, 0 en cas contrari.
 */
int is_valid_date(const char *date);

/**
 * Indica si la línia que acaba de llegir fgets supera els 511 bytes.
 * Si el buffer s'ha omplert sense '\n', mira el següent caràcter: si és EOF,
 * és l'última línia (de 511 bytes justos sense salt de línia) i és vàlida.
 * Retorna 1 si la línia és massa llarga, 0 si no.
 */
int line_too_long(FILE *f, const char *line, size_t size);

/**
 * Indica si un camp té format numèric "net": comença per dígit o per '-'
 * seguit de dígit, i no és hexadecimal. Rebutja espais inicials, '+',
 * camps buits, "nan", "inf" i "0x...". Retorna 1 si és vàlid, 0 si no.
 */
int is_plain_number(const char *s);

#endif // UTILS_H

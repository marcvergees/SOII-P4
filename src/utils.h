#ifndef UTILS_H
#define UTILS_H

#define MAX_LINE 512
#define LIMIT_LINE_DATASET 26
#define LIMIT_LINE_CATALOGO 6

/**
 * Valida si una cadena de data té el format AAAA-MM-DD
 * i representa una data real de calendari.
 * Retorna 1 si és vàlida, 0 en cas contrari.
 */
int is_valid_date(const char *date);

#endif // UTILS_H

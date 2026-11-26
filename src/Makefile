CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11
PTHREAD = -pthread

# Binaris a generar
# Per a la primera entrega tenim secuencial. Si més endavant s'afegeixen paralelo.c o prodcons.c, s'inclouen automàticament.
TARGETS = secuencial

ifneq ($(wildcard paralelo.c),)
    TARGETS += paralelo
endif

ifneq ($(wildcard prodcons.c),)
    TARGETS += prodcons
endif

.PHONY: all debug clean run

# Objectiu principal (all): compila amb -Wall -Wextra -O2 -std=gnu11
all: $(TARGETS)

secuencial: secuencial.o
	$(CC) $(CFLAGS) -o $@ $^

paralelo: paralelo.o
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^

prodcons: prodcons.o
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Objectiu debug: afegeix -g per a l'anàlisi amb Valgrind/GDB i recompila
debug: CFLAGS += -g
debug: clean all

# Executa la versió seqüencial amb el catàleg i el joc de dades reduït
run: secuencial
	./secuencial catalogo.csv XVPCA_small.csv

# Neteja de binaris i fitxers objecte
clean:
	rm -f secuencial paralelo prodcons *.o callgrind.out*

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11
PTHREAD = -pthread

# Mòduls comuns
COMMON_OBJS = register.o measures.o utils.o

# Binaris a generar
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

secuencial: secuencial.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

paralelo: paralelo.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^ -lm

prodcons: prodcons.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) $(PTHREAD) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependències de capçaleres
secuencial.o: secuencial.c register.h measures.h utils.h
register.o: register.c register.h utils.h
measures.o: measures.c measures.h register.h utils.h
utils.o: utils.c utils.h

# Objectiu debug: afegeix -g per a l'anàlisi amb Valgrind/GDB i recompila
debug: CFLAGS += -g
debug: clean all

# Execució ràpida de prova del seqüencial amb el joc de dades reduït
run: secuencial
	./secuencial catalogo.csv XVPCA_small.csv

# Neteja de binaris i fitxers objecte
clean:
	rm -f *.o $(TARGETS)

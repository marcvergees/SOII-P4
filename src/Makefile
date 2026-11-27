CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=gnu11

# Mòduls auxiliars
OBJS = secuencial.o register.o measures.o utils.o

.PHONY: all debug clean run

# Objectiu principal (all): compila amb -Wall -Wextra -O2 -std=gnu11
all: secuencial

secuencial: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dependències de capçaleres
secuencial.o: secuencial.c register.h measures.h utils.h
register.o: register.c register.h utils.h
measures.o: measures.c measures.h register.h utils.h
utils.o: utils.c utils.h

# Objectiu debug: neteja i recompila amb les mateixes opcions + -g (Valgrind/GDB).
# Es fa en dos passos seqüencials perquè "make -j" no executi clean i all alhora.
debug:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS) -g"

# Execució ràpida de prova del seqüencial amb el joc de dades reduït
run: secuencial
	./secuencial catalogo.csv XVPCA_small.csv

# Neteja de binaris i fitxers objecte
clean:
	rm -f *.o secuencial

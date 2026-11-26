#!/usr/bin/env bash
# Pruebas de la versión secuencial.
#
#   ./run_tests.sh [binario]      (por defecto ../secuencial)
#
# Cada caso de casos.tsv indica qué catálogo y qué fichero de medidas usar y
# qué se espera:
#   OK  -> el programa acaba con exit 0 y la salida es igual a esperado/<caso>.tsv
#   N   -> el programa acaba con error, no escribe nada por stdout y el mensaje
#          de stderr indica la línea N (la cabecera es la línea 1)

cd "$(dirname "$0")" || exit 1
BIN=${1:-../secuencial}
OUT=$(mktemp)
ERR=$(mktemp)
INV=$(mktemp)
trap 'rm -f "$OUT" "$ERR" "$INV"' EXIT
ok=0
mal=0

fallo() {
    echo "FALLO $1: $2"
    mal=$((mal + 1))
}

# Comprueba que, para cada ID de la salida, PRESENTES + AUSENTES = 24 x filas de ese ID
suma_correcta() {   # suma_correcta fichero_medidas fichero_salida
    awk 'NR == FNR { if (FNR > 1) filas[$1]++; next }
         FNR > 1   { if ($11 + $12 != 24 * filas[$1]) mal = 1 }
         END       { exit mal }' FS=',' "$1" FS='\t' "$2"
}

# Ejecuta el caso con las filas de datos en orden inverso (la cabecera se queda
# la primera) y comprueba que la salida no cambia: los empates no deben
# depender del orden en que se procesan las filas.
orden_indiferente() {   # orden_indiferente catalogo medidas salida_esperada
    awk 'NR == 1 { print; next } { l[++n] = $0 } END { for (i = n; i > 0; i--) print l[i] }' \
        "$2" > "$INV"
    "$BIN" "$1" "$INV" 2> /dev/null | diff -q "$3" - > /dev/null
}

while IFS=$'\t' read -r nombre cat med esperado desc; do
    [ "$nombre" = NOMBRE ] && continue

    "$BIN" "$cat" "$med" > "$OUT" 2> "$ERR"
    rc=$?

    if [ "$esperado" = OK ]; then
        if [ $rc -ne 0 ]; then
            fallo "$nombre" "exit $rc ($(head -n 1 "$ERR")) — $desc"
        elif ! diff -q "esperado/$nombre.tsv" "$OUT" > /dev/null; then
            fallo "$nombre" "salida distinta de esperado/$nombre.tsv — $desc"
        elif ! suma_correcta "$med" "$OUT"; then
            fallo "$nombre" "PRESENTES + AUSENTES no es 24 x filas — $desc"
        elif ! orden_indiferente "$cat" "$med" "esperado/$nombre.tsv"; then
            fallo "$nombre" "la salida cambia al invertir el orden de las filas — $desc"
        else
            ok=$((ok + 1))
        fi
    else
        # Se quita el nombre del fichero del mensaje antes de buscar el número de línea
        msg=$(sed -e "s#$cat##g" -e "s#$med##g" "$ERR")
        if [ $rc -eq 0 ]; then
            fallo "$nombre" "no ha dado error — $desc"
        elif [ -s "$OUT" ]; then
            fallo "$nombre" "ha escrito en stdout aunque ha fallado — $desc"
        elif ! echo "$msg" | grep -Eq "(^|[^0-9])$esperado([^0-9]|$)"; then
            fallo "$nombre" "el error no indica la línea $esperado: $(head -n 1 "$ERR")"
        else
            ok=$((ok + 1))
        fi
    fi
done < casos.tsv

# Fichero pequeño proporcionado (si está la carpeta de datos)
if [ -f ../data/XVPCA_small.csv ] && [ -f ../data/catalogo.csv ]; then
    "$BIN" ../data/catalogo.csv ../data/XVPCA_small.csv > "$OUT" 2> "$ERR"
    if [ $? -ne 0 ]; then
        fallo "XVPCA_small" "exit distinto de 0: $(head -n 1 "$ERR")"
    elif ! suma_correcta ../data/XVPCA_small.csv "$OUT"; then
        fallo "XVPCA_small" "PRESENTES + AUSENTES no es 24 x filas"
    else
        ok=$((ok + 1))
    fi

    # Memcheck (solo si Valgrind está instalado, p. ej. en la MV)
    if command -v valgrind > /dev/null; then
        if valgrind -q --leak-check=full --error-exitcode=99 \
               "$BIN" ../data/catalogo.csv ../data/XVPCA_small.csv > /dev/null 2> "$ERR"; then
            ok=$((ok + 1))
        else
            fallo "memcheck" "Valgrind encuentra errores o fugas: $(grep -m 1 '==' "$ERR")"
        fi
    else
        echo "(Valgrind no está instalado: se omite Memcheck)"
    fi
fi

# Errores de argumentos y de apertura de ficheros
for args in "" "catalogo_mini.csv" "catalogo_mini.csv medidas_mini.csv sobra" \
            "no_existe.csv medidas_mini.csv" "catalogo_mini.csv no_existe.csv"; do
    "$BIN" $args > "$OUT" 2> "$ERR"
    if [ $? -eq 0 ] || [ -s "$OUT" ] || [ ! -s "$ERR" ]; then
        fallo "argumentos" "'$args' debería dar error solo por stderr"
    else
        ok=$((ok + 1))
    fi
done

echo "Correctos: $ok   Fallos: $mal"
[ $mal -eq 0 ]

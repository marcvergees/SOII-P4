# Pràctica 4: Projecte de Programació Concurrent (Sistemes Operatius 2)
## Versió Seqüencial (`secuencial`)

Aquest projecte implementa la versió seqüencial per a l'anàlisi de mesures de qualitat de l'aire a les estacions de mesurament de Catalunya, a partir del catàleg d'estacions/magnituds (`catalogo.csv`) i els fitxers de mesures (`XVPCA.csv` / `XVPCA_small.csv`).

---

### 1. Estructura Modular del Codi

El codi està dissenyat de manera modular dividit en capçaleres (`.h`) i fitxers d'implementació (`.c`):

* **`secuencial.c`**: Punt d'entrada del programa (`main`). S'encarrega de validar els arguments de la línia de comandes, orquestrar la lectura del catàleg i les mesures, invocar la impressió dels resultats i alliberar tota la memòria dinàmica abans de finalitzar.
* **`register.h` / `register.c`**: Gestiona l'estructura `Register`, la càrrega dinàmica del catàleg a memòria (`read_catalogo`), la impressió dels resultats en format TSV segons l'especificació (`print_results`) i l'alliberament de la memòria (`free_registers`).
* **`measures.h` / `measures.c`**: Gestiona l'estructura `Measure` i la lectura línia a línia del fitxer de mesures (`read_measures`). Calcula de forma acumulativa els mínims, màxims i desempats (per data més antiga o hora menor) i el recompte de mesures presents i absents per a cada ID.
* **`utils.h` / `utils.c`**: Constants del sistema (`MAX_LINE`, límits de camps per CSV) i funcions d'utilitat i validació de dades malformades, com ara la comprovació del format estricte de dates `AAAA-MM-DD` (`is_valid_date`).

---

### 2. Instruccions de Compilació

El projecte disposa d'un `Makefile`:

```bash
# Compilació estàndard optimitzada (-Wall -Wextra -O2 -std=gnu11)
make
# o bé
make all

# Recompilació amb símbols de depuració (-g) per a Valgrind, Callgrind o GDB
make debug

# Neteja de binaris generats i fitxers objecte (*.o)
make clean
```

---

### 3. Instruccions d'Execució

El binari rep dos paràmetres obligatoris: el fitxer del catàleg i el fitxer de mesures.

```bash
./secuencial <catalogo.csv> <dataset.csv>
```

#### Exemples d'execució:

1. **Joc de proves petit (`XVPCA_small.csv`)**:
   ```bash
   ./secuencial catalogo.csv XVPCA_small.csv
   ```
   *(També es pot executar directament amb `make run`).*

2. **Joc de dades complet (`XVPCA.csv`) redirigint a fitxer**:
   ```bash
   ./secuencial catalogo.csv XVPCA.csv > resultat.tsv
   ```

---

### 4. Proves

El directori `tests/` conté fitxers mínims amb un catàleg reduït (`catalogo_mini.csv`) i un catàleg només amb capçalera (`catalogo_vacio.csv`, K = 0):

* **`validos/`**: casos correctes (una mesura, zeros, negatius, empats, hores absents, ID sense mesures, només capçalera...).
* **`errores/`**: fitxers malformats (`m_*` de mesures, `c_*` de catàleg). El programa ha d'indicar la línia de l'error per `stderr` i acabar amb `EXIT_FAILURE`.

```bash
./secuencial tests/catalogo_mini.csv tests/validos/v_ceros.csv 
./secuencial tests/errores/c_id_duplicado.csv tests/medidas_mini.csv; echo $?
```

---

### 5. Format de Sortida

La sortida s'emet per la sortida estàndard (`stdout`) en format **TSV** (valors separats per tabuladors), ordenada per `ID`, mostrant exclusivament els IDs presents al fitxer de mesures:

```text
ID	CODI_EOI	MAGNITUD	UNIDAD	MIN	FECHA_MIN	HORA_MIN	MAX	FECHA_MAX	HORA_MAX	PRESENTES	AUSENTES
```

* Els valors `MIN` i `MAX` s'imprimeixen amb precisió `%.3f`.
* Les hores s'imprimeixen amb etiqueta de dues xifres (`%02d`, de 01 a 24).
* Si un ID només disposa de mesures absents, els camps extrems i temporals es representen amb `NA`.

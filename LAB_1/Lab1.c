#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Definición de la estructura básica para representar cada celda
typedef struct {
    int fila;
    int col;
    double valor;
} ElementoEsparso;

// Paso 1: Generación Aleatoria Eficiente en Texto[cite: 1]
void generar_matriz_txt(const char *filename, int filas, int cols) {
    FILE *archivo = fopen(filename, "w");
    if (archivo == NULL){
        printf("Error al abrir el archivo.\n");
        return;
    } 

    // Se especifican las dimensiones en la primera línea
    fprintf(archivo, "%d %d\n", filas, cols);
    
    // Cálculo del 5% de elementos no nulos
    int total_elementos = (int)((filas * cols) * 0.05);

    for (int i = 0; i < total_elementos; i++) {
        int fila = rand() % filas;
        int col = rand() % cols;
        double valor = ((rand() % 1000) / 10.0) + 1.0;
        fprintf(archivo, "%d %d %.2f\n", fila, col, valor);
    }
    fclose(archivo);
}

// Paso 2: Lectura y Conversión a Binario Intermedio
void convertir_txt_a_bin(const char *txt_filename, const char *bin_filename) {
    FILE *ft = fopen(txt_filename, "r");
    FILE *fb = fopen(bin_filename, "wb");
    if (ft ==NULL || fb == NULL) {
        printf("Error al leer el archivo.\n");
        return;
    }

    int filas, cols;
    fscanf(ft, "%d %d", &filas, &cols);

    ElementoEsparso e;
    // Se lee fila por fila y se escribe inmediatamente en binario compacto
    while (fscanf(ft, "%d %d %lf", &e.fila, &e.col, &e.valor) == 3) {
        fwrite(&e, sizeof(ElementoEsparso), 1, fb);
    }

    fclose(ft);
    fclose(fb);
}

// Paso 3: Multiplicación Gestionada por Archivos Binarios
void multiplicar_matrices_bin(const char *bin_a, const char *bin_b, const char *bin_acum) {
    FILE *fa = fopen(bin_a, "rb");
    FILE *fb = fopen(bin_b, "rb");
    FILE *facum = fopen(bin_acum, "wb");
    if (!fa || !fb || !facum) return;

    ElementoEsparso ea, eb, e_temp;
    // Se lee un elemento A y se realiza un barrido completo de la matriz B
    while (fread(&ea, sizeof(ElementoEsparso), 1, fa)) {
        rewind(fb);
        while (fread(&eb, sizeof(ElementoEsparso), 1, fb)) {
            // Se busca coincidencia donde la fila B == col A
            if (eb.fila == ea.col) {
                e_temp.fila = ea.fila;
                e_temp.col = eb.col;
                e_temp.valor = ea.valor * eb.valor;
                // El resultado parcial se escribe en el búfer temporal
                fwrite(&e_temp, sizeof(ElementoEsparso), 1, facum);
            }
        }
    }

    fclose(fa);
    fclose(fb);
    fclose(facum);
}

// Función auxiliar de comparación para ordenar el binario en RAM
int comparar_elementos(const void *a, const void *b) {
    ElementoEsparso *ea = (ElementoEsparso *)a;
    ElementoEsparso *eb = (ElementoEsparso *)b;
    if (ea->fila != eb->fila) return ea->fila - eb->fila;
    return ea->col - eb->col;
}

// Paso 4: Consolidación Binaria y Escritura del Resultado Final
void consolidar_y_escribir_txt(const char *bin_acum, const char *txt_final, int filas_final, int cols_final) {
    FILE *facum = fopen(bin_acum, "rb");
    if (!facum) return;

    fseek(facum, 0, SEEK_END);
    long tamano_archivo = ftell(facum);
    int num_elementos = tamano_archivo / sizeof(ElementoEsparso);
    rewind(facum);

    ElementoEsparso *buffer = (ElementoEsparso *)malloc(tamano_archivo);
    fread(buffer, sizeof(ElementoEsparso), num_elementos, facum);
    fclose(facum);

    // Ordenamiento rápido en RAM para juntar celdas repetidas
    qsort(buffer, num_elementos, sizeof(ElementoEsparso), comparar_elementos);

    FILE *ft = fopen(txt_final, "w");
    fprintf(ft, "%d %d\n", filas_final, cols_final); // Dimensiones finales en la primera línea

    if (num_elementos > 0) {
        ElementoEsparso actual = buffer[0];
        for (int i = 1; i < num_elementos; i++) {
            if (buffer[i].fila == actual.fila && buffer[i].col == actual.col) {
                actual.valor += buffer[i].valor;
            } else {
                // Escritura de celdas consolidadas usando formato legible
                fprintf(ft, "%d %d %.2f\n", actual.fila, actual.col, actual.valor);
                actual = buffer[i];
            }
        }
        fprintf(ft, "%d %d %.2f\n", actual.fila, actual.col, actual.valor);
    }

    fclose(ft);
    free(buffer);
}

int main() {
    srand(time(NULL));
    int M = 100, N = 100, P = 100; // Modificar a 1000 según límite requerido

    generar_matriz_txt("matriz_a.txt", M, N);
    generar_matriz_txt("matriz_b.txt", N, P);

    convertir_txt_a_bin("matriz_a.txt", "matriz_a.bin");
    convertir_txt_a_bin("matriz_b.txt", "matriz_b.bin");

    multiplicar_matrices_bin("matriz_a.bin", "matriz_b.bin", "acumulado.bin");

    consolidar_y_escribir_txt("acumulado.bin", "matriz_axb.txt", M, P);

    return 0;
}
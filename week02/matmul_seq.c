/* Sequential dense matrix multiplication C = A * B  (the baseline for the
 * whole semester).  usage: ./matmul_seq [n]
 * Inputs are small integers, so every sum is exact and the checksum is
 * identical for all variants (sequential, OpenMP, tiled, ...). */
#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

int main(int argc, char **argv) {
    int n = argc > 1 ? atoi(argv[1]) : 512;
    size_t N2 = (size_t)n * n;
    double *A = malloc(N2 * sizeof *A), *B = malloc(N2 * sizeof *B), *C = malloc(N2 * sizeof *C);
    if (!A || !B || !C) { fprintf(stderr, "out of memory\n"); return 1; }
    for (size_t i = 0; i < N2; i++) { A[i] = (double)(i % 7 + 1); B[i] = (double)(i % 5 + 1); }

    double t0 = wtime();
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            double s = 0.0;
            for (int k = 0; k < n; k++) s += A[(size_t)i * n + k] * B[(size_t)k * n + j];
            C[(size_t)i * n + j] = s;
        }
    double t = wtime() - t0;

    double chk = 0.0;
    for (size_t i = 0; i < N2; i++) chk += C[i];
    printf("n=%d checksum=%.6e\n", n, chk);
    printf("time_s=%.6f\n", t);
    free(A); free(B); free(C);
    return 0;
}

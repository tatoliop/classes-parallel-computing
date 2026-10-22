/* A program with three phases, used to practise profiling.
 * Which function is the hotspot?  What does Amdahl say about the best
 * possible speedup if ONLY that function is parallelised?
 *
 *   make profile_me_pg && ./profile_me_pg && gprof ./profile_me_pg gmon.out | head -20
 *   perf record ./profile_me && perf report          (Linux, if perf works)
 */
#include <stdio.h>
#include <stdlib.h>

#define NOINLINE __attribute__((noinline))

NOINLINE static void generate(double *a, long n) {          /* phase 1 */
    unsigned long long s = 88172645463325252ULL;
    for (long i = 0; i < n; i++) {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        a[i] = (double)(s % 1000) / 1000.0;
    }
}

NOINLINE static void smooth(double *a, double *b, long n, int iters) {   /* phase 2 */
    for (int it = 0; it < iters; it++) {
        for (long i = 1; i < n - 1; i++) b[i] = (a[i - 1] + a[i] + a[i + 1]) / 3.0;
        b[0] = a[0]; b[n - 1] = a[n - 1];
        double *t = a; a = b; b = t;
    }
}

NOINLINE static double checksum(const double *a, long n) {   /* phase 3 */
    double s = 0;
    for (long i = 0; i < n; i++) s += a[i];
    return s;
}

int main(int argc, char **argv) {
    long n = argc > 1 ? atol(argv[1]) : 2000000;
    int iters = argc > 2 ? atoi(argv[2]) : 100;
    double *a = malloc(n * sizeof *a), *b = malloc(n * sizeof *b);
    generate(a, n);
    smooth(a, b, n, iters);
    printf("checksum=%.6f\n", checksum(iters % 2 ? b : a, n));
    free(a); free(b);
    return 0;
}

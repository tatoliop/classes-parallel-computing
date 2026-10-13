/* Keeps ONE core 100% busy for N seconds (default 10).
 * Start several copies and watch them spread across cores in htop:
 *     ./burn 20 & ./burn 20 & ./burn 20 & ./burn 20 &
 * Use `jobs` / `kill %1` (or `pkill burn`) to stop them. */
#include <stdio.h>
#include <stdlib.h>
#include "timing.h"

int main(int argc, char **argv) {
    double secs = argc > 1 ? atof(argv[1]) : 10.0;
    volatile double x = 1.0;
    double t0 = wtime();
    while (wtime() - t0 < secs) x = x * 1.0000001 + 1e-9;
    printf("done after %.1f s (x=%g)\n", wtime() - t0, x);
    return 0;
}

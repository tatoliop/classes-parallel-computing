/* Βήμα 5 – Ένθετοι βρόχοι: collapse και ordered (Μάργαρης, Κεφ. 4, σλ. 115–119)
 *
 * Έννοιες: collapse(n) σε τέλεια ένθετους ορθογώνιους βρόχους, ordered για
 * τμήμα που πρέπει να εκτελεστεί με τη σειρά των επαναλήψεων.
 *
 * Ωριαίοι μέσοι όροι: για κάθε αισθητήρα και κάθε ώρα (24 ώρες).
 * Ο εξωτερικός βρόχος έχει S επαναλήψεις, ο εσωτερικός 24· με collapse(2)
 * μοιράζονται S*24 επαναλήψεις. Μετά τυπώνουμε μια αναφορά για τους πρώτους
 * αισθητήρες: υπολογίζεται παράλληλα αλλά τυπώνεται με τη σωστή σειρά.
 *   OMP_NUM_THREADS=4 ./step05_collapse_ordered -s 8
 */
#include "sensors.h"
#define HOURS 24

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    int per_hour = c.T / HOURS;
    float *data = malloc(N * sizeof *data);
    double *hourly = malloc((size_t)c.S * HOURS * sizeof *hourly);
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++) data[(size_t)s * c.T + t] = reading(&c, s, t);

    /* (α) collapse(2): ένας ενιαίος χώρος S*24 επαναλήψεων */
    double t0 = omp_get_wtime();
    #pragma omp parallel for collapse(2)
    for (int s = 0; s < c.S; s++)
        for (int h = 0; h < HOURS; h++) {          /* τίποτα ανάμεσα στα δύο for */
            double sum = 0;
            for (int t = h * per_hour; t < (h + 1) * per_hour; t++) sum += data[(size_t)s * c.T + t];
            hourly[(size_t)s * HOURS + h] = sum / per_hour;
        }
    printf("Ωριαίοι μέσοι όροι (%d × %d): %.4f s\n", c.S, HOURS, omp_get_wtime() - t0);

    /* (β) ordered: η αναφορά τυπώνεται με τη σειρά των αισθητήρων */
    int report = c.S < 10 ? c.S : 10;
    #pragma omp parallel for ordered schedule(static, 1)
    for (int s = 0; s < report; s++) {
        double hot = -1e9; int hot_h = 0;                 /* παράλληλο τμήμα */
        for (int h = 0; h < HOURS; h++)
            if (hourly[(size_t)s * HOURS + h] > hot) { hot = hourly[(size_t)s * HOURS + h]; hot_h = h; }
        #pragma omp ordered                               /* σειριακό τμήμα, με τη σειρά του s */
        printf("  αισθητήρας %3d: θερμότερη ώρα %02d:00 (%.2f °C) [νήμα %d]\n", s, hot_h, hot, omp_get_thread_num());
    }
    /* Δοκιμάστε να σβήσετε τη γραμμή "#pragma omp ordered": τι αλλάζει στην έξοδο; */
    free(data); free(hourly);
    return 0;
}

/* Βήμα 3 – Καταμερισμός βρόχων με omp for (Μάργαρης, Κεφ. 4, σλ. 101–103)
 *
 * Έννοιες: #pragma omp for μέσα σε parallel και ο συνδυασμός parallel for,
 * τι γίνεται ΧΩΡΙΣ το for (κάθε νήμα εκτελεί όλο τον βρόχο), κανονική μορφή
 * βρόχου, το implicit barrier, nowait, lastprivate.
 *
 * Τώρα και η δημιουργία των δεδομένων γίνεται παράλληλα.
 *   OMP_NUM_THREADS=4 ./step03_for
 */
#include "sensors.h"

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    float *data = malloc(N * sizeof *data);
    double *mean = malloc(c.S * sizeof *mean);
    float *peak = malloc(c.S * sizeof *peak);

    /* (α) Χωρίς "for": όλα τα νήματα εκτελούν ΟΛΕΣ τις επαναλήψεις */
    int count = 0;
    #pragma omp parallel
    for (int s = 0; s < 4; s++) {
        #pragma omp atomic
        count++;
    }
    printf("(α) 4 επαναλήψεις χωρίς for: εκτελέστηκαν %d φορές συνολικά\n", count);

    /* (β) Παράλληλη δημιουργία δεδομένων: κάθε μέτρηση είναι ανεξάρτητη */
    double t0 = omp_get_wtime();
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++)
            data[(size_t)s * c.T + t] = reading(&c, s, t);
    printf("(β) δημιουργία δεδομένων: %.3f s\n", omp_get_wtime() - t0);

    /* (γ) Δύο ανεξάρτητοι βρόχοι στην ίδια περιοχή: ο πρώτος με nowait,
     *     αφού ο δεύτερος δεν διαβάζει τίποτα από όσα γράφει ο πρώτος. */
    int last_sensor = -1;
    t0 = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp for nowait
        for (int s = 0; s < c.S; s++) {
            double sum = 0;
            for (int t = 0; t < c.T; t++) sum += data[(size_t)s * c.T + t];
            mean[s] = sum / c.T;
        }
        #pragma omp for lastprivate(last_sensor)    /* τιμή της ΤΕΛΕΥΤΑΙΑΣ επανάληψης */
        for (int s = 0; s < c.S; s++) {
            float m = data[(size_t)s * c.T];
            for (int t = 1; t < c.T; t++) if (data[(size_t)s * c.T + t] > m) m = data[(size_t)s * c.T + t];
            peak[s] = m;
            last_sensor = s;
        }
    }   /* implicit barrier: εδώ όλοι οι πίνακες είναι πλήρεις */
    printf("(γ) μέση τιμή και μέγιστο: %.3f s · lastprivate: last_sensor = %d\n",
           omp_get_wtime() - t0, last_sensor);

    /* (δ) Βρόχος που ΔΕΝ είναι σε κανονική μορφή (το πλήθος επαναλήψεων δεν είναι
     *     γνωστό εκ των προτέρων) δεν παραλληλοποιείται με for:
     *       int s = 0; while (mean[s] < 20) s++;
     *     Πρέπει πρώτα να ξαναγραφτεί ως for με γνωστά όρια. */

    int bad = 0;
    for (int s = 0; s < c.S; s++) bad += fabs(mean[s] - sensor_mean(&c, data, s)) > 1e-9;
    printf("Αισθητήρας 0: μέση %.3f °C, μέγιστη %.2f °C · διαφορές από τη σειριακή: %d\n",
           mean[0], peak[0], bad);
    free(data); free(mean); free(peak);
    return 0;
}

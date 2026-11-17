/* Βήμα 1 – Η ομάδα νημάτων (Μάργαρης, Κεφ. 4, σλ. 79–91)
 *
 * Έννοιες: #pragma omp parallel, fork/join, πλήθος νημάτων με 3 τρόπους
 * (OMP_NUM_THREADS, omp_set_num_threads, όρος num_threads), αναγνωριστικό
 * νήματος, «χειροκίνητη» ανάθεση έργου με βάση το id, ο όρος if.
 *
 * Υπολογίζουμε τη μέση θερμοκρασία κάθε αισθητήρα, πρώτα σειριακά και μετά
 * παράλληλα, και συγκρίνουμε αποτελέσματα και χρόνους. Κάθε νήμα παίρνει ένα
 * συνεχές κομμάτι αισθητήρων που το υπολογίζει μόνο του από το id του.
 *   OMP_NUM_THREADS=4 ./step01_team
 */
#include "sensors.h"

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    float *data = make_data(&c);
    double *mean = malloc(c.S * sizeof *mean);

    /* (α) Ποιοι είμαστε; */
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int n  = omp_get_num_threads();
        printf("  νήμα %d από %d%s\n", id, n, id == 0 ? "  (ηγετικό νήμα)" : "");
    }

    /* (β) Οι τρεις τρόποι ορισμού του πλήθους νημάτων: η μεταβλητή
     * περιβάλλοντος, η omp_set_num_threads και ο όρος num_threads.
     * Ο όρος έχει την υψηλότερη προτεραιότητα. */
    int from_env = omp_get_max_threads();               /* ό,τι όρισε το OMP_NUM_THREADS */
    printf("  από το OMP_NUM_THREADS (ή την προεπιλογή): %d νήματα\n", from_env);
    omp_set_num_threads(3);
    #pragma omp parallel
    #pragma omp single
    printf("  μετά την omp_set_num_threads(3): %d νήματα\n", omp_get_num_threads());
    #pragma omp parallel num_threads(2)
    #pragma omp single
    printf("  με τον όρο num_threads(2):       %d νήματα\n", omp_get_num_threads());
    omp_set_num_threads(from_env);                      /* επαναφορά της αρχικής τιμής */

    /* (γ) Σειριακή εκτέλεση: ο χρόνος αναφοράς για τη σύγκριση */
    const int REPS = 5;                     /* κάθε μέτρηση 5 φορές, κρατάμε την καλύτερη */
    double *serial = malloc(c.S * sizeof *serial);
    double ts = 1e30;
    for (int r = 0; r < REPS; r++) {
        double t0 = omp_get_wtime();
        for (int s = 0; s < c.S; s++) serial[s] = sensor_mean(&c, data, s);
        double dt = omp_get_wtime() - t0;
        if (dt < ts) ts = dt;
    }

    /* (δ) Παράλληλη εκτέλεση με χειροκίνητη κατανομή: κάθε νήμα υπολογίζει το
     * δικό του κομμάτι. Ο όρος if: για λίγα δεδομένα δεν αξίζει να δημιουργηθεί ομάδα. */
    int used = 0;                                            /* πόσα νήματα έτρεξαν πραγματικά */
    int *range = calloc(2 * omp_get_max_threads(), sizeof *range);   /* [first, last) ανά νήμα */
    double tp = 1e30;
    for (int r = 0; r < REPS; r++) {
        double t0 = omp_get_wtime();
        #pragma omp parallel if ((long)c.S * c.T > 100000)
        {
            int id = omp_get_thread_num(), n = omp_get_num_threads();
            if (id == 0) used = n;
            int chunk = c.S / n, rest = c.S % n;             /* οι πρώτοι "rest" παίρνουν +1 */
            int first = id * chunk + (id < rest ? id : rest);
            int last  = first + chunk + (id < rest ? 1 : 0); /* [first, last) */
            for (int s = first; s < last; s++) {
                double sum = 0;                              /* τοπική: δηλωμένη μέσα στο νήμα */
                for (int t = 0; t < c.T; t++) sum += data[(size_t)s * c.T + t];
                mean[s] = sum / c.T;                         /* κάθε νήμα γράφει άλλες θέσεις */
            }
            range[2 * id] = first; range[2 * id + 1] = last;
        }
        double dt = omp_get_wtime() - t0;
        if (dt < tp) tp = dt;
    }
    for (int id = 0; id < used; id++)
        printf("  νήμα %d: αισθητήρες %d–%d\n", id, range[2 * id], range[2 * id + 1] - 1);

    /* (ε) Έλεγχος αποτελεσμάτων και σύγκριση χρόνων */
    int bad = 0;
    for (int s = 0; s < c.S; s++) bad += fabs(mean[s] - serial[s]) > 1e-9;
    double speedup = ts / tp;
    printf("Μέση θερμοκρασία αισθητήρα 0: %.3f °C, αισθητήρα %d: %.3f °C\n", mean[0], c.S - 1, mean[c.S - 1]);
    printf("Διαφορές από τη σειριακή: %d αισθητήρες\n", bad);
    printf("Σειριακά:  %8.3f ms\n", 1e3 * ts);
    printf("Παράλληλα: %8.3f ms με %d νήματα\n", 1e3 * tp, used);
    printf("Επιτάχυνση (speedup) = %.2f · αποδοτικότητα (efficiency) = %.0f%%\n", speedup, 100.0 * speedup / used);
    free(serial); free(range);
    free(data); free(mean);
    return 0;
}

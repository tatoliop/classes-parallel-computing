/* Βήμα 6 – Συγχρονισμός (Μάργαρης, Κεφ. 4, σλ. 120–129)
 *
 * Έννοιες: critical (με όνομα), atomic, master/masked και single, barrier,
 * flush, και πότε χρειάζεται το καθένα.
 *
 * Ανίχνευση συναγερμών: μετρήσεις πάνω από ALARM_TEMP. Μετράμε πόσες είναι
 * (atomic), κρατάμε τις πρώτες MAXLOG σε κοινόχρηστο πίνακα (critical),
 * και στο τέλος ένα νήμα τυπώνει τη σύνοψη (single) αφού όλοι τελειώσουν (barrier).
 *   OMP_NUM_THREADS=4 ./step06_sync
 */
#include "sensors.h"
#define MAXLOG 8

typedef struct { int s, t; float temp; } alarm_t;

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    float *data = malloc(N * sizeof *data);
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++) data[(size_t)s * c.T + t] = reading(&c, s, t);

    long alarms = 0;                 /* συνολικό πλήθος συναγερμών */
    alarm_t log[MAXLOG]; int nlog = 0;
    int ready = 0;                   /* σημαία για το (δ) */
    double t0 = omp_get_wtime();

    #pragma omp parallel
    {
        #pragma omp master           /* μόνο το νήμα 0, χωρίς barrier */
        printf("Έναρξη ανίχνευσης με %d νήματα\n", omp_get_num_threads());

        #pragma omp for
        for (int s = 0; s < c.S; s++)
            for (int t = 0; t < c.T; t++) {
                float x = data[(size_t)s * c.T + t];
                if (x > ALARM_TEMP) {
                    #pragma omp atomic           /* μία απλή ενημέρωση: atomic αρκεί */
                    alarms++;
                    #pragma omp critical(alarm_log)   /* δύο εντολές μαζί: χρειάζεται critical */
                    if (nlog < MAXLOG) { log[nlog].s = s; log[nlog].t = t; log[nlog].temp = x; nlog++; }
                }
            }
        /* implicit barrier του for: όλοι έχουν τελειώσει την ανίχνευση */

        #pragma omp single           /* ένα (οποιοδήποτε) νήμα· οι άλλοι περιμένουν στο τέλος */
        {
            printf("Συναγερμοί: %ld (%.4f%% των μετρήσεων), χρόνος %.3f s\n", alarms, 100.0 * alarms / N, omp_get_wtime() - t0);
            for (int i = 0; i < nlog; i++)
                printf("  αισθητήρας %4d, ώρα %02d:%02d → %.1f °C\n", log[i].s,
                       log[i].t * 24 / c.T, (log[i].t * 24 * 60 / c.T) % 60, log[i].temp);
        }

        /* (δ) flush: το νήμα 1 περιμένει μια σημαία που σηκώνει το νήμα 0.
         * Η ανάγνωση και η εγγραφή της σημαίας γίνονται atomic, και το flush
         * εξασφαλίζει ότι το νήμα 1 βλέπει και τα δεδομένα που γράφτηκαν πριν. */
        if (omp_get_num_threads() > 1) {
            if (omp_get_thread_num() == 0) {
                alarms = -alarms;                         /* "δεδομένα" για το νήμα 1 */
                #pragma omp flush
                #pragma omp atomic write
                ready = 1;
            } else if (omp_get_thread_num() == 1) {
                int r = 0;
                while (!r) {
                    #pragma omp atomic read
                    r = ready;
                }
                #pragma omp flush
                printf("Το νήμα 1 είδε τη σημαία· alarms = %ld\n", alarms);
            }
        }
        #pragma omp barrier          /* ρητή χρονική φραγή */
    }
    free(data);
    return 0;
}

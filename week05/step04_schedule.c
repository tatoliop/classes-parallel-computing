/* Βήμα 4 – Χρονοδρομολόγηση βρόχων (Μάργαρης, Κεφ. 4, σλ. 104–114)
 *
 * Έννοιες: schedule(static[,c]), dynamic[,c], guided[,c], auto, runtime,
 * η μεταβλητή περιβάλλοντος OMP_SCHEDULE.
 *
 * Εξομάλυνση (κινητός μέσος όρος) των μετρήσεων. Οι αισθητήρες με μεγαλύτερο
 * αριθμό είναι πιο «θορυβώδεις» και χρειάζονται μεγαλύτερο παράθυρο, άρα το
 * κόστος ανά αισθητήρα ΔΕΝ είναι ίσο: από 1 έως 65 προσθέσεις ανά μέτρηση.
 *   ./run_schedules.sh
 *   OMP_SCHEDULE="dynamic,8" ./step04_schedule
 */
#include "sensors.h"

static inline int window(const cfg_t *c, int s) { return 1 + 2 * (s * 32 / c->S); }  /* 1, 3, ..., 63 */

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    float *data = malloc(N * sizeof *data), *smooth = malloc(N * sizeof *smooth);
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++) data[(size_t)s * c.T + t] = reading(&c, s, t);

    int nt = omp_get_max_threads();
    double *busy = calloc(nt, sizeof *busy);
    double t0 = omp_get_wtime();
    #pragma omp parallel
    {
        #pragma omp for schedule(runtime) nowait        /* η πολιτική έρχεται από το OMP_SCHEDULE */
        for (int s = 0; s < c.S; s++) {
            int w = window(&c, s), h = w / 2;
            const float *x = &data[(size_t)s * c.T];
            float *y = &smooth[(size_t)s * c.T];
            for (int t = 0; t < c.T; t++) {
                float sum = 0; int n = 0;
                for (int k = t - h; k <= t + h; k++)
                    if (k >= 0 && k < c.T) { sum += x[k]; n++; }
                y[t] = sum / n;
            }
        }
        busy[omp_get_thread_num()] = omp_get_wtime() - t0;   /* πότε τελείωσε αυτό το νήμα */
    }
    double total = omp_get_wtime() - t0;

    const char *sched = getenv("OMP_SCHEDULE");    /* η πολιτική που ζητήσαμε */
    double mx = 0, sum = 0;
    for (int i = 0; i < nt; i++) { sum += busy[i]; if (busy[i] > mx) mx = busy[i]; }
    printf("OMP_SCHEDULE=%-11s χρόνος=%.3f s  ανισορροπία (max/avg)=%.2f  ", sched ? sched : "(προεπιλογή)", total, mx / (sum / nt));
    printf("[νήματα:");
    for (int i = 0; i < nt; i++) printf(" %.2f", busy[i]);
    printf("]\nΕξομαλυμένη θερμοκρασία αισθητήρα %d στο μεσημέρι: %.3f °C\n", c.S - 1, smooth[(size_t)(c.S - 1) * c.T + c.T / 2]);
    free(data); free(smooth); free(busy);
    return 0;
}

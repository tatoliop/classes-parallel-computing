/* Βήμα 8 – Ανεξάρτητες εργασίες με sections (Μάργαρης, Κεφ. 4, σλ. 157–158)
 *
 * Έννοιες: sections / section, κάθε ενότητα εκτελείται από ένα (οποιοδήποτε)
 * νήμα, τι γίνεται όταν τα νήματα είναι περισσότερα ή λιγότερα από τις
 * ενότητες, nowait, lastprivate σε sections.
 *
 * Τρεις εντελώς διαφορετικές αναλύσεις των ίδιων δεδομένων τρέχουν ταυτόχρονα:
 * στατιστικά, ανίχνευση συναγερμών και εξαγωγή αναφοράς σε αρχείο CSV.
 *   OMP_NUM_THREADS=2 ./step08_sections ; OMP_NUM_THREADS=4 ./step08_sections
 */
#include "sensors.h"

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    float *data = malloc(N * sizeof *data);
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++) data[(size_t)s * c.T + t] = reading(&c, s, t);

    double mean = 0; long alarms = 0; int rows = 0;
    const char *last = "";
    double t0 = omp_get_wtime();
    #pragma omp parallel sections lastprivate(last)
    {
        #pragma omp section
        {   /* 1. στατιστικά */
            double sum = 0;
            for (size_t i = 0; i < N; i++) sum += data[i];
            mean = sum / N;
            printf("  [νήμα %d] στατιστικά: έτοιμα στα %.3f s\n", omp_get_thread_num(), omp_get_wtime() - t0);
            last = "στατιστικά";
        }
        #pragma omp section
        {   /* 2. συναγερμοί */
            for (size_t i = 0; i < N; i++) alarms += data[i] > ALARM_TEMP;
            printf("  [νήμα %d] συναγερμοί: έτοιμοι στα %.3f s\n", omp_get_thread_num(), omp_get_wtime() - t0);
            last = "συναγερμοί";
        }
        #pragma omp section
        {   /* 3. αναφορά: μέση θερμοκρασία ανά αισθητήρα σε CSV */
            FILE *f = fopen("report.csv", "w");
            fprintf(f, "sensor,mean\n");
            for (int s = 0; s < c.S; s++, rows++) fprintf(f, "%d,%.4f\n", s, sensor_mean(&c, data, s));
            fclose(f);
            printf("  [νήμα %d] αναφορά: έτοιμη στα %.3f s\n", omp_get_thread_num(), omp_get_wtime() - t0);
            last = "αναφορά";
        }
    }   /* implicit barrier */
    printf("Μέση θερμοκρασία %.3f °C, συναγερμοί %ld, report.csv με %d γραμμές  [συνολικά %.3f s]\n",
           mean, alarms, rows, omp_get_wtime() - t0);
    printf("lastprivate: η τιμή της τελευταίας (στον κώδικα) ενότητας = \"%s\"\n", last);
    printf("Με 2 νήματα: ποιο νήμα εκτελεί δύο ενότητες; Με 4: τι κάνει το τέταρτο;\n");
    free(data);
    return 0;
}

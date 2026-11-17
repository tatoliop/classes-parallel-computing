/* Βήμα 7 – Μείωση δεδομένων, reduction (Μάργαρης, Κεφ. 4, σλ. 130–150)
 *
 * Έννοιες: reduction(+:), reduction(min:), reduction(max:), πολλαπλές μειώσεις
 * σε έναν βρόχο, και μείωση ορισμένη από τον χρήστη (declare reduction).
 *
 * Συνολικά στατιστικά όλων των μετρήσεων: μέση τιμή, ελάχιστη, μέγιστη και
 * τυπική απόκλιση. Πρώτα με ενσωματωμένες μειώσεις, μετά με έναν δικό μας
 * τύπο stats_t που τα υπολογίζει όλα μαζί σε ένα πέρασμα.
 *   OMP_NUM_THREADS=4 ./step07_reduction
 */
#include "sensors.h"
#include <float.h>

typedef struct { long n; double sum, sumsq; float min, max; } stats_t;

static void stats_add(stats_t *a, float x) {
    a->n++; a->sum += x; a->sumsq += (double)x * x;
    if (x < a->min) a->min = x;
    if (x > a->max) a->max = x;
}
static void stats_merge(stats_t *out, const stats_t *in) {
    out->n += in->n; out->sum += in->sum; out->sumsq += in->sumsq;
    if (in->min < out->min) out->min = in->min;
    if (in->max > out->max) out->max = in->max;
}
/* Η δική μας πράξη μείωσης: πώς συνδυάζονται δύο μερικά αποτελέσματα
 * (omp_out, omp_in) και ποια είναι η αρχική τιμή κάθε ιδιωτικού αντιγράφου. */
#pragma omp declare reduction(merge : stats_t : stats_merge(&omp_out, &omp_in)) \
        initializer(omp_priv = (stats_t){ 0, 0.0, 0.0, FLT_MAX, -FLT_MAX })

int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    size_t N = (size_t)c.S * c.T;
    float *data = malloc(N * sizeof *data);
    #pragma omp parallel for
    for (int s = 0; s < c.S; s++)
        for (int t = 0; t < c.T; t++) data[(size_t)s * c.T + t] = reading(&c, s, t);

    /* (α) Ενσωματωμένες μειώσεις: κάθε νήμα έχει ιδιωτικό αντίγραφο αρχικοποιημένο
     *     στο ουδέτερο στοιχείο (0 για +, +∞ για min, −∞ για max) */
    double sum = 0, sumsq = 0;
    float mn = FLT_MAX, mx = -FLT_MAX;
    double t0 = omp_get_wtime();
    #pragma omp parallel for reduction(+:sum, sumsq) reduction(min:mn) reduction(max:mx)
    for (size_t i = 0; i < N; i++) {
        float x = data[i];
        sum += x; sumsq += (double)x * x;
        if (x < mn) mn = x;
        if (x > mx) mx = x;
    }
    double mean = sum / N, sd = sqrt(sumsq / N - mean * mean);
    printf("(α) reduction(+, min, max): μέση %.4f, min %.2f, max %.2f, σ %.4f  [%.3f s]\n", mean, mn, mx, sd, omp_get_wtime() - t0);

    /* (β) Η ίδια δουλειά με τη δική μας μείωση */
    stats_t st = { 0, 0.0, 0.0, FLT_MAX, -FLT_MAX };
    t0 = omp_get_wtime();
    #pragma omp parallel for reduction(merge:st)
    for (size_t i = 0; i < N; i++) stats_add(&st, data[i]);
    mean = st.sum / st.n; sd = sqrt(st.sumsq / st.n - mean * mean);
    printf("(β) declare reduction:      μέση %.4f, min %.2f, max %.2f, σ %.4f  [%.3f s]  n=%ld\n", mean, st.min, st.max, sd, omp_get_wtime() - t0, st.n);

    /* (γ) Σειριακός έλεγχος */
    stats_t ser = { 0, 0.0, 0.0, FLT_MAX, -FLT_MAX };
    for (size_t i = 0; i < N; i++) stats_add(&ser, data[i]);
    printf("(γ) σειριακά:               μέση %.4f, min %.2f, max %.2f\n", ser.sum / ser.n, ser.min, ser.max);
    printf("Γιατί οι μέσες τιμές μπορεί να διαφέρουν στο τελευταίο δεκαδικό; (σειρά προσθέσεων)\n");
    free(data);
    return 0;
}

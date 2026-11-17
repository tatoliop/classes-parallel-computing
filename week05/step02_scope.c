/* Βήμα 2 – Εμβέλεια μεταβλητών (Μάργαρης, Κεφ. 4, σλ. 92–97)
 *
 * Έννοιες: shared (η προεπιλογή), private, firstprivate, threadprivate,
 * default(none), και τι γίνεται όταν ξεχάσουμε να κάνουμε μια μεταβλητή private.
 *
 * Υπολογίζουμε ξανά τη μέση θερμοκρασία ανά αισθητήρα, τώρα με διόρθωση
 * βαθμονόμησης (offset) που προστίθεται σε κάθε μέτρηση.
 *   ./step02_scope                       (σωστή εκδοχή)
 *   make step02_bug && ./step02_bug      (το sum ξεχάστηκε shared, μεταγλώττιση με -O0)
 *   make step02_bug_O2 && ./step02_bug_O2  (το ίδιο λάθος με -O2: συνήθως ΚΡΥΒΕΤΑΙ!)
 *
 * Γιατί με -O2 το λάθος κρύβεται; Ο compiler βλέπει ότι μέσα στον εσωτερικό
 * βρόχο κανείς άλλος δεν «πρέπει» να αγγίζει το sum (το race είναι
 * απροσδιόριστη συμπεριφορά), οπότε το κρατά σε καταχωρητή του κάθε νήματος
 * και το γράφει στη μνήμη μόνο στο τέλος. Το πρόγραμμα παραμένει λάθος: με
 * άλλον compiler, άλλη έκδοση ή μια μικρή αλλαγή στον κώδικα, το λάθος
 * εμφανίζεται. Με -O0 το sum διαβάζεται και γράφεται στη μνήμη σε κάθε
 * επανάληψη, οπότε τα νήματα «πατούν» το ένα το άθροισμα του άλλου.
 */
#include "sensors.h"

static long processed = 0;               /* μετρητής μετρήσεων ΑΝΑ ΝΗΜΑ */
#pragma omp threadprivate(processed)     /* κάθε νήμα έχει το δικό του αντίγραφο, που διατηρείται */
                                         /* από παράλληλη περιοχή σε παράλληλη περιοχή */
int main(int argc, char **argv) {
    cfg_t c = parse_args(argc, argv);
    float *data = make_data(&c);
    double *mean = malloc(c.S * sizeof *mean);
    double offset = 0.5;                 /* βαθμονόμηση: +0.5 °C σε κάθε μέτρηση */
    double sum;                          /* δηλωμένη ΕΞΩ από την παράλληλη περιοχή */
    int s, t;


#ifdef BUG
    #pragma omp parallel private(s, t) firstprivate(offset)        /* ξεχάσαμε το sum */
#else
    #pragma omp parallel default(none) private(s, t, sum) firstprivate(offset) shared(c, data, mean)
#endif
    {
        int id = omp_get_thread_num(), n = omp_get_num_threads();
        for (s = id; s < c.S; s += n) {          /* κυκλική κατανομή: id, id+n, id+2n, ... */
            sum = 0;
            for (t = 0; t < c.T; t++) sum += data[(size_t)s * c.T + t] + offset;
            mean[s] = sum / c.T;
            processed += c.T;
        }
        offset = -999;   /* αλλάζει μόνο το ιδιωτικό αντίγραφο: το offset του main δεν επηρεάζεται */
    }
    printf("Μετά την περιοχή: offset = %.1f (το αρχικό, γιατί ήταν firstprivate)\n", offset);

    /* Δεύτερη παράλληλη περιοχή: κάθε νήμα βρίσκει το processed όπως το άφησε */
    #pragma omp parallel
    {
        #pragma omp critical
        printf("  νήμα %d: είχε επεξεργαστεί %ld μετρήσεις\n", omp_get_thread_num(), processed);
    }

    /* Έλεγχος με τη σειριακή εκδοχή: πόσοι αισθητήρες έχουν λάθος μέση τιμή; */
    int bad = 0, worst = 0;
    double maxerr = 0;
    for (int k = 0; k < c.S; k++) {
        double err = fabs(mean[k] - (sensor_mean(&c, data, k) + 0.5));
        if (err > 1e-6) bad++;
        if (err > maxerr) { maxerr = err; worst = k; }
    }
    printf("Μέση θερμοκρασία αισθητήρα 0: %.3f °C\n", mean[0]);
    if (bad == 0)
        printf("Διαφορές από τη σειριακή: 0 αισθητήρες ✓\n");
    else
        printf("Διαφορές από τη σειριακή: %d αισθητήρες ΛΑΘΟΣ! Μεγαλύτερο σφάλμα %.3f °C στον αισθητήρα %d "
               "(%.3f αντί για %.3f)\n", bad, maxerr, worst, mean[worst], sensor_mean(&c, data, worst) + 0.5);
    free(data); free(mean);
    return 0;
}

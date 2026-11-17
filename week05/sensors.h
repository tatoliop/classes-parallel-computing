/* sensors.h – κοινός κώδικας για το παράδειγμα «σταθμός αισθητήρων θερμοκρασίας».
 *
 * Το σενάριο: S αισθητήρες καταγράφουν θερμοκρασία κάθε 10 δευτερόλεπτα για
 * ένα 24ωρο (T = 8640 μετρήσεις ο καθένας). Τα δεδομένα είναι συνθετικά αλλά
 * ντετερμινιστικά (με λίγες μεμονωμένες αιχμές-σφάλματα και τοπικούς
 * «καύσωνες» 13:00–15:00 σε κάποιους αισθητήρες): η μέτρηση (s, t) είναι πάντα ίδια, όποια σειρά κι αν
 * ακολουθήσουν τα νήματα. Έτσι κάθε παράλληλη έκδοση μπορεί να συγκριθεί
 * με τη σειριακή.
 *
 * Εδώ βρίσκεται μόνο ό,τι ΔΕΝ αφορά το OpenMP, ώστε τα αρχεία stepNN να
 * δείχνουν μόνο τη νέα έννοια.
 */
#ifndef SENSORS_H
#define SENSORS_H

#include <math.h>
#include <omp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ALARM_TEMP 40.0f      /* όριο συναγερμού (°C) */

typedef struct { int S, T; } cfg_t;

static cfg_t parse_args(int argc, char **argv) {
    cfg_t c = { 1000, 8640 };
    int opt;
    while ((opt = getopt(argc, argv, "s:t:")) != -1) {
        if (opt == 's') c.S = atoi(optarg);
        else if (opt == 't') c.T = atoi(optarg);
        else { fprintf(stderr, "χρήση: %s [-s αισθητήρες] [-t μετρήσεις]\n", argv[0]); exit(1); }
    }
    return c;
}

static inline uint32_t hash32(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352dU;
    x ^= x >> 15; x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

/* Η μέτρηση του αισθητήρα s τη χρονική στιγμή t:
 * βάση ανά αισθητήρα + ημερήσιος κύκλος + θόρυβος + σπάνιες αιχμές (συναγερμοί). */
static inline float reading(const cfg_t *c, int s, int t) {
    uint32_t h = hash32((uint32_t)s * 2654435761U ^ ((uint32_t)t * 40503U + 12345U));
    float base  = 12.0f + 10.0f * (float)((s * 37) % 100) / 100.0f + 4.0f * s / c->S;
    float daily = 8.0f * sinf(6.2831853f * t / c->T - 1.5707963f);
    float noise = ((float)(h & 0xffff) / 65535.0f - 0.5f) * (0.5f + 0.2f * (s % 10));
    float spike = ((h >> 16) % 4000 == 0) ? 25.0f : 0.0f;                 /* σφάλμα αισθητήρα */
    float wave  = (s % 97 == 5 && t > c->T * 13 / 24 && t < c->T * 15 / 24) ? 18.0f : 0.0f;  /* καύσωνας */
    return base + daily + noise + spike + wave;
}

/* Σειριακή δημιουργία των δεδομένων: data[s*T + t]. */
__attribute__((unused)) static float *make_data(const cfg_t *c) {
    float *d = malloc((size_t)c->S * c->T * sizeof *d);
    for (int s = 0; s < c->S; s++)
        for (int t = 0; t < c->T; t++)
            d[(size_t)s * c->T + t] = reading(c, s, t);
    return d;
}

/* Μέσος όρος ενός αισθητήρα (σειριακά, για έλεγχο αποτελεσμάτων). */
__attribute__((unused)) static double sensor_mean(const cfg_t *c, const float *d, int s) {
    double sum = 0;
    for (int t = 0; t < c->T; t++) sum += d[(size_t)s * c->T + t];
    return sum / c->T;
}

#endif

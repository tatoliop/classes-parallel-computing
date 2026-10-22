# Εβδομάδα 3 – Profiling

```bash
make
# 1. gprof: ποια συνάρτηση είναι το hotspot;
./profile_me_pg                       # δημιουργεί gmon.out
gprof ./profile_me_pg gmon.out | head -15

# 2. perf (Linux, αν δουλεύει)
perf stat ./profile_me
perf record ./profile_me && perf report
```

## Προσοχή
* `gprof` χρειάζεται `-pg` (στο `make profile_me_pg`). Σε macOS δεν υπάρχει → χρησιμοποιήστε μόνο `time`.
* Το `perf` συνήθως δεν δουλεύει σε WSL/Docker/VM χωρίς δικαιώματα (`sudo sysctl kernel.perf_event_paranoid=1`).

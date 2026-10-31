// Parallel array sum with std::thread: block decomposition + partial results.
//   ./sum_threads [n] [threads]
// Exercise: (1) measure speedup for 1,2,4,8 threads;
//           (2) store the running sum directly in partial[t] inside the loop
//               (instead of a local variable) and compare -> false sharing!
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include "timing.h"

int main(int argc, char **argv) {
    long n = argc > 1 ? std::atol(argv[1]) : 200000000L;
    int T = argc > 2 ? std::atoi(argv[2]) : (int)std::thread::hardware_concurrency();
    if (T < 1) T = 1;

    std::vector<int> a(n);
    for (long i = 0; i < n; i++) a[i] = (int)(i % 10);

    double t0 = wtime();
    long serial = 0;
    for (long i = 0; i < n; i++) serial += a[i];
    double t_serial = wtime() - t0;

    t0 = wtime();
    std::vector<long> partial(T, 0);
    std::vector<std::thread> ts;
    for (int t = 0; t < T; t++) {
        long lo = n / T * t, hi = (t == T - 1) ? n : n / T * (t + 1);
        ts.emplace_back([&, t, lo, hi] {
            long local = 0;                       // accumulate privately ...
            for (long i = lo; i < hi; i++) local += a[i];
            partial[t] = local;                   // ... write shared memory once
        });
    }
    for (auto &th : ts) th.join();
    long parallel = 0;
    for (long p : partial) parallel += p;
    double t_par = wtime() - t0;

    std::printf("n=%ld threads=%d serial=%ld parallel=%ld %s\n", n, T, serial, parallel,
                serial == parallel ? "OK" : "WRONG");
    std::printf("serial_s=%.6f parallel_s=%.6f speedup=%.2f\n", t_serial, t_par, t_serial / t_par);
    std::printf("time_s=%.6f\n", t_par);
}

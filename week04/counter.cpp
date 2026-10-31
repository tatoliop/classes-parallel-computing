// The classic data race, and two ways to fix it.
//   ./counter racy   [threads] [iters]   -> WRONG result (lost updates)
//   ./counter mutex  [threads] [iters]   -> correct, slower (lock per increment)
//   ./counter atomic [threads] [iters]   -> correct, faster than the mutex
//
// `counter_racy` is volatile on purpose: otherwise the optimiser keeps the
// counter in a register inside the loop and the bug may not show up.
// Run `make tsan && ./counter_tsan racy` to let ThreadSanitizer find the race.
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>
#include "timing.h"

static volatile long counter_racy = 0;
static long counter_mutex = 0;
static std::atomic<long> counter_atomic{0};
static std::mutex m;

int main(int argc, char **argv) {
    const char *mode = argc > 1 ? argv[1] : "racy";
    int nthreads = argc > 2 ? std::atoi(argv[2]) : 4;
    long iters = argc > 3 ? std::atol(argv[3]) : 1000000;

    auto body = [&]() {
        for (long i = 0; i < iters; i++) {
            if (!std::strcmp(mode, "racy")) {
                counter_racy = counter_racy + 1;                       // read, add, write: NOT atomic
            } else if (!std::strcmp(mode, "mutex")) {
                std::lock_guard<std::mutex> lk(m);
                counter_mutex++;
            } else {
                counter_atomic.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };

    double t0 = wtime();
    std::vector<std::thread> ts;
    for (int i = 0; i < nthreads; i++) ts.emplace_back(body);
    for (auto &t : ts) t.join();
    double t = wtime() - t0;

    long got = !std::strcmp(mode, "racy") ? counter_racy
             : !std::strcmp(mode, "mutex") ? counter_mutex : counter_atomic.load();
    long expected = (long)nthreads * iters;
    std::printf("mode=%s threads=%d expected=%ld got=%ld %s\n", mode, nthreads, expected, got,
                got == expected ? "OK" : "<-- WRONG (lost updates)");
    std::printf("time_s=%.6f\n", t);
}

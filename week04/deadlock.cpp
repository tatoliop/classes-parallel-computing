// Two threads take two mutexes in opposite order => deadlock.
//   timeout 3 ./deadlock        # hangs, `timeout` kills it after 3 s (exit code 124)
//   ./deadlock fix              # std::scoped_lock locks both without deadlock
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>

static std::mutex a, b;
using namespace std::chrono_literals;

int main(int argc, char **argv) {
    bool fix = argc > 1 && !std::strcmp(argv[1], "fix");

    std::thread t1([&] {
        if (fix) { std::scoped_lock lk(a, b); std::puts("T1 got both locks"); return; }
        std::lock_guard<std::mutex> la(a);
        std::puts("T1 holds A, wants B");
        std::this_thread::sleep_for(100ms);              // make the bad interleaving certain
        std::lock_guard<std::mutex> lb(b);
        std::puts("T1 got both locks");
    });
    std::thread t2([&] {
        if (fix) { std::scoped_lock lk(a, b); std::puts("T2 got both locks"); return; }
        std::lock_guard<std::mutex> lb(b);
        std::puts("T2 holds B, wants A");
        std::this_thread::sleep_for(100ms);
        std::lock_guard<std::mutex> la(a);
        std::puts("T2 got both locks");
    });
    t1.join();
    t2.join();
    std::puts("finished without deadlock");
}

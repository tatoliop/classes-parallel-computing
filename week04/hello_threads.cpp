// Create N threads, let each print, then join them.
// build: g++ -std=c++17 -pthread hello_threads.cpp -o hello_threads
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <thread>
#include <vector>

static std::mutex print_mutex;   // try removing the lock: output gets interleaved

static void worker(int id) {
    std::lock_guard<std::mutex> lock(print_mutex);
    std::printf("hello from thread %d (std::thread::id %zu)\n", id,
                std::hash<std::thread::id>{}(std::this_thread::get_id()) % 100000);
}

int main(int argc, char **argv) {
    int n = argc > 1 ? std::atoi(argv[1]) : 4;
    std::vector<std::thread> threads;
    for (int i = 0; i < n; i++) threads.emplace_back(worker, i);
    for (auto &t : threads) t.join();
    std::printf("all %d threads joined (hardware threads: %u)\n", n,
                std::thread::hardware_concurrency());
}

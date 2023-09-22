#include <thread>
#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <functional>
#include <chrono>


template<typename Iterator, typename T>
struct accumulate_block {
    void operator () (Iterator first, Iterator last, T& result) {
        result = std::accumulate(first, last, result);
    }
};

template<typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    auto length = std::distance(first, last);
    if (!length) return init;

    int const min_length_per_thread = 25;
    int const max_threads = (length + min_length_per_thread - 1) / min_length_per_thread;

    int hardware_threads = std::jthread::hardware_concurrency();
    if (!hardware_threads) hardware_threads = 2;

    int const n_threads = std::min(hardware_threads, max_threads);
    int const block_size = length / n_threads;

    std::vector<T> results(n_threads);
    std::vector<std::jthread> threads(n_threads);

    Iterator block_start = first, block_end = first;

    for (int i = 0; i < n_threads; i++) {
        std::advance(block_end, block_size);
        threads[i] = std::jthread(
            accumulate_block<Iterator, T>(),
            block_start, block_end, std::ref(results[i])
        );
        block_start = block_end;
    }

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::jthread::join));

    return std::accumulate(results.begin(), results.end(), init);
}

using namespace std::chrono;
int main() {
    std::vector<int> example(1000000000, 1); 
    std::cout << std::thread::hardware_concurrency() << std::endl;
    auto start = high_resolution_clock::now();
    std::cout << std::accumulate(example.begin(), example.end(), 0) << std::endl;     
    auto end = high_resolution_clock::now();
    auto interval = duration_cast<microseconds>(end - start);
    std::cout << interval.count() << std::endl;

    start = high_resolution_clock::now();
    std::cout << parallel_accumulate(example.begin(), example.end(), 0) << std::endl;
    end = high_resolution_clock::now();
    interval = duration_cast<microseconds>(end-start);
    std::cout << interval.count() << std::endl;

    return 0;
}
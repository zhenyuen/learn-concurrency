#include <chrono>
#include <iostream>
#include <random>
#include <semaphore>
#include <thread>

namespace constants {
constexpr size_t MAX_BUFFER_SIZE = 10;
};

using counting_semaphore = std::counting_semaphore<constants::MAX_BUFFER_SIZE>;
using mutex = std::mutex;

int produce() {
    // Create a random number engine
    std::random_device rd;
    std::mt19937 gen(rd());  // Mersenne Twister engine

    // Define a range for the random integers
    int lower_bound = 1;
    int upper_bound = 5;

    // Create a distribution to map the random numbers to the desired range
    std::uniform_int_distribution<int> dist(lower_bound, upper_bound);

    // Produce a new item between 1 to 5 seconds
    int seconds = dist(gen);
    std::this_thread::sleep_for(std::chrono::seconds(seconds));

    std::cout << "Produced item: " << seconds << "\n";
    return seconds;
}

void consume(int item) {
    std::random_device rd; // to seed the random generator
    std::mt19937 gen(rd());

    int lower_bound = 1;
    int upper_bound = 5;

    std::uniform_int_distribution<int> dist(lower_bound, upper_bound);

    // Take between 1 to 5 seconds to consume an item
    std::this_thread::sleep_for(std::chrono::seconds(dist(gen)));

    std::cout << "Consumed item: " << item << "\n";
}

void producer(int *buffer, size_t &in, counting_semaphore &items,
              counting_semaphore &spaces, bool &exit, mutex &mtx) {
    do {
        {
            std::scoped_lock<mutex> lock(mtx);
            if (exit) break;
        }
        int item = produce();  //  Does not belong in the critical section
        spaces.acquire();
        // R/W from/to buffer belong to critical section
        buffer[in] = item;
        in = (in + 1) % constants::MAX_BUFFER_SIZE;
        items.release();
    } while (true);
}

void consumer(int *buffer, size_t &out, counting_semaphore &items,
              counting_semaphore &spaces, bool &exit, mutex &mtx) {
    do {
        {
            std::scoped_lock<mutex> lock(mtx);
            if (exit) break;
        }
        items.acquire();
        // R/W from/to buffer belong to critical section
        int item = buffer[out];
        out = (out + 1) % constants::MAX_BUFFER_SIZE;
        spaces.release();
        consume(item);  //  Does not belong in the critical section
    } while (true);
}

int main() {
    const size_t N = constants::MAX_BUFFER_SIZE;

    int buffer[N];
    size_t in = 0, out = 0;
    counting_semaphore spaces(N), items(0);
    std::mutex mtx;
    bool exit = false;

    // Must use std::ref wrapper to pass objects by reference safely
    std::thread t_producer(producer, buffer, std::ref(in), std::ref(items),
                           std::ref(spaces), std::ref(exit), std::ref(mtx));
    std::thread t_consumer(consumer, buffer, std::ref(out), std::ref(items),
                           std::ref(spaces), std::ref(exit), std::ref(mtx));

    // Terminate program after 30 seconds
    std::this_thread::sleep_for(std::chrono::seconds(30));

    {
        // Wrap mutex in scoped lock as a RAII guard to handle exceptions safely
        std::scoped_lock<mutex> lock(mtx);
        exit = true;
        // Instead of using guards, we can manually lock unlock mutexes by doing:
            // mtx.lock(); // blocks if unable to acquire lock
            // exit = true;
            // mtx.unlock();
        // However, this does not have exception safety
    }

    t_producer.join();
    t_consumer.join();

    return 0;
}
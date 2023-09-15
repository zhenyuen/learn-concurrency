#include <chrono>
#include <iostream>
#include <random>
#include <semaphore>
#include <thread>

namespace constants {
constexpr size_t MAX_READER_COUNT = 3;
};

using counting_semaphore = std::counting_semaphore<constants::MAX_READER_COUNT>;
using binary_semaphore = std::counting_semaphore<1>;
using mutex = std::mutex;

int write(int seconds) {
    std::cout << "Write: " << seconds << "\n";
    return seconds;
}

void read(int item) {
    std::cout << "Read: " << item << "\n";
}

void writer(int &data, binary_semaphore &wr, mutex &turn, bool &exit,
            mutex &mtx) {
    do {
        {
            std::scoped_lock<mutex> lock(mtx);
            if (exit) break;
        }
        // Create a random number engine
        std::random_device rd;
        std::mt19937 gen(rd());  // Mersenne Twister engine
        // Write to data every 3 to 5 seconds
        std::uniform_int_distribution<int> dist(3, 5);

        int seconds = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        
        {
            std::scoped_lock<mutex> lock(turn); // prevents deadlocks
            wr.acquire();
            data = write(seconds);
            wr.release();
        }
    } while (true);
}

void reader(int &data, binary_semaphore &wr, counting_semaphore &rd,
            mutex &turn, bool &exit, mutex &mtx) {
    do {
        {
            std::scoped_lock<mutex> lock(mtx);
            if (exit) break;
        }
        // Create a random number engine
        std::random_device rand;
        std::mt19937 gen(rand());  // Mersenne Twister engine
        // Write to data every 1 to 3 seconds
        std::uniform_int_distribution<int> dist(1, 3);

        int seconds = dist(gen);
        std::this_thread::sleep_for(std::chrono::seconds(seconds));

        {
            std::scoped_lock<mutex> lock(turn); // prevents deadlocks
        }

        // BELOW MAY LEAD TO DEADLOCKS
        // turn.lock(); // Ensure writer is not waiting
        // turn.unlock(); 

        rd.acquire();  // rd -= 1
        wr.acquire();
        rd.release();  // rd += 1

        read(data);

        rd.acquire();  // rd -= 1
        wr.release();
        rd.release();  // rd += 1
    } while (true);
}

int main() {
    const size_t N = constants::MAX_READER_COUNT;

    int data = -1;
    counting_semaphore rd(N);
    binary_semaphore wr(1);
    std::mutex turn, mtx;

    bool exit = false;

    // Must use std::ref wrapper to pass objects by reference safely
    std::thread t_writer(writer, std::ref(data), std::ref(wr), std::ref(turn),
                         std::ref(exit), std::ref(mtx));
    std::thread t_reader_1(reader, std::ref(data), std::ref(wr), std::ref(rd),
                           std::ref(turn), std::ref(exit), std::ref(mtx));
    std::thread t_reader_2(reader, std::ref(data), std::ref(wr), std::ref(rd),
                           std::ref(turn), std::ref(exit), std::ref(mtx));
    std::thread t_reader_3(reader, std::ref(data), std::ref(wr), std::ref(rd),
                           std::ref(turn), std::ref(exit), std::ref(mtx));

    // Terminate program after 30 seconds
    std::this_thread::sleep_for(std::chrono::seconds(20));

    {
        // Wrap mutex in scoped lock as a RAII guard to handle exceptions safely
        std::scoped_lock<mutex> lock(mtx);
        exit = true;
        // Instead of using guards, we can manually lock unlock mutexes by
        // doing: mtx.lock(); // blocks if unable to acquire lock exit = true;
        // mtx.unlock();
        // However, this does not have exception safety
    }

    t_writer.join();
    t_reader_1.join();
    t_reader_2.join();
    t_reader_3.join();

    return 0;
}
#include <mutex>

class object {};
void swap(object &rhs, object &lhs);

class X {
private:
    object some_detail;
    std::mutex m;

public:
    X(object const& sd): some_detail(sd) {};
    friend void swap(X& lhs, X& rhs) {
        if (&lhs == &rhs) return; // to prevent undefined behaviour
        // This function is used to lock multiple mutexes simultaneously in a deadlock-free manner. 
        std::lock(lhs.m, rhs.m);
        // adopt lock tells program to adopt ownership of the existing lock on the mutex.
        std::lock_guard<std::mutex> lock_a(lhs.m, std::adopt_lock);
        std::lock_guard<std::mutex> lock_b(rhs.m, std::adopt_lock);

        // A more costly but flexible way to this is:
        // std::unique_lock<std::mutex> lock_a(lhs.m, std::defer_lock);
        // std::unique_lock<std::mutex> lock_b(rhs.m, std::defer_lock);
        // std::lock(lhs.m, rhs.m);

        swap(lhs.some_detail, rhs.some_detail);
    }
};
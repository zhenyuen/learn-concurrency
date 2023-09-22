#include <mutex>

class some_class {};
std::mutex the_mutex;

void get_and_process_data() {
    std::unique_lock<std::mutex> my_lock(the_mutex);
    some_class data_to_process = get_next_data_chunk();
    my_lock.unlock();
    // do not need mutex to be lock for processing
    result_type result=process(data_to_process);
    my_lock.lock();
    write_result(data_to_process, result);
}
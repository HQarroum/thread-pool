#include <iostream>
#include "../../includes/thread_pool.hpp"
#include "../../includes/thread_pool_callable.hpp"
#include "../../common/executor/async_executor.hpp"

/**
 * \brief The number of callables to be scheduled.
 */
static const size_t size = 100;

/**
 * \brief An atomic counter keeping track of the
 * amount of produced work.
 */
static std::atomic<size_t> count;

/**
 * \brief An array of callable objects to schedule.
 */
static std::function<void()> callables[size] = {};

/**
 * \brief A static function worker.
 */
void static_void_function(int value) {
 // Have this thread sleep for 100ms.
 std::this_thread::sleep_for(std::chrono::milliseconds(value));
 count++;
}

int main() {
 thread::pool::pool_t pool(std::thread::hardware_concurrency() + 1);
 
 // Filling our array with `size` amount of functions,
 // bound to the number `42`.
 std::fill_n(callables, size, std::bind(static_void_function, 100));
 // Scheduling the execution in bulk.
 auto result = pool.schedule_bulk(callables, size);
 // Log whether the insertion was successful.
 std::cout << "The insertion has " << (result ? "succeeded." : "failed.") << std::endl;
 
 // Waiting for the consumers to complete.
 while (count < size)
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
 std::cout << "Consumers have finished executing tasks." << std::endl;
  
 return (0);
}
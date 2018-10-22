#include <iostream>
#include "../includes/thread_pool.hpp"
#include "../includes/thread_pool_callable.hpp"

/**
 * \brief The number of callables to be scheduled.
 */
static const size_t iterations = 100;

/**
 * \brief An array of callable objects to schedule.
 */
static std::function<void()> callables[iterations] = {};

/**
 * \brief A static function worker.
 */
int static_void_function(int argument) {
 // Have this thread sleep for 100ms.
 std::this_thread::sleep_for(std::chrono::milliseconds(100));
 return (argument + 1);
}

int main() {
 thread::pool::pool_t pool(std::thread::hardware_concurrency() + 1);
 
 // Filling our array with `iterations` amount of functions,
 // bound to the number `42`.
 std::fill_n(callables, iterations, std::bind(static_void_function, 42));
 // Scheduling the execution in bulk.
 auto result = pool.schedule_bulk(callables, iterations);
 // Log whether the insertion was successful.
 std::cout << "The insertion has " << (result ? "succeeded" : "failed") << std::endl;
 assert(result == true);
 return (0);
}
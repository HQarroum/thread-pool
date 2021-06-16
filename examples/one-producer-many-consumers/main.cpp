#include <iostream>
#include <set>
#include <future>
#include <list>
#include <random>
#include "../../includes/thread_pool.hpp"
#include "../../includes/thread_pool_callable.hpp"

/**
 * \brief The number of work to distribute across
 * consumers.
 */
static const size_t work_to_spawn = 10000;

/**
 * \brief Static void function declaration.
 */
static void static_void_function() {
  std::this_thread::sleep_for(std::chrono::microseconds(1000));
}

/**
 * \brief Application entry point.
 */
int main() {

  // A list of futures.
  std::list<std::future<void>> list;

  // Producer and consumer thread pools.
  thread::pool::parameterized_pool_t<1, 0> pool_of_consumers(std::thread::hardware_concurrency() + 1);

  // Scheduling the producers.
  for (size_t i = 0; i < work_to_spawn; ++i) {
    list.push_back(pool_of_consumers.schedule(static_void_function));
  }
  
  // Counting the start time.
  auto start = std::chrono::high_resolution_clock::now();

  // Waiting for the consumers to complete.
  for (std::future<void>& future : list) {
    future.wait();
  }

  // Measuring and dumping the elapsed time.
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> diff = end - start;
  std::cout << "Elasped time : " << diff.count() << " ms" << std::endl;

  return (0);
}

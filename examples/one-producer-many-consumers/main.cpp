#include <iostream>
#include <set>
#include "../../includes/thread_pool.hpp"
#include "../../includes/thread_pool_callable.hpp"

/**
 * \brief An atomic counter keeping track of the
 * amount of produced work.
 */
static std::atomic<size_t> count;

/**
 * \brief The number of work to distribute across
 * consumers.
 */
static const size_t work_to_spawn = 1000;

/**
 * \brief Static void function declaration.
 */
int static_void_function(int value) {
  count++;
  return (++value);
}

/**
 * \brief Application entry point.
 */
int main() {
  // Producer and consumer thread pools.
  thread::pool::pool_t pool_of_consumers(5);

  // Counting the start time.
  auto start = std::chrono::high_resolution_clock::now();

  // Scheduling the producers.
  for (size_t i = 0; i < work_to_spawn; ++i) {
    auto result = pool_of_consumers.schedule(static_void_function, i);
    assert(result.get() == (int) (i + 1));
  }
  
  // Waiting for the consumers to complete.
  while (count < work_to_spawn)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Measuring and dumping the elapsed time.
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> diff = end - start;
  std::cout << "Elasped time : " << diff.count() << " ms" << std::endl;

  return (0);
}

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
 * \brief The number of producer threads to spawn.
 */
static const size_t workers_to_spawn = 1000;

/**
 * \brief The number of work per producer to schedule.
 */
static const size_t work_by_worker = 1000;

/**
 * \brief Static void function declaration.
 */
int static_void_function() {
  return (++count);
}

/**
 * \brief A producer worker scheduling work on
 * the consumer pool.
 */
void producer(thread::pool::pool_t* pool) {
  for (size_t i = 0; i < work_by_worker; ++i) {
    pool->schedule(static_void_function);
  }
} 

/**
 * \brief Application entry point.
 */
int main() {
  // Producer and consumer thread pools.
  thread::pool::pool_t pool_of_consumers(5);
  thread::pool::pool_t pool_of_producers(5);

  // Counting the start time.
  auto start = std::chrono::high_resolution_clock::now();

  // Array of producer workers.
  auto producers = new std::function<void()>[workers_to_spawn];
  // Filling the array with the producers.
  std::fill_n(producers, workers_to_spawn, std::bind(producer, &pool_of_consumers));

  // Scheduling the producers.
  pool_of_producers.schedule_bulk(producers, workers_to_spawn);

  // Waiting for the producers to complete.
  pool_of_producers.stop().await();
  
  // Waiting for the consumers to complete.
  while (count < workers_to_spawn * work_by_worker)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Measuring and dumping the elapsed time.
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> diff = end - start;
  std::cout << "Elasped time : " << diff.count() << " ms" << std::endl;

  // Deleting the producer array.
  delete[] producers;
  return (0);
}

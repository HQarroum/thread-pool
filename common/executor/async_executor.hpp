#include <functional>
#include "../../includes/blocking_concurrent_queue.hpp"

struct async_executor_t {

  /**
   * \brief A callable object type definition.
   */
  using callable_t = std::function<void()>;
  
  /**
   * \brief Enqueues the given callable object to be
   * executed asynchronously by consumers of the queue.
   */
  void execute_async(const callable_t& value) {
    queue.enqueue(value);
  }

  /**
   * \brief Dequeues a callable object from the queue.
   * \return a boolean value indicating whether there was
   * an element to dequeue.
   */
  bool dequeue(callable_t& callable) {
    return (queue.wait_dequeue_timed(callable, 0));
  }

  /**
   * \brief Runs an event loop until the internal loop
   * is empty.
   */
  int run() {
    // Dequeuing and outputing logs from the workers.
    async_executor_t::callable_t callable;
    while (dequeue(callable)) {
      callable();
    }
    return (0);
  }

private:

  /**
   * \brief Using a queue to output the logs produced by the
   * workers on the main thread.
   */
  moodycamel::BlockingConcurrentQueue<callable_t> queue;
};
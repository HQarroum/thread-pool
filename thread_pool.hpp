#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <vector>
#include <functional>
#include <thread>
#include <type_traits>
#include <future>
#include <unordered_map>

#include "blocking_concurrent_queue.hpp"
#include "optional.hpp"

namespace thread {

  namespace pool {

    /**
     * \brief Hint indicating the use of lightweight processing
     * of tasks within a worker.
     */
    const size_t WORK_PARTITIONING_LIGHT    = 10;
    
    /**
     * \brief Hint indicating the use of sparse processing
     * of tasks within a worker.
     */
    const size_t WORK_PARTITIONING_SPARSE   = 100;
    
    /**
     * \brief Hint indicating the use of a balanced processing
     * of tasks within a worker.
     */
    const size_t WORK_PARTITIONING_BALANCED = 250;
    
    /**
     * \brief Hint indicating the use of heavy processing
     * of tasks within a worker.
     */
    const size_t WORK_PARTITIONING_HEAVY    = 500;
    
    /**
     * \brief Hint indicating the use of heavier processing
     * of tasks within a worker.
     */
    const size_t WORK_PARTITIONING_HEAVIER  = 2000;

    /**
     * \brief Type referring to the client consumer worker implementation.
     */
    using consumer_t = std::function<void()>;

    /**
     * \brief Type referring to a time value expressed in milliseconds.
     */
    using milliseconds_t = std::chrono::milliseconds::rep;

    /**
     * \struct parameterized_pool_t
     */
    template <
      size_t BULK_MAX_ITEMS = WORK_PARTITIONING_HEAVY,
      milliseconds_t DEQUEUE_TIMEOUT = 1 * 1000
    >
    struct parameterized_pool_t {
      
      /**
       * \constructor
       * \brief Creates a new thread pool and allocates `concurrency`
       * number of threads.
       */
      parameterized_pool_t(size_t concurrency)
        : tasks_(concurrency),
          producer_token_(tasks_),
          producers_execution_count_(0),
          done_(false) {
        while (concurrency--) {
          threads_.push_back(std::thread(&parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>::worker, this));
        }
      }

      /**
       * \destructor
       * \brief Will stop the running threads.
       */
      ~parameterized_pool_t() {
        stop();
      }

      /**
       * \brief A thread pool object is non-copyable.
       */
      parameterized_pool_t(const parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>&) = delete;

      /**
       * \brief A thread pool object is non-copyable.
       */
      parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>& operator=(const parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>&) = delete;

      /**
       * \brief Pushes data of type `Type_` on the internal
       * blocking queue used to dispatch work to the worker
       * threads.
       */
      template<class F, class... Args>
      std::future<typename std::result_of<F(Args...)>::type> schedule(F&& f, Args&&... args) {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> future = task->get_future();
        tasks_.enqueue(producer_token_, [task] () { (*task)(); });
        return (future);
      }

      /**
       * Same as `.schedule()`, except that this method does not allow clients
       * of the thread-pool to retrieve the result of their runnable. Use this
       * method if you do not need to explicitely get the result of your runnable,
       * and you want to avoid the performance overhead of it.
       */
      template<class F, class... Args>
      bool schedule_and_forget(F&& f, Args&&... args) {
        using return_type = typename std::result_of<F(Args...)>::type;
        std::function<return_type()> bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return (tasks_.enqueue(producer_token_, bound));
      }

      /**
       * \brief Schedules the execution of an array of runnable
       * amonst the available worker threads.
       * \return a true value if the schedule operation was
       * successful, false otherwise.
       */
      bool schedule_bulk(const consumer_t array[], size_t size) {
        return (tasks_.enqueue_bulk(producer_token_, array, size));
      }

      /**
       * \brief Blocks until every threads in the thread pool
       * have been terminated.
       */
      parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>& await() {
        for (std::thread& t : threads_) {
          t.join();
        }
        return (*this);
      }

      /**
       * \brief Stops the execution of the threads allocated
       * by the thread pool.
       */
      parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>& stop() {
        done_.store(true);
        return (*this);
      }

    private:

      /**
       * \brief Worker threads vector container.
       */
      std::vector<std::thread> threads_;

      /**
       * \brief Blocking concurrent queue used to store and dispatch work
       * amonst worker threads.
       */
      moodycamel::BlockingConcurrentQueue<consumer_t> tasks_;

      /**
       * \brief The producer token used by the thread pool when
       * enqueuing new runnable in the queue.
       */
      moodycamel::ProducerToken producer_token_;

      /**
       * \brief States whether the execution of worker threads
       * should continue.
       */
      std::atomic<bool> done_;

      const std::function<void(parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>*)> consumer_;

      std::atomic<int> producers_execution_count_;

      std::unordered_map<int, const consumer_t> set_;

      /**
       * \brief Internal worker dispatching work to the given
       * consumer worker implementation.
       */
      void worker() {
        moodycamel::ConsumerToken token(tasks_);
        consumer_t runnable[BULK_MAX_ITEMS] = {};
        while (!done_) {
          auto available = tasks_.wait_dequeue_bulk_timed(token, runnable, BULK_MAX_ITEMS, std::chrono::milliseconds(DEQUEUE_TIMEOUT));
          if (available) {
            for (size_t i = 0; i < available; ++i) {
              //producers_execution_count_.fetch_add(1, std::memory_order_release);
              /*if (set_.find(producers_execution_count_) != set_.end()) {
                set_[producers_execution_count_]();
              }*/
              runnable[i]();
            }
          }
        }
      }
    };

    typedef parameterized_pool_t<> pool_t;
  };
};

#endif // THREAD_POOL_H_
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <vector>
#include <functional>
#include <thread>
#include <future>
#include <type_traits>
#include <unordered_map>

#include "blocking_concurrent_queue.hpp"

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
     * \brief Alias type to a `moodycamel::ProducerToken`.
     */
    using producer_token_t = moodycamel::ProducerToken;

    /**
     * \brief Alias type to a `moodycamel::ConsumerToken`.
     */
    using consumer_token_t = moodycamel::ConsumerToken;

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
          done_(false) {
        while (concurrency--) {
          threads_.push_back(std::thread(&parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>::worker, this));
        }
      }

      /**
       * \destructor
       * \brief Will stop the running threads and await for
       * them to have completed. The destructor will catch
       * potential exceptions arising from the fact that
       * internal threads may have already been terminated.
       */
      ~parameterized_pool_t() noexcept {
        try {
          stop().await();
        } catch (std::system_error&) {}
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
      std::future<typename std::result_of<F(Args...)>::type> schedule(const moodycamel::ProducerToken& token, F&& f, Args&&... args) {
        using return_type = typename std::result_of<F(Args...)>::type;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
          std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<return_type> future = task->get_future();
        if (!tasks_.enqueue(token, [task] () { (*task)(); })) {
          throw std::length_error("Couldn't enqueue the given callable object");
        }
        return (future);
      }

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
        if (!tasks_.enqueue([task] () { (*task)(); })) {
          throw std::length_error("Couldn't enqueue the given callable object");
        }
        return (future);
      }

      /**
       * Same as `.schedule()`, except that this method does not allow clients
       * of the thread-pool to retrieve the result of their runnable. Use this
       * method if you do not need to explicitely get the result of your runnable,
       * and you want to avoid the performance overhead of it.
       */
      template<class F, class... Args>
      bool schedule_and_forget(const moodycamel::ProducerToken& token, F&& f, Args&&... args) noexcept {
        using return_type = typename std::result_of<F(Args...)>::type;
        std::function<return_type()> bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return (tasks_.enqueue(token, bound));
      }

      /**
       * Same as `.schedule()`, except that this method does not allow clients
       * of the thread-pool to retrieve the result of their runnable. Use this
       * method if you do not need to explicitely get the result of your runnable,
       * and you want to avoid the performance overhead of it.
       */
      template<class F, class... Args>
      bool schedule_and_forget(F&& f, Args&&... args) noexcept {
        using return_type = typename std::result_of<F(Args...)>::type;
        std::function<return_type()> bound = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        return (tasks_.enqueue(bound));
      }

      /**
       * \brief Schedules the execution of an array of runnable
       * amonst the available worker threads.
       * \return a true value if the schedule operation was
       * successful, false otherwise.
       */
      bool schedule_bulk(const moodycamel::ProducerToken& token, const consumer_t array[], size_t size) noexcept {
        return (tasks_.enqueue_bulk(token, array, size));
      }

      /**
       * \brief Schedules the execution of an array of runnable
       * amonst the available worker threads.
       * \return a true value if the schedule operation was
       * successful, false otherwise.
       */
      bool schedule_bulk(const consumer_t array[], size_t size) noexcept {
        return (tasks_.enqueue_bulk(array, size));
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
      parameterized_pool_t<BULK_MAX_ITEMS, DEQUEUE_TIMEOUT>& stop() noexcept {
        done_.store(true);
        return (*this);
      }

      /**
       * \brief Creates a new producer token associated with
       * the internal queue.
       */
      template <typename T>
      typename std::enable_if<std::is_same<T, producer_token_t>::value, producer_token_t>::type
      create_token_of() {
        return (T(tasks_));
      }

      /**
       * \brief Creates a new consumer token associated with
       * the internal queue.
       */
      template <typename T>
      typename std::enable_if<std::is_same<T, consumer_token_t>::value, consumer_token_t>::type
      create_token_of() {
        return (T(tasks_));
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
       * \brief States whether the execution of worker threads
       * should continue.
       */
      std::atomic<bool> done_;

      /**
       * \brief Internal worker dispatching work to the given
       * consumer worker implementation.
       */
      void worker() {
        auto token = create_token_of<thread::pool::consumer_token_t>();
        while (!done_) {
          consumer_t runnable[BULK_MAX_ITEMS] = {};
          auto available = tasks_.wait_dequeue_bulk_timed(token, runnable, BULK_MAX_ITEMS, std::chrono::milliseconds(DEQUEUE_TIMEOUT));
          for (size_t i = 0; i < available; ++i) {
            runnable[i]();
          }
        }
      }
    };

    /**
     * \brief The `pool_t` type is an alias to the `parameterized_pool_t<>` type
     * which uses default values for template parameters.
     */
    typedef parameterized_pool_t<> pool_t;
  };
};

#endif // THREAD_POOL_H_
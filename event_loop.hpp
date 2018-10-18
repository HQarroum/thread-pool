#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <vector>
#include <functional>
#include <thread>
#include <type_traits>

#include "BlockingCollection.hpp"

template<typename Type_> class event_loop_t {

  /**
   * \brief Type referring to the client consumer worker implementation.
   */
  using consumer_t = std::function<void(const Type_&)>;

  using default_value_t = std::function<Type_()>;

  /**
   * \brief Worker threads vector container.
   */
  std::vector<std::thread> threads_;

  /**
   * \brief Blocking concurrent queue used to store and dispatch work
   * amonst worker threads.
   */
  code_machina::BlockingCollection<Type_> tasks_;

  /**
   * \brief Client consumer worker type.
   */
  consumer_t consumer_;

  /**
   * \brief States whether the execution of worker threads
   * should continue.
   */
  std::atomic<bool> done_;

  default_value_t value_;

  /**
   * \brief Internal worker dispatching work to the given
   * consumer worker implementation.
   */
  template<class Q = void>
  typename std::enable_if<std::is_default_constructible<Type_>::value, Q>::type
  worker() {
    while (!done_) {
      Type_ data;
      if (tasks_.take(data) == code_machina::BlockingCollectionStatus::Ok) {
        consumer_(data);
      }
    }
  }

  /**
   * \brief Internal worker dispatching work to the given
   * consumer worker implementation.
   */
  template<class Q = void>
  typename std::enable_if<!std::is_default_constructible<Type_>::value, Q>::type
  worker() {
    while (!done_) {
      if (!value_) {
        throw std::invalid_argument("No value definition constructor defined for non default-constructible type");
      }
      Type_ data = value_();
      if (tasks_.take(data) == code_machina::BlockingCollectionStatus::Ok) {
        consumer_(data);
      }
    }
  }

public:

  /**
   * \constructor
   * \brief Creates a new thread pool and allocates `concurrency`
   * number of threads.
   */
  event_loop_t(consumer_t consumer, size_t concurrency, default_value_t value)
    : tasks_(100), consumer_(consumer), done_(false), value_(value) {
    while (concurrency--) {
      threads_.push_back(std::thread(&event_loop_t<Type_>::worker<void>, this));
    }
  }

  /**
   * \constructor
   * \brief Creates a new thread pool and allocates `concurrency`
   * number of threads.
   */
  event_loop_t(consumer_t consumer, size_t concurrency)
    : tasks_(100), consumer_(consumer), done_(false) {
    while (concurrency--) {
      threads_.push_back(std::thread(&event_loop_t<Type_>::worker<void>, this));
    }
  }

  /**
   * \brief Pushes data of type `Type_` on the internal
   * blocking queue used to dispatch work to the worker
   * threads.
   */
  void push(const Type_& data) {
    tasks_.add(data);
  }

  /**
   * \brief Blocks until every threads in the thread pool
   * have been terminated.
   */
  void await() {
    for (std::thread& t : threads_) {
      t.join();
    }
  }

  /**
   * \brief Stops the execution of the threads allocated
   * by the thread pool.
   */
  void stop() {
    done_ = true;
    tasks_.complete_adding();
  }
};

#endif // THREAD_POOL_H_
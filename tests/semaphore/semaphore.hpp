#include <mutex>
#include <condition_variable>

/**
 * Simple semaphore wrapper class.
 */
namespace thread {
  
  namespace pool {

    class semaphore_t {
      std::mutex mutex_;
      std::condition_variable condition_;
      int count_;
      
    public:

      /**
      * \constructor
      * Initializaes the counter.
      */
      semaphore_t(int count = 0) : count_{count} {}

      /**
      * Notifies one listening thread of a count change.
      */
      void notify() {
        std::unique_lock<std::mutex> guard(mutex_);
        --count_;
        condition_.notify_one();
      }

      /**
      * Awaits until the counter is set to zero.
      */
      void wait() {
        std::unique_lock<std::mutex> guard(mutex_);
        while (count_ > 0) {
          condition_.wait(guard);
        }
      }
    };
  };
};
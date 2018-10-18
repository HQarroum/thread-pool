#include <iostream>
#include "../thread_pool.hpp"
#include "../thread_pool_callable.hpp"

std::mutex mutex;

/**
 * \brief Concurrency level is calculated based on the CPU cores.
 */
static const size_t concurrency = std::thread::hardware_concurrency() + 1;

/**
 * \brief Basic worker thread implementation.
 */
static auto worker = [] (const std::string& foo) {
  std::cout << "Received " << foo << std::endl;
};

/**
 * \brief Static void function declaration.
 */
void static_void_function(const std::string& s) {
  std::lock_guard<std::mutex> lock_guard(mutex);
  std::cout << "[*] static_void_function called with value : " << s << std::endl;
}

/**
 * \brief Static int function declaration.
 */
int static_int_function(int foo) {
  std::lock_guard<std::mutex> lock_guard(mutex);
  std::cout << "[*] static_int_function called with value : " << foo << std::endl;
  return (42);
}

/**
 * \brief Application entry point.
 */
int main(int argc, char* argv[]) {
  thread::pool::pool_t pool(concurrency);

  // Schedule a `local_lambda` test.
  auto local_lambda = [] (const std::string& goo) {
    std::lock_guard<std::mutex> lock_guard(mutex);
    std::cout << "[*] local_lambda called with value " << goo << std::endl;
    return (1);
  };
  auto local_lambda_result = pool.schedule(local_lambda, "hello_local_lambda");
  //std::cout << " [+] `local_lambda` result : " << local_lambda_result.get() << std::endl;
  

  // Bind on `static_void_function` test.
  auto bind_static_void_function = thread::pool::bind(pool, static_void_function);
  bind_static_void_function("hello_static_void_function");
  

  // Bind on `bind_static_int_function` test.
  auto bind_static_int_function = thread::pool::bind(pool, static_int_function);
  auto static_int_function_result = bind_static_int_function(42);
  //std::cout << " [+] static_int_function result : " << static_int_function_result.get() << std::endl;


  // Bind on anonymous void lambda test.
  auto bind_anonymous_void_lambda = thread::pool::bind(pool, [] () {
    std::lock_guard<std::mutex> lock_guard(mutex);
    std::cout << "[*] Anonymous (void) lambda called" << std::endl;
  });
  bind_anonymous_void_lambda();


  // Bind on anonymous int lambda test.
  auto bind_anonymous_int_lambda = thread::pool::bind<int>(pool, [] (int value) {
    std::lock_guard<std::mutex> lock_guard(mutex);
    std::cout << "[*] Anonymous (int) lambda called with value : " << value << std::endl;
  });
  bind_anonymous_int_lambda(0xFF);


  // Bind on anonymous sum argument lambda test.
  auto bind_anonymous_sum_lambda = thread::pool::bind<int, double>(pool, [] (int a, double b) -> int {
    std::lock_guard<std::mutex> lock_guard(mutex);
    std::cout << "[*] Anonymous (sum) lambda called with values " << a << " and " << b << std::endl;
    return (a + b);
  });
  auto anonymous_sum_lambda_result = bind_anonymous_sum_lambda(1, 2);
  //std::cout << " [+] Anonymous (sum) lambda result : " << anonymous_sum_lambda_result.get() << std::endl;

  // Stopping the pool.
  pool.stop();

  // Waiting for the workers to finish.
  pool.await();
  
  /*loop.on("job.done", [] (const std::string& foo) {
    std::cout << foo << std::endl;
  });
  // Awaits for the execution to be finished.
  looper.loop();*/
  return (EXIT_SUCCESS);
}

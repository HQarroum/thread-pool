#include <iostream>
#include <cstring>
#include "../../includes/thread_pool.hpp"
#include "../../includes/thread_pool_callable.hpp"
#include "../../common/executor/async_executor.hpp"

/**
 * \brief Concurrency level is calculated based on the CPU cores.
 */
static const size_t concurrency = std::thread::hardware_concurrency() + 1;

/**
 * \brief An instance of the asynchronous logger used by workers
 * to output logs in a thread-safe manner.
 */
static async_executor_t executor;

/**
 * \brief Basic worker thread implementation.
 */
static auto static_lambda = [] (const std::string& foo) {
  executor.execute_async([foo] () {
    std::cout << "[*] static_lambda called with value : " << foo << std::endl;
  });
  return (foo);
};

/**
 * \brief Static void function declaration.
 */
void static_void_function(const std::string& s) {
  executor.execute_async([s] () {
    std::cout << "[*] static_void_function called with value : " << s << std::endl;
  });
}

/**
 * \brief Static int function declaration.
 */
int static_int_function(int foo) {
  executor.execute_async([foo] () {
    std::cout << "[*] static_int_function called with value : " << foo << std::endl;
  });
  return (42);
}

/**
 * \brief Application entry point.
 */
int main() {
  thread::pool::pool_t pool(concurrency);

  // Schedule a `static_lambda` test.
  auto bound_static_lambda_input = "hello_static_lambda";
  auto bound_static_lambda_result = pool.schedule(static_lambda, bound_static_lambda_input).get();
  std::cout << "[+] `static_lambda` result : " << bound_static_lambda_result << std::endl;
  assert(bound_static_lambda_result == bound_static_lambda_input);

  
  // Schedule a `local_lambda` test.
  auto local_lambda_function_input = "hello_local_lambda";
  auto local_lambda = [&local_lambda_function_input] (const std::string& foo) {
    assert(foo == local_lambda_function_input);
    executor.execute_async([foo] () {
      std::cout << "[*] local_lambda called with value " << foo << std::endl;
    });
    return (foo.length());
  };
  auto local_lambda_result = pool.schedule(local_lambda, local_lambda_function_input).get();
  std::cout << "[+] `local_lambda` result : " << local_lambda_result << std::endl;
  assert(local_lambda_result == ::strlen(local_lambda_function_input));
  

  // Schedule a `static_void_function` test.
  pool.schedule(static_void_function, "hello_static_void_function");
  

  // Schedule a `bound_static_int_function` test.
  auto bound_static_int_input = 42;
  auto static_int_function_result = pool.schedule(static_int_function, bound_static_int_input).get();
  std::cout << "[+] static_int_function result : " << static_int_function_result << std::endl;
  assert(bound_static_int_input == static_int_function_result);


  // Schedule a anonymous void lambda test.
  pool.schedule([] () {
    executor.execute_async([] () {
      std::cout << "[*] Anonymous (void) lambda called" << std::endl;
    });
  });


  // Schedule a anonymous int lambda test.
  pool.schedule([] (int value) {
    executor.execute_async([value] () {
      std::cout << "[*] Anonymous (int) lambda called with value : " << value << std::endl;
    });
  }, 0xFF);


  // Schedule a anonymous sum argument lambda test.
  auto anonymous_sum_lambda_result = pool.schedule([] (int a, double b) -> int {
    executor.execute_async([a, b] () {
      std::cout << "[*] Anonymous (sum) lambda called with values " << a << " and " << b << std::endl;
    });
    return (a + b);
  }, 1, 2).get();
  std::cout << "[+] Anonymous (sum) lambda result : " << anonymous_sum_lambda_result << std::endl;
  assert(anonymous_sum_lambda_result == 3);

  // Running the event loop.
  return (executor.run());
}

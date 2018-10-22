#include <iostream>
#include <cstring>
#include "../includes/thread_pool.hpp"
#include "../includes/thread_pool_callable.hpp"
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

  // Bind on a `static_lambda` test.
  auto bound_static_lambda_input = "hello_static_lambda";
  auto bound_static_lambda_function = thread::pool::bind<const std::string&>(pool, static_lambda);
  auto bound_static_lambda_result = bound_static_lambda_function(bound_static_lambda_input).get();
  std::cout << "[+] `static_lambda` result : " << bound_static_lambda_result << std::endl;
  assert(bound_static_lambda_result == bound_static_lambda_input);

  
  // Bind on a `local_lambda` test.
  auto local_lambda_function_input = "hello_local_lambda";
  auto local_lambda = [&local_lambda_function_input] (const std::string& foo) {
    assert(foo == local_lambda_function_input);
    executor.execute_async([foo] () {
      std::cout << "[*] local_lambda called with value " << foo << std::endl;
    });
    return (foo.length());
  };
  auto local_lambda_function = thread::pool::bind<const std::string&>(pool, local_lambda);
  auto local_lambda_result = local_lambda_function(local_lambda_function_input).get();
  std::cout << "[+] `local_lambda` result : " << local_lambda_result << std::endl;
  assert(local_lambda_result == ::strlen(local_lambda_function_input));
  

  // Bind on `static_void_function` test.
  auto bound_static_void_function = thread::pool::bind(pool, static_void_function);
  bound_static_void_function("hello_static_void_function");
  

  // Bind on `bound_static_int_function` test.
  auto bound_static_int_input = 42;
  auto bound_static_int_function = thread::pool::bind(pool, static_int_function);
  auto static_int_function_result = bound_static_int_function(bound_static_int_input).get();
  std::cout << "[+] static_int_function result : " << static_int_function_result << std::endl;
  assert(bound_static_int_input == static_int_function_result);


  // Bind on anonymous void lambda test.
  auto bound_anonymous_void_lambda = thread::pool::bind(pool, [] () {
    executor.execute_async([] () {
      std::cout << "[*] Anonymous (void) lambda called" << std::endl;
    });
  });
  bound_anonymous_void_lambda();


  // Bind on anonymous int lambda test.
  auto bound_anonymous_int_lambda = thread::pool::bind<int>(pool, [] (int value) {
    executor.execute_async([value] () {
      std::cout << "[*] Anonymous (int) lambda called with value : " << value << std::endl;
    });
  });
  bound_anonymous_int_lambda(0xFF);


  // Bind on anonymous sum argument lambda test.
  auto bound_anonymous_sum_lambda = thread::pool::bind<int, double>(pool, [] (int a, double b) -> int {
    executor.execute_async([a, b] () {
      std::cout << "[*] Anonymous (sum) lambda called with values " << a << " and " << b << std::endl;
    });
    return (a + b);
  });
  auto anonymous_sum_lambda_result = bound_anonymous_sum_lambda(1, 2).get();
  std::cout << "[+] Anonymous (sum) lambda result : " << anonymous_sum_lambda_result << std::endl;
  assert(anonymous_sum_lambda_result == 3);

  // Running the event loop.
  return (executor.run());
}

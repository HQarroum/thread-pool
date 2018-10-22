#ifndef CALLABLE_H_
#define CALLABLE_H_

#include <functional>
#include "thread_pool.hpp"

namespace thread {

  namespace pool {

     /**
      * \class callable_t
      * \brief Proxy class used for asynchronously invoking
      * a function which is scheduled on the given thread pool.
      */
      template <typename R, typename... Args>
      class callable_t {

        /**
        * Callable object with partial type.
        */
        std::function<R(Args...)> callable_;

        /**
        * \brief Local reference to the thread pool to which
        * functions calls should be proxied.
        */
        pool_t& pool_;

      public:

        /**
        * \constructor
        * \brief Creates an immutable `callable_t` object.
        */
        callable_t(pool_t& pool, std::function<R(Args...)> foo)
          : callable_(foo), pool_(pool) {}

        /**
        * \brief Schedules the function call on the bound
        * thread pool.
        */
        template <typename... FunctionArgs>
        std::future<R> operator()(FunctionArgs... args) {
          return (pool_.schedule(callable_, std::forward<FunctionArgs>(args)...));
        }
      };
            
      /**
       * \brief A traits utility structure used for type introspection purposes.
       */
      template<class T> struct traits;

      /**
       * \brief Specialization of the `traits` structure used for
       * introspection of type and return value of callables.
       */
      template<class C, class Ret, class... Args>
      struct traits<Ret(C::*)(Args...) const> {
        using return_value = Ret;
      };

      /**
       * \brief Binds a given pointer to function to a `pool_t` instance,
       * and returns a callable object which will dispatch the call on the
       * thread pool.
       */
      template <typename Return, typename ...Arguments>
      static callable_t<Return, Arguments...> bind(pool_t& pool, Return(*x)(Arguments...)) {
        return (callable_t<Return, Arguments...>(pool, x));
      }

      /**
       * \brief Binds a given movable callable to a `pool_t` instance,
       * and returns a callable object which will dispatch the call on the
       * thread pool.
       */
      template <typename... Params, typename L>
      static callable_t<
        typename traits<decltype(&L::operator())>::return_value,
        Params... 
      > bind(pool_t& pool, L&& lambda) {
        using return_t = typename traits<decltype(&L::operator())>::return_value;
        return (callable_t<return_t, Params...>(pool, std::forward<L>(lambda) ));
      }

      /**
       * \brief Binds a given callable reference to a `pool_t` instance,
       * and returns a callable object which will dispatch the call on the
       * thread pool.
       */
      template <typename... Params, typename L>
      static callable_t<
        typename traits<decltype(&L::operator())>::return_value,
        Params... 
      > bind(pool_t& pool, L& lambda) {
        using return_t = typename traits<decltype(&L::operator())>::return_value;
        return (callable_t<return_t, Params...>(pool, std::forward<L>(lambda)));
      }

      /**
       * \brief Binds a given `std::function` reference to a `pool_t` instance,
       * and returns a callable object which will dispatch the call on the
       * thread pool.
       */
      template <typename Return, typename ...Arguments>
      static callable_t<Return, Arguments...> bind(pool_t& pool, std::function<Return(Arguments...)>& x) {
        return (callable_t<Return, Arguments...>(pool, x));
      }
  };
};

#endif // CALLABLE_H_
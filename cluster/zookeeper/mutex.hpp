/**
 * Copyright (c) 2009 Carnegie Mellon University.
 *     All rights reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing,
 *  software distributed under the License is distributed on an "AS
 *  IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 *  express or implied.  See the License for the specific language
 *  governing permissions and limitations under the License.
 *
 * For more about this software visit:
 *
 *      http://www.graphlab.ml.cmu.edu
 *
 */

#ifndef GRAPHLAB_MUTEX_HPP
#define GRAPHLAB_MUTEX_HPP


#include <pthread.h>
#include "atomic_ops.hpp"
#include "is_pod.hpp"
#include <boost/type_traits/is_integral.hpp>

namespace lfgraph {
  
  template<typename T, bool IsIntegral>
  class atomic_impl {};
  /**
   * \internal
   * \brief atomic object
   * A templated class for creating atomic numbers.
   */
  template<typename T>
  class atomic_impl <T, true>: public IS_POD_TYPE {
  public:
    //! The current value of the atomic number
    volatile T value;
    
    //! Creates an atomic number with value "value"
    atomic_impl(const T& value = T()) : value(value) { }
    
    //! Performs an atomic increment by 1, returning the new value
    T inc() { return __sync_add_and_fetch(&value, 1);  }
    
    //! Performs an atomic decrement by 1, returning the new value
    T dec() { return __sync_sub_and_fetch(&value, 1);  }
    
    //! Lvalue implicit cast
    operator T() const { return value; }
    
    //! Performs an atomic increment by 1, returning the new value
    T operator++() { return inc(); }
    
    //! Performs an atomic decrement by 1, returning the new value
    T operator--() { return dec(); }
    
    //! Performs an atomic increment by 'val', returning the new value
    T inc(const T val) { return __sync_add_and_fetch(&value, val);  }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T dec(const T val) { return __sync_sub_and_fetch(&value, val);  }
    
    //! Performs an atomic increment by 'val', returning the new value
    T operator+=(const T val) { return inc(val); }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T operator-=(const T val) { return dec(val); }
    
    //! Performs an atomic increment by 1, returning the old value
    T inc_ret_last() { return __sync_fetch_and_add(&value, 1);  }
    
    //! Performs an atomic decrement by 1, returning the old value
    T dec_ret_last() { return __sync_fetch_and_sub(&value, 1);  }
    
    //! Performs an atomic increment by 1, returning the old value
    T operator++(int) { return inc_ret_last(); }
    
    //! Performs an atomic decrement by 1, returning the old value
    T operator--(int) { return dec_ret_last(); }
    
    //! Performs an atomic increment by 'val', returning the old value
    T inc_ret_last(const T val) { return __sync_fetch_and_add(&value, val);  }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T dec_ret_last(const T val) { return __sync_fetch_and_sub(&value, val);  }
    
    //! Performs an atomic exchange with 'val', returning the previous value
    T exchange(const T val) { return __sync_lock_test_and_set(&value, val);  }
  };
  
  // specialization for floats and doubles
  template<typename T>
  class atomic_impl <T, false>: public IS_POD_TYPE {
  public:
    //! The current value of the atomic number
    volatile T value;
    
    //! Creates an atomic number with value "value"
    atomic_impl(const T& value = T()) : value(value) { }
    
    //! Performs an atomic increment by 1, returning the new value
    T inc() { return inc(1);  }
    
    //! Performs an atomic decrement by 1, returning the new value
    T dec() { return dec(1);  }
    
    //! Lvalue implicit cast
    operator T() const { return value; }
    
    //! Performs an atomic increment by 1, returning the new value
    T operator++() { return inc(); }
    
    //! Performs an atomic decrement by 1, returning the new value
    T operator--() { return dec(); }
    
    //! Performs an atomic increment by 'val', returning the new value
    T inc(const T val) {
      T prev_value;
      T new_value;
      do {
        prev_value = value;
        new_value = prev_value + val;
      } while(!atomic_compare_and_swap(value, prev_value, new_value));
      return new_value;
    }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T dec(const T val) {
      T prev_value;
      T new_value;
      do {
        prev_value = value;
        new_value = prev_value - val;
      } while(!atomic_compare_and_swap(value, prev_value, new_value));
      return new_value;
    }
    
    //! Performs an atomic increment by 'val', returning the new value
    T operator+=(const T val) { return inc(val); }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T operator-=(const T val) { return dec(val); }
    
    //! Performs an atomic increment by 1, returning the old value
    T inc_ret_last() { return inc_ret_last(1);  }
    
    //! Performs an atomic decrement by 1, returning the old value
    T dec_ret_last() { return dec_ret_last(1);  }
    
    //! Performs an atomic increment by 1, returning the old value
    T operator++(int) { return inc_ret_last(); }
    
    //! Performs an atomic decrement by 1, returning the old value
    T operator--(int) { return dec_ret_last(); }
    
    //! Performs an atomic increment by 'val', returning the old value
    T inc_ret_last(const T val) {
      T prev_value;
      T new_value;
      do {
        prev_value = value;
        new_value = prev_value + val;
      } while(!atomic_compare_and_swap(value, prev_value, new_value));
      return prev_value;
    }
    
    //! Performs an atomic decrement by 'val', returning the new value
    T dec_ret_last(const T val) {
      T prev_value;
      T new_value;
      do {
        prev_value = value;
        new_value = prev_value - val;
      } while(!atomic_compare_and_swap(value, prev_value, new_value));
      return prev_value;
    }
    
    //! Performs an atomic exchange with 'val', returning the previous value
    T exchange(const T val) { return __sync_lock_test_and_set(&value, val);  }
  };
  
  template <typename T>
  class atomic: public atomic_impl<T, boost::is_integral<T>::value> {
  public:
    //! Creates an atomic number with value "value"
    atomic(const T& value = T()):
    atomic_impl<T, boost::is_integral<T>::value>(value) { }
    
  };
  
  /**
   * \ingroup util
   *
   * Simple wrapper around pthread's mutex.
   * Before you use, see \ref parallel_object_intricacies.
   */
  class mutex {
  public:
    // mutable not actually needed
    mutable pthread_mutex_t m_mut;
    /// constructs a mutex
    mutex() {
      int error = pthread_mutex_init(&m_mut, NULL);
      if (error) {
        exit(0);
      }
    }
    /** Copy constructor which does not copy. Do not use!
     Required for compatibility with some STL implementations (LLVM).
     which use the copy constructor for vector resize,
     rather than the standard constructor.    */
    mutex(const mutex&) {
      int error = pthread_mutex_init(&m_mut, NULL);
      if (error) {
        exit(0);
      }
    }
    
    ~mutex(){
      int error = pthread_mutex_destroy( &m_mut );
      if (error) {
        exit(0);
      }
    }
    
    // not copyable
    void operator=(const mutex& m) { }
    
    /// Acquires a lock on the mutex
    inline void lock() const {
      int error = pthread_mutex_lock( &m_mut  );
      // if (error) std::cout << "mutex.lock() error: " << error << std::endl;
      if (error) {
        exit(0);
      }
    }
    /// Releases a lock on the mutex
    inline void unlock() const {
      int error = pthread_mutex_unlock( &m_mut );
      if (error) {
        exit(0);
      }
    }
    /// Non-blocking attempt to acquire a lock on the mutex
    inline bool try_lock() const {
      return pthread_mutex_trylock( &m_mut ) == 0;
    }
    friend class conditional;
  }; // End of Mutex
  
  
  
  
  /**
   * \ingroup util
   *
   * Simple wrapper around pthread's recursive mutex.
   * Before you use, see \ref parallel_object_intricacies.
   */
  class recursive_mutex {
  public:
    // mutable not actually needed
    mutable pthread_mutex_t m_mut;
    /// constructs a mutex
    recursive_mutex() {
      pthread_mutexattr_t attr;
      int error = pthread_mutexattr_init(&attr);
      if (error) {
        exit(0);
      }
      error = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      if (error) {
        exit(0);
      }
      error = pthread_mutex_init(&m_mut, &attr);
      if (error) {
        exit(0);
      }
      pthread_mutexattr_destroy(&attr);
    }
    /** Copy constructor which does not copy. Do not use!
     Required for compatibility with some STL implementations (LLVM).
     which use the copy constructor for vector resize,
     rather than the standard constructor.    */
    recursive_mutex(const recursive_mutex&) {
      pthread_mutexattr_t attr;
      int error = pthread_mutexattr_init(&attr);
      if (error) {
        exit(0);
      }
      error = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      if (error) {
        exit(0);
      }
      error = pthread_mutex_init(&m_mut, &attr);
      if (error) {
        exit(0);
      }
      pthread_mutexattr_destroy(&attr);
    }
    
    ~recursive_mutex(){
      int error = pthread_mutex_destroy( &m_mut );
      if (error) {
        exit(0);
      }
    }
    
    // not copyable
    void operator=(const recursive_mutex& m) { }
    
    /// Acquires a lock on the mutex
    inline void lock() const {
      int error = pthread_mutex_lock( &m_mut  );
      // if (error) std::cout << "mutex.lock() error: " << error << std::endl;
      if (error) {
        exit(0);
      }
    }
    /// Releases a lock on the mutex
    inline void unlock() const {
      int error = pthread_mutex_unlock( &m_mut );
      if (error) {
        exit(0);
      }
    }
    /// Non-blocking attempt to acquire a lock on the mutex
    inline bool try_lock() const {
      return pthread_mutex_trylock( &m_mut ) == 0;
    }
    friend class conditional;
  }; // End of Mutex
  
  /**
   * \ingroup util
   * Wrapper around pthread's condition variable
   *
   * Before you use, see \ref parallel_object_intricacies.
   */
  class conditional {
  private:
    mutable pthread_cond_t  m_cond;
    
  public:
    conditional() {
      int error = pthread_cond_init(&m_cond, NULL);
      //ASSERT_TRUE(!error);
      if (error) {
        exit(0);
      }
    }
    
    /** Copy constructor which does not copy. Do not use!
     Required for compatibility with some STL implementations (LLVM).
     which use the copy constructor for vector resize,
     rather than the standard constructor.    */
    conditional(const conditional &) {
      int error = pthread_cond_init(&m_cond, NULL);
      if (error) {
        exit(0);
      }
    }
    
    // not copyable
    void operator=(const conditional& m) { }
    
    
    /// Waits on condition. The mutex must already be acquired. Caller
    /// must be careful about spurious wakes.
    inline void wait(const mutex& mut) const {
      int error = pthread_cond_wait(&m_cond, &mut.m_mut);
      if (error) {
        exit(0);
      }
    }
    /// Like wait() but with a time limit of "sec" seconds
    inline int timedwait(const mutex& mut, size_t sec) const {
      struct timespec timeout;
      struct timeval tv;
      struct timezone tz;
      gettimeofday(&tv, &tz);
      timeout.tv_nsec = tv.tv_usec * 1000;
      timeout.tv_sec = tv.tv_sec + (time_t)sec;
      return pthread_cond_timedwait(&m_cond, &mut.m_mut, &timeout);
    }
    /// Like wait() but with a time limit of "ms" milliseconds
    inline int timedwait_ms(const mutex& mut, size_t ms) const {
      struct timespec timeout;
      struct timeval tv;
      gettimeofday(&tv, NULL);
      // convert ms to s and ns
      size_t s = ms / 1000;
      ms = ms % 1000;
      size_t ns = ms * 1000000;
      // convert timeval to timespec
      timeout.tv_nsec = tv.tv_usec * 1000;
      timeout.tv_sec = tv.tv_sec;
      
      // add the time
      timeout.tv_nsec += (suseconds_t)ns;
      timeout.tv_sec += (time_t)s;
      // shift the nsec to sec if overflow
      if (timeout.tv_nsec > 1000000000) {
        timeout.tv_sec ++;
        timeout.tv_nsec -= 1000000000;
      }
      return pthread_cond_timedwait(&m_cond, &mut.m_mut, &timeout);
    }
    /// Like wait() but with a time limit of "ns" nanoseconds
    inline int timedwait_ns(const mutex& mut, size_t ns) const {
      struct timespec timeout;
      struct timeval tv;
      gettimeofday(&tv, NULL);
      assert(ns > 0);
      // convert ns to s and ns
      size_t s = ns / 1000000;
      ns = ns % 1000000;
      
      // convert timeval to timespec
      timeout.tv_nsec = tv.tv_usec * 1000;
      timeout.tv_sec = tv.tv_sec;
      
      // add the time
      timeout.tv_nsec += (suseconds_t)ns;
      timeout.tv_sec += (time_t)s;
      // shift the nsec to sec if overflow
      if (timeout.tv_nsec > 1000000000) {
        timeout.tv_sec ++;
        timeout.tv_nsec -= 1000000000;
      }
      return pthread_cond_timedwait(&m_cond, &mut.m_mut, &timeout);
    }
    /// Signals one waiting thread to wake up
    inline void signal() const {
      int error = pthread_cond_signal(&m_cond);
      if (error) {
        exit(0);
      }
    }
    /// Wakes up all waiting threads
    inline void broadcast() const {
      int error = pthread_cond_broadcast(&m_cond);
      if (error) {
        exit(0);
      }
    }
    ~conditional() {
      int error = pthread_cond_destroy(&m_cond);
      if (error) {
        exit(0);
      }
    }
  }; // End conditional
}

#endif

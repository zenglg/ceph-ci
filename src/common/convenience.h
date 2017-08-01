// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
// vim: ts=8 sw=2 smarttab
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */

#include <mutex>
#include <memory>
#include <type_traits>
#include <utility>

#include <boost/thread/shared_mutex.hpp>

#include "common/backport14.h"
#include "common/shunique_lock.h"

#include "include/assert.h" // I despise you. Not you the reader, I'm talking
                            // to the include file.


#ifndef CEPH_COMMON_CONVENIENCE_H
#define CEPH_COMMON_CONVENIENCE_H

namespace ceph {

// Lock Factories
// ==============
//
// I used to, whenever I declared a mutex member variable of a class,
// declare a pile of types like:
// ```cpp
// using unique_lock = ::std::unique_lock<decltype(membermutex)>;
// ```
// to avoid having to type that big, long type at every use. It also
// let me change the mutex type later. It's inelegant and breaks down
// if you have more than one type of mutex in the same class. So here
// are some lock factories.
template<typename Mutex, typename ...Args>
inline auto uniquely_lock(Mutex&& m, Args&& ...args)
  -> std::unique_lock<remove_reference_t<Mutex> > {
  return std::unique_lock<remove_reference_t<Mutex> >(
    std::forward<Mutex>(m), std::forward<Args>(args)... );
}

template<typename Mutex, typename ...Args>
inline auto sharingly_lock(Mutex&& m, Args&& ...args)
  -> boost::shared_lock<remove_reference_t<Mutex> > {
  return
    boost::shared_lock<remove_reference_t<Mutex> >(
      std::forward<Mutex>(m), std::forward<Args>(args)...);
}

template<typename Mutex, typename ...Args>
inline auto shuniquely_lock(std::unique_lock<Mutex>&& m, Args&& ...args)
  -> shunique_lock<remove_reference_t<Mutex> > {
  return shunique_lock<remove_reference_t<Mutex> >(
    std::forward<std::unique_lock<Mutex> >(m), std::forward<Args>(args)...);
}

template<typename Mutex, typename ...Args>
inline auto shuniquely_lock(boost::shared_lock<Mutex>&& m, Args&& ...args)
  -> shunique_lock<remove_reference_t<Mutex> > {
  return shunique_lock<remove_reference_t<Mutex> >(
    std::forward<boost::shared_lock<Mutex> >(m),
    std::forward<Args>(args)...);
}

template<typename Mutex, typename ...Args>
inline auto shuniquely_lock(Mutex&& m, Args&& ...args)
  -> shunique_lock<remove_reference_t<Mutex> > {
  return shunique_lock<remove_reference_t<Mutex> >(
    std::forward<Mutex>(m), std::forward<Args>(args)...);
}

// All right! These two don't work like the others. You cannot do
// `auto l = guardedly_lock(m)` since copy elision before C++17 is
// optional. C++17 makes it mandatory so these workarounds won't be
// needed.
//
// To use this, you'll need to do something like:
// `auto&& l = guardedly_lock(m)`
// This way, we aren't actually copying or moving a value, we're
// binding a reference to a temporary which extends its lifetime until
// the end of the enclosing block.
//
// You may in fact want to use
// `[[gnu::unused]] auto&& l = guardedly_lock(m)`
// To avoid the unused variable warning. Since reference assignment
// doesn't have side effects, normally, this just looks to the
// compiler (whose static analysis is not the sharpest hammer in the
// drawer) like a reference we bind for no reason and never
// use. Perhaps future compilers will be smarter about this, but since
// they'll also implement C++17 people might not care.
//

template<typename Mutex>
inline auto guardedly_lock(Mutex&& m)
  -> std::lock_guard<remove_reference_t<Mutex> > {
  m.lock();
  // So the way this works is that Copy List Initialization creates
  // one and only one Temporary. There is no implicit copy that is
  // generally optimized away the way there is if we were to just try
  // something like `return std::lock_guard<Mutex>(m)`.
  //
  // The function then returns this temporary as a prvalue. We cannot
  // bind it to a variable, because that would implicitly copy it
  // (even if in practice RVO would mean there is no copy), so instead
  // the user can bind it to a reference. (It has to be either a const
  // lvalue reference or an rvalue reference.)
  //
  // So we get something analogous to all the others with a mildly
  // wonky syntax. The need to use [[gnu::unused]] is honestly the
  // worst part. It makes this construction unfortunately rather
  // long.
  return { std::forward<Mutex>(m), std::adopt_lock };
}

template<typename Mutex>
inline auto guardedly_lock(Mutex&& m, std::adopt_lock_t)
  -> std::lock_guard<remove_reference_t<Mutex> > {
  return { std::forward<Mutex>(m), std::adopt_lock };
}

template<typename Mutex, typename Fun, typename...Args>
inline auto with_unique_lock(Mutex&& mutex, Fun&& fun, Args&&... args)
  -> decltype(fun(std::forward<Args>(args)...)) {
  // Yes I know there's a lock guard inside and not a unique lock, but
  // the caller doesn't need to know or care about the internal
  // details, and the semantics are those of unique locking.
  [[gnu::unused]] auto&& l = guardedly_lock(std::forward<Mutex>(mutex));
  return std::forward<Fun>(fun)(std::forward<Args>(args)...);
}

template<typename Mutex, typename Fun, typename...Args>
inline auto with_shared_lock(Mutex&& mutex, Fun&& fun, Args&&... args)
  -> decltype(fun(std::forward<Args>(args)...)) {
  auto l = sharingly_lock(std::forward<Mutex>(mutex));
  return std::forward<Fun>(fun)(std::forward<Args>(args)...);
}
}

// Lock Types
// ----------
//
// Lock factories are nice, but you still have to type out a huge,
// obnoxious template type when declaring a function that takes or
// returns a lock class.
//
#define UNIQUE_LOCK_T(m) \
  ::std::unique_lock<ceph::remove_reference_t<decltype(m)>>
#define SHARED_LOCK_T(m) \
  ::std::shared_lock<ceph::remove_reference_t<decltype(m)>>
#define SHUNIQUE_LOCK_T(m) \
  ::ceph::shunique_lock<ceph::remove_reference_t<decltype(m)>>

#endif // CEPH_COMMON_CONVENIENCE_H

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2018 Vicente J. Botet Escriba
//
// The design of this class follows the design of std::optional as implemented in libc++

//===-------------------------- optional ----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef JASEL_EXPERIMENTAL_V3_HELPERS_DETAIL_HPP
#define JASEL_EXPERIMENTAL_V3_HELPERS_DETAIL_HPP

#include <experimental/fundamental/v2/config.hpp>

#include <type_traits>
#include <utility>

namespace std
{
namespace experimental
{
inline namespace fundamental_v3
{
template <class T>
struct is_trivially_destructible_or_void : is_trivially_destructible<T> {};
template <>
struct is_trivially_destructible_or_void<void> : true_type {};

template <class T>
struct is_trivially_default_constructible_or_void : is_trivially_default_constructible<T> {};
template <>
struct is_trivially_default_constructible_or_void<void> : true_type {};

template <class T>
struct is_trivially_copy_constructible_or_void : is_trivially_copy_constructible<T> {};
template <>
struct is_trivially_copy_constructible_or_void<void> : true_type {};

template <class T>
struct is_trivially_move_constructible_or_void : is_trivially_move_constructible<T> {};
template <>
struct is_trivially_move_constructible_or_void<void> : true_type {};

template <class T>
struct is_trivially_copy_assignable_or_void : is_trivially_copy_assignable<T> {};
template <>
struct is_trivially_copy_assignable_or_void<void> : true_type {};

template <class T>
struct is_trivially_move_assignable_or_void : is_trivially_move_assignable<T> {};
template <>
struct is_trivially_move_assignable_or_void<void> : true_type {};

template <class T>
struct is_copy_constructible_or_void : is_copy_constructible<T> {};
template <>
struct is_copy_constructible_or_void<void> : true_type {};

template <class T>
struct is_move_constructible_or_void : is_move_constructible<T> {};
template <>
struct is_move_constructible_or_void<void> : true_type {};

template <class T>
struct is_copy_assignable_or_void : is_copy_assignable<T> {};
template <>
struct is_copy_assignable_or_void<void> : true_type {};

template <class T>
struct is_move_assignable_or_void : is_move_assignable<T> {};
template <>
struct is_move_assignable_or_void<void> : true_type {};

template <class T>
struct is_default_constructible_or_void : is_default_constructible<T> {};
template <>
struct is_default_constructible_or_void<void> : true_type {};



struct uninitialized_t {};

namespace helpers_detail
{

struct check_implicit {
  template <class ...U>
  static constexpr bool enable_implicit() {
      return true;
  }

  template <class ...U>
  static constexpr bool enable_explicit() {
      return false;
  }
};

struct check_fail {
  template <class ...U>
  static constexpr bool enable_implicit() {
      return false;
  }

  template <class ...U>
  static constexpr bool enable_explicit() {
      return false;
  }
};
template <class T, class QualU>
struct check_constructible {
  template <class U>
  static constexpr bool enable_implicit() {
      return is_constructible<T, QualU>::value &&
             is_convertible<QualU, T>::value;
  }

  template <class U>
  static constexpr bool enable_explicit() {
      return is_constructible<T, QualU>::value &&
             !is_convertible<QualU, T>::value;
  }
  template <class U>
  static constexpr bool enable_assign() {
      return is_constructible<T, QualU>::value &&
             is_assignable<T&, QualU>::value;
  }
};

template <class T, class QualU>
using check_void_or_constructible = typename conditional<
    !is_void<T>::value,
    helpers_detail::check_constructible<T, QualU>,
    helpers_detail::check_implicit
>::type;

template <class T, class U, class QualU>
using check_diff_or_constructible = typename conditional<
    !is_same<T, U>::value,
    helpers_detail::check_constructible<T, QualU>,
    helpers_detail::check_fail
>::type;

template <bool Cnd, class T, class QualU>
using check_if_constructible_else_fail = typename conditional<Cnd,
    helpers_detail::check_constructible<T, QualU>,
    helpers_detail::check_fail
>::type;




template <class T, class E, class QualU, class QualG>
struct check_constructibles {
  template <class U, class G>
  static constexpr bool enable_implicit() {
      return is_constructible<T, QualU>::value && is_constructible<E, QualG>::value &&
             is_convertible<QualU, T>::value && is_convertible<QualG, E>::value;
  }

  template <class U, class G>
  static constexpr bool enable_explicit() {
      return is_constructible<T, QualU>::value && is_constructible<E, QualG>::value &&
             ! ( is_convertible<QualU, T>::value && is_convertible<QualG, E>::value );
  }
  template <class U, class G>
  static constexpr bool enable_assign() {
      return is_constructible<T, QualU>::value && is_constructible<E, QualG>::value &&
             is_assignable<T&, QualU>::value && is_assignable<E&, QualG>::value;
  }

};




template <class T, class E, class QualU, class QualG>
using check_void_or_constructibles = typename conditional<
    !is_void<T>::value,
    helpers_detail::check_constructibles<T, E, QualU, QualG>,
    helpers_detail::check_implicit
>::type;


template <class T, class E, class U, class G, class QualU, class QualG>
using check_diffs_or_constructibles = typename conditional<
    !is_same<T, U>::value || !is_same<E, G>::value,
    helpers_detail::check_constructibles<T, E, QualU, QualG>,
    helpers_detail::check_fail
>::type;



}
}
}
}
#endif // header

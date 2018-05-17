// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Copyright (C) 2017 Vicente J. Botet Escriba

#ifndef JASEL_FUNDAMENTAL_V3_STRONG_STRICT_BOOL_HPP
#define JASEL_FUNDAMENTAL_V3_STRONG_STRICT_BOOL_HPP

#include <experimental/fundamental/v3/strong/strong_type.hpp>
#include <experimental/fundamental/v3/strong/mixins/comparable.hpp>
#include <experimental/fundamental/v3/strong/mixins/logical.hpp>
#include <experimental/fundamental/v3/strong/mixins/hashable.hpp>
#include <experimental/fundamental/v3/strong/mixins/streamable.hpp>
#include <experimental/fundamental/v3/strong/mixins/ordinal.hpp>

#include <type_traits>
#include <functional>

namespace std
{
namespace experimental
{
inline namespace fundamental_v3
{

  /**
  `strict_bool` is a strongly type that wraps a bool and behaves like an `bool`
  The main goal is to be able to define strong bools that don't mix between them.
  The single conversion is the explicit conversion to bool type.
  This is a safe bool, in the sense that it doesn't allows conversions from integral, floating point and pointer types.
  In addition it forbids conversions between different strong bools types.

  As any tagged type, it makes the parameter more explicit.
  Example
  <code>
  using X = strict_bool<XTag>;
  using Y = strict_bool<YTag>;
  void f(X, Y);

  f(X(true), Y(false));

  </code>
  */
  // Do we need a Bool parameter? Which type will be a good candidate?
  // bool takes 4 bytes in some machines. Sometimes we want to use just 1 byte
  template <class Tag, class Bool = bool>
  struct strict_bool final
  : strong_type<strict_bool<Tag, Bool>, Bool>
  , mixin::comparable<strict_bool<Tag, Bool>>
  , mixin::explicit_convertible_to<strong_bool<Tag, Bool>, bool>
  , mixin::logical<strict_bool<Tag, Bool>>
  , mixin::hashable<strict_bool<Tag, Bool>>
  , mixin::streamable<strict_bool<Tag, Bool>>
  , mixin::ordinal<strict_bool<Tag, Bool>>
  {
      using base_type = strong_type<strict_bool<Tag, Bool>, Bool>;
      using base_type::base_type;

      // copy constructor/assignment default
      constexpr strict_bool() noexcept = default;
      // If Bool is not bool, we want an explicit conversion from bool but not Bool.
      constexpr explicit strict_bool (Bool) = delete;
      constexpr explicit strict_bool (bool b) : base_type(b) {}

      // unwanted conversions
      constexpr explicit strict_bool (int) = delete;
      constexpr explicit strict_bool (double) = delete;
      constexpr explicit strict_bool (void*) = delete;
      template <class R, class ... Args>
      constexpr explicit strict_bool (R(*)(Args...)) = delete;
      template <class C, class R, class ... Args>
      constexpr explicit strict_bool (R(C::*)(Args...)) = delete;

      // boolean operators
      // fixme do we need these && and || operators?
      // strict_bool is explicit convertible to bool and so && and || operator will force the conversion to bool
      // If we don't define them we need to be explicit to convert to strict_bool
      // If we define them, we need to be explicit to convert to bool.
      // todo do we need mixed boolean operators?
      // If yes, which would be the result. IMO, it should be bool.
      // If not, there will be a conversion to bool and the result is bool

  };

  template <class Tag>
  struct strict_bool<Tag, bool> final
    : strong_type<strict_bool<Tag, bool>, bool>
    , mixins<strict_bool<Tag, bool>,
          meta_mixin::comparable<>
        , meta_mixin::explicit_convertible_to<bool>
        , meta_mixin::logical<>
        , meta_mixin::hashable<>
        , meta_mixin::streamable<>
        , meta_mixin::ordinal<>
    >
  {
      using base_type = strong_type<strict_bool<Tag, bool>, bool>;
      using base_type::base_type;

      // copy constructor/assignment default
      constexpr strict_bool() noexcept = default;

      // unwanted conversions
      constexpr explicit strict_bool (int) = delete;
      constexpr explicit strict_bool (double) = delete;
      constexpr explicit strict_bool (void*) = delete;
      template <class R, class ... Args>
      constexpr explicit strict_bool (R(*)(Args...)) = delete;
      template <class C, class R, class ... Args>
      constexpr explicit strict_bool (R(C::*)(Args...)) = delete;

  };

#if __cplusplus <= 201402L || (! defined __clang__ && defined __GNUC__ && __GNUC__ <= 6)
  static_assert(std::is_pod<strict_bool<bool>>::value, "");
  static_assert(std::is_trivially_default_constructible<strict_bool<bool>>::value, "");
  static_assert(std::is_trivially_copyable<strict_bool<bool>>::value, "");
  static_assert(std::is_standard_layout<strict_bool<bool>>::value, "");
  static_assert(std::is_trivial<strict_bool<bool>>::value, "");
#endif

}
}

  template <class Tag, class UT>
  struct hash<experimental::strict_bool<Tag,UT>> :
    experimental::wrapping_hash<experimental::strict_bool<Tag,UT>> {};

}

#endif // header

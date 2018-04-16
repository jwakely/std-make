// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Copyright (C) 2017 Vicente J. Botet Escriba

#ifndef JASEL_FUNDAMENTAL_V3_STRONG_STRONG_INT_HPP
#define JASEL_FUNDAMENTAL_V3_STRONG_STRONG_INT_HPP

#include <experimental/fundamental/v3/strong/strong_type.hpp>
#include <experimental/fundamental/v3/strong/mixins/additive.hpp>
#include <experimental/fundamental/v3/strong/mixins/bitwise.hpp>
#include <experimental/fundamental/v3/strong/mixins/comparable.hpp>
#include <experimental/fundamental/v3/strong/mixins/integer_multiplicative.hpp>
#include <experimental/fundamental/v3/strong/mixins/hashable.hpp>
#include <experimental/fundamental/v3/strong/mixins/streamable.hpp>
#include <experimental/fundamental/v3/strong/mixins/ordinal.hpp>
#include <experimental/type_traits.hpp>
#include <experimental/fundamental/v2/config.hpp>

#include <cstdint>
#include <limits>
#include <functional>
#include <iosfwd>


namespace std
{
namespace experimental
{
inline namespace fundamental_v3
{

  /**
  `strong_integral` is a strongly type tagged by the tag @c Tag that wraps an integral type @c UT and behaves like an @c UT
  The main goal is to be able to define strong integers that don't mix between them.
  No implicit conversion to the underlying integer type is provided.

  Note that this is not a safe int, it just forbids the conversions between
  different strong integers types with non compatible tags.

  Example
  <code>
  using X = strong_integral<XTag, int>;
  using Y = strong_integral<YTag, int>;
  void f(X, Y);

  f(X(1), Y(1));

  </code>
  */
  template <class Tag, class UT>
  struct strong_integral;

  namespace mixin {
    template <class Tag, class UT1, class UT2>
    struct is_compatible_with<strong_integral<Tag, UT1>, strong_integral<Tag, UT2>> : true_type {};
  }

  template <class Tag, class UT>
  struct strong_integral final : strong_type<strong_integral<Tag, UT>, UT>, mixins<strong_integral<Tag, UT>
      , meta_mixin::additive_with_if<>
      , meta_mixin::bitwise_with_if<>
      , meta_mixin::comparable_with_if<>
      , meta_mixin::integer_multiplicative_with_if<>
      , meta_mixin::hashable<>
      , meta_mixin::streamable<>
      , meta_mixin::ordinal<>
  >
  {
      static_assert(is_integral<UT>::value, "UT must be integral");
      static_assert(! is_same<UT, bool>::value, "UT cannot be bool");

      using base_type = strong_type<strong_integral<Tag, UT>, UT>;
      using base_type::base_type;

  };

  /// @par Returns A strong int with the tag `Tag` wrapping the value `x`
  template <class Tag, class R>
  constexpr strong_integral<Tag, R> make_strong_int(R x)
  {
    return strong_integral<Tag, R>(x);
  }

  // < C++17 static_assert(std::is_pod<strong_integral<bool,int>>::value, "");
  static_assert(std::is_trivially_default_constructible<strong_integral<bool,int>>::value, "");
  static_assert(std::is_trivially_copyable<strong_integral<bool,int>>::value, "");
  static_assert(std::is_standard_layout<strong_integral<bool,int>>::value, "");
  static_assert(std::is_trivial<strong_integral<bool,int>>::value, "");
  static_assert(is_ordinal<strong_integral<bool,int>>::value, "");

  // fixme: Should integer be a strong int without tag?
  using integer = strong_integral<default_tag, int>;
  using integer8_t = strong_integral<default_tag, int8_t>;
  using uinteger8_t = strong_integral<default_tag, uint8_t>;
  using integer16_t = strong_integral<default_tag, int16_t>;
  using uinteger16_t = strong_integral<default_tag, uint16_t>;
  using integer32_t = strong_integral<default_tag, int32_t>;
  using uinteger32_t = strong_integral<default_tag, uint32_t>;
  using integer64_t = strong_integral<default_tag, int64_t>;
  using uinteger64_t = strong_integral<default_tag, uint64_t>;

}
}

  // fixme: Should experimental::strong_integral<Tag,UT> satisfy is_integral?

  /// Hash specialization forwarding to the hash of underlying type
  template <class Tag, class UT>
  struct hash<experimental::strong_integral<Tag,UT>>
    : experimental::wrapping_hash<experimental::strong_integral<Tag, UT>> {};

  /// numeric_limits specialization forwarding to the numeric_limits of underlying type
  template <class Tag, class UT>
  struct numeric_limits<experimental::strong_integral<Tag,UT>> : numeric_limits<UT> {  };


}
#endif // header

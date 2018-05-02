// Copyright (C) 2016 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/monad.hpp>
// <experimental/optional.hpp>


#include <boost/detail/lightweight_test.hpp>
#include <experimental/monad.hpp>
#include <experimental/optional.hpp>

std::experimental::optional<int> twice(int i) {
  if (i==0) return std::experimental::nullopt;
  return std::experimental::make_optional(2*i);
}

int main()
{
  namespace stde = std::experimental;

  static_assert(stde::is_nullable<stde::optional<int>>::value, "ERROR");
  static_assert(stde::is_type_constructible<stde::optional<int>>::value, "ERROR");
  static_assert(stde::is_monad<stde::optional<stde::_t>>::value, "ERROR");

  {
    stde::optional<int> x = stde::none<stde::optional>();
    stde::optional<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(! stde::has_value(y));
  }
  {
    int v=1;
    stde::optional<int> x = stde::make<stde::optional>(v);
    stde::optional<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(stde::has_value(y));
    BOOST_TEST(*y == 2);
  }
  {
    int v=0;
    stde::optional<int> x = stde::make<stde::optional>(v);
    stde::optional<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(! stde::has_value(y));
  }
  {
    int v=0;
    stde::optional<stde::optional<int>> x = stde::make<stde::optional>(v);
    stde::optional<int> y = stde::monad::unwrap(x);
    BOOST_TEST(stde::has_value(y));
    BOOST_TEST(*y == 0);
  }

  return ::boost::report_errors();
}

// Copyright (C) 2016 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/monad.hpp>
// <experimental/expected.hpp>

#include <experimental/expected.hpp>

#include <boost/detail/lightweight_test.hpp>

std::experimental::expected<int> twice(int i) {
  if (i==0) return std::experimental::make_unexpected(1);
  return std::experimental::make_expected(2*i);
}

int main()
{
#if __cplusplus >= 201402L
  namespace stde = std::experimental;

  static_assert(stde::is_type_constructible_v<stde::expected<int>>, "ERROR");
  static_assert(std::is_base_of<stde::monad::tag, stde::monad::traits<stde::expected<stde::_t>>> ::value, "ERROR");
  static_assert(stde::is_monad<stde::expected<stde::_t>>::value, "ERROR");

  {
    stde::expected<int> x = stde::make_unexpected(1);
    stde::expected<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(! y);
  }
  {
    int v=1;
    stde::expected<int> x = stde::make<stde::expected>(v);
    stde::expected<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(y);
    BOOST_TEST(*y == 2);
  }
  {
    int v=0;
    stde::expected<int> x = stde::make<stde::expected>(v);
    stde::expected<int> y = stde::monad::bind(x,twice);
    BOOST_TEST(! y);
  }
#endif

  return ::boost::report_errors();
}

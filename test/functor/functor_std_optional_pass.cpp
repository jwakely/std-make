// Copyright (C) 2014-2016 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/functor.hpp>
// <experimental/optional.hpp>

#include <boost/detail/lightweight_test.hpp>
#include <experimental/functor.hpp>
#include <experimental/optional.hpp>

int twice(int i) {
  return 2*i;
}

bool even(int i) {
  return i%2==0;
}
int main()
{
  namespace stde = std::experimental;

  static_assert(stde::is_nullable<stde::optional<int>>::value, "ERROR");
  static_assert(stde::is_type_constructible<stde::optional<int>>::value, "ERROR");
  static_assert(stde::is_functor<stde::optional<stde::_t>>::value, "ERROR");

  {
    stde::optional<int> x = stde::none<stde::optional>();
    BOOST_TEST(! x);
    BOOST_TEST(! stde::has_value(x));
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x == stde::none<stde::optional<stde::_t>>());

    stde::optional<int> y = stde::functor::transform(x, twice);
    BOOST_TEST(! stde::has_value(y));
  }
  {
    int v=1;
    stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);
    BOOST_TEST(x != stde::none());
    stde::optional<int> y = stde::functor::transform(x, twice);
    BOOST_TEST(*y == 2);
    x = 2;
    BOOST_TEST(*x == 2);
  }
  {
    int v=1;
    const stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);
    BOOST_TEST(x != stde::none());

    stde::optional<int> y = stde::functor::transform(x, twice);
    BOOST_TEST(*y == 2);
  }
  {
    int v=1;
    const stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);


    stde::optional<int> y = stde::functor::transform(x, twice);
    BOOST_TEST(*y == 2);
  }
  {
    int v=1;
    const stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);

    stde::optional<int> y = stde::functor::map(twice, x);
    BOOST_TEST(*y == 2);
  }
  {
    int v=1;
    stde::optional<int> x = stde::make_optional(v);

    stde::optional<int> y = stde::functor::adjust_if(x, even, twice);
    BOOST_TEST(*y == 1);
    *x=2;
    y = stde::functor::adjust_if(x, even, twice);
    std::cout << *y << std::endl;
    BOOST_TEST(*y == 4);

  }
  return ::boost::report_errors();
}

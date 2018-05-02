// Copyright (C) 2017 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/optionals.hpp>

// todo
// * add constexpr tests
// * add noexcept tests

#include <boost/detail/lightweight_test.hpp>
#include <experimental/optionals.hpp>

namespace stdex = std::experimental;

struct A
{
  constexpr A() :
      v(4)
  {
  }
  int v;
};
struct B
{
};

void f1(stdex::optionals<A, B> const& opt_a_b)
{
  BOOST_TEST(! (opt_a_b.template has_value<0>()));
  //stdex::optional_ref<0, const A> opt_a = stdex::get<0>(opt_a_b);
  auto opt_a = stdex::get<0>(opt_a_b);
  BOOST_TEST(!opt_a);
}

void f2(stdex::optionals<A, B> const& opt_a_b)
{
  //stdex::optional_ref<0, const A> opt_a = stdex::get<0>(opt_a_b);
  auto opt_a = stdex::get<0>(opt_a_b);
  BOOST_TEST(opt_a->v == 4);
}

int main()
{

  {
    constexpr stdex::optionals<A, B> opt_a_b;
    BOOST_TEST(! (opt_a_b.template has_value<0>()));
  }
  {
    stdex::optionals<A, B> opt_a_b;
    //stdex::optional_ref<A> opt_a = stdex::get<0>(opt_a_b);
    auto opt_a = stdex::get<0>(opt_a_b);
    BOOST_TEST(!opt_a);
    BOOST_TEST(opt_a == stdex::nullopt);
    BOOST_TEST(stdex::nullopt == opt_a);
    //stdex::optional_ref<1,B> opt_b = stdex::get<1>(opt_a_b);
    auto opt_b = stdex::get<1>(opt_a_b);
    BOOST_TEST(!opt_b);
  }
  {
    constexpr stdex::optionals<A, B> opt_a_b;
    //fixme
    //constexpr
    //stdex::optional_ref<0, const A> opt_a = stdex::get<0>(opt_a_b);
    auto opt_a = stdex::get<0>(opt_a_b);
    BOOST_TEST(!opt_a);
    BOOST_TEST(opt_a == stdex::nullopt);
    BOOST_TEST(stdex::nullopt == opt_a);
    //stdex::optional_ref<1, const B> opt_b = stdex::get<1>(opt_a_b);
    auto opt_b = stdex::get<1>(opt_a_b);
    BOOST_TEST(!opt_b);
  }
  {
    stdex::optionals<A, B> opt_a_b;
    //stdex::optional_ref<0, A> opt_a = stdex::get<0>(opt_a_b);
    auto opt_a = stdex::get<0>(opt_a_b);
    opt_a = A {};
    BOOST_TEST(opt_a);
    BOOST_TEST(opt_a != stdex::nullopt);
    BOOST_TEST(stdex::nullopt != opt_a);
    BOOST_TEST( (opt_a_b.template has_value<0>()));
    BOOST_TEST(opt_a->v == 4);
    BOOST_TEST((*opt_a).v == 4);

    opt_a = stdex::nullopt;
    BOOST_TEST(!opt_a);
    BOOST_TEST(! (opt_a_b.template has_value<0>()));
  }
  {
    stdex::optionals<int, B> opt_a_b;
    //stdex::optional_ref<0, int> opt_a = stdex::get<0>(opt_a_b);
    auto opt_a = stdex::get<0>(opt_a_b);
    opt_a = 1;
    BOOST_TEST(opt_a);
    BOOST_TEST( (opt_a_b.template has_value<0>()));
    BOOST_TEST(opt_a == 1);
    BOOST_TEST(opt_a > 0);
    BOOST_TEST(opt_a >= 0);
    BOOST_TEST(opt_a >= 1);
    BOOST_TEST(opt_a < 2);
    BOOST_TEST(opt_a <= 1);
    BOOST_TEST(opt_a <= 2);

    opt_a = stdex::nullopt;
    BOOST_TEST(!opt_a);
    BOOST_TEST(! (opt_a_b.template has_value<0>()));
  }
  {
    stdex::optionals<A, B> opts1;
    stdex::optionals<A, B> opts2;
    opts2 = opts1;
    BOOST_TEST(! (opts1.template has_value<0>()));
    BOOST_TEST(! (opts1.template has_value<1>()));
    BOOST_TEST(! (opts2.template has_value<0>()));
    BOOST_TEST(! (opts2.template has_value<1>()));
  }
  {
    stdex::optionals<A, B> opts1;
    //stdex::optional_ref<0, A> opt_a = stdex::get<0>(opts1);
    auto opt_a = stdex::get<0>(opts1);
    BOOST_TEST(! (opts1.template has_value<0>()));
    opt_a = A {};
    BOOST_TEST( (opts1.template has_value<0>()));
    BOOST_TEST(! (opts1.template has_value<1>()));
    stdex::optionals<A, B> opts2;
    opts2 = opts1;
    BOOST_TEST( (opts1.template has_value<0>()));
    BOOST_TEST(! (opts1.template has_value<1>()));
    BOOST_TEST( (opts2.template has_value<0>()));
    BOOST_TEST(! (opts2.template has_value<1>()));
  }
  {
    stdex::optionals<A, B> opts1;
    stdex::optionals<A, B> opts2;
    BOOST_TEST(! (opts1.template has_value<0>()));
    BOOST_TEST(! (opts2.template has_value<0>()));
    swap(opts1, opts2);
    BOOST_TEST(! (opts1.template has_value<0>()));
    BOOST_TEST(! (opts2.template has_value<0>()));
  }
  return ::boost::report_errors();
}


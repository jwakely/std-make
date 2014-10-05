// Copyright (C) 2014 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/make.hpp>

//  template <template <class ...> M, class X>
//  auto make(X&& x);
//  template <class M, class ...Args>
//  auto make(Args&& ...args);


#include <experimental/make.hpp>
#include <future>
#include <boost/detail/lightweight_test.hpp>

namespace std {

// customization point for template (needed because std::future don't uses experimental::in_place_t)
template <class X>
shared_future<typename std::decay<X>::type> make(std::experimental::tc<std::shared_future>, X&& x)
{
  typedef typename std::decay<X>::type value_type;
  promise<value_type> p;
  p.set_value(std::forward<X>(x));
  return p.get_future().share();
}

// customization point for template (needed because std::shared_future don't uses experimental::in_place_t)
template <class X, class ...Args>
shared_future<X> make(std::experimental::c<shared_future<X>>, Args&& ...args)
{
  typedef X value_type;
  promise<value_type> p;
  p.set_value(value_type(std::forward<Args>(args)...));
  return p.get_future().share();
}

// Holder specialization
template <>
struct shared_future<std::experimental::_t> {};

// customization point for holder
template <class X>
shared_future<typename std::decay<X>::type> make(std::experimental::c<shared_future<std::experimental::_t>>, X&& x)
{
  typedef typename std::decay<X>::type value_type;
  promise<value_type> p;
  p.set_value(std::forward<X>(x));
  return p.get_future().share();
}

}
struct A
{
  int v;
  A(): v(3) {}
  A(int v): v(std::move(v)) {}
  A(int v1, int v2): v(v1+v2) {}
};

int main()
{
  {
    int v=0;
    std::shared_future<int> x = std::experimental::make<std::shared_future>(v);
    BOOST_TEST(x.get() == 0);
  }
  {
    int v=0;
    std::shared_future<int> x = std::experimental::make<std::shared_future<int>>(v);
    BOOST_TEST(x.get() == 0);
  }
  {
    int v=1;
    std::shared_future<A> x = std::experimental::make<std::shared_future<A>>(v,v);
    BOOST_TEST(x.get().v == 2);
  }
  {
    std::shared_future<int> x = std::experimental::make<std::shared_future<int>>();
    BOOST_TEST_EQ(x.get(),  0);
  }
  {
    int v=0;
    std::shared_future<int&> x = std::experimental::make<std::shared_future<int&>>(v);
    BOOST_TEST(&v == &x.get());
  }
  {
    using namespace std::experimental;
    int v=0;
    std::shared_future<int> x = make<std::shared_future<_t>>(v);
    BOOST_TEST(x.get() == 0);
  }
  return ::boost::report_errors();
}

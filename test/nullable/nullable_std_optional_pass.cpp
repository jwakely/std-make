// Copyright (C) 2014-2016 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// <experimental/nullable.hpp>
// <experimental/optional.hpp>

//#define JASEL_STD_EXPERIMENTAL_FACTORIES_USE_OPTIONAL
//#define JASEL_FUNDAMENTAL_EXTENDED

#include <boost/detail/lightweight_test.hpp>
#include <experimental/optional.hpp>

std::experimental::nullopt_t null() { return std::experimental::nullopt;}

struct A
{
  int v;
  A(): v(3) {}
  A(int v_): v(std::move(v_)) {}
  A(int v1, int v2): v(v1+v2) {}
};

int twice(int i) {
  return 2*i;
}

std::experimental::optional<int> mtwice(int i) {
  return std::experimental::make_optional(2*i);
}

std::experimental::optional<double> inverse(double x) {
  if (x==0.0) return std::experimental::nullopt;
  return 1/x;
}

template <class T>
struct check;


//template<class T>
//T& g(T &) {
//  return std::experimental::deref(T(1));
//}
//
//template<class T>
//void g1(T &) {
//  auto xx = std::experimental::deref(T(1));
//  std::cout << xx << std::endl;
//
//}

//template<class T>
//T const& h(T const &x) {
//  return g(x);
//}
//template<class T>
//void h1(T const &x) {
//  g1(x);
//}

int res(std::experimental::nullopt_t) {
  return -1;
}


int main()
{
  namespace stde = std::experimental;

  static_assert(not stde::is_nullable<stde::none_t>::value, "ERROR");
  static_assert(stde::is_nullable<stde::optional<int>>::value, "ERROR");

  static_assert(std::is_same<
      stde::nullable::none_type_t<stde::optional<int>>,
      stde::nullopt_t
    >::value, "ERROR");

#if defined JASEL_FUNDAMENTAL_EXTENDED
  static_assert(std::is_same<stde::value_type_t<stde::optional<int&>>, int&>::value, "ERROR");
#endif
  {
    auto n = stde::none_t{};
    stde::optional<int> x(n);
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
    BOOST_TEST(stde::holds_alternative_value(x, stde::none()));
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x == stde::none<stde::optional<stde::_t>>());
    BOOST_TEST(stde::nullopt == stde::nullopt);

  }
  {
    stde::optional<stde::optional<int>> x = stde::none() ;
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
    BOOST_TEST(stde::holds_alternative_value(x, stde::none()));
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x == stde::none<stde::optional<stde::_t>>());
  }
  {
    stde::optional<stde::optional<int>> x { stde::none() };
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
    BOOST_TEST(stde::holds_alternative_value(x, stde::none()));
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x == stde::none<stde::optional<stde::_t>>());
  }
  {
    stde::optional<stde::optional<int>> x = stde::optional<int>(stde::none()) ;
    BOOST_TEST(x);
    BOOST_TEST(has_value(x));
    BOOST_TEST(stde::holds_alternative_value(x, stde::optional<int>(stde::none())));
    BOOST_TEST(x != stde::none());
    BOOST_TEST(stde::none() != x);
    BOOST_TEST(x != stde::none<stde::optional<stde::_t>>());

  }
  {
    stde::optional<stde::optional<int>> x { stde::optional<int>(stde::none()) };
    BOOST_TEST(x);
    BOOST_TEST(has_value(x));
    BOOST_TEST(stde::holds_alternative_value(x, stde::optional<int>(stde::none())));
  }

# if !defined __clang__ && defined __GNUC__ // NOTE: GNUC is also defined for Clang
#   if (__GNUC__ < 6)
  {
    stde::optional<stde::optional<int>> x { { stde::none() } };
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
#endif
#endif

#if defined __clang__
  {
    stde::optional<stde::optional<int>> x { { stde::none() } };
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
#endif


#if defined __clang__
  {
    stde::optional<stde::optional<int>> x { { null() } };
    BOOST_TEST( ! x);
    BOOST_TEST( ! has_value(x));
  }
  {
    stde::optional<stde::optional<int>> x { { stde::nullopt } };
    BOOST_TEST( ! x);
    BOOST_TEST( ! has_value(x));
  }
#endif
  {
    stde::optional<stde::optional<int>> x { stde::none<stde::optional>() };
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
  {
    stde::optional<stde::optional<int>> x = stde::none<stde::optional>() ;
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
  {
    stde::optional<stde::optional<int>> x = stde::none<stde::optional<stde::optional<int>> >() ;
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
  // fixme: do we want this to work. See https://github.com/viboes/std-make/issues/14
#if 0
  {
    stde::optional<stde::optional<int>> x = stde::none<stde::optional<int>>() ;
    BOOST_TEST(x);
    BOOST_TEST(has_value(x));
  }
#endif
  {
    stde::optional<stde::optional<int>> x { stde::none<stde::optional>() };
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }

#if defined __clang__
  {
    stde::optional<stde::optional<int>> x { { stde::none<stde::optional>() } };
    BOOST_TEST( ! x);
    BOOST_TEST( ! has_value(x));
  }
#endif
  {
    stde::optional<int> x = stde::none<stde::optional>();
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x == stde::none<stde::optional<stde::_t>>());

#if defined JASEL_FUNDAMENTAL_EXTENDED
    stde::optional<int> y = stde::fmap(twice, x);
    BOOST_TEST(! stde::has_value(y));
#endif
  }
  {
    int v=1;
    stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);
    BOOST_TEST(x != stde::none());
#if defined JASEL_FUNDAMENTAL_EXTENDED
    stde::optional<int> y = stde::fmap(twice, x);
#endif
    x = 2;

  }
  {
    int v=1;
    const stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 1);
    BOOST_TEST(x != stde::none());
    //h1(x);

#if defined JASEL_FUNDAMENTAL_EXTENDED
    stde::optional<int> y = stde::fmap(twice, x);
#endif

  }
  {
    int v=0;
    stde::optional<int> x = stde::make_optional(v);
    BOOST_TEST(*x == 0);
  }
  {

    int v=0;
    static_assert(std::is_same<stde::optional<int>, decltype(stde::make_optional(v))>::value, "a");
  }
#if 0
  {
    short v=0;
    stde::optional<int> x = stde::make_optional<int>(v);
    BOOST_TEST(*x == 0);
    BOOST_TEST(value(x) == 0);
  }
#endif
  static_assert(std::is_constructible<A,int, int>::value, "a");
  static_assert(std::is_constructible<A>::value, "b");

  static_assert(std::is_constructible<stde::optional<A>, stde::in_place_t,int, int>::value, "a");
  static_assert(std::is_constructible<stde::optional<A>, stde::in_place_t>::value, "b");

#if 0
  {
    int v=1;
    stde::optional<A> x = stde::make_optional<A>(stde::in_place, v,v);
    BOOST_TEST(x->v == 2);
  }
  {
    stde::optional<int> x = stde::make_optional<int>(stde::in_place);
    BOOST_TEST_EQ(*x,  0);
  }
#endif
  {
    stde::optional<int> o((stde::in_place));
    BOOST_TEST(o);
    BOOST_TEST(has_value(o));
    BOOST_TEST(stde::holds_alternative_value(o, 0));
  }
#if 0
  {
    stde::optional<int> o((stde::in_place));
    stde::optional<int> x = stde::make_optional<int>();
    BOOST_TEST(! x);
    BOOST_TEST(! has_value(x));
  }
#endif
  {
    int v=0;
    stde::optional<int&> x = stde::make_optional(std::ref(v));
    BOOST_TEST(&v == &x.value());
    BOOST_TEST(&v == std::addressof(x.value()));

  }
  {
    stde::optional<int> x;
    BOOST_TEST(x == stde::none());
    BOOST_TEST(stde::none() == x);
    BOOST_TEST(x <= stde::none());
    BOOST_TEST(stde::none() <= x);
    BOOST_TEST(x >= stde::none());
    BOOST_TEST(stde::none() >= x);
  }
  {
    stde::optional<int> x=stde::make_optional(1);
    BOOST_TEST(stde::holds_alternative_value(x, 1));
    BOOST_TEST(x != stde::none());
    BOOST_TEST(stde::none() != x);
    BOOST_TEST(x > stde::none());
    BOOST_TEST(stde::none() < x);
  }
  {
    stde::optional<int> x=stde::make_optional(1);
    BOOST_TEST(x != stde::none<stde::optional<int>>());
    BOOST_TEST(stde::holds_alternative_value(x, 1));
    BOOST_TEST(stde::none() != x);
    BOOST_TEST(x > stde::none());
    BOOST_TEST(stde::none() < x);
  }


  return ::boost::report_errors();
}


// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// (C) Copyright 2013,2014 Vicente J. Botet Escriba
// (C) Copyright 2013 Pierre Talbot

// todo
// * add noexcept
// * fix never empty implementation
// * adapt to std::optional (c++17)
// * pod::expected?
// * expected_view?


#ifndef JASEL_EXPERIMENTAL_V3_EXPECTED_EXPECTED_HPP
#define JASEL_EXPERIMENTAL_V3_EXPECTED_EXPECTED_HPP
#include <experimental/fundamental/v2/config.hpp>

#include <experimental/fundamental/v3/expected/config.hpp>
#include <experimental/fundamental/v3/expected/unexpected.hpp>
#include <experimental/utility.hpp>
#include <experimental/fundamental/v3/config/requires.hpp>
#include <experimental/fundamental/v3/expected/error_traits.hpp>
#include <experimental/fundamental/v3/expected/bad_expected_access.hpp>
#include <utility>
#include <experimental/fundamental/v3/in_place.hpp>
#include <experimental/type_constructible.hpp>
#include <experimental/meta/v1/rebind.hpp>
#include <experimental/meta/v1/id.hpp>
#include <experimental/fundamental/v3/result/success_failure.hpp>

#include <stdexcept>
#include <initializer_list>
#include <type_traits>
#include <memory>

namespace std
{
namespace experimental
{
inline namespace fundamental_v3
{

  namespace adl
  {
    using std::swap;
    template <class T, class U>
    void swap_impl(T& x, U& y) noexcept(noexcept(swap(x,y)))
    {
      swap(x,y);
    }
  }

struct expect_t {};
BOOST_CONSTEXPR_OR_CONST expect_t expect = {};

struct unexpect_t {};
BOOST_CONSTEXPR_OR_CONST unexpect_t unexpect = {};

namespace detail {

struct only_set_initialized_t{};
BOOST_CONSTEXPR_OR_CONST only_set_initialized_t only_set_initialized = {};

#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
template<class T, class E>
struct unrestricted_union_emulation_storage
{
  char _bytes[sizeof(T)>sizeof(E) ? sizeof(T) : sizeof(E)];
};
template<class E>
struct unrestricted_union_emulation_storage<void, E>
{
  char _bytes[sizeof(E)];
};
template<class T>
struct unrestricted_union_emulation_storage<T, void>
{
  char _bytes[sizeof(T)];
};
struct boost_expected_unrestricted_union_emulation_default_tag { };
template<class Base, class value_type, class error_type>
struct unrestricted_union_emulation_val_tag
{
  char *_bytes() { return static_cast<unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<Base *>(this))->_bytes; }
  const char *_bytes() const { return static_cast<const unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<const Base *>(this))->_bytes; }
  const value_type &val() const { return *reinterpret_cast<const value_type *>(_bytes()); }
  value_type &val() { return *reinterpret_cast<value_type *>(_bytes()); }
  unrestricted_union_emulation_val_tag() {}
  unrestricted_union_emulation_val_tag(boost_expected_unrestricted_union_emulation_default_tag)
  {
    ::new(&val()) value_type();
  }
  template<class Arg, class... Args> explicit unrestricted_union_emulation_val_tag(Arg&& arg, Args&&... args)
  {
    ::new(&val()) value_type(forward<Arg>(arg), forward<Args>(args)...);
  }
};
template<class Base, class value_type, class error_type>
struct unrestricted_union_emulation_err_tag
{
  using unexpected_t = unexpected<error_type>;
  char *_bytes() { return static_cast<unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<Base *>(this))->_bytes; }
  const char *_bytes() const { return static_cast<const unrestricted_union_emulation_storage<value_type, error_type> *>(static_cast<const Base *>(this))->_bytes; }
  const error_type &err() const { return *reinterpret_cast<const error_type *>(_bytes()); }
  error_type &err() { return *reinterpret_cast<error_type *>(_bytes()); }
  const unexpected_t &unexpected_() const { return *reinterpret_cast<const unexpected_t *>(_bytes()); }
  unexpected_t &unexpected_() { return *reinterpret_cast<unexpected_t *>(_bytes()); }
  unrestricted_union_emulation_err_tag() {}
  unrestricted_union_emulation_err_tag(boost_expected_unrestricted_union_emulation_default_tag)
  {
    ::new(&unexpected_()) unexpected_t();
  }
  template<class Arg, class... Args> explicit unrestricted_union_emulation_err_tag(Arg&& arg, Args&&... args)
  {
    ::new(&unexpected_()) unexpected_t(forward<Arg>(arg), forward<Args>(args)...);
  }
};
#else
#define boost_expected_unrestricted_union_emulation_default_tag(...) // NONLINT chances of collision are low
#endif // JASEL_NO_CXX11_UNRESTRICTED_UNIONS

template <class T, class E>
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
struct trivial_expected_storage
  : unrestricted_union_emulation_storage<T, E>,
  unrestricted_union_emulation_val_tag<trivial_expected_storage<T, E>, T, E>,
  unrestricted_union_emulation_err_tag<trivial_expected_storage<T, E>, T, E>
#else
union trivial_expected_storage
#endif
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<trivial_expected_storage<T, E>, T, E> _err;
  typedef unrestricted_union_emulation_val_tag<trivial_expected_storage<T, E>, T, E> _val;
#else
  value_type _val;
  unexpected_t _unexpected;
  constexpr const unexpected_t &unexpected_() const { return _unexpected; }
  unexpected_t &unexpected_() { return _unexpected; }
  constexpr const error_type &err() const { return unexpected_().value(); }
  error_type &err() { return unexpected_().value(); }
  constexpr const value_type &val() const { return _val; }
  value_type &val() { return _val; }
#endif

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr trivial_expected_storage()
    BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  constexpr explicit trivial_expected_storage(unexpected_t const& e)
  : _unexpected(e)
  {}

  constexpr explicit trivial_expected_storage(unexpected_t && e)
  : _unexpected(move(e))
  {}

  template <class Err>
  constexpr explicit trivial_expected_storage(unexpected<Err> const& e)
  : _unexpected(error_traits<error_type>::make_error(e.value()))
  {}

  constexpr explicit trivial_expected_storage(in_place_t)
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  template <class... Args>
  constexpr explicit trivial_expected_storage(in_place_t, Args&&... args)
  : _val(constexpr_forward<Args>(args)...)
  {}

  trivial_expected_storage(trivial_expected_storage const&) = default;
  trivial_expected_storage(trivial_expected_storage &&)  = default;
  trivial_expected_storage& operator=(trivial_expected_storage const&) = default;
  trivial_expected_storage& operator=(trivial_expected_storage &&) = default;
  ~trivial_expected_storage() = default;
};

template <typename E>
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
struct trivial_expected_storage<void, E>
  : unrestricted_union_emulation_storage<void, E>,
  unrestricted_union_emulation_err_tag<trivial_expected_storage<void, E>, void, E>
#else
union trivial_expected_storage<void, E>
#endif
{
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  unsigned char dummy;
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<trivial_expected_storage<void, E>, void, E> _unexpected;
#else
  unexpected_t _unexpected;
  constexpr const unexpected_t &unexpected_() const { return _unexpected; }
  unexpected_t &unexpected_() { return _unexpected; }
  constexpr const error_type &err() const { return unexpected_().value(); }
  error_type &err() { return unexpected_().value(); }
#endif

  constexpr trivial_expected_storage()
  : dummy(0)
  //: _err(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  constexpr explicit trivial_expected_storage(unexpected_t const& e)
  : _unexpected(e)
  {}
  constexpr explicit trivial_expected_storage(unexpected_t && e)
  : _unexpected(move(e))
  {}

  template <class Err>
  constexpr explicit trivial_expected_storage(unexpected<Err> const& e)
  : _unexpected(error_traits<error_type>::make_error(e.value()))
  {}

  constexpr explicit trivial_expected_storage(in_place_t)
  : dummy(0)
  {}

//  trivial_expected_storage(trivial_expected_storage const&) = default;
//  trivial_expected_storage(trivial_expected_storage &&) = default;
//  trivial_expected_storage& operator=(trivial_expected_storage const&) = default;
//  trivial_expected_storage& operator=(trivial_expected_storage &&) = default;
//  ~trivial_expected_storage() = default;
};

template <typename T, typename E >
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
struct no_trivial_expected_storage
  : unrestricted_union_emulation_storage<T, E>,
  unrestricted_union_emulation_val_tag<no_trivial_expected_storage<T, E>, T, E>,
  unrestricted_union_emulation_err_tag<no_trivial_expected_storage<T, E>, T, E>
#else
union no_trivial_expected_storage
#endif
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<no_trivial_expected_storage<T, E>, T, E> _unexpected;
  typedef unrestricted_union_emulation_val_tag<no_trivial_expected_storage<T, E>, T, E> _val;
#else
  unsigned char dummy;
  value_type _val;
  unexpected_t _unexpected;
#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CXX14_CONSTEXPR const unexpected_t &unexpected_() const& { return _unexpected; }
  BOOST_CXX14_CONSTEXPR unexpected_t &unexpected_() &{ return _unexpected; }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS unexpected_t &&unexpected_() &&{ return move(_unexpected); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const unexpected_t &&unexpected_() const &&{ return move(_unexpected); }
  BOOST_CXX14_CONSTEXPR const error_type &err() const& { return _unexpected.value(); }
#if defined __clang__
  BOOST_CXX14_CONSTEXPR error_type &err() & { return _unexpected.value(); }
#else
  BOOST_CXX14_CONSTEXPR error_type &err() & { return _unexpected.error_; } // fixme
#endif
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS error_type &&err() && { return move(_unexpected.value()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const error_type &&err() const && { return move(_unexpected.value()); }
#else
  constexpr const unexpected_t &unexpected_() const { return _unexpected; }
  constexpr unexpected_t &unexpected_() { return _unexpected; }
  constexpr const error_type &err() const { return unexpected_().value(); }
  constexpr error_type &err() { return unexpected_().value(); }
#endif
  constexpr const value_type &val() const { return _val; }
  value_type &val() { return _val; }
#endif

  constexpr explicit no_trivial_expected_storage(only_set_initialized_t)
  : dummy(0)
  {}
  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr no_trivial_expected_storage()
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  constexpr explicit no_trivial_expected_storage(value_type const& v)
  : _val(v)
  {}
  constexpr explicit no_trivial_expected_storage(value_type && v)
  : _val(move(v))
  {}

  constexpr explicit no_trivial_expected_storage(unexpected_t const& e)
  : _unexpected(e)
  {}
  constexpr explicit no_trivial_expected_storage(unexpected_t && e)
  : _unexpected(move(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_storage(unexpected<Err> const& e)
  : _unexpected(error_traits<error_type>::make_error(e.value()))
  {}

  constexpr explicit no_trivial_expected_storage(in_place_t) //BOOST_NOEXCEPT_IF()
  : _val(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  template <class... Args>
  constexpr explicit no_trivial_expected_storage(in_place_t, Args&&... args) //BOOST_NOEXCEPT_IF()
  : _val(constexpr_forward<Args>(args)...)
  {}

  no_trivial_expected_storage(no_trivial_expected_storage const&) = default;
  no_trivial_expected_storage(no_trivial_expected_storage &&) noexcept = default;
  no_trivial_expected_storage& operator=(no_trivial_expected_storage const&) = default;
  no_trivial_expected_storage& operator=(no_trivial_expected_storage &&) noexcept = default;

  ~no_trivial_expected_storage() {}
};

template <typename E>
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
struct no_trivial_expected_storage<void, E>
  : unrestricted_union_emulation_storage<void, E>,
  unrestricted_union_emulation_err_tag<no_trivial_expected_storage<void, E>, void, E>
#else
union no_trivial_expected_storage<void, E>
#endif
{
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  unsigned char dummy;
#ifdef JASEL_NO_CXX11_UNRESTRICTED_UNIONS
  typedef unrestricted_union_emulation_err_tag<no_trivial_expected_storage<void, E>, void, E> _unexpected;
#else
  unexpected_t _unexpected;
  constexpr const unexpected_t &unexpected_() const { return _unexpected; }
  unexpected_t &unexpected_() { return _unexpected; }
  constexpr const error_type &err() const { return unexpected_().value(); }
  error_type &err() { return unexpected_().value(); }
#endif

  constexpr explicit no_trivial_expected_storage(only_set_initialized_t)
  : dummy(0)
  {}
  constexpr no_trivial_expected_storage()
  : dummy(0)
  //: _err(boost_expected_unrestricted_union_emulation_default_tag())
  {}

  constexpr explicit no_trivial_expected_storage(unexpected_t const& e)
  : _unexpected(e)
  {}
  constexpr explicit no_trivial_expected_storage(unexpected_t && e)
  : _unexpected(move(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_storage(unexpected<Err> const& e)
  : _unexpected(error_traits<error_type>::make_error(e.value()))
  {}

  constexpr explicit no_trivial_expected_storage(in_place_t)
  : dummy(0)
  {}

  no_trivial_expected_storage(no_trivial_expected_storage const&) = default;
  no_trivial_expected_storage(no_trivial_expected_storage &&) noexcept = default;
  no_trivial_expected_storage& operator=(no_trivial_expected_storage const&) = default;
  no_trivial_expected_storage& operator=(no_trivial_expected_storage &&) noexcept = default;

  ~no_trivial_expected_storage() {}
};


template <typename T, typename E, bool AreCopyConstructible, bool AreMoveConstructible >
struct trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  trivial_expected_storage<T, E> storage;

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr trivial_expected_base()
    BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  constexpr explicit trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place, v)
  {}

  constexpr explicit trivial_expected_base(value_type&& v)
  : has_value(true), storage(in_place, constexpr_move(v))
  {}

  constexpr explicit trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}

  constexpr explicit trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  template <class... Args>
  explicit constexpr
  trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit constexpr
  trivial_expected_base(in_place_t, initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr()
  { return addressof(storage.val()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const value_type* dataptr() const
  {
    return detail::static_addressof(storage.val()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  error_type* errorptr()
  {
    return addressof(storage.err()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr()
  {
    return addressof(storage.unexpected_()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr const value_type& contained_val() const& { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() &
  {
    return storage.val(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const && { return move(storage.val()); }

  constexpr const unexpected_t& contained_unexpected() const&
  {
    return storage.unexpected_(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected_t() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected_t() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected_t() const && { return move(storage.unexpected_()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(storage.err()); }

#else
  constexpr const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  JASEL_0_REQUIRES(
//        is_copy_constructible<value_type>::value &&
//        is_copy_constructible<error_type>::value
//  )
  trivial_expected_base(const trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<value_type>::value &&
          is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      has_value = rhs.has_value;
    }

//  JASEL_0_REQUIRES(
//      is_move_constructible<value_type>::value &&
//      is_move_constructible<error_type>::value
//  )
  trivial_expected_base(trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<value_type>::value &&
          is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(move(rhs.contained_val()));
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      has_value = rhs.has_value;
    }

  // fixme: define these operations
//   trivial_expected_base& operator=(trivial_expected_base const&) = default;
//   trivial_expected_base& operator=(trivial_expected_base &&) = default;

  template <class Err, bool C, bool M>
  trivial_expected_base(const trivial_expected_base<value_type, Err, C, M >& rhs
    //, JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  {
    if (rhs.has_value)
    {
      ::new (dataptr()) value_type(rhs.contained_val());
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
    }
    has_value = rhs.has_value;
  }

  template <class Err, bool C, bool M>
  trivial_expected_base(trivial_expected_base<value_type, Err, C, M >&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  {
    if (rhs.has_value)
    {
      ::new (dataptr()) value_type(move(rhs.contained_val()));
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
    }
    has_value = rhs.has_value;
  }

   ~trivial_expected_base() = default;
};

template <typename E, bool AreCopyConstructible, bool AreMoveConstructible>
struct trivial_expected_base<void, E, AreCopyConstructible, AreMoveConstructible>
{
  typedef void value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  trivial_expected_storage<void, E> storage;

  constexpr trivial_expected_base()
  : has_value(true) {}

  constexpr explicit trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}
  constexpr explicit trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}
  template <class Err>
  constexpr explicit trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}
  constexpr explicit trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place)
  {}

  // Access
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_t(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(storage.err()); }

#else
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

//  JASEL_0_REQUIRES(
//        is_copy_constructible<error_type>::value
//  )
  trivial_expected_base(const trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      has_value = rhs.has_value;
    }

//  JASEL_0_REQUIRES(
//      is_move_constructible<error_type>::value
//  )
  trivial_expected_base(trivial_expected_base&& rhs
    )
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      has_value = rhs.has_value;
    }

  // fixme: define these operations
//   trivial_expected_base& operator=(trivial_expected_base const&) = default;
//   trivial_expected_base& operator=(trivial_expected_base &&) = default;

  template <class Err, bool C, bool M>
  trivial_expected_base(const trivial_expected_base<void, Err, C, M >& rhs
    //, JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  {
    if (rhs.has_value)
    {
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
    }
    has_value = rhs.has_value;
  }

  template <class Err, bool C, bool M>
  trivial_expected_base(trivial_expected_base<void, Err, C, M >&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  {
    if (rhs.has_value)
    {
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
    }
    has_value = rhs.has_value;
  }

   ~trivial_expected_base() = default;
};

template <typename T, typename E, bool AreCopyConstructible, bool AreMoveConstructible >
struct no_trivial_expected_base
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  constexpr explicit no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place, v)
  {}

  constexpr explicit no_trivial_expected_base(value_type&& v)
  : has_value(true), storage(in_place, constexpr_move(v))
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  template <class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr()
  {
    return addressof(storage.val()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() {
    return addressof(storage.unexpected_()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const value_type& contained_val() const& { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() &
  {
    return storage.val(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const&& { return move(storage.val()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() &
  {
    return storage.unexpected_(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const&& { return move(storage.unexpected_()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const&& { return move(storage.err()); }

#else
  constexpr const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  error_type& contained_unexpected() { return storage.unexpected_(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<value_type>::value &&
          is_nothrow_copy_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      //has_value = rhs.has_value;
    }

  no_trivial_expected_base(no_trivial_expected_base&& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<value_type>::value &&
          is_nothrow_move_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(move(rhs.contained_val()));
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      //has_value = rhs.has_value;
    }

  // fixme: define these operations
  //no_trivial_expected_base& operator=(no_trivial_expected_base const&) = default;
  //no_trivial_expected_base& operator=(no_trivial_expected_base &&) = default;

  template <class Err, bool C, bool M>
  no_trivial_expected_base(const no_trivial_expected_base<value_type, Err, C, M >& rhs
    //, JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  {
    if (rhs.has_value)
    {
      ::new (dataptr()) value_type(rhs.contained_val());
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
    }
    has_value = rhs.has_value;
  }

  template <class Err, bool C, bool M>
  no_trivial_expected_base(no_trivial_expected_base<value_type, Err, C, M >&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  {
    if (rhs.has_value)
    {
      ::new (dataptr()) value_type(move(rhs.contained_val()));
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
    }
    has_value = rhs.has_value;
  }

  ~no_trivial_expected_base()
  {
    if (has_value) contained_val().~value_type();
    else contained_unexpected().~unexpected_t();
  }
};

template <typename T, typename E, bool AreMoveConstructible >
struct no_trivial_expected_base<T, E, false, AreMoveConstructible>
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  constexpr explicit no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place, v)
  {}

  constexpr explicit no_trivial_expected_base(value_type&& v)
  : has_value(true), storage(in_place, constexpr_move(v))
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  template <class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr() { return addressof(storage.val()); }
  constexpr const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const value_type& contained_val() const& { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const && { return move(storage.val()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const&& { return move(storage.err()); }

#else
  constexpr const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
#endif

  no_trivial_expected_base(const no_trivial_expected_base& rhs) = delete;

  no_trivial_expected_base(no_trivial_expected_base&& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<value_type>::value &&
          is_nothrow_move_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(move(rhs.contained_val()));
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      //has_value = rhs.has_value;
    }

  // fixme: define these operations
  //no_trivial_expected_base& operator=(no_trivial_expected_base const&) = delete;
  //no_trivial_expected_base& operator=(no_trivial_expected_base &&) = delete;

  template <class Err, bool C, bool M>
  no_trivial_expected_base(no_trivial_expected_base<value_type, Err, C, M >&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  {
    if (rhs.has_value)
    {
      ::new (dataptr()) value_type(move(rhs.contained_val()));
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
    }
    has_value = rhs.has_value;
  }


  ~no_trivial_expected_base()
  {
    if (has_value) storage.val().~value_type();
    else storage.unexpected_().~unexpected_t();
  }
};

template <typename T, typename E, bool AreCopyConstructible>
struct no_trivial_expected_base<T, E, AreCopyConstructible, false>
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  constexpr explicit no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place, v)
  {}

  constexpr explicit no_trivial_expected_base(value_type&& v)
  : has_value(true), storage(in_place, constexpr_move(v))
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  template <class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr() { return addressof(storage.val()); }
  constexpr const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const value_type& contained_val() const& { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const && { return move(storage.val()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

#else
  constexpr const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
#endif

  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<value_type>::value &&
          is_nothrow_copy_constructible<error_type>::value
        )
        : has_value(rhs.has_value), storage(only_set_initialized)
    {
      if (rhs.has_value)
      {
        ::new (dataptr()) value_type(rhs.contained_val());
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      //has_value = rhs.has_value;
    }

  // fixme: define these operations
  no_trivial_expected_base(no_trivial_expected_base&& rhs) = delete;
  //no_trivial_expected_base& operator=(no_trivial_expected_base const&) = default;
  //no_trivial_expected_base& operator=(no_trivial_expected_base &&) = default;

  ~no_trivial_expected_base()
  {
    if (has_value) storage.val().~value_type();
    else storage.unexpected_().~unexpected_t();
  }
};

template <typename T, typename E>
struct no_trivial_expected_base<T, E, false, false>
{
  typedef T value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<T, E> storage;

  JASEL_0_REQUIRES(
        is_default_constructible<value_type>::value
  )
  constexpr no_trivial_expected_base()
    //BOOST_NOEXCEPT_IF(is_nothrow_default_constructible<value_type>::value)
  : has_value(true)
  {}

  constexpr explicit no_trivial_expected_base(const value_type& v)
  : has_value(true), storage(in_place, v)
  {}

  constexpr explicit no_trivial_expected_base(value_type&& v)
  : has_value(true), storage(in_place, constexpr_move(v))
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  template <class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, Args&&... args)
  : has_value(true), storage(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args>
  explicit constexpr
  no_trivial_expected_base(in_place_t, initializer_list<U> il, Args&&... args)
  : has_value(true), storage(in_place, il, constexpr_forward<Args>(args)...)
  {}

  // Access
  value_type* dataptr() { return addressof(storage.val()); }
  constexpr const value_type* dataptr() const { return detail::static_addressof(storage.val()); }
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedtr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const value_type& contained_val() const& { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() & { return storage.val(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const&& { return move(storage.val()); }

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const&& { return move(storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

#else
  constexpr const value_type& contained_val() const { return storage.val(); }
  value_type& contained_val() { return storage.val(); }
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
#endif

  // fixme: define these operations
//  no_trivial_expected_base(no_trivial_expected_base const&) = default;
//  no_trivial_expected_base(no_trivial_expected_base &&) = default;
//  no_trivial_expected_base& operator=(no_trivial_expected_base const&) = default;
//  no_trivial_expected_base& operator=(no_trivial_expected_base &&) = default;

  ~no_trivial_expected_base()
  {
    if (has_value) storage.val().~value_type();
    else storage.unexpected_().~unexpected_t();
  }
};


template <typename E, bool AreCopyConstructible, bool AreMoveConstructible>
struct no_trivial_expected_base<void, E, AreCopyConstructible, AreMoveConstructible> {
  typedef void value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<void, E> storage;

  constexpr no_trivial_expected_base()
  : has_value(true) {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}
  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  constexpr explicit no_trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place)
  {}

  // Access
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr()
  {
    return addressof(storage.unexpected_()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() &
  {
    return storage.unexpected_(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

#else
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }

  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
#endif

  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      has_value = rhs.has_value;
    }

  no_trivial_expected_base(no_trivial_expected_base&& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      has_value = rhs.has_value;
    }

  // fixme: define these operations
  //no_trivial_expected_base& operator=(no_trivial_expected_base const&) = default;
  //no_trivial_expected_base& operator=(no_trivial_expected_base &&) = default;

  template <class Err, bool C, bool M>
  no_trivial_expected_base(const no_trivial_expected_base<void, Err, C, M >& rhs
    //, JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  {
    if (rhs.has_value)
    {
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
    }
    has_value = rhs.has_value;
  }

  template <class Err, bool C, bool M>
  no_trivial_expected_base(no_trivial_expected_base<void, Err, C, M >&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  {
    if (rhs.has_value)
    {
    }
    else
    {
      ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
    }
    has_value = rhs.has_value;
  }
  ~no_trivial_expected_base() {
    if (! has_value)
    {
      contained_unexpected().~unexpected_t();
    }
  }
};

template <typename E, bool AreMoveConstructible>
struct no_trivial_expected_base<void, E, false, AreMoveConstructible> {
  typedef void value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<void, E> storage;

  constexpr no_trivial_expected_base()
  : has_value(true) {}

  constexpr explicit no_trivial_expected_base(unexpected<error_type> const& e)
  : has_value(false), storage(e)
  {}
  constexpr explicit no_trivial_expected_base(unexpected<error_type> && e)
  : has_value(false), storage(constexpr_forward<unexpected<error_type>>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  constexpr explicit no_trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place)
  {}

  // Access
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

#else
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
#endif

  // fixme: define these operations
  no_trivial_expected_base(const no_trivial_expected_base& rhs) = delete;

  no_trivial_expected_base(no_trivial_expected_base&& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_move_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(move(rhs.contained_unexpected()));
      }
      has_value = rhs.has_value;
    }

  // fixme: define these operations
  //no_trivial_expected_base& operator=(no_trivial_expected_base const&) = delete;
  //no_trivial_expected_base& operator=(no_trivial_expected_base &&) = delete;

  ~no_trivial_expected_base() {
    if (! has_value)
      storage.unexpected_().~unexpected_t();
  }
};

template <typename E, bool AreCopyConstructible>
struct no_trivial_expected_base<void, E, AreCopyConstructible, false> {
  typedef void value_type;
  typedef E error_type;
  using unexpected_t = unexpected<error_type>;

  bool has_value;
  no_trivial_expected_storage<void, E> storage;

  constexpr no_trivial_expected_base()
  : has_value(true) {}

  constexpr explicit no_trivial_expected_base(unexpected_t const& e)
  : has_value(false), storage(e)
  {}
  constexpr explicit no_trivial_expected_base(unexpected_t && e)
  : has_value(false), storage(constexpr_forward<unexpected_t>(e))
  {}

  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> const& e)
  : has_value(false), storage(e)
  {}
  template <class Err>
  constexpr explicit no_trivial_expected_base(unexpected<Err> && e)
  : has_value(false), storage(constexpr_forward<unexpected<Err>>(e))
  {}

  constexpr explicit no_trivial_expected_base(in_place_t)
  : has_value(true), storage(in_place)
  {}

  // Access
  error_type* errorptr() { return addressof(storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(storage.err()); }
  unexpected_t* unexpectedptr() { return addressof(storage.unexpected_()); }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr const error_type& contained_err() const& { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const&& { return move(storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const& { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() & { return storage.unexpected_(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(storage.unexpected_()); }

#else
  constexpr const error_type& contained_err() const { return storage.err(); }
  error_type& contained_err() { return storage.err(); }
  constexpr const unexpected_t& contained_unexpected() const { return storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return storage.unexpected_(); }
#endif

  no_trivial_expected_base(const no_trivial_expected_base& rhs)
        BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<error_type>::value
        )
    {
      if (rhs.has_value)
      {
      }
      else
      {
        ::new (unexpectedptr()) unexpected_t(rhs.contained_unexpected());
      }
      has_value = rhs.has_value;
    }

  // fixme: define these operations
  no_trivial_expected_base(no_trivial_expected_base&& rhs) = delete;
//  no_trivial_expected_base& operator=(no_trivial_expected_base const&) = delete;
//  no_trivial_expected_base& operator=(no_trivial_expected_base &&) = delete;

  ~no_trivial_expected_base() {
    if (! has_value)
      storage.unexpected_().~unexpected_t();
  }
};
template <typename T, typename E, bool AreCopyConstructible, bool AreMoveConstructible >
  using expected_base = typename conditional<
    is_trivially_destructible<T>::value && is_trivially_destructible<E>::value,
    trivial_expected_base<T,E, AreCopyConstructible, AreMoveConstructible>,
    no_trivial_expected_base<T,E, AreCopyConstructible, AreMoveConstructible>
  >::type;

} // namespace detail

struct holder;
template <typename ValueType, typename ErrorType>
class expected;

}
namespace meta
{
  template <class T, class E, class U>
  struct rebind<expected<T, E>, U> : id<expected<U, E>> {};
}

inline namespace fundamental_v3
{
namespace expected_detail
{

  template <class C>
  struct unwrap_result_type;

  template <class T, class E>
  struct unwrap_result_type<expected<T,E>> {
    using type = expected<T,E>;
  };

  template <class T, class E>
  struct unwrap_result_type<expected<expected<T,E>,E>> {
    using type = expected<T,E>;
  };

  template <class C>
  using unwrap_result_type_t = typename unwrap_result_type<C>::type;

}

template <typename T>
struct is_expected : false_type {};
template <class T, class E>
struct is_expected<expected<T,E>> : true_type {};

template <typename ValueType, typename ErrorType>
class expected
: public detail::expected_base<ValueType, ErrorType,
    is_copy_constructible<ValueType>::value,
    is_move_constructible<ValueType>::value>
{
public:
  using T = ValueType;
  using E = ErrorType;
  typedef ValueType value_type;
  typedef ErrorType error_type;
  using unexpected_t = unexpected<error_type>;

private:
  typedef expected<value_type, ErrorType> this_type;
  typedef detail::expected_base<ValueType, ErrorType,
      is_copy_constructible<ValueType>::value,
      is_move_constructible<ValueType>::value> base_type;

  // Static asserts.
  typedef is_unexpected<value_type> is_unexpected_value_t;
  static_assert( !is_unexpected_value_t::value, "bad ValueType" );
  typedef is_same<value_type, in_place_t> is_same_value_in_place_t;
  static_assert( !is_same_value_in_place_t::value, "bad ValueType" );
  typedef is_same<value_type, unexpect_t> is_same_value_unexpect_t;
  static_assert( !is_same_value_unexpect_t::value, "bad ValueType" );
  typedef is_same<value_type, expect_t> is_same_value_expect_t;
  static_assert( !is_same_value_expect_t::value, "bad ValueType" );
  typedef is_unexpected<error_type> is_unexpected_error_t;
  static_assert( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef is_same<error_type, in_place_t> is_same_error_in_place_t;
  static_assert( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef is_same<error_type, unexpect_t> is_same_error_unexpect_t;
  static_assert( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef is_same<error_type, expect_t> is_same_error_expect_t;
  static_assert( !is_same_error_expect_t::value, "bad ErrorType" );

  value_type* dataptr()
  {
    return addressof(base_type::storage.val()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const value_type* dataptr() const { return detail::static_addressof(base_type::storage.val()); }
  error_type* errorptr() { return addressof(base_type::storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(base_type::storage.err()); }
  unexpected_t* unexpectedptr()
  {
    return addressof(base_type::storage.unexpected_()); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  constexpr const unexpected_t* unexpectedptr() const { return detail::static_addressof(base_type::storage.unexpected_()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr const bool& contained_has_value() const& { return base_type::has_value; }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return move(base_type::has_value); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const bool&& contained_has_value() const && { return move(base_type::has_value); }

  constexpr const value_type& contained_val() const&
  {
    return base_type::storage.val(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type& contained_val() &
  {
    return base_type::storage.val(); // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type&& contained_val() && { return move(base_type::storage.val()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const value_type&& contained_val() const && { return move(base_type::storage.val()); }

  constexpr const error_type& contained_err() const& { return base_type::storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return base_type::storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(base_type::storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(base_type::storage.err()); }

  constexpr const unexpected_t& contained_unexpected() const&
  {
    return base_type::storage.unexpected_();  // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t& contained_unexpected() &
  {
    return base_type::storage.unexpected_();  // NOLINT cppcoreguidelines-pro-type-union-access
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  unexpected_t&& contained_unexpected() && { return move(base_type::storage.unexpected_()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const unexpected_t&& contained_unexpected() const && { return move(base_type::storage.unexpected_()); }

#else
  constexpr const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  constexpr const value_type& contained_val() const { return base_type::storage.val(); }
  value_type& contained_val() { return base_type::storage.val(); }
  constexpr const error_type& contained_err() const { return base_type::storage.err(); }
  error_type& contained_err() { return base_type::storage.err(); }
  constexpr const unexpected_t& contained_unexpected() const { return base_type::storage.unexpected_(); }
  unexpected_t& contained_unexpected() { return base_type::storage.unexpected_(); }
#endif

public:

  // Using a template alias here causes an ICE in VS2013 and VS14 CTP 3
  // so back to the old fashioned way
  template <class T>
  struct rebind
  {
    typedef expected<T, error_type> type;
  };

  //using type_constructor = expected<holder, error_type>;


  // Constructors/Destructors/Assignments

  JASEL_0_REQUIRES(
      is_default_constructible<value_type>::value
  )
  constexpr expected()
      BOOST_NOEXCEPT_IF(
          is_nothrow_default_constructible<value_type>::value
      )
  : base_type()
  {}

  JASEL_0_REQUIRES(
      is_copy_constructible<value_type>::value
  )
  constexpr expected(const value_type& v) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<value_type>::value
      )
  : base_type(v)
  {}

  JASEL_0_REQUIRES(
    is_move_constructible<value_type>::value
  )
  constexpr expected(value_type&& v  ) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
            is_nothrow_move_constructible<value_type>::value
      )
  : base_type(constexpr_move(v))
  {}

  JASEL_0_REQUIRES(
      is_copy_constructible<value_type>::value
  )
  constexpr expected(const success<value_type>& v) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
          is_nothrow_copy_constructible<value_type>::value
      )
  : base_type(v.value)
  {}

  JASEL_0_REQUIRES(
    is_move_constructible<value_type>::value
  )
  constexpr expected(success<value_type>&& v  ) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
            is_nothrow_move_constructible<value_type>::value
      )
  : base_type(constexpr_move(v.value))
  {}

  expected(const expected& rhs) = default;
  expected(expected&& rhs)  = default;

  template <class Err>
  expected(const expected<value_type, Err>& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  : base_type(rhs)
  {
  }

  template <class Err>
  expected(expected<value_type, Err>&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  : base_type(forward<expected<value_type,Err>>(rhs))
  {
  }

  JASEL_0_REQUIRES(
      is_copy_constructible<error_type>::value
  )
  expected(unexpected<error_type> const& e) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
        is_nothrow_copy_constructible<error_type>::value
      )
  : base_type(e)
  {}
  JASEL_0_REQUIRES(is_move_constructible<error_type>::value)
  expected(unexpected<error_type> && e) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF(
        is_nothrow_move_constructible<error_type>::value
      ) // NOLINT google-explicit-constructor
  : base_type(forward<unexpected<error_type>>(e))
  {}

  template <class Err
    , JASEL_T_REQUIRES(is_constructible<error_type, Err>::value)
  >
  expected(unexpected<Err> const& e) // NOLINT google-explicit-constructor
      BOOST_NOEXCEPT_IF((
        is_nothrow_constructible<error_type, Err>::value
      )) // NOLINT google-explicit-constructor
  : base_type(e)
  {}
  template <class Err
    //, JASEL_T_REQUIRES(is_constructible<error_type, Err&&>::value)
  >
  expected(unexpected<Err> && e // NOLINT google-explicit-constructor
  )
  //BOOST_NOEXCEPT_IF(
    //is_nothrow_constructible<error_type, Err&&>::value
  //)
  // NOLINT google-explicit-constructor
  : base_type(forward<unexpected<Err>>(e))
  {}

  template <class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , JASEL_T_REQUIRES(is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, Args&&... args // NOLINT google-explicit-constructor
  )
  BOOST_NOEXCEPT_IF(
    is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(unexpected<error_type>(error_type(forward<Args>(args)...)))
  {}


  template <class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , JASEL_T_REQUIRES(is_constructible<value_type, typename decay<Args>::type...>::value)
#endif
    >
  constexpr explicit expected(in_place_t, Args&&... args)
  : base_type(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , JASEL_T_REQUIRES(is_constructible<value_type, initializer_list<U>>::value)
#endif
    >
  constexpr explicit expected(in_place_t, initializer_list<U> il, Args&&... args)
  : base_type(in_place, il, constexpr_forward<Args>(args)...)
  {}

  template <class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , JASEL_T_REQUIRES(is_constructible<value_type, Args&...>::value)
#endif
    >
  constexpr explicit expected(expect_t, Args&&... args)
  : base_type(in_place, constexpr_forward<Args>(args)...)
  {}

  template <class U, class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , JASEL_T_REQUIRES(is_constructible<value_type, initializer_list<U>>::value)
#endif
    >
  constexpr explicit expected(expect_t, initializer_list<U> il, Args&&... args)
  : base_type(in_place, il, constexpr_forward<Args>(args)...)
  {}

  ~expected() = default;

  // Assignments
  expected& operator=(expected const& e) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    this_type(e).swap(*this);
    return *this;
  }

  expected& operator=(expected&& e) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    this_type(move(e)).swap(*this);
    return *this;
  }

  template <class U, JASEL_T_REQUIRES(is_same<typename decay<U>::type, value_type>::value)>
  expected& operator=(U const& value) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    this_type(value).swap(*this);
    return *this;
  }

  template <class U, JASEL_T_REQUIRES(is_same<typename decay<U>::type, value_type>::value)>
  expected& operator=(U&& value) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    this_type(forward<U>(value)).swap(*this);
    return *this;
  }

  template <class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
    , JASEL_T_REQUIRES(is_constructible<value_type, Args&...>::value)
#endif
    >
  void emplace(Args&&... args) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    this_type(in_place, constexpr_forward<Args>(args)...).swap(*this);
  }

    template <class U, class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
      , JASEL_T_REQUIRES(is_constructible<value_type, initializer_list<U>, Args&...>::value)
#endif
      >
    void emplace(initializer_list<U> il, Args&&... args) noexcept
      (
          is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
          is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
      )
      {
      this_type(in_place, il, constexpr_forward<Args>(args)...).swap(*this);
    }

  // Modifiers
  void swap(expected& rhs) noexcept
  (
      is_nothrow_move_constructible<T>::value && noexcept( adl::swap_impl( declval<T&>(), declval<T&>() ) ) &&
      is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
  )
  {
    if (valid())
    {
      if (rhs.valid())
      {
        using std::swap;
        swap(contained_val(), rhs.contained_val());
      }
      else
      {
        unexpected_t t = move(rhs.contained_unexpected());
        rhs.unexpectedptr()->~unexpected<E>();
        ::new (rhs.dataptr()) value_type(move(contained_val()));
        dataptr()->~T();
        ::new (unexpectedptr()) unexpected_t(t);
        using std::swap;
        swap(contained_has_value(), rhs.contained_has_value());
      }
    }
    else
    {
      if (rhs.valid())
      {
        rhs.swap(*this);
      }
      else
      {
        using std::swap;
        swap(contained_unexpected(), rhs.contained_unexpected());
      }
    }
  }

  // Observers
  constexpr bool valid() const BOOST_NOEXCEPT
  {
    return contained_has_value();
  }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
  constexpr bool operator !() const BOOST_NOEXCEPT
  {
    return !valid();
  }
  constexpr explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  constexpr value_type const& value() const&
  {
    return valid()
      ? contained_val()
      : (
          error_traits<error_type>::rethrow(contained_err()),
          contained_val()
        )
      ;
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type& value() &
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return contained_val();
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type&& value() &&
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return move(contained_val());
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const value_type&& value() const&&
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return move(contained_val());
  }

#else
  value_type& value()
  {
    if (!valid()) error_traits<error_type>::rethrow(contained_err());
    return contained_val();
  }

  constexpr value_type const& value() const
  {
    return valid()
      ? contained_val()
      : (
          error_traits<error_type>::rethrow(contained_err()),
          contained_val()
        )
      ;
  }
#endif
#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr value_type const& operator*() const& BOOST_NOEXCEPT
  {
    return contained_val();
  }

  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type& operator*() & BOOST_NOEXCEPT
  {
    return contained_val();
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type&& operator*() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_val());
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const value_type&& operator*() const && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_val());
  }
#else
  constexpr value_type const& operator*() const BOOST_NOEXCEPT
  {
    return contained_val();
  }

  value_type& operator*() BOOST_NOEXCEPT
  {
    return contained_val();
  }
#endif

  constexpr value_type const* operator->() const BOOST_NOEXCEPT
  {
    return dataptr();
  }

  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  value_type* operator->() BOOST_NOEXCEPT
  {
    return dataptr();
  }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CXX14_CONSTEXPR error_type const& error() const& BOOST_NOEXCEPT
  {
    return contained_err();
  }
  BOOST_CXX14_CONSTEXPR error_type& error() & BOOST_NOEXCEPT
  {
    return contained_err();
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS error_type&& error() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_err());
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const error_type&& error() const && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_err());
  }
#else
  constexpr error_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }
  error_type& error() BOOST_NOEXCEPT
  {
    return contained_err();
  }
#endif


#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  BOOST_CXX14_CONSTEXPR unexpected<error_type> const& get_unexpected() const& BOOST_NOEXCEPT
  {
    return contained_unexpected();
  }
  BOOST_CXX14_CONSTEXPR unexpected<error_type> & get_unexpected() & BOOST_NOEXCEPT
  {
    return contained_unexpected();
  }

  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS unexpected<error_type>&& get_unexpected() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_unexpected());
  }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS const unexpected<error_type>&& get_unexpected() const && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_unexpected());
  }
#else
  constexpr unexpected<error_type> const& get_unexpected() const BOOST_NOEXCEPT
  {
    return contained_unexpected();
  }
  constexpr unexpected<error_type> & get_unexpected() BOOST_NOEXCEPT
  {
    return contained_unexpected();
  }
#endif

  // Utilities

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS

  template <class V>
  constexpr value_type value_or(V&& v) const&
  {
    return *this
      ? **this
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class V>
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type value_or(V&& v) &&
  {
    return *this
      ? constexpr_move(this->contained_val())
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class Exception>
  constexpr value_type value_or_throw() const&
  {
    return *this
      ? **this
      : throw Exception(contained_err());
  }

  template <class Exception>
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS value_type value_or_throw() &&
  {
    return *this
        ? constexpr_move(this->contained_val())
      : throw Exception(contained_err());
  }

# else

  template <class V>
  constexpr value_type value_or(V&& v) const {
    return *this
      ? **this
      : static_cast<value_type>(constexpr_forward<V>(v));
  }

  template <class Exception>
  constexpr value_type value_or_throw() const {
    return *this
      ? **this
      : throw Exception(contained_err());
  }

# endif


#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr typename expected_detail::unwrap_result_type<expected>::type unwrap() const&;
  inline JASEL_CONSTEXPR_IF_MOVE_ACCESSORS typename expected_detail::unwrap_result_type<expected>::type unwrap() &&;
#else
  constexpr typename expected_detail::unwrap_result_type<expected>::type unwrap() const;
#endif

  template <typename F>
  typename rebind<void>::type
  catch_all_type_void(F&& f) const
  {
    typedef typename rebind<void>::type result_type;
#if defined JASEL_CATCH_EXCEPTIONS
    try {
#endif
      forward<F>(f)(**this);
      return result_type(in_place);
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }

  template <typename F>
  typename result_of<F(value_type)>::type
  catch_all_type_type(F&& f) const
  {
#if defined JASEL_CATCH_EXCEPTIONS
    try {
#endif
      return forward<F>(f)(**this);
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename result_of<F(value_type)>::type>::type
  catch_all_type_etype(F&& f) const
  {
#if defined JASEL_CATCH_EXCEPTIONS
    //typedef typename rebind<typename result_of<F(value_type)>::type>::type result_type;
    try {
#endif
      return forward<F>(f)(**this);
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<void>::type
  catch_all_etype_void(F&& f)
  {
    typedef typename rebind<void>::type result_type;
#if defined JASEL_CATCH_EXCEPTIONS
    try {
#endif
      forward<F>(f)(move(*this));
      return result_type(in_place);
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }

  template <typename F>
  typename result_of<F(expected)>::type
  catch_all_etype_type(F&& f)
  {
#if defined JASEL_CATCH_EXCEPTIONS
    //typedef typename result_of<F(expected)>::type result_type;
    try {
#endif
      return forward<F>(f)(move(*this));
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename result_of<F(expected)>::type>::type
  catch_all_etype_etype(F&& f)
  {
#if defined JASEL_CATCH_EXCEPTIONS
    //typedef typename rebind<typename result_of<F(expected)>::type>::type result_type;
    try {
#endif
      return forward<F>(f)(move(*this));
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }


  template <typename F>
  typename rebind<void>::type
  map(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(value_type)>::type, void>::value))
  const
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_type_void(forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<void>::type result_type;
    return (valid()
        ? catch_all_type_void(forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  typename rebind<typename result_of<F(value_type)>::type>::type
  map(F&& f,
    JASEL_REQUIRES(!is_same<typename result_of<F(value_type)>::type, void>::value))
  const
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
      return catch_all_type_etype(forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<typename result_of<F(value_type)>::type>::type result_type;
    return (valid()
        ? catch_all_type_etype(forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

//  template <typename F>
//  typename rebind<void>::type
//  bind(F&& f,
//    JASEL_REQUIRES(is_same<typename result_of<F(value_type)>::type, void>::value))
//  {
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return catch_all_type_void(forward<F>(f));
//    }
//    return get_unexpected();
//#else
//    typedef typename rebind<void>::type result_type;
//    return (valid()
//        ? catch_all_type_void(forward<F>(f))
//        : result_type( get_unexpected() )
//        );
//#endif
//  }
//
//  template <typename F>
//  typename rebind<typename result_of<F(value_type)>::type>::type
//  bind(F&& f,
//    JASEL_REQUIRES(!is_same<typename result_of<F(value_type)>::type, void>::value
//        && !is_expected<typename result_of<F(value_type)>::type>::value
//        ))
//  {
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return catch_all_type_etype(forward<F>(f));
//    }
//    return get_unexpected();
//#else
//    typedef typename rebind<typename result_of<F(value_type)>::type>::type result_type;
//    return (valid()
//        ? catch_all_type_etype(forward<F>(f))
//        : result_type( get_unexpected() )
//        );
//#endif
//  }

  template <typename F>
  typename result_of<F(value_type)>::type
  bind(F&& f,
    JASEL_REQUIRES(is_expected<typename result_of<F(value_type)>::type>::value
        )
    )
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
      return catch_all_type_type(forward<F>(f));

    }
    return get_unexpected();
#else
     return valid()
         ? catch_all_type_type(forward<F>(f))
         : typename result_of<F(value_type)>::type(get_unexpected());
#endif
  }

  template <typename F>
  typename rebind<void>::type
  then(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(expected)>::type, void>::value))
  {
    //typedef typename rebind<void>::type result_type;
    return catch_all_etype_void(forward<F>(f));
  }

  template <typename F>
  typename rebind<typename result_of<F(expected)>::type>::type
  then(F&& f,
    JASEL_REQUIRES(!is_same<typename result_of<F(expected)>::type, void>::value
        && !is_expected<typename result_of<F(expected)>::type>::value
        ))
  {
    return catch_all_etype_etype(forward<F>(f));
    //typedef typename rebind<typename result_of<F(value_type)>::type>::type result_type;
    //return result_type(forward<F>(f)(move(*this)));
  }

  template <typename F>
  typename result_of<F(expected)>::type
  then(F&& f,
    JASEL_REQUIRES(is_expected<typename result_of<F(expected)>::type>::value)
    )
  {
    return catch_all_etype_type(forward<F>(f));
    //return forward<F>(f)(move(*this));
  }

  template <typename F>
  this_type
  catch_error(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(!valid())
    {
        return this_type(forward<F>(f)(contained_err()));
    }
    return *this;
#else
    return ( ! valid()
         ? this_type(forward<F>(f)(contained_err()))
         : *this
           );
#endif
  }

  template <typename F>
  this_type catch_error(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(error_type)>::type, this_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(!valid())
    {
        return forward<F>(f)(contained_err());
    }
    return *this;
#else
    return ( ! valid()
         ? forward<F>(f)(contained_err())
         : *this
         );
#endif
  }

  template <typename F>
  this_type catch_error(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(error_type)>::type, unexpected<error_type>>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(!valid())
    {
        return forward<F>(f)(contained_err());
    }
    return *this;
#else
    return (! valid()
        ? this_type( forward<F>(f)(contained_err()) )
        : *this
        );
#endif
    }

  // adapt_error factory
  // [T]:E x (E->G) -> [T]:G

  template <typename F>
  expected<value_type,  typename result_of<decay_t<F>(error_type)>::type>
  adapt_error(F&& f)
  {
    typedef expected<value_type,  typename result_of<F(error_type)>::type> result_type;
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(! valid())
    {
        return result_type(unexpect, forward<F>(f)(contained_err()));
    }
    return result_type(contained_val());
#else
    return (! valid()
        ? result_type(unexpect, forward<F>(f)(contained_err()))
        : result_type(contained_val())
        );
#endif
    }
  template <typename Ex, typename F>
  this_type catch_exception(F&& f,
    JASEL_REQUIRES(
        is_same<typename result_of<F(Ex &)>::type, this_type>::value
        )) const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return forward<F>(f)(e);
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex, typename F>
  this_type catch_exception(F&& f,
    JASEL_REQUIRES(
        is_same<typename result_of<F(Ex &)>::type, value_type>::value
        )) const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return this_type(forward<F>(f)(e));
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex>
  bool has_exception() const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
  }

};

template <typename E>
class expected<holder,E> {
public:
  template <class T>
  using type = expected<T, E>;
  template <class T>
  expected<T,E> make(T&& v) {
    return expected<T,E>(forward(v));
  }
};

template <typename T>
using exception_or = expected<T, exception_ptr>;

template <typename ErrorType>
class expected<void, ErrorType>
: public detail::expected_base<void, ErrorType,
  true,
  true >
{
public:
  using E = ErrorType;
  typedef void value_type;
  typedef ErrorType error_type;
  using errored_type = unexpected<error_type>;

private:
  typedef expected<void, error_type> this_type;
  typedef detail::expected_base<void, ErrorType,
      true,
      true> base_type;

  // Static asserts.
  typedef is_unexpected<error_type> is_unexpected_error_t;
  static_assert( !is_unexpected_error_t::value, "bad ErrorType" );
  typedef is_same<error_type, in_place_t> is_same_error_in_place_t;
  static_assert( !is_same_error_in_place_t::value, "bad ErrorType" );
  typedef is_same<error_type, unexpect_t> is_same_error_unexpect_t;
  static_assert( !is_same_error_unexpect_t::value, "bad ErrorType" );
  typedef is_same<error_type, expect_t> is_same_error_expect_t;
  static_assert( !is_same_error_expect_t::value, "bad ErrorType" );

  error_type* errorptr() { return addressof(base_type::storage.err()); }
  constexpr const error_type* errorptr() const { return detail::static_addressof(base_type::storage.err()); }

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr const bool& contained_has_value() const& { return base_type::has_value; }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  bool& contained_has_value() & { return base_type::has_value; }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  bool&& contained_has_value() && { return move(base_type::has_value); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const bool&& contained_has_value() const && { return move(base_type::has_value); }

  constexpr const error_type& contained_err() const& { return base_type::storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type& contained_err() & { return base_type::storage.err(); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  error_type&& contained_err() && { return move(base_type::storage.err()); }
  JASEL_CONSTEXPR_IF_MOVE_ACCESSORS
  const error_type&& contained_err() const && { return move(base_type::storage.err()); }

#else
  constexpr const bool& contained_has_value() const BOOST_NOEXCEPT { return base_type::has_value; }
  bool& contained_has_value() BOOST_NOEXCEPT { return base_type::has_value; }
  constexpr const error_type& contained_err() const { return base_type::storage.err(); }
  error_type& contained_err() { return base_type::storage.err(); }
#endif

public:

  // Using a template alias here causes an ICE in VS2013 and VS14 CTP 3
  // so back to the old fashioned way
  template <class T>
  struct rebind
  {
    typedef expected<T, error_type> type;
  };

  using type_constructor = expected<holder, error_type>;

  // Constructors/Destructors/Assignments

  expected(const expected& rhs) = default;
  expected(expected&& rhs) noexcept = default;

  template <class Err>
  expected(const expected<void, Err>& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err>::value )
  )
  BOOST_NOEXCEPT_IF((
      is_nothrow_constructible<error_type, Err>::value
  ))
  : base_type(rhs)
  {
  }

  template <class Err>
  expected(expected<void, Err>&& rhs
    , JASEL_REQUIRES( is_constructible<error_type, Err&&>::value)
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err&&>::value
  ))
  : base_type(forward<expected<void,Err>>(rhs))
  {
  }

  constexpr explicit expected(in_place_t) BOOST_NOEXCEPT
  : base_type(in_place)
  {}
  constexpr explicit expected(expect_t) BOOST_NOEXCEPT
  : base_type(in_place)
  {}

  constexpr expected() BOOST_NOEXCEPT
  : base_type()
  {}

  constexpr expected(success<void> const&) BOOST_NOEXCEPT  // NOLINT google-explicit-constructor
  : base_type()
  {}

  constexpr expected(success<void> &&) BOOST_NOEXCEPT // NOLINT google-explicit-constructor
  : base_type()
  {}

  JASEL_0_REQUIRES(is_copy_constructible<error_type>::value)
  expected(unexpected<error_type> const& e) // NOLINT google-explicit-constructor
  BOOST_NOEXCEPT_IF(
    is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(e)
  {}

  JASEL_0_REQUIRES(is_move_constructible<error_type>::value)
  expected(unexpected<error_type> && e // NOLINT google-explicit-constructor
  )
  BOOST_NOEXCEPT_IF(
    is_nothrow_move_constructible<error_type>::value
  )
  : base_type(forward<unexpected<error_type>>(e))
  {}

  template <class Err
  , JASEL_T_REQUIRES(is_constructible<error_type, Err>::value)
  >
  expected(unexpected<Err> const& e // NOLINT google-explicit-constructor
  )
  BOOST_NOEXCEPT_IF((
    is_nothrow_constructible<error_type, Err>::value
  ))
  : base_type(e)
  {}

  template <class Err>
  expected(unexpected<Err> && e // NOLINT google-explicit-constructor
//    , JASEL_REQUIRES(is_constructible<error_type, Err&&>::value)
  )
//  BOOST_NOEXCEPT_IF(
//    is_nothrow_constructible<error_type, Err&&>::value
//  )
  : base_type(forward<unexpected<Err>>(e))
  {}

  template <class... Args
#if !defined JASEL_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
  , JASEL_T_REQUIRES(is_constructible<error_type, Args&...>::value)
#endif
  >
  expected(unexpect_t, Args&&... args // NOLINT google-explicit-constructor
  )
  BOOST_NOEXCEPT_IF(
      is_nothrow_copy_constructible<error_type>::value
  )
  : base_type(unexpected<error_type>(error_type(forward<Args>(args)...)))
  {}

  ~expected() = default;

  // Assignments
  expected& operator=(expected const& e)
    noexcept
        (
            is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
        )
  {
    this_type(e).swap(*this);
    return *this;
  }

  expected& operator=(expected&& e)
  noexcept
      (
          is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
      )

  {
    this_type(move(e)).swap(*this);
    return *this;
  }


  void emplace() noexcept
      (
          is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
      )
  {
    this_type(in_place).swap(*this);
  }

  // Modifiers
  void swap(expected& rhs) noexcept
      (
          is_nothrow_move_constructible<E>::value && noexcept( adl::swap_impl( declval<E&>(), declval<E&>() ) )
      )
  {
    if (valid())
    {
      if (! rhs.valid())
      {
        error_type t = move(rhs.contained_err());
        ::new (errorptr()) error_type(t);
        swap(contained_has_value(), rhs.contained_has_value());
      }
    }
    else
    {
      if (rhs.valid())
      {
        rhs.swap(*this);
      }
      else
      {
        using std::swap;
        swap(contained_err(), rhs.contained_err());
      }
    }
  }

  // Observers
  constexpr bool valid() const BOOST_NOEXCEPT
  {
    return contained_has_value();
  }

#if ! defined(BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS)
  constexpr bool operator !() const BOOST_NOEXCEPT
  {
    return !valid();
  }
  constexpr explicit operator bool() const BOOST_NOEXCEPT
  {
    return valid();
  }
#endif

  void value() const
  {
    if(!valid())
    {
      error_traits<error_type>::rethrow(contained_err());
    }
  }

#if ! defined JASEL_NO_CXX11_MOVE_ACCESSORS
  constexpr error_type const& error() const& BOOST_NOEXCEPT
  {
    return contained_err();
  }
  constexpr error_type& error() & BOOST_NOEXCEPT
  {
    return contained_err();
  }
  constexpr error_type&& error() && BOOST_NOEXCEPT
  {
    return constexpr_move(contained_err());
  }
#else
  constexpr error_type const& error() const BOOST_NOEXCEPT
  {
    return contained_err();
  }
  error_type & error() BOOST_NOEXCEPT
  {
    return contained_err();
  }
#endif

#if ! defined JASEL_NO_CXX11_MOVE_ACCESSORS
  constexpr unexpected<error_type> get_unexpected() const& BOOST_NOEXCEPT
  {
    return unexpected<error_type>(contained_err());
  }

  constexpr unexpected<error_type> get_unexpected() && BOOST_NOEXCEPT
  {
    return unexpected<error_type>(constexpr_move(contained_err()));
  }
#else
  constexpr unexpected<error_type> get_unexpected() const BOOST_NOEXCEPT
  {
    return unexpected<error_type>(contained_err());
  }
#endif

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  constexpr typename expected_detail::unwrap_result_type<expected>::type unwrap() const&;
  inline JASEL_CONSTEXPR_IF_MOVE_ACCESSORS typename expected_detail::unwrap_result_type<expected>::type unwrap() &&;
#else
  constexpr typename expected_detail::unwrap_result_type<expected>::type unwrap() const;
#endif

  template <typename F>
  typename rebind<void>::type
  catch_all_void_void(F&& f)
  {
    typedef typename rebind<void>::type result_type;
#if defined JASEL_CATCH_EXCEPTIONS
    try {
#endif
      forward<F>(f)();
      return result_type(in_place);
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename result_of<F()>::type
  catch_all_void_type(F&& f)
  {
#if defined JASEL_CATCH_EXCEPTIONS
    //typedef typename result_of<F()>::type result_type;
    try {
#endif
      return forward<F>(f)();
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
  template <typename F>
  typename rebind<typename result_of<F()>::type>::type
  catch_all_void_etype(F&& f)
  {
#if defined JASEL_CATCH_EXCEPTIONS
    //typedef typename rebind<typename result_of<F()>::type>::type result_type;
    try {
#endif
      return forward<F>(f)();
#if defined JASEL_CATCH_EXCEPTIONS
    } catch (...) {
      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
    }
#endif
  }
//  template <typename F>
//  typename rebind<void>::type
//  catch_all_evoid_void(F&& f)
//  {
//    typedef typename rebind<void>::type result_type;
//    try {
//      forward<F>(f)(move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//    return result_type(in_place);
//  }
//  template <typename F>
//  typename result_of<F(expected)>::type
//  catch_all_evoid_type(F&& f)
//  {
//    //typedef typename result_of<F(expected)>::type result_type;
//    try {
//      return forward<F>(f)(move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//  }
//  template <typename F>
//  typename rebind<typename result_of<F(expected)>::type>::type
//  catch_all_evoid_etype(F&& f)
//  {
//    //typedef typename rebind<typename result_of<F(expected)>::type>::type result_type;
//    try {
//      return forward<F>(f)(move(*this));
//    } catch (...) {
//      return make_unexpected(error_traits<error_type>::make_error_from_current_exception());
//    }
//  }

  template <typename F>
  typename rebind<void>::type
  map(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(value_type)>::type, void>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_void_void(forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<void>::type result_type;
    return (valid()
        ? catch_all_void_void(forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  template <typename F>
  typename rebind<typename result_of<F(value_type)>::type>::type
  map(F&& f,
    JASEL_REQUIRES(!is_same<typename result_of<F(value_type)>::type, void>::value))
  const
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return catch_all_void_etype(forward<F>(f));
    }
    return get_unexpected();
#else
    typedef typename rebind<typename result_of<F(value_type)>::type>::type result_type;
    return (valid()
        ? catch_all_void_etype(forward<F>(f))
        : result_type( get_unexpected() )
        );
#endif
  }

  // bind factory

//  template <typename F>
//  constexpr typename rebind<void>::type bind(F&& f,
//    JASEL_REQUIRES(is_same<typename result_of<F()>::type, void>::value)) const
//  {
//    typedef typename rebind<void>::type result_type;
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        forward<F>(f)();
//        return result_type(in_place);
//    }
//    return get_unexpected();
//#else
//    return ( valid()
//        ? ( forward<F>(f)(), result_type(in_place) )
//        :  result_type(get_unexpected())
//        );
//#endif
//  }
//
//  template <typename F>
//  typename rebind<typename result_of<F()>::type>::type
//  bind(F&& f,
//    JASEL_REQUIRES( ! is_same<typename result_of<F()>::type, void>::value
//        && ! is_expected<typename result_of<F(value_type)>::type>::value
//        ) )
//  {
//    typedef typename rebind<typename result_of<F()>::type>::type result_type;
//#if ! defined BOOST_NO_CXX14_CONSTEXPR
//    if(valid())
//    {
//        return result_type(forward<F>(f)());
//    }
//    return get_unexpected();
//#else
//    return ( valid()
//        ? result_type(forward<F>(f)())
//        :  result_type(get_unexpected())
//        );
//#endif
//  }

  template <typename F>
  typename result_of<F()>::type
  bind(F&& f,
    JASEL_REQUIRES( is_expected<typename result_of<F(value_type)>::type>::value
        ) )
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(valid())
    {
        return forward<F>(f)();
    }
    return get_unexpected();
#else
    typedef typename result_of<F()>::type result_type;
    return ( valid()
        ? forward<F>(f)()
        :  result_type(get_unexpected())
        );
#endif
  }

  template <typename F>
  typename rebind<void>::type
  then(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(expected)>::type, void>::value))
  {
    typedef typename rebind<void>::type result_type;
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    forward<F>(f)(move(*this));
    return result_type(in_place);
#else
    return ( forward<F>(f)(move(*this)), result_type(in_place) );
#endif
  }

  // then factory
  template <typename F>
  typename rebind<typename result_of<F(expected)>::type>::type
  then(F&& f,
    JASEL_REQUIRES(!is_expected<typename result_of<F(expected)>::type>::value
        ))
  {
    typedef typename rebind<typename result_of<F(expected)>::type>::type result_type;
    return result_type(forward<F>(f)(move(*this)));
  }

  template <typename F>
  typename result_of<F(expected)>::type
  then(F&& f,
    JASEL_REQUIRES(!is_same<typename result_of<F(expected)>::type, void>::value
        && is_expected<typename result_of<F(expected)>::type>::value
        )
    )
  {
    return forward<F>(f)(move(*this));
  }

  // catch_error factory

  template <typename F>
  this_type catch_error(F&& f,
    JASEL_REQUIRES(is_same<typename result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(! valid())
    {
        return this_type(forward<F>(f)(contained_err()));
    }
    return *this;
#else
    return (! valid()
        ? this_type(forward<F>(f)(contained_err()))
        : *this
        );
#endif
    }

  template <typename F>
  this_type catch_error(F&& f,
      JASEL_REQUIRES(! is_same<typename result_of<F(error_type)>::type, value_type>::value))
  {
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(!valid())
    {
      return forward<F>(f)(contained_err());
    }
    return *this;
#else
    return (! valid()
        ? forward<F>(f)(contained_err())
        : *this
        );
#endif
  }

  // adapt_error factory
  // [T]:E x (E->G) -> [T]:G

  template <typename F>
  expected<void,  typename result_of<decay_t<F>(error_type)>::type>
  adapt_error(F&& f)
  {
    typedef expected<void,  typename result_of<F(error_type)>::type> result_type;
#if ! defined BOOST_NO_CXX14_CONSTEXPR
    if(! valid())
    {
        return result_type(unexpect, forward<F>(f)(contained_err()));
    }
    return result_type();
#else
    return (! valid()
        ? result_type(unexpect, forward<F>(f)(contained_err()))
        : result_type()
        );
#endif
    }


  template <typename Ex, typename F>
  this_type catch_exception(F&& f,
    JASEL_REQUIRES(
        is_same<typename result_of<F(Ex &)>::type, this_type>::value
        )) const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return forward<F>(f)(e);
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex, typename F>
  this_type catch_exception(F&& f,
    JASEL_REQUIRES(
        is_same<typename result_of<F(Ex &)>::type, value_type>::value
        )) const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return this_type(forward<F>(f)(e));
    }
    catch (...)
    {
      return *this;
    }
    return *this;
  }

  template <typename Ex>
  bool has_exception() const
  {
    try {
      if(!valid()) rethrow_exception(contained_err());
    }
    catch(Ex& e)
    {
      return true;
    }
    catch(...)
    {
    }
    return false;
  }
};

// Relational operators
template <class T, class E>
constexpr bool operator==(const expected<T,E>& x, const expected<T,E>& y)
{
  return (x && y)
    ? *x == *y
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class E>
constexpr bool operator==(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x && y)
    ? true
    : (!x && !y)
      ?  x.get_unexpected() == y.get_unexpected()
      : false;
}

template <class T, class E>
constexpr bool operator!=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(x == y);
}

template <class T, class E>
constexpr bool operator<(const expected<T,E>& x, const expected<T,E>& y)
{
  return (x)
    ? (y) ? *x < *y : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class E>
constexpr bool operator<(const expected<void, E>& x, const expected<void, E>& y)
{
  return (x)
    ? (y) ? false : false
    : (y) ? true : x.get_unexpected() < y.get_unexpected();
}

template <class T, class E>
constexpr bool operator>(const expected<T,E>& x, const expected<T,E>& y)
{
  return (y < x);
}

template <class T, class E>
constexpr bool operator<=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(y < x);
}

template <class T, class E>
constexpr bool operator>=(const expected<T,E>& x, const expected<T,E>& y)
{
  return !(x < y);
}

// Relational operators with T
template <class T, class E>
constexpr bool operator==(const expected<T,E>& x, const T& v)
{
  return (x) ? *x == v :  false;
}
template <class E>
constexpr bool operator==(const E& v, const expected<E,void>& x)
{
  return x == v;
}

template <class T, class E>
constexpr bool operator!=(const expected<T,E>& x, const T& v)
{
  return ! (x == v);
}
template <class T, class E>
constexpr bool operator!=(const T& v, const expected<T,E>& x)
{
  return x != v;
}

template <class T, class E>
constexpr bool operator<(const expected<T,E>& x, const T& v)
{
  return (x) ? (*x < v) : true ;
}
template <class T, class E>
constexpr bool operator<(const T& v, const expected<T,E>& x)
{
  return (x) ? (v < x) : false ;
}

template <class T, class E>
constexpr bool operator>(const expected<T,E>& x, const T& v)
{
  return v < x;
}
template <class T, class E>
constexpr bool operator>(const T& v, const expected<T,E>& x)
{
  return x < v;
}

template <class T, class E>
constexpr bool operator<=(const expected<T,E>& x, const T& v)
{
  return ! (v < x);
}
template <class T, class E>
constexpr bool operator<=(const T& v, const expected<T,E>& x)
{
  return ! (x < v);
}

template <class T, class E>
constexpr bool operator>=(const expected<T,E>& x, const T& v)
{
  return ! (x < v);
}
template <class T, class E>
constexpr bool operator>=(const T& v, const expected<T,E>& x)
{
  return ! (v < x);
}

// Relational operators with unexpected<E>
template <class T, class E>
constexpr bool operator==(const expected<T,E>& x, const unexpected<E>& e)
{
  return (!x) ? x.get_unexpected() == e :  false;
}
template <class T, class E>
constexpr bool operator==(const unexpected<E>& e, const expected<T,E>& x)
{
  return (x == e);
}
template <class T, class E>
constexpr bool operator!=(const expected<T,E>& x, const unexpected<E>& e)
{
  return ! (x == e);
}
template <class T, class E>
constexpr bool operator!=(const unexpected<E>& e, const expected<T,E>& x)
{
  return ! (x == e);
}

template <class T, class E>
constexpr bool operator<(const expected<T,E>& x, const unexpected<E>& e)
{
  return (!x) ? (x.get_unexpected() < e) : false ;
}
template <class T, class E>
constexpr bool operator<(const unexpected<E>& e, const expected<T,E>& x)
{
  return (!x) ? (e < x.get_unexpected()) : true ;
}

template <class T, class E>
constexpr bool operator>(const expected<T,E>& x, const unexpected<E>& e)
{
  return (e <  x);
}
template <class T, class E>
constexpr bool operator>(const unexpected<E>& e, const expected<T,E>& x)
{
  return (x <  e);
}

template <class T, class E>
constexpr bool operator<=(const expected<T,E>& x, const unexpected<E>& e)
{
  return ! (e < x);
}
template <class T, class E>
constexpr bool operator<=(const unexpected<E>& e, const expected<T,E>& x)
{
  return ! (x < e);
}

template <class T, class E>
constexpr bool operator>=(const expected<T,E>& x, const unexpected<E>& e)
{
  return ! (e > x);
}
template <class T, class E>
constexpr bool operator>=(const unexpected<E>& e, const expected<T,E>& x)
{
  return ! (x > e);
}

// Specialized algorithms
template <class T, class E>
void swap(expected<T,E>& x, expected<T,E>& y) BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT_EXPR(x.swap(y)))
{
  x.swap(y);
}

// Factories

template <typename T>
constexpr success<typename decay<T>::type> make_expected(T&& v)
{
  return success<typename decay<T>::type>(constexpr_forward<T>(v));
}
template <typename T, class U>
constexpr success<T> make_expected(U&& v)
{
  return success<T>(constexpr_forward<T>(v));
}

BOOST_FORCEINLINE success<void> make_expected()
{
  return success<void>();
}

template <typename T>
constexpr exception_or<typename decay<T>::type> make_exception_or(T&& v)
{
  return exception_or<typename decay<T>::type>(constexpr_forward<T>(v));
}

BOOST_FORCEINLINE exception_or<void> make_exception_or()
{
  return expected<void, exception_ptr>(in_place);
}

template <typename T>
BOOST_FORCEINLINE expected<T, exception_ptr> make_expected_from_current_exception() BOOST_NOEXCEPT
{
  return expected<T, exception_ptr>(make_unexpected_from_current_exception());
}

template <typename T>
BOOST_FORCEINLINE expected<T, exception_ptr> make_expected_from_exception(exception_ptr e) BOOST_NOEXCEPT
{
  return expected<T, exception_ptr>(unexpected<exception_ptr>(e));
}

template <class T, class E>
BOOST_FORCEINLINE expected<T, exception_ptr> make_expected_from_exception(E&& e) BOOST_NOEXCEPT
{
  return expected<T, exception_ptr>(unexpected<exception_ptr>(constexpr_forward<E>(e)));
}

template <class T, class E>
BOOST_FORCEINLINE constexpr
expected<T, typename decay<E>::type> make_expected_from_error(E&& e) BOOST_NOEXCEPT
{
  return expected<T, typename decay<E>::type>(make_unexpected(constexpr_forward<E>(e)));
}

template <class T, class E, class U>
BOOST_FORCEINLINE constexpr
expected<T, E> make_expected_from_error(U&& u) BOOST_NOEXCEPT
{
  return expected<T, E>(make_unexpected(E(constexpr_forward<U>(u))));
}

template <typename F>
expected<typename result_of<F()>::type, exception_ptr>
BOOST_FORCEINLINE make_expected_from_call(F funct
  , JASEL_REQUIRES( ! is_same<typename result_of<F()>::type, void>::value)
) BOOST_NOEXCEPT
{
  try
  {
    return make_exception_or(funct());
  }
  catch (...)
  {
    return make_unexpected_from_current_exception();
  }
}

template <typename F>
inline expected<void, exception_ptr>
make_expected_from_call(F funct
  , JASEL_REQUIRES( is_same<typename result_of<F()>::type, void>::value)
) BOOST_NOEXCEPT
{
  try
  {
    funct();
    return make_exception_or();
  }
  catch (...)
  {
    return make_unexpected_from_current_exception();
  }
}

template <class T, class E>
BOOST_FORCEINLINE constexpr unexpected<E> make_unexpected(expected<T,E>& ex)
{
  return unexpected<E>(ex.error());
}

namespace expected_detail
{

  // Factories

  template <class T, class E>
  constexpr expected<T,E> unwrap(expected<expected<T,E>, E > const& ee)
  {
     return (ee) ? *ee : ee.get_unexpected();
  }
  template <class T, class E>
  constexpr expected<T,E> unwrap(expected<expected<T,E>, E >&& ee)
  {
    return (ee) ? move(*ee) : ee.get_unexpected();
  }
  template <class T, class E>
  constexpr expected<T,E> unwrap(expected<T,E> const& e)
  {
    return e;
  }
  template <class T, class E>
  constexpr expected<T,E> unwrap(expected<T,E> && e)
  {
    return move(e);
  }

} // namespace expected_detail

#if ! defined JASEL_NO_CXX11_RVALUE_REFERENCE_FOR_THIS
  template <class T, class E>
  constexpr typename expected_detail::unwrap_result_type<expected<T,E>>::type
  expected<T,E>::unwrap() const&
  {
    return expected_detail::unwrap(*this);
  }
  template <class T, class E>
  inline JASEL_CONSTEXPR_IF_MOVE_ACCESSORS typename expected_detail::unwrap_result_type<expected<T,E>>::type
  expected<T,E>::unwrap() &&
  {
    return expected_detail::unwrap(*this);
  }
#else
  template <class T, class E>
  constexpr typename expected_detail::unwrap_result_type<expected<T,E>>::type
  expected<T,E>::unwrap() const
  {
    return expected_detail::unwrap(*this);
  }
#endif

  template <class E>
  struct expected<_t, E>: std::experimental::meta::bind_back<expected, E> {
    using error_type = E;
  };

  namespace type_constructible
  {

//      template <class T, class E>
//      struct traits<expected<T,E>> : traits_constructor<expected<T,E>> {
//        using traits_constructor<expected<T,E>>::make;
//
//        template <class ...Xs>
//        static constexpr
//        expected<T,E> make(in_place_t, Xs&& ...xs)
//        {
//          return expected<T, E>(in_place_t{}, forward<Xs>(xs)...);
//        }
//      };

  }
  // type_constructor customization
  template <class T, class E>
  struct type_constructor<expected<T, E>> : meta::id<expected<_t, E>> {};
  //    // value_type customization
  //    template <class T, class E>
  //    struct value_type<boost::expected<T, E>> : id<T> {};

}}
} // namespace


#endif // JASEL_EXPERIMENTAL_V3_EXPECTED_EXPECTED_HPP

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Copyright (C) 2017,2019 Vicente J. Botet Escriba

#ifndef JASEL_FUNDAMENTAL_V3_STRONG_MIXIN_CONVERTIBLE_HPP
#define JASEL_FUNDAMENTAL_V3_STRONG_MIXIN_CONVERTIBLE_HPP

#include <experimental/fundamental/v2/config.hpp>
#include <experimental/fundamental/v3/strong/mixins/is_compatible_with.hpp>
#include <experimental/type_traits.hpp>
#include <experimental/meta/v1/rebind.hpp>

namespace std
{
  namespace experimental
  {
    inline  namespace fundamental_v3
  {
    namespace mixin
    {
    template <class Final, class T>
    struct explicit_convertible_to_ref
    {
        JASEL_CXX14_CONSTEXPR explicit operator T& () noexcept
        {
            return static_cast<T>(Final::_final(this)._backdoor()._underlying());
        }
        constexpr explicit operator T const& () const noexcept
        {
            return static_cast<T>(Final::_final(this)._backdoor()._underlying());
        }
    };
    template <class Final, class T>
    struct explicit_convertible_to
    {
      constexpr explicit operator T () const noexcept
      {
          return static_cast<T>(Final::_final(this).underlying());
      }
    };
    template <class Final, class T>
    struct implicit_convertible_to_ref
    {
        constexpr operator T const& () const noexcept
        {
            return Final::_final(this)._backdoor()._underlying();
        }
        JASEL_CXX14_CONSTEXPR operator T& () noexcept
        {
            return Final::_final(this)._backdoor()._underlying();
        }
    };
    template <class Final, class T>
    struct implicit_convertible_to
    {
      constexpr operator T () const noexcept
      {
          return Final::_final(this).underlying();
      }
    };

    }
    namespace meta_mixin
    {
    template <class T>
    struct explicit_convertible_to
    {
      template <class Final>
      using type = mixin::explicit_convertible_to<Final, T>;
    };
    template <class T>
    struct explicit_convertible_to_ref
    {
      template <class Final>
      using type = mixin::explicit_convertible_to_ref<Final, T>;
    };
    template <class T>
    struct implicit_convertible_to
    {
      template <class Final>
      using type = mixin::implicit_convertible_to<Final, T>;
    };
    template <class T>
    struct implicit_convertible_to_ref
    {
      template <class Final>
      using type = mixin::implicit_convertible_to_ref<Final, T>;
    };
    }
  }
 
}
}

#endif // header

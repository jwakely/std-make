// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// (C) Copyright 2017 Vicente J. Botet Escriba
// modeled slightly after Andrescu's talk and article(s)
// Based on https://github.com/PeterSommerlad/SC22WG21_Papers/blob/master/workspace/P0052_scope_exit/src/scope_exit.h

/*
 * MIT License
Copyright (c) 2016/2017 Eric Niebler, adapted by Peter Sommerlad
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#ifndef JASEL_EXPERIMENTAL_SCOPED_SCOPE_HPP
#define JASEL_EXPERIMENTAL_SCOPED_SCOPE_HPP

#include <experimental/fundamental/v3/scoped/detail/box.hpp>
#include <exception>
#include <utility>
#include <limits>

namespace std
{
  namespace experimental
  {
    inline namespace fundamental_v3
    {
#if ! defined JASEL_DOXYGEN_INVOKED
      // new policy-based exception proof design by Eric Niebler
      namespace scope_detail
      {
        struct _on_exit_policy
        {
          bool execute = true;

          void release() noexcept
          {
            execute = false;
          }

          bool should_execute() noexcept
          {
            return execute;
          }
        };

#if __cplusplus > 201402L
#if defined __GNUC && ! defined __clan__
#define JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS
#endif
#endif
#if defined JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS
        struct _on_fail_policy
        {
          int ec = uncaught_exceptions();

          void release() noexcept
          {
            ec = numeric_limits<int>::max();
          }

          bool should_execute() noexcept
          {
            return ec < uncaught_exceptions();
          }
        };

        struct _on_success_policy
        {
          int ec = uncaught_exceptions();

          void release() noexcept
          {
            ec = -1;
          }

          bool should_execute() noexcept
          {
            return ec >= uncaught_exceptions();
          }
        };
#endif
      }
#endif

      template<class EF, class Policy = scope_detail::_on_exit_policy>
      class basic_scope_exit;

      //PS: It would be nice if just the following would work in C++17
      //PS: however, we need a real class for template argument deduction
      //PS: and a deduction guide, because the ctors are partially instantiated

#if defined JASEL_HAS_CONSTRUCTOR_GUIDES || defined JASEL_DOXYGEN_INVOKED
      template<class EF>
      struct scope_exit : basic_scope_exit<EF, scope_detail::_on_exit_policy>
      {
        using basic_scope_exit<EF, scope_detail::_on_exit_policy>::basic_scope_exit;
      };

      template <class EF>
      scope_exit(EF &&ef) -> scope_exit<decay_t<EF>>;
#else
      template<class EF>
      using scope_exit = basic_scope_exit<EF, scope_detail::_on_exit_policy>;
#endif

#if defined JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS  || defined JASEL_DOXYGEN_INVOKED
#if defined JASEL_HAS_CONSTRUCTOR_GUIDES || defined JASEL_DOXYGEN_INVOKED
      template<class EF>
      struct scope_fail : basic_scope_exit<EF, scope_detail::_on_fail_policy>
      {
        using basic_scope_exit<EF, scope_detail::_on_fail_policy>::basic_scope_exit;
      };
      template <class EF>
      scope_fail(EF &&ef) -> scope_fail<decay_t<EF>>;
#else
      template<class EF>
      using scope_fail = basic_scope_exit<EF, scope_detail::_on_fail_policy>;
#endif


#if defined JASEL_HAS_CONSTRUCTOR_GUIDES || defined JASEL_DOXYGEN_INVOKED
      template<class EF>
      struct scope_success : basic_scope_exit<EF, scope_detail::_on_success_policy>
      {
        using basic_scope_exit<EF,scope_detail::_on_success_policy>::basic_scope_exit;
      };
      template <class EF>
      scope_success(EF &&ef) -> scope_success<decay_t<EF>>;
#else
      template<class EF>
      using scope_success = basic_scope_exit<EF, scope_detail::_on_success_policy>;
#endif
#endif

#if ! defined JASEL_DOXYGEN_INVOKED
      namespace scope_detail
      {
        // DETAIL:
        template<class Policy, class EF>
        auto _make_guard(EF &&ef) -> decltype(basic_scope_exit<decay_t<EF>, Policy>(forward<EF>(ef)))
        {
          return basic_scope_exit<decay_t<EF>, Policy>(forward<EF>(ef));
        }
        struct _empty_scope_exit
        {
          void release() noexcept
          {}
        };
      }
#endif
      // Requires: EF is Callable
      // Requires: EF is nothrow MoveConstructible OR CopyConstructible
      template<class EF, class Policy /*= _on_exit_policy*/>
      class basic_scope_exit : Policy
      {
        scope_detail::_box<EF> _exit_function;

        static auto _make_failsafe(true_type, const void *) -> decltype(scope_detail::_empty_scope_exit{})
        {
          return scope_detail::_empty_scope_exit{};
        }
        template<typename Fn>
        static auto _make_failsafe(false_type, Fn *fn) -> decltype(basic_scope_exit<Fn &, Policy>(*fn))
        {
          return basic_scope_exit<Fn &, Policy>(*fn);
        }
        template<typename EFP>
        using _ctor_from = is_constructible<scope_detail::_box<EF>, EFP, scope_detail::_empty_scope_exit>;
        template<typename EFP>
        using _noexcept_ctor_from = integral_constant<bool, noexcept(scope_detail::_box<EF>(declval<EFP>(), scope_detail::_empty_scope_exit
                {}))>;
      public:
        template<typename EFP, typename = enable_if_t<_ctor_from<EFP>::value>>
        explicit basic_scope_exit(EFP &&ef) noexcept(_noexcept_ctor_from<EFP>::value)
        : _exit_function(static_cast<EFP &&>(ef), _make_failsafe(_noexcept_ctor_from<EFP>
                {}, &ef))
        {}
        basic_scope_exit(basic_scope_exit &&that) noexcept(noexcept(scope_detail::_box<EF>(that._exit_function.move(), that)))
        : Policy(that), _exit_function(that._exit_function.move(), that)
        {}
        ~basic_scope_exit() noexcept(noexcept(_exit_function.get()()))
        {
          if(this->should_execute())
          _exit_function.get()();
        }
        basic_scope_exit(const basic_scope_exit &) = delete;
        basic_scope_exit &operator=(const basic_scope_exit &) = delete;
        basic_scope_exit &operator=(basic_scope_exit &&) = delete;

        using Policy::release;

      };

      template<class EF, class Policy>
      void swap(basic_scope_exit<EF, Policy> &, basic_scope_exit<EF, Policy> &) = delete;

      template<class EF>
      auto make_scope_exit(EF &&ef)
      noexcept(is_nothrow_constructible<decay_t<EF>, EF>::value)
      -> decltype(scope_detail::_make_guard<scope_detail::_on_exit_policy>(forward<EF>(ef)))
      {
        return scope_detail::_make_guard<scope_detail::_on_exit_policy>(forward<EF>(ef));
      }

#if defined JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS || defined JASEL_DOXYGEN_INVOKED
      template<class EF>
      auto make_scope_fail(EF &&ef)
      noexcept(is_nothrow_constructible<decay_t<EF>, EF>::value)
      {
        return scope_detail::_make_guard<scope_detail::_on_fail_policy>(forward<EF>(ef));
      }

      template<class EF>
      auto make_scope_success(EF &&ef)
      noexcept(is_nothrow_constructible<decay_t<EF>, EF>::value)
      {
        return scope_detail::_make_guard<scope_detail::_on_success_policy>(forward<EF>(ef));
      }
#endif
    }}}

#if defined JASEL_HAS_CONSTRUCTOR_GUIDES && ! defined JASEL_DOXYGEN_INVOKED
#define JASEL_SCOPE_EXIT(G,F) \
    scope_exit G(F);
#if defined JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS
#define JASEL_SCOPE_FAIL(G,F) \
    scope_fail G(F);
#define JASEL_SCOPE_FAIL(G,F) \
    scope_success G(F);
#endif
#else
#define JASEL_SCOPE_EXIT(G,F) \
    auto G = make_scope_exit(F);
#if defined JASEL_STD_HAS_UNCAUGHT_EXCEPTIONS
#define JASEL_SCOPE_FAIL(G,F) \
    auto G = make_scope_fail(F);
#define JASEL_SCOPE_SUCCESS(G,F) \
    auto G = make_scope_success(F);
#endif
#endif

#endif // header

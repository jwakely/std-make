//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Vicente J. Botet Escriba 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file // LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef JASEL_FUNDAMENTAL_V3_RANGE_RANGE_HPP
#define JASEL_FUNDAMENTAL_V3_RANGE_RANGE_HPP

///////////////////////////////////////////////////////////////////////////////////////
///
/// In these definitions, let t denote an expression of type T.
/// An object t is a Range if and only if:
///
/// - the expressions std::experimental::range::begin(t) and std::experimental::range::end(t) are valid, and
/// - the result are a satisfy the Iterator requirements.
///
/// std::experimental::range::begin/std::experimental::range::endn are defined as:
///
/// If has_adl_begin_end<T> calls begin(t)/end(t).
/// Otherwise, the expression range::traits<T>::begin(t)/end(t) if it is a valid expression
/// Otherwise if it is a c-array t+0, t+extent<T>::value
/// Otherwise if it has members begin/end call them
/// Otherwise it is not defined
///
/// has_adl_begin_end<T> is true if the expressions begin(t)/end(t) are a valid when evaluated in the following context:
///
/// std::experimental::range::traits<T> is specialized for types for which the expressions
/// begin(t)/end(t) are valid.
///////////////////////////////////////////////////////////////////////////////////////

#include <iterator>
#include <utility>
#include <experimental/type_traits.hpp>
#include <experimental/meta.hpp>
#include <experimental/type_traits.hpp>
#include <experimental/fundamental/v2/config.hpp>

namespace std
{
  namespace experimental
  {
    inline namespace fundamental_v3
    {
      template <class R, class Enabler=void>
      struct is_range : false_type {};

#if __cplusplus >= 201402L || defined JASEL_DOXYGEN_INVOKED
      template <class T>
        constexpr bool is_range_v = is_range<T>::value;
#endif
      namespace range // here range stands for a concept
      {
        namespace adl_begin_end {

        #if defined __clang__
          template <class T, size_t N> void begin(T(&)[N]) = delete;
          template <class T, size_t N> void end(T(&)[N]) = delete;
        #endif

          template <class T, typename = void>
            struct has_adl_begin_end
              : false_type {};
          template <class T>
            struct has_adl_begin_end<T, void_t<
                  decltype( begin(static_cast<T&&>(declval<T>())) )
                , decltype( end(static_cast<T&&>(declval<T>())) )
                > >
              : true_type {};

#if __cplusplus >= 201402L || defined JASEL_DOXYGEN_INVOKED
          template <class T>
            constexpr bool has_adl_begin_end_v = has_adl_begin_end<T>::value;
#endif
          template <class T>
          auto apply_begin(T&& t)
                JASEL_DECLTYPE_RETURN_NOEXCEPT(
                  begin(forward<T>(t))
                )
           template <class T>
           auto apply_end(T&& t)
                JASEL_DECLTYPE_RETURN_NOEXCEPT(
                  end(forward<T>(t))
                )
        }

        template <class R, class Enabler=void>
        struct traits: traits<R, meta::when<true>>
        {};

        // Default failing specialization
        template <typename R, bool condition>
        struct traits<R, meta::when<condition>>
        {
#if __cplusplus >= 201402L || defined JASEL_DOXYGEN_INVOKED
          template <class T>
          static constexpr auto begin(T&& x) = delete;
          template <class T>
          static constexpr auto end(T&& x) = delete;
#endif
        };
        template <class T, typename = void>
          struct has_trait_range
            : false_type {};
        template <class T>
          struct has_trait_range<T, void_t<
                decltype( traits<T>::begin(declval<T>()) )
              , decltype( traits<T>::end(declval<T>()) )
            > >
            : true_type {};

#if __cplusplus >= 201402L || defined JASEL_DOXYGEN_INVOKED
        template <class T>
          constexpr bool has_trait_range_v = has_trait_range<T>::value;

        template <class T>
          constexpr bool has_adl_begin_end_v = adl_begin_end::has_adl_begin_end<T>::value;
#endif

        template <class T, class Enabler=void>
          struct has_members_begin_end : false_type {};

        template <class T>
          struct has_members_begin_end<T,
            void_t<
              decltype( (declval<T&>()).begin() )
            , decltype( (declval<T&>()).end() )
            >
          > : true_type {};

#if __cplusplus >= 201402L || defined JASEL_DOXYGEN_INVOKED
        template <class T>
          constexpr bool has_members_begin_end_v = has_members_begin_end<T>::value;
#endif

        template <class T>
        constexpr auto begin(T&& x) noexcept -> decltype( traits<remove_cvref_t<T>>::begin(forward<T>(x)) )
        { return traits<remove_cvref_t<T>>::begin(forward<T>(x));}
        template <class T>
        constexpr auto end(T&& x) noexcept -> decltype( traits<remove_cvref_t<T>>::end(forward<T>(x)) )
        { return traits<remove_cvref_t<T>>::end(forward<T>(x));}

        // overload for c-arrays
#if 0
        template <class T, size_t N>
          T* begin(T (&t)[N]) noexcept { return t+0; }
        template <class T, size_t N>
          T* end(T (&t)[N]) noexcept { return t+N; }
#else
        template <class U, size_t M>
        struct traits<U [M]>
        {
          template <class T, size_t N>
            static constexpr T* begin(T (&t)[N]) noexcept { return t+0; }
          template <class T, size_t N>
            static constexpr T* end(T (&t)[N]) noexcept { return t+N; }
        };
#endif
        template <typename R>
        struct traits<R, meta::when<has_members_begin_end<R>::value>>
        {
          template <class T>
            static constexpr auto begin(T&& x) -> decltype(x.begin())
          { return x.begin(); }
          template <class T>
            static constexpr auto end(T&& x) -> decltype(x.end())
          { return x.end(); }
        };
        template <typename R>
        struct traits<R, meta::when<! has_members_begin_end<R>::value && adl_begin_end::has_adl_begin_end<R>::value>>
        {
          template <class T>
            static constexpr auto begin(T&& x) -> decltype(x.begin())
          { return x.begin(); }
          template <class T>
            static constexpr auto end(T&& x)  -> decltype(x.end())
          { return x.end(); }
        };

      }

      template <class T>
        struct is_range<T,
          void_t<
            decltype( range::begin(declval<T&>()) )
          , decltype( range::end(declval<T&>()) )
          >
        >: true_type {};

//      template <class T, size_t N>
//        struct is_range<T (&)[N]>: true_type {};
    }
  }
}
#endif // header

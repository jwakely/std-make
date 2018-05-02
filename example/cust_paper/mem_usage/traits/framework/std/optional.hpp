// Copyright (C) 2016 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef JASEL_EXAMPLE_FRAMEWORK_STD_OPTIONAL_HPP
#define JASEL_EXAMPLE_FRAMEWORK_STD_OPTIONAL_HPP

#if __cplusplus >= 201402L
#include "../mem_usage.hpp"
#include <experimental/optional.hpp>

namespace framework
{
  template <typename U>
  struct mem_usage_traits< std::experimental::optional<U> >
  {

    template <typename T>
    static auto apply(const std::experimental::optional<T>& v) -> decltype( framework::mem_usage(*v) )
    {
      size_t ans = sizeof(v);
      if (v) ans += framework::mem_usage(*v) - sizeof(*v);
      return ans;
    }
  };
}

#endif
#endif

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Copyright (C) 2010,2017 Vicente J. Botet Escriba

#ifndef JASEL_FUNDAMENTAL_V3_STRONG_STRONG_TYPE_HPP
#define JASEL_FUNDAMENTAL_V3_STRONG_STRONG_TYPE_HPP

#include <experimental/fundamental/v3/strong/tagged.hpp>
#include <experimental/fundamental/v2/config.hpp>

namespace std
{
namespace experimental
{
inline  namespace fundamental_v3
{


    //! strong_type wraps an underlying type providing access to the underlying value with a specific @c Final tag.
    //!
    //! The main goal of the Final tag is double : make two strong_type types with different Final tag different types and
    //! have access to the Final
    //!
    //! @Requires
    //! @li @c Final must publicly inherit from this class to make it a model of <em>FinalUnderlying</em>
    //! @li @c UT must be <em>CopyConstructible</em> and <em>Assignable</em>.
    //!
    //! @tparam Tag the tag type
    //! @tparam UT the underlying type

    //! strong_type provides a explicit constructor from convertibles to the underlying type.
    //! Some StronType types could need
    //! * implicit construction
    //!    => They must add an implicit constructor and forward (e.g. safe_bool)
    //! * construction only from the underlying type (e.g. byte)
    //!    => They must add an SFINAE constructor

    template <class Final, class UT, class Tag=Final>
    struct strong_type
    : tagged<Tag, UT>
    {
      using base_type = tagged<Tag, UT>;
      using base_type::base_type;

      using typename base_type::underlying_type;

      //! explicit conversion from a base const class to the @c Final class.
      //! this should be accessible only to the mixin classes, but there is no risk to provide it as public interface,
      //! as it doesn't provide additional access to the clients of the class `Final.
      //! The use of the prefix `_` is intended as make his use less friendly.
      template<typename F>
      static constexpr Final const& _final(F const* f)  {
        return static_cast<Final const&>(*f);
      }
      //! explicit conversion from a base class to the @c Final class.
      template<typename F>
      static constexpr Final& _final(F* f) {
        return static_cast<Final&>(*f);
      }

    private:
      friend struct backdoor;
      friend struct const_backdoor;
      class backdoor {
      public:
        constexpr backdoor(Final& f)
        : that(&f) {}

        //! explicit conversion to the underlying type.
        JASEL_CXX14_CONSTEXPR underlying_type& _underlying() const {
          return that->_value;
        }
      private:
        Final* that;
      };
      class const_backdoor {
      public:
        constexpr const_backdoor(Final const& f)
        : that(&f) {}

        //! explicit conversion to the underlying type.
        JASEL_CXX14_CONSTEXPR underlying_type const& _underlying() const {
          return that->_value;
        }
      private:
        Final const* that;
      };

    public:
      // The following should be accessible only to mixins of the Final class.
      // But there is no way to declare friendship transitively or by inheritance.
      // In order to have this access safe, we would need to add the mixin concept in the language and say that any mixing has friend access to the class.
      // Until then, we use a prefix _ to signal that these functions are almost private and usable only by mixins.
      // This is a way to limit the access to the underlying storage
      // Alternatively the access to a reference to the underlying type should be public,
      // but this opens too much the strong type.
      template<typename F>
      static constexpr const_backdoor _backdoor(F const* f)  {
        return const_backdoor(_final(f));
      }
      constexpr const_backdoor _backdoor() const {
        return const_backdoor(_final(this));
      }
      template<typename F>
      static constexpr backdoor _backdoor(F* f) {
        return backdoor(_final(f));
      }
      JASEL_CXX14_CONSTEXPR backdoor _backdoor() {
        return backdoor(_final(this));
      }
    };

    //!  A MetaMixin is a MetaFunction class having the Final class as parameter

    //! the mixin template class is a helper used to ovoid the repetition of the Final class.
    // The mixin is create with a Final class and a list of MetaMixins
    // It is equivalent to inherit from all the application of all the MetaMixins to the Final class
    template <
      typename Final,
      typename ...MetaMixins
    >
    struct mixins : MetaMixins::template type<Final>...
    {
    };

    //! The new_class template class takes a Final class, the UT underlying type and a list of MetaMixins
    //! The result is a strong type with the Final tag and UT underlying type that inherits from the mixins
    //! Usage: Define a Final class that inheriting from this class
    template <
      typename Final,
      typename Tag,
      typename UT,
      typename ...MetaMixins
    >
    struct new_class : strong_type<Final, UT, Tag>, MetaMixins::template type<Final>...
    {
      using base_type = strong_type<Final, UT, Tag>;
      using base_type::base_type;
    };

    //! The new_type template class takes a Tag class, the UT underlying type and a list of MetaMixins
    //! The result is a strong type class with this new_type as Tag class, UT as underlying type that inherits from the mixins
    //! Usage: Define an alias type of this class.
    template <
      typename Tag,
      typename UT,
      typename ...MetaMixins
    >
    struct new_type final : new_class<new_type<Tag, UT, MetaMixins...>, Tag, UT, MetaMixins...>
    {
      using base_type = new_class<new_type<Tag, UT, MetaMixins...>, Tag, UT, MetaMixins...>;
      using base_type::base_type;
    };

}
}
}

#endif // header

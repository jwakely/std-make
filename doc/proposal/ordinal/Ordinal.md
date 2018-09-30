<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" width="607">
    <tr>
        <td width="172" align="left" valign="top">Document number:</td>
        <td width="435"><span style="background-color: #FFFF00">DXXXXR0</span></td>
    </tr>
    <tr>
        <td width="172" align="left" valign="top">Date:</td>
        <td width="435">2018-01-20</td>
    </tr>
    <tr>
        <td width="172" align="left" valign="top">Project:</td>
        <td width="435">ISO/IEC JTC1 SC22 WG21 Programming Language C++</td>
    </tr>
    <tr>
        <td width="172" align="left" valign="top">Audience:</td>
        <td width="435">Library Evolution Working Group/Reflection Working Group</td>
    </tr>
    <tr>
        <td width="172" align="left" valign="top">Reply-to:</td>
        <td width="435">Vicente J. Botet Escrib&aacute; &lt;<a href="mailto:vicente.botet@wanadoo.fr">vicente.botet@wanadoo.fr</a>&gt;</td>
    </tr>
</table>

# C++ Ordinal types and associated containers
=============================================

**Abstract**

This paper proposes the addition of *Ordinal* types which are isomorphic to `0`..`N` as well as containers using *Ordinal* types as elements or indexes

   * `ordinal_set` using a bit for each element of the *Ordinal* type.
   * `ordinal_array` using an *Ordinal* type as index.
   * `ordinal_range` a run-time range view of an *Ordinal* type.


# Table of Contents

* [Introduction](#introduction)
* [Motivation and Scope](#motivation-and-scope)
* [Proposal](#proposal)
* [Design Rationale](#design-rationale)
* [Impact on the standard](#impact-on-the-standard)
* [Proposed Wording](#proposed-wording)
* [Implementability](#implementability)
* [Open points](#open-points)
* [Acknowledgements](#acknowledgements)
* [References](#references)


# Introduction

With the advent of more strong typing, like bounded integers `month`, `weekday`, we have now that these types cannot be used directly as index of `std::bit_set`. Kevlin Henney presented in Flag Waiving [Flag_Waiving] a safe design for enum sets which is based on the association of an enumeration element and its position on the enumeration itself or to some kind of transformation (bit flags) to the range `0`..`N`. 

This paper propose a generalization of those `enum_set` to any type that is isomorphic to `0`..`N` and name those types *Ordinal* types.


# Motivation and Scope

With the advent of more strong typing, like bounded integers `month`, `weekday`, we have now that these types cannot be used directly as index of `std::bit_set`. Kevlin Henney presented in Flag Waiving [Flag_Waiving] a safe design for enum sets which is based on the association of an enumeration element and its position on the enumeration itself or to some kind of transformation (bit flags) to the range `0`..`N`. 

Let's see how we can use a strong `bounded_int<1,10>` as element of a set. As `bounded_int<1,10>` is isomorphic to `0..9` we could use a `std::bitset` to represent the set

```c++
using B = bounded_int<1,10> ;
using S = std::bitset<10> ;
S set;
B b;
do_something(set[int(b)-1]);
```

However we need to do an explicit mapping from the bounded integer to its position in the range `0..9` using the formula `int(b)-1`.

With `ordinal_set<B>` we avoid the explicit size of the ordinal type as well as the transformation which make the code more readable and safe.

```c++
using B = bounded_int<1,10> ;
using S = ordinal_set<B> ;
S set;
B b;
do_something(set[b]);
```

We have that types as `bounded_int` cannot be used as index of arrays even if `bounded_int` was implicitly convertible to int, as the value and the range are not the same. The same kind of  transformation must be used when we want to use a bounded int as index of an `std::array`. 

```c++
using B = bounded_int<1,10> ;
using A = array<X,B::size> ;
A arr;
B b;
do_something(arr[int(b)-1]);
```

Again `ordinal_array` avoids the explicit size and transformation.

```c++
using B = bounded_int<1,10> ;
using A = ordinal_array<X,B> ;
A arr;
B b;
do_something(arr[b]);
```

The range-v3 library [RangeV3] proposes some integral range views.
Given a type that is isomorphic to `0`..`N`, it is natural to consider the type itself as the range of its values.

Without

```c++
using B = bounded_int<1,10> ;
for (int i = B::first_index; i<=B::last_index, ++i )
    do_someting(B(i));    
```

With `iota` view we can simplify the things a little bit, but we need to state explicitly the bounds. 

```c++
using B = bounded_int<1,10> ;
for (auto b : iota(B::first; B::last) )
    do_someting(b);    
```

With `ordinal_range` we use directly the bounded ordinal type, instead of seen the associated bounds.

```c++
using B = bounded_int<1,10> ;
for (auto b : ordinal_range<B>())
    do_someting(b);    
```

This kind of types are found in language as Ada, which defines attributes on enumerations like `first`, `last`, `succ`, `val`, `pos`, ... This association has constant time complexity most of the time, as usual enumerations follows linear or exponential (base 2) progressions. 

Examples of libraries that have tried to cope with some of these aspects are:

*    Boost.Enum by Frank Laub, takes care of scoping, underlying type, iteration and string conversions, but doesn't take care of avoiding implicit conversion and portability to C++11 scoped enums.
    This thread Smart enums and this one safe assign of int to enum are quite interesting. Unfortunately there were no concrete proposal.
*    Boost.SmartEnums has complex design to take care of iteration.
*    BEnum.

But most of them have been restricted to enum to string conversions. This paper doesn't take in account this conversion, as we will have it almost for free with static reflection.

# Proposal

This paper proposes to add the following 

* *Ordinal* types isomorphic to `0`..`N`.
* Containers using *Ordinal* types as elements or indexes
    * `ordinal_set` using a bit for each element of the *Ordinal* type.
    * `ordinal_array` using an *Ordinal* type as index.
    * `ordinal_range` a run-time range view of an *Ordinal* type.

# Design Rationale


## *Ordinal* types

So, an *ordinal* type is a type that is isomorphic to the range `0`..`N`.
The `N` conditions the `size` of the type, and we need two functions that map the isomorphism, `val` from the position to the ordinal type and `pos` from the ordinal type to the position.

## Traits based customization concept

The design of this proposal is based on a `traits` specialization instead of using ADL. The main reason is to make the mapping of ordinal types explicit, and the second is that we need to customize operations `size` and `pos` that don't have an *Ordinal* type as function parameter, but as template parameter.

The alternative would be to add an additional parameter that coveys the *Ordinal* type.

```c++
size_t ordinal_size(identity_type<O>);
O ordinal_val(identity_type<O>, size_t);
size_t ordinal_pos(O);
```

## `ordinal_set` for ordinal types with small cardinality

`ordinal_set` implementation would use as underlying type a `std::bitset` as the interface is based on `std::bitset`. 

However for ordinal sets with a small cardinality, we could expect to use a more compact memory than the one used by `bitset`.

Before proposing an `ordinal_set` that could have a more compact representation,  we believe that we need to generalize the `bitset` interface with an additional block type. Lets, call it `basic_bitset<N, Block>`. In this way `bitset<N>` could have as representation a `basic_bitset<N, long>`. Then we could define a `basic_ordinal_set<O, Block=long>`. This proposal, doesn't propose such a `basic_bitset<N, long>` type yet.

## `ordinal_range` and the Range views

The range library proposes an integral range view factory `std::experimental:ranges::iota`.

`ordinal_range<O>` should be a model of a *Range*. In order to limit the dependencies between TS, this paper doesn't propose yet to see it as a *range*, but in fact it `ordinal_range<O>` is *Range*.

# Impact on the standard

These changes are entirely based on library extensions and do not require any language features beyond what is available in C++17. There are however some classes in the standard that can be customized.

Once we will have reflection, we could reflect enums with unique values as ordinal types.

# Proposed Wording

The proposed changes are expressed as edits to [N4564] the Working Draft - C++ Extensions for Library Fundamentals V2.

**Add a "Ordinal Types" section**

## Ordinal Types

Ordinal types are types that are isomorphic to the range `0`..`n`.
They can be used as index of ordinal arrays, as element of an ordinal set or as element of an ordinal range. 

### *Ordinal* requirements

A *Ordinal* is a type that supports the `size/val/pos` functions. A type `O` meets the requirements of *Ordinal* if:

* `O` satisfies the requirements of *EqualityComparable* *DefaultConstructible*, and *Destructible*,
* the expressions shown in the table below are valid and have the indicated semantics, and
* `T` satisfies all the other requirements of this sub-clause.

In Table X below, `o` denotes an rvalue of type `O`, `p` denotes a rvalue of type `index_t` an alias of `int`.

<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#111111" width="850">
    <tr>
        <td align="left" valign="top"> <b>Expression</b> </td>
        <td align="left" valign="top"> <b>Return Type</b> </td>
        <td align="left" valign="top"> <b>Operational Semantics</b> </td>
    </tr>
    <tr>
        <td align="left" valign="top"> std::experiental::ordinal::size&lt;O>() </td>
        <td align="left" valign="top"> std::experiental::ordinal::index_t </td>
        <td align="left" valign="top"> number of elements in O</td>
    </tr>
    <tr>
        <td align="left" valign="top"> std::experiental::ordinal::val&lt;O>(p) </td>
        <td align="left" valign="top"> O </td>
        <td align="left" valign="top"> value at position p, for p in 0..(size<O>-1)</td>
    </tr>
    <tr>
        <td align="left" valign="top"> std::experiental::ordinal::pos(o) </td>
        <td align="left" valign="top"> index_t </td>
        <td align="left" valign="top"> position of o</td>
    </tr>

</table>

### Header <experimental/ordinal> synopsis [ordinal.synop]

```c++
namespace std {
namespace experimental {
inline namespace fundamentals_v3 {
namespace ordinal {
  using index_t =  int;

  template <class TC, class Enabler=void>
    struct traits {};
    
  template <class Ordinal>
    constexpr index_t size() noexcept;
  template <class Ordinal>
    constexpr Ordinal val(index_t pos);
  template <class Ordinal>
    constexpr index_t pos(Ordinal&& val) noexcept;   
  template <class Ordinal>
    constexpr Ordinal first() noexcept;
  template <class Ordinal>
    constexpr Ordinal last() noexcept;
  template <class Ordinal>
    constexpr Ordinal succ(Ordinal&& val);
  template <class Ordinal>
    constexpr Ordinal pred(Ordinal&& val);

  template <class T, T Low, T High, T Step = T{1}, class SizeType=index_t>
    struct arithmetic_traits;  
  template <class T>
    struct logarithmic_traits;
  template <typename T>
    struct integral_traits; 
    
  template <> struct traits<int>;
  template <> struct traits<short>;
  template <> struct traits<signed char>;
  template <> struct traits<unsigned int>;
  template <> struct traits<unsigned short>;
  template <> struct traits<unsigned char>;
  template <> struct traits<bool>;
        
}

  template <class T>
    struct is_ordinal;
  template <class T>
    struct is_ordinal<const T>;
  template <class T>
    struct is_ordinal<volatile T>;
  template <class T>
    struct is_ordinal<const volatile T>;    
  template <class T>
    inline constexpr bool is_ordinal_v;
}
}
}
```

####  Class Template `traits` [ordinal.traits]

```c++
namespace ordinal {
    template <class T, class Enabler=void>
        struct traits {};
} 
```

This traits must be specialized for a type to become an *Ordinal* type. 
The specialization must define the 3 customization points `size`, `val` and `pos`.

Next follows the archetype of this customization

```c++
namespace ordinal {
    template <>
    struct traits<O>
    {
      using size = integral_constant<index_t, S>;
      static O val(index_t p);
      static index_t pos(O u);
    };
}
```

where the specialization shall have the following semantics:

* `ordinal::traits<O>::size::value` and `ordinal::traits<O>::size{}` should give the number of elements of the *Ordinal* type `O`.

* `ordinal::traits<O>::val(p)` should give the value at the position `p` in the *Ordinal* type `O`.

* `ordinal::traits<O>::pos(o)` should give the position of the value `o` in the *Ordinal* type `O`.


####  Function Template `size` [ordinal.size]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr auto size() noexcept;
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Returns*: `traits<Ordinal>::size::value` 

####  Function Template `val` [ordinal. val]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr Ordinal val(index_t pos);
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Pre-condition*: `0 <= pos and pos < ordinal::size<O>()`.

*Returns*: `traits<Ordinal>::val(pos)` 

####  Function Template `pos` [ordinal. pos]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr index_t pos(Ordinal&& val) noexcept;
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Returns*: `traits<decay_t<Ordinal>>::pos(forward<Ordinal>(val))` 

####  Function Template `first` [ordinal.first]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr Ordinal first() noexcept;
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Equivalent to*: `ordinal::val<Ordinal>(0)` 

####  Function Template `last` [ordinal.last]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr Ordinal last() noexcept;
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Returns*: `ordinal::val<Ordinal>(ordinal::size<Ordinal>()-1)` 

####  Function Template `succ ` [ordinal.succ]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr decay_t<Ordinal> succ(Ordinal&& val);
}
```

*Requires*: `Ordinal` is an *Ordinal* type.

*Pre-condition*: `ordinal::pos(val)+1 < ordinal::size<decay_t<Ordinal>>()`.

*Returns*: `ordinal::val<decay_t<Ordinal>>(ordinal::pos(val)+1)` 

####  Function Template `pred ` [ordinal.pred]

```c++
namespace ordinal {
  template <class Ordinal>
    constexpr decay_t<Ordinal> pred(Ordinal&& val);
        
}
```
*Requires*: `Ordinal` is an *Ordinal* type.

*Pre-condition*: `ordinal::pos(val) > 0`.

*Returns*: `ordinal::val<Ordinal>(ordinal::pos(val)-1)` 

####  Template class `is_ordinal` [ordinal.is_ordinal]

```c++ 
template <class T>
    struct is_ordinal
    { 
        using type = see below;
    };
```

The nested type alias `type` is `true_type` if the specialization `ordinal::traits<T>` is well formed, the nested alias type `ordinal::traits<T>::size` is an `integral_constant<ordinal::index_t, X>`, the expression `ordinal::traits<T>::val(i)` is well formed for any `i` in the range `0..(ordinal::traits<T>::size::value-1)` and `ordinal::traits<T>::pos(o)` is well formed for any instance `o` of `O` and the mappings are isomorphic, that is `ordinal::traits<T>::val(i1) == ordinal::traits<T>::val(i2)` if and only if `i1 == i2` and `ordinal::traits<T>::pos(o1) == ordinal::traits<T>::pos(o2)` if and only if `o1 == o2`.

####  Template class `arithmetic_traits` [ordinal.arith]

```c++
namespace ordinal {
  template <class T, T Low, T High, T Step = T{1}>
    struct arithmetic_traits
    {
      static_assert(is_integral<T>::value, "T must be integral");

      using value_type = T;

      static constexpr index_t size_v = (index_t(High)-index_t(Low)+1u);
      using size = integral_constant<index_t, size_v>;
      static constexpr value_type val(index_t p) { 
        return value_type{p*Step+Low}; 
      }

      static constexpr index_t pos(value_type v)  { 
        return static_cast<index_t>((v-Low)/Step); 
      };
    };
}
```

`arithmetic_traits` defines a mapping from a linear progression to `0`..`N`  where `val(p) = p*Step+Low`.

####  Template class `logarithmic_traits` [ordinal.arith]

```c++
namespace ordinal {
  template <class T, index_t N>
    struct logarithmic_traits
    {
      static_assert(is_integral<T>::value, "T must be integral");

      using value_type = T;

      static constexpr index_t size_v = N;
      using size = integral_constant<index_t, size_v>;
      static constexpr value_type val(index_t p) { 
        return value_type{1u << p}; 
      }

      static constexpr index_t pos(value_type v)  { 
        return int_log2(index_t(v)); 
      };
    
    };
```

`logarithmic_traits` defines a mapping from a logarithmic progression to `0`..`N` where `val(p) = 2^p`.

####  Template class `integral_traits` [ordinal.num]

```c++
namespace ordinal {
    template <typename T>
        struct integral_traits 
            : arithmetic_traits<T, numeric_limits<T>::min(),
                                   numeric_limits<T>::max()>
}
```
    
*Requires* `T` is an integral type that can be used in a constant expression.

[Note This class could be considered an implementation detail for the definition of the following traits specialization.]

####  Ordinal Traits specialization `traits<>` [ordinal.traits_spec]

```c++
  template <> struct traits<int>;
  template <> struct traits<short>;
  template <> struct traits<signed char>;
  template <> struct traits<unsigned int>;
  template <> struct traits<unsigned short>;
  template <> struct traits<unsigned char>;
  template <> struct traits<bool>;
```

Each one of the preceding traits is specialized using the `integral_traits` with the integral type.

### Header <experimental/ordinal_array> synopsis [ordinal_array.synop]

As `std::array` but replacing the size `N` by the ordinal type `O`. 


```c++
#include <initializer_list>
namespace std::experimental {
inline namespace fundamentals_v3 {

  template<class T, typename O>
    // requires Ordinal<O>
    class ordinal_array;
    
  // , comparable   template <class T, class O>
    bool operator==(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);
  template <class T, class O>
    bool operator!=(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);
  template <class T, class O>
    bool operator<(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);
  template <class T, class O>
    bool operator>(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);
  template <class T, class O>
    bool operator<=(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);
  template <class T, class O>
    bool operator>=(const ordinal_array<T, O>& x, const ordinal_array<T, O>& y);

  // , swappable   template <class T, class O >
    void swap(ordinal_array<T, O>& x, ordinal_array<T, O>& y) noexcept(noexcept(x.swap(y)));


  // , tuple-like customization  template <class T> class tuple_size;
  template <size_t I, class T> class tuple_element;
  template <class T, class O> struct tuple_size<ordinal_array<T, O>>;
  template <size_t I, class T, class O> struct tuple_element<I, ordinal_array<T, O>>;
  template <size_t I, class T, class O> constexpr T& get(ordinal_array<T, O>&) noexcept;
  template <size_t I, class T, class O> constexpr const T& get(const ordinal_array<T, O>&) noexcept;
  template <size_t I, class T, class O> constexpr T&& get(ordinal_array<T, O>&&) noexcept;
  template <size_t I, class T, class O> constexpr const T&& get(const ordinal_array<T, O>&&) noexcept;    

// , hash supporttemplate<class T, class O> struct hash<ordinal_array<T, O>>;

}
}
```

#### Class template `ordinal_array` [ordinal_array]


The header `<ordinal_array>` defines a class template for storing fixed-size sequences of objects identified by an *Ordinal* type. An `ordinal_array` is a contiguous container (26.2.1). An instance of `ordinal_array<T, O>` stores `ordinal::size<O>::value` elements of type `T`, so that `size() == ordinal::size<O>::value` is an invariant.An `ordinal_array` is an aggregate (11.6.1) that can be list-initialized with up to `ordinal::size<O>::value` elements whose types are convertible to `T`.An `ordinal_array` satisfies all of the requirements of a container and of a reversible container (26.2), except that a default constructed ordinal array object is not empty and that `swap` does not have constant complexity. An `ordinal_array` satisfies some of the requirements of a sequence container (26.2.3). Descriptions are provided here only for operations on `ordinal_array` that are not described in one of these tables and for operations where there is additional semantic information.


```c++
template <class T, class O >
struct ordinal_array
{
    // types:
    using value_type = T;    using pointer = T*;    using const_pointer = const T*;    using reference = T&;    using const_reference = const T&;    using key_type = 0;    using size_type = typename ordinal<O>::index_t;    using difference_type = ptrdiff_t;    using iterator = implementation defined; // see 26.2    using const_iterator = implementation defined; // see 26.2    using reverse_iterator = std::reverse_iterator<iterator>;    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    

    // No explicit construct/copy/destroy for aggregate type
    
    void fill(const T& u);
    void swap(ordinal_array& a) noexcept(is_nothrow_swappable_v<T>);

    // iterators:
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    iterator end() noexcept;
    const_iterator end() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;

    const_iterator cbegin() const noexcept;
    const_iterator cend() const noexcept;
    const_reverse_iterator crbegin() const noexcept;
    const_reverse_iterator crend() const noexcept;

    // capacity:
    [[nodiscard]] constexpr bool empty() const noexcept;
    constexpr size_type size() const noexcept;
    constexpr size_type max_size() const noexcept;

    // element access:
    constexpr reference operator[](key_type n);
    constexpr const_reference operator[](key_type n) const;
    constexpr const_reference at(key_type n) const;
    constexpr reference at(key_type n);

    constexpr reference front();
    constexpr const_reference front() const;
    constexpr reference back();
    constexpr const_reference back() const;

    constexpr T* data() noexcept;
    constexpr const T* data() const noexcept;
};

  template<class O, class T, class... U>    ordinal_array(T, U...) -> ordinal_array<T, O>;
    
```

## x.y.z Ordinal set [ordinal_set]

### x.y.z.1 Header <experimental/ordinal_set> synopsis [ordinal_set.synop]

As `std::bitset` but replacing the size `N` by the ordinal type `O`. 

```c++
namespace std::experimental {
inline namespace fundamentals_v3 {


    template<typename O>
    // x.y.z.2 class
    class ordinal_set;

    // x.y.z.3 ordinal_set operators:
    template <class O>
        ordinal_set<O> operator&(const ordinal_set<O>&, const ordinal_set<O>&) noexcept;

    template <class O>
        ordinal_set<O> operator|(const ordinal_set<O>&, const ordinal_set<O>&) noexcept;

    template <class O>
        ordinal_set<O> operator^(const ordinal_set<O>&, const ordinal_set<O>&) noexcept;

    // x.y.z.3 ordinal_set comparable:
    
    // x.y.z.3 ordinal_set swappable:

    // x.y.z.3 ordinal_set streaming operators:
    template <class charT, class traits, class O>
        basic_istream<charT, traits>& operator>>(basic_istream<charT, traits>& is, ordinal_set<O>& x);

    template <class charT, class traits, class O>
        basic_ostream<charT, traits>& operator<<(basic_ostream<charT, traits>& os, const ordinal_set<O>& x);

    // x.y.z.3 ordinal_set hash support:
    template <class O> struct hash<std::ordinal_set<NO>;

}
}
```

#### Class template `ordinal_set` [ordinal_set.ordinal_set]

```c++
template <class O>
class ordinal_set
{
public:
    // bit reference:
    class reference
    {
        friend class ordinal_set;
        reference() noexcept;
    public:
        ~reference() noexcept;
        reference& operator=(bool x) noexcept;           // for b[i] = x;
        reference& operator=(const reference&) noexcept; // for b[i] = b[j];
        bool operator~() const noexcept;                 // flips the bit
        operator bool() const noexcept;                  // for x = b[i];
        reference& flip() noexcept;                      // for b[i].flip();
    };

    using key_type = 0;

    // 23.3.5.1 constructors:
    constexpr ordinal_set() noexcept;
    constexpr ordinal_set(unsigned long long val) noexcept;
    template <class charT>
        explicit ordinal_set(const charT* str,
                        typename basic_string<charT>::size_type n = basic_string<charT>::npos,
                        charT zero = charT('0'), charT one = charT('1'));
    template<class charT, class traits, class Allocator>
        explicit ordinal_set(const basic_string<charT,traits,Allocator>& str,
                        typename basic_string<charT,traits,Allocator>::size_type pos = 0,
                        typename basic_string<charT,traits,Allocator>::size_type n =
                                 basic_string<charT,traits,Allocator>::npos,
                        charT zero = charT('0'), charT one = charT('1'));

    // 23.3.5.2 ordinal_set operations:
    ordinal_set& operator&=(const ordinal_set& rhs) noexcept;
    ordinal_set& operator|=(const ordinal_set& rhs) noexcept;
    ordinal_set& operator^=(const ordinal_set& rhs) noexcept;
    ordinal_set& operator<<=(size_t pos) noexcept;
    ordinal_set& operator>>=(size_t pos) noexcept;
    ordinal_set& set() noexcept;
    ordinal_set& set(key_type pos, bool val = true);
    ordinal_set& reset() noexcept;
    ordinal_set& reset(key_type pos);
    ordinal_set operator~() const noexcept;
    ordinal_set& flip() noexcept;
    ordinal_set& flip(key_type pos);

    // element access:
    constexpr bool operator[](size_t pos) const; // for b[i];
    reference operator[](size_t pos);            // for b[i];
    unsigned long to_ulong() const;
    unsigned long long to_ullong() const;
    template <class charT, class traits, class Allocator>
        basic_string<charT, traits, Allocator> to_string(charT zero = charT('0'), charT one = charT('1')) const;
    template <class charT, class traits>
        basic_string<charT, traits, allocator<charT> > to_string(charT zero = charT('0'), charT one = charT('1')) const;
    template <class charT>
        basic_string<charT, char_traits<charT>, allocator<charT> > to_string(charT zero = charT('0'), charT one = charT('1')) const;
    basic_string<char, char_traits<char>, allocator<char> > to_string(char zero = '0', char one = '1') const;
    size_t count() const noexcept;
    constexpr size_t size() const noexcept;
    bool operator==(const ordinal_set& rhs) const noexcept;
    bool operator!=(const ordinal_set& rhs) const noexcept;
    bool test(size_t pos) const;
    bool all() const noexcept;
    bool any() const noexcept;
    bool none() const noexcept;
    ordinal_set operator<<(size_t pos) const noexcept;
    ordinal_set operator>>(size_t pos) const noexcept;
};

// x.y.z.3, hash supporttemplate<class T> struct hash; 
template<class O> struct hash<ordinal_set<O>>;

```

### Header <experimental/ordinal_range> synopsis [ordinal_range.synop]

Similar to `std::experimental::ranges::iota` but replacing the size `n` by the ordinal type `O`. 


```c++
namespace std::experimental {
inline namespace fundamentals_v3 {

    template<typename O>
    // requires Ordinal<O>
    class ordinal_range;

}
}
```


#### Class template `ordinal_range` [ordinal_range.ordinal_range]

```c++
template<typename O>
// requires Ordinal<O>
class ordinal_range {
public:
    // types:
    using value_type = O;    using size_type = typename ordinal<O>::index_t;    using difference_type = ptrdiff_t;    using iterator = implementation defined; // see 26.2    using const_iterator = implementation defined; // see 26.2    using reverse_iterator = std::reverse_iterator<iterator>;    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // constructors
    constexpr ordinal_range();
    constexpr ordinal_range(O first, O last)   
    
    // iterators:
    constexpr iterator begin() noexcept;
    constexpr const_iterator begin() const noexcept;
    constexpr iterator end() noexcept;
    constexpr const_iterator end() const noexcept;

    constexpr reverse_iterator rbegin() noexcept;
    constexpr const_reverse_iterator rbegin() const noexcept;
    constexpr reverse_iterator rend() noexcept;
    constexpr const_reverse_iterator rend() const noexcept;

    constexpr const_iterator cbegin() const noexcept;
    constexpr const_iterator cend() const noexcept;
    constexpr const_reverse_iterator crbegin() const noexcept;
    constexpr const_reverse_iterator crend() const noexcept;

    constexpr size_t size() noexcept;
    constexpr bool empty() noexcept;


```


# Implementability

This proposal can be implemented as pure library extension, without any language support, in C++17. See [ORDINAL] for a POC.

Additional support will be proposed based on the future reflection library. 

# Open points

The authors would like to have an answer to the following points if there is any interest at all in this proposal:

## Is the customization approach acceptable?

Do we want to use traits as customization points? Or ADL?

## Are the names `size`, `pos`, `val`, `succ`, `pred` the correct ones?

Having these names inside the namespace ordinal avoids confusion. However we could use complete words `position`, `value`, `successor`, `predecessor`

## Do we want a nested `ordinal` namespace?

There is a paper that suggest to don't add more nested namespaces in `std`.
The author believes that we should add more nested and explicit namespaces. What we are missing are explicit namespaces that avoid ADL.

Anyway, the alternatives are to prefix the operations and types with `ordinal_`.

* `ordinal::traits` => `ordinal_traits`
* `ordinal::size` => `ordinal_size`
* `ordinal::pos` => `ordinal_pos`
* `ordinal::val` => `ordinal_val`
* `ordinal::succ` => `ordinal_succ`
* `ordinal::pred` => `ordinal_pred`
* `ordinal::first` => `ordinal_first`
* `ordinal::last` => `ordinal_last`

or to use a struct `ordinal` and static functions and nested types.

## Should we have *Ordinal* types with modulo arithmetic

The proposed `succ` and `pred` functions have undefined behavior when used on the bounds of the *Ordinal* type. We could as well have a modulo arithmetic. Note that the classes `month` and `weekday` have modulo arithmetic.

## Should small integral types be considered as *Ordinal*s?

There is not additional cost to allow them. However, integral types work already well with arrays, `bitset` and ranges. Nevertheless mapping them as ordinal types could take advantage of future features that could require an ordinal type.

# Future work

## Standard classes customization

Some standard classes can be seen as *Ordinal*, in particular enumerations that have all its enumerators different. For the enums we should wait for a stable reflection proposal. However we could already do the mapping for the `month`, `weekday` , ...

## Other classes

If classes as bounded integers are added to the standard, they could be seen as *Ordinal* types using the `arithmetic_traits`.

## *Ordinal* order

*Ordinal* types can have a specific total order by ordering their position `ordinal_compare`. We can define an `ordinal_less<O>` predicate that could be used as order for maps that have an *Ordinal* key. So we could define an alias `ordinal_map<O,T>` that defaults its `Comparator` to `ordinal_less<O>`.

E.g. the type `weekday` is not ordered. However we can define an order using his position and so we will be able to use it as key of an ordered map `std::map`.

## Compile-time ordinal types

In addition to the run-type mapping, we could have a compile-time mapping for enums and integral types.

## Reflection

Any enumeration having all its enumerators different can be seen as *Ordinal* types. An `ordinal::enum_traits` could be defined using reflection. The mapping could be implicit or explicit.

## `bounded_vector<T, N>`

If we end by adopting a `bounded_vector<T, N>`, we could as well have `ordinal_bounded_vector<T, O>`.

## `ordinal::subrange<O>`

Given an *Ordinal* type, we can consider the subrange defined by the values between two ordinal values `o1` and `o2`.

## `ordinal::interval<O, O1, O2>`

Given a compile-time *Ordinal* type, we could consider the subrange defined by the values between two ordinal values `O1` and `O2` known at compile time.

# Acknowledgements

Special thanks and recognition goes to Technical Center of Nokia - Lannion for supporting in part the production of this proposal.

# References

[N4564]: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4564.pdf "N4564 - Working Draft, C++ Extensions for Library Fundamentals, Version 2 PDTS"

[ORDINAL]: https://github.com/viboes/std-make/tree/master/include/experimental/fundamental/v3/ordinal "Ordinal Types"
 
[ADA-enums]: https://en.wikibooks.org/wiki/Ada_Programming/Types/Enumeration  "Ada enumerations"
 
[ADA-arrays]: https://en.wikibooks.org/wiki/Ada_Programming/Types/array "Ada arrays"
    
[Flag_Waiving]: https:// "Flag Waiving" 

* [N4564] N4564 - Working Draft, C++ Extensions for Library Fundamentals, Version 2 PDTS

    http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4564.pdf

* [ORDINAL] Ordinal Types

    https://github.com/viboes/std-make/tree/master/include/experimental/fundamental/v3/ordinal

* [ADA-enums] Ada enumerations
 
    https://en.wikibooks.org/wiki/Ada_Programming/Types/Enumeration 
 
* [ADA-arrays] Ada arrays

    https://en.wikibooks.org/wiki/Ada_Programming/Types/array
    
* [Flag_Waiving] Flag Waiving

    : https://  

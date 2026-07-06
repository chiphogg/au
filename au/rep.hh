// Copyright 2024 Aurora Operations, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "au/fwd.hh"
#include "au/stdx/experimental/is_detected.hh"
#include "au/stdx/type_traits.hh"

namespace au {

//
// A type trait that determines if a type is a valid representation type for `Quantity` or
// `QuantityPoint`.
//
template <typename T>
struct IsValidRep;

//
// A type trait to indicate whether the product of two types is a valid rep.
//
// Will validly return `false` if the product does not exist.
//
template <typename T, typename U>
struct IsProductValidRep;

//
// A type trait to indicate whether the quotient of two types is a valid rep.
//
// Will validly return `false` if the quotient does not exist.
//
template <typename T, typename U>
struct IsQuotientValidRep;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementation details below.
////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {
template <typename T>
struct IsAuType : std::false_type {};

template <typename U, typename R>
struct IsAuType<::au::Quantity<U, R>> : std::true_type {};

template <typename U, typename R>
struct IsAuType<::au::QuantityPoint<U, R>> : std::true_type {};

//
// `NormalizeRep<T>`: strip vendor attributes (e.g. Green Hills' `__packed`) from an integral rep by
// naming a clean standard type, rather than relying on `std::decay` to drop the attribute (which
// GHS does not do).
//
// This is the IDENTITY on every genuine standard type --- integral or not --- so it is a provable
// no-op for any rep a user would normally write.  It only rewrites a type that is integral yet
// names *none* of the standard integer types (which is exactly what an attributed type such as
// `__packed uint16_t` is), mapping it to the standard integer with the same `sizeof` and
// signedness.
//

// Is `T` *exactly* one of the standard integer types?  An attributed integral type is `is_integral`
// but compares unequal to all of these, so it is not "standard" by this definition.
template <typename T>
struct IsStandardInteger : stdx::disjunction<std::is_same<T, bool>,
                                             std::is_same<T, char>,
                                             std::is_same<T, signed char>,
                                             std::is_same<T, unsigned char>,
                                             std::is_same<T, char16_t>,
                                             std::is_same<T, char32_t>,
                                             std::is_same<T, wchar_t>,
                                             std::is_same<T, short>,
                                             std::is_same<T, unsigned short>,
                                             std::is_same<T, int>,
                                             std::is_same<T, unsigned int>,
                                             std::is_same<T, long>,
                                             std::is_same<T, unsigned long>,
                                             std::is_same<T, long long>,
                                             std::is_same<T, unsigned long long>> {};

// Pick the fixed-width standard integer type (`int8_t` ... `int64_t` and unsigned counterparts)
// with the given `sizeof` and signedness; if none matches, fall back to `Fallback` (so an exotic
// integral such as `__int128`, whose width no fixed-width type covers, is left untouched rather
// than becoming a hard error).  We use the fixed-width candidates deliberately: there is exactly
// one per (size, signedness), so the selection is unambiguous --- no reliance on integer-rank
// tie-breaking.  Since the fixed-width types are themselves typedefs to the fundamental types, the
// resulting concrete type is exactly the platform's natural integer of that width.
template <std::size_t Size, bool Signed, typename Fallback, typename... Candidates>
struct FirstMatchingIntegerOr : stdx::type_identity<Fallback> {};

template <std::size_t Size, bool Signed, typename Fallback, typename C, typename... Rest>
struct FirstMatchingIntegerOr<Size, Signed, Fallback, C, Rest...>
    : std::conditional_t<sizeof(C) == Size && (std::is_signed<C>::value == Signed),
                         stdx::type_identity<C>,
                         FirstMatchingIntegerOr<Size, Signed, Fallback, Rest...>> {};

template <typename T, typename Enable = void>
struct NormalizeRepImpl : stdx::type_identity<T> {};  // non-integral or already-standard: identity

template <typename T>
struct NormalizeRepImpl<
    T,
    std::enable_if_t<std::is_integral<T>::value && !IsStandardInteger<T>::value>>
    : FirstMatchingIntegerOr<sizeof(T),
                             std::is_signed<T>::value,
                             T,
                             std::int8_t,
                             std::uint8_t,
                             std::int16_t,
                             std::uint16_t,
                             std::int32_t,
                             std::uint32_t,
                             std::int64_t,
                             std::uint64_t> {};

template <typename T>
using NormalizeRep = typename NormalizeRepImpl<T>::type;

template <typename T>
using CorrespondingUnit = typename CorrespondingQuantity<T>::Unit;

template <typename T>
using CorrespondingRep = typename CorrespondingQuantity<T>::Rep;

template <typename T>
struct HasCorrespondingQuantity
    : stdx::conjunction<stdx::experimental::is_detected<CorrespondingUnit, T>,
                        stdx::experimental::is_detected<CorrespondingRep, T>> {};

template <typename T>
using LooksLikeAuOrOtherQuantity = stdx::disjunction<IsAuType<T>, HasCorrespondingQuantity<T>>;

// We need a way to form an "operation on non-quantity types only".  That is: it's some operation,
// but _if either input is a quantity_, then we _don't even form the type_.
//
// The reason this very specific machinery lives in `rep.hh` is because when we're dealing with
// operations on "types that might be a rep", we know we can exclude quantity types right away.
// (Note that we're using the term "quantity" in an expansive sense, which includes not just
// `au::Quantity`, but also `au::QuantityPoint`, and "quantity-like" types from other libraries
// (which we consider as "anything that has a `CorrespondingQuantity`".
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl;
template <template <class...> class Op, typename... Ts>
using ResultIfNoneAreQuantity = typename ResultIfNoneAreQuantityImpl<Op, Ts...>::type;

// Default implementation where we know that none are quantities.
template <bool AreAnyQuantity, template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityHelper : stdx::type_identity<Op<Ts...>> {};

// Implementation if any of the types are quantities.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityHelper<true, Op, Ts...> : stdx::type_identity<void> {};

// The main implementation.
template <template <class...> class Op, typename... Ts>
struct ResultIfNoneAreQuantityImpl
    : ResultIfNoneAreQuantityHelper<stdx::disjunction<LooksLikeAuOrOtherQuantity<Ts>...>::value,
                                    Op,
                                    Ts...> {};

// The `std::is_empty` is a good way to catch all of the various unit and other monovalue types in
// our library, which have little else in common.  It's also just intrinsically true that it
// wouldn't make much sense to use an empty type as a rep.
template <typename T>
struct IsKnownInvalidRep
    : stdx::disjunction<std::is_empty<T>, LooksLikeAuOrOtherQuantity<T>, std::is_same<void, T>> {};

// The type of the product of two types.
template <typename T, typename U>
using ProductType = decltype(std::declval<T>() * std::declval<U>());

template <typename T, typename U>
using ProductTypeOrVoid = stdx::experimental::detected_or_t<void, ProductType, T, U>;

// The type of the quotient of two types.
template <typename T, typename U>
using QuotientType = decltype(std::declval<T>() / std::declval<U>());

template <typename T, typename U>
using QuotientTypeOrVoid = stdx::experimental::detected_or_t<void, QuotientType, T, U>;
}  // namespace detail

// Implementation for `IsValidRep`.
//
// For now, we'll accept anything that isn't explicitly known to be invalid.  We may tighten this up
// later, but this seems like a reasonable starting point.
template <typename T>
struct IsValidRep : stdx::negation<detail::IsKnownInvalidRep<T>> {};

template <typename T, typename U>
struct IsProductValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantity<detail::ProductTypeOrVoid, T, U>> {};

template <typename T, typename U>
struct IsQuotientValidRep
    : IsValidRep<detail::ResultIfNoneAreQuantity<detail::QuotientTypeOrVoid, T, U>> {};

}  // namespace au

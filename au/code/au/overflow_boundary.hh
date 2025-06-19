// Copyright 2025 Aurora Operations, Inc.
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

#include <limits>

#include "au/stdx/type_traits.hh"

namespace au {
namespace detail {

//
// A monovalue type to indicate that no values can overflow in this direction.
//
// By "direction", we mean that if this is the result for an upper limit, then no value can be large
// enough to overflow; likewise, if a lower limit, no value can be small enough.
//
struct CannotOverflow {};

//
// `MinGood<Op>::value()` is `CannotOverflow{}` if the input type of `Op` cannot be made to overflow
// by taking lower and lower values.  Otherwise, it is a constexpr constant of type `OpInput<Op>`
// that is the minimum value that does not overflow.
//
template <typename Op>
struct MinGoodImpl;
template <typename Op>
using MinGood = typename MinGoodImpl<Op>::type;

//
// `StaticCast<T, U>` represents an operation that converts from `T` to `U` via `static_cast`.
//
template <typename T, typename U>
struct StaticCast {};

//
// `OpInput<Op>` and `OpOutput<Op>` are the input and output types of an operation.
//
template <typename Op>
struct OpInputImpl;
template <typename Op>
using OpInput = typename OpInputImpl<Op>::type;

template <typename Op>
struct OpOutputImpl;
template <typename Op>
using OpOutput = typename OpOutputImpl<Op>::type;

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////

// The implementation strategy will be to decompose to increasingly specific cases, using
// `std::conditional` constructs that are _at most one layer deep_.  This should keep every
// individual piece as easy to understand as possible, although it does mean we'll tend to be
// navigating many layers deep from the top-level API to the ultimate implementation.
//
// It's easier to navigate these helpers if we put a shorthand comment at the top of each.  Here's
// the key:
//
// (A) = arithmetic (integral or floating point)
// (F) = floating point
// (I) = integral (signed or unsigned)
// (N) = non-arithmetic
// (S) = signed integral
// (U) = unsigned integral
// (X) = any type

// Inherit from this struct to produce a compiler error in case we try to use a combination of types
// that isn't yet supported.
template <typename T>
struct OverflowBoundaryNotYetImplemented {
    struct NotYetImplemented {};
    static_assert(std::is_same<T, NotYetImplemented>::value,
                  "Overflow boundary not yet implemented for this type.");
};

struct ValueIsCannotOverflow {
    static constexpr CannotOverflow value() { return CannotOverflow{}; }
};

template <typename T>
struct ValueIsZero {
    static constexpr T value() { return T{0}; }
};

template <typename T, typename U>
struct ValueIsLowestInDestination {
    static constexpr T value() { return static_cast<T>(std::numeric_limits<U>::lowest()); }

    static_assert(static_cast<U>(value()) == std::numeric_limits<U>::lowest(),
                  "This utility assumes lossless round trips");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

template <typename T, typename U>
struct OpInputImpl<StaticCast<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<StaticCast<T, U>> : stdx::type_identity<U> {};

//
// `MinGood<StaticCast<T, U>>` implementation cluster:
//

// (N) -> (X) (placeholder)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (S) -> (S)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromSignedToSigned
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueIsCannotOverflow,
                       ValueIsLowestInDestination<T, U>> {};

// (S) -> (I)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromSignedToIntegral
    : std::conditional_t<std::is_unsigned<U>::value,
                         stdx::type_identity<ValueIsZero<T>>,
                         MinGoodImplForStaticCastFromSignedToSigned<T, U>> {};

// (S) -> (A)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromSignedToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         stdx::type_identity<ValueIsCannotOverflow>,
                         MinGoodImplForStaticCastFromSignedToIntegral<T, U>> {};

// (I) -> (A)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<std::is_unsigned<T>::value,
                         stdx::type_identity<ValueIsCannotOverflow>,
                         MinGoodImplForStaticCastFromSignedToArithmetic<T, U>> {};

// (F) -> (F)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueIsCannotOverflow,
                       ValueIsLowestInDestination<T, U>> {};

// (F) -> (A)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MinGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U>,
                         stdx::type_identity<ValueIsLowestInDestination<T, U>>> {};

// (A) -> (A)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MinGoodImplForStaticCastFromIntegralToArithmetic<T, U>,
                         MinGoodImplForStaticCastFromFloatingPointToArithmetic<T, U>> {};

// (A) -> (X)
template <typename T, typename U>
struct MinGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MinGoodImplForStaticCastFromArithmeticToArithmetic<T, U>,
                         MinGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U>> {};

// (X) -> (X)
template <typename T, typename U>
struct MinGoodImpl<StaticCast<T, U>>
    : std::conditional_t<std::is_arithmetic<T>::value,
                         MinGoodImplForStaticCastFromArithmetic<T, U>,
                         MinGoodImplForStaticCastFromNonArithmetic<T, U>> {};

}  // namespace detail
}  // namespace au

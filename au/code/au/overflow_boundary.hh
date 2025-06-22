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

#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"

namespace au {
namespace detail {

//
// `MinGood<Op>::value()` is a constexpr constant of type `OpInput<Op>` that is the minimum value
// that does not overflow.
//
// IMPORTANT: the result must always be non-positive.  The code is structured on this assumption.
//
template <typename Op, typename Limits>
struct MinGoodImpl;
template <typename Op, typename Limits = void>
using MinGood = typename MinGoodImpl<Op, Limits>::type;

//
// `MaxGood<Op>::value()` is a constexpr constant of type `OpInput<Op>` that is the maximum value
// that does not overflow.
//
// IMPORTANT: the result must always be non-negative.  The code is structured on this assumption.
//
template <typename Op, typename Limits = void>
struct MaxGoodImpl;
template <typename Op, typename Limits = void>
using MaxGood = typename MaxGoodImpl<Op, Limits>::type;

//
// `StaticCast<T, U>` represents an operation that converts from `T` to `U` via `static_cast`.
//
template <typename T, typename U>
struct StaticCast {};

//
// `MultiplyTypeBy<T, M>` represents an operation that multiplies a value of type `T` by the
// magnitude `M`.
//
template <typename T, typename M>
struct MultiplyTypeBy {};

//
// `OpSequence<Ops...>` represents an ordered sequence of operations.
//
// We require that the output type of each operation is the same as the input type of the next one
// (see below for `OpInput` and `OpOutput`).
//
template <typename... Ops>
struct OpSequence {};

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

template <typename T>
constexpr bool is_definitely_unsigned() {
    return std::numeric_limits<T>::is_specialized && !std::numeric_limits<T>::is_signed;
}

// `LowerLimit<T, Limits>::value()` returns `Limits::lower()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the lowest possible value.
template <typename T, typename Limits>
struct LowerLimit {
    static constexpr T value() { return Limits::lower(); }
};
template <typename T>
struct LowerLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::lowest(); }
};

// `UpperLimit<T, Limits>::value()` returns `Limits::upper()` (assumed to be of type `T`), unless
// `Limits` is `void`, in which case it means "no limit" and we return the highest possible value.
template <typename T, typename Limits>
struct UpperLimit {
    static constexpr T value() { return Limits::upper(); }
};
template <typename T>
struct UpperLimit<T, void> {
    static constexpr T value() { return std::numeric_limits<T>::max(); }
};

// `LimitsFor<Op, Limits>` produces a type which can be the `Limits` argument for some other op.
template <typename Op, typename Limits>
struct LimitsFor {
    static constexpr OpInput<Op> lower() { return MinGood<Op, Limits>::value(); }
    static constexpr OpInput<Op> upper() { return MaxGood<Op, Limits>::value(); }
};

// Inherit from this struct to produce a compiler error in case we try to use a combination of types
// that isn't yet supported.
template <typename T>
struct OverflowBoundaryNotYetImplemented {
    struct NotYetImplemented {};
    static_assert(std::is_same<T, NotYetImplemented>::value,
                  "Overflow boundary not yet implemented for this type.");
};

// A type whose `::value()` function returns `0`, expressed in `T`.
template <typename T>
struct ValueIsZero {
    static constexpr T value() { return T{0}; }
};

// A type whose `::value()` function returns the higher of `std::numeric_limits<T>::lowest()`, or
// `LowerLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueIsSourceLowestUnlessDestLimitIsHigher {
    static constexpr T value() {
        constexpr auto LOWEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::lowest());
        constexpr auto U_LIMIT = LowerLimit<U, ULimit>::value();
        return (LOWEST_T_IN_U <= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                          : std::numeric_limits<T>::lowest();
    }
};

// A type whose `::value()` function returns the lower of `std::numeric_limits<T>::max()`, or
// `UpperLimit<U, ULimit>` expressed in `T`.  Assumes that `U` is more expansive than `T`, so that
// we can cast everything to `U` to do the comparisons.
template <typename T, typename U, typename ULimit>
struct ValueIsSourceHighestUnlessDestLimitIsLower {
    static constexpr T value() {
        constexpr auto HIGHEST_T_IN_U = static_cast<U>(std::numeric_limits<T>::max());
        constexpr auto U_LIMIT = UpperLimit<U, ULimit>::value();
        return (HIGHEST_T_IN_U >= U_LIMIT) ? static_cast<T>(U_LIMIT)
                                           : std::numeric_limits<T>::max();
    }
};

// A type whose `::value()` function returns the lowest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueIsLowestInDestination {
    static constexpr T value() { return static_cast<T>(LowerLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == LowerLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function returns the highest value of `U`, expressed in `T`.
template <typename T, typename U = T, typename ULimit = void>
struct ValueIsHighestInDestination {
    static constexpr T value() { return static_cast<T>(UpperLimit<U, ULimit>::value()); }

    static_assert(static_cast<U>(value()) == UpperLimit<U, ULimit>::value(),
                  "This utility assumes lossless round trips");
};

// A type whose `::value()` function is capped at the highest value in `Float` (assumed to be a
// floating point type) that can be cast to `Int` (assumed to be an integral type).  We need to be
// really careful in how we express this, because max int values tend not to be nice powers of 2.
// Therefore, even though we can cast the `Int` max to `Float` successfully, casting back to `Int`
// will produce a compile time error because the closest representable integer in `Float` is
// slightly _higher_ than that max.
//
// On the implementation side, keep in mind that our library supports C++14, and most common
// floating point utilities (such as `std::nextafter`) are not `constexpr` compatible in C++14.
// Therefore, we need to use alternative strategies to explore the floating point type.  These are
// always evaluated at compile time, so we are not especially concerned about the efficiency: it
// should have no runtime effect at all, and we expect even the compile time impact --- which we
// measure regularly as we land commits --- to be too small to measure.
template <typename Float, typename Int, typename IntLimit>
struct ValueIsMaxFloatNotExceedingMaxInt {
    // The `Float` value where all mantissa bits are set to `1`, and the exponent is `0`.
    static constexpr Float max_mantissa() {
        constexpr Float ONE = Float{1};
        Float x = ONE;
        Float last = x;
        while (x + ONE > x) {
            last = x;
            x += x + ONE;
        }
        return last;
    }

    // Function to do the actual computation of the value.
    static constexpr Float compute_value() {
        constexpr Float LIMIT = static_cast<Float>(std::numeric_limits<Int>::max());
        constexpr Float MAX_MANTISSA = max_mantissa();

        if (LIMIT <= MAX_MANTISSA) {
            return LIMIT;
        }

        Float x = MAX_MANTISSA;
        while (x + x < LIMIT) {
            x += x;
        }
        return x;
    }

    // `value()` implementation simply computes the result _once_ (caching it), and then returns it.
    static constexpr Float value() {
        constexpr Float FLOAT_LIMIT = compute_value();
        constexpr Float EXPLICIT_LIMIT = static_cast<Float>(UpperLimit<Int, IntLimit>::value());
        return (FLOAT_LIMIT <= EXPLICIT_LIMIT) ? FLOAT_LIMIT : EXPLICIT_LIMIT;
    }
};

// Name reads as "lowest of (limits divided by value)".  Remember that the value can be negative, so
// we just take whichever limit is smaller _after_ dividing.  And since `Abs<M>` can be assumed to
// be greater than one, we know that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct LowestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? LowerLimit<T, Limits>::value() : UpperLimit<T, Limits>::value();
        return RELEVANT_LIMIT / get_value<T>(M{});
    }
};

// Name reads as "clamp lowest of (limits times inverse value)".  First, remember that the value can
// be negative, so multiplying can sometimes switch the sign: we want whichever is smaller _after_
// that operation.  Next, this is the version for when `Abs<M>` is _smaller_ than 1, so its inverse
// can grow values, and we risk overflow.  Therefore, we have to start from the bounds of the type,
// and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr T RELEVANT_LIMIT =
            IsPositive<M>::value ? LowerLimit<T, Limits>::value() : -UpperLimit<T, Limits>::value();
        constexpr T ABS_DIVISOR = get_value<T>(MagInverseT<Abs<M>>{});

        constexpr bool IS_CLAMPABLE =
            std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_bounded;
        if (!IS_CLAMPABLE) {
            return RELEVANT_LIMIT * ABS_DIVISOR;
        }

        constexpr T RELEVANT_BOUND = IsPositive<M>::value
                                         ? (std::numeric_limits<T>::lowest() / ABS_DIVISOR)
                                         : -(std::numeric_limits<T>::max() / ABS_DIVISOR);
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND >= RELEVANT_LIMIT;
        return SHOULD_CLAMP ? std::numeric_limits<T>::lowest() : RELEVANT_LIMIT * ABS_DIVISOR;
    }
};

// Name reads as "highest of (limits divided by value)".  Remember that the value can be negative,
// so we just take whichever limit is larger _after_ dividing.  And since `Abs<M>` can be assumed to
// be greater than one, we know that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct HighestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? UpperLimit<T, Limits>::value() : LowerLimit<T, Limits>::value();

        // Special handling for signed int min being slightly more negative than max is positive.
        if (get_value<T>(M{}) == std::numeric_limits<T>::lowest()) {
            return T{1};
        }
        if (M{} == -mag<1>() &&
            (LowerLimit<T, Limits>::value() == std::numeric_limits<T>::lowest())) {
            // We are implicitly assuming that _unsigned_ types would have already been handled on a
            // different branch.
            return std::numeric_limits<T>::max();
        }

        return RELEVANT_LIMIT / get_value<T>(M{});
    }
};

// Name reads as "clamp highest of (limits times inverse value)".  First, remember that the value
// can be negative, so multiplying can sometimes switch the sign: we want whichever is larger
// _after_ that operation.  Next, this is the version for when `Abs<M>` is _smaller_ than 1, which
// means its inverse can grow values, and we risk overflow.  Therefore, we have to start from the
// bounds of the type, and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValue {
    static constexpr T value() {
        constexpr T RELEVANT_LIMIT =
            IsPositive<M>::value ? UpperLimit<T, Limits>::value() : -LowerLimit<T, Limits>::value();
        constexpr T ABS_DIVISOR = get_value<T>(MagInverseT<Abs<M>>{});

        constexpr bool IS_CLAMPABLE =
            std::numeric_limits<T>::is_specialized && std::numeric_limits<T>::is_bounded;
        if (!IS_CLAMPABLE) {
            return RELEVANT_LIMIT * ABS_DIVISOR;
        }

        constexpr T RELEVANT_BOUND = IsPositive<M>::value
                                         ? (std::numeric_limits<T>::max() / ABS_DIVISOR)
                                         : -(std::numeric_limits<T>::lowest() / ABS_DIVISOR);
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND <= RELEVANT_LIMIT;
        return SHOULD_CLAMP ? std::numeric_limits<T>::max() : RELEVANT_LIMIT * ABS_DIVISOR;
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

template <typename T, typename U>
struct OpInputImpl<StaticCast<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<StaticCast<T, U>> : stdx::type_identity<U> {};

//
// `MinGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (S) -> (S)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToSigned
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueIsSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueIsLowestInDestination<T, U, ULimit>> {};

// (S) -> (I)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToIntegral
    : std::conditional_t<std::is_unsigned<U>::value,
                         stdx::type_identity<ValueIsZero<T>>,
                         MinGoodImplForStaticCastFromSignedToSigned<T, U, ULimit>> {};

// (S) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromSignedToArithmetic
    : std::conditional_t<
          std::is_floating_point<U>::value,
          stdx::type_identity<ValueIsSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToIntegral<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_unsigned<T>::value,
          stdx::type_identity<ValueIsSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>>,
          MinGoodImplForStaticCastFromSignedToArithmetic<T, U, ULimit>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueIsSourceLowestUnlessDestLimitIsHigher<T, U, ULimit>,
                       ValueIsLowestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MinGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueIsLowestInDestination<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MinGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MinGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MinGoodImpl<StaticCast<T, U>, ULimit>
    : std::conditional_t<std::is_arithmetic<T>::value,
                         MinGoodImplForStaticCastFromArithmetic<T, U, ULimit>,
                         MinGoodImplForStaticCastFromNonArithmetic<T, U, ULimit>> {};

//
// `MaxGood<StaticCast<T, U>>` implementation cluster.
//
// See comment above for meanings of (N), (X), (A), etc.
//

// (N) -> (X) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (A) -> (N) (placeholder)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToNonArithmetic
    : OverflowBoundaryNotYetImplemented<StaticCast<T, U>> {};

// (I) -> (I)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToIntegral
    : std::conditional<(static_cast<std::common_type_t<T, U>>(std::numeric_limits<T>::max()) <=
                        static_cast<std::common_type_t<T, U>>(std::numeric_limits<U>::max())),
                       ValueIsSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueIsHighestInDestination<T, U, ULimit>> {};

// (I) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromIntegralToArithmetic
    : std::conditional_t<
          std::is_integral<U>::value,
          MaxGoodImplForStaticCastFromIntegralToIntegral<T, U, ULimit>,
          stdx::type_identity<ValueIsSourceHighestUnlessDestLimitIsLower<T, U, ULimit>>> {};

// (F) -> (F)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint
    : std::conditional<sizeof(T) <= sizeof(U),
                       ValueIsSourceHighestUnlessDestLimitIsLower<T, U, ULimit>,
                       ValueIsHighestInDestination<T, U, ULimit>> {};

// (F) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromFloatingPointToArithmetic
    : std::conditional_t<std::is_floating_point<U>::value,
                         MaxGoodImplForStaticCastFromFloatingPointToFloatingPoint<T, U, ULimit>,
                         stdx::type_identity<ValueIsMaxFloatNotExceedingMaxInt<T, U, ULimit>>> {};

// (A) -> (A)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmeticToArithmetic
    : std::conditional_t<std::is_integral<T>::value,
                         MaxGoodImplForStaticCastFromIntegralToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromFloatingPointToArithmetic<T, U, ULimit>> {};

// (A) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImplForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         MaxGoodImplForStaticCastFromArithmeticToArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromArithmeticToNonArithmetic<T, U, ULimit>> {};

// (X) -> (X)
template <typename T, typename U, typename ULimit>
struct MaxGoodImpl<StaticCast<T, U>, ULimit>
    : std::conditional_t<std::is_arithmetic<T>::value,
                         MaxGoodImplForStaticCastFromArithmetic<T, U, ULimit>,
                         MaxGoodImplForStaticCastFromNonArithmetic<T, U, ULimit>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

//
// `MinGood<StaticCast<T, U>>` implementation cluster.
//

// Assume `T` is non-integral, for now.  If `T` were integral, then (for now) we would only need to
// cover multiplying or dividing by other integers, and other structs below cover these.
template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByNeitherIntNorInverseInt
    : std::conditional<(get_value<T>(Abs<M>{}) > T{1}),
                       LowestOfLimitsDividedByValue<T, M, Limits>,
                       ClampLowestOfLimitsTimesInverseValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByNonInteger
    : std::conditional_t<IsInteger<MagInverseT<M>>::value,
                         stdx::type_identity<ClampLowestOfLimitsTimesInverseValue<T, M, Limits>>,
                         MinGoodImplForMultiplyTypeByNeitherIntNorInverseInt<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByAssumingSigned
    : std::conditional_t<IsInteger<M>::value,
                         stdx::type_identity<LowestOfLimitsDividedByValue<T, M, Limits>>,
                         MinGoodImplForMultiplyTypeByNonInteger<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : std::conditional_t<is_definitely_unsigned<T>(),
                         stdx::type_identity<ValueIsZero<T>>,
                         MinGoodImplForMultiplyTypeByAssumingSigned<T, M, Limits>> {};

//
// `MaxGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByNeitherIntNorInverseInt
    : std::conditional<(get_value<T>(Abs<M>{}) > T{1}),
                       HighestOfLimitsDividedByValue<T, M, Limits>,
                       ClampHighestOfLimitsTimesInverseValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByNonInteger
    : std::conditional_t<IsInteger<MagInverseT<M>>::value,
                         stdx::type_identity<ClampHighestOfLimitsTimesInverseValue<T, M, Limits>>,
                         MaxGoodImplForMultiplyTypeByNeitherIntNorInverseInt<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor
    : std::conditional_t<IsInteger<M>::value,
                         stdx::type_identity<HighestOfLimitsDividedByValue<T, M, Limits>>,
                         MaxGoodImplForMultiplyTypeByNonInteger<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : std::conditional_t<
          (is_definitely_unsigned<T>() && !IsPositive<M>::value),
          stdx::type_identity<ValueIsZero<T>>,
          MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

template <typename Op, typename... Ops>
struct OpInputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpInput<Op>> {};

template <typename Op, typename... Ops>
struct OpOutputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpOutput<OpSequence<Ops...>>> {};
template <typename OnlyOp>
struct OpOutputImpl<OpSequence<OnlyOp>> : stdx::type_identity<OpOutput<OnlyOp>> {};

//
// `MinGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MinGoodImpl<OpSequence<OnlyOp>, Limits> : stdx::type_identity<MinGood<OnlyOp, Limits>> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MinGoodImpl<OpSequence<Op1, Op2, Ops...>, Limits>
    : stdx::type_identity<MinGood<Op1, LimitsFor<OpSequence<Op2, Ops...>, Limits>>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

//
// `MaxGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MaxGoodImpl<OpSequence<OnlyOp>, Limits> : stdx::type_identity<MaxGood<OnlyOp, Limits>> {};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MaxGoodImpl<OpSequence<Op1, Op2, Ops...>, Limits>
    : stdx::type_identity<MaxGood<Op1, LimitsFor<OpSequence<Op2, Ops...>, Limits>>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

}  // namespace detail
}  // namespace au

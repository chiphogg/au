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

#include "au/abstract_operations.hh"
#include "au/magnitude.hh"
#include "au/stdx/type_traits.hh"

namespace au {
namespace detail {

//
// `MinPossible<Op>::value()` is the smallest allowable "scalar type" for `OpInput<Op>`.
//
// The "scalar type" of `T` is usually just `T`, but if `T` something like `std::complex<U>`, or
// `Eigen::Vector<U, N>`, then it would be `U`.
//
template <typename Op>
struct MinPossibleImpl;
template <typename Op>
using MinPossible = typename MinPossibleImpl<Op>::type;

//
// `MaxPossible<Op>::value()` is the largest allowable "scalar type" for `OpInput<Op>`.  (See
// `MinPossible` above for what we mean by "scalar type".)
//
template <typename Op>
struct MaxPossibleImpl;
template <typename Op>
using MaxPossible = typename MaxPossibleImpl<Op>::type;

//
// `MinGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// minimum value that does not overflow.
//
// IMPORTANT: the result must always be non-positive.  The code is structured on this assumption.
//
template <typename Op, typename Limits>
struct MinGoodImpl;
template <typename Op, typename Limits = void>
using MinGood = typename MinGoodImpl<Op, Limits>::type;

//
// `MaxGood<Op>::value()` is a constexpr constant of the "scalar type" for `OpInput<Op>` that is the
// maximum value that does not overflow.
//
// IMPORTANT: the result must always be non-negative.  The code is structured on this assumption.
//
template <typename Op, typename Limits = void>
struct MaxGoodImpl;
template <typename Op, typename Limits = void>
using MaxGood = typename MaxGoodImpl<Op, Limits>::type;

//
// `CanOverflowBelow<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowBelow;

//
// `CanOverflowAbove<Op>::value` is `true` if there is any value in `OpInput<Op>` that can cause the
// operation to exceed its bounds.
//
template <typename Op>
struct CanOverflowAbove;

// `MinValueChecker<Op>::is_too_small(x)` checks whether the value `x` is small enough to overflow
// the bounds of the operation.
template <typename Op>
struct MinValueChecker;

// `MaxValueChecker<Op>::is_too_large(x)` checks whether the value `x` is large enough to overflow
// the bounds of the operation.
template <typename Op>
struct MaxValueChecker;

// `would_input_produce_overflow<Op>(x)` checks whether the value `x` would exceed the bounds of the
// operation at any stage.
template <typename Op>
constexpr bool would_input_produce_overflow(const OpInput<Op> &x) {
    return MinValueChecker<Op>::is_too_small(x) || MaxValueChecker<Op>::is_too_large(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
using IsClampable = stdx::conjunction<stdx::bool_constant<(std::numeric_limits<T>::is_specialized)>,
                                      stdx::bool_constant<(std::numeric_limits<T>::is_bounded)>>;

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

// `NegativeLowerLimit<T, Limits>::value()` returns `-LowerLimit<T, Limits>::value()`, unless
// `LowerLimit<T, Limits>::value()` is `std::numeric_limits<T>::lowest()` _and_ `T` is a signed
// integral type, in which case it returns `std::numeric_limits<T>::max()`.
template <bool ShouldJustUseMax, typename T, typename Limits>
struct NegativeLowerLimitImplIfShouldJustUseMaxIs {
    static constexpr T value() { return -LowerLimit<T, Limits>::value(); }
};
template <typename T, typename Limits>
struct NegativeLowerLimitImplIfShouldJustUseMaxIs<true, T, Limits> {
    static constexpr T value() { return std::numeric_limits<T>::max(); }
};
template <typename T, typename Limits>
struct NegativeLowerLimit
    : NegativeLowerLimitImplIfShouldJustUseMaxIs<
          stdx::conjunction<std::is_signed<T>,
                            std::is_integral<T>,
                            stdx::bool_constant<(LowerLimit<T, Limits>::value() ==
                                                 std::numeric_limits<T>::lowest())>>::value,
          T,
          Limits> {};

// `LimitsFor<Op, Limits>` produces a type which can be the `Limits` argument for some other op.
template <typename Op, typename Limits>
struct LimitsFor {
    static constexpr RealPart<OpInput<Op>> lower() { return MinGood<Op, Limits>::value(); }
    static constexpr RealPart<OpInput<Op>> upper() { return MaxGood<Op, Limits>::value(); }
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

template <typename T, typename MagT, MagRepresentationOutcome>
struct MagHelper {
    static constexpr bool equal(const T &, const T &) { return false; }
    static constexpr T div(const T &, const T &) {
        // We assume the most likely reason to be here is that the magnitude was too large to
        // represent in the type, so we treat this as a "divide by infinity" and return zero.
        //
        // Really, this should probably just not be called, but it definitely needs to exist for the
        // code to compile.
        return T{0};
    }
};

template <typename T, typename MagT>
struct MagHelper<T, MagT, MagRepresentationOutcome::OK> {
    static constexpr bool equal(const T &x, const T &value) { return x == value; }
    static constexpr T div(const T &a, const T &b) { return a / b; }
};

template <typename T, typename... BPs>
constexpr T divide_by_mag(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::div(x, result.value);
}

// Name reads as "lowest of (limits divided by value)".  Remember that the value can be negative, so
// we just take whichever limit is smaller _after_ dividing.  And since `Abs<M>` can be assumed to
// be greater than one, we know that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct LowestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? LowerLimit<T, Limits>::value() : UpperLimit<T, Limits>::value();

        return divide_by_mag(RELEVANT_LIMIT, M{});
    }
};

// This section is for implementing `ClampLowestOfLimitsTimesInverseValue<T, M, Limits>`, including
// various helper types.
//
// Read the name as "clamp lowest of (limits times inverse value)".  First, remember that the value
// can be negative, so multiplying can sometimes switch the sign: we want whichever is smaller
// _after_ that operation.  Next, this is the version for when `Abs<M>` is _smaller_ than 1, so its
// inverse can grow values, and we risk overflow.  Therefore, we have to start from the bounds of
// the type, and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct CommonVarsForClampLowest {
    static constexpr T RELEVANT_LIMIT =
        IsPositive<M>::value ? LowerLimit<T, Limits>::value() : -UpperLimit<T, Limits>::value();
    static constexpr MagRepresentationOrError<T> ABS_DIVISOR_RESULT =
        get_value_result<T>(MagInverseT<Abs<M>>{});
};

template <bool IsClampable, typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValueIfClampableIs : CommonVarsForClampLowest<T, M, Limits> {
    static_assert(IsClampable, "Wrong specialization (internal library error)");

    using CommonVarsForClampLowest<T, M, Limits>::RELEVANT_LIMIT;
    using CommonVarsForClampLowest<T, M, Limits>::ABS_DIVISOR_RESULT;

    static constexpr T value() {
        if (ABS_DIVISOR_RESULT.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT) {
            return std::numeric_limits<T>::lowest();
        }

        // It should be impossible to actually _use_ `T{1}`.  However, it appears (somewhat
        // surprisingly) that the compiler will try to perform the division by `ABS_DIVISOR` _even
        // when the logic does not take that branch_.  Therefore, we need to give a nonzero value.
        constexpr T ABS_DIVISOR = ABS_DIVISOR_RESULT.outcome == MagRepresentationOutcome::OK
                                      ? ABS_DIVISOR_RESULT.value
                                      : T{1};

        constexpr T RELEVANT_BOUND = IsPositive<M>::value
                                         ? (std::numeric_limits<T>::lowest() / ABS_DIVISOR)
                                         : -(std::numeric_limits<T>::max() / ABS_DIVISOR);
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND >= RELEVANT_LIMIT;
        return SHOULD_CLAMP ? std::numeric_limits<T>::lowest() : RELEVANT_LIMIT * ABS_DIVISOR;
    }
};

template <typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValueIfClampableIs<false, T, M, Limits>
    : CommonVarsForClampLowest<T, M, Limits> {
    using CommonVarsForClampLowest<T, M, Limits>::RELEVANT_LIMIT;
    using CommonVarsForClampLowest<T, M, Limits>::ABS_DIVISOR;
    static constexpr T value() { return RELEVANT_LIMIT * ABS_DIVISOR; }
};

template <typename T, typename M, typename Limits>
struct ClampLowestOfLimitsTimesInverseValue
    : ClampLowestOfLimitsTimesInverseValueIfClampableIs<IsClampable<T>::value, T, M, Limits> {};

template <typename T, typename... BPs>
constexpr bool mag_representation_equals(const T &x, Magnitude<BPs...> m) {
    constexpr auto result = get_value_result<T>(m);
    return MagHelper<T, Magnitude<BPs...>, result.outcome>::equal(x, result.value);
}

// Name reads as "highest of (limits divided by value)".  Remember that the value can be negative,
// so we just take whichever limit is larger _after_ dividing.  And since `Abs<M>` can be assumed to
// be greater than one, we know that dividing by `M` will shrink values, so we don't risk overflow.
template <typename T, typename M, typename Limits>
struct HighestOfLimitsDividedByValue {
    static constexpr T value() {
        constexpr auto RELEVANT_LIMIT =
            IsPositive<M>::value ? UpperLimit<T, Limits>::value() : LowerLimit<T, Limits>::value();

        // Special handling for signed int min being slightly more negative than max is positive.
        if (mag_representation_equals(std::numeric_limits<T>::lowest(), M{})) {
            return T{1};
        }
        if (M{} == -mag<1>() &&
            (LowerLimit<T, Limits>::value() == std::numeric_limits<T>::lowest())) {
            // We are implicitly assuming that _unsigned_ types would have already been handled on a
            // different branch.
            return std::numeric_limits<T>::max();
        }

        return divide_by_mag(RELEVANT_LIMIT, M{});
    }
};

// This section is for implementing `ClampHighestOfLimitsTimesInverseValue<T, M, Limits>`, including
// various helper types.
//
// Read the name as "clamp highest of (limits times inverse value)".  First, remember that the value
// can be negative, so multiplying can sometimes switch the sign: we want whichever is larger
// _after_ that operation.  Next, this is the version for when `Abs<M>` is _smaller_ than 1, which
// means its inverse can grow values, and we risk overflow.  Therefore, we have to start from the
// bounds of the type, and back out the most extreme value for the limit that will _not_ overflow.
template <typename T, typename M, typename Limits>
struct CommonVarsForClampHighest {
    static constexpr T RELEVANT_LIMIT = IsPositive<M>::value
                                            ? UpperLimit<T, Limits>::value()
                                            : NegativeLowerLimit<T, Limits>::value();
    static constexpr MagRepresentationOrError<T> ABS_DIVISOR_RESULT =
        get_value_result<T>(MagInverseT<Abs<M>>{});
};

template <bool IsClampable, typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValueIfClampableIs
    : CommonVarsForClampHighest<T, M, Limits> {
    static_assert(IsClampable, "Wrong specialization (internal library error)");

    using CommonVarsForClampHighest<T, M, Limits>::RELEVANT_LIMIT;
    using CommonVarsForClampHighest<T, M, Limits>::ABS_DIVISOR_RESULT;

    static constexpr T value() {
        if (ABS_DIVISOR_RESULT.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT) {
            return std::numeric_limits<T>::max();
        }

        // It should be impossible to actually _use_ `T{1}`.  However, it appears (somewhat
        // surprisingly) that the compiler will try to perform the division by `ABS_DIVISOR` _even
        // when the logic does not take that branch_.  Therefore, we need to give a nonzero value.
        constexpr T ABS_DIVISOR = ABS_DIVISOR_RESULT.outcome == MagRepresentationOutcome::OK
                                      ? ABS_DIVISOR_RESULT.value
                                      : T{1};

        constexpr T RELEVANT_BOUND = IsPositive<M>::value
                                         ? (std::numeric_limits<T>::max() / ABS_DIVISOR)
                                         : -(std::numeric_limits<T>::lowest() / ABS_DIVISOR);
        constexpr bool SHOULD_CLAMP = RELEVANT_BOUND <= RELEVANT_LIMIT;
        return SHOULD_CLAMP ? std::numeric_limits<T>::max() : RELEVANT_LIMIT * ABS_DIVISOR;
    }
};

template <typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValueIfClampableIs<false, T, M, Limits>
    : CommonVarsForClampHighest<T, M, Limits> {
    using CommonVarsForClampHighest<T, M, Limits>::RELEVANT_LIMIT;
    using CommonVarsForClampHighest<T, M, Limits>::ABS_DIVISOR;
    static constexpr T value() { return RELEVANT_LIMIT * ABS_DIVISOR; }
};

template <typename T, typename M, typename Limits>
struct ClampHighestOfLimitsTimesInverseValue
    : ClampHighestOfLimitsTimesInverseValueIfClampableIs<IsClampable<T>::value, T, M, Limits> {};

constexpr bool is_ok_or_err_cannot_fit(MagRepresentationOutcome outcome) {
    return outcome == MagRepresentationOutcome::OK ||
           outcome == MagRepresentationOutcome::ERR_CANNOT_FIT;
}

template <typename T, typename M>
struct IsCompatibleApartFromMaybeOverflow
    : stdx::bool_constant<is_ok_or_err_cannot_fit(get_value_result<T>(M{}).outcome)> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinPossible<Op>` implementation.

template <typename Op>
struct MinPossibleImpl
    : stdx::type_identity<LowestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MaxPossible<Op>` implementation.

template <typename Op>
struct MaxPossibleImpl
    : stdx::type_identity<HighestOfLimitsDividedByValue<RealPart<OpInput<Op>>, Magnitude<>, void>> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

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
struct MinGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MinGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MinGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MinGoodImpl<StaticCast<T, U>, ULimit> : MinGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

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
struct MaxGoodImplForStaticCastUsingRealPart
    : std::conditional_t<
          std::is_arithmetic<RealPart<T>>::value,
          MaxGoodImplForStaticCastFromArithmetic<RealPart<T>, RealPart<U>, ULimit>,
          MaxGoodImplForStaticCastFromNonArithmetic<RealPart<T>, RealPart<U>, ULimit>> {};

template <typename T, typename U, typename ULimit>
struct MaxGoodImpl<StaticCast<T, U>, ULimit> : MaxGoodImplForStaticCastUsingRealPart<T, U, ULimit> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

//
// `MinGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M>
constexpr bool abs_is_probably_bigger_than_one(M) {
    constexpr auto abs_value_result = get_value_result<T>(Abs<M>{});
    return abs_value_result.outcome == MagRepresentationOutcome::ERR_CANNOT_FIT ||
           abs_value_result.value >= T{1};
}

// Assume `T` is non-integral, for now.  If `T` were integral, then (for now) we would only need to
// cover multiplying or dividing by other integers, and other structs below cover these.
template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyCompatibleTypeBy
    : std::conditional<abs_is_probably_bigger_than_one<T>(M{}),
                       LowestOfLimitsDividedByValue<T, M, Limits>,
                       ClampLowestOfLimitsTimesInverseValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByAssumingSigned
    : std::conditional_t<IsCompatibleApartFromMaybeOverflow<T, M>::value,
                         MinGoodImplForMultiplyCompatibleTypeBy<T, M, Limits>,
                         stdx::type_identity<ValueIsZero<T>>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<is_definitely_unsigned<T>(),
                         stdx::type_identity<ValueIsZero<T>>,
                         MinGoodImplForMultiplyTypeByAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MinGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<MultiplyTypeBy<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyCompatibleTypeByNeitherIntNorInverseInt
    : std::conditional<abs_is_probably_bigger_than_one<T>(M{}),
                       HighestOfLimitsDividedByValue<T, M, Limits>,
                       ClampHighestOfLimitsTimesInverseValue<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImplForMultiplyTypeByNeitherIntNorInverseInt
    : std::conditional_t<
          IsCompatibleApartFromMaybeOverflow<T, M>::value,
          MaxGoodImplForMultiplyCompatibleTypeByNeitherIntNorInverseInt<T, M, Limits>,
          stdx::type_identity<ValueIsZero<T>>> {};

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
struct MaxGoodImplForMultiplyTypeByUsingRealPart
    : std::conditional_t<
          (is_definitely_unsigned<T>() && !IsPositive<M>::value),
          stdx::type_identity<ValueIsZero<T>>,
          MaxGoodImplForMultiplyTypeByAssumingSignedTypeOrPositiveFactor<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<MultiplyTypeBy<T, M>, Limits>
    : MaxGoodImplForMultiplyTypeByUsingRealPart<RealPart<T>, M, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `DivideTypeByInteger<T, M>` implementation.

//
// `MinGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerAssumingSigned
    : stdx::type_identity<ClampLowestOfLimitsTimesInverseValue<T, MagInverseT<M>, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImplForDivideTypeByIntegerUsingRealPart
    : std::conditional_t<is_definitely_unsigned<T>(),
                         stdx::type_identity<ValueIsZero<T>>,
                         MinGoodImplForDivideTypeByIntegerAssumingSigned<T, M, Limits>> {};

template <typename T, typename M, typename Limits>
struct MinGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MinGoodImplForDivideTypeByIntegerUsingRealPart<RealPart<T>, M, Limits> {};

//
// `MaxGood<DivideTypeByInteger<T, M>>` implementation cluster.
//

template <typename T, typename M, typename Limits>
struct MaxGoodImpl<DivideTypeByInteger<T, M>, Limits>
    : MaxGoodImpl<MultiplyTypeBy<T, MagInverseT<M>>, Limits> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

//
// `MinGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MinGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : stdx::type_identity<MinGood<OnlyOp, Limits>> {
};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MinGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : stdx::type_identity<MinGood<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

//
// `MaxGood<OpSequence<Ops...>>` implementation cluster.
//

template <typename OnlyOp, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<OnlyOp>, Limits> : stdx::type_identity<MaxGood<OnlyOp, Limits>> {
};

template <typename Op1, typename Op2, typename... Ops, typename Limits>
struct MaxGoodImpl<OpSequenceImpl<Op1, Op2, Ops...>, Limits>
    : stdx::type_identity<MaxGood<Op1, LimitsFor<OpSequenceImpl<Op2, Ops...>, Limits>>> {
    static_assert(std::is_same<OpOutput<Op1>, OpInput<Op2>>::value,
                  "Output of each op in sequence must match input of next op");
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowBelow<Op>` implementation.

template <typename Op>
struct CanOverflowBelow : stdx::bool_constant<(MinGood<Op>::value() > MinPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `CanOverflowAbove<Op>` implementation.

template <typename Op>
struct CanOverflowAbove : stdx::bool_constant<(MaxGood<Op>::value() < MaxPossible<Op>::value())> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MinValueChecker<Op>` and `MaxValueChecker<Op>` implementation.

template <typename Op, bool IsOverflowPossible>
struct MinValueCheckerImpl {
    static constexpr bool is_too_small(const OpInput<Op> &x) { return x < MinGood<Op>::value(); }
};
template <typename Op>
struct MinValueCheckerImpl<Op, false> {
    static constexpr bool is_too_small(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MinValueChecker : MinValueCheckerImpl<Op, CanOverflowBelow<Op>::value> {};

template <typename Op, bool IsOverflowPossible>
struct MaxValueCheckerImpl {
    static constexpr bool is_too_large(const OpInput<Op> &x) { return x > MaxGood<Op>::value(); }
};
template <typename Op>
struct MaxValueCheckerImpl<Op, false> {
    static constexpr bool is_too_large(const OpInput<Op> &) { return false; }
};
template <typename Op>
struct MaxValueChecker : MaxValueCheckerImpl<Op, CanOverflowAbove<Op>::value> {};

}  // namespace detail
}  // namespace au

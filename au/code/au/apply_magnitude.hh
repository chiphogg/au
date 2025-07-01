// Copyright 2023 Aurora Operations, Inc.
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

#include "au/apply_rational_magnitude_to_integral.hh"
#include "au/magnitude.hh"
#include "au/overflow_boundary.hh"
#include "au/truncation_risk.hh"

namespace au {
namespace detail {

// `MinValueChecker<Op>::is_too_small(x)` checks whether the value `x` is small enough to overflow
// the bounds of the operation.
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

// `MaxValueChecker<Op>::is_too_large(x)` checks whether the value `x` is large enough to overflow
// the bounds of the operation.
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

template <typename Op>
constexpr bool would_input_produce_overflow(const OpInput<Op> &x) {
    return MinValueChecker<Op>::is_too_small(x) || MaxValueChecker<Op>::is_too_large(x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Newer ideas below
////////////////////////////////////////////////////////////////////////////////////////////////////

//
// `ApplicationStrategyFor`
//

enum class MagKind {
    DEFAULT,
    NONTRIVIAL_RATIONAL,
};

template <typename M>
constexpr MagKind mag_kind_for(M) {
    return stdx::disjunction<IsInteger<M>,
                             IsInteger<MagInverseT<M>>,
                             stdx::negation<IsRational<M>>>::value
               ? MagKind::DEFAULT
               : MagKind::NONTRIVIAL_RATIONAL;
}

template <typename T, typename Mag, MagKind>
struct ApplicationStrategyForImpl : stdx::type_identity<MultiplyTypeBy<T, Mag>> {};
template <typename T, typename Mag>
using ApplicationStrategyFor =
    typename ApplicationStrategyForImpl<T, Mag, mag_kind_for(Mag{})>::type;

template <typename T, typename Mag>
struct ApplicationStrategyForImpl<T, Mag, MagKind::NONTRIVIAL_RATIONAL>
    : std::conditional<
          std::is_integral<T>::value,
          OpSequence<MultiplyTypeBy<T, NumeratorT<Mag>>, DivideTypeBy<T, DenominatorT<Mag>>>,
          MultiplyTypeBy<T, Mag>> {};

//
// more
//

//
// `ConversionForRepsAndFactor<OldRep, NewRep, Factor>` is the operation that takes a value of
// `OldRep`, and produces the application of the magnitude `Factor` in the type `NewRep`.
//
template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl;
template <typename OldRep, typename NewRep, typename Factor>
using ConversionForRepsAndFactor =
    typename ConversionForRepsAndFactorImpl<OldRep, NewRep, Factor>::type;

template <typename OldRep, typename PromotedCommon, typename NewRep, typename Factor>
struct FullConversionImpl
    : stdx::type_identity<OpSequence<StaticCast<OldRep, PromotedCommon>,
                                     ApplicationStrategyFor<PromotedCommon, Factor>,
                                     StaticCast<PromotedCommon, NewRep>>> {};

template <typename OldRepIsPromotedCommon, typename NewRep, typename Factor>
struct FullConversionImpl<OldRepIsPromotedCommon, OldRepIsPromotedCommon, NewRep, Factor>
    : stdx::type_identity<OpSequence<ApplicationStrategyFor<OldRepIsPromotedCommon, Factor>,
                                     StaticCast<OldRepIsPromotedCommon, NewRep>>> {};

template <typename OldRep, typename NewRepIsPromotedCommon, typename Factor>
struct FullConversionImpl<OldRep, NewRepIsPromotedCommon, NewRepIsPromotedCommon, Factor>
    : stdx::type_identity<OpSequence<StaticCast<OldRep, NewRepIsPromotedCommon>,
                                     ApplicationStrategyFor<NewRepIsPromotedCommon, Factor>>> {};

template <typename Rep, typename Factor>
struct FullConversionImpl<Rep, Rep, Rep, Factor>
    : stdx::type_identity<ApplicationStrategyFor<Rep, Factor>> {};

template <typename OldRep, typename NewRep, typename Factor>
struct ConversionForRepsAndFactorImpl
    : FullConversionImpl<OldRep, PromotedType<std::common_type_t<OldRep, NewRep>>, NewRep, Factor> {
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// New versions of old ideas
////////////////////////////////////////////////////////////////////////////////////////////////////
}  // namespace detail
}  // namespace au

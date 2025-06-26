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

#include "au/magnitude.hh"

namespace au {
namespace detail {

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

//
// `StaticCast<T, U>` represents an operation that converts from `T` to `U` via `static_cast`.
//
template <typename T, typename U>
struct StaticCast;

//
// `MultiplyTypeBy<T, M>` represents an operation that multiplies a value of type `T` by the
// magnitude `M`.
//
// Note that this operation does *not* model integer promotion.  It will always force the result to
// be `T`.  To model integer promotion, form a compound operation with `OpSequence` that includes
// appropriate `StaticCast`.
//
template <typename T, typename M>
struct MultiplyTypeBy;
template <typename T, typename M>
using DivideTypeBy = MultiplyTypeBy<T, MagInverseT<M>>;

//
// `OpSequence<Ops...>` represents an ordered sequence of operations.
//
// We require that the output type of each operation is the same as the input type of the next one
// (see below for `OpInput` and `OpOutput`).
//
template <typename... Ops>
struct OpSequence;

//
// `MultiplyWithPromotionAndStaticCast<T, M, U>` represents a sequence of operations:
//
// 1. Static cast from `T` to the "promoted type" `P` of `T`, if it is different from `T`.
// 2. Multiply by the magnitude `M`.
// 3. Static cast from `P` (which, again, is usually just `T`) to the destination type `U`.
//
template <typename T, typename M, typename U>
struct MultiplyWithPromotionAndStaticCast;

// The various categories by which a magnitude can be applied to a numeric quantity.
enum class MagMultiplyApproach {
    MULTIPLY,
    DIVIDE_BY_INVERSE,
};

template <typename... BPs>
constexpr MagMultiplyApproach approach_for_multiplying_by_mag(Magnitude<BPs...>) {
    if (IsInteger<Magnitude<BPs...>>::value) {
        return MagMultiplyApproach::MULTIPLY;
    }

    if (IsInteger<MagInverseT<Magnitude<BPs...>>>::value) {
        return MagMultiplyApproach::DIVIDE_BY_INVERSE;
    }

    return MagMultiplyApproach::MULTIPLY;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`abstract_operations.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename U>
struct OpInputImpl<StaticCast<T, U>> : stdx::type_identity<T> {};
template <typename T, typename U>
struct OpOutputImpl<StaticCast<T, U>> : stdx::type_identity<U> {};

// `StaticCast<T, U>` operation:
template <typename T, typename U>
struct StaticCast {
    static constexpr U apply_to(T value) { return static_cast<U>(value); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` implementation.

// `OpInput` and `OpOutput`:
template <typename T, typename M>
struct OpInputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};
template <typename T, typename M>
struct OpOutputImpl<MultiplyTypeBy<T, M>> : stdx::type_identity<T> {};

// `MultiplyTypeBy<T, M>` operation:
template <typename T, typename Mag, MagMultiplyApproach>
struct MultiplyTypeByImpl {};

template <typename T, typename Mag>
struct DefaultImplementationIsMultiplyByRepresentationInType {
    static constexpr T apply_to(T value) {
        return static_cast<T>(value * get_value<RealPart<T>>(Mag{}));
    }
};

template <typename T, typename Mag>
struct MultiplyTypeByImpl<T, Mag, MagMultiplyApproach::MULTIPLY>
    : DefaultImplementationIsMultiplyByRepresentationInType<T, Mag> {};

template <typename T, typename Divisor, MagRepresentationOutcome MagOutcome>
struct DivideTypeByInt {
    static constexpr T apply_to(T value) {
        static_assert(MagOutcome == MagRepresentationOutcome::OK);
        return static_cast<T>(value / get_value<RealPart<T>>(Divisor{}));
    }
};

template <typename T, typename Divisor>
struct DivideTypeByInt<T, Divisor, MagRepresentationOutcome::ERR_CANNOT_FIT> {
    // If a number is too big to fit in the type, then dividing by it should produce 0.
    static constexpr T apply_to(T value) { return T{0}; }
};

template <typename T, typename Mag>
struct MultiplyTypeByImpl<T, Mag, MagMultiplyApproach::DIVIDE_BY_INVERSE>
    : DivideTypeByInt<T,
                      MagInverseT<Mag>,
                      get_value_result<RealPart<T>>(MagInverseT<Mag>{}).outcome> {};

template <typename T, typename M>
struct MultiplyTypeBy : MultiplyTypeByImpl<T, M, approach_for_multiplying_by_mag(M{})> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<Ops...>` implementation.

// `OpInput`:
template <typename Op, typename... Ops>
struct OpInputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpInput<Op>> {};

// `OpOutput`:
template <typename Op, typename... Ops>
struct OpOutputImpl<OpSequence<Op, Ops...>> : stdx::type_identity<OpOutput<OpSequence<Ops...>>> {};
template <typename OnlyOp>
struct OpOutputImpl<OpSequence<OnlyOp>> : stdx::type_identity<OpOutput<OnlyOp>> {};

template <typename Op>
struct OpSequence<Op> {
    static constexpr auto apply_to(OpInput<OpSequence> value) { return Op::apply_to(value); }
};

template <typename Op, typename... Ops>
struct OpSequence<Op, Ops...> {
    static constexpr auto apply_to(OpInput<OpSequence> value) {
        return OpSequence<Ops...>::apply_to(Op::apply_to(value));
    }
};

}  // namespace detail
}  // namespace au

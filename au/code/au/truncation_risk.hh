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

#include "au/abstract_operations.hh"

namespace au {
namespace detail {

template <typename Op>
struct TruncationRiskForImpl;
template <typename Op>
using TruncationRiskFor = typename TruncationRiskForImpl<Op>::type;

template <typename T>
struct NoTruncationRisk {};

template <typename T>
struct AllNonzeroValues {};

template <typename T>
struct NonIntegerValues {};

template <typename T, typename M>
struct ValuesNotDivisibleBy {};

template <typename T, typename Op>
struct CannotAssessTruncationRiskFor {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION DETAILS (`truncation_risk.hh`):
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast<T, U>` section:

// (A) -> (A)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmeticToArithmetic
    : std::conditional<stdx::conjunction<std::is_floating_point<T>, std::is_integral<U>>::value,
                       NonIntegerValues<T>,
                       NoTruncationRisk<T>> {};

// (A) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromArithmetic
    : std::conditional_t<std::is_arithmetic<U>::value,
                         TruncationRiskForStaticCastFromArithmeticToArithmetic<T, U>,
                         stdx::type_identity<CannotAssessTruncationRiskFor<T, StaticCast<T, U>>>> {
};

// (N) -> (X)
template <typename T, typename U>
struct TruncationRiskForStaticCastFromNonArithmetic
    : stdx::type_identity<CannotAssessTruncationRiskFor<T, StaticCast<T, U>>> {};

// (X) -> (X)
template <typename T, typename U>
struct TruncationRiskForImpl<StaticCast<T, U>>
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForStaticCastFromArithmetic<T, U>,
                         TruncationRiskForStaticCastFromNonArithmetic<T, U>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy<T, M>` section:

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByNonIntegerRational
    : std::conditional<std::is_integral<T>::value,
                       ValuesNotDivisibleBy<T, DenominatorT<M>>,
                       NoTruncationRisk<T>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyNonArithmeticByNonIntegerRational
    : stdx::type_identity<CannotAssessTruncationRiskFor<T, MultiplyTypeBy<T, M>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyArithmeticByIrrational
    : std::conditional<std::is_integral<T>::value, AllNonzeroValues<T>, NoTruncationRisk<T>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyNonArithmeticByIrrational
    : stdx::type_identity<CannotAssessTruncationRiskFor<T, MultiplyTypeBy<T, M>>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByNonIntegerRational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByNonIntegerRational<T, M>,
                         TruncationRiskForMultiplyNonArithmeticByNonIntegerRational<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByRational
    : std::conditional_t<IsInteger<M>::value,
                         stdx::type_identity<NoTruncationRisk<T>>,
                         TruncationRiskForMultiplyByNonIntegerRational<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForMultiplyByIrrational
    : std::conditional_t<std::is_arithmetic<T>::value,
                         TruncationRiskForMultiplyArithmeticByIrrational<T, M>,
                         TruncationRiskForMultiplyNonArithmeticByIrrational<T, M>> {};

template <typename T, typename M>
struct TruncationRiskForImpl<MultiplyTypeBy<T, M>>
    : std::conditional_t<IsRational<M>::value,
                         TruncationRiskForMultiplyByRational<T, M>,
                         TruncationRiskForMultiplyByIrrational<T, M>> {};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence<...>` section:

template <typename Op1, typename Op2>
struct PrependToOpSequenceImpl : stdx::type_identity<OpSequence<Op1, Op2>> {};
template <typename Op1, typename Op2>
using PrependToOpSequence = typename PrependToOpSequenceImpl<Op1, Op2>::type;
template <typename Op1, typename... Ops>

struct PrependToOpSequenceImpl<Op1, OpSequence<Ops...>>
    : stdx::type_identity<OpSequence<Op1, Ops...>> {};

//
// `UpdateRisk<Op, Risk>` adapts a "downstream" risk to the "upstream" interface.
//
// At minimum, this updates the input type to `OpInput<Op>`.  But it may also tweak the parameters
// (e.g., for `ValuesNotDivisibleBy`), or even change the risk type entirely.
//
template <typename Op, typename Risk>
struct UpdateRiskImpl;
template <typename Op, typename Risk>
using UpdateRisk = typename UpdateRiskImpl<Op, Risk>::type;

template <template <class> class Risk, typename T, typename U>
struct UpdateRiskImpl<StaticCast<T, U>, Risk<U>> : stdx::type_identity<Risk<T>> {};

template <typename T, typename U, typename M>
struct UpdateRiskImpl<StaticCast<T, U>, ValuesNotDivisibleBy<U, M>>
    : stdx::type_identity<ValuesNotDivisibleBy<T, M>> {};

template <typename Op, typename OldOp>
struct UpdateRiskImpl<Op, CannotAssessTruncationRiskFor<OpOutput<Op>, OldOp>>
    : stdx::type_identity<
          CannotAssessTruncationRiskFor<OpInput<Op>, PrependToOpSequence<Op, OldOp>>> {};

//
// Full `TruncationRiskFor` implementation for `OpSequence<Op>`:
//

template <typename Op>
struct TruncationRiskForImpl<OpSequence<Op>> : TruncationRiskForImpl<Op> {};

template <typename Op, typename... Ops>
struct TruncationRiskForImpl<OpSequence<Op, Ops...>> {};

}  // namespace detail
}  // namespace au

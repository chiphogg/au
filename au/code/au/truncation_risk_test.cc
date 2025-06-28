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

#include "au/truncation_risk.hh"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {

template <typename T>
struct UnknownOp {};
template <typename T>
struct OpInputImpl<UnknownOp<T>> : stdx::type_identity<T> {};

namespace {

constexpr auto PI = Magnitude<Pi>{};

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast` section:

TEST(TruncationRiskFor, StaticCastArithToArithFloatAssumedToNeverTruncate) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int16_t, float>>, NoTruncationRisk<int16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint16_t, double>>,
                       NoTruncationRisk<uint16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<float, double>>, NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<long double, float>>,
                       NoTruncationRisk<long double>>();
}

TEST(TruncationRiskFor, StaticCastArithIntToArithAssumedToNeverTruncate) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int, int16_t>>, NoTruncationRisk<int>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int16_t, int>>, NoTruncationRisk<int16_t>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<int, int8_t>>, NoTruncationRisk<int>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint8_t, int>>, NoTruncationRisk<uint8_t>>();
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<uint64_t, float>>,
                       NoTruncationRisk<uint64_t>>();
}

TEST(TruncationRiskFor, StaticCastArithFloatToArithIntRisksNonIntegerValues) {
    StaticAssertTypeEq<TruncationRiskFor<StaticCast<float, int>>, NonIntegerValues<float>>();

    StaticAssertTypeEq<TruncationRiskFor<StaticCast<double, uint16_t>>, NonIntegerValues<double>>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy` section:

TEST(TruncationRiskFor, MultiplyAnythingByIntNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<int16_t, decltype(mag<2>())>>,
                       NoTruncationRisk<int16_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<uint32_t, decltype(-mag<1>())>>,
                       NoTruncationRisk<uint32_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(mag<3000>())>>,
                       NoTruncationRisk<float>>();
}

TEST(TruncationRiskFor, DividingFloatByIntNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(mag<1>() / mag<2>())>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<double, decltype(mag<1>() / mag<3456>())>>,
                       NoTruncationRisk<double>>();
}

TEST(TruncationRiskFor, MultiplyIntByIrrationalTruncatesForAllNonzeroValues) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<uint8_t, decltype(PI / mag<180>())>>,
                       AllNonzeroValues<uint8_t>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<int32_t, decltype(sqrt(mag<2>()))>>,
                       AllNonzeroValues<int32_t>>();
}

TEST(TruncationRiskFor, MultiplyFloatByIrrationalNeverTruncates) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<float, decltype(PI / mag<180>())>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<double, decltype(sqrt(mag<2>()))>>,
                       NoTruncationRisk<double>>();
}

TEST(TruncationRiskFor, DivideIntByIntTruncatesNumbersNotDivisibleByIt) {
    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<int16_t, decltype(mag<1>() / mag<3>())>>,
                       ValuesNotDivisibleBy<int16_t, decltype(mag<3>())>>();

    StaticAssertTypeEq<TruncationRiskFor<MultiplyTypeBy<uint32_t, decltype(mag<1>() / mag<432>())>>,
                       ValuesNotDivisibleBy<uint32_t, decltype(mag<432>())>>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence` section:

TEST(TruncationRiskFor, OpSequenceOfOneOpHasRiskOfThatOp) {
    StaticAssertTypeEq<TruncationRiskFor<OpSequence<StaticCast<int16_t, float>>>,
                       TruncationRiskFor<StaticCast<int16_t, float>>>();

    StaticAssertTypeEq<
        TruncationRiskFor<OpSequence<MultiplyTypeBy<int16_t, decltype(mag<1>() / mag<2>())>>>,
        TruncationRiskFor<MultiplyTypeBy<int16_t, decltype(mag<1>() / mag<2>())>>>();
}

// TEST(TruncationRiskFor, StaticCastIntToFloatToIntNeverTruncates) {
//     StaticAssertTypeEq<
//         TruncationRiskFor<OpSequence<StaticCast<int16_t, float>, StaticCast<float, int>>>,
//         NoTruncationRisk<int16_t>>();
//
//     StaticAssertTypeEq<
//         TruncationRiskFor<OpSequence<StaticCast<uint32_t, double>, StaticCast<double,
//         uint32_t>>>, NoTruncationRisk<uint32_t>>();
// }

////////////////////////////////////////////////////////////////////////////////////////////////////
// `UpdateRisk` section:

TEST(UpdateRisk, StaticCastFloatToFloatPreservesRiskButChangesInputType) {
    StaticAssertTypeEq<UpdateRisk<StaticCast<float, double>, NoTruncationRisk<double>>,
                       NoTruncationRisk<float>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<double, float>, NonIntegerValues<float>>,
                       NonIntegerValues<double>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<long double, float>, AllNonzeroValues<float>>,
                       AllNonzeroValues<long double>>();

    StaticAssertTypeEq<UpdateRisk<StaticCast<float, long double>,
                                  ValuesNotDivisibleBy<long double, decltype(mag<3>())>>,
                       ValuesNotDivisibleBy<float, decltype(mag<3>())>>();
}

TEST(UpdateRisk, AnyOpBeforeCannotAssessTruncationRiskUpdatesInputTypeAndPrependsOp) {
    using Op1 = StaticCast<float, int>;
    using Op2 = MultiplyTypeBy<int, decltype(mag<2>())>;
    using WeirdOp = UnknownOp<int>;

    StaticAssertTypeEq<UpdateRisk<Op1, CannotAssessTruncationRiskFor<int, WeirdOp>>,
                       CannotAssessTruncationRiskFor<float, OpSequence<Op1, WeirdOp>>>();

    StaticAssertTypeEq<
        UpdateRisk<Op1, CannotAssessTruncationRiskFor<int, OpSequence<Op2, WeirdOp>>>,
        CannotAssessTruncationRiskFor<float, OpSequence<Op1, Op2, WeirdOp>>>();
}

}  // namespace
}  // namespace detail
}  // namespace au

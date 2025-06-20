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

#include "au/overflow_boundary.hh"

#include "au/testing.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {
namespace {

////////////////////////////////////////////////////////////////////////////////////////////////////
// `StaticCast` section:

TEST(StaticCast, HasExpectedInputAndOutputTypes) {
    StaticAssertTypeEq<OpInput<StaticCast<int16_t, float>>, int16_t>();
    StaticAssertTypeEq<OpOutput<StaticCast<int16_t, float>>, float>();
}

//
// `MinGood`:
//

TEST(StaticCast, MinGoodIsLowestIfDestinationEqualsSource) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<float, float>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TEST(StaticCast, MinGoodIsLowestIfCastWidens) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<float, double>>::value()),
                Eq(std::numeric_limits<float>::lowest()));
}

TEST(StaticCast, MinGoodIsZeroFromAnySignedToAnyUnsigned) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, uint64_t>>::value()), SameTypeAndValue(int8_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int16_t, uint8_t>>::value()), SameTypeAndValue(int16_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int32_t, uint32_t>>::value()), SameTypeAndValue(int32_t{0}));
}

TEST(StaticCast, MinGoodIsZeroFromAnyUnsignedToAnyArithmetic) {
    EXPECT_THAT((MinGood<StaticCast<uint8_t, int64_t>>::value()), Eq(uint8_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint8_t>>::value()), Eq(uint16_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint32_t, int16_t>>::value()), Eq(uint32_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, int64_t>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, float>>::value()), Eq(uint64_t{0}));
    EXPECT_THAT((MinGood<StaticCast<uint8_t, double>>::value()), Eq(uint8_t{0}));
}

TEST(StaticCast, MinGoodIsLowestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MinGood<StaticCast<int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<StaticCast<double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::lowest())));
}

TEST(StaticCast, MinGoodIsZeroFromAnyFloatingPointToAnyUnsigned) {
    EXPECT_THAT((MinGood<StaticCast<double, uint8_t>>::value()), SameTypeAndValue(0.0));
    EXPECT_THAT((MinGood<StaticCast<float, uint64_t>>::value()), SameTypeAndValue(0.0f));
}

TEST(StaticCast, MinGoodIsLowestInDestinationFromAnyFloatingPointToAnySigned) {
    EXPECT_THAT((MinGood<StaticCast<double, int32_t>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<int32_t>::lowest())));
    EXPECT_THAT((MinGood<StaticCast<float, int64_t>>::value()),
                SameTypeAndValue(static_cast<float>(std::numeric_limits<int64_t>::lowest())));
}

TEST(StaticCast, MinGoodIsLowestFromAnySignedToAnyFloatingPoint) {
    // We could imagine some hypothetical floating point and integral types for which this is not
    // true.  But floating point is designed to cover a very wide range between its min and max
    // values, and in practice, this is true for all commonly used floating point and integral
    // types.
    EXPECT_THAT((MinGood<StaticCast<int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::lowest()));

    EXPECT_THAT((MinGood<StaticCast<int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::lowest()));
}

//
// `MaxGood`:
//

TEST(StaticCast, MaxGoodIsHighestIfDestinationEqualsSource) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, int8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint16_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<float, float>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TEST(StaticCast, MaxGoodIsHighestIfCastWidens) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, int16_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint8_t, uint16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<float, double>>::value()),
                Eq(std::numeric_limits<float>::max()));
}

TEST(StaticCast, MaxGoodIsHighestFromSignedToUnsignedOfSameSize) {
    EXPECT_THAT((MaxGood<StaticCast<int8_t, uint8_t>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int16_t, uint16_t>>::value()),
                Eq(std::numeric_limits<int16_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint32_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromUnsignedToSignedOfSameSize) {
    EXPECT_THAT((MaxGood<StaticCast<uint8_t, int8_t>>::value()),
                SameTypeAndValue(static_cast<uint8_t>(std::numeric_limits<int8_t>::max())));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, int64_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int64_t>::max())));
}

TEST(StaticCast, MaxGoodIsHighestFromAnyIntegerToAnyLargerInteger) {
    EXPECT_THAT((MaxGood<StaticCast<uint8_t, int16_t>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint64_t>>::value()),
                Eq(std::numeric_limits<int32_t>::max()));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromAnyIntegerToAnySmallerInteger) {
    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<int32_t, uint16_t>>::value()),
                SameTypeAndValue(static_cast<int32_t>(std::numeric_limits<uint16_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<uint64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<uint64_t>(std::numeric_limits<int32_t>::max())));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationWhenNarrowingToSameFamily) {
    EXPECT_THAT((MaxGood<StaticCast<uint16_t, uint8_t>>::value()),
                SameTypeAndValue(static_cast<uint16_t>(std::numeric_limits<uint8_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<int64_t, int32_t>>::value()),
                SameTypeAndValue(static_cast<int64_t>(std::numeric_limits<int32_t>::max())));
    EXPECT_THAT((MaxGood<StaticCast<double, float>>::value()),
                SameTypeAndValue(static_cast<double>(std::numeric_limits<float>::max())));
}

TEST(StaticCast, MaxGoodIsHighestInDestinationFromAnyFloatingPointToAnySmallIntegral) {
    // The precondition for this test is that the max for the (destination) integral type is
    // _exactly_ representable in the (source) floating point type.  This helper will double check
    // this assumption.
    auto expect_max_good_is_exact_representation_of_destination_int_max = [](auto float_type_val,
                                                                             auto int_type_val) {
        using Float = decltype(float_type_val);
        using Int = decltype(int_type_val);

        constexpr auto expected = static_cast<Float>(std::numeric_limits<Int>::max());

        ASSERT_THAT(static_cast<Int>(expected), Eq(std::numeric_limits<Int>::max()));
        EXPECT_THAT((MaxGood<StaticCast<Float, Int>>::value()), SameTypeAndValue(expected));
    };

    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, uint32_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(double{}, int32_t{});

    expect_max_good_is_exact_representation_of_destination_int_max(float{}, uint8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int8_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, uint16_t{});
    expect_max_good_is_exact_representation_of_destination_int_max(float{}, int16_t{});
}

TEST(StaticCast, MaxGoodIsHighestRepresentableFloatBelowCastedIntMaxForTooBigInt) {
    // `float` to 64-bit integer:
    EXPECT_THAT((MaxGood<StaticCast<float, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<float>(std::numeric_limits<int64_t>::max()), 1.0f)));
    EXPECT_THAT((MaxGood<StaticCast<float, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<float>(std::numeric_limits<uint64_t>::max()), 1.0f)));

    // `double` to 64-bit integer:
    EXPECT_THAT((MaxGood<StaticCast<double, int64_t>>::value()),
                SameTypeAndValue(
                    std::nextafter(static_cast<double>(std::numeric_limits<int64_t>::max()), 1.0)));
    EXPECT_THAT((MaxGood<StaticCast<double, uint64_t>>::value()),
                SameTypeAndValue(std::nextafter(
                    static_cast<double>(std::numeric_limits<uint64_t>::max()), 1.0)));
}

TEST(StaticCast, MaxGoodIsHighestFromAnyIntegralToAnyFloatingPoint) {
    // See comments in `MinGoodIsLowestFromAnySignedToAnyFloatingPoint` for more on the assumptions
    // we're making here.

    EXPECT_THAT((MaxGood<StaticCast<int8_t, double>>::value()),
                Eq(std::numeric_limits<int8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint8_t, double>>::value()),
                Eq(std::numeric_limits<uint8_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<int64_t, float>>::value()),
                Eq(std::numeric_limits<int64_t>::max()));

    EXPECT_THAT((MaxGood<StaticCast<uint64_t, float>>::value()),
                Eq(std::numeric_limits<uint64_t>::max()));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy` section:

TEST(MultiplyTypeBy, InputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>,
                       uint32_t>();
}

TEST(MultiplyTypeBy, OutputTypeForFloatingPointIsInputType) {
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<float, decltype(mag<2>())>>, float>();
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<double, decltype(mag<3>() / mag<4>())>>, double>();
}

TEST(MultiplyTypeBy, OutputTypeIsSubjectToIntegerPromotion) {
    using T = int8_t;
    using Promoted = decltype(int8_t{} * int8_t{});
    ASSERT_THAT((std::is_same<T, Promoted>::value), IsFalse());

    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<T, decltype(mag<2>())>>, Promoted>();
}

}  // namespace
}  // namespace detail
}  // namespace au

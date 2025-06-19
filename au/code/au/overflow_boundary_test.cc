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
using ::testing::StaticAssertTypeEq;

namespace au {
namespace detail {

// We want to be able to compare `CannotOverflow` for equality with itself and other types.  We
// could build that into the type, but we want to avoid that if we don't need it in the library
// itself.  Keeping it in the test file is a conservative move.

// `operator==`:
constexpr bool operator==(CannotOverflow, CannotOverflow) { return true; }
template <typename T>
constexpr bool operator==(CannotOverflow, const T &) {
    return false;
}
template <typename T>
constexpr bool operator==(const T &, CannotOverflow) {
    return false;
}

// `operator!=`:
constexpr bool operator!=(CannotOverflow, CannotOverflow) { return false; }
template <typename T>
constexpr bool operator!=(CannotOverflow, const T &) {
    return true;
}
template <typename T>
constexpr bool operator!=(const T &, CannotOverflow) {
    return true;
}

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

TEST(StaticCast, MinGoodIsCannotOverflowIfDestinationEqualsSource) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int8_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint16_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<float, float>>::value()), Eq(CannotOverflow{}));
}

TEST(StaticCast, MinGoodIsCannotOverflowIfCastWidens) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, int16_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint8_t, uint16_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<float, double>>::value()), Eq(CannotOverflow{}));
}

TEST(StaticCast, MinGoodIsZeroFromAnySignedToAnyUnsigned) {
    EXPECT_THAT((MinGood<StaticCast<int8_t, uint64_t>>::value()), SameTypeAndValue(int8_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int16_t, uint8_t>>::value()), SameTypeAndValue(int16_t{0}));
    EXPECT_THAT((MinGood<StaticCast<int32_t, uint32_t>>::value()), SameTypeAndValue(int32_t{0}));
}

TEST(StaticCast, MinGoodIsCannotOverflowFromAnyUnsignedToAnyArithmetic) {
    EXPECT_THAT((MinGood<StaticCast<uint8_t, int64_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint16_t, uint8_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint32_t, int16_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, int64_t>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint64_t, float>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<uint8_t, double>>::value()), Eq(CannotOverflow{}));
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

TEST(StaticCast, MinGoodIsCannotOverflowFromAnySignedToAnyFloatingPoint) {
    // We could imagine some hypothetical floating point and integral types for which this is not
    // true.  But floating point is designed to cover a very wide range between its min and max
    // values, and in practice, this is true for all commonly used floating point and integral
    // types.
    EXPECT_THAT((MinGood<StaticCast<int8_t, double>>::value()), Eq(CannotOverflow{}));
    EXPECT_THAT((MinGood<StaticCast<int64_t, float>>::value()), Eq(CannotOverflow{}));
}

}  // namespace
}  // namespace detail
}  // namespace au

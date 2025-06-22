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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

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

////////////////////////////////////////////////////////////////////////////////////////////////////
// `MultiplyTypeBy` section:

TEST(MultiplyTypeBy, InputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpInput<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>,
                       uint32_t>();
}

TEST(MultiplyTypeBy, OutputTypeIsTypeParameter) {
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<int16_t, decltype(mag<2>())>>, int16_t>();
    StaticAssertTypeEq<OpOutput<MultiplyTypeBy<double, decltype(mag<3>() / mag<4>())>>, double>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// `OpSequence` section:

TEST(OpSequence, InputTypeIsInputTypeOfFirstOperation) {
    StaticAssertTypeEq<OpInput<OpSequence<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>>,
                       uint32_t>();

    StaticAssertTypeEq<OpInput<OpSequence<StaticCast<int16_t, uint16_t>,
                                          MultiplyTypeBy<uint16_t, decltype(mag<2>())>>>,
                       int16_t>();
}

TEST(OpSequence, OutputTypeIsOutputTypeOfLastOperation) {
    StaticAssertTypeEq<
        OpOutput<OpSequence<MultiplyTypeBy<uint32_t, decltype(mag<3>() / mag<4>())>>>,
        uint32_t>();

    StaticAssertTypeEq<OpOutput<OpSequence<StaticCast<int16_t, uint16_t>,
                                           MultiplyTypeBy<uint16_t, decltype(mag<2>())>,
                                           StaticCast<uint16_t, double>>>,
                       double>();
}

}  // namespace
}  // namespace detail
}  // namespace au

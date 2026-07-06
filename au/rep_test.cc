// Copyright 2024 Aurora Operations, Inc.
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

#include "au/rep.hh"

#include <complex>
#include <cstdint>

#include "au/chrono_interop.hh"
#include "au/constant.hh"
#include "au/magnitude.hh"
#include "au/prefix.hh"
#include "au/quantity.hh"
#include "au/quantity_point.hh"
#include "au/unit_symbol.hh"
#include "au/units/liters.hh"
#include "au/units/meters.hh"
#include "au/units/miles.hh"
#include "au/units/webers.hh"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace au {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StaticAssertTypeEq;

namespace {

// A custom quantity that corresponds to `QuantityI<Meters>`.
struct MyMeters {
    int value;
};

// A custom rep with no operations defined on it.
struct IntWithNoOps {
    int value;
};

// A custom type that can left-multiply a `double`.
struct LeftMultiplyDoubleByThree {
    friend double operator*(const LeftMultiplyDoubleByThree &, double x) { return 3.0 * x; }
};

// A custom type that divides a `float` into `10.0f`.
struct DivideTenByFloat {
    friend float operator/(const DivideTenByFloat &, float x) { return 10.0f / x; }
};

}  // namespace

// Set up the correspondence between `MyMeters` and `QuantityI<Meters>`.
template <>
struct CorrespondingQuantity<MyMeters> {
    using Unit = Meters;
    using Rep = int;
};

TEST(IsValidRep, FalseForVoid) { EXPECT_THAT(IsValidRep<void>::value, IsFalse()); }

TEST(IsValidRep, TrueForArithmeticTypes) {
    EXPECT_THAT(IsValidRep<int>::value, IsTrue());
    EXPECT_THAT(IsValidRep<float>::value, IsTrue());
    EXPECT_THAT(IsValidRep<double>::value, IsTrue());
    EXPECT_THAT(IsValidRep<uint8_t>::value, IsTrue());
    EXPECT_THAT(IsValidRep<int64_t>::value, IsTrue());
}

TEST(IsValidRep, TrueForStdComplex) {
    EXPECT_THAT(IsValidRep<std::complex<float>>::value, IsTrue());
    EXPECT_THAT(IsValidRep<std::complex<uint16_t>>::value, IsTrue());
}

TEST(IsValidRep, FalseForMagnitude) {
    EXPECT_THAT(IsValidRep<decltype(mag<84>())>::value, IsFalse());
    EXPECT_THAT(IsValidRep<decltype(sqrt(Magnitude<Pi>{}))>::value, IsFalse());
}

TEST(IsValidRep, FalseForUnits) {
    EXPECT_THAT(IsValidRep<Liters>::value, IsFalse());
    EXPECT_THAT(IsValidRep<Nano<Webers>>::value, IsFalse());
}

TEST(IsValidRep, FalseForQuantity) {
    EXPECT_THAT((IsValidRep<Quantity<Milli<Liters>, int>>::value), IsFalse());
}

TEST(IsValidRep, FalseForQuantityPoint) {
    EXPECT_THAT((IsValidRep<QuantityPoint<Miles, double>>::value), IsFalse());
}

TEST(IsValidRep, FalseForConstant) {
    EXPECT_THAT(IsValidRep<decltype(make_constant(liters / mile))>::value, IsFalse());
}

TEST(IsValidRep, FalseForSymbol) { EXPECT_THAT(IsValidRep<SymbolFor<Webers>>::value, IsFalse()); }

TEST(IsValidRep, FalseForTypeWithCorrespondingQuantity) {
    EXPECT_THAT(IsValidRep<MyMeters>::value, IsFalse());
    EXPECT_THAT(IsValidRep<std::chrono::nanoseconds>::value, IsFalse());
}

TEST(IsProductValidRep, FalseIfProductDoesNotExist) {
    EXPECT_THAT((IsProductValidRep<IntWithNoOps, int>::value), IsFalse());
    EXPECT_THAT((IsProductValidRep<int, IntWithNoOps>::value), IsFalse());
}

TEST(IsProductValidRep, TrueOnlyForSideWhereProductExists) {
    ASSERT_THAT(LeftMultiplyDoubleByThree{} * 4.5, Eq(13.5));

    EXPECT_THAT((IsProductValidRep<LeftMultiplyDoubleByThree, double>::value), IsTrue());
    EXPECT_THAT((IsProductValidRep<double, LeftMultiplyDoubleByThree>::value), IsFalse());
}

TEST(IsQuotientValidRep, FalseIfQuotientDoesNotExist) {
    EXPECT_THAT((IsQuotientValidRep<IntWithNoOps, int>::value), IsFalse());
    EXPECT_THAT((IsQuotientValidRep<int, IntWithNoOps>::value), IsFalse());
}

TEST(IsQuotientValidRep, FalseIfQuotientIsQuantity) {
    // Dividing by a Quantity can complicate matters because it involves hard compiler errors when
    // that quantity has an integral rep.  Make sure we handle this gracefully.
    EXPECT_THAT((IsQuotientValidRep<int, Quantity<Miles, int>>::value), IsFalse());
}

TEST(IsQuotientValidRep, TrueOnlyForSideWhereQuotientExists) {
    ASSERT_THAT(DivideTenByFloat{} / 2.0f, Eq(5.0f));

    EXPECT_THAT((IsQuotientValidRep<float, DivideTenByFloat>::value), IsFalse());
    EXPECT_THAT((IsQuotientValidRep<DivideTenByFloat, float>::value), IsTrue());
}

namespace detail {

TEST(ResultIfNoneAreQuantity, GivesResultWhenNoneAreQuantity) {
    StaticAssertTypeEq<int, ResultIfNoneAreQuantity<std::common_type_t, int, int>>();
    StaticAssertTypeEq<std::tuple<int, double, float>,
                       ResultIfNoneAreQuantity<std::tuple, int, double, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsQuantity) {
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantity<std::common_type_t, int, Quantity<Miles, int>>>();
    StaticAssertTypeEq<void,
                       ResultIfNoneAreQuantity<std::tuple, int, Quantity<Miles, int>, float>>();
}

TEST(ResultIfNoneAreQuantity, GivesVoidWhenAnyIsCorrespondingQuantity) {
    StaticAssertTypeEq<void, ResultIfNoneAreQuantity<std::common_type_t, int, MyMeters>>();
    StaticAssertTypeEq<void, ResultIfNoneAreQuantity<std::tuple, int, std::chrono::nanoseconds>>();
}

TEST(ProductTypeOrVoid, GivesProductTypeForArithmeticInputs) {
    StaticAssertTypeEq<int, ProductTypeOrVoid<int, int>>();
}

TEST(ProductTypeOrVoid, GivesVoidForInputsWithNoProductType) {
    StaticAssertTypeEq<void, ProductTypeOrVoid<IntWithNoOps, int>>();
    StaticAssertTypeEq<void, ProductTypeOrVoid<int, IntWithNoOps>>();
}

// The blast-radius guarantee: `NormalizeRep` is the identity on every standard integer type, so no
// rep a user would normally write ever changes.
TEST(NormalizeRep, IsIdentityOnStandardIntegerTypes) {
    StaticAssertTypeEq<NormalizeRep<bool>, bool>();
    StaticAssertTypeEq<NormalizeRep<char>, char>();
    StaticAssertTypeEq<NormalizeRep<signed char>, signed char>();
    StaticAssertTypeEq<NormalizeRep<unsigned char>, unsigned char>();
    StaticAssertTypeEq<NormalizeRep<short>, short>();
    StaticAssertTypeEq<NormalizeRep<unsigned short>, unsigned short>();
    StaticAssertTypeEq<NormalizeRep<int>, int>();
    StaticAssertTypeEq<NormalizeRep<unsigned int>, unsigned int>();
    StaticAssertTypeEq<NormalizeRep<long>, long>();
    StaticAssertTypeEq<NormalizeRep<unsigned long>, unsigned long>();
    StaticAssertTypeEq<NormalizeRep<long long>, long long>();
    StaticAssertTypeEq<NormalizeRep<unsigned long long>, unsigned long long>();

    // Fixed-width aliases are just spellings of the above, so they round-trip too.
    StaticAssertTypeEq<NormalizeRep<int8_t>, int8_t>();
    StaticAssertTypeEq<NormalizeRep<uint16_t>, uint16_t>();
    StaticAssertTypeEq<NormalizeRep<int32_t>, int32_t>();
    StaticAssertTypeEq<NormalizeRep<uint64_t>, uint64_t>();
}

// Identity on non-integral reps: floating point and custom/user rep types are untouched.
TEST(NormalizeRep, IsIdentityOnNonIntegralTypes) {
    StaticAssertTypeEq<NormalizeRep<float>, float>();
    StaticAssertTypeEq<NormalizeRep<double>, double>();
    StaticAssertTypeEq<NormalizeRep<long double>, long double>();
    StaticAssertTypeEq<NormalizeRep<std::complex<double>>, std::complex<double>>();
    StaticAssertTypeEq<NormalizeRep<IntWithNoOps>, IntWithNoOps>();
}

// The mechanism that fires on GHS: the size+signedness remap that `NormalizeRep` uses for a
// non-standard integral type (the portable stand-in for the non-portable attributed
// `__packed uint16_t` that motivates the trait).  We exercise `FirstMatchingIntegerOr` directly,
// since there is no portable way to spell an attributed integral type.
TEST(NormalizeRep, RemapPicksStandardIntegerBySizeAndSignedness) {
    StaticAssertTypeEq<FirstMatchingIntegerOr<sizeof(int16_t),
                                              true,
                                              /*Fallback=*/void,
                                              std::int8_t,
                                              std::uint8_t,
                                              std::int16_t,
                                              std::uint16_t,
                                              std::int32_t,
                                              std::uint32_t,
                                              std::int64_t,
                                              std::uint64_t>::type,
                       int16_t>();
    StaticAssertTypeEq<FirstMatchingIntegerOr<sizeof(uint16_t),
                                              false,
                                              /*Fallback=*/void,
                                              std::int8_t,
                                              std::uint8_t,
                                              std::int16_t,
                                              std::uint16_t,
                                              std::int32_t,
                                              std::uint32_t,
                                              std::int64_t,
                                              std::uint64_t>::type,
                       uint16_t>();
}

// When no fixed-width standard integer matches (e.g. a hypothetical oversized integral), the remap
// leaves the type untouched via its fallback, so exotic reps like `__int128` never become a hard
// error.
TEST(NormalizeRep, RemapFallsBackWhenNoStandardIntegerMatches) {
    struct Tag {};  // stand-in for "some type"; only used as the fallback here
    StaticAssertTypeEq<FirstMatchingIntegerOr<sizeof(std::int64_t) * 2,
                                              true,
                                              Tag,
                                              std::int8_t,
                                              std::int16_t,
                                              std::int32_t,
                                              std::int64_t>::type,
                       Tag>();
}

// The chosen normalized type always preserves size and signedness (the correctness contract for the
// remap path, checked across all integer sizes).
TEST(NormalizeRep, PreservesSizeAndSignednessForAllStandardIntegers) {
    auto check = [](auto tag) {
        using T = decltype(tag);
        using N = NormalizeRep<T>;
        EXPECT_THAT(sizeof(N), Eq(sizeof(T)));
        EXPECT_THAT(std::is_signed<N>::value, Eq(std::is_signed<T>::value));
    };
    check(int8_t{});
    check(uint8_t{});
    check(int16_t{});
    check(uint16_t{});
    check(int32_t{});
    check(uint32_t{});
    check(int64_t{});
    check(uint64_t{});
}

}  // namespace detail
}  // namespace au

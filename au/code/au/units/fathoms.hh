// Copyright 2022 Aurora Operations, Inc.
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

#include "au/units/fathoms_fwd.hh"
// Keep corresponding `_fwd.hh` file on top.

#include "au/quantity.hh"
#include "au/unit_symbol.hh"
#include "au/units/feet.hh"

namespace au {

// DO NOT follow this pattern to define your own units.  This is for library-defined units.
// Instead, follow instructions at (https://aurora-opensource.github.io/au/main/howto/new-units/).
template <typename T>
struct FathomsLabel {
    static constexpr const char label[] = "ftm";
};
template <typename T>
constexpr const char FathomsLabel<T>::label[];
struct Fathoms : decltype(Feet{} * mag<6>()), FathomsLabel<void> {
    using FathomsLabel<void>::label;
};
constexpr auto fathom = SingularNameFor<Fathoms>{};
constexpr auto fathoms = QuantityMaker<Fathoms>{};

namespace symbols {
constexpr auto ftm = SymbolFor<Fathoms>{};
}
}  // namespace au

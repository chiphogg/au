// Copyright 2026 Aurora Operations, Inc.
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

// NOTE TO EDITORS: this file is line-aligned with `raw.cc`.  See the note in that file.

#include "au/au.hh"

#include <cstdio>

#include "au/constants/planck_constant.hh"
#include "au/constants/speed_of_light.hh"
#include "au/prefix.hh"
#include "au/units/joules.hh"
#include "au/units/meters.hh"

using namespace au;

// clang-format off
// --8<-- [start:example]
// Energy of a 532 nm (green) photon.



int main() {
    constexpr auto lambda = nano(meters)(532.0);
    constexpr auto energy = PLANCK_CONSTANT * SPEED_OF_LIGHT / lambda;
    std::printf("%.4g J\n", energy.in(joules));
}
// --8<-- [end:example]
// clang-format on

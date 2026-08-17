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

#include <cstdio>

#include "au/prefix.hh"
#include "au/units/joules.hh"
#include "au/units/meters.hh"
#include "au/units/seconds.hh"
#include "examples/atomic_units/atomic_units.hh"

using namespace au;
using namespace atomic_units;

// --8<-- [start:usage]
int main() {
    // Crossing out to SI.  The accuracy here is set by the two measured inputs, and nothing else:
    // Au composes the entire definition chain into one exact rational factor before applying it.
    std::printf("a_0 = %.6g m\n", BOHR_RADIUS.in<double>(meters));
    std::printf("E_h = %.6g J\n", HARTREE.in<double>(joules));
    std::printf("t_a = %.6g s\n", ATOMIC_TIME.in<double>(seconds));

    // Staying inside the system.  This is exactly 1, not 0.9999999997, because the atomic time
    // unit is *defined* as hbar / E_h rather than pasted in as a decimal.
    std::printf("hbar = %.15g E_h t_a\n", hbar.in<double>(hartrees * atomic_time_units));
}
// --8<-- [end:usage]

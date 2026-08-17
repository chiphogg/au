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

// NOTE TO EDITORS: this file is line-aligned with `au.cc`, and the doc-visible region is fenced off
// from clang-format so the alignment survives.  See `examples/angular_velocity/raw.cc` for the
// full explanation.

#include <cstdio>

// clang-format off
// --8<-- [start:example]
// Energy of a 532 nm (green) photon.
const double PLANCK = 6.62607015e-34;  // J s -- and don't mix this up with hbar
const double C = 2.99792458e8;         // m / s

int main() {
    const double lambda_m = 532e-9;
    const double energy_j = PLANCK * C / lambda_m;
    std::printf("%.4g J\n", energy_j);
}
// --8<-- [end:example]
// clang-format on

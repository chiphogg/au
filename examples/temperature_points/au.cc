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

#include "au/units/celsius.hh"
#include "au/units/fahrenheit.hh"

using namespace au;

// clang-format off
// --8<-- [start:example]
// A thermometer reading of 68 F, and a temperature rise of 18 F.  Au gives them
// different types: a reading is a *point*, and a rise is a *quantity*.
constexpr auto reading = fahrenheit_pt(68.0);
constexpr auto rise = fahrenheit_qty(18.0);

int main() {
    const double reading_c = reading.in(celsius_pt);
    const double rise_c = rise.in(celsius_qty);
    std::printf("%.1f C, %.1f C rise\n", reading_c, rise_c);
}
// --8<-- [end:example]
// clang-format on

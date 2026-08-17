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

#include "au/prefix.hh"
#include "au/units/meters.hh"
#include "au/units/minutes.hh"
#include "au/units/radians.hh"
#include "au/units/revolutions.hh"
#include "au/units/seconds.hh"

using namespace au;
using namespace au::symbols;

// clang-format off
// --8<-- [start:example]
using Rpm = UnitQuotient<Revolutions, Minutes>;

// --8<-- [start:headline]
// The types state the units.  Nothing to remember; nothing to convert.
QuantityF<Rpm> wheel_rpm(QuantityF<UnitQuotient<Meters, Seconds>> v, QuantityF<Meters> r) {
    return v * (rad / r);
}
// --8<-- [end:headline]

int main() {

    const auto omega = wheel_rpm((meters / second)(15.0f), milli(meters)(350.0f));
    std::printf("%.1f RPM\n", omega.in(revolutions / minute));
}
// --8<-- [end:example]
// clang-format on

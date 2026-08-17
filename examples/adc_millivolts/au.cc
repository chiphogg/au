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
#include "au/units/volts.hh"

using namespace au;

// clang-format off
// --8<-- [start:example]
// A 12-bit ADC: `counts` out of 4096 spans a 3300 mV reference.
QuantityI32<Milli<Volts>> adc_to_millivolts(int counts) {
    return milli(volts)(3300) * counts / 4096;  // Multiply first, exactly as before.
}

int main() {
    const auto v = adc_to_millivolts(2048);
    // `v.in(volts)` would not compile: Au rejects the truncating conversion.
    std::printf("%d mV\n", v.in(milli(volts)));
}
// --8<-- [end:example]
// clang-format on

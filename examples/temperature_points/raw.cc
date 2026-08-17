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
// A thermometer reading of 68 F, and a temperature rise of 18 F.  Both are
// "degrees Fahrenheit", but they convert to Celsius by different rules.
const double reading_f = 68.0;
const double rise_f = 18.0;

int main() {
    const double reading_c = (reading_f - 32.0) * 5.0 / 9.0;  // subtract the offset
    const double rise_c = rise_f * 5.0 / 9.0;                 // ...but not here!
    std::printf("%.1f C, %.1f C rise\n", reading_c, rise_c);
}
// --8<-- [end:example]
// clang-format on

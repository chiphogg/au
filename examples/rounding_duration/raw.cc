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

#include <cmath>
#include <cstdio>

// clang-format off
// --8<-- [start:example]
// A 100-second task, reported to the nearest whole minute.
int main() {
    const int elapsed_s = 100;
    // Remember the 60, remember to go through double, remember to round.
    const long whole_minutes = std::lround(elapsed_s / 60.0);
    std::printf("%ld min\n", whole_minutes);
}
// --8<-- [end:example]
// clang-format on

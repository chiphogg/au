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

#include "au/math.hh"
#include "au/units/minutes.hh"
#include "au/units/seconds.hh"

using namespace au;

// clang-format off
// --8<-- [start:example]
// A 100-second task, reported to the nearest whole minute.
int main() {
    const auto elapsed = seconds(100);

    const auto whole_minutes = round_as<int>(minutes, elapsed);
    std::printf("%d min\n", whole_minutes.in(minutes));
}
// --8<-- [end:example]
// clang-format on

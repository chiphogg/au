#!/bin/sh
# Copyright 2026 Aurora Operations, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Checks that an example program prints exactly what the docs say it prints.
#
# Usage: check_output.sh BINARY EXPECTED_TXT

set -eu

binary="$1"
expected_txt="$2"
actual_txt="${TEST_TMPDIR:-/tmp}/actual.txt"

if ! "${binary}" > "${actual_txt}"; then
    echo "FAIL: the example exited nonzero." >&2
    exit 1
fi

if ! cmp -s "${actual_txt}" "${expected_txt}"; then
    echo "FAIL: the example printed the wrong output." >&2
    echo "  expected: $(od -c "${expected_txt}" | head -10)" >&2
    echo "  actual:   $(od -c "${actual_txt}" | head -10)" >&2
    exit 1
fi

# Rounding to the nearest minute

A task took 100 seconds.  Report that as a whole number of minutes.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** — convert, then round, and keep the two steps straight yourself.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/rounding_duration/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** — one operation that names the unit you want to round to.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/rounding_duration/au.cc:example"
    ```

Both programs print `2 min`.

## What's happening

Rounding a quantity is a two-part operation that is easy to get subtly wrong, because "round" and
"convert" have to happen in the right order and in a representation that can hold the intermediate.

The raw version spells out all of it: divide by 60, go through `double` so the division doesn't
truncate first, then round. Each step is a place to slip.  Writing `elapsed_s / 60` instead of
`/ 60.0` silently yields 1 rather than 2, and reads almost identically.

Au's [`round_as`](../reference/math.md) makes it a single operation, with the unit as an argument:

- **The rounding unit is explicit.**  `round_as<int>(minutes, elapsed)` says exactly what is being
  rounded, and to what.  There is no separate conversion step to get out of order, and no `60` to
  remember or mistype.

- **The intermediate representation is Au's problem, not yours.**  The input is an integer quantity
  of seconds; `round_as` handles getting to a representation that can round correctly and back
  again.

- **The result is still a quantity.**  `whole_minutes` is a `Quantity<Minutes, int>`, not a bare
  `int` that has to be labeled by its variable name.  Feeding it somewhere that expects seconds
  converts correctly; feeding it somewhere that expects a length does not compile.

- **The whole family is available**, and each member names its unit the same way: `round_in`,
  `floor_as`, `floor_in`, `ceil_as`, `ceil_in`.  The `_as` forms return a quantity; the `_in` forms
  return the raw number.

## Related reading

- [Math: rounding functions](../reference/math.md), including the `floor` and `ceil` variants.
- [ADC counts to millivolts](./adc-millivolts.md), for the conversion Au refuses outright rather
  than rounding.

# Linear speed to RPM

A wheel rolls along the ground at some speed.  How fast is it turning, in revolutions per minute?

<!--
  AUTHORING NOTE.  The two tabs below are a blink comparison: readers flip between them and compare
  corresponding lines in place.  Keep the tab labels byte-for-byte identical to every other example
  page, or Material's linked tabs will stop flipping together.  Keep each banner to one short line,
  or the code will start at a different height in each tab.  The real explanation belongs under
  "What's happening", not in the banner.
-->

=== "⚠️ Before: raw C++"

    ⚠️ **Before** — the conversion factors are magic numbers, and the units live only in comments.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/angular_velocity/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** — the types carry the units, and `rad / r` states the definition directly.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/angular_velocity/au.cc:example"
    ```

Both programs print `409.3 RPM`.  That is not a claim you have to take on faith: they are both
compiled and run in CI, and their output is checked against the same expected text.

## What's happening

The raw version computes $\omega = \frac{v}{2 \pi r} \cdot 60$.  Every piece of that formula is
doing unit bookkeeping, but none of it says so:

- **The `2π` is a unit conversion.**  It is there because one revolution corresponds to a distance
  of $2 \pi r$.  Written as a bare constant, it looks like geometry rather than what it is.

- **The `60` is another unit conversion**, from per-second to per-minute.  Nothing connects it to
  the `RPM` in the function's name.

- **The units live in the parameter names.**  `v_mps` and `r_m` are a convention, and a convention
  is only as good as the caller's memory.  Passing a radius in millimeters compiles cleanly and
  gives a silently wrong answer, so the caller has to convert by hand first.

The Au version replaces all of that with one idea.  A radian is *defined* as the angle whose arc
length equals the radius, so for a wheel of radius `r`, the ratio `rad / r` is exactly the
angle-per-unit-distance the wheel turns through.  Multiplying a speed by that ratio is the whole
conversion:

- **`v * (rad / r)`** is a statement about what a radian *is*, not a formula to be rederived.  The
  `2π` and the `60` never appear in the source, because they are consequences of the unit
  definitions rather than facts about this problem.

- **The return type does the rest.**  `wheel_rpm` is declared to return
  `QuantityF<UnitQuotient<Revolutions, Minutes>>`, and `v * (rad / r)` is an angular velocity in
  some other units entirely.  Au works out the single conversion factor between them at compile
  time, and applies one multiplication at the `return`.

- **The call site converts itself.**  `milli(meters)(350.0f)` is a length, and `wheel_rpm` takes a
  length, so Au converts it — there is nothing for the caller to remember and nothing to get wrong.
  Passing a mass, or a bare `350.0f`, would not compile.

- **Nothing here costs anything at runtime.**  The conversion factor is a compile-time constant, so
  the generated code is the same arithmetic the raw version does by hand.

## Related reading

- [Unit symbols](../reference/unit.md#symbols), such as the `rad` used here.
- [Quantity](../reference/quantity.md), and how conversions are applied.
- [Types for combined units](../reference/unit.md#types-for-combined-units), for expressions like
  `UnitQuotient<Revolutions, Minutes>`.

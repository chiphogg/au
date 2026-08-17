# ADC counts to millivolts

A 12-bit ADC reports a reading of 2048 counts, out of a full scale of 4096, against a 3300 mV
reference.  What voltage is that?  The whole computation has to stay in integer arithmetic.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** — the units live in a variable name, and truncation is silent.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/adc_millivolts/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** — the type carries the unit, and the truncating conversion is refused.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/adc_millivolts/au.cc:example"
    ```

Both programs print `1650 mV`.

## What's happening

This is the case where a units library is often assumed to be a poor fit: integer arithmetic on a
microcontroller, where every operation has to stay exactly as cheap as the hand-written version.
Au is designed for it — Aurora's embedded teams have been first-class customers since the library's
inception — and the two versions here compile to the same arithmetic.

Note first what Au does **not** do for you.  The `counts * 3300 / 4096` ordering matters in both
versions: the per-count resolution is $3300/4096$ mV, which is not an integer, so dividing first
would truncate the scale factor to zero.  Au will not reorder your arithmetic, and it does not
pretend that integer division is exact.  What it changes is everything around that:

- **The return type is a voltage.**  `QuantityI32<Milli<Volts>>` says what the function produces,
  in a way the compiler enforces.  In the raw version, the `int` could be anything, and only the
  function's name suggests otherwise.

- **The dangerous conversion doesn't compile.**  In the raw version, `mv / 1000` is a perfectly
  legal expression that reports 1650 mV as "1 V".  In the Au version, `v.in(volts)` is a compile
  error: the conversion from millivolts to volts on an integer rep would truncate, so Au refuses it
  until you say what you meant.  You can round with [`round_as`](../reference/math.md), or
  explicitly accept the truncation, but you cannot do it by accident.

- **Scaling by a plain number keeps the rep.**  `milli(volts)(3300) * counts` multiplies the
  underlying `int32_t` by an `int`, and stays an integer quantity throughout.  Au does not silently
  promote you to `double` to make a conversion work.

- **The safety surface adapts to the type.**  The same conversion Au refuses here is perfectly
  legal on a floating point rep, where it doesn't lose anything.  The rules follow from what the
  representation can actually hold, rather than from a blanket policy.

## Related reading

- [Quantity: `.in()` and `.as()`](../reference/quantity.md), and which conversions are allowed.
- [Prefixes](../reference/prefix.md), for `milli(volts)`.
- [Rounding](../reference/math.md), when you *do* want the truncating conversion.

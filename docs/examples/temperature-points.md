# Temperature readings vs. temperature changes

A thermometer reads 68 °F.  Over the next hour, the temperature rises by 18 °F.  Convert both to
Celsius.

Both numbers are "degrees Fahrenheit", but they convert by different rules — and getting them
mixed up is one of the most common unit bugs there is.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** — two different rules, and nothing but care to keep them apart.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/temperature_points/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** — different types, so the right rule is the only one available.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/temperature_points/au.cc:example"
    ```

Both programs print `20.0 C, 10.0 C rise`.

## What's happening

A thermometer reading and a temperature change are not the same kind of thing, even though both are
measured in degrees Fahrenheit:

- A **reading** is a *point* on a scale.  Converting it to Celsius means accounting for the fact
  that the two scales have different zeros: $(68 - 32) \cdot \frac{5}{9} = 20$.

- A **change** is a *displacement*.  There is no zero to account for, so only the scale factor
  applies: $18 \cdot \frac{5}{9} = 10$.

Apply the wrong rule and you get $(18 - 32) \cdot \frac{5}{9} = -7.8$, which is not a small error —
it isn't even the right sign.  The raw version has both rules written out, sitting next to each
other, distinguished only by a comment and the discipline of whoever calls them.

Au gives the two things different types, so the distinction stops being a matter of discipline:

- **`fahrenheit_pt(68.0)` is a `QuantityPoint`**, and `fahrenheit_qty(18.0)` is a `Quantity`.  The
  makers have different names on purpose.  Au deliberately provides no plain `fahrenheit()` maker —
  the name would be ambiguous, so trying to use it produces an error telling you to pick one.

- **Each type converts by its own rule**, and there is no way to ask for the other one.
  `reading.in(celsius_pt)` applies the offset; `rise.in(celsius_qty)` does not.

- **The type system enforces what the arithmetic means.**  Subtracting two `QuantityPoint`s gives a
  `Quantity` — the difference between two readings is a change, which is exactly right.  Adding two
  `QuantityPoint`s is not defined at all, because the sum of two temperatures on an offset scale is
  meaningless.

- **This generalizes past temperature.**  Timestamps versus durations, positions versus
  displacements, and pressures relative to vacuum versus ambient are all the same distinction.

## Related reading

- [`QuantityPoint`](../reference/quantity_point.md), and how it differs from `Quantity`.
- [Unit origins](../reference/unit.md#origins), which is what makes an offset scale work.

# Photon energy from wavelength

What is the energy of a 532 nm photon — the green of a laser pointer?  The formula is
$E = hc/\lambda$.

=== "⚠️ Before: raw C++"

    ⚠️ **Before** — the physical constants arrive as hand-copied decimal literals.
    { .ab-banner .ab-before }

    ```cpp
    --8<-- "examples/photon_energy/raw.cc:example"
    ```

=== "✅ After: with Au"

    ✅ **After** — the constants come from the library, and the wavelength stays in nm.
    { .ab-banner .ab-after }

    ```cpp
    --8<-- "examples/photon_energy/au.cc:example"
    ```

Both programs print `3.734e-19 J`.

## What's happening

The raw version's problems are all in the two lines that vanish in the Au version:

- **The constants have to be typed in correctly**, and the values are long enough that a
  transposed digit is easy to make and hard to notice.  A wrong digit in the eleventh place
  produces an answer that looks entirely plausible.

- **The units of each constant live in a trailing comment.**  `PLANCK` is in J·s here; if
  a future edit swaps it for the reduced Planck constant $\hbar$, nothing complains, and the answer
  is quietly off by $2\pi$.

- **The caller has to pre-convert.**  `532e-9` is a wavelength in nanometers written in meters,
  because the formula demands SI base units.  That conversion is invisible and unchecked.

The Au version states the physics and lets the library handle the rest:

- **`PLANCK_CONSTANT` and `SPEED_OF_LIGHT` are library constants**, carrying their units with them.
  Using $\hbar$ instead is a different name, `REDUCED_PLANCK_CONSTANT`, so the substitution that
  silently costs a factor of $2\pi$ above is not available by accident.

- **The wavelength stays in the units you have.**  `nano(meters)(532.0)` says 532 nm, and Au works
  out the rest.

- **Constants are not just quantities with a value.**  Au's [`Constant`](../reference/constant.md)
  type holds its magnitude symbolically, as an exact rational.  Multiplying two constants together
  composes those magnitudes exactly, with no intermediate rounding, and the conversion to joules is
  applied once at the end.

- **This also sidesteps overflow.**  Computing $hc$ as a floating point intermediate is fine for
  `double`, but on a narrower type the product of a very small and a very large number can
  underflow or overflow. Because Au never materializes the intermediate — it composes the
  conversion factor symbolically and applies it once — that class of problem does not arise.

- **The whole computation is `constexpr`.**  Nothing here happens at runtime.

## Related reading

- [Constants](../reference/constant.md), and why they are not simply `Quantity` values.
- [Magnitude](../reference/magnitude.md), the exact compile-time arithmetic underneath.
- [How to define new constants](../howto/new-constants.md).

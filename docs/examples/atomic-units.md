# Atomic units

This example is different from the others.  Rather than a snippet you would drop into a function,
it is a small **system of units** built on top of Au — and it is meant to be copied into your
project and used.

Hartree atomic units are the working units of quantum chemistry.  They are convenient precisely
because they are defined so that the quantities in the Schrödinger equation come out around 1.

## Why Au doesn't ship these

Every unit Au provides has an *exact* definition in terms of other units.  A foot is exactly 0.3048
meters, by definition, and that will not change.

Atomic units are not like that.  The electron mass and the fine structure constant are **measured**,
and the best known values move as experiments improve.  A units library that hard-coded them would
be baking one particular CODATA release into every program that used it, and there would be no good
answer for a user who needed a different one.

So Au takes the other route: make it cheap for you to define them in your own project, where you
control which values you're using.  That's what this page walks through.  It is a complete,
compiled, tested drop-in — [`examples/atomic_units/`](
https://github.com/aurora-opensource/au/tree/main/examples/atomic_units) in the repository.

## The definitions

```cpp
--8<-- "examples/atomic_units/atomic_units.hh:definitions"
```

In C++14, the unit labels need one out-of-line definition each, in a `.cc` file:

```cpp
--8<-- "examples/atomic_units/atomic_units.cc:labels"
```

## Using them

```cpp
--8<-- "examples/atomic_units/main.cc:usage"
```

This prints:

```
a_0 = 5.29177e-11 m
E_h = 4.35974e-18 J
t_a = 2.41888e-17 s
hbar = 1 E_h t_a
```

## What's happening

The structure above is doing the real work, and it is worth being explicit about why it's shaped
this way.

**Everything derives from exactly five inputs.**  Three of them — the speed of light, the reduced
Planck constant, and the elementary charge — are SI-*defining* constants.  They have no
uncertainty, because the SI defines the kilogram, meter and second in terms of them rather than the
other way round.  The other two — the electron mass and the fine structure constant — are measured.
Separating them this way means that updating to a newer CODATA release is a two-line change, and
you can see at a glance exactly which numbers in your program are experimental inputs.

**Each unit is defined by its formula, not by a decimal value.**  The Hartree is written as
`m_e * squared(c) * squared(alpha)`, not as `4.3597447e-18` joules.  That is the difference that
makes the last line of the output read `hbar = 1 E_h t_a` rather than `0.999999999...`.  The atomic
time unit is *defined* as $\hbar/E_h$, so within the system that relationship is exact — Au carries
it as an exact rational [magnitude](../reference/magnitude.md), never as a rounded double.

Two properties follow, and together they are the point of the whole exercise:

- **Inside the system, relationships are exact.**  No accumulated error from round-tripping through
  SI, no drift, no epsilon comparisons.

- **Crossing out to SI is as accurate as physics currently allows.**  Au composes the entire
  definition chain — through the Hartree, through $\alpha^2$, through the electron mass — into a
  single exact rational factor, and applies it once. The only error is the uncertainty in the two
  measured inputs.

**The labels make output readable.**  Because each unit declares a `label`, printing a quantity
gives `a_0` or `E_h` rather than an unwieldy composite of SI base units.

**Nothing here is special-cased.**  These are ordinary Au units, using the same mechanism as
[any other custom unit](../howto/new-units.md).  They compose with the built-in units, participate
in the same conversion checks, and cost nothing at runtime.

!!! note
    The subtleties of *which* set of atomic units to use — Hartree versus Rydberg, and how
    different conventions handle the electromagnetic units — deserve more room than this page.
    See [issue #660](https://github.com/aurora-opensource/au/issues/660) for that discussion.

## Related reading

- [How to define new units](../howto/new-units.md), including the C++14 versus C++17 label forms.
- [How to define new constants](../howto/new-constants.md).
- [Magnitude](../reference/magnitude.md), the exact compile-time arithmetic that makes the
  in-system relationships exact.

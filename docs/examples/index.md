# Code examples

Short, complete programs that show what using Au actually looks like.

Most of these come in two versions: the way the problem is usually solved in plain C++, and the way
it looks with Au.  They are presented as tabs so you can flip between them and compare
corresponding lines in place — the two versions are kept line-aligned on purpose.  Flipping any tab
flips every other tab on the page along with it.

## The examples

- **[Linear speed to RPM](./angular-velocity.md).**  Converting a wheel's road speed into
  revolutions per minute, without ever writing `2π` or `60`.

- **[ADC counts to millivolts](./adc-millivolts.md).**  Integer arithmetic on an embedded target,
  and the truncating conversion Au refuses to compile.

- **[Temperature readings vs. changes](./temperature-points.md).**  Why 68 °F and "a rise of 18 °F"
  convert to Celsius by different rules, and how types keep them apart.

- **[Photon energy from wavelength](./photon-energy.md).**  Physical constants that carry their own
  units, and compose exactly.

- **[Rounding to the nearest minute](./rounding-duration.md).**  Unit-aware rounding in a single
  operation.

- **[Atomic units](./atomic-units.md).**  Building an entire system of units on top of Au — exact
  within itself, and as accurate as physics allows at the boundary.

## About these examples

Every example here is real source code from the
[`examples/`](https://github.com/aurora-opensource/au/tree/main/examples) directory of the
repository, inlined into these pages directly.  CI compiles and runs both versions of each one, and
checks that they produce identical output — so the "before" and "after" really do solve the same
problem, and neither can drift out of date with the library.

<!--
  ADDING AN EXAMPLE: see `examples/README.md` in the repository.  It covers the invariants these
  pages depend on (line-aligned regions, byte-identical tab labels, one-line banners), the design
  decisions behind them, and what is still to do.  Deliberately kept in one place so the two copies
  cannot drift.

  Quality over quantity.  A good example here is a real problem someone actually had, small enough
  to read in one screenful, where the Au version is better in a way the reader can see rather than
  one we have to assert.
-->

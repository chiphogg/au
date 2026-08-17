![Au library logo](docs/assets/au-logo-color.png)

[![clang17-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/clang17-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/clang17-ubuntu.yml)
[![clang14-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/clang14-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/clang14-ubuntu.yml)
[![clang11-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/clang11-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/clang11-ubuntu.yml)
<br>
[![gcc15-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/gcc15-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/gcc15-ubuntu.yml)
[![gcc12-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/gcc12-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/gcc12-ubuntu.yml)
<br>
[![MSVC 2022 x64](
https://github.com/aurora-opensource/au/actions/workflows/msvc-2022-x64.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/msvc-2022-x64.yml)
[![MSVC 2025 x64](
https://github.com/aurora-opensource/au/actions/workflows/msvc-2025-x64.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/msvc-2025-x64.yml)
<br>
[![cuda-ubuntu](
https://github.com/aurora-opensource/au/actions/workflows/cuda-ubuntu.yml/badge.svg?branch=main&event=push)](
https://github.com/aurora-opensource/au/actions/workflows/cuda-ubuntu.yml)

# Au: A C++14-compatible units library, by Aurora

Au (pronounced "ay yoo") is a C++ units library, by [Aurora](https://aurora.tech/).  What the
`<chrono>` library did for time variables, _Au_ does for _all physical quantities_ (lengths, speeds,
voltages, and so on). Namely:

- Catch unit errors at compile time, with **no runtime penalty**.
- Make unit conversions effortless to get right.
- _Accelerate and improve your general developer experience._

In short: if your C++ programs handle physical quantities, Au will make you faster and more
effective at your job.  You'll find everything you need in our [full documentation
website](https://aurora-opensource.github.io/au).

> _Try it out on [Compiler Explorer ("godbolt")](https://godbolt.org/z/o91nfh5Wc)!_

## What it looks like

Converting a wheel's road speed to RPM.  First, the way it usually gets written:

<!-- BEGIN EXAMPLE: examples/angular_velocity/raw.cc:headline -->
```cpp
// Speed must be m/s.  Radius must be meters.  Returns RPM.
float wheel_rpm(float v_mps, float r_m) {
    return v_mps / (2.0f * PI * r_m) * 60.0f;
}
```
<!-- END EXAMPLE -->

The units live in the parameter names, where nothing checks them, and the `2π` and `60` are unit
conversions disguised as magic numbers.  With Au:

<!-- BEGIN EXAMPLE: examples/angular_velocity/au.cc:headline -->
```cpp
// The types state the units.  Nothing to remember; nothing to convert.
QuantityF<Rpm> wheel_rpm(QuantityF<UnitQuotient<Meters, Seconds>> v, QuantityF<Meters> r) {
    return v * (rad / r);
}
```
<!-- END EXAMPLE -->

The `2π` and the `60` are gone — not hidden, but genuinely unnecessary.  A radian is _defined_ as
the angle whose arc length equals the radius, so `rad / r` is exactly the angle a wheel turns per
unit distance travelled.  Au derives the rest from the return type, at compile time, at no runtime
cost.  Callers pass whatever units they have; a radius in millimeters converts itself, and a mass
doesn't compile.

See our [**code examples**](https://aurora-opensource.github.io/au/main/examples/) for this one in
full, and several more.

## Why Au?

There are many other C++ units libraries, several quite well established.  Each of them offers
_some_ of the following properties, but **only Au** offers _all_ of them:

- Wide compatibility with C++ versions (anything C++14 or newer).
- Easy installation in any project (including a customizable single-header option).
- Small compile time burden.
- Concise, readable typenames in compiler errors.

We also provide several totally new features, including fully unit-safe APIs, an adaptive "safety
surface" that protects conversions against overflow, unit-aware rounding and inverse functions, and
many more.

Forged in the crucible of Aurora's diverse, demanding use cases, Au has a proven track record of
usability and reliability. This **includes embedded and GPU support**: Aurora's embedded teams have
been first class customers since the library's inception, and Au now also works out of the box with
CUDA.

To learn more about our place in the C++ units library ecosystem, see our [detailed library
comparison](https://aurora-opensource.github.io/au/main/alternatives/).

## Getting started

Our [installation instructions](https://aurora-opensource.github.io/au/main/install/) can have you
up and running in minutes, in any project that supports C++14 or newer.

To use the library effectively, we recommend working through the
[tutorials](https://aurora-opensource.github.io/au/main/tutorial/), starting with [Au 101: Quantity
Makers](https://aurora-opensource.github.io/au/main/tutorial/101-quantity-makers/).  To
get set up with the tutorials — or, to contribute to the library — check out our [development
guide](https://aurora-opensource.github.io/au/main/develop/).

## As seen at CppCon 2021

At CppCon 2021, we
[presented](https://cppcon2021.sched.com/event/nvCp/units-libraries-and-autonomous-vehicles-lessons-from-the-trenches)
the properties we found to be most important in units libraries, and advice on using them
effectively.  Because Au was designed from the ground up with these best practices in mind, it
thoroughly exemplifies them.  Check out the video below, and follow along with [the slide
deck](https://chogg.name/cppcon-2021-units/) if you like.

[![Chip Hogg's CppCon 2021 Aurora units talk](https://github.com/user-attachments/assets/ca87d98d-4256-4d10-a112-723d1ee56275)](https://www.youtube.com/watch?v=5dhFtSu3wCo)

> NOTE: This open-source version has been significantly improved from what was presented in the
talk: both in its user interfaces, _and_ under the hood!  The one downside is that matrix and vector
support hasn't yet been implemented.  See [#70](https://github.com/aurora-opensource/au/issues/70)
for more details, and subscribe to that issue to watch for progress.

## ...and, at CppCon 2023

After CppCon 2021, we found that telling people what to look for in a units library wasn't good
enough, if they couldn't find one that met those criteria.  We saw so many people struggling with
problems that we had already solved robustly!  So, we set about sharing our work.  With a clean
slate, we made a new library that was a drop-in replacement for Aurora's internal library, but with
zero Aurora-internal dependencies, so that we could easily open source it.  The result was Au, and
we shared it at CppCon 2023.  Check out the video below, and follow along with [the slide
deck](https://chogg.name/cppcon-2023-au-units) if you like.

[![Chip Hogg's CppCon 2023 Aurora units talk](https://github.com/user-attachments/assets/5f359644-d36c-43f9-8d6f-b49bb1d3cb2e)](https://www.youtube.com/watch?v=o0ck5eqpOLc)

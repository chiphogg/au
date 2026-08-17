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

#pragma once

#include "au/au.hh"
#include "au/constants/elementary_charge.hh"
#include "au/constants/reduced_planck_constant.hh"
#include "au/constants/speed_of_light.hh"
#include "au/units/grams.hh"

// A drop-in set of Hartree atomic units, built on top of Au.
//
// Au cannot ship these units itself, because they have no exact definition in terms of SI units:
// two of the numbers below are *measured*, and the best known values change as experiments improve.
// A units library that hard-coded them would be baking a particular CODATA release into everyone's
// program.  Defining them in your own project is a better fit, and Au makes it cheap.
//
// The structure below is what makes this work well.  Everything is derived from exactly five
// inputs: three SI-defining constants (exact by definition) and two measured constants.  Every
// atomic unit is then defined *symbolically* in terms of those five, rather than by pasting in a
// decimal value.  Two things follow:
//
//   - Relationships *within* the system are exact.  `hbar` really is exactly one Hartree times one
//     atomic time unit, because that is how the atomic time unit is defined here -- not a value
//     that rounds to 1.0.
//
//   - Conversions *out* to SI are as accurate as the measured inputs allow, and no worse.  Au
//     composes the whole chain as a single exact rational magnitude and applies it once.
//
// To update to a newer CODATA release, change the two measured values and nothing else.

// --8<-- [start:definitions]
namespace atomic_units {

//
// Exact inputs: SI-defining constants.  These have no uncertainty; the SI *defines* them.
//
constexpr auto c = au::SPEED_OF_LIGHT;
constexpr auto hbar = au::REDUCED_PLANCK_CONSTANT;
constexpr auto e = au::ELEMENTARY_CHARGE;

//
// Measured inputs.  These are the only two values here that a future experiment can revise.
//

// Fine structure constant.  CODATA 2018: 7.297 352 5693(11) e-3
constexpr auto alpha = au::mag<72973525693>() * au::pow<-13>(au::mag<10>());

// Electron mass.  CODATA 2018: 9.109 383 7015(28) e-31 kg
struct ElectronMassUnit
    : decltype(au::Kilo<au::Grams>{} * au::mag<91093837015>() * au::pow<-41>(au::mag<10>())) {
    static constexpr const char label[] = "m_e";
};
constexpr auto ELECTRON_MASS = au::make_constant(ElectronMassUnit{});
constexpr auto m_e = ELECTRON_MASS;

//
// Derived atomic units.  Each is defined by its physical formula, not by a decimal value, so the
// relationships among them are exact.
//

// Energy: the Hartree, E_h = m_e c^2 alpha^2.
struct Hartrees : decltype(au::associated_unit(m_e * au::squared(c) * au::squared(alpha))) {
    static constexpr const char label[] = "E_h";
};
constexpr auto HARTREE = au::make_constant(Hartrees{});
constexpr auto hartree = au::SingularNameFor<Hartrees>{};
constexpr auto hartrees = au::QuantityMaker<Hartrees>{};

// Length: the Bohr radius, a_0 = hbar / (m_e c alpha).
struct BohrRadii : decltype(au::associated_unit(hbar / (m_e * c * alpha))) {
    static constexpr const char label[] = "a_0";
};
constexpr auto BOHR_RADIUS = au::make_constant(BohrRadii{});
constexpr auto bohr_radius = au::SingularNameFor<BohrRadii>{};
constexpr auto bohr_radii = au::QuantityMaker<BohrRadii>{};

// Time: t_a = hbar / E_h.
struct AtomicTimeUnits : decltype(au::associated_unit(hbar / hartrees)) {
    static constexpr const char label[] = "t_a";
};
constexpr auto ATOMIC_TIME = au::make_constant(AtomicTimeUnits{});
constexpr auto atomic_time_unit = au::SingularNameFor<AtomicTimeUnits>{};
constexpr auto atomic_time_units = au::QuantityMaker<AtomicTimeUnits>{};

}  // namespace atomic_units
// --8<-- [end:definitions]

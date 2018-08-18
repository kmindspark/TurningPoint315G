/**
 * @author Mikhail Semenov
 * @author Benjamin Jurke
 * @author Ryan Benasutti, WPI
 *
 * This code is a modified version of Benjamin Jurke's work in 2015. You can read his blog post
 * here:
 * https://benjaminjurke.com/content/articles/2015/compile-time-numerical-unit-dimension-checking/
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _OKAPI_QANGULARSPEED_HPP_
#define _OKAPI_QANGULARSPEED_HPP_

#include "okapi/api/units/QAngle.hpp"
#include "okapi/api/units/QTime.hpp"
#include "okapi/api/units/RQuantity.hpp"

QUANTITY_TYPE(0, 0, -1, 1, QAngularSpeed)

constexpr QAngularSpeed radps = radian / second;
constexpr QAngularSpeed rpm = (360 * degree) / minute;

constexpr QAngularSpeed operator"" _rpm(long double x) {
  return x * rpm;
}
constexpr QAngularSpeed operator"" _rpm(unsigned long long int x) {
  return static_cast<double>(x) * rpm;
}

#endif

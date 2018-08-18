/**
 * @author Ryan Benasutti, WPI
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _OKAPI_ABSTRACTTIMER_HPP_
#define _OKAPI_ABSTRACTTIMER_HPP_

#include "okapi/api/units/QFrequency.hpp"
#include "okapi/api/units/QTime.hpp"

namespace okapi {
class AbstractTimer {
  public:
  virtual ~AbstractTimer();

  /**
   * Returns the current time in units of QTime.
   *
   * @return the current time
   */
  virtual QTime millis() const = 0;

  /**
   * Returns the time passed in ms since the previous call of this function.
   *
   * @return The time passed in ms since the previous call of this function
   */
  virtual QTime getDt() = 0;

  /**
   * Returns the time the timer was first constructed.
   *
   * @return The time the timer was first constructed
   */
  virtual QTime getStartingTime() const = 0;

  /**
   * Returns the time since the timer was first constructed.
   *
   * @return The time since the timer was first constructed
   */
  virtual QTime getDtFromStart() const = 0;

  /**
   * Place a time marker. Placing another marker will overwrite the previous one.
   */
  virtual void placeMark() = 0;

  /**
   * Place a hard time marker. Placing another hard marker will not overwrite the previous one;
   * instead, call clearHardMark() and then place another.
   */
  virtual void placeHardMark() = 0;

  /**
   * Clears the hard marker.
   *
   * @return The old hard marker
   */
  virtual QTime clearHardMark() = 0;

  /**
   * Returns the time since the time marker.
   *
   * @return The time since the time marker
   */
  virtual QTime getDtFromMark() const = 0;

  /**
   * Returns the time since the hard time marker.
   *
   * @return The time since the hard time marker
   */
  virtual QTime getDtFromHardMark() const = 0;

  /**
   * Returns true when the input time period has passed, then resets. Meant to be used in loops
   * to run an action every time period without blocking.
   *
   * @param time time period
   * @return true when the input time period has passed, false after reading true until the
   *   period has passed again
   */
  virtual bool repeat(QTime time) = 0;

  /**
   * Returns true when the input time period has passed, then resets. Meant to be used in loops
   * to run an action every time period without blocking.
   *
   * @param frequency the repeat frequency
   * @return true when the input time period has passed, false after reading true until the
   *   period has passed again
   */
  virtual bool repeat(QFrequency frequency) = 0;
};
} // namespace okapi

#endif

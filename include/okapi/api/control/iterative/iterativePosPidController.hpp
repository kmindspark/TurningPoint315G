/**
 * Based on the Arduino PID controller: https://github.com/br3ttb/Arduino-PID-Library
 *
 * @author Ryan Benasutti, WPI
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _OKAPI_ITERATIVEPOSPIDCONTROLLER_HPP_
#define _OKAPI_ITERATIVEPOSPIDCONTROLLER_HPP_

#include "okapi/api/control/iterative/iterativePositionController.hpp"
#include "okapi/api/control/util/settledUtil.hpp"
#include "okapi/api/util/timeUtil.hpp"
#include <memory>

namespace okapi {
class IterativePosPIDControllerArgs : public IterativePositionControllerArgs {
  public:
  IterativePosPIDControllerArgs(double ikP, double ikI, double ikD, double ikBias = 0);

  const double kP, kI, kD, kBias;
};

class IterativePosPIDController : public IterativePositionController {
  public:
  /**
   * Position PID controller.
   */
  IterativePosPIDController(double ikP, double ikI, double ikD, double ikBias,
                            const TimeUtil &itimeUtil);

  /**
   * Position PID controller.
   *
   * @param params PosPIDControllerArgs
   */
  IterativePosPIDController(const IterativePosPIDControllerArgs &params, const TimeUtil &itimeUtil);

  /**
   * Do one iteration of the controller. Returns the reading in the range [-127, 127] unless the
   * bounds have been changed with setOutputLimits().
   *
   * @param inewReading new measurement
   * @return controller output
   */
  double step(double inewReading) override;

  /**
   * Sets the target for the controller.
   *
   * @param itarget new target position
   */
  void setTarget(double itarget) override;

  /**
   * Returns the last calculated output of the controller. Output is in the range [-1, 1]
   * unless the bounds have been changed with setOutputLimits().
   */
  double getOutput() const override;

  /**
   * Returns the last error of the controller.
   */
  double getError() const override;

  /**
   * Returns the last derivative (change in error) of the controller.
   */
  double getDerivative() const override;

  /**
   * Returns whether the controller has settled at the target. Determining what settling means is
   * implementation-dependent.
   *
   * @return whether the controller is settled
   */
  bool isSettled() override;

  /**
   * Set controller gains.
   *
   * @param ikP proportional gain
   * @param ikI integral gain
   * @param ikD derivative gain
   * @param ikBias bias (constant offset added to the output)
   */
  virtual void setGains(double ikP, double ikI, double ikD, double ikBias = 0);

  /**
   * Set time between loops in ms.
   *
   * @param isampleTime time between loops
   */
  void setSampleTime(QTime isampleTime) override;

  /**
   * Set controller output bounds. Default bounds are [-1, 1].
   *
   * @param imax max output
   * @param imin min output
   */
  void setOutputLimits(double imax, double imin) override;

  /**
   * Set integrator bounds. Default bounds are [-1, 1].
   *
   * @param imax max integrator value
   * @param imin min integrator value
   */
  virtual void setIntegralLimits(double imax, double imin);

  /**
   * Set the error sum bounds. Default bounds are [500, 1250]. Error will only be added to the
   * integral term when its absolute value between these bounds of either side of the target.
   *
   * @param imax max error value that will be summed
   * @param imin min error value that will be summed
   */
  virtual void setErrorSumLimits(double imax, double imin);

  /**
   * Resets the controller so it can start from 0 again properly. Keeps gains and limits from
   * before.
   */
  void reset() override;

  /**
   * Set whether the integrator should be reset when error is 0 or changes sign.
   *
   * @param iresetOnZero true to reset
   */
  virtual void setIntegratorReset(bool iresetOnZero);

  /**
   * Changes whether the controller is off or on. Turning the controller on after it was off will
   * cause the controller to move to its last set target, unless it was reset in that time.
   */
  void flipDisable() override;

  /**
   * Sets whether the controller is off or on. Turning the controller on after it was off will
   * cause the controller to move to its last set target, unless it was reset in that time.
   *
   * @param iisDisabled whether the controller is disabled
   */
  void flipDisable(bool iisDisabled) override;

  /**
   * Returns whether the controller is currently disabled.
   *
   * @return whether the controller is currently disabled
   */
  bool isDisabled() const override;

  /**
   * Get the last set sample time.
   *
   * @return sample time
   */
  QTime getSampleTime() const override;

  protected:
  double kP, kI, kD, kBias;
  QTime sampleTime = 10_ms;
  double target = 0;
  double lastReading = 0;
  double error = 0;
  double lastError = 0;

  // Integral bounds
  double integral = 0;
  double integralMax = 1;
  double integralMin = -1;

  // Error will only be added to the integral term within these bounds on either side of the target
  double errorSumMin = 500;
  double errorSumMax = 1250;

  double derivative = 0;

  // Output bounds
  double output = 0;
  double outputMax = 1;
  double outputMin = -1;

  // Reset the integrated when the controller crosses 0 or not
  bool shouldResetOnCross = true;

  bool isOn = true;

  std::unique_ptr<AbstractTimer> loopDtTimer;
  std::unique_ptr<SettledUtil> settledUtil;
};
} // namespace okapi

#endif

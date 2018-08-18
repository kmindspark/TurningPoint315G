/**
 * @author Ryan Benasutti, WPI
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _OKAPI_ASYNCWRAPPER_HPP_
#define _OKAPI_ASYNCWRAPPER_HPP_

#include "okapi/api/control/async/asyncController.hpp"
#include "okapi/api/control/controllerInput.hpp"
#include "okapi/api/control/controllerOutput.hpp"
#include "okapi/api/control/iterative/iterativeController.hpp"
#include "okapi/api/control/util/settledUtil.hpp"
#include "okapi/api/coreProsAPI.hpp"
#include "okapi/api/util/abstractRate.hpp"
#include "okapi/api/util/supplier.hpp"
#include <memory>

namespace okapi {
class AsyncWrapper : virtual public AsyncController {
  public:
  /**
   * A wrapper class that transforms an IterativeController into an AsyncController by running it in
   * another task. The input controller will act like an AsyncController. The output of the
   * IterativeController will be scaled by the given scale (127 by default).
   *
   * @param iinput controller input, passed to the IterativeController
   * @param ioutput controller output, written to from the IterativeController
   * @param icontroller the controller to use
   * @param irateSupplier used for rates used in the main loop and in waitUntilSettled
   * @param isettledUtil used in waitUntilSettled
   * @param iscale the scale applied to the controller output
   */
  AsyncWrapper(std::shared_ptr<ControllerInput> iinput, std::shared_ptr<ControllerOutput> ioutput,
               std::unique_ptr<IterativeController> icontroller,
               const Supplier<std::unique_ptr<AbstractRate>> &irateSupplier,
               std::unique_ptr<SettledUtil> isettledUtil);

  /**
   * Sets the target for the controller.
   */
  void setTarget(double itarget) override;

  /**
   * Returns the last calculated output of the controller. Default is 0.
   */
  double getOutput() const override;

  /**
   * Returns the last error of the controller.
   */
  double getError() const override;

  /**
   * Returns whether the controller has settled at the target. Determining what settling means is
   * implementation-dependent.
   *
   * @return whether the controller is settled
   */
  bool isSettled() override;

  /**
   * Set time between loops. Default does nothing.
   *
   * @param isampleTime time between loops
   */
  void setSampleTime(QTime isampleTime) override;

  /**
   * Set controller output bounds. Default does nothing.
   *
   * @param imax max output
   * @param imin min output
   */
  void setOutputLimits(double imax, double imin) override;

  /**
   * Resets the controller so it can start from 0 again properly. Keeps configuration from
   * before.
   */
  void reset() override;

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
   * Blocks the current task until the controller has settled. Determining what settling means is
   * implementation-dependent.
   */
  void waitUntilSettled() override;

  protected:
  std::shared_ptr<ControllerInput> input;
  std::shared_ptr<ControllerOutput> output;
  std::unique_ptr<IterativeController> controller;
  std::unique_ptr<AbstractRate> loopRate;
  std::unique_ptr<AbstractRate> settledRate;
  std::unique_ptr<SettledUtil> settledUtil;
  CROSSPLATFORM_THREAD task;

  static void trampoline(void *context);
  void loop();
};
} // namespace okapi

#endif

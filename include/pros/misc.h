/**
 * \file pros/misc.h
 *
 * \brief Contains miscellaneous declarations such as controller and competition
 * functions
 *
 * This file should not be modified by users, since it gets replaced whenever
 * a kernel upgrade occurs.
 *
 * Visit https://pros.cs.purdue.edu/v5/tutorials/topcial/controller to learn more.
 *
 * Copyright (c) 2017-2018, Purdue University ACM SIGBots.
 * All rights reservered.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _PROS_MISC_H_
#define _PROS_MISC_H_

#define NUM_V5_PORTS (22)

/******************************************************************************/
/**                             V5 Competition                               **/
/******************************************************************************/
#define COMPETITION_AUTONOMOUS (1 << 0)
#define COMPETITION_DISABLED (1 << 1)
#define COMPETITION_CONNECTED (1 << 2)

/**
 * Get the current status of the competition control.
 *
 * \return The competition control status as a mask of bits with
 * 			   COMPETITION_{ENABLED,AUTONOMOUS,CONNECTED}.
 */
uint8_t competition_get_status(void);
#define competition_is_disabled() ((competition_get_status() & COMPETITION_DISABLED) != 0)
#define competition_is_connected() ((competition_get_status() & COMPETITION_CONNECTED) != 0)
#define competition_is_autonomous() ((competition_get_status() & COMPETITION_AUTONOMOUS) != 0)

/******************************************************************************/
/**                              V5 Controller                               **/
/******************************************************************************/
typedef enum { E_CONTROLLER_MASTER = 0, E_CONTROLLER_PARTNER } controller_id_e_t;

typedef enum {
	E_CONTROLLER_ANALOG_LEFT_X = 0,
	E_CONTROLLER_ANALOG_LEFT_Y,
	E_CONTROLLER_ANALOG_RIGHT_X,
	E_CONTROLLER_ANALOG_RIGHT_Y
} controller_analog_e_t;

typedef enum {
	E_CONTROLLER_DIGITAL_L1 = 6,
	E_CONTROLLER_DIGITAL_L2,
	E_CONTROLLER_DIGITAL_R1,
	E_CONTROLLER_DIGITAL_R2,
	E_CONTROLLER_DIGITAL_UP,
	E_CONTROLLER_DIGITAL_DOWN,
	E_CONTROLLER_DIGITAL_LEFT,
	E_CONTROLLER_DIGITAL_RIGHT,
	E_CONTROLLER_DIGITAL_X,
	E_CONTROLLER_DIGITAL_B,
	E_CONTROLLER_DIGITAL_Y,
	E_CONTROLLER_DIGITAL_A
} controller_digital_e_t;

#ifdef PROS_USE_SIMPLE_NAMES
#define CONTROLLER_MASTER E_CONTROLLER_MASTER
#define CONTROLLER_PARTNER E_CONTROLLER_PARTNER
#define ANALOG_LEFT_X E_CONTROLLER_ANALOG_LEFT_X
#define ANALOG_LEFT_Y E_CONTROLLER_ANALOG_LEFT_Y
#define ANALOG_RIGHT_X E_CONTROLLER_ANALOG_RIGHT_X
#define ANALOG_RIGHT_Y E_CONTROLLER_ANALOG_RIGHT_Y
#define DIGITAL_L1 E_CONTROLLER_DIGITAL_L1
#define DIGITAL_L2 E_CONTROLLER_DIGITAL_L2
#define DIGITAL_R1 E_CONTROLLER_DIGITAL_R1
#define DIGITAL_R2 E_CONTROLLER_DIGITAL_R2
#define DIGITAL_UP E_CONTROLLER_DIGITAL_UP
#define DIGITAL_DOWN E_CONTROLLER_DIGITAL_DOWN
#define DIGITAL_LEFT E_CONTROLLER_DIGITAL_LEFT
#define DIGITAL_RIGHT E_CONTROLLER_DIGITAL_RIGHT
#define DIGITAL_X E_CONTROLLER_DIGITAL_X
#define DIGITAL_B E_CONTROLLER_DIGITAL_B
#define DIGITAL_Y E_CONTROLLER_DIGITAL_Y
#define DIGITAL_A E_CONTROLLER_DIGITAL_A
#endif

#ifdef __cplusplus
extern "C" {
namespace pros {
namespace c {
#endif

/**
 * Checks if the controller is connected.
 *
 * This function uses the following values of errno when an error state is reached:
 * EINVAL - A value other than E_CONTROLLER_MASTER or E_CONTROLLER_PARTNER is given.
 * EACCES - Another resource is currently trying to access the controller port.
 *
 * \param id
 *        The ID of the controller (e.g. the master or partner controller).
 *        Must be one of CONTROLLER_MASTER or CONTROLLER_PARTNER
 *
 * \return 1 if the controller is connected, 0 otherwise
 */
int32_t controller_is_connected(controller_id_e_t id);

/**
 * Gets the value of an analog channel (joystick) on a controller.
 *
 * This function uses the following values of errno when an error state is reached:
 * EINVAL - A value other than E_CONTROLLER_MASTER or E_CONTROLLER_PARTNER is given.
 * EACCES - Another resource is currently trying to access the controller port.
 *
 * \param id
 *        The ID of the controller (e.g. the master or partner controller).
 *        Must be one of CONTROLLER_MASTER or CONTROLLER_PARTNER
 * \param channel
 *        The analog channel to get.
 *        Must be one of ANALOG_LEFT_X, ANALOG_LEFT_Y, ANALOG_RIGHT_X, ANALOG_RIGHT_Y
 *
 * \return The current reading of the analog channel: [-127, 127].
 *         If the controller was not connected, then 0 is returned
 */
int32_t controller_get_analog(controller_id_e_t id, controller_analog_e_t channel);

/**
 * Checks if a digital channel (button) on the controller is currently pressed.
 *
 * \note The naming scheme for the buttons is not yet finalized as VEX finalizes
 *       the controller naming pattern
 *
 * This function uses the following values of errno when an error state is reached:
 * EINVAL - A value other than E_CONTROLLER_MASTER or E_CONTROLLER_PARTNER is given.
 * EACCES - Another resource is currently trying to access the controller port.
 *
 * \param id
 *        The ID of the controller (e.g. the master or partner controller).
 *        Must be one of CONTROLLER_MASTER or CONTROLLER_PARTNER
 * \param button
 *        The button to read.
 *        Must be one of DIGITAL_{RIGHT,DOWN,LEFT,UP,A,B,Y,X,R1,R2,L1,L2}
 *
 * \return 1 if the button on the controller is pressed.
 * 			   If the controller was not connected, then 0 is returned
 */
int32_t controller_get_digital(controller_id_e_t id, controller_digital_e_t button);

/**
 * Returns a rising-edge case for a controller button press.
 *
 * This function is not thread-safe.
 * Multiple tasks polling a single button may return different results under the
 * same circumstances, so only one task should call this function for any given
 * button. E.g., Task A calls this function for buttons 1 and 2. Task B may call
 * this function for button 3, but should not for buttons 1 or 2. A typical
 * use-case for this function is to call inside opcontrol to detect new button
 * presses, and not in any other tasks.
 *
 * This function uses the following values of errno when an error state is reached:
 * EINVAL - A value other than E_CONTROLLER_MASTER or E_CONTROLLER_PARTNER is given.
 * EACCES - Another resource is currently trying to access the controller port.
 *
 * \param id
 *        The ID of the controller (e.g. the master or partner controller).
 *        	Must be one of CONTROLLER_MASTER or CONTROLLER_PARTNER
 * \param button
 * 			  The button to read.
 * 			  Must be one of DIGITAL_{RIGHT,DOWN,LEFT,UP,A,B,Y,X,R1,R2,L1,L2}
 *
 * \return 1 if the button on the controller is pressed and had not been pressed
 *         the last time this function was called, 0 otherwise.
 */
int32_t controller_get_digital_new_press(controller_id_e_t id, controller_digital_e_t button);

/**
 * Gets the current voltage of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current voltage of the battery
 */
double battery_get_voltage(void);

/**
 * Gets the current current of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current current of the battery
 */
double battery_get_current(void);

/**
 * Gets the current temperature of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current temperature of the battery
 */
double battery_get_temperature(void);

/**
 * Gets the current capacity of the battery, as reported by VEXos.
 *
 * This function uses the following values of errno when an error state is reached:
 * EACCES - Another resource is currently trying to access the battery port.
 *
 * \return The current capacity of the battery
 */
double battery_get_capacity(void);

#ifdef __cplusplus
}
}
}
#endif

#endif  // _PROS_MISC_H_

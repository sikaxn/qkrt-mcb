/*
 * Copyright (c) 2020-2021 Queen's Knights Robotics Team
 *
 * This file is part of qkrt-mcb.
 *
 * qkrt-mcb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * qkrt-mcb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with qkrt-mcb.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "flywheel_on_command.hpp"
#include "tap/communication/gpio/leds.hpp"
#include "tap/control/command.hpp"
#include "control/control_operator_interface.hpp"
#include "flywheel_subsystem.hpp"

#include <thread>  // Include the thread library for delays
#include <chrono>  // Include the chrono library for time duration

#include "tap/algorithms/math_user_utils.hpp"



namespace control
{
namespace flywheel
{

void FlywheelOnCommand::initialize() {}

void busy_wait_delay_ms(int milliseconds) {
    volatile int count = 0;
    const int delay_count = 24000 * milliseconds; // Adjust this factor based on your CPU speed
    while (count < delay_count) {
        ++count;
    }
}

void FlywheelOnCommand::execute()
{
    //busy_wait_delay_ms(10000);
    flywheel->setDesiredOutput(spinning_pwm);
    busy_wait_delay_ms(10000); //10s delay
    flywheel->setDesiredOutput(OFF_PWM);
}

void FlywheelOnCommand::end(bool) { flywheel->setDesiredOutput(0.25f); }

bool FlywheelOnCommand::isFinished() const { return false; }
}  // namespace flywheel
}  // namespace control

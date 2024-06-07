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

// #include "chassis_omni_drive_command.hpp"

// #include "tap/algorithms/math_user_utils.hpp"

// #include "control/control_operator_interface.hpp"

// #include "chassis_subsystem.hpp"

// #include <thread>  // Include the thread library for delays
// #include <chrono>  // Include the chrono library for time duration

// using tap::algorithms::limitVal;

// namespace control::chassis
// {
// ChassisOmniDriveCommand::ChassisOmniDriveCommand(
//     ChassisSubsystem &chassis,
//     ControlOperatorInterface &operatorInterface)
//     : chassis(chassis),
//       operatorInterface(operatorInterface)
// {
//     addSubsystemRequirement(&chassis);
// }

// void ChassisOmniDriveCommand::execute()
// {
//     auto scale = [](float raw) -> float
//     { return limitVal(raw, -1.0f, 1.0f) * MAX_CHASSIS_SPEED_MPS; };
//     chassis.setVelocityOmniDrive(scale(1), scale(1), scale(1), scale(1));
//     modm::delay_ms(50000);
//     chassis.setVelocityOmniDrive(scale(-1), scale(-1), scale(-1), scale(-1));
//     modm::delay_ms(50000);
// }

// void ChassisOmniDriveCommand::end(bool) { chassis.setVelocityOmniDrive(.0f, .0f, .0f, .0f); }

// };  // namespace control::chassis

#include "chassis_omni_drive_command.hpp"
#include <chrono>  // Include the chrono library for time duration
#include <thread>  // Include the thread library for delays
#include "tap/algorithms/math_user_utils.hpp"
#include "control/control_operator_interface.hpp"
#include "chassis_subsystem.hpp"

using tap::algorithms::limitVal;

namespace control::chassis
{
ChassisOmniDriveCommand::ChassisOmniDriveCommand(
    ChassisSubsystem &chassis,
    ControlOperatorInterface &operatorInterface)
    : chassis(chassis),
      operatorInterface(operatorInterface)
{
    addSubsystemRequirement(&chassis);
}

bool flag = true;


//NOTE: calculate the actual distance it covers and the speed the robot moves at to get the time in seconds or miliseconds.
//Use that calculated time as delay to make the robot cover that exact distance and stop


void ChassisOmniDriveCommand::execute()
{
    if (flag)
    {
        chassis.setVelocityOmniDrive(1, 1, 1, 1);
        modm::delay_ms(50000);

        flag = false;
    }
    else
    {
        chassis.setVelocityOmniDrive(0, 0, 0, 0);
        modm::delay_ms(50000);
    }
}

void ChassisOmniDriveCommand::end(bool) { chassis.setVelocityOmniDrive(.0f, .0f, .0f, .0f); }

};  // namespace control::chassis

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

#include "chassis_subsystem.hpp"

#include "tap/algorithms/math_user_utils.hpp"

#include "drivers.hpp"

#include "control/control_operator_interface.hpp"

using tap::algorithms::limitVal;

namespace control::chassis
{

ChassisSubsystem::ChassisSubsystem(Drivers &drivers, const ChassisConfig &config)
    : tap::control::Subsystem(&drivers),
      desiredOutput{},
      pidControllers{},
      motors{
          Motor(&drivers, config.leftFrontId, config.canBus, false, "LF"),
          Motor(&drivers, config.leftBackId, config.canBus, false, "LB"),
          Motor(&drivers, config.rightFrontId, config.canBus, true, "RF"),
          Motor(&drivers, config.rightBackId, config.canBus, true, "RB")
      }
{
    for (auto &controller : pidControllers)
    {
        controller.setParameter(config.wheelVelocityPidConfig);
    }
}

void ChassisSubsystem::initialize()
{
    for (auto &motor : motors)
    {
        motor.initialize();
        drivers->pwm.setTimerFrequency(tap::gpio::Pwm::TIMER1,50.0f);
        drivers->pwm.setTimerFrequency(tap::gpio::Pwm::TIMER8,50.0f);
    }
}


void ChassisSubsystem::setVelocityOmniDrive(float leftFront,
                                            float leftBack,
                                            float rightFront,
                                            float rightBack)
{
    
    leftFront  = mpsToRpm(leftFront);
    leftBack   = mpsToRpm(leftBack);
    rightFront = mpsToRpm(rightFront);
    rightBack  = mpsToRpm(rightBack);

    leftFront  = limitVal(leftFront,  -MAX_WHEELSPEED_RPM, MAX_WHEELSPEED_RPM);
    leftBack   = limitVal(leftBack,   -MAX_WHEELSPEED_RPM, MAX_WHEELSPEED_RPM);
    rightFront = limitVal(rightFront, -MAX_WHEELSPEED_RPM, MAX_WHEELSPEED_RPM);
    rightBack  = limitVal(rightBack,  -MAX_WHEELSPEED_RPM, MAX_WHEELSPEED_RPM);

    desiredOutput[static_cast<uint8_t>(MotorId::LF)] = leftFront;
    desiredOutput[static_cast<uint8_t>(MotorId::LB)] = leftBack;
    desiredOutput[static_cast<uint8_t>(MotorId::RF)] = rightFront;
    desiredOutput[static_cast<uint8_t>(MotorId::RB)] = rightBack;
}

void ChassisSubsystem:: setPWMArcadedrive(float left, float right){
    drivers->pwm.write(left, tap::gpio::Pwm::C1);
    drivers->pwm.write(left, tap::gpio::Pwm::C2);
    drivers->pwm.write(right, tap::gpio::Pwm::C3);
    drivers->pwm.write(right, tap::gpio::Pwm::C4);
}

void ChassisSubsystem::refresh()
{
    auto runPid = [](Pid &pid, Motor &motor, float desiredOutput) {
        pid.update(desiredOutput - motor.getShaftRPM());
        motor.setDesiredOutput(pid.getValue());
    };

    for (size_t ii = 0; ii < motors.size(); ii++)
    {
        runPid(pidControllers[ii], motors[ii], desiredOutput[ii]);
    }
}

}  // namespace control::chassis

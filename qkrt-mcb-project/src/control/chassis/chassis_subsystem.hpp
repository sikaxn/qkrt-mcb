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

#pragma once

#include <array>

#include "tap/control/subsystem.hpp"
#include "tap/util_macros.hpp"

#include "modm/math/filter/pid.hpp"
#include "modm/math/geometry/angle.hpp"

#if defined(PLATFORM_HOSTED) && defined(ENV_UNIT_TESTS)
#include "tap/mock/dji_motor_mock.hpp"
#else
#include "tap/motor/dji_motor.hpp"
#endif

class Drivers;

namespace control::chassis
{
struct ChassisConfig
{
    tap::motor::MotorId leftFrontId;
    tap::motor::MotorId leftBackId;
    tap::motor::MotorId rightBackId;
    tap::motor::MotorId rightFrontId;
    tap::can::CanBus canBus;
    modm::Pid<float>::Parameter wheelVelocityPidConfig;
};

///
/// @brief This subsystem encapsulates four motors that control the chassis.
///
class ChassisSubsystem : public tap::control::Subsystem
{
public:
    /// @brief Motor ID to index into the velocityPid and motors object.
    enum class MotorId : uint8_t
    {
        LF = 0,  ///< Left front
        LB,      ///< Left back
        RF,      ///< Right front
        RB,      ///< Right back
        NUM_MOTORS,
    };

    using Pid = modm::Pid<float>;

#if defined(PLATFORM_HOSTED) && defined(ENV_UNIT_TESTS)
    using Motor = testing::NiceMock<tap::mock::DjiMotorMock>;
#else
    using Motor = tap::motor::DjiMotor;
#endif

    static constexpr float MAX_WHEELSPEED_RPM = 7000;

    ChassisSubsystem(Drivers& drivers, const ChassisConfig& config);

    ///
    /// @brief Initializes the drive motors.
    ///
    void initialize() override;

    ///
    /// @brief Control the chassis using omni drive. Sets the wheel velocity of the four drive
    /// motors individually based on on the input left/right desired velocity.
    ///
    /// @param leftFront Desired speed in m/s of the left-front wheel of the chassis. Positive speed is
    /// forward, negative is backwards.
    /// @param leftBack Desired speed in m/s of the left-back wheel of the chassis. Positive speed is
    /// forward, negative is backwards.
    /// @param rightFront Desired chassis speed in m/s of the right-front of the chassis. Positive speed is
    /// forward, negative is backwards.
    /// @param rightBack Desired speed in m/s of the right-back wheel of the chassis. Positive speed is
    /// forward, negative is backwards.
    ///
    void setVelocityOmniDrive(float leftFront, float leftBack, float rightFront, float rightBack);
    void setPWMArcadedrive(float left, float right);
    ///
    /// @brief Runs velocity PID controllers for the drive motors.
    ///
    void refresh() override;

    const char* getName() override { return "Chassis"; }

private:
    inline float mpsToRpm(float mps)
    {
        static constexpr float GEAR_RATIO = 19.0f;
        static constexpr float WHEEL_DIAMETER_M = 0.056f;
        static constexpr float WHEEL_CIRCUMFERANCE_M = M_PI * WHEEL_DIAMETER_M;
        static constexpr float SEC_PER_M = 60.0f;

        return (mps / WHEEL_CIRCUMFERANCE_M) * SEC_PER_M * GEAR_RATIO;
    }

    /// Desired wheel output for each motor
    std::array<float, static_cast<uint8_t>(MotorId::NUM_MOTORS)> desiredOutput;

    /// PID controllers. Input desired wheel velocity, output desired motor current.
    std::array<Pid, static_cast<uint8_t>(MotorId::NUM_MOTORS)> pidControllers;

protected:
    /// Motors.
    std::array<Motor, static_cast<uint8_t>(MotorId::NUM_MOTORS)> motors;
};  // class ChassisSubsystem
}  // namespace control::chassis

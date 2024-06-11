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

#include "control_operator_interface.hpp"

#include "tap/algorithms/math_user_utils.hpp"
#include "tap/communication/serial/remote.hpp"
#include "tap/communication/sensors/imu/bmi088/bmi088.hpp"
#include "tap/architecture/clock.hpp"

#include "internal.hpp"

using tap::algorithms::limitVal;
using tap::communication::serial::Remote;
using tap::communication::sensors::imu::bmi088::Bmi088;

namespace control
{

#define WHEEL_DEADZONE      100.0f

struct ControlState {
    float x = 0.0;
    float y = 0.0;
    float w = 0.0;
    float normFactor = 1.0;
    float pitch = 0.0;
    float yaw = 0.0;
    float moveSpeed = 0.3f;
    float pitchSensitivity = 0.0075f;
    float yawSensitivity = 0.0075f;
    bool flywheel = false;
    bool agitator = false;
    bool beyblade = false;
};

static ControlState control_s;

ControlOperatorInterface::ControlOperatorInterface(Remote &remote, Bmi088& imu)
    : activeDevice(DeviceType::CONTROLLER), remote(remote), imu(imu) 
{
}

void ControlOperatorInterface::pollInputDevices() {
    /* toggle between controller and keyboard mode */
    static bool pressedLastInterval = false;

    if(remote.getSwitch(Remote::Switch::LEFT_SWITCH) == Remote::SwitchState::UP){
        if (!pressedLastInterval) {
            activeDevice = (activeDevice == DeviceType::CONTROLLER) ?
                    DeviceType::KEYBOARDMOUSE : DeviceType::CONTROLLER;
            pressedLastInterval = true;
        }
        else return;
    }
    else pressedLastInterval = false;
/*
    if (remote.keyPressed(Remote::Key::Q)) {
        if (!pressedLastInterval) {
            activeDevice = (activeDevice == DeviceType::CONTROLLER) ?
                    DeviceType::KEYBOARDMOUSE : DeviceType::CONTROLLER;
            pressedLastInterval = true;
        }
        else return;
    }
    else pressedLastInterval = false;*/

    /* poll for input using the chosen active device */
    static float rawX, rawY;
    static int16_t wheelInput;

    switch (activeDevice) {
        case DeviceType::CONTROLLER:
            rawX = remote.getChannel(Remote::Channel::LEFT_HORIZONTAL);
            rawY = remote.getChannel(Remote::Channel::LEFT_VERTICAL);
            control_s.pitch = remote.getChannel(Remote::Channel::RIGHT_VERTICAL);
            control_s.yaw   = remote.getChannel(Remote::Channel::RIGHT_HORIZONTAL) - imu.getGz()/515;
            //control_s.flywheel = remote.getSwitch(Remote::Switch::LEFT_SWITCH) == Remote::SwitchState::UP;
            control_s.flywheel = (remote.getSwitch(Remote::Switch::RIGHT_SWITCH) == Remote::SwitchState::MID || remote.getSwitch(Remote::Switch::RIGHT_SWITCH) == Remote::SwitchState::UP);
            control_s.agitator = (remote.getSwitch(Remote::Switch::RIGHT_SWITCH) == Remote::SwitchState::UP );
            wheelInput = remote.getWheel();
            control_s.beyblade =
                wheelInput >  WHEEL_DEADZONE ? true  :
                wheelInput < -WHEEL_DEADZONE ? false :
                control_s.beyblade;
            break;
        case DeviceType::KEYBOARDMOUSE:
            rawX = remote.keyPressed(Remote::Key::D) * control_s.moveSpeed + remote.keyPressed(Remote::Key::A) * -control_s.moveSpeed;
            rawY = remote.keyPressed(Remote::Key::W) * control_s.moveSpeed + remote.keyPressed(Remote::Key::S) * -control_s.moveSpeed;
            control_s.pitch = -static_cast<float>(remote.getMouseY()) * control_s.pitchSensitivity;
            control_s.yaw   =  static_cast<float>(remote.getMouseX()) * control_s.yawSensitivity - imu.getGz()/500;
            control_s.flywheel = remote.getMouseR();
            control_s.agitator = remote.getMouseL();
            control_s.beyblade =
                remote.keyPressed(Remote::Key::Z) ? true  :
                remote.keyPressed(Remote::Key::X) ? false :
                control_s.beyblade;
            break;
        default:
            rawX = rawY = 0.0;
            control_s.pitch = control_s.yaw = 0.0;
    }



    control_s.x = control_s.yaw;//std::cos(-internal::turretYaw) * rawX - std::sin(-internal::turretYaw) * rawY;
    control_s.y = control_s.pitch;//std::sin(-internal::turretYaw) * rawX + std::cos(-internal::turretYaw) * rawY;
    control_s.w = 0;//control_s.beyblade ? 0.4f : 0.0f;
    control_s.normFactor = std::max(std::abs(control_s.x) + std::abs(control_s.y) + std::abs(control_s.w), 1.0f);
}

float ControlOperatorInterface::getChassisOmniLeftFrontInput() {
    return (control_s.y + control_s.x + control_s.w) / control_s.normFactor;
}

float ControlOperatorInterface::getChassisOmniLeftBackInput() {
    return (control_s.y - control_s.x + control_s.w) / control_s.normFactor;
}

float ControlOperatorInterface::getChassisOmniRightFrontInput() {
    return (control_s.y - control_s.x - control_s.w) / control_s.normFactor;
}

float ControlOperatorInterface::getChassisOmniRightBackInput() {
    return (control_s.y + control_s.x - control_s.w) / control_s.normFactor;
}

float ControlOperatorInterface::getTurretPitchInput() {
    return control_s.pitch;
}

float ControlOperatorInterface::getTurretYawInput() {
    return control_s.yaw;
}

bool ControlOperatorInterface::getFlyWheelInput() {
    return control_s.flywheel;
}

bool ControlOperatorInterface::getAgitatorInput() {
    return control_s.agitator;
}


}  // namespace control

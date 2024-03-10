/*****************************************************************************/
/********** !!! WARNING: CODE GENERATED BY TAPROOT. DO NOT EDIT !!! **********/
/*****************************************************************************/

/*
 * Copyright (c) 2020-2021 Advanced Robotics at the University of Washington <robomstr@uw.edu>
 *
 * This file is part of Taproot.
 *
 * Taproot is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Taproot is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Taproot.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "move_command.hpp"

#include "tap/algorithms/math_user_utils.hpp"
#include "tap/architecture/clock.hpp"

namespace tap
{
namespace control
{
namespace setpoint
{
MoveCommand::MoveCommand(
    SetpointSubsystem* setpointSubsystem,
    float targetDisplacement,
    uint32_t moveTime,
    uint32_t pauseAfterMoveTime,
    bool setToTargetOnEnd,
    float setpointTolerance)
    : setpointSubsystem(setpointSubsystem),
      targetDisplacement(targetDisplacement),
      rampToTargetValue(0.0f),
      moveTime(moveTime),
      pauseAfterMoveTime(pauseAfterMoveTime),
      setpointTolerance(setpointTolerance),
      previousMoveTime(0),
      setToTargetOnEnd(setToTargetOnEnd)
{
    this->addSubsystemRequirement(setpointSubsystem);
}

void MoveCommand::initialize()
{
    // set the ramp start and target values
    float currentSetpoint = setpointSubsystem->getSetpoint();
    float targetValue = currentSetpoint + targetDisplacement;

    rampToTargetValue.setTarget(targetValue);
    rampToTargetValue.setValue(currentSetpoint);

    previousMoveTime = tap::arch::clock::getTimeMilliseconds();

    // Stop timeout, doesn't start until target reached
    minMoveTimeout.stop();
}

void MoveCommand::execute()
{
    // Don't move setpoint if the subsystem is jammed or offline
    if (setpointSubsystem->isJammed() || !setpointSubsystem->isOnline())
    {
        previousMoveTime = tap::arch::clock::getTimeMilliseconds();
        return;
    }
    // update the subsystem setpoint ramp
    uint32_t currTime = tap::arch::clock::getTimeMilliseconds();
    rampToTargetValue.update(
        (currTime - previousMoveTime) * targetDisplacement / static_cast<float>(moveTime));
    previousMoveTime = currTime;
    setpointSubsystem->setSetpoint(rampToTargetValue.getValue());

    if (minMoveTimeout.isStopped() &&
        fabsf(setpointSubsystem->getCurrentValue() - rampToTargetValue.getTarget()) <=
            setpointTolerance)
    {
        minMoveTimeout.restart(pauseAfterMoveTime);
    }
}

void MoveCommand::end(bool interrupted)
{
    // if the subsystem is not interrupted, then it exited normally
    // (i.e. reached the desired value) and is not jammed. If it is
    // jammed we thus want to set the subsystem value to the current value,
    // so the motor does not attempt to keep rotating forward (and possible stalling)
    if (interrupted || setpointSubsystem->isJammed() || !setpointSubsystem->isOnline() ||
        !setToTargetOnEnd)
    {
        setpointSubsystem->setSetpoint(setpointSubsystem->getCurrentValue());
    }
    else
    {
        setpointSubsystem->setSetpoint(rampToTargetValue.getTarget());
    }
}

bool MoveCommand::isFinished() const
{
    // The subsystem is jammed or offline or it is within the setpoint tolerance, the ramp is
    // finished, and the minimum rotate time is expired.
    return setpointSubsystem->isJammed() || !setpointSubsystem->isOnline() ||
           (algorithms::compareFloatClose(
                setpointSubsystem->getCurrentValue(),
                rampToTargetValue.getTarget(),
                setpointTolerance) &&
            rampToTargetValue.isTargetReached() && minMoveTimeout.isExpired());
}

}  // namespace setpoint

}  // namespace control

}  // namespace tap
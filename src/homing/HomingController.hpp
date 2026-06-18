#pragma once

#include "../motionExecution/StepperAxis.hpp"
#include "../motionExecution/CoreXYKinematics.hpp"
#include "settings/RuntimeSettings.hpp"
#include "rtos/MotionState.hpp"
#include "rtos/MotionCommand.hpp"

class HomingController {
public:
    HomingController(StepperAxis& axisA, StepperAxis& axisB, MotorDriver& driverA, MotorDriver& driverB, MotionState& motionState, MotionCommand& motionCommand, RuntimeSettings& runtimeSettings, CoreXYKinematics& kinematics);

    void home();

private:
    StepperAxis& _axisA;
    StepperAxis& _axisB;
    MotorDriver& _driverA;
    MotorDriver& _driverB;
    MotionState& _motionState;
    MotionCommand& _motionCommand;
    RuntimeSettings& _runtimeSettings;
    CoreXYKinematics& _kinematics;

    void moveToLimit(bool Afw, bool Bfw, uint16_t backOffSteps);
    bool checkPauseAbort();  // Handle pause/abort during homing; return true if should abort
};
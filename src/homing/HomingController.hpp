#pragma once

#include "../motionExecution/StepperAxis.hpp"
#include "settings/RuntimeSettings.hpp"
#include "rtos/MotionState.hpp"

class HomingController {
public:
    HomingController(StepperAxis& axisA, StepperAxis& axisB, MotorDriver& driverA, MotorDriver& driverB, MotionState& motionState, RuntimeSettings& runtimeSettings);

    void home();

private:
    StepperAxis& _axisA;
    StepperAxis& _axisB;
    MotorDriver& _driverA;
    MotorDriver& _driverB;
    MotionState& _motionState;
    RuntimeSettings& _runtimeSettings;

    void moveToLimit(bool Afw, bool Bfw, uint16_t backOffSteps);
    bool checkPauseAbort();  // Handle pause/abort during homing; return true if should abort
};
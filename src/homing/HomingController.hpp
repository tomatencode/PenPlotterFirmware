#pragma once

#include "../motionExecution/StepperAxis.hpp"
#include "rtos/MotionState.hpp"

// Forward declaration 
// it: why needed? cyclic dep? 
class RuntimeSettings;

class HomingController {
public:
   // it: again: you may want to use move semantics to clarify ownership and lifetimes of the dependencies. As it is now, we need to ensure that all references remain valid for the lifetime of the HomingController instance, which can be error-prone.
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
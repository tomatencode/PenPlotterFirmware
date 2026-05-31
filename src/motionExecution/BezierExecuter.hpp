#pragma once

#include "CoreXYKinematics.hpp"
#include "StepperAxis.hpp"
#include "rtos/MotionState.hpp"
#include "rtos/MotionCommand.hpp"

class BezierExecuter {
public:
    BezierExecuter(StepperAxis& axisA, StepperAxis& axisB, CoreXYKinematics& kinematics, MotionState& motionState, MotionCommand& motionCommand);

    void bezierTo(const XYPos& targetPos, double mm_per_s, bool clipToWorkspace);
    XYPos getCurrentPos() const;

private:
    StepperAxis& _axisA;
    StepperAxis& _axisB;
    CoreXYKinematics& _kinematics;
    MotionState& _motionState;
    MotionCommand& _motionCommand;
};

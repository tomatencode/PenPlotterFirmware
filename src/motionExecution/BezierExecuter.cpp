#include "BezierExecuter.hpp"
#include "config/HardwareConfig.hpp"
#include <cmath>
#include <algorithm>
#include <Arduino.h>

BezierExecuter::BezierExecuter(StepperAxis& axisA, StepperAxis& axisB, CoreXYKinematics& kinematics, MotionState& motionState, MotionCommand& motionCommand)
    : _axisA(axisA), _axisB(axisB), _kinematics(kinematics), _motionState(motionState), _motionCommand(motionCommand) {}

XYPos BezierExecuter::getCurrentPos() const {
    MotorSteps steps = {_axisA.positionSteps(), _axisB.positionSteps()};
    return _kinematics.stepsToMm(steps);
}

void BezierExecuter::bezierTo(const XYPos& targetPos, double mm_per_s, bool clipToWorkspace) {
    MotorSteps currentSteps = {_axisA.positionSteps(), _axisB.positionSteps()};
    uint16_t msA = _axisA.microsteps();
    uint16_t msB = _axisB.microsteps();

    XYPos startPos = _kinematics.stepsToMm(currentSteps);

    double dx, dy;

    if (clipToWorkspace) {
        double clippedX = std::clamp(targetPos.xMm, 0.0, static_cast<double>(MAX_X_MM));
        double clippedY = std::clamp(targetPos.yMm, 0.0, static_cast<double>(MAX_Y_MM));
        if (clippedX != targetPos.xMm || clippedY != targetPos.yMm) {
            ESP_LOGW("BezierExecuter", "Target position (%.2f, %.2f) is outside workspace. Clipping to (%.2f, %.2f).",
                     targetPos.xMm, targetPos.yMm, clippedX, clippedY);
        }
        dx = clippedX - startPos.xMm;
        dy = clippedY - startPos.yMm;
    } else {
        dx = targetPos.xMm - startPos.xMm;
        dy = targetPos.yMm - startPos.yMm;
    }

    double distance_mm = std::sqrt(dx * dx + dy * dy);
    if (distance_mm <= 0.0) return;

    if (mm_per_s <= 0.0) return;

    MotorSteps dfullsteps = _kinematics.mmToSteps({dx, dy});

    int32_t deltaA = static_cast<int32_t>((dfullsteps.a) * msA);
    int32_t deltaB = static_cast<int32_t>((dfullsteps.b) * msB);

    int32_t absA = std::abs(deltaA);
    int32_t absB = std::abs(deltaB);

    bool dirA = (deltaA >= 0);
    bool dirB = (deltaB >= 0);

    int32_t stepsInLoop = std::max(absA, absB);
    if (stepsInLoop == 0) return;

    double move_time_s = distance_mm / mm_per_s;
    double step_frequency = stepsInLoop / move_time_s;
    uint32_t step_interval_us = static_cast<uint32_t>(1e6 / step_frequency);
    if (step_interval_us == 0) step_interval_us = 1;

    uint32_t next_step_time = micros();

    long errA = 0;
    long errB = 0;

    for (int32_t i = 0; i < stepsInLoop; i++) {

        while ((int32_t)(micros() - next_step_time) < 0) {
            yield();
        }

        if (_motionCommand.getCommand() == MotionCommandType::PAUSE) {
            _motionState.setState(MotionStateType::PAUSED);
            while (_motionCommand.getCommand() == MotionCommandType::PAUSE) {
                yield();
            }
            _motionState.setState(MotionStateType::RUNNING);
            next_step_time = micros(); // Reset so pause duration doesn't cause a step burst on resume
        }
        else if (_motionCommand.getCommand() == MotionCommandType::ABORT) {
            return;
        }

        next_step_time += step_interval_us;

        errA += absA;
        errB += absB;

        if (errA >= stepsInLoop) {
            _axisA.step(dirA);
            errA -= stepsInLoop;
        }

        if (errB >= stepsInLoop) {
            _axisB.step(dirB);
            errB -= stepsInLoop;
        }

        _motionState.setMachineX(startPos.xMm + (i + 1) * dx / stepsInLoop);
        _motionState.setMachineY(startPos.yMm + (i + 1) * dy / stepsInLoop);
    }

    _motionState.setMachineX(targetPos.xMm);
    _motionState.setMachineY(targetPos.yMm);
}

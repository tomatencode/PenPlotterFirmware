#include "CoreXYKinematics.hpp"

CoreXYKinematics::CoreXYKinematics(double stepsPerMm, bool swapXY, bool invertX, bool invertY)
    : _stepsPerMm(stepsPerMm), _swapXY(swapXY), _invertX(invertX), _invertY(invertY) {}

// Convert X/Y space in mm to motor step space
MotorSteps CoreXYKinematics::mmToSteps(const XYPos& pos) const {
    MotorSteps m;
    if (_swapXY) {
        m.a = (pos.yMm * (_invertY ? -1 : 1) + pos.xMm * (_invertX ? -1 : 1)) * _stepsPerMm;
        m.b = (pos.yMm * (_invertY ? -1 : 1) - pos.xMm * (_invertX ? -1 : 1)) * _stepsPerMm;
    } else {
        m.a = (pos.xMm * (_invertX ? -1 : 1) + pos.yMm * (_invertY ? -1 : 1)) * _stepsPerMm;
        m.b = (pos.xMm * (_invertX ? -1 : 1) - pos.yMm * (_invertY ? -1 : 1)) * _stepsPerMm;
    }
    return m;
}

// Convert motor step space to X/Y space in mm
XYPos CoreXYKinematics::stepsToMm(const MotorSteps& steps) const {
    XYPos pos;
    if (_swapXY) {
        pos.yMm = (steps.a + steps.b) / (2.0 * _stepsPerMm);
        pos.xMm = (steps.a - steps.b) / (2.0 * _stepsPerMm);
    } else {
        pos.xMm = (steps.a + steps.b) / (2.0 * _stepsPerMm);
        pos.yMm = (steps.a - steps.b) / (2.0 * _stepsPerMm);
    }
    if (_invertX) pos.xMm = -pos.xMm;
    if (_invertY) pos.yMm = -pos.yMm;
    return pos;
}
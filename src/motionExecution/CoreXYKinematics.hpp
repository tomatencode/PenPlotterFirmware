#pragma once
#include <cstdint>

struct MotorSteps {
    double a;
    double b;
};

struct XYPos {
    double xMm;
    double yMm;
};

class CoreXYKinematics {
public:
    explicit CoreXYKinematics(double stepsPerMm, bool swapXY = false, bool invertX = false, bool invertY = false);

    MotorSteps mmToSteps(const XYPos& pos) const;
    XYPos stepsToMm(const MotorSteps& steps) const;

private:
    double _stepsPerMm;
    bool _swapXY;
    bool _invertX;
    bool _invertY;
};
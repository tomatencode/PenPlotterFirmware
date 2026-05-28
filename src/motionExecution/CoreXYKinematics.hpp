#pragma once
#include <cstdint>

struct MotorSteps {
    // it: arent the steps integrals?
    double a;
    double b;
};

struct XYPos {
    // it: you may want to use std::complex<double> for representing 2D positions to get expected math operations for free
    double xMm;
    double yMm;
};

class CoreXYKinematics {
public:
    explicit CoreXYKinematics(double newStepsPerMm);

    MotorSteps mmToSteps(const XYPos& pos) const;
    XYPos stepsToMm(const MotorSteps& steps) const;

private:
    double _stepsPerMm;
};
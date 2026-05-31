#include "MotionExecuter.hpp"
#include <cmath>
#include <algorithm>
#include <Arduino.h>


MotionExecuter::MotionExecuter(BezierExecuter& bezierExecuter, MotionCommand& motionCommand, RuntimeSettings& runtimeSettings)
    : _bezierExecuter(bezierExecuter), _motionCommand(motionCommand), _runtimeSettings(runtimeSettings)
    {}

XYPos MotionExecuter::getCurrentPos() const {
    return _bezierExecuter.getCurrentPos();
}

void MotionExecuter::LineToXY(
    const XYPos& targetPos,
    double mm_per_s
) {
    _bezierExecuter.bezierTo(targetPos, mm_per_s, false);
}

void MotionExecuter::arcToXY(
    const XYPos& targetPos,
    const XYPos& centerPos,
    bool clockwise,
    double mm_per_s
) {
    XYPos currentPos = _bezierExecuter.getCurrentPos();

    // Calculate the radius and angles
    double dx = currentPos.xMm - centerPos.xMm;
    double dy = currentPos.yMm - centerPos.yMm;
    double radius = std::sqrt(dx * dx + dy * dy);

    double startAngle = atan2(currentPos.yMm - centerPos.yMm, currentPos.xMm - centerPos.xMm);
    double endAngle = atan2(targetPos.yMm - centerPos.yMm, targetPos.xMm - centerPos.xMm);

    // Calculate the total angle to move through
    double totalAngle;
    if (clockwise) {
        totalAngle = startAngle - endAngle;
        if (totalAngle <= 0) totalAngle += 2 * M_PI; // Ensure positive angle
    } else {
        totalAngle = endAngle - startAngle;
        if (totalAngle <= 0) totalAngle += 2 * M_PI; // Ensure positive angle
    }

    // Calculate the arc length and time to move
    double arcLength = radius * totalAngle;

    if (_runtimeSettings.minFeatureSize_mm() <= 0.0) return;

    int linesegments = static_cast<int>(arcLength / _runtimeSettings.minFeatureSize_mm()); // Number of line segments to approximate the arc
    linesegments = std::max(1, std::min(linesegments, 1000));

    for (int32_t i = 1; i <= linesegments; i++) {
        // Calculate the current angle along the arc
        double t = static_cast<double>(i) / (linesegments); // Normalized progress along the arc
        double angle = startAngle + (clockwise ? -1 : 1) * t * totalAngle;
        double x = centerPos.xMm + radius * cos(angle);
        double y = centerPos.yMm + radius * sin(angle);
        LineToXY({x, y}, mm_per_s); // Move to the calculated position

        // Check for abort
        if (_motionCommand.getCommand() == MotionCommandType::ABORT)
        {
            return; // Exit the motion loop immediately on abort
        }
    }
}

void MotionExecuter::quadraticBezierToXY(
    const XYPos& controlPoint,
    const XYPos& targetPos,
    float mm_per_s
) {
    XYPos startPos = _bezierExecuter.getCurrentPos();

    auto bezier = [&](double t) -> XYPos {
        double u = 1.0 - t;
        return {
            u*u*startPos.xMm + 2*u*t*controlPoint.xMm + t*t*targetPos.xMm,
            u*u*startPos.yMm + 2*u*t*controlPoint.yMm + t*t*targetPos.yMm
        };
    };

    const double maxSegmentLength = _runtimeSettings.minFeatureSize_mm();
    double t0 = 0.0;
    XYPos lastPos = startPos;
    const double dt = 0.01; // initial increment

    while (t0 < 1.0) {
        double t1 = t0 + dt;
        if (t1 > 1.0) t1 = 1.0;

        XYPos p1 = bezier(t1);

        // Fully adaptive: increase t1 until segment length >= maxSegmentLength
        while (t1 < 1.0) {
            double dx = p1.xMm - lastPos.xMm;
            double dy = p1.yMm - lastPos.yMm;
            double dist2 = dx*dx + dy*dy;
            if (dist2 >= maxSegmentLength*maxSegmentLength) break;

            t1 += dt;
            if (t1 > 1.0) t1 = 1.0;
            p1 = bezier(t1);
        }

        LineToXY(p1, mm_per_s);

        // Prepare for next segment
        lastPos = p1;
        t0 = t1;

        // Check for abort
        if (_motionCommand.getCommand() == MotionCommandType::ABORT)
        {
            return; // Exit the motion loop immediately on abort
        }
    }

    // Only move to exact target if not already there
    double dx = targetPos.xMm - lastPos.xMm;
    double dy = targetPos.yMm - lastPos.yMm;
    if (dx*dx + dy*dy > 1e-12) LineToXY(targetPos, mm_per_s);
}

void MotionExecuter::cubicBezierToXY(
    const XYPos& controlPoint1,
    const XYPos& controlPoint2,
    const XYPos& targetPos,
    float mm_per_s
) {
    XYPos startPos = _bezierExecuter.getCurrentPos();

    // Lambda to evaluate the cubic Bézier at t
    auto bezier = [&](double t) -> XYPos {
        double u = 1.0 - t;
        double uu = u * u;
        double uuu = uu * u;
        double tt = t * t;
        double ttt = tt * t;

        XYPos p;
        p.xMm = uuu * startPos.xMm
                 + 3 * uu * t * controlPoint1.xMm
                 + 3 * u * tt * controlPoint2.xMm
                 + ttt * targetPos.xMm;

        p.yMm = uuu * startPos.yMm
                 + 3 * uu * t * controlPoint1.yMm
                 + 3 * u * tt * controlPoint2.yMm
                 + ttt * targetPos.yMm;

        return p;
    };

    const double maxSegmentLength = _runtimeSettings.minFeatureSize_mm();

    double t0 = 0.0;
    XYPos lastPos = startPos;

    // Fully adaptive loop
    while (t0 < 1.0) {
        // Small initial increment in t
        double dt = 0.01;
        double t1 = t0 + dt;
        if (t1 > 1.0) t1 = 1.0;

        XYPos p1 = bezier(t1);

        // Increase t1 until distance >= maxSegmentLength or t1 reaches 1.0
        while (t1 < 1.0) {
            double dx = p1.xMm - lastPos.xMm;
            double dy = p1.yMm - lastPos.yMm;
            double dist = std::sqrt(dx*dx + dy*dy);
            if (dist >= maxSegmentLength) break;

            t1 += dt;
            if (t1 > 1.0) t1 = 1.0;
            p1 = bezier(t1);
        }

        // Move the pen to the computed point
        LineToXY(p1, mm_per_s);

        // Prepare for next segment
        lastPos = p1;
        t0 = t1;

        // Check for abort
        if (_motionCommand.getCommand() == MotionCommandType::ABORT)
        {
            return; // Exit the motion loop immediately on abort
        }
    }

    // Ensure the pen ends exactly at target
    LineToXY(targetPos, mm_per_s);
}
#pragma once

#include <cstdint>

#include "BezierExecuter.hpp"
#include "rtos/MotionCommand.hpp"
#include "settings/RuntimeSettings.hpp"


class MotionExecuter {
    public:
        MotionExecuter(BezierExecuter& bezierExecuter, MotionCommand& motionCommand, RuntimeSettings& runtimeSettings);

        void LineToXY(
            const XYPos& targetPos,
            double mm_per_s
        );
        void arcToXY(
            const XYPos& targetPos,
            const XYPos& centerPos,
            bool clockwise,
            double mm_per_s
        );
        void quadraticBezierToXY(
            const XYPos& controlPoint,
            const XYPos& targetPos,
            float mm_per_s
        );
        void cubicBezierToXY(
            const XYPos& controlPoint1,
            const XYPos& controlPoint2,
            const XYPos& targetPos,
            float mm_per_s
        );

        XYPos getCurrentPos() const;

    private:
        BezierExecuter& _bezierExecuter;
        MotionCommand& _motionCommand;
        RuntimeSettings& _runtimeSettings;
};
#pragma once

#include <Arduino.h>
#include "settings/RuntimeSettings.hpp"
#include "settings/SettingPersistence.hpp"
#include "settings/SettingObserver.hpp"
#include "hardware/steppers/Stepper.hpp"
#include "hardware/drivers/TMC2209Driver.hpp"
#include "motionExecution/StepperAxis.hpp"
#include "motionExecution/CoreXYKinematics.hpp"
#include "motionExecution/MotionExecuter.hpp"
#include "motionExecution/BezierExecuter.hpp"
#include "pen/Pen.hpp"
#include "homing/HomingController.hpp"
#include "gcode/GCodeExecuter.hpp"
#include "gcode/GCodeParser.hpp"
#include "rtos/MotionState.hpp"
#include "rtos/RtosQueue.hpp"
#include "rtos/GcodeMessage.hpp"


class PlottingController : public SettingObserver
{
public:
    /*
        it: Similar to Pen, should PlottingController become owner of its hardware components (drivers, steppers, pen) and manage their lifecycles? This would encapsulate the hardware management within the controller and reduce the risk of dangling references. However, it also means that PlottingController would need to handle the initialization and cleanup of these components, which could increase its complexity.
    */
    PlottingController(MotionState& motionState, RtosQueue<GcodeMessage>& gcodeQueue, SettingPersistence& settingsPersistence, RuntimeSettings& runtimeSettings);
    ~PlottingController();

    void init();
    void update();

private:

    void configureDriver(TMC2209Driver& driver);

    void onRelevantSettingsChanged() override;

    // UART
    HardwareSerial _driverSerial;

    // Servo
    Servo _penServo;

    // Raw drivers
    TMC2209Stepper _rawDriverA;
    TMC2209Stepper _rawDriverB;

    // Driver abstraction
    TMC2209Driver _driverA;
    TMC2209Driver _driverB;

    // Motion hardware
    Stepper _stepA;
    Stepper _stepB;

    StepperAxis _axisA;
    StepperAxis _axisB;

    Pen _pen;

    HomingController _homingController;

    CoreXYKinematics _kinematics;

    BezierExecuter _bezierExecuter;

    MotionExecuter _motionExecuter;

    GCodeExecuter _gcodeExecuter;

    // State and communication
    MotionState& _motionState;
    RtosQueue<GcodeMessage>& _gcodeQueue;
    RuntimeSettings& _runtimeSettings;
    SettingPersistence& _settingPersistence;
};
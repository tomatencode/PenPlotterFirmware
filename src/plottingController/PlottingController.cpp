#include "PlottingController.hpp"

#include "config/PinsConfig.hpp"
#include "config/HardwareConfig.hpp"
#include "rtos/RtosQueue.hpp"
#include "rtos/GcodeMessage.hpp"
#include <esp_log.h>

static const char* TAG = "PlottingController";

PlottingController::PlottingController(MotionState& motionState, MotionCommand& motionCommand, RtosQueue<GcodeMessage>& gcodeQueue, SettingPersistence& settingsPersistence, RuntimeSettings& runtimeSettings)
    : SettingObserver({Setting::DriverCurrent, Setting::Microsteps, Setting::StallguardThreshold}),
    
    _motionState(motionState),
    _motionCommand(motionCommand),
    _gcodeQueue(gcodeQueue),
    _settingPersistence(settingsPersistence),
    _runtimeSettings(runtimeSettings),
    
    _driverSerial(1),

    _rawDriverA(&_driverSerial, R_SENSE, 0),
    _rawDriverB(&_driverSerial, R_SENSE, 2),

    _driverA(_rawDriverA),
    _driverB(_rawDriverB),

    _stepA(STEP_PIN_A, DIR_PIN_A),
    _stepB(STEP_PIN_B, DIR_PIN_B),

    _axisA(_stepA, _driverA, true),
    _axisB(_stepB, _driverB, true),

    _pen(_penServo, motionState, settingsPersistence, runtimeSettings),

    _homingController(
        _axisA,
        _axisB,
        _driverA,
        _driverB,
        motionState,
        motionCommand,
        runtimeSettings
    ),

    _kinematics(STEPS_PER_MM),

    _bezierExecuter(_axisA, _axisB, _kinematics, motionState, motionCommand),

    _motionExecuter(_bezierExecuter, motionCommand, runtimeSettings),

    _gcodeExecuter(
        _motionExecuter,
        _pen,
        _homingController,
        runtimeSettings,
        motionState
    )
{
    _settingPersistence.registerObserver(this);
}

PlottingController::~PlottingController() {
    _settingPersistence.unregisterObserver(this);
}

void PlottingController::configureDriver(TMC2209Driver& driver)
{
    driver.setStallGuardThreshold(_runtimeSettings.stallguardThreshold());
    driver.setCurrent(_runtimeSettings.driverCurrent_mA());
    driver.setMicrosteps(_runtimeSettings.microsteps());
}

void PlottingController::init()
{
    pinMode(SERVO_PIN, OUTPUT);

    _penServo.attach(SERVO_PIN);
    _pen.init();

    pinMode(EN_PIN, OUTPUT);
    digitalWrite(EN_PIN, LOW);

    _driverSerial.begin(115200, SERIAL_8N1, DRIVER_RX_PIN, DRIVER_TX_PIN);

    _driverA.begin();
    configureDriver(_driverA);

    _driverB.begin();
    configureDriver(_driverB);

    ESP_LOGI(TAG, "Plotting controller initialized");
}

void PlottingController::update()
{
    auto msg = _gcodeQueue.tryReceive(0);

    if (msg.has_value() && _motionCommand.getCommand() != MotionCommandType::ABORT)
    {
        _motionState.setState(MotionStateType::RUNNING);

        ESP_LOGD(TAG, "Executing line: %s", msg->line);

        auto command = GCodeParser::parse(msg->line);
        if (command) _gcodeExecuter.execute(*command);
    }
    else
    {
        if (_motionCommand.getCommand() == MotionCommandType::ABORT) {
            _pen.up();
        }
        _motionState.setState(MotionStateType::IDLE);
    }

    _pen.checkIfSettingsChanged();
    SettingObserver::checkIfSettingsChanged();
}

void PlottingController::onRelevantSettingsChanged() {
    // Update driver configurations based on the changed settings
    ESP_LOGI(TAG, "Reconfiguring drivers due to relevant setting change");
    configureDriver(_driverA);
    configureDriver(_driverB);
}
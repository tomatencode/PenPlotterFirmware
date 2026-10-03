#include "HomingController.hpp"

#include <algorithm>
#include <esp_log.h>
#include "config/HardwareConfig.hpp"

static const char* TAG = "HomingController";

HomingController::HomingController(StepperAxis& axisA, StepperAxis& axisB, MotorDriver& driverA, MotorDriver& driverB, MotionState& motionState, MotionCommand& motionCommand, RuntimeSettings& runtimeSettings, CoreXYKinematics& kinematics)
    : _axisA(axisA), _axisB(axisB), _driverA(driverA), _driverB(driverB), _motionState(motionState), _motionCommand(motionCommand), _runtimeSettings(runtimeSettings), _kinematics(kinematics) {}

// Handle pause/abort during movement; return true if homing should be aborted
bool HomingController::checkPauseAbort() {
    if (_motionCommand.getCommand() == MotionCommandType::PAUSE) {
        _driverA.setSpeed(0);
        _driverB.setSpeed(0);
        _motionState.setState(MotionStateType::PAUSED);

        ESP_LOGI(TAG, "Homing paused");
        while (_motionCommand.getCommand() == MotionCommandType::PAUSE) {
            yield();
        }
        ESP_LOGI(TAG, "Homing resumed");
        _motionState.setState(MotionStateType::RUNNING);
        return false;  // Continue homing
    } else if (_motionCommand.getCommand() == MotionCommandType::ABORT) {
        ESP_LOGI(TAG, "Homing aborted");
        _driverA.setSpeed(0);
        _driverB.setSpeed(0);
        return true;  // Abort homing
    }
    return false;  // Continue normally
}

// Move stepper axes toward limit switches until stallguard detects stall
void HomingController::moveToLimit(LimitDirection direction, uint16_t backOffSteps) {
    float speed_stps_per_s = _runtimeSettings.homingSpeed_stp_per_s();
    float stallGuard_threshold = _runtimeSettings.stallguardThreshold();
    uint32_t homingTimeout_us = _runtimeSettings.homingTimeout_us();
    uint16_t sgCheckInterval_ms = _runtimeSettings.sgCheckInterval_ms();
    uint16_t sgStartTimeout_ms = _runtimeSettings.sgStartTimeout_ms();
    uint8_t sgHistorySize = _runtimeSettings.sgHistorySize();

    // Compute motor directions toward each limit using the configured coordinate system
    float xDir = (direction == LimitDirection::X_PLUS) ? 1.0f : (direction == LimitDirection::X_MINUS) ? -1.0f : 0.0f;
    float yDir = (direction == LimitDirection::Y_PLUS) ? 1.0f : (direction == LimitDirection::Y_MINUS) ? -1.0f : 0.0f;

    MotorSteps limitDir = _kinematics.mmToSteps({xDir, yDir});

    bool Afw = limitDir.a > 0;
    bool Bfw = limitDir.b > 0;
    
    if (speed_stps_per_s <= 0.0f) {
        ESP_LOGE(TAG, "Invalid homing speed");
        return;
    }
    
    if (_driverA.getMicrosteps() != _driverB.getMicrosteps()) {
        ESP_LOGE(TAG, "Drivers have different microstep settings");
        return;
    }

    constexpr uint32_t positionUpdateIntervalUs = 20000UL;
    uint32_t lastPositionUpdateUs = micros();

    uint32_t sgCheckInterval_us = sgCheckInterval_ms * 1000UL;
    uint32_t last_SGcheck_time = micros() + sgStartTimeout_ms * 1000UL;
    uint32_t start_time = micros();

    std::vector<int> sgHistory(sgHistorySize, 0);
    uint8_t sgHistoryIndex = 0;
    bool bufferFilled = false;

    double speedA = speed_stps_per_s * _driverA.getMicrosteps() * (Afw ? 1 : -1);
    double speedB = speed_stps_per_s * _driverB.getMicrosteps() * (Bfw ? 1 : -1);
    _driverA.setSpeed(speedA);
    _driverB.setSpeed(speedB);



    while (true) {
        while ((uint32_t)(micros() - last_SGcheck_time) < sgCheckInterval_us) {
            const uint32_t nowUs = micros();
            const uint32_t elapsedUs = nowUs - lastPositionUpdateUs;
            if (elapsedUs >= positionUpdateIntervalUs) {
                const double distanceMm = (speed_stps_per_s * elapsedUs) / (STEPS_PER_MM * 1000000.0);
                const double newPosX = std::clamp(
                    _motionState.getMachineX() + (xDir * distanceMm),
                    0.0,
                    static_cast<double>(MAX_X_MM));
                const double newPosY = std::clamp(
                    _motionState.getMachineY() + (yDir * distanceMm),
                    0.0,
                    static_cast<double>(MAX_Y_MM));

                _motionState.setMachineX(newPosX);
                _motionState.setMachineY(newPosY);
                lastPositionUpdateUs = nowUs;
            }
            yield();
        }
        last_SGcheck_time = micros();

        if (checkPauseAbort()) {
            return;
        }

        int stallGuardA = _driverA.getStallGuardResult();
        int stallGuardB = _driverB.getStallGuardResult();
        int stallGuardMin = std::min(stallGuardA, stallGuardB);
        
        sgHistory[sgHistoryIndex] = stallGuardMin;
        sgHistoryIndex = (sgHistoryIndex + 1) % sgHistorySize;
        
        // Mark buffer as filled after first full cycle
        if (sgHistoryIndex == 0) {
            bufferFilled = true;
        }

        if (bufferFilled) {
            int sum = 0;
            for (uint8_t i = 0; i < sgHistorySize; i++) {
                sum += sgHistory[i];
            }
            int average = sum / sgHistorySize;
            
            if (average < stallGuard_threshold) {
                ESP_LOGD(TAG, "Stall detected: avg SG = %d", average);
                break;  // Stall detected, stop moving
            }
        }

        // Safety timeout
        if ((uint32_t)(micros() - start_time) > homingTimeout_us) {
            ESP_LOGW(TAG, "Homing timeout, limit not found");
            break;
        }
    }

    _driverA.setSpeed(0);
    _driverB.setSpeed(0);

    // Interruptible delay before backing off
    uint32_t delayStart = millis();
    while ((uint32_t)(millis() - delayStart) < 100) {
        if (checkPauseAbort()) return;
        yield();
    }

    // Back off a few steps
    double microsteps_per_s = _runtimeSettings.homingBackOffSpeed_stp_per_s() * _axisA.microsteps();
    if (microsteps_per_s <= 0.0f) {
        ESP_LOGE(TAG, "Invalid homing back-off speed");
        return;
    }
    uint16_t stepInterval_us = 1000000UL / microsteps_per_s;

    const uint32_t backOffStartUs = micros();
    uint32_t lastBackOffPositionUpdateUs = backOffStartUs;
    const double backOffStartX = _motionState.getMachineX();
    const double backOffStartY = _motionState.getMachineY();
    const uint32_t totalBackOffMicrosteps = backOffSteps * _axisA.microsteps();

    for (uint32_t i = 0; i < totalBackOffMicrosteps; i++) {
        if (checkPauseAbort()) return;
        
        _axisA.step(!Afw);
        _axisB.step(!Bfw);
        delayMicroseconds(stepInterval_us);

        const uint32_t nowUs = micros();
        if (nowUs - lastBackOffPositionUpdateUs >= positionUpdateIntervalUs || i + 1 == totalBackOffMicrosteps) {
            const double distanceMm = static_cast<double>(i + 1) / (_axisA.microsteps() * STEPS_PER_MM);
            _motionState.setMachineX(std::clamp(backOffStartX - (xDir * distanceMm), 0.0, static_cast<double>(MAX_X_MM)));
            _motionState.setMachineY(std::clamp(backOffStartY - (yDir * distanceMm), 0.0, static_cast<double>(MAX_Y_MM)));
            lastBackOffPositionUpdateUs = nowUs;
        }
    }
}

void HomingController::home() {
    if (_axisA.microsteps() != _axisB.microsteps()) {
        ESP_LOGE(TAG, "Axes have different microstep settings");
        return;
    }

    // Move to X limit
    moveToLimit(LimitDirection::X_MINUS, _runtimeSettings.backOffStepsX());
    if (_motionCommand.getCommand() == MotionCommandType::ABORT) return;

    // Interruptible delay before moving to Y limit
    uint32_t delayStart = millis();
    while ((uint32_t)(millis() - delayStart) < 100) {
        if (checkPauseAbort()) return;
        yield();
    }

    // Move to Y limit
    moveToLimit(LimitDirection::Y_MINUS, _runtimeSettings.backOffStepsY());
    if (_motionCommand.getCommand() == MotionCommandType::ABORT) return;

    // Zero both axes
    _axisA.setPositionSteps(0);
    _axisB.setPositionSteps(0);

    // Update motion state machine position
    _motionState.setMachineX(0.0);
    _motionState.setMachineY(0.0);
}
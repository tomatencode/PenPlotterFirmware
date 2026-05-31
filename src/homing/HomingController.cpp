#include "HomingController.hpp"

#include <esp_log.h>

static const char* TAG = "HomingController";

HomingController::HomingController(StepperAxis& axisA, StepperAxis& axisB, MotorDriver& driverA, MotorDriver& driverB, MotionState& motionState, RuntimeSettings& runtimeSettings)
    : _axisA(axisA), _axisB(axisB), _driverA(driverA), _driverB(driverB), _motionState(motionState), _runtimeSettings(runtimeSettings) {}

// Handle pause/abort during movement; return true if homing should be aborted
bool HomingController::checkPauseAbort() {
    if (_motionState.getCommand() == MotionCommand::PAUSE) {
        _driverA.setSpeed(0);
        _driverB.setSpeed(0);
        _motionState.setState(MotionStateType::PAUSED);

        ESP_LOGI(TAG, "Homing paused");
        while (_motionState.getCommand() == MotionCommand::PAUSE) {
            yield();
        }
        ESP_LOGI(TAG, "Homing resumed");
        _motionState.setState(MotionStateType::RUNNING);
        return false;  // Continue homing
    } else if (_motionState.getCommand() == MotionCommand::ABORT) {
        ESP_LOGI(TAG, "Homing aborted");
        _driverA.setSpeed(0);
        _driverB.setSpeed(0);
        return true;  // Abort homing
    }
    return false;  // Continue normally
}

// Move stepper axes toward limit switches until stallguard detects stall
void HomingController::moveToLimit(bool Afw, bool Bfw, uint16_t backOffSteps) {
    float speed_stps_per_s = _runtimeSettings.homingSpeed_stp_per_s();
    float stallGuard_threshold = _runtimeSettings.stallguardThreshold();
    uint32_t homingTimeout_us = _runtimeSettings.homingTimeout_us();
    uint16_t sgCheckInterval_ms = _runtimeSettings.sgCheckInterval_ms();
    uint16_t sgStartTimeout_ms = _runtimeSettings.sgStartTimeout_ms();
    uint8_t sgHistorySize = _runtimeSettings.sgHistorySize();
    
    if (speed_stps_per_s <= 0.0f) {
        ESP_LOGE(TAG, "Invalid homing speed");
        return;
    }
    
    if (_driverA.getMicrosteps() != _driverB.getMicrosteps()) {
        ESP_LOGE(TAG, "Drivers have different microstep settings");
        return;
    }

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

    for (uint16_t i = 0; i < backOffSteps * _axisA.microsteps(); i++) {
        if (checkPauseAbort()) return;
        
        _axisA.step(!Afw);
        _axisB.step(!Bfw);
        delayMicroseconds(stepInterval_us);
    }
}

void HomingController::home() {
    if (_axisA.microsteps() != _axisB.microsteps()) {
        ESP_LOGE(TAG, "Axes have different microstep settings");
        return;
    }

    // Move to X limit (both axes negative)
    moveToLimit(false, false, _runtimeSettings.backOffStepsX());
    if (_motionState.getCommand() == MotionCommand::ABORT) return;

    // Interruptible delay before moving to Y limit
    uint32_t delayStart = millis();
    while ((uint32_t)(millis() - delayStart) < 100) {
        if (checkPauseAbort()) return;
        yield();
    }

    // Move to Y limit (B axis positive)
    moveToLimit(false, true, _runtimeSettings.backOffStepsY());
    if (_motionState.getCommand() == MotionCommand::ABORT) return;

    // Zero both axes
    _axisA.setPositionSteps(0);
    _axisB.setPositionSteps(0);

    // Update motion state machine position
    _motionState.setMachineX(0.0);
    _motionState.setMachineY(0.0);
}
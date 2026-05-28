#include "Pen.hpp"
#include "settings/RuntimeSettings.hpp"

Pen::Pen(Servo& servo, MotionState& motionState, SettingPersistence& settingPersistence, RuntimeSettings& runtimeSettings)
    : SettingObserver({Setting::PenUpAngle, Setting::PenDownAngle}),
    _servo(servo),
    _motionState(motionState),
    _settingPersistence(settingPersistence),
    _runtimeSettings(runtimeSettings)
{
    _settingPersistence.registerObserver(this);
};

/*
    IS this class really RAII? init() is a code smell that it's not...
*/
void Pen::init() {
    up();
}

Pen::~Pen() {
    _settingPersistence.unregisterObserver(this);
}

void Pen::down() {
    _servo.write((int)_runtimeSettings.penDownAngle_deg());
    _motionState.setPenIsDown(true);
}

void Pen::up() {
    _servo.write((int)_runtimeSettings.penUpAngle_deg());
    _motionState.setPenIsDown(false);
}

void Pen::onRelevantSettingsChanged() {
    // If pen is currently down, we need to update the servo position to reflect new angles
    if (_motionState.penIsDown()) {
        down();
    } else {
        up();
    }
}
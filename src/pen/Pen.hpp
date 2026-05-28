#pragma once
#include <ESP32Servo.h>
#include "Pen.hpp"
#include "rtos/MotionState.hpp"
#include "settings/RuntimeSettings.hpp"
#include "settings/SettingPersistence.hpp"
#include "settings/SettingObserver.hpp"

#include <cstdint>

/*
it: you may want to put all classes in a namespace, as class names such as Pen seem to be rather generic and could easily lead to name clashes in larger projects or when integrating third-party libraries. A namespace would help to encapsulate the class and avoid potential conflicts.
*/

class Pen : public SettingObserver {
public:
    /*
        it: should Pen become owner of 'servo' and manage its lifecycle? Otherwise we need to ensure that the servo outlives the Pen instance, which can be error-prone.
        Furthermore, thread-safety could be compromised. Same is true for the references to MotionState, SettingPersistence, and RuntimeSettings.
    */
    Pen(Servo& servo, MotionState& motionState, SettingPersistence& settingPersistence, RuntimeSettings& runtimeSettings);
    ~Pen();

    void init();

    void down();
    void up();

    void onRelevantSettingsChanged() override;

private:
    Servo& _servo;
    MotionState& _motionState;
    RuntimeSettings& _runtimeSettings;
    SettingPersistence& _settingPersistence;
};
#pragma once
#include <cstdint>

enum class Setting {
    Name, // not threadsafe
    PenSlots, // not threadsafe
    SSID, // not threadsafe
    Password, // not threadsafe
    MdnsName, // not threadsafe
    DriverCurrent,
    Microsteps,
    DrawFeedRate,
    TravelFeedRate,
    PenLiftTime,
    MinFeatureSize,
    HomingSpeed,
    StallguardThreshold,
    HomingBackOffSpeed,
    BackOffStepsX,
    BackOffStepsY,
    HomingTimeout,
    SGCheckInterval,
    SGStartTimeout,
    SGHistorySize,
    PenUpAngle,
    PenDownAngle,
};

struct PenSlot {
    bool empty; // whether this slot is empty
    float stroke;
    uint8_t color[4]; // RGBA
};
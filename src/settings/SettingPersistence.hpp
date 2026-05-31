#pragma once
#include "SettingObserver.hpp"
#include "RuntimeSettings.hpp"
#include "Settings.hpp"
#include "SettingLimits.hpp"
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <memory>

class SettingPersistence {
public:
    explicit SettingPersistence(RuntimeSettings& runtimeSettings)
        : _runtimeSettings(runtimeSettings)
    {}

    // Initialize: load all settings from NVS at startup
    void init();

    // setters: persist → RuntimeSettings → notify
    void setName(const std::string& value);
    void setPenSlots(const std::array<PenSlot, NUM_PEN_SLOTS>& penSlots);
    void setSSID(const std::string& value);
    void setPassword(const std::string& value);
    void setMdnsName(const std::string& value);
    void setDriverCurrent_mA(float value);
    void setMicrosteps(float value);
    void setDrawFeedRate_mm_per_s(float value);
    void setTravelFeedRate_mm_per_s(float value);
    void setPenLiftTime_s(float value);
    void setMinFeatureSize_mm(float value);
    void setHomingSpeed_stp_per_s(float value);
    void setStallguardThreshold(float value);
    void setHomingBackOffSpeed_stp_per_s(float value);
    void setBackOffStepsX(uint16_t value);
    void setBackOffStepsY(uint16_t value);
    void setHomingTimeout_us(uint32_t value);
    void setSGCheckInterval_ms(uint16_t value);
    void setSGStartTimeout_ms(uint16_t value);
    void setSGHistorySize(uint8_t value);
    void setPenUpAngle_deg(float value);
    void setPenDownAngle_deg(float value);

    void clearAllSettings();  // Clear all NVS data and reload defaults

    // Observer Management
    void registerObserver(SettingObserver* observer);
    void unregisterObserver(SettingObserver* observer);

private:

    // Type-dispatched NVS write helpers — each opens its own prefs scope
    void persist(const char* key, float v);
    void persist(const char* key, uint8_t v);
    void persist(const char* key, uint16_t v);
    void persist(const char* key, uint32_t v);
    void persist(const char* key, const std::string& v);

    void loadSettings();
    void notifyObservers(Setting setting) const;

    std::vector<SettingObserver*> _observers;
    RuntimeSettings& _runtimeSettings;
};
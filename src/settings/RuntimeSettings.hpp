#pragma once
#include <atomic>
#include <array>
#include <string>

#include "Settings.hpp"
#include "config/PenSlotsConfig.hpp"
#include "defaults/PlottingDefaults.hpp"

class RuntimeSettings {
public:

    std::string name() const { return _name; }
    std::array<PenSlot, NUM_PEN_SLOTS> penSlots() const { return _penSlots; }
    std::string ssid() const { return _ssid; }
    std::string password() const { return _password; }
    std::string mdnsName() const { return _mdnsName; }
    float driverCurrent_mA() const { return _driverCurrent_mA.load(std::memory_order_relaxed); }
    float microsteps() const { return _microsteps.load(std::memory_order_relaxed); }
    float drawFeedRate_mm_per_s() const { return _drawFeedRate_mm_per_s.load(std::memory_order_relaxed); }
    float travelFeedRate_mm_per_s() const { return _travelFeedRate_mm_per_s.load(std::memory_order_relaxed); }
    float penLiftTime_s() const { return _penLiftTime_s.load(std::memory_order_relaxed); }
    float minFeatureSize_mm() const { return _minFeatureSize_mm.load(std::memory_order_relaxed); }
    float homingSpeed_stp_per_s() const { return _homingSpeed_stp_per_s.load(std::memory_order_relaxed); }
    float homingBackOffSpeed_stp_per_s() const { return _homingBackOffSpeed_stp_per_s.load(std::memory_order_relaxed); }
    float stallguardThreshold() const { return _stallguardThreshold.load(std::memory_order_relaxed); }
    uint16_t backOffStepsX() const { return _backOffStepsX.load(std::memory_order_relaxed); }
    uint16_t backOffStepsY() const { return _backOffStepsY.load(std::memory_order_relaxed); }
    uint32_t homingTimeout_us() const { return _homingTimeout_us.load(std::memory_order_relaxed); }
    uint16_t sgCheckInterval_ms() const { return _sgCheckInterval_ms.load(std::memory_order_relaxed); }
    uint16_t sgStartTimeout_ms() const { return _sgStartTimeout_ms.load(std::memory_order_relaxed); }
    uint8_t sgHistorySize() const { return _sgHistorySize.load(std::memory_order_relaxed); }
    float penUpAngle_deg() const { return _penUpAngle_deg.load(std::memory_order_relaxed); }
    float penDownAngle_deg() const { return _penDownAngle_deg.load(std::memory_order_relaxed); }

private:

    friend class SettingPersistence;
    
    // setters called by SettingPersistence when loading from NVS or updating a setting
    void setName(const std::string& v) { _name = v; }
    void setPenSlots(const std::array<PenSlot, NUM_PEN_SLOTS>& penSlots) { _penSlots = penSlots; }
    void setSSID(const std::string& v) { _ssid = v; }
    void setPassword(const std::string& v) { _password = v; }
    void setMdnsName(const std::string& v) { _mdnsName = v; }
    void setDriverCurrent_mA(float v) { _driverCurrent_mA.store(v, std::memory_order_relaxed); }
    void setMicrosteps(float v) { _microsteps.store(v, std::memory_order_relaxed); }
    void setDrawFeedRate_mm_per_s(float v) { _drawFeedRate_mm_per_s.store(v, std::memory_order_relaxed); }
    void setTravelFeedRate_mm_per_s(float v) { _travelFeedRate_mm_per_s.store(v, std::memory_order_relaxed); }
    void setPenLiftTime_s(float v) { _penLiftTime_s.store(v, std::memory_order_relaxed); }
    void setMinFeatureSize_mm(float v) { _minFeatureSize_mm.store(v, std::memory_order_relaxed); }
    void setHomingSpeed_stp_per_s(float v) { _homingSpeed_stp_per_s.store(v, std::memory_order_relaxed); }
    void setStallguardThreshold(float v) { _stallguardThreshold.store(v, std::memory_order_relaxed); }
    void setHomingBackOffSpeed_stp_per_s(float v) { _homingBackOffSpeed_stp_per_s.store(v, std::memory_order_relaxed); }
    void setBackOffStepsX(uint16_t v) { _backOffStepsX.store(v, std::memory_order_relaxed); }
    void setBackOffStepsY(uint16_t v) { _backOffStepsY.store(v, std::memory_order_relaxed); }
    void setHomingTimeout_us(uint32_t v) { _homingTimeout_us.store(v, std::memory_order_relaxed); }
    void setSGCheckInterval_ms(uint16_t v) { _sgCheckInterval_ms.store(v, std::memory_order_relaxed); }
    void setSGStartTimeout_ms(uint16_t v) { _sgStartTimeout_ms.store(v, std::memory_order_relaxed); }
    void setSGHistorySize(uint8_t v) { _sgHistorySize.store(v, std::memory_order_relaxed); }
    void setPenUpAngle_deg(float v) { _penUpAngle_deg.store(v, std::memory_order_relaxed); }
    void setPenDownAngle_deg(float v) { _penDownAngle_deg.store(v, std::memory_order_relaxed); }

private:

    std::atomic<float> _driverCurrent_mA{DRIVER_CURRENT_MA};
    std::atomic<float> _microsteps{MICROSTEPS};
    std::atomic<float> _drawFeedRate_mm_per_s{FEED_RATE_DRAW_MM_PER_S};
    std::atomic<float> _travelFeedRate_mm_per_s{FEED_RATE_TRAVEL_MM_PER_S};
    std::atomic<float> _penLiftTime_s{PEN_LIFT_TIME_S}; // default pen lift time in seconds
    std::atomic<float> _minFeatureSize_mm{MIN_FEATURE_SIZE_MM};
    std::atomic<float> _homingSpeed_stp_per_s{HOMING_SPEED_STP_PER_S};
    std::atomic<float> _stallguardThreshold{STALLGUARD_THRESHOLD};
    std::atomic<float> _homingBackOffSpeed_stp_per_s{HOMING_BACK_OFF_SPEED_STP_PER_S};
    std::atomic<uint16_t> _backOffStepsX{BACK_OFF_STEPS_X};
    std::atomic<uint16_t> _backOffStepsY{BACK_OFF_STEPS_Y};
    std::atomic<uint32_t> _homingTimeout_us{HOMING_TIMEOUT_US};
    std::atomic<uint16_t> _sgCheckInterval_ms{SG_CHECK_INTERVAL_MS};
    std::atomic<uint16_t> _sgStartTimeout_ms{SG_START_TIMEOUT_MS};
    std::atomic<uint8_t> _sgHistorySize{SG_HISTORY_SIZE};
    std::atomic<float> _penUpAngle_deg{PEN_UP_DEG};
    std::atomic<float> _penDownAngle_deg{PEN_DOWN_DEG};
    
    // cant make these atomics since std::string is not trivially copyable
    // but they are only accessed on Core 0 (main thread) so should be safe without synchronization
    std::string _name;
    std::array<PenSlot, NUM_PEN_SLOTS> _penSlots{};
    std::string _ssid;
    std::string _password;
    std::string _mdnsName;
};

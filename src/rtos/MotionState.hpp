#pragma once

#include <atomic>
#include <optional>

enum class MotionStateType
{
    IDLE = 0,
    RUNNING = 1,
    PAUSED = 2
};

class MotionState {
public:
    void setState(MotionStateType newState) { _state.store(newState); }
    MotionStateType getState() const { return _state.load(); }

    void setMachineX(float x) { _machineX.store(x); }
    void setMachineY(float y) { _machineY.store(y); }
    void setPenIsDown(bool isDown) { _penIsDown.store(isDown); }
    void setActivePenSlot(std::optional<size_t> slot) { _activePenSlot.store(slot.has_value() ? slot.value() : -1); }

    float getMachineX() const { return _machineX.load(); }
    float getMachineY() const { return _machineY.load(); }
    bool penIsDown() const { return _penIsDown.load(); }
    std::optional<size_t> getActivePenSlot() const { 
        int slot = _activePenSlot.load();
        return slot == -1 ? std::nullopt : std::optional<size_t>(slot);
    }

private:
    std::atomic<MotionStateType> _state = MotionStateType::IDLE;

    std::atomic<float> _machineX = 0.0f;
    std::atomic<float> _machineY = 0.0f;
    std::atomic<bool> _penIsDown = false;

    std::atomic<int> _activePenSlot = -1; // -1 means no pen, otherwise index into pen slots
};
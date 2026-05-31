#pragma once

#include <atomic>
#include <optional>

enum class MotionCommandType
{
    NONE = 0,
    PAUSE = 1,
    ABORT = 2
};

class MotionCommand {
public:
    void setCommand(MotionCommandType cmd) { _command.store(cmd); }
    MotionCommandType getCommand() const { return _command.load(); }
private:
    std::atomic<MotionCommandType> _command = MotionCommandType::NONE;
};
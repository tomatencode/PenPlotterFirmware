#pragma once
#include <Arduino.h>
#include <FS.h>
#include <string>
#include <vector>

#include "gcode/GCodeSender.hpp"
#include "hardware/buzzer/Buzzer.hpp"
#include "storage/FileManager.hpp"
#include "rtos/MotionState.hpp"
#include "rtos/MotionCommand.hpp"
#include "settings/RuntimeSettings.hpp"
#include "JobObserver.hpp"

struct PlotJob {
    File file = File();
    std::string filename = "";

    uint32_t totalLines = 0;
    uint32_t currentBufferLine = 0;

    uint32_t totalTimeSeconds = 0;
    uint32_t jobStartTimeMS = 0;
    uint32_t totalPausedMS = 0;
    uint32_t pauseStartTimeMS = 0;
};

class JobController {
public:
    JobController(GCodeSender& gcodeSender, MotionState& motionState, MotionCommand& motionCommand, RuntimeSettings& runtimeSettings, FileManager& fileManager, Buzzer& buzzer)
        : _currentJob(PlotJob()),
          _active(false),
          _gcodeSender(gcodeSender),
          _motionState(motionState),
          _motionCommand(motionCommand),
          _runtimeSettings(runtimeSettings),
          _fileManager(fileManager),
          _buzzer(buzzer)
    {}

    // Job control
    void start(const std::string& filename);
    void pause();
    void resume();
    void abort();

    // Observer management
    void registerObserver(JobObserver* observer);
    void unregisterObserver(JobObserver* observer);

    // Status queries
    bool isActive() const { return _active; }
    std::string getCurrentFile() const { return _currentJob.filename; }
    uint32_t getTotalLines() const { return _currentJob.totalLines; }
    uint32_t getCurrentLine() const;
    uint32_t getTotalTimeSeconds() const;
    uint32_t getTimeRemainingSeconds() const;
    double getProgress() const;

    void update();

private:
    PlotJob _currentJob;
    bool _active;

    void endCurrentJob();

    GCodeSender& _gcodeSender;
    std::optional<GCodeSender::Token> _gcodeToken;
    MotionState& _motionState;
    MotionCommand& _motionCommand;
    RuntimeSettings& _runtimeSettings;
    FileManager& _fileManager;
    Buzzer& _buzzer;

    std::vector<JobObserver*> _observers;

    // Internal notification method
    void notifyObservers(const JobEventType& event);

    // Melodies for different job events
    std::vector<Buzzer::Note> _jobStartMelody = {
        {523, 150}, // C5
        {659, 150}, // E5
        {784, 150}  // G5
    };

    std::vector<Buzzer::Note> _jobCompleteMelody = {
        {784, 150}, // G5
        {659, 150}, // E5
        {523, 150}  // C5
    };

    std::vector<Buzzer::Note> _jobAbortMelody = {
        {523, 200}, // C5
        {493, 200}, // B4
    };

    std::vector<Buzzer::Note> _jobPauseMelody = {
        {440, 100}, // A4
    };

    std::vector<Buzzer::Note> _jobResumeMelody = {
        {523, 100}, // C5
    };

};
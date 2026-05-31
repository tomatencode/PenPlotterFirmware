#include "JobController.hpp"
#include "config/DirectoriesConfig.hpp"

#include "JobStatsCalculator.hpp"

#include <FS.h>
#include <cstring>
#include <esp_log.h>

static const char* TAG = "JobController";

void JobController::start(const std::string& filename)
{
    abort(); // Ensure any existing job is stopped before starting a new one

    _gcodeToken = _gcodeSender.tryAcquire();
    if (!_gcodeToken) {
        ESP_LOGW(TAG, "Cannot start job - GCode sender is busy");
        return;
    }

    ESP_LOGI(TAG, "Starting job: %s", filename.c_str());

    auto stats = calculateStats(_fileManager, _runtimeSettings, PLOTTING_DIRECTORY + filename);

    _motionCommand.setCommand(MotionCommandType::NONE); // Clear any existing motion commands
    _currentJob.file = _fileManager.openFileRead(PLOTTING_DIRECTORY + filename);

    if (!_currentJob.file)
    {
        ESP_LOGE(TAG, "Failed to open file: %s", filename.c_str());
        return;
    }

    _currentJob.filename = filename;
    _currentJob.totalLines = stats.totalLines;
    _currentJob.currentBufferLine = 0;
    _currentJob.totalTimeSeconds = stats.totalTimeSeconds;
    _currentJob.jobStartTimeMS = millis();
    _active = true;

    notifyObservers({.type = JobEvent::STARTED, .filename = _currentJob.filename});

    _buzzer.playMelody(_jobStartMelody);
}

void JobController::pause()
{
    if (!_active) return;

    _buzzer.playMelody(_jobPauseMelody);
    
    ESP_LOGI(TAG, "Pausing job");
    _currentJob.pauseStartTimeMS = millis();
    _motionCommand.setCommand(MotionCommandType::PAUSE);
    notifyObservers({.type = JobEvent::PAUSED, .filename = _currentJob.filename});
}

void JobController::resume()
{
    if (!_active) return;

    _buzzer.playMelody(_jobResumeMelody);

    ESP_LOGI(TAG, "Resuming job");
    if (_currentJob.pauseStartTimeMS != 0) {
        _currentJob.totalPausedMS += millis() - _currentJob.pauseStartTimeMS;
        _currentJob.pauseStartTimeMS = 0;
    }
    _motionCommand.setCommand(MotionCommandType::NONE);
    notifyObservers({.type = JobEvent::RESUMED, .filename = _currentJob.filename});
}

void JobController::abort()
{
    if (!_active) return;

    _buzzer.playMelody(_jobAbortMelody);

    ESP_LOGI(TAG, "Aborting job");
    _motionCommand.setCommand(MotionCommandType::ABORT);
    _gcodeToken->clearQueue();

    std::string filename = _currentJob.filename; // Capture filename before ending job
    endCurrentJob();

    notifyObservers({.type = JobEvent::ABORTED, .filename = filename});
}

void JobController::update()
{
    if (_motionState.getState() == MotionStateType::IDLE
        && _motionCommand.getCommand() == MotionCommandType::ABORT) {
        // Clear abort command once we've come to a stop
        _motionCommand.setCommand(MotionCommandType::NONE);
    }

    if (!_active) return;

    // Check for completion
    if (!_currentJob.file.available() && _gcodeToken->messagesWaiting() == 0 && _motionState.getState() == MotionStateType::IDLE) {
        std::string filename = _currentJob.filename; // Capture filename before ending job
        endCurrentJob();
        _buzzer.playMelody(_jobCompleteMelody);
        notifyObservers({.type = JobEvent::COMPLETED, .filename = filename});
        return;
    }
    
    if (!_currentJob.file.available()) return; // No more lines to read

    UBaseType_t space = _gcodeToken->spacesAvailable();
    for (UBaseType_t i = 0; i < space && _currentJob.file.available(); i++)
    {
        String line = _currentJob.file.readStringUntil('\n');
        std::string lineStr(line.c_str());
        // trim whitespace and carriage returns
        lineStr.erase(0, lineStr.find_first_not_of(" \t\r\n"));
        if (!lineStr.empty()) lineStr.erase(lineStr.find_last_not_of(" \t\r\n") + 1);

        _currentJob.currentBufferLine++;

        if (lineStr.length() == 0) continue; // skip empty lines

        if (!_gcodeToken->send(lineStr)) break; // queue full
    }
}

void JobController::endCurrentJob()
{
    _gcodeToken = std::nullopt; // RAII destructor releases the lock

    _active = false;
    if (_currentJob.file) {
        _currentJob.file.close();
    }
    _currentJob = PlotJob();
    ESP_LOGI(TAG, "Job ended");
}


uint32_t JobController::getCurrentLine() const
{
    if (!_active || !_gcodeToken) return 0;
    size_t linesInQueue = _gcodeToken->messagesWaiting();
    if (linesInQueue > _currentJob.currentBufferLine) return 0;
    return _currentJob.currentBufferLine - linesInQueue;
}

uint32_t JobController::getTotalTimeSeconds() const
{
    if (!_active) return 0;
    return _currentJob.totalTimeSeconds;
}

uint32_t JobController::getTimeRemainingSeconds() const
{
    if (!_active) return 0;
    uint32_t pausedMS = _currentJob.totalPausedMS;
    if (_currentJob.pauseStartTimeMS != 0) pausedMS += millis() - _currentJob.pauseStartTimeMS; // currently paused
    uint32_t elapsedSeconds = (millis() - _currentJob.jobStartTimeMS - pausedMS) / 1000;
    if (elapsedSeconds >= _currentJob.totalTimeSeconds) return 0;
    return _currentJob.totalTimeSeconds - elapsedSeconds;
}

double JobController::getProgress() const
{
    if (!_active || _currentJob.totalLines == 0) return 0.0;
    return static_cast<double>(getCurrentLine()) / _currentJob.totalLines;
}


void JobController::registerObserver(JobObserver* observer)
{
    if (observer == nullptr) return;
    
    // Avoid duplicate registrations
    for (auto obs : _observers) {
        if (obs == observer) return;
    }
    
    _observers.push_back(observer);
}

void JobController::unregisterObserver(JobObserver* observer)
{
    if (observer == nullptr) return;
    
    for (size_t i = 0; i < _observers.size(); i++) {
        if (_observers[i] == observer) {
            _observers.erase(_observers.begin() + i);
            return;
        }
    }
}

void JobController::notifyObservers(const JobEventType& event)
{
    for (auto observer : _observers) {
        if (observer != nullptr) {
            observer->onJobEvent(event);
        }
    }
}
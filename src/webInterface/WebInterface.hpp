#pragma once

#include <WebServer.h>
#include <WebSocketsServer.h>

#include "rtos/MotionState.hpp"
#include "storage/FileManager.hpp"
#include "gcode/GCodeSender.hpp"
#include "jobController/JobController.hpp"
#include "settings/SettingPersistence.hpp"
#include "settings/RuntimeSettings.hpp"
#include "settings/SettingObserver.hpp"
#include "WifiController.hpp"

class WebInterface : public SettingObserver
{
public:
    WebInterface(JobController& jobController,
                 GCodeSender& gcodeSender,
                 MotionState& motionState,
                 FileManager& fileManager,
                 WifiController& wifiController,
                 SettingPersistence& settingsPersistence,
                 RuntimeSettings& runtimeSettings)

        : SettingObserver({Setting::MdnsName}),
          _jobController(jobController),
          _gcodeSender(gcodeSender),
          _motionState(motionState),
          _fileManager(fileManager),
          _wifiController(wifiController),
          _settingPersistence(settingsPersistence),
          _runtimeSettings(runtimeSettings),
          _httpServer(80),
          _wsServer(81)
    {}
    
    ~WebInterface();

    void init();
    void update();

private:
    JobController& _jobController;
    GCodeSender& _gcodeSender;
    MotionState& _motionState;
    FileManager& _fileManager;
    SettingPersistence& _settingPersistence;
    RuntimeSettings& _runtimeSettings;
    WifiController& _wifiController;

    WebServer _httpServer;
    WebSocketsServer _wsServer;
    bool _serverStarted = false;
    unsigned long _lastStateMs = 0;

    // Upload state tracking
    File _currentUploadFile;
    std::string _currentUploadPath;
    std::string _currentTempPath;
    size_t _uploadedBytes = 0;
    static constexpr size_t MAX_UPLOAD_SIZE = 10 * 1024 * 1024; // 10 MB limit

    void setupServer();

    // WebSocket
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
    void broadcastState();
    void sendState(uint8_t num);

    std::string buildStateJson();
    std::string buildAllSettingsJson();
    std::string penSlotsToJson() const;
    bool penSlotsFromJson(const std::string& json, std::array<PenSlot, NUM_PEN_SLOTS>& out) const;

    // Upload helpers
    bool validateFileName(const std::string& filename);
    void resetUploadState();

    // HTTP handlers
    void handleGetItteration();
    void handleGetFirmwareVersion();
    void handleGetWorkspace();

    void handleGetPosition();
    void handleGetMotionState();
    void handleGetActivePenSlot();

    void handleGetName();
    void changeName();
    void handleGetMDNSName();
    void changeMDNSName();

    void handleUploadJob();
    void handleDeleteJob();
    void handleListJobs();
    void handleGetFileInfo();
    void handleDownloadJob();
    void handleStartJob();
    void handleAbortJob();
    void handlePauseJob();
    void handleResumeJob();

    void handleGetJobStatus();

    void handleExecuteLine();

    void handleGetSetting();
    void handleSetSetting();
    void handleGetAllSettings();

    void onRelevantSettingsChanged() override;
};
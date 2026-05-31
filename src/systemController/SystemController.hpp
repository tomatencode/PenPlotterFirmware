#pragma once

#include <LCD-I2C.h>
#include "gcode/GCodeSender.hpp"
#include "jobController/JobController.hpp"
#include "webInterface/WebInterface.hpp"
#include "webInterface/WifiController.hpp"
#include "storage/FileManager.hpp"
#include "ui/UiOrchestrator.hpp"
#include "ui/InputMapper.hpp"
#include "ui/framework/router/Router.hpp"
#include "ui/framework/renderer/Renderer.hpp"
#include "ui/screens/ScreensContext.hpp"
#include "rtos/MotionState.hpp"
#include "rtos/MotionCommand.hpp"
#include "rtos/RtosQueue.hpp"
#include "rtos/GcodeMessage.hpp"
#include "settings/SettingPersistence.hpp"
#include "settings/RuntimeSettings.hpp"

class SystemController
{
public:
    SystemController(MotionState& motionState, MotionCommand& motionCommand, RtosQueue<GcodeMessage>& gcodeQueue,
                       SettingPersistence& settingPersistence, RuntimeSettings& runtimeSettings);
    
    void init();
    void update();

private:

    LCD_I2C _lcd;
    LcdDisplay _display;
    RotaryEncoder _encoder;
    Buzzer _buzzer;

    GCodeSender _gcodeSender;

    JobController _jobController;
    FileManager _fileManager;
    WifiController _wifiController;
    WebInterface _webInterface;

    ui::Router _router;
    ui::Renderer _renderer;
    ui::InputMapper _inputMapper;
    ui::UiOrchestrator _uiOrchestrator;
};
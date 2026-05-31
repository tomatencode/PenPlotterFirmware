#include "SystemController.hpp"

#include "config/PinsConfig.hpp"
#include "config/UiConfig.hpp"

const Buzzer::Melody startupMelody = {
    {262, 200},
    {294, 200},
    {330, 200}
};

SystemController::SystemController(MotionState& motionState, MotionCommand& motionCommand, RtosQueue<GcodeMessage>& gcodeQueue,
                                       SettingPersistence& settingPersistence, RuntimeSettings& runtimeSettings)
    : _lcd(LCD_I2C_ADDRESS, LCD_COLS, LCD_ROWS),
      _display(_lcd),
      _encoder(ENCODER_DT_PIN, ENCODER_CLK_PIN, ENCODER_SW_PIN, ENCODER_DEBOUNCE_MS),
      _buzzer(BUZZER_PIN, 5),
      _gcodeSender(gcodeQueue),
      _fileManager(),
      _jobController(_gcodeSender, motionState, motionCommand, runtimeSettings, _fileManager, _buzzer),
      _wifiController(settingPersistence, runtimeSettings),
      _webInterface(_jobController, _gcodeSender, motionState, _fileManager, _wifiController, settingPersistence, runtimeSettings),
      _router(),
      _renderer(_display),
      _inputMapper(_encoder),
      _uiOrchestrator(_jobController, _router, _renderer, _inputMapper, _buzzer,
                        ui::ScreensContext{
                            _jobController,
                            _gcodeSender,
                            motionState,
                            _fileManager,
                            settingPersistence,
                            runtimeSettings,
                            [this]() -> bool { return _wifiController.isConnected(); }
                        }
                    )
{}

void SystemController::init()
{
    Wire.begin();

    _lcd.begin(&Wire);
    _lcd.display();
    _lcd.backlight();

    _buzzer.begin();
    _encoder.begin();

    _fileManager.init();

    _uiOrchestrator.init();

    _wifiController.init();
    _webInterface.init();

    _buzzer.playMelody(startupMelody);
}

void SystemController::update()
{
    _wifiController.update();
    _webInterface.update();
    _jobController.update();
    _buzzer.update();
    _uiOrchestrator.update();
}
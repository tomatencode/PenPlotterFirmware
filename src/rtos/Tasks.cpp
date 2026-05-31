#include "Tasks.hpp"

#include "RtosQueue.hpp"
#include "GcodeMessage.hpp"
#include "MotionState.hpp"
#include "MotionCommand.hpp"
#include "systemController/SystemController.hpp"
#include "plottingController/PlottingController.hpp"
#include "settings/RuntimeSettings.hpp"
#include "settings/SettingPersistence.hpp"
#include <esp_log.h>

static const char* TAG = "Tasks";

TaskHandle_t plottingTaskHandle = nullptr;
TaskHandle_t systemTaskHandle = nullptr;

/*
    CORE 1
    Plotting task
*/
void plottingTask(void *parameter)
{
    ESP_LOGI(TAG, "Plotting task running on core %d", xPortGetCoreID());

    PlottingController* plottingController =
        static_cast<PlottingController*>(parameter);

    plottingController->init();

    while (true)
    {
        plottingController->update();
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

/*
    CORE 0
    System task
*/
void systemTask(void *parameter)
{
    ESP_LOGI(TAG, "System task running on core %d", xPortGetCoreID());

    SystemController* app =
        static_cast<SystemController*>(parameter);

    app->init();

    while (true)
    {
        app->update();

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

/*
    Create all tasks
*/
void startRtosTasks()
{
    size_t gcodeQueueSize = 32;
    static RtosQueue<GcodeMessage> gcodeQueue(gcodeQueueSize);
    static MotionState motionState;
    static MotionCommand motionCommand;
    static RuntimeSettings runtimeSettings;
    static SettingPersistence settingPersistence(runtimeSettings);
    
    // Load settings from NVS at startup
    settingPersistence.init();

    static SystemController appManager(motionState, motionCommand, gcodeQueue, settingPersistence, runtimeSettings);
    static PlottingController plottingController(motionState, motionCommand, gcodeQueue, settingPersistence, runtimeSettings);

    // Plotting task (CORE 1)
    xTaskCreatePinnedToCore(
        plottingTask,
        "PlottingTask",
        10000,
        &plottingController,
        2,
        &plottingTaskHandle,
        1);

    // System task (CORE 0)
    xTaskCreatePinnedToCore(
        systemTask,
        "SystemTask",
        6000,
        &appManager,
        1,
        &systemTaskHandle,
        0);
}
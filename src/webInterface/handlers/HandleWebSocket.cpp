#include "../WebInterface.hpp"

#include "config/DirectoriesConfig.hpp"
#include "rtos/GcodeMessage.hpp"

#include <WebSocketsServer.h>
#include <esp_log.h>
#include <string>
#include <functional>
#include <cstdlib>
#include <cstdio>

static const char* WS_TAG = "WebSocket";

std::string WebInterface::buildStateJson() {
    const char* motionStateStr;
    switch (_motionState.getState()) {
        case MotionStateType::RUNNING: motionStateStr = "running"; break;
        case MotionStateType::PAUSED:  motionStateStr = "paused";  break;
        default:                       motionStateStr = "idle";    break;
    }

    std::string activePenSlotStr;
    auto slotOpt = _motionState.getActivePenSlot();
    if (slotOpt.has_value()) {
        activePenSlotStr = std::to_string(slotOpt.value());
    } else {
        activePenSlotStr = "null";
    }

    char buf[512];
    snprintf(buf, sizeof(buf),
        "{"
        "\"type\":\"state\"," 
        "\"x\":%.3f,"
        "\"y\":%.3f,"
        "\"penDown\":%s,"
        "\"activePenSlot\":%s,"
        "\"motionState\":\"%s\"," 
        "\"jobActive\":%s,"
        "\"jobFile\":\"%s\"," 
        "\"jobProgress\":%.4f,"
        "\"jobLine\":%u,"
        "\"jobTotalLines\":%u,"
        "\"jobRemainingSeconds\":%u"
        "}",
        _motionState.getMachineX(),
        _motionState.getMachineY(),
        _motionState.penIsDown() ? "true" : "false",
        activePenSlotStr.c_str(),
        motionStateStr,
        _jobController.isActive() ? "true" : "false",
        _jobController.getCurrentFile().c_str(),
        (float)_jobController.getProgress(),
        (unsigned int)_jobController.getCurrentLine(),
        (unsigned int)_jobController.getTotalLines(),
        (unsigned int)_jobController.getTimeRemainingSeconds()
    );
    return buf;
}

void WebInterface::sendState(uint8_t num) {
    std::string json = buildStateJson();
    _wsServer.sendTXT(num, json.c_str());
}

void WebInterface::broadcastState() {
    std::string json = buildStateJson();
    _wsServer.broadcastTXT(json.c_str());
}


void WebInterface::handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            ESP_LOGI(WS_TAG, "Client #%u connected", num);
            sendState(num); // send current state immediately on connect
            break;

        case WStype_DISCONNECTED:
            ESP_LOGI(WS_TAG, "Client #%u disconnected", num);
            break;

        case WStype_TEXT:
            // Ignore commands, only allow state push
            ESP_LOGI(WS_TAG, "Client #%u sent text (ignored)", num);
            break;

        default:
            break;
    }
}

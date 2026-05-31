#include "GCodeExecuter.hpp"

#include <esp_log.h>

static const char* TAG = "GCodeExecuter";

GCodeExecuter::GCodeExecuter(MotionExecuter& motion, Pen& pen, HomingController& homingController, 
                             RuntimeSettings& runtimeSettings, MotionState& motionState)
        : _motion(motion), _pen(pen), _homingController(homingController), _runtimeSettings(runtimeSettings),
          _absolute(true), _motionState(motionState) {}

void GCodeExecuter::execute(const GCodeCommand& command) {
    const std::string& cmd = command.cmd;
    const std::map<char,double>& params = command.params;

    if (cmd == "G0" || cmd == "G1") handleG0G1(params);
    else if (cmd == "G2") handleG2G3(params, true);
    else if (cmd == "G3") handleG2G3(params, false);
    else if (cmd == "G5") handleQUAD(params);
    else if (cmd == "G5.1") handleCUBIC(params);
    else if (cmd == "M3" || cmd == "M5") handlePenUpDown(cmd);
    else if (cmd == "G90" || cmd == "G91") handleG90G91(cmd);
    else if (cmd == "G28") handleHoming(cmd);
    else {
        ESP_LOGW(TAG, "Unknown command: %s", cmd.c_str());
    }
}

void GCodeExecuter::handleG0G1(const std::map<char,double>& params) {
    XYPos target = _motion.getCurrentPos();
    double feed = _motionState.penIsDown() ? _runtimeSettings.drawFeedRate_mm_per_s() : _runtimeSettings.travelFeedRate_mm_per_s();

    if (params.count('X')) target.xMm = _absolute ? params.at('X') : target.xMm + params.at('X');
    if (params.count('Y')) target.yMm = _absolute ? params.at('Y') : target.yMm + params.at('Y');
    if (params.count('F')) feed = params.at('F');

    _motion.LineToXY(target, feed);
}

void GCodeExecuter::handleG2G3(const std::map<char,double>& params, bool clockwise) {
    XYPos current = _motion.getCurrentPos();
    if (!params.count('X') || !params.count('Y') || !params.count('I') || !params.count('J')) {
        ESP_LOGW(TAG, "G2/G3 command missing required parameter");
        return;
    }

    XYPos center = {current.xMm + params.at('I'), current.yMm + params.at('J')};
    XYPos target;
    target.xMm = _absolute ? params.at('X') : current.xMm + params.at('X');
    target.yMm = _absolute ? params.at('Y') : current.yMm + params.at('Y');
    double feed = _motionState.penIsDown() ? _runtimeSettings.drawFeedRate_mm_per_s() : _runtimeSettings.travelFeedRate_mm_per_s();
    if (params.count('F')) feed = params.at('F');

    _motion.arcToXY(target, center, clockwise, feed);
}

void GCodeExecuter::handleQUAD(const std::map<char,double>& params) {
    XYPos current = _motion.getCurrentPos();
    if (!params.count('X') || !params.count('Y') || !params.count('C') || !params.count('D')) {
        ESP_LOGW(TAG, "G5 command missing required parameter");
        return;
    }

    XYPos target, control;
    target.xMm = _absolute ? params.at('X') : current.xMm + params.at('X');
    target.yMm = _absolute ? params.at('Y') : current.yMm + params.at('Y');
    control.xMm = params.at('C');       // Control X
    control.yMm = params.at('D');       // Control Y (use 'D' as second letter for clarity)
    double feed = _motionState.penIsDown() ? _runtimeSettings.drawFeedRate_mm_per_s() : _runtimeSettings.travelFeedRate_mm_per_s();
    if (params.count('F')) feed = params.at('F');

    _motion.quadraticBezierToXY(control, target, feed);
}

void GCodeExecuter::handleCUBIC(const std::map<char,double>& params) {
    XYPos current = _motion.getCurrentPos();
    if (!params.count('X') || !params.count('Y') || !params.count('A') || !params.count('B') || !params.count('C') || !params.count('D')) {
        ESP_LOGW(TAG, "G5.1 command missing required parameter");
        return;
    }

    XYPos target, c1, c2;
    target.xMm = _absolute ? params.at('X') : current.xMm + params.at('X');
    target.yMm = _absolute ? params.at('Y') : current.yMm + params.at('Y');
    c1.xMm = params.at('A'); c1.yMm = params.at('B');
    c2.xMm = params.at('C'); c2.yMm = params.at('D');
    double feed = _motionState.penIsDown() ? _runtimeSettings.drawFeedRate_mm_per_s() : _runtimeSettings.travelFeedRate_mm_per_s();
    if (params.count('F')) feed = params.at('F');

    _motion.cubicBezierToXY(c1, c2, target, feed);
}

// Pen up/down
void GCodeExecuter::handlePenUpDown(const std::string& cmd) {
    if (cmd == "M3") {
        _pen.down();
        delay(100); // Small delay to allow pen to move down before starting motion
    } else if (cmd == "M5") {
        _pen.up();
        delay(100); // Small delay to allow pen to move up before starting motion
    } else {
        ESP_LOGW(TAG, "Unknown pen command: %s", cmd.c_str());
    }
}

void GCodeExecuter::handleG90G91(const std::string& cmd) {
    if (cmd == "G90") {
        _absolute = true;
    } else if (cmd == "G91") {
        _absolute = false;
    } else {
        ESP_LOGW(TAG, "Unknown coordinate mode command: %s", cmd.c_str());
    }
}

void GCodeExecuter::handleHoming(const std::string& cmd) {
    if (cmd == "G28") {
        _homingController.home();
    } else {
        ESP_LOGW(TAG, "Unknown homing command: %s", cmd.c_str());
    }
}
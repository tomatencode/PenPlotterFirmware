#include "RotaryEncoder.hpp"

static const int8_t transitionTable[16] = {
     0, -1,  1,  0,
     1,  0,  0, -1,
    -1,  0,  0,  1,
     0,  1, -1,  0
};

RotaryEncoder::RotaryEncoder(uint8_t dt, uint8_t clk, uint8_t sw, uint16_t debounce)
    : _dt(dt), _clk(clk), _sw(sw), _debounce(debounce), _state(0), _position_delta(0), _stepAccumulator(0),
      _buttonPrsFlag(false), _buttonRelFlag(false), _lastButtonState(HIGH)
{
}

void RotaryEncoder::begin()
{
    pinMode(_dt, INPUT);
    pinMode(_clk, INPUT);
    pinMode(_sw, INPUT_PULLUP);

    _state = (digitalRead(_clk) << 1) | digitalRead(_dt);

    attachInterruptArg(digitalPinToInterrupt(_dt),
        [](void* arg) { static_cast<RotaryEncoder*>(arg)->updateEncoder(); },
        this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(_clk),
        [](void* arg) { static_cast<RotaryEncoder*>(arg)->updateEncoder(); },
        this, CHANGE);
    attachInterruptArg(digitalPinToInterrupt(_sw),
        [](void* arg) { static_cast<RotaryEncoder*>(arg)->handleButton(); },
        this, CHANGE);
}

int RotaryEncoder::getPositionDelta()
{
    noInterrupts();
    int pos_delta = _position_delta;
    _position_delta = 0; // reset after reading
    interrupts();
    return pos_delta;
}

void RotaryEncoder::reset()
{
    noInterrupts();
    _stepAccumulator = 0;
    _position_delta = 0;
    interrupts();
}

bool RotaryEncoder::buttonPressed()
{
    if (_buttonPrsFlag) {
        _buttonPrsFlag = false;
        return true;
    }
    return false;
}

bool RotaryEncoder::buttonReleased()
{
    if (_buttonRelFlag) {
        _buttonRelFlag = false;
        return true;
    }
    return false;
}

bool RotaryEncoder::buttonDown()
{
    noInterrupts();
    bool isDown = digitalRead(_sw) == LOW;
    interrupts();
    return isDown;
}

void RotaryEncoder::updateEncoder()
{
    uint8_t newState = (digitalRead(_clk) << 1) | digitalRead(_dt);

    uint8_t index = (_state << 2) | newState;

    int8_t tempAcc = _stepAccumulator;
    tempAcc += transitionTable[index];

    // Emit a full detent step if accumulator reaches 4
    if (tempAcc >= 4) {
        int tempPos = _position_delta;
        tempPos++;
        _position_delta = tempPos;
        tempAcc -= 4;
    } 
    else if (tempAcc <= -4) {
        int tempPos = _position_delta;
        tempPos--;
        _position_delta = tempPos;
        tempAcc += 4;
    }

    _stepAccumulator = tempAcc;

    _state = newState;
}

void RotaryEncoder::handleButton()
{
    bool buttonState = digitalRead(_sw);

    if (buttonState == _lastButtonState) {
        return; // No change in button state, ignore
    }

    unsigned long now = millis();

    // Handle button press with debounce
    if (now - _ButtonPrsDebounceTime > _debounce && now - _ButtonRelDebounceTime > _debounce) {
        // often a release can trigger a press, so we want to make sure both debounce times have passed before registering a new press
        // this also means that if the button is repressed very quickly, it will be missed but that is a reasonable tradeoff

        // confirm the button is getting pressed
        if (buttonState == LOW) {
            _buttonPrsFlag = true;
            _ButtonPrsDebounceTime = now;
        }
    }

    // Handle button release with debounce
    if (now - _ButtonRelDebounceTime > _debounce) {

        // confirm the button is getting released
        if (buttonState == HIGH) {
            _buttonRelFlag = true;
            _ButtonRelDebounceTime = now;
        }
    }

    _lastButtonState = buttonState;
}


// Created by NayaVu, 2021
// https://github.com/nayavu , https://naya.vu
// MIT License

#ifndef INTERRUPT_DRIVEN_BUTTON_H
#define INTERRUPT_DRIVEN_BUTTON_H

#include <Arduino.h>

// Minimal possible duration (ms) of a click. If click lasted in less time, it is ignored. Very simple button debounce mechanism
#ifndef IDB_MIN_CLICK_DURATION
#define IDB_MIN_CLICK_DURATION 20
#endif

// Within this time (ms) after the last click, the algorithm waits for the next click / hold to detect click series.
#ifndef IDB_MAX_CLICK_SERIES_WAIT_TIME
#define IDB_MAX_CLICK_SERIES_WAIT_TIME 500
#endif

// Minimal duration (ms) of button hold to be recognized as IDB_EVENT_HOLD
#ifndef IDB_HOLD_MIN_DURATION
#define IDB_HOLD_MIN_DURATION 1000
#endif

// Period (ms) to generate "ticks" while button is hold. This period does not include IDB_HOLD_MIN_DURATION
#ifndef IDB_HOLD_TICKS_PERIOD
#define IDB_HOLD_TICKS_PERIOD 500
#endif

// Within this time (ms) after MCU started, the algorithm waits for button hold to possibly recognize IDB_EVENT_BOOT_HOLD
#ifndef IDB_BOOT_HOLD_MAX_MCU_START_TIME
#define IDB_BOOT_HOLD_MAX_MCU_START_TIME 250
#endif

// Minimal duration (ms) of button hold to be recognized as IDB_EVENT_BOOT_HOLD if other criteria matches
#ifndef IDB_BOOT_HOLD_MIN_DURATION
#define IDB_BOOT_HOLD_MIN_DURATION 3000
#endif


#define IDB_EVENT_NONE 0 // there's no button event at the moment
#define IDB_EVENT_CLICKS 1 // the button has been clicked one time or several times in a row (click series)
#define IDB_EVENT_HOLD 2 // the button is being held right now
#define IDB_EVENT_BOOT_HOLD 3 // the button has been held for several seconds during the boot

// It's not possible to pass non-static method as a callback (ISR) and neither capturing lambdas work (it's quite an overhead anyways),
// so here a helper callback function is created.
//
// Usage:
//
// InterruptingButton button(..params..);
// DEFINE_IDB_ISR(button)
// button.setup(IDB_ISR(button));
//
#define DEFINE_IDB_ISR(variableName) \
void ISR__##variableName() { variableName.onInterrupt(); }

#define IDB_ISR(variableName) ISR__##variableName

typedef struct InterruptDrivenButtonEvent {
    byte type = IDB_EVENT_NONE;
    byte clicks = 0;
    byte holdTicks = 0;
} InterruptingButtonEvent;

class InterruptDrivenButton {
private:
    const byte pin;

    // note, that declaration order matters because of alignment - all bit fields should be as close as possible
    // otherwise, it'll add 2 bytes for booleans
    const bool pressedLevel:1; // the default signal level when button is pressed
    volatile bool on:1; // indicates whether the last button event was related to pressed button
    volatile bool touched:1; // the button has been pressed, trying to detect a sequence of clicks or the button is holding
    volatile bool timerOverflow:1; // indicates whether MCU time counter has overflow to prevent IDB_EVENT_BOOT_HOLD misdetection in such cases

    volatile byte clicks = 0; // a number of clicks in the sequence (in any)
    volatile byte holdTicks = 0; // a number of ticks passed since the button has been held

    volatile unsigned long changedAt = 0; // timestamp of last interrupt
    volatile InterruptingButtonEvent event = {}; // the last triggered event

public:
    InterruptDrivenButton(byte _pin, bool _pressedLevel): pin(_pin), pressedLevel(_pressedLevel), on(false), touched(false), timerOverflow(false) {};
    InterruptDrivenButton(byte _pin): InterruptDrivenButton(_pin, HIGH) {};

    void setup(void(callback)());
    void loop();

    bool hasEvent();
    InterruptDrivenButtonEvent pollEvent();

    void onInterrupt();
};


#endif //INTERRUPT_DRIVEN_BUTTON_H

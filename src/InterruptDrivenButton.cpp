#include "InterruptDrivenButton.h"

void InterruptDrivenButton::setup(void(callback)()) {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), callback, CHANGE);
}

inline uint32_t timeDiff(uint32_t future, uint32_t past) {
    // Compare two timestamps considering possible time overflow.
    // It works for time intervals less than ~24 days
    if (past <= UINT32_MAX/2 || future > past) {
        return future - past;
    } else {
        // time overflow
        return (UINT32_MAX - past) + future;
    }
}

void InterruptDrivenButton::onInterrupt() {
    unsigned long now = millis();
    if (now < IDB_MIN_CLICK_DURATION) {
        // ignore any events within first milliseconds after the boot
        // to allow IDB_EVENT_BOOT_HOLD detection (holding the button while Arduino is starting)
        return;
    }

    on = digitalRead(pin) == this->pressedLevel;

    if (!touched) {
        if (!on) {
            // the button has been released but we never tracked when it was pressed - most likely it had been pressed before MCU booted
            if (now >= IDB_BOOT_HOLD_MIN_DURATION && !timerOverflow) {
                event.type = IDB_EVENT_BOOT_HOLD;
                event.clicks = 0;
                event.holdTicks = 0;
            }
        } else {
            // button is pressed - remember it
            touched = true;
        }

        clicks = 0;
        holdTicks = 0;

        changedAt = now;
        return;
    }

    // if button is pressed
    if (on) {
        // the button is pressed but we already tracked the previous click - it's either click series or click (click series) + hold
        changedAt = now;
        return;
    }

    // if the button is released

    // Check when the button was pressed. If right after MCU boot, then it's IDB_EVENT_BOOT_HOLD
    if (changedAt <= IDB_BOOT_HOLD_MAX_MCU_START_TIME && !timerOverflow) {
        // Ignore "short" clicks (debounce)
        if (now >= IDB_BOOT_HOLD_MIN_DURATION) {
            event.type = IDB_EVENT_BOOT_HOLD;
            event.clicks = 0;
            event.holdTicks = 0;
        }
        touched = false;

        clicks = 0;
        holdTicks = 0;

        return;
    }

    if (holdTicks) {
        // the button is released after hold
        touched = false;

        clicks = 0;
        holdTicks = 0;

        return;
    }

    // the button is released and it's a click
    if (timeDiff(now, changedAt) >= IDB_MIN_CLICK_DURATION) {
        // Ignore "short" clicks (debounce)
        clicks++;
    }

    changedAt = now;
}

void InterruptDrivenButton::loop() {
    if (!touched) {
        return;
    }
    if (changedAt <= IDB_BOOT_HOLD_MAX_MCU_START_TIME) {
        return;
    }

    unsigned long now = millis();
    if (!timerOverflow && now > IDB_BOOT_HOLD_MAX_MCU_START_TIME) {
        // detect time overflow to prevent invalid IDB_EVENT_BOOT_HOLD triggers
        timerOverflow = true;
    }

    unsigned long timeSinceLastChange = timeDiff(now, changedAt);
    if (on && timeSinceLastChange > IDB_HOLD_MIN_DURATION) {
        // check how long the button has been holding and trigger event with appropriate number of ticks
        byte ticks = (timeSinceLastChange - IDB_HOLD_MIN_DURATION) / IDB_HOLD_TICKS_PERIOD + 1;
        if (ticks != holdTicks) {
            holdTicks = ticks;
            event.type = IDB_EVENT_HOLD;
            event.clicks = clicks;
            event.holdTicks = holdTicks;
        }
    } else if (!on && timeSinceLastChange >= IDB_MAX_CLICK_SERIES_WAIT_TIME) {
        if (clicks && !holdTicks) {
            event.type = IDB_EVENT_CLICKS;
            event.clicks = clicks;
            event.holdTicks = 0;
        }
        touched = false;
        clicks = 0;
        holdTicks = 0;
    }
}

bool InterruptDrivenButton::hasEvent() {
    return event.type != IDB_EVENT_NONE;
}

InterruptDrivenButtonEvent InterruptDrivenButton::pollEvent() {
    InterruptDrivenButtonEvent res;
    memcpy(&res, (const void *) &event, sizeof(event));
    memset((void *) &event, 0, sizeof(event));
    return res;
}

#include <Arduino.h>
#include <InterruptDrivenButton.h>

InterruptDrivenButton myButton(2);
DEFINE_IDB_ISR(myButton)

void setup() {
    myButton.setup(IDB_ISR(myButton));
    Serial.begin(19200);
    Serial.println("Initialized");
}

byte prevHoldingTicks = 0;
void handleButtonEvents() {
    if (!myButton.hasEvent()) {
        return;
    }

    InterruptDrivenButtonEvent event = myButton.pollEvent();
    if (event.type == IDB_EVENT_NONE) {
        return;
    }

    if (event.type == IDB_EVENT_CLICKS) {
        Serial.print("Clicked ");
        Serial.print(event.clicks);
        Serial.println(" times");
    } else if (event.type == IDB_EVENT_HOLD) {
        if (prevHoldingTicks == event.holdTicks) {
            return;
        }
        if (event.clicks) {
            Serial.print("Clicked ");
            Serial.print(event.clicks);
            Serial.print(" times and hold ");
        } else {
            Serial.print("Hold ");
        }
        Serial.print(event.holdTicks);
        Serial.println(" ticks");

        prevHoldingTicks = event.holdTicks;
    } else if (event.type == IDB_EVENT_BOOT_HOLD) {
        Serial.println("Hold during boot");
    }
}

void loop() {
    myButton.loop();
    handleButtonEvents();
    delay(1);
}

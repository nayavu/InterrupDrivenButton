# InterruptDrivenButton

InterruptDrivenButton v0.1.0

[russian version](README_ru.md)

## Description

This library manages button events for Arduino. Key features:
- button state is listened via interrupts which allows to precisely detect button press and release even when MCU is busy 
  in contrast to the traditional approach of polling button state in a loop;
- detects click series, button hold, click series + hold and button hold during the MCU boot;
- normally closed and normally opened buttons are supported.

When the button is clicked several times in a row, the library counts number of clicks and notifies the program only when click series is completed.
While holding the button, the library counts number of ticks (i.e. instantly growing duration of hold).

Note, that the button MUST be connected to a pin which supports interrupts. For Arduino Uno, Nano and Mini it's pin 2 or 3.
Look [here](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/) for details.

## Methods and functions

```c++

// constructor - normally opened button
InterruptDrivenButton button1(BTN_PIN);

// constructor - normally closed button (the second argument stays for the signal level while pressed)
InterruptDrivenButton button1(BTN_PIN, LOW);

// helping macro to create a callback ISR
DEFINE_IDB_ISR(button1) // it basically creates this: void ISR__button1 { button1.onInterrupt(); }

// helping macro to get the ISR created in DEFINE_IDB_ISR
IDB_ISR(button1) // returns ISR__button1

// runtime initialisation, the argument is ISR (result of IDB_ISR(button1) )
button1.setup(ISR__callback);

// internal loop 
button1.loop();

// checks whether button event happened, returns true or false
// It's not strongly necessary to call `hasEvent` before `pollEvent` as the latest also can say whether any event happened
// but it's more effective to do explicit check for slightly better performance
button1.hasEvent();

// returns button event (if any)
InterruptDrivenButtonEvent event = button.pollEvent();

```

`InterruptDrivenButtonEvent` structure contains 3 fields:
- `byte type` - event type (see below)
- `byte clicks` - number of clicks in click series
- `byte holdTicks` - number of ticks since the button has been held

Event types:
- `IDB_EVENT_NONE` - there's no button event at the moment
- `IDB_EVENT_CLICKS` - the button has been clicked one time or several times in a row (click series). The `clicks` field contains number of clicks.
  The event is triggered when click series is detected.
- `IDB_EVENT_HOLD` - the button is being held right now. The `holdTicks` field contains number of ticks (on other words, time since button has been pressed). The `clicks` field may contain number of clicks before the hold if this hold happens in click series.
  The event is triggered during button hold. Usage example: increase/decrease light brightness.
- `IDB_EVENT_BOOT_HOLD` - the button has been held for several seconds during the boot.
  The event is triggered when the button is released. Usage example: reset EEPROM.

## Configuration

All time parameters are set globally in milliseconds via `#define`.

- `IDB_MIN_CLICK_DURATION` - Minimal possible duration of a click. If click lasted in less time, it is ignored. Very simple button debounce mechanism. Default is `20ms`
- `IDB_MAX_CLICK_SERIES_WAIT_TIME` - Within this time after the last click, the algorithm waits for the next click / hold to detect click series. Default is `500ms`
- `IDB_HOLD_MIN_DURATION` - Minimal duration of button hold to be recognized as `IDB_EVENT_HOLD`. Default is `1000ms` 
- `IDB_HOLD_TICKS_PERIOD` - Period to generate "ticks" while button is being held. `IDB_HOLD_MIN_DURATION` is not counted. Default is `500ms`
- `IDB_BOOT_HOLD_MAX_MCU_START_TIME` - Within this time after MCU started, the algorithm waits for button hold to possibly detect `IDB_EVENT_BOOT_HOLD`. Does not count time overflows.
- `IDB_BOOT_HOLD_MIN_DURATION` - Minimal duration of button hold to be recognized as `IDB_EVENT_BOOT_HOLD` if previous criteria matches

## Examples

```c++
#include <InterruptDrivenButton.h>

InterruptDrivenButton myButton(BTN_PIN);

// helping macro to create ISR (won't work without this!) 
DEFINE_IDB_ISR(myButton)

void setup() {
    // IDB_ISR - another helping macro to get ISR name (won't work without this!)
    myButton.setup(IDB_ISR(myButton)); 
}

void loop() {
    myButton.loop();
    
    ...
    
    if (myButton.hasEvent()) {
        
        InterruptDrivenButtonEvent event = myButton.pollEvent(); 
        if (event.type != IDB_EVENT_NONE) {
            // despite the fact that hasEvent() == true, it's worth to check the actual event

            if (event.type == IDB_EVENT_CLICKS) {
                switch (event.clicks) {
                    case 1:
                        // 1 click
                        break;
               
                    case 2:
                        // 2 clicks 
                        break;
                
                    case 3:
                        // 3 clicks
                        break;    
                    .....
                }
            } else if (event.type == IDB_EVENT_HOLD) {
                byte ticks = event.holdingTicks; // 1, 2, 3, 4.... constantly increasing during button hold
                byte clicks = event.clicks; // may contain number of clicks if it's click series 
            } else if (event.type == IDB_EVENT_BOOT_HOLD) {
                // button was held during the boot
            }
        }
    }
}
```

Inspired by [GyverButton](https://github.com/AlexGyver/GyverLibs/tree/master/GyverButton) from Alex Gyver.

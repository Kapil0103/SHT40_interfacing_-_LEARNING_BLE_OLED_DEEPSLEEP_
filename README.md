# SHT45 Sensor System

An Arduino-based sensor system built around the **Seeed XIAO nRF52840**. The firmware reads temperature and relative humidity from a Sensirion SHT4x/SHT45 sensor, displays a startup screen on an SSD1306 OLED, and exposes the latest readings over Bluetooth Low Energy (BLE).

The project also contains a motor-control module and a push-button interface for starting a measurement or entering the nRF52 System OFF sleep mode.

## Features

- **Temperature and humidity sensing:** Reads the SHT4x sensor over I2C using the Sensirion library.
- **OLED display:** Uses a 128 × 64 SSD1306 display at I2C address `0x3C`.
- **BLE data service:** Advertises a custom BLE service with separate temperature and humidity characteristics. Both characteristics support read and notify operations.
- **Button interface:** Uses an active-low push button with the internal pull-up enabled.
- **Measurement collection:** Collects up to 20 readings at approximately one-second intervals, with a maximum measurement duration of 20 seconds.
- **Sleep mode:** A long press enters nRF52 System OFF after stopping the motor output, turning off the OLED, and stopping BLE advertising.
- **Motor-control module:** Provides setup, PWM speed, and stop functions for a motor driver.

## Hardware

| Component | Connection / configuration |
|---|---|
| Controller | Seeed XIAO nRF52840 |
| Temperature and humidity sensor | SHT45 / SHT4x, I2C |
| OLED | SSD1306, 128 × 64, I2C address `0x3C` |
| Push button | `D2`, configured as `INPUT_PULLUP` (button active LOW) |
| Motor driver enable | `D3` (PWM output) |
| Motor driver input 1 | `D8` |
| Motor driver input 2 | `D9` |

**I2C pins on XIAO nRF52840:** `D4` (SDA) and `D5` (SCL).

Connect the sensor and OLED to the I2C bus. Check the pinout and electrical requirements of the particular sensor, display, and motor-driver boards before wiring. The motor should use an appropriate external motor supply; connect the controller and driver grounds together.

> The motor module currently defines the pins and provides PWM/stop functions, but the main sketch does not currently run a motor-speed sequence. Verify the motor-driver wiring and direction logic before using it with a motor.

## Button operation

The button is configured with the internal pull-up resistor, so a press reads `LOW` and a release reads `HIGH`.

| Action | Behaviour |
|---|---|
| Short press and release while idle | Starts a measurement collection |
| Long press (2 seconds) | Enters System OFF sleep mode |
| Long press during measurement | Enters System OFF when the button handler next runs |
| Short press during measurement | Does not start another measurement because `measurementRunning` is true |

The firmware calls `handleButton()` from both `loop()` and the measurement routine. This is necessary because the measurement routine currently uses a blocking loop. Button detection therefore happens when the program reaches a call to `handleButton()`; it is not interrupt-driven and cannot run while another function is executing.

In the current sketch, the button handler is called before each sensor read. The one-second sampling wait uses a short `delay(1)` loop but does not call `handleButton()` inside that wait. As a result, a long press during that interval may not be detected until the wait finishes and the next measurement-loop iteration begins.

## Measurement behaviour

The `collectSensorData()` function:

1. Sets `measurementRunning` to `true` and resets the reading count.
2. Starts a 20-second measurement window.
3. Reads temperature and humidity from the SHT4x sensor.
4. Stores each successful reading and its elapsed timestamp in the `readings` array.
5. Updates the BLE temperature and humidity characteristics and sends notifications when a BLE connection is subscribed.
6. Waits for the next approximately one-second sampling point.
7. Stops after 20 seconds or when the 20-reading array is full, then sets `measurementRunning` to `false`.

The current firmware prints readings and the total number collected to the Serial interface. It does not currently display the live readings on the OLED or save them to non-volatile storage.

## BLE service

The BLE implementation is in `blet.cpp` and uses the Adafruit Bluefruit nRF52 library.

| BLE item | UUID | Properties |
|---|---|---|
| Sensor service | `0x1234` | Custom service |
| Temperature characteristic | `0x1235` | Read, Notify |
| Humidity characteristic | `0x1236` | Read, Notify |

The device advertises with the name `SHT45_Device`. Temperature and humidity values are formatted as strings with two decimal places. Notifications are sent when a connected central has enabled notifications for the characteristics.

## Sleep mode and wake-up

`enterSleepMode()` currently performs these actions:

1. Stops the motor PWM output.
2. Displays `SLEEP` on the OLED.
3. Turns the OLED display off.
4. Stops BLE advertising.
5. Configures an nRF GPIO input for wake-up on a LOW level.
6. Calls `sd_power_system_off()` to enter System OFF.

System OFF is a deep low-power state. A wake event causes the nRF52840 to reset and execute `setup()` again; it does not resume execution from the line after `sd_power_system_off()`.

**Important wake-pin check:** The sketch configures `NRF_GPIO_PIN_MAP(0, 28)` as the wake-up pin and comments that this corresponds to XIAO `D2`. Confirm this mapping against the Seeed XIAO nRF52840 board variant/pinout and your actual wiring before relying on button wake-up. The button must also be in the correct released/pressed state when entering System OFF to avoid an unintended immediate wake.

## Project structure

```text
sht45usage/
├── sht45usage.ino             # Main application, setup, loop, button and measurement logic
├── sht40.h / sht40.cpp        # SHT4x sensor initialization and reading functions
├── oled.h / oled.cpp          # SSD1306 OLED setup and display object
├── blet.h / blet.cpp          # BLE service, characteristics and reading updates
├── motor.h / motor.cpp        # Motor-driver setup, PWM speed and stop functions
├── SensirionI2cSht4x.h        # Sensirion sensor library header included in project
└── SensirionI2cSht4x.cpp      # Sensirion sensor library implementation included in project
```

## Software and libraries

Use the Arduino IDE with the **Seeed nRF52 board package** and select the Seeed XIAO nRF52840 board.

The source includes or uses these libraries/components:

- Arduino core for Seeed nRF52 / XIAO nRF52840
- Sensirion I2C SHT4x library
- Adafruit GFX Library
- Adafruit SSD1306
- Adafruit Bluefruit nRF52 library
- Adafruit TinyUSB (included by the sketch)
- nRF5 SDK/SoftDevice interfaces supplied by the board package (`nrf.h`, `nrf_soc.h`, and `nrf_gpio.h`)

Install the required libraries through Arduino Library Manager or the appropriate board-package library installation method. The exact board package and library versions used for a build should be recorded alongside the project when creating a reproducible release.

## Build and upload

1. Install the Seeed nRF52 board package and the libraries listed above.
2. Open `sht45usage.ino` in Arduino IDE. Keep the `.h` and `.cpp` files in the same sketch folder.
3. Select **Seeed XIAO nRF52840** as the target board.
4. Select the correct USB port.
5. Compile and upload the sketch.
6. Open Serial Monitor at **115200 baud** to view startup messages and measurement readings.

The sketch currently contains `while (!Serial)`, which can wait for the USB serial connection to become available. For standalone operation, consider replacing this with a bounded timeout or removing the wait so setup does not depend on the Serial Monitor being open.

## Current limitations / items to verify

- Button polling is cooperative: it only happens when `handleButton()` is called. A long blocking sensor or BLE operation can delay button response.
- The one-second measurement wait currently does not poll the button; adding `handleButton()` inside that wait would improve responsiveness.
- `measurementRunning` prevents a short press from starting a second measurement, but the current long-press action always enters System OFF rather than returning to an idle screen.
- Verify that XIAO `D2` really maps to nRF GPIO `P0.28` for the selected board variant before relying on System OFF wake-up.
- Check motor direction pin initialization and the motor-driver/motor compatibility before connecting hardware. The current `motorSetup()` writes `IN1` HIGH and then LOW; it does not explicitly set `IN2` to a direction level.
- BLE notifications require a connected BLE central and notification subscription. Advertising alone does not guarantee that readings are being received by an app.
- The sketch uses a fixed 20-reading buffer and does not currently persist data after reset or power loss.

## Repository notes

This README describes the source snapshot in this project folder. Update the hardware pin table, library versions, and behaviour notes whenever the wiring or firmware changes.

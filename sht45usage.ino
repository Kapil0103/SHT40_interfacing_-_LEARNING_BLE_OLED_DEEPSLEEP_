#include <Arduino.h>
#include "SensirionI2cSht4x.h"
#include <Wire.h>
#include "sht40.h"
#include "oled.h"
#include <Adafruit_TinyUSB.h>
#include "blet.h"
#include "motor.h"
#include <nrf.h>
#include <nrf_soc.h>
#include <nrf_gpio.h>
#include <bluefruit.h>


const int BUTTON_PIN = D2;

const unsigned long LONG_PRESS_TIME = 2000;

unsigned long buttonPressTime = 0;
bool buttonPressed = false;
bool longPressHandled = false;
bool measurementRunning = false;


float temperature = 0.0;
float humidity = 0.0;

struct SensorReading {
  float temperature;
  float humidity;
  unsigned long timestamp;
};

const int MAX_READINGS = 20;

SensorReading readings[MAX_READINGS];

int readingCount = 0;

void collectSensorData();
void showStartupScreen();
void handleButton();
void enterSleepMode();

const uint8_t startupLogo[] = {
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x1F, 0xF8, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x07, 0xF0, 0x0E, 0xF0,
  0x08, 0xE0, 0x07, 0x08,
  0x08, 0xC0, 0x03, 0x08,
  0x00, 0xC1, 0x83, 0x00,
  0x00, 0xC3, 0xC3, 0x00,
  0x00, 0xC6, 0x63, 0x00,
  0x00, 0xC6, 0x63, 0x00,
  0x00, 0xC3, 0xC3, 0x00,
  0x00, 0xC1, 0x83, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xCF, 0xF3, 0x00,
  0x00, 0xCF, 0xF3, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xCF, 0xF3, 0x00,
  0x00, 0xCF, 0xF3, 0x00,
  0x00, 0xC0, 0x03, 0x00,
  0x00, 0xE0, 0x07, 0x00,
  0x00, 0x70, 0x0E, 0x00,
  0x00, 0x3F, 0xFC, 0x00,
  0x00, 0x1F, 0xF8, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

void setup() {

  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(1000);
  while (!Serial) {
    delay(100);
  }
  bleSetup();
  motorSetup();

  if (!shtSetup()) {
    Serial.println("Sht40 Initialization failed");
  }
  if (!oledSetup()) {
    Serial.print("oled init failed");
    while (1)
      ;
  }

  showStartupScreen();
  delay(2000);

  Serial.println("System started!");
  Serial.println("Short press: Start collection");
  Serial.println("Long press: Sleep mode");
  // if(!){
  //   Serial.print("ble init failed");
  // }
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("HELLO");

  display.setTextSize(1);
  Serial.println("SHT40READY!");
  Serial.println("OLEdREADY!");
  display.println("SHT40 READY");
  display.println("SYSTEM STARTED");

  display.display();

  Serial.println("SHT40READY!");
  Serial.println("OLEdREADY!");

  Serial.println("Press button to start data collection");
  display.println("Press button to start data collection");
}

void loop() {

  //   if (shtRead(temperature, humidity)) {
  //     Serial.print("Temperature: ");
  //     Serial.print(temperature);

  //     Serial.print(" Humidity: ");
  //     Serial.println(humidity);
  //   }

  // motorSetSpeed(30);
  // delay(3000);

  // motorSetSpeed(60);
  // delay(3000);

  // motorSetSpeed(100);
  // delay(3000);

  // motorStop();
  // delay(3000);

  // if (digitalRead(BUTTON_PIN) == LOW) {
  //   delay(30);  //Button Debounce
  //   if (digitalRead(BUTTON_PIN) == LOW) {
  //     Serial.println("BUTON PRESSED!");
  //     // collectSensorData();
  //     for (int i = 0; i < 100; i += 10) {
  //       motorSetSpeed(i);
  //       delay(1000);
  //     }
  //     Serial.println("Press button to start next collection");
  //     motorStop();

  //     while (digitalRead(BUTTON_PIN) == LOW) {
  //       delay(10);
  //     }
  //     delay(30);
  //   }
  // }
  handleButton();
}

void collectSensorData() {
  measurementRunning = true;

  readingCount = 0;

  unsigned long startTime = millis();
  unsigned long nextSample = startTime;

  while (millis() - startTime < 20000 && readingCount < MAX_READINGS) {
    handleButton();
    float temp = 0.0;
    float hum = 0.0;

    if (shtRead(temp, hum)) {
      readings[readingCount].temperature = temp;
      readings[readingCount].humidity = hum;
      readings[readingCount].timestamp = millis() - startTime;

      bleUpdateReadings(temp, hum);


      Serial.print(readings[readingCount].timestamp);
      Serial.print("\t\t");

      Serial.print(readings[readingCount].temperature);
      Serial.print("\t\t");

      Serial.println(readings[readingCount].humidity);

      readingCount++;

    }

    nextSample += 1000;

    while ((long)(nextSample - millis()) > 0) {
      delay(1);
    }
  }
  // Print collected data
  Serial.println("\nCollection completed!");

  Serial.print("Total readings: ");
  Serial.println(readingCount);

  Serial.println("Time(ms)\tTemperature\tHumidity");

  // for (int i = 0; i < readingCount; i++)
  // {
  //     Serial.print(readings[i].timestamp);
  //     Serial.print("\t\t");

  //     Serial.print(readings[i].temperature);
  //     Serial.print("\t\t");

  //     Serial.println(readings[i].humidity);
  // }
  measurementRunning = false;
}

void showStartupScreen() {
  display.clearDisplay();

  display.drawBitmap(48, 3, startupLogo, 32, 32, SSD1306_WHITE);

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(19, 39);
  display.println("SENSOR");

  display.setTextSize(1);
  display.setCursor(32, 57);
  display.println("SYSTEM READY");

  display.display();
}

void handleButton() {
  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW && !buttonPressed) {
    delay(30);
    if (digitalRead(BUTTON_PIN) == LOW) {
      buttonPressed = true;
      buttonPressTime = millis();
      longPressHandled = false;
    }
  }

  if (buttonPressed && !longPressHandled) {
    if (millis() - buttonPressTime >= LONG_PRESS_TIME) {
      longPressHandled = true;
      Serial.println("Long press detected!");
      enterSleepMode();
    }
  }
  if (buttonState == HIGH && buttonPressed) {
    if (!longPressHandled) {
      Serial.println("Short press detected!");
      if (!measurementRunning) {
        collectSensorData();
      }
    }
    buttonPressed = false;
  }
}

void enterSleepMode() {

  Serial.println("Entering sleep mode...");

  motorStop();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println("SLEEP");
  display.display();

  delay(500);

  // Turn OLED display off
  display.ssd1306_command(SSD1306_DISPLAYOFF);

  // Stop BLE advertising
  Bluefruit.Advertising.stop();

  // Configure D2 (P0.28) as wake-up source
  nrf_gpio_cfg_sense_input(
    NRF_GPIO_PIN_MAP(0, 28),
    NRF_GPIO_PIN_PULLUP,
    NRF_GPIO_PIN_SENSE_LOW);

  // Enter System OFF
  sd_power_system_off();

  // Should not execute after entering System OFF
  while (1)
    ;
}

#include "blet.h"
#include <bluefruit.h>

BLEService sensorService(0x1234);

BLECharacteristic temperatureChar(0x1235);
BLECharacteristic humidityChar(0x1236);




bool bleSetup() {

  Bluefruit.begin();

  Bluefruit.setName("SHT45_Device");

  sensorService.begin();

  temperatureChar.setProperties(
    CHR_PROPS_READ | CHR_PROPS_NOTIFY);

  temperatureChar.setPermission(
    SECMODE_OPEN,
    SECMODE_NO_ACCESS);

  temperatureChar.setMaxLen(12);
  temperatureChar.begin();

  temperatureChar.write("0.00");

  humidityChar.setProperties(
    CHR_PROPS_READ | CHR_PROPS_NOTIFY);

  humidityChar.setPermission(
    SECMODE_OPEN,
    SECMODE_NO_ACCESS);

  humidityChar.setMaxLen(12);
  humidityChar.begin();

  humidityChar.write("0.00");

  Bluefruit.Advertising.addService(sensorService);
  Bluefruit.Advertising.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);

  Bluefruit.Advertising.start(0);

  return true;
}

void bleUpdateReadings(float temperature, float humidity) {

  char tempBuffer[12];
  char humidityBuffer[12];

  snprintf(tempBuffer, sizeof(tempBuffer), "%.2f", temperature);
  snprintf(humidityBuffer, sizeof(humidityBuffer), "%.2f", humidity);

  temperatureChar.write(tempBuffer);
  humidityChar.write(humidityBuffer);

  temperatureChar.notify(tempBuffer);
  humidityChar.notify(humidityBuffer);
}
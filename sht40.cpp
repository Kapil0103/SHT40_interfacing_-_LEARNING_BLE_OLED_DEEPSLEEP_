#include "SHT40.h"
#include "SensirionI2cSht4x.h"
#include <Wire.h>


// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

//Sensor object
static SensirionI2cSht4x sensor;

static char errorMessage[64];
static int16_t error;

bool shtSetup() {

  Wire.begin();
  sensor.begin(Wire, SHT40_I2C_ADDR_44);

  sensor.softReset();
  delay(10);
  uint32_t serialNumber = 0;
  error = sensor.serialNumber(serialNumber);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute serialNumber(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return false;
  }
  Serial.print("serialNumber: ");
  Serial.print(serialNumber);
  Serial.println();

  return true;
}

bool shtRead(float &temperature, float &humidity) {

  error = sensor.measureLowestPrecision(temperature, humidity);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute measureLowestPrecision(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return false;
  }

  return true;
}

#pragma once

#include <Arduino.h>

bool bleSetup();

void bleUpdateReadings(float temperature, float humidity);
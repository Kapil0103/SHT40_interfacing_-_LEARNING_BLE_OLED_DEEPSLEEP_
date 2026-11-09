#pragma once

#include <Arduino.h>


bool shtSetup();

bool shtRead(float &temperature, float &humidity);


/**
 * @file fram_test.cpp
 * @author Marcos Barrios
 * @brief Meant to test whether FRAM's pins are properly connected.
 * @version 0.1
 * @date 2023-09-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>

#include <memory_fram.h>
#include <SPIMemory.h>

#include "SPI.h"
#include "Array.h"

// **** first update chip select pins on the class ****

uint8_t obtainedByte = 0x66; // dummy value
bool enabled;

uint8_t obtainedValue = 0x00;
Array<uint8_t, 256> outputBuffer{};

int8_t kClock = 18;
int8_t kOutput = 19;
int8_t kInput = 23;
int8_t kChipSelect = CHIP_SELECT_FRAM;



bool readSerialStr(String &inputStr);

int memoryCapacity;

void setup() {
  int8_t pins[4];
  pins[0] = kClock;
  pins[1] = kOutput;
  pins[2] = kInput;
  pins[3] = kChipSelect;
  SPIFlash flash(pins[0]);

  Serial.begin(9600);
  flash.begin();

  delay(2000);

  memoryCapacity = flash.getUniqueID();
}

void loop() {
  Serial.println("Memory capacity: ");
  Serial.print(memoryCapacity);
  delay(1000);
}

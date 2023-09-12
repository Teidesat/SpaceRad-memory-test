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

#include "SPI.h"

// **** first update chip select pins on the class ****

MemoryFRAM fram;

uint8_t obtainedByte = 0x66; // dummy value

void setup() {
  SPI.begin();
  Serial.begin(9600);
  delay(1000); // TODO: datasheet in chinese, unsure of powerup delay.
  fram.enableWrite();
  delay(1);
  const uint8_t kByteToWrite = 0x83;
  fram.writeByte(kByteToWrite, 22222); // arbitrary address
  obtainedByte = fram.readByte(22222);
}

void loop() {
  Serial.println("Obtained byte value from FRAM read operation: " + obtainedByte);
  delay(1000);
}

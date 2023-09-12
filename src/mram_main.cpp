/**
 * @file mram_test.cpp
 * @author Marcos Barrios
 * @brief Meant to test whether MRAM's pins are properly connected.
 * @version 0.1
 * @date 2023-09-12
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <Arduino.h>
#include <memory_mram.h>

#include "SPI.h"

// **** first update chip select pins on the class ****

MemoryMRAM mram;

uint8_t obtainedByte = 0x66; // dummy value

void setup() {
  SPI.begin();
  Serial.begin(9600);
  delay(400); // minimum wait times according to datasheet
  mram.enableWrite();
  delay(1);
  const uint8_t kByteToWrite = 0x83;
  mram.writeByte(kByteToWrite, 22222); // arbitrary address
  delay(1);
  obtainedByte = mram.readByte(22222);
}

void loop() {
  Serial.println("Obtained byte value from MRAM read operation: " + obtainedByte);
  delay(1000);
}

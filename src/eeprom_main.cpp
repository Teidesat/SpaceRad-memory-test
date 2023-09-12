/**
 * @file eeprom_test.cpp
 * @author Marcos Barrios
 * @brief Meant to test whether EEPROM's pins are properly connected.
 * @version 0.1
 * @date 2023-09-12
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <Arduino.h>
#include <memory_eeprom.h>

#include "SPI.h"

// **** first update chip select pins on the class ****

MemoryEEPROM eeprom;

uint8_t obtainedByte = 0x66; // dummy value

void setup() {
  SPI.begin();
  Serial.begin(9600);
  delay(1000);
  eeprom.enableWrite();
  delay(1);
  const uint8_t kByteToWrite = 0x83;
  eeprom.writeByte(kByteToWrite, 22222); // arbitrary address
  //eeprom.waitUntilReady();
  delay(5); // write time is never larger than 5 ms according to the datasheet
  obtainedByte = eeprom.readByte(22222);
}

void loop() {
  Serial.println("Obtained byte value from EEPROM read operation: " + obtainedByte);
  delay(1000);
}

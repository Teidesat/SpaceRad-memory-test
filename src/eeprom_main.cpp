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

#include <memory_eeprom.h>
#include <Arduino.h>

#include "SPI.h"

// **** first update chip select pins on the class ****

MemoryEEPROM eeprom;

uint8_t obtainedByte = 0x66; // dummy value

SPIClass hspi(HSPI); // ESP32 apparently requires to declare which SPI instance to use

void setup() {
  pinMode(CHIP_SELECT_EEPROM, OUTPUT);
  hspi.begin();
  Serial.begin(9600);
  delay(1000);
  eeprom.enableWrite();
  delay(1);
  bool enabled = eeprom.isWriteEnabled();
  delay(1);
  // if (enabled) {
  //   Serial.println("Write is enabled");
  // } else {
  //   Serial.println("Write is disabled.");
  // }
  const uint8_t kByteToWrite = 0x83;
  eeprom.writeByte(kByteToWrite, 22222); // arbitrary address
  //eeprom.waitUntilReady();
  delay(5); // write time is never larger than 5 ms according to the datasheet
  obtainedByte = eeprom.readByte(22222);
}

void loop() {
  Serial.println("Obtained byte value from EEPROM read operation: ");
  Serial.print(HSPI);
  Serial.println();
  // Serial.print(obtainedByte);
  // Serial.println();
  // int valueOfInput = digitalRead(13);
  // Serial.print("Input pin: ");
  // Serial.print(valueOfInput);
  // Serial.println();
  // int valueOfOutput = digitalRead(12);
  // Serial.print("Output pin: ");
  // Serial.print(valueOfOutput);
  // Serial.println();
  // int valueOfChipSelect = digitalRead(18);
  // Serial.print("Chip select pin:valueOf ");
  // Serial.print(valueOfChipSelect);
  // Serial.println();
  // int valueOfHold = digitalRead(19);
  // Serial.print("Hold pin: ");
  // Serial.print(valueOfHold);
  // Serial.println();
  // int valueOfWrite = digitalRead(20);
  // Serial.print("Write protect pinvalueOf: ");
  // Serial.print(valueOfWrite);
  // Serial.println();
  delay(1000);
}

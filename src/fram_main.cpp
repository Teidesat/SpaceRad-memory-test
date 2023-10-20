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
#include "Array.h"

// **** first update chip select pins on the class ****

MemoryFRAM fram;

uint8_t obtainedByte = 0x66; // dummy value
bool enabled;

uint8_t obtainedValue = 0x00;
Array<uint8_t, 256> outputBuffer{};

void setup() {
  pinMode(CHIP_SELECT_FRAM, OUTPUT);
  SPI.begin();
  Serial.begin(9600);
  // delay(1000); // TODO: datasheet in chinese, unsure of powerup delay.
  // fram.enableWrite();
  // delay(1);
  // enabled = fram.isWriteEnabled();
  // const uint8_t kByteToWrite = 0x83;
  // fram.writeByte(kByteToWrite, 22222); // arbitrary address
  // obtainedByte = fram.readByte(22222);
  delay(1000);
  uint8_t valueToWrite = 0x76;
  Array<uint8_t, 256> buffer{};
  for (int i = 0; i < 256; i++) {
    buffer[i] = i;
  }
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  for (int i = 0; i < 256; i++) {
    outputBuffer[i] = SPI.transfer(buffer[i]);
  }
  SPI.endTransaction();
}

void loop() {
  Serial.println("Written array: ");
  for (int i = 0; i < 256; i++) {
    Serial.print(outputBuffer[i]);
    Serial.print(" ");
    if (i % 80 == 0) {
      Serial.println();
    }
  }
  Serial.println();
  // Serial.print("Obtained byte value from FRAM read operation: ");
  // Serial.print(obtainedByte);
  // Serial.println();
  // if (enabled) {
  //   Serial.println("Write enabled");
  // } else {
  //   Serial.println("Write disabled");
  // }

  // Serial.print("OUTPUT: ");
  // Serial.print(MISO);
  // Serial.println();
  // Serial.print("INPUT: ");
  // Serial.print(MOSI);
  // Serial.println();
  // Serial.print("CLOCK: ");
  // Serial.print(SCK);
  // Serial.println();
  // Serial.print("SS: ");
  // Serial.print(SS);
  // Serial.println();

  delay(1000);
}

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

void transferN(int* foo, int size) {
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  SPI.transfer(foo, 1);
  SPI.endTransaction();
}

void bar(int* buffer) {
  transferN(buffer, 1);
}

int baz() {
  int baz = 17;
  bar(&baz);
  return baz;
}

void setup() {
  SPI.begin();
  Serial.begin(9600);
  delay(100);
  Serial.println("Bar value: "); 
  Serial.print(baz());
  Serial.println();
}

void loop() {}

// void setup() {
//   pinMode(CHIP_SELECT_FRAM, OUTPUT);
//   SPI.begin();
//   Serial.begin(9600);
//   delay(3000); // TODO: datasheet in chinese, unsure of powerup delay.
//   fram.enableWrite();
//   delay(1);
//   enabled = fram.isWriteEnabled();
//   const uint8_t kByteToWrite = 0x83;
//   fram.writeByte(kByteToWrite, 22222); // arbitrary address
//   obtainedByte = fram.readByte(22222);
// }

// void loop() {
//   Serial.print("Obtained byte value from FRAM read operation: ");
//   Serial.print(obtainedByte);
//   Serial.println();
//   if (enabled) {
//     Serial.println("Write enabled");
//   } else {
//     Serial.println("Write disabled");
//   }
//   delay(1000);
// }

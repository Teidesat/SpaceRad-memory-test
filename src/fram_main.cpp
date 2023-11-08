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
  pinMode(CHIP_SELECT_FRAM, OUTPUT);
  SPI.begin();
  Serial.begin(9600);
  delay(1000); // TODO: datasheet in chinese, unsure of powerup delay.

  // Enable write
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(WREN_FRAM);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();


  delay(1);
  uint8_t kByteToWrite = 0x83;
  const uint8_t address = 0x03;

  // Transfer bytes
  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(WRITE_FRAM);
  SPI.transfer((byte)(address >> 16));
  SPI.transfer((byte)(address >> 8));
  SPI.transfer((byte)address);
  SPI.transfer(&kByteToWrite, 1);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();

  // Read bytes

  SPI.beginTransaction(SPISettings(SPI_TRANSFER_SPEED_FRAM, MSBFIRST, SPI_MODE0));
  digitalWrite(CHIP_SELECT_FRAM, LOW);
  SPI.transfer(READ_FRAM);
  SPI.transfer((byte)(address >> 16));
  SPI.transfer((byte)(address >> 8));
  SPI.transfer((byte)address);
  SPI.transfer(&obtainedByte, 1);
  digitalWrite(CHIP_SELECT_FRAM, HIGH);
  SPI.endTransaction();

}

void loop() {
  Serial.println("Obtained byte value from FRAM read operation: ");
  Serial.print(obtainedByte);
  Serial.println();
  delay(1000);
}
# TEIDESAT-I Radiation memory experiment

Testing the radiation resistance of a FRAM, MRAM, NAND, NOR and EEPROM is one of the objectives of the cubesat TEIDESAT-I. The objective is to verify that the memories are in a failureless operational state.

## Breakout board

An arduino chip is to be connected with the following board:

![docs/breaktout_board.png](docs/breakout_board.png)

## TODO

- Test on real hardware, right now it is all theoretical programming based on the memory datasheets. 30/8

 - In the class EEPROM, change <code>writeByte</code> and <code>writePage</code> to a <code>writeBytes</code> that allows an arbitrary amount of bytes between 1 and 256 to be written (256 is the page size, and it's the maximum size allowed to be written at once). 30/8

- Because EEPROM has a delay when writing, check if the current code for <code>writeByte</code> and <code>writePage</code> is valid or if they need a delay to take into account the write cycle. Maybe reading the WIP flag on the status register is a viable approach. 30/8

 - In [EEPROM's](lib/Memory/memory_eeprom.cpp) <code>writeByte</code> and <code>writePage</code> an enable write is performed first, but I am unsure about whether it's write enable instruction can be always performed or only if the memory is not busy. 30/8

- Implement all instructions as member functions on the FRAM memory class. 30/8

- Implement fast read in FRAM memory class. 30/8.

- Expand [NANDFlash](lib/Memory/memory_nand_flash.h) class definition for dual and quad variants for read and write methods.

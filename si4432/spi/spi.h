#ifndef __SI4432SPI_H__
#define __SI4432SPI_H__

#include <stdint.h>

void InitSpiPort();
void SpiWrite(uint8_t addr, uint8_t value);
uint8_t SpiRead(uint8_t addr);
void SpiBurstWrite(uint8_t addr, uint8_t buf[], uint8_t length);
void SpiBurstRead(uint8_t addr, uint8_t buf[], uint8_t length);
void CloseSpi();

#endif

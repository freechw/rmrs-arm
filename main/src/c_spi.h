#ifndef __C_SPI_H__
#define __C_SPI_H__

extern void c_open(const char *spiDevice, const char *spiInterruptDevice);
extern void c_close();
extern void c_write(unsigned char addr, unsigned char value);
extern unsigned char c_read(unsigned char addr);
extern void c_burstWrite(unsigned char addr, unsigned char buf[], unsigned char length);
extern void c_burstRead(unsigned char addr, unsigned char buf[], unsigned char length);
extern unsigned char c_getInterrupt();

#endif

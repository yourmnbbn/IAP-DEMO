#ifndef __IAP_CRC32_HEADER__ 
#define __IAP_CRC32_HEADER__

#include <stdint.h>

uint32_t updateCRC32(uint8_t ch, uint32_t crc);
uint32_t crc32buf(uint8_t *buf, uint32_t len);

#endif


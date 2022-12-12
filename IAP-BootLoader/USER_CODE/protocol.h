#ifndef __IAP_BOOTLOADER_PROTOCOL__ 
#define __IAP_BOOTLOADER_PROTOCOL__

#include <stdint.h>

#define FLIP_BITS(src, index)				\
				if(src & (1 << index)				\
					src &= ~(1 << index);			\
				else												\
					src |= (1 << index);			
 
typedef struct TransmissionControl_s
{
	int i;
} TransmissionControl_t;


#endif

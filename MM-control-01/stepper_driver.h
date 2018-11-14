#ifndef _STEPDRV_H
#define _STEPDRV_H

#include <inttypes.h>
#include "config.h"


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

extern void driver_init();
extern void driver_enable_motor(uint8_t motor);
extern void driver_enable_all();
extern void driver_disable_motor(uint8_t motor);
extern void driver_disable_all();

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)
#endif //_STEPDRV_H

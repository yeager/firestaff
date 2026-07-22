#ifndef COMPILE_H
#define COMPILE_H

#include <stdint.h>

#define __TURBOC__ 1
#define HUGE
#define huge
#define SEPARATOR ,
#define FINAL_SEPARATOR )
#define STATICFUNCTION static

/* int16_t/uint16_t are supplied by <stdint.h>; do not redefine them here.
 * Historically this header typedef'd them differently, which collided with
 * system headers when COMPILE.H was included first. */

#endif

#ifndef FIRESTAFF_REDMCSB_F0933_GET_HEX_STRING_FROM_VALUE_H
#define FIRESTAFF_REDMCSB_F0933_GET_HEX_STRING_FROM_VALUE_H

#include <stdint.h>

/* ReDMCSB PRIM1.C F0933_GetHexStringFromValue. The caller provides space
 * for the uppercase hexadecimal digits and the trailing null character. */
int F0933_GetHexStringFromValue(uint32_t value, char *string);

#endif

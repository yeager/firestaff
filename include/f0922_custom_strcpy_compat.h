#ifndef FIRESTAFF_F0922_CUSTOM_STRCPY_COMPAT_H
#define FIRESTAFF_F0922_CUSTOM_STRCPY_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bounded adapter for ReDMCSB PRIM1.C F0922_Custom_strcpy.
 *
 * On success, copies source including its terminating NUL and returns
 * destination, just as F0922_Custom_strcpy does. Returns NULL when an input
 * is invalid or destination_capacity cannot hold the complete string; those
 * failures leave destination unchanged. Source and destination must not
 * overlap.
 */
char *f0922_custom_strcpy_compat(char *destination,
                                 size_t destination_capacity,
                                 const char *source);

#ifdef __cplusplus
}
#endif

#endif

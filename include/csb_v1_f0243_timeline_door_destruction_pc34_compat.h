/* ReDMCSB TIMELINE.C F0243 -- C02 door destruction event. */
#ifndef FIRESTAFF_CSB_V1_F0243_TIMELINE_DOOR_DESTRUCTION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0243_TIMELINE_DOOR_DESTRUCTION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sets the three-bit state stored in a CSB door square to C5_DESTROYED.
 * ReDMCSB TIMELINE.C:872-883 performs this sole square mutation. */
void F0243_TIMELINE_ProcessEvent2_DoorDestruction(uint8_t *door_square);

#ifdef __cplusplus
}
#endif

#endif

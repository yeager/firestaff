#ifndef THERON_V1_PROFILE_MEDIA_AVAILABILITY_H
#define THERON_V1_PROFILE_MEDIA_AVAILABILITY_H

#include "theron_v1_media_inventory.h"

typedef enum {
    THERON_V1_PROFILE_MEDIA_MISSING = 0,
    THERON_V1_PROFILE_MEDIA_END_VARIANT = 1,
    THERON_V1_PROFILE_MEDIA_RAW_READY = 2
} Theron_V1ProfileMediaAvailability;

Theron_V1ProfileMediaAvailability theron_v1_profile_media_availability(
    const Theron_V1MediaInventoryReceipt *receipt);

const char *theron_v1_profile_media_availability_name(
    Theron_V1ProfileMediaAvailability availability);

#endif

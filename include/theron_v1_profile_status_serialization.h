#ifndef THERON_V1_PROFILE_STATUS_SERIALIZATION_H
#define THERON_V1_PROFILE_STATUS_SERIALIZATION_H

#include <stddef.h>

typedef struct {
    const char *media;
    const char *audio;
    const char *trace;
    int session_allocated;
} Theron_V1ProfileStatusSerializable;

int theron_v1_profile_status_serialize(
    const Theron_V1ProfileStatusSerializable *in,
    char *out,
    size_t out_cap);

int theron_v1_profile_status_parse(
    const char *text,
    Theron_V1ProfileStatusSerializable *out);

#endif

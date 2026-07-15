#include "theron_v1_profile_status_serialization.h"

#include <stdio.h>
#include <string.h>

int theron_v1_profile_status_serialize(
    const Theron_V1ProfileStatusSerializable *in,
    char *out,
    size_t out_cap) {
    int written;

    if (out && out_cap > 0u) {
        out[0] = '\0';
    }
    if (!in || !out || out_cap == 0u || !in->media || !in->audio ||
        !in->trace) {
        return 0;
    }
    written = snprintf(out, out_cap, "tqps1 media=%s audio=%s trace=%s",
                       in->media, in->audio, in->trace);
    return written > 0 && (size_t)written < out_cap;
}

int theron_v1_profile_status_parse(
    const char *text,
    Theron_V1ProfileStatusSerializable *out) {
    static char media[64];
    static char audio[64];
    static char trace[64];

    if (!text || !out ||
        sscanf(text, "tqps1 media=%63s audio=%63s trace=%63s",
               media, audio, trace) != 3) {
        return 0;
    }
    out->media = media;
    out->audio = audio;
    out->trace = trace;
    out->session_allocated = 0;
    return 1;
}

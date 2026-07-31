/*
 * theron_v22_shapes_runtime_noop.c — production source-data boundary.
 *
 * theron_v22_shapes.c contains an inferred modern material/shape book. It
 * remains available to focused V2.2 fixture targets, but production cannot
 * expose those guessed materials until a source-owned Track 02 route exists.
 */

#include "theron_v22_shapes.h"

void theron_v22_shapes_init(void) { }

const char *theron_v22_shapes_source_evidence(void) {
    return "Track 02 V2.2 shape/material records not decoded; production route blocked";
}

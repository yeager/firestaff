#ifndef THERON_V1_TRACK02_GROUND_REF_H
#define THERON_V1_TRACK02_GROUND_REF_H

#include <stdint.h>

#define THERON_REF_NONE   0xFFFE
#define THERON_REF_UNUSED 0xFFFF

static inline unsigned int theron_ref_id(uint16_t ref) {
    return ref & 0x03FF;
}

static inline unsigned int theron_ref_category(uint16_t ref) {
    return (ref >> 10) & 0x0F;
}

static inline unsigned int theron_ref_position(uint16_t ref) {
    return (ref >> 14) & 0x03;
}

static inline uint16_t theron_ref_make(unsigned int category,
                                        unsigned int position,
                                        unsigned int id) {
    return (uint16_t)((id & 0x03FF) |
                      ((category & 0x0F) << 10) |
                      ((position & 0x03) << 14));
}

static inline int theron_ref_is_end(uint16_t ref) {
    return ref == THERON_REF_NONE || ref == THERON_REF_UNUSED;
}

#endif

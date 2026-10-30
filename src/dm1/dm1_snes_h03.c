#include "dm1_snes_h03.h"

int dm1_snes_h03_decode_slot(const uint8_t *source, size_t source_size,
                            uint8_t *destination, size_t destination_capacity,
                            size_t *decoded_size, size_t *consumed) {
    size_t input = 4;
    size_t output = 0;
    size_t expected;

    if (decoded_size) *decoded_size = 0;
    if (consumed) *consumed = 0;
    if (!source || !destination || source_size < 4 ||
        source_size > DM1_SNES_H03_SLOT_SIZE) return 0;

    expected = ((size_t)source[0] << 24) |
               ((size_t)source[1] << 16) |
               ((size_t)source[2] << 8) |
               (size_t)source[3];
    if (expected == 0 || expected > destination_capacity) return 0;

    while (output < expected) {
        uint8_t control;
        size_t count;

        if (input >= source_size) return 0;
        control = source[input++];
        if ((control & 0x80u) == 0) {
            count = control;
            if (count > expected - output || count > source_size - input)
                return 0;
            for (size_t i = 0; i < count; ++i)
                destination[output++] = source[input++];
        } else {
            size_t distance;
            count = (size_t)(control & 0x7fu) >> 1;
            if (input >= source_size) return 0;
            distance = ((size_t)(control & 1u) << 8) | source[input++];
            if (count > expected - output) return 0;
            if (distance > output || (count != 0 && distance == 0)) return 0;
            for (size_t i = 0; i < count; ++i) {
                destination[output] = destination[output - distance];
                ++output;
            }
        }
    }

    if (decoded_size) *decoded_size = output;
    if (consumed) *consumed = input;
    return 1;
}

#include "redmcsb_f0802_is_magic_map_pc34_compat.h"

#include <stdint.h>
#include <string.h>

static uint16_t captured_text_index;
static uint16_t captured_flags;
static int decode_call_count;
static const char *decoded_result;

static void capture_decode(char *decoded_text,
                           uint16_t text_string_thing_index,
                           uint16_t text_type_and_flags)
{
    captured_text_index = text_string_thing_index;
    captured_flags = text_type_and_flags;
    ++decode_call_count;
    strcpy(decoded_text, decoded_result);
}

int main(void)
{
    uint16_t scroll_text_indices[4] = { 11, 22, 33, 44 };
    uint16_t scroll_thing = (uint16_t)((REDMCSB_F0802_THING_TYPE_SCROLL << 10) | 2);

    decoded_result = "MAGICMAP";
    if (!redmcsb_f0802_is_magic_map_pc34_compat(
            scroll_thing, scroll_text_indices, "MAGICMAP", capture_decode)) {
        return 1;
    }
    if (decode_call_count != 1 || captured_text_index != 33 ||
        captured_flags != (REDMCSB_F0802_TEXT_TYPE_SCROLL |
                           REDMCSB_F0802_DECODE_EVEN_IF_INVISIBLE)) {
        return 2;
    }

    decoded_result = "NOT A MAP";
    if (redmcsb_f0802_is_magic_map_pc34_compat(
            scroll_thing, scroll_text_indices, "MAGICMAP", capture_decode)) {
        return 3;
    }

    decode_call_count = 0;
    if (redmcsb_f0802_is_magic_map_pc34_compat(
            UINT16_C(0x0002), scroll_text_indices, "MAGICMAP", capture_decode)) {
        return 4;
    }
    if (decode_call_count != 0) {
        return 5;
    }
    return 0;
}

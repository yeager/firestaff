#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

#define REQUIRE(condition) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "requirement failed: %s at line %d\n", #condition, \
                    __LINE__); \
            return 1; \
        } \
    } while (0)

typedef struct {
    uint8_t *bitmap_destination;
    uint16_t width;
    int16_t zone_index;
    int16_t text_color;
    int16_t background_color;
    int16_t height;
    unsigned int call_count;
    char string[REDMCSB_F0768_LOCAL_STRING_CAPACITY_PC34];
} capture;

static void capture_print(void *context, uint8_t *bitmap_destination,
                          uint16_t width, int16_t zone_index,
                          int16_t text_color, int16_t background_color,
                          const char *string, int16_t height)
{
    capture *state = context;

    state->bitmap_destination = bitmap_destination;
    state->width = width;
    state->zone_index = zone_index;
    state->text_color = text_color;
    state->background_color = background_color;
    state->height = height;
    state->call_count++;
    strcpy(state->string, string);
}

int main(void)
{
    uint8_t bitmap[4] = {0};
    capture state = {0};

    redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat(
        capture_print, &state, bitmap, 160, 80, 4, 0, "READY", 12, 200);

    REQUIRE(state.call_count == 1U);
    REQUIRE(state.bitmap_destination == bitmap);
    REQUIRE(state.width == 160U);
    REQUIRE(state.zone_index == 80);
    REQUIRE(state.text_color == 4);
    REQUIRE(state.background_color == 0);
    REQUIRE(state.height == 200);
    REQUIRE(strcmp(state.string, "READY       ") == 0);
    REQUIRE(strstr(
                redmcsb_f0768_print_to_zone_with_trailing_spaces_source_evidence_pc34(),
                "TEXT.C:1272-1315") != NULL);

    memset(&state, 0, sizeof(state));
    redmcsb_f0768_print_to_zone_with_trailing_spaces_pc34_compat(
        capture_print, &state, bitmap, 160, 85, 4, 0, "READY", 3, 200);
    REQUIRE(state.call_count == 1U);
    REQUIRE(strcmp(state.string, "READY") == 0);

    puts("ok: ReDMCSB F0768 PC 3.4 zone text padding");
    return 0;
}

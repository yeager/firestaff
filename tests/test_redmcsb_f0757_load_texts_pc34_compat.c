#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0757_load_texts_pc34_compat.h"

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
    const uint8_t *graphic;
    uint16_t graphic_size;
    uint8_t text_storage[32];
    char *string_storage[8];
    unsigned int allocation_count;
    uint16_t count_graphic_index;
    uint16_t load_graphic_index_and_flags;
    uint16_t allocation_type[2];
    uint16_t allocation_flags[2];
    uint32_t allocation_sizes[2];
} capture;

static uint16_t get_byte_count(void *context, uint16_t graphic_index)
{
    capture *state = context;

    state->count_graphic_index = graphic_index;
    return state->graphic_size;
}

static void *allocate(void *context, uint32_t byte_count,
                      uint16_t allocation_type, uint16_t allocation_flags)
{
    capture *state = context;
    unsigned int allocation_index = state->allocation_count++;

    state->allocation_sizes[allocation_index] = byte_count;
    state->allocation_type[allocation_index] = allocation_type;
    state->allocation_flags[allocation_index] = allocation_flags;
    return allocation_index == 0 ? (void *)state->text_storage
                                 : (void *)state->string_storage;
}

static void load_graphic(void *context, uint16_t graphic_index_and_flags,
                         void *destination)
{
    capture *state = context;

    state->load_graphic_index_and_flags = graphic_index_and_flags;
    memcpy(destination, state->graphic, state->graphic_size);
}

int main(void)
{
    static const uint8_t graphic[] = {
        'R', 'E', 'A', 'D', 'Y', 0, 'C', 'L', 'I', 'C', 'K', 0, 0};
    capture state = {0};
    redmcsb_f0757_text_loader_pc34_compat loader;
    redmcsb_f0757_texts_pc34_compat texts = {0};

    state.graphic = graphic;
    state.graphic_size = (uint16_t)sizeof(graphic);
    loader.get_graphic_decompressed_byte_count = get_byte_count;
    loader.allocate = allocate;
    loader.load_decompress_and_expand_graphic = load_graphic;
    loader.context = &state;

    redmcsb_f0757_load_texts_pc34_compat(&loader, &texts);

    REQUIRE(state.count_graphic_index == REDMCSB_F0757_GRAPHIC_TEXTS_PC34);
    REQUIRE(state.load_graphic_index_and_flags ==
            (REDMCSB_F0757_LOAD_FLAGS_PC34 |
             REDMCSB_F0757_GRAPHIC_TEXTS_PC34));
    REQUIRE(state.allocation_count == 2);
    REQUIRE(state.allocation_sizes[0] == sizeof(graphic));
    REQUIRE(state.allocation_sizes[1] == 3U * sizeof(char *));
    REQUIRE(state.allocation_type[0] == REDMCSB_F0757_ALLOCATION_PERMANENT_PC34);
    REQUIRE(state.allocation_type[1] == REDMCSB_F0757_ALLOCATION_PERMANENT_PC34);
    REQUIRE(state.allocation_flags[0] == REDMCSB_F0757_ALLOCATION_FLAGS_PC34);
    REQUIRE(state.allocation_flags[1] == REDMCSB_F0757_ALLOCATION_FLAGS_PC34);
    REQUIRE(texts.texts == (char *)state.text_storage);
    REQUIRE(texts.strings == state.string_storage);
    REQUIRE(texts.string_count == 3);
    REQUIRE(strcmp(texts.strings[0], "READY") == 0);
    REQUIRE(strcmp(texts.strings[1], "CLICK") == 0);
    REQUIRE(strcmp(texts.strings[2], "") == 0);
    REQUIRE(strstr(redmcsb_f0757_load_texts_source_evidence_pc34(),
                   "LANGUAGE.C:15-41") != NULL);

    puts("ok: ReDMCSB F0757 PC 3.4 C700 text loader");
    return 0;
}

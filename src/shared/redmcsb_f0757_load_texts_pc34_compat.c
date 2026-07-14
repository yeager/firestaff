#include "redmcsb_f0757_load_texts_pc34_compat.h"

void redmcsb_f0757_load_texts_pc34_compat(
    const redmcsb_f0757_text_loader_pc34_compat *loader,
    redmcsb_f0757_texts_pc34_compat *texts)
{
    char *character;
    char **string;
    uint16_t byte_index;
    uint16_t string_count;
    uint16_t byte_count;

    /* ReDMCSB LANGUAGE.C:15-41, MEDIA736_I34M PC 3.4 route. */
    byte_count = loader->get_graphic_decompressed_byte_count(
        loader->context, REDMCSB_F0757_GRAPHIC_TEXTS_PC34);
    texts->texts = character = loader->allocate(
        loader->context, byte_count, REDMCSB_F0757_ALLOCATION_PERMANENT_PC34,
        REDMCSB_F0757_ALLOCATION_FLAGS_PC34);
    loader->load_decompress_and_expand_graphic(
        loader->context,
        REDMCSB_F0757_LOAD_FLAGS_PC34 | REDMCSB_F0757_GRAPHIC_TEXTS_PC34,
        character);
    string_count = 0;
    for (byte_index = 0; byte_index < byte_count; byte_index++) {
        if (!*character++) {
            string_count++;
        }
    }
    texts->string_count = string_count;
    texts->strings = string = loader->allocate(
        loader->context, (uint32_t)string_count * sizeof(*string),
        REDMCSB_F0757_ALLOCATION_PERMANENT_PC34,
        REDMCSB_F0757_ALLOCATION_FLAGS_PC34);
    character = texts->texts;
    *string++ = character;
    while (--string_count > 0) {
        while (*character++) {
        }
        *string++ = character;
    }
}

const char *redmcsb_f0757_load_texts_source_evidence_pc34(void)
{
    return "ReDMCSB WIP20210206 LANGUAGE.C:15-41, called from "
           "STARTUP2.C:1369-1371: F0757 allocates the decompressed byte count "
           "for C700_GRAPHIC_TEXTS permanently with MEMREQ, loads it with "
           "NOT_EXPANDED|DO_NOT_COPY_DIMENSIONS, counts NUL bytes, then "
           "allocates and fills one pointer per string.";
}

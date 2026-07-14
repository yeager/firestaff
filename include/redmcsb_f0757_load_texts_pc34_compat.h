/* ReDMCSB LANGUAGE.C F0757_LoadTexts, PC 3.4 language-text route. */
#ifndef FIRESTAFF_REDMCSB_F0757_LOAD_TEXTS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0757_LOAD_TEXTS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0757_GRAPHIC_TEXTS_PC34 = 700,
    REDMCSB_F0757_LOAD_FLAGS_PC34 = 0xc000,
    REDMCSB_F0757_ALLOCATION_PERMANENT_PC34 = 1,
    REDMCSB_F0757_ALLOCATION_FLAGS_PC34 = 0x0400
};

typedef struct {
    uint16_t (*get_graphic_decompressed_byte_count)(void *context,
                                                      uint16_t graphic_index);
    void *(*allocate)(void *context, uint32_t byte_count,
                      uint16_t allocation_type, uint16_t allocation_flags);
    void (*load_decompress_and_expand_graphic)(void *context,
                                               uint16_t graphic_index_and_flags,
                                               void *destination);
    void *context;
} redmcsb_f0757_text_loader_pc34_compat;

typedef struct {
    char *texts;
    char **strings;
    uint16_t string_count;
} redmcsb_f0757_texts_pc34_compat;

/*
 * Source-locks F0757's valid GRAPHICS.DAT route. As in ReDMCSB, the supplied
 * C700 text graphic must contain at least one NUL-terminated string and the
 * allocator must succeed; malformed-asset recovery is deliberately not added.
 */
void redmcsb_f0757_load_texts_pc34_compat(
    const redmcsb_f0757_text_loader_pc34_compat *loader,
    redmcsb_f0757_texts_pc34_compat *texts);

const char *redmcsb_f0757_load_texts_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef FIRESTAFF_REDMCSB_F1001_JAPANESE_LOAD_ANK_CHARACTER_PATTERNS_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1001_JAPANESE_LOAD_ANK_CHARACTER_PATTERNS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F1001_ANK_CHARACTER_COUNT = 256u,
    REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES = 16u,
    REDMCSB_F1001_ANK_SEGMENT_BYTES = 4096u,
    REDMCSB_F1001_TIMER_INTERRUPT = 0x0au
};

typedef void (*redmcsb_f1001_port_write_pc34_compat)(
    void *context, uint16_t port, uint8_t value);
typedef uint8_t (*redmcsb_f1001_port_read_pc34_compat)(
    void *context, uint16_t port);
typedef void (*redmcsb_f1001_callback_pc34_compat)(void *context);

/* Models the P20JA branch's pushf/cli and popf around one glyph transfer. */
typedef void (*redmcsb_f1001_critical_section_pc34_compat)(void *context);

/* Represents the DOS interrupt vector's far target on the host. */
typedef void (*redmcsb_f1001_interrupt_handler_pc34_compat)(void *context);

typedef void (*redmcsb_f1001_get_interrupt_vector_pc34_compat)(
    void *context,
    uint8_t interrupt_number,
    redmcsb_f1001_interrupt_handler_pc34_compat *handler,
    void **handler_context);
typedef void (*redmcsb_f1001_set_interrupt_vector_pc34_compat)(
    void *context,
    uint8_t interrupt_number,
    redmcsb_f1001_interrupt_handler_pc34_compat handler,
    void *handler_context);

typedef struct {
    redmcsb_f1001_port_write_pc34_compat port_write;
    redmcsb_f1001_port_read_pc34_compat port_read;
    redmcsb_f1001_callback_pc34_compat wait_vertical_blank;
    redmcsb_f1001_critical_section_pc34_compat enter_critical_section;
    redmcsb_f1001_critical_section_pc34_compat leave_critical_section;
    redmcsb_f1001_get_interrupt_vector_pc34_compat get_interrupt_vector;
    redmcsb_f1001_set_interrupt_vector_pc34_compat set_interrupt_vector;
    redmcsb_f1001_callback_pc34_compat wait_for_interrupt;
} redmcsb_f1001_japanese_io_pc34_compat;

/*
 * ReDMCSB JAPANESE.C:103-132, MEDIA457_P20JA. Loads all 256 ANK glyphs
 * after one host-delivered vertical blank each. The supplied buffer models
 * the original A100h segment; no host timing or glyph values are invented.
 */
void redmcsb_f1001_japanese_load_ank_character_patterns_p20ja_pc34_compat(
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES],
    const redmcsb_f1001_japanese_io_pc34_compat *io,
    void *context);

/*
 * ReDMCSB JAPANESE.C:134-188, MEDIA469_P20JB. Temporarily replaces DOS
 * interrupt 0x0A, loads one glyph at each host-delivered interrupt, chains
 * the former vector, and restores it once all 256 glyphs have been loaded.
 */
void redmcsb_f1001_japanese_load_ank_character_patterns_p20jb_pc34_compat(
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES],
    const redmcsb_f1001_japanese_io_pc34_compat *io,
    void *context);

const char *redmcsb_f1001_japanese_load_ank_character_patterns_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1001_JAPANESE_LOAD_ANK_CHARACTER_PATTERNS_PC34_COMPAT_H */

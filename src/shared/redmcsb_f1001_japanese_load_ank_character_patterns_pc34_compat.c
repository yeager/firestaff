#include "redmcsb_f1001_japanese_load_ank_character_patterns_pc34_compat.h"

#include <stddef.h>

#define REDMCSB_F1001_CHARACTER_CODE_HIGH_PORT UINT16_C(0x00a3)
#define REDMCSB_F1001_CHARACTER_CODE_LOW_PORT UINT16_C(0x00a1)
#define REDMCSB_F1001_CHARACTER_PATTERN_ADDRESS_PORT UINT16_C(0x00a5)
#define REDMCSB_F1001_CHARACTER_PATTERN_DATA_PORT UINT16_C(0x00a9)

typedef struct {
    uint8_t *ank_segment;
    const redmcsb_f1001_japanese_io_pc34_compat *io;
    void *io_context;
    redmcsb_f1001_interrupt_handler_pc34_compat previous_handler;
    void *previous_handler_context;
    unsigned int character_index;
} redmcsb_f1001_p20jb_state_pc34_compat;

static void redmcsb_f1001_load_character_pattern(
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES],
    unsigned int character_index,
    const redmcsb_f1001_japanese_io_pc34_compat *io,
    void *context)
{
    unsigned int row;
    unsigned int offset =
        character_index * REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES;

    io->port_write(context, REDMCSB_F1001_CHARACTER_CODE_HIGH_PORT,
                   (uint8_t)character_index);
    io->port_write(context, REDMCSB_F1001_CHARACTER_CODE_LOW_PORT, 0u);
    for (row = 0u; row < REDMCSB_F1001_ANK_CHARACTER_PATTERN_BYTES; ++row) {
        io->port_write(context, REDMCSB_F1001_CHARACTER_PATTERN_ADDRESS_PORT,
                       (uint8_t)row);
        ank_segment[offset + row] = io->port_read(
            context, REDMCSB_F1001_CHARACTER_PATTERN_DATA_PORT);
    }
}

static void redmcsb_f1001_p20jb_interrupt(void *context)
{
    redmcsb_f1001_p20jb_state_pc34_compat *state = context;

    if (state->character_index < REDMCSB_F1001_ANK_CHARACTER_COUNT) {
        redmcsb_f1001_load_character_pattern(
            state->ank_segment, state->character_index, state->io,
            state->io_context);
        ++state->character_index;
    }
    state->previous_handler(state->previous_handler_context);
}

void redmcsb_f1001_japanese_load_ank_character_patterns_p20ja_pc34_compat(
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES],
    const redmcsb_f1001_japanese_io_pc34_compat *io,
    void *context)
{
    unsigned int character_index;

    for (character_index = 0u;
         character_index < REDMCSB_F1001_ANK_CHARACTER_COUNT;
         ++character_index) {
        io->wait_vertical_blank(context);
        io->enter_critical_section(context);
        redmcsb_f1001_load_character_pattern(ank_segment, character_index,
                                             io, context);
        io->leave_critical_section(context);
    }
}

void redmcsb_f1001_japanese_load_ank_character_patterns_p20jb_pc34_compat(
    uint8_t ank_segment[REDMCSB_F1001_ANK_SEGMENT_BYTES],
    const redmcsb_f1001_japanese_io_pc34_compat *io,
    void *context)
{
    redmcsb_f1001_p20jb_state_pc34_compat state = {
        ank_segment, io, context, NULL, NULL, 0u
    };

    io->get_interrupt_vector(context, REDMCSB_F1001_TIMER_INTERRUPT,
                             &state.previous_handler,
                             &state.previous_handler_context);
    io->set_interrupt_vector(context, REDMCSB_F1001_TIMER_INTERRUPT,
                             redmcsb_f1001_p20jb_interrupt, &state);
    while (state.character_index < REDMCSB_F1001_ANK_CHARACTER_COUNT) {
        io->wait_for_interrupt(context);
    }
    io->set_interrupt_vector(context, REDMCSB_F1001_TIMER_INTERRUPT,
                             state.previous_handler,
                             state.previous_handler_context);
}

const char *redmcsb_f1001_japanese_load_ank_character_patterns_source_evidence_pc34(
    void)
{
    return "ReDMCSB JAPANESE.C:97-188 defines "
           "F1001_JAPANESE_LoadANKCharacterPatterns. MEDIA457_P20JA "
           "(lines 103-132) calls F0693_WaitVerticalBlank for each of 256 "
           "characters, writes its index and zero to ports 0xA3/0xA1, then "
           "reads 16 bytes selected through ports 0xA5/0xA9 into A100h. "
           "MEDIA469_P20JB (lines 134-188) installs interrupt vector 0x0A, "
           "loads one character per interrupt, chains the former vector, "
           "and restores that vector after index 0x100.";
}

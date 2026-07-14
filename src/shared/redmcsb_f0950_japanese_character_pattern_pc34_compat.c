#include "redmcsb_f0950_japanese_character_pattern_pc34_compat.h"

#define REDMCSB_F0950_DISPLAY_MODE_PORT UINT16_C(0x0068)
#define REDMCSB_F0950_CHARACTER_CODE_HIGH_PORT UINT16_C(0x00A3)
#define REDMCSB_F0950_CHARACTER_CODE_LOW_PORT UINT16_C(0x00A1)
#define REDMCSB_F0950_CHARACTER_PATTERN_ADDRESS_PORT UINT16_C(0x00A5)
#define REDMCSB_F0950_CHARACTER_PATTERN_DATA_PORT UINT16_C(0x00A9)

void redmcsb_f0950_japanese_character_pattern_pc34_compat(
    int16_t character_code,
    uint8_t character_pattern[
        REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_BYTE_COUNT],
    const redmcsb_f0950_japanese_io_pc34_compat *io,
    void *context)
{
    uint16_t code;
    uint8_t row;

    io->port_write(context, REDMCSB_F0950_DISPLAY_MODE_PORT, UINT8_C(0x0B));

    code = (uint16_t)character_code;
    io->port_write(context, REDMCSB_F0950_CHARACTER_CODE_HIGH_PORT,
                   (uint8_t)((code >> 8) - UINT8_C(0x20)));
    io->port_write(context, REDMCSB_F0950_CHARACTER_CODE_LOW_PORT,
                   (uint8_t)code);

    for (row = 0U; row < UINT8_C(16); ++row) {
        io->enter_critical_section(context);
        io->port_write(context, REDMCSB_F0950_CHARACTER_PATTERN_ADDRESS_PORT,
                       (uint8_t)(row | UINT8_C(0x20)));
        character_pattern[(uint8_t)(row * UINT8_C(2))] =
            io->port_read(context, REDMCSB_F0950_CHARACTER_PATTERN_DATA_PORT);
        io->leave_critical_section(context);

        io->enter_critical_section(context);
        io->port_write(context, REDMCSB_F0950_CHARACTER_PATTERN_ADDRESS_PORT,
                       row);
        character_pattern[(uint8_t)(row * UINT8_C(2) + UINT8_C(1))] =
            io->port_read(context, REDMCSB_F0950_CHARACTER_PATTERN_DATA_PORT);
        io->leave_critical_section(context);
    }

    io->port_write(context, REDMCSB_F0950_DISPLAY_MODE_PORT, UINT8_C(0x0A));
}

const char *redmcsb_f0950_japanese_character_pattern_source_evidence_pc34(void)
{
    return "ReDMCSB JAPANESE.C:36-74 defines F0950_JAPANESE_: it writes "
           "0x0B then 0x0A to port 0x68, sends the swapped character-code "
           "bytes (high minus 0x20, then low) to 0xA3 and 0xA1, and for "
           "each of 16 rows reads the 0x20-selected then unselected byte "
           "through ports 0xA5 and 0xA9 under pushf/cli/popf protection.";
}

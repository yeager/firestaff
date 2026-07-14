#ifndef FIRESTAFF_REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_BYTE_COUNT 32U

typedef void (*redmcsb_f0950_port_write_pc34_compat)(
    void *context,
    uint16_t port,
    uint8_t value);

typedef uint8_t (*redmcsb_f0950_port_read_pc34_compat)(
    void *context,
    uint16_t port);

/* Models the source pushf/cli and popf pairs around each address/read. */
typedef void (*redmcsb_f0950_critical_section_pc34_compat)(void *context);

typedef struct {
    redmcsb_f0950_port_write_pc34_compat port_write;
    redmcsb_f0950_port_read_pc34_compat port_read;
    redmcsb_f0950_critical_section_pc34_compat enter_critical_section;
    redmcsb_f0950_critical_section_pc34_compat leave_critical_section;
} redmcsb_f0950_japanese_io_pc34_compat;

/*
 * ReDMCSB JAPANESE.C F0950_JAPANESE_. The host I/O boundary and destination
 * buffer must be valid. The routine copies the 16 two-byte PC-98 glyph rows.
 */
void redmcsb_f0950_japanese_character_pattern_pc34_compat(
    int16_t character_code,
    uint8_t character_pattern[
        REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_BYTE_COUNT],
    const redmcsb_f0950_japanese_io_pc34_compat *io,
    void *context);

const char *redmcsb_f0950_japanese_character_pattern_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0950_JAPANESE_CHARACTER_PATTERN_PC34_COMPAT_H */

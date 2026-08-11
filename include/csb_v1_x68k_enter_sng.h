#ifndef FIRESTAFF_CSB_V1_X68K_ENTER_SNG_H
#define FIRESTAFF_CSB_V1_X68K_ENTER_SNG_H

#include <stddef.h>
#include <stdint.h>

/* Read-only Standard MIDI File receipt for the X68000 CSB entrance music.
 * ReDMCSB ENTRANCE.C opens ENTER.SNG, reads its exact byte length into a
 * temporary buffer, then passes that buffer to F0813_PlayMIDIMusic on the
 * X68000 / PC-98 routes. This module only validates and inventories those
 * source bytes; playback stays with the host MIDI owner. */

typedef struct {
    uint16_t format;
    uint16_t track_count;
    uint16_t ticks_per_quarter_note;
    uint32_t event_count;
    uint32_t channel_event_count;
    uint32_t meta_event_count;
    uint32_t sysex_event_count;
    uint32_t note_on_count;
    uint32_t tempo_event_count;
    uint32_t end_of_track_count;
    uint64_t longest_track_ticks;
} CSB_V1_X68kEnterSngReceipt;

/* Validate a complete format-1, metrical-time Standard MIDI File. The parser
 * accepts channel events, meta events, SysEx and legal channel running status;
 * malformed VLQs, truncated payloads, system-data misuse and tracks without a
 * terminal End-of-Track event are rejected. */
int csb_v1_x68k_enter_sng_probe(const uint8_t *bytes, size_t byte_count,
                                CSB_V1_X68kEnterSngReceipt *out_receipt);

/* Extract ENTER.SNG from a validated raw CSB X68000 HDM, then apply the same
 * MIDI receipt. The function owns all temporary allocation internally and
 * never exposes or writes original game bytes. */
int csb_v1_x68k_enter_sng_probe_hdm(const uint8_t *hdm, size_t hdm_size,
                                    CSB_V1_X68kEnterSngReceipt *out_receipt);

#endif /* FIRESTAFF_CSB_V1_X68K_ENTER_SNG_H */

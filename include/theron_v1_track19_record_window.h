#ifndef THERON_V1_TRACK19_RECORD_WINDOW_H
#define THERON_V1_TRACK19_RECORD_WINDOW_H

#include <stddef.h>
#include <stdint.h>
#include "theron_v1_track02_item_properties.h"

/* The first byte of the known 66-entry item-property table precedes the
 * historical opaque window by one byte. The table is byte-identical in the
 * US/JP Track 19 ISOs and is validated against the source-bound Track 02
 * records. The remaining window bytes stay opaque until their consumer is
 * proven by the original loader/disassembly. */
#define THERON_TRACK19_ITEM_PROPERTY_TABLE_US_OFFSET 0x0E951Du
#define THERON_TRACK19_ITEM_PROPERTY_TABLE_JP_OFFSET 0x0E955Du
#define THERON_TRACK19_ITEM_PROPERTY_TABLE_BYTES 396u
#define THERON_TRACK19_ITEM_PROPERTY_TABLE_COUNT 66u
#define THERON_TRACK19_ITEM_PROPERTY_RECORD_BYTES 6u

#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_US_OFFSET 0x0E951Eu
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_JP_OFFSET 0x0E955Eu
#define THERON_TRACK19_OPAQUE_RECORD_WINDOW_BYTES 502u

/* The same 32x27 startup envelope is present at this offset in the real US
 * and JP Track 19 ISOs and in the canonical retail Track 02 concatenation.
 * Its bytes are admitted as a provenance receipt only; no object, tile,
 * palette, or runtime dungeon meaning is inferred from the envelope. */
#define THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_OFFSET 0x5A9114u
#define THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_BYTES 0x36Cu
#define THERON_TRACK19_STARTUP_LEVEL_ENVELOPE_FNV1A 0x54FCE0A0u
#define THERON_TRACK19_STARTUP_LEVEL_WIDTH 32u
#define THERON_TRACK19_STARTUP_LEVEL_HEIGHT 27u
#define THERON_TRACK19_STARTUP_LEVEL_HEADER_BYTES 12u

typedef struct {
    size_t envelope_offset;
    size_t envelope_bytes;
    uint32_t envelope_fnv1a;
    uint16_t width;
    uint16_t height;
    uint16_t header_words[6];
    const uint8_t *payload;
    size_t payload_bytes;
    size_t nonzero_payload_bytes;
    uint32_t payload_fnv1a;
} Theron_Track19LevelEnvelope;

int theron_v1_track19_item_property_table_validate(
    const uint8_t *iso, size_t iso_size, int japanese_variant,
    size_t *out_offset, size_t *out_bytes);

/* Return one record only after the complete real Track 19 table has matched
 * the source-bound Track 02 property records. No inferred record is returned
 * for a partial, mutated, or unrecognised ISO. */
int theron_v1_track19_item_property_from_iso(
    const uint8_t *iso, size_t iso_size, int japanese_variant,
    unsigned int index, Theron_ItemPropertyRecord *out);

int theron_v1_track19_opaque_record_window_validate(
    const uint8_t *iso, size_t iso_size, int japanese_variant,
    size_t *out_offset, size_t *out_bytes);

int theron_v1_track19_startup_level_envelope_validate(
    const uint8_t *iso, size_t iso_size, size_t *out_offset,
    size_t *out_bytes, uint32_t *out_fnv1a);

/* Binds the real envelope's dimensions, six raw header words, and bounded
 * payload span. The payload remains borrowed/opaque; no tile or object
 * semantics are assigned here. */
int theron_v1_track19_startup_level_envelope_read(
    const uint8_t *iso, size_t iso_size,
    Theron_Track19LevelEnvelope *out);

#endif

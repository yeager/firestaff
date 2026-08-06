/* DM2 V1 Save/Load — SUPPRESS codec + slot manager
 *
 * Source lock:
 *   SKULL.ASM: _2066_#### save/load entry points
 *   docs/dm2_save_format.md — full save format + SUPPRESS codec
 *   docs/dm2_save_slots.md — 10 slots and authenticated DOS header shape
 *   docs/dm2_party_state.md — champion squad persistence, masks
 *
 * SUPPRESS is a bit-plane RLE codec used throughout the DM2 save file:
 *   every set bit in mask[i] selects the matching bit of data[i]
 * Encoded data is packed MSB-first into output bytes and consecutive save
 * sections share the same bit stream until SUPPRESS_FLUSH.
 */

#include "dm2_v1_save_load.h"
#include "dm2_v1_new_game.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#if !defined(_WIN32)
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#define DM2_SK_CORPUS_RECURSE_CANDIDATE_CAP 64u
#define DM2_SK_CORPUS_RECURSE_DEPTH 4

/* This is the retired 56-byte D2RS diagnostic envelope, not SKProject's
 * 60-byte s_savegamebuffer. Keep the diagnostic codec byte-exact so its
 * fixtures remain deterministic; raw SKSave uses the separate source reader. */
_Static_assert(sizeof(DM2_GameStateBlock) == DM2_GAME_STATE_BLOCK_SIZE,
               "DM2_GameStateBlock must match D2RS diagnostic size");
_Static_assert(offsetof(DM2_GameStateBlock, wTimersCount) == 20,
               "DM2_GameStateBlock wTimersCount offset must match D2RS");
_Static_assert(offsetof(DM2_GameStateBlock, dw22) == 22,
               "DM2_GameStateBlock dw22 offset must match D2RS");
_Static_assert(offsetof(DM2_GameStateBlock, bRainStrength) == 44,
               "DM2_GameStateBlock rain strength must match D2RS");
_Static_assert(offsetof(DM2_GameStateBlock, dwRainSpecialNextTick) == 52,
               "DM2_GameStateBlock rain timer must match D2RS");

/* ════════════════════════════════════════════════════════════════
 * SUPPRESS codec
 * ════════════════════════════════════════════════════════════════ */

void dm2_suppress_writer_init(DM2_SuppressWriter *writer)
{
    if (writer) memset(writer, 0, sizeof(*writer));
}

int dm2_suppress_writer_write(DM2_SuppressWriter *writer,
                              const uint8_t *data, const uint8_t *mask,
                              size_t count, uint8_t *out,
                              size_t out_capacity, size_t *out_size)
{
    size_t out_pos = 0;

    if (!writer || !data || !mask || !out || !out_size) return -1;
    for (size_t i = 0; i < count; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            uint8_t bit_mask = (uint8_t)(1u << bit);
            if ((mask[i] & bit_mask) == 0u) continue;
            writer->pending_byte = (uint8_t)((writer->pending_byte << 1) |
                                              ((data[i] & bit_mask) != 0u));
            if (++writer->pending_bits == 8u) {
                if (out_pos >= out_capacity) return -1;
                out[out_pos++] = writer->pending_byte;
                writer->pending_byte = 0;
                writer->pending_bits = 0;
            }
        }
    }
    *out_size = out_pos;
    return 0;
}

int dm2_suppress_writer_write_bit(DM2_SuppressWriter *writer,
                                  int bit_value,
                                  uint8_t *out, size_t out_capacity,
                                  size_t *out_size)
{
    uint8_t data_byte = (uint8_t)(bit_value & 1);
    uint8_t mask_byte = 1;
    return dm2_suppress_writer_write(writer, &data_byte, &mask_byte,
                                     1, out, out_capacity, out_size);
}

int dm2_suppress_writer_flush(DM2_SuppressWriter *writer,
                              uint8_t *out, size_t out_capacity,
                              size_t *out_size)
{
    if (!writer || !out || !out_size) return -1;
    *out_size = 0;
    if (writer->pending_bits == 0u) return 0;
    if (out_capacity == 0u) return -1;
    out[0] = (uint8_t)(writer->pending_byte << (8u - writer->pending_bits));
    writer->pending_byte = 0;
    writer->pending_bits = 0;
    *out_size = 1;
    return 0;
}

void dm2_suppress_reader_init(DM2_SuppressReader *reader,
                              const uint8_t *data, size_t size)
{
    if (!reader) return;
    reader->data = data;
    reader->size = size;
    reader->position = 0;
    reader->current_byte = 0;
    reader->bits_remaining = 0;
}

int dm2_suppress_reader_read(DM2_SuppressReader *reader,
                             const uint8_t *mask, size_t count,
                             uint8_t *out, uint8_t fill)
{
    if (!reader || !mask || !out) return -1;
    memset(out, fill ? 0xFF : 0x00, count);
    for (size_t i = 0; i < count; ++i) {
        for (int bit = 7; bit >= 0; --bit) {
            uint8_t bit_mask = (uint8_t)(1u << bit);
            if ((mask[i] & bit_mask) == 0u) continue;
            if (reader->bits_remaining == 0u) {
                if (!reader->data || reader->position >= reader->size) return -1;
                reader->current_byte = reader->data[reader->position++];
                reader->bits_remaining = 8;
            }
            if ((reader->current_byte & 0x80u) != 0u) out[i] |= bit_mask;
            else out[i] &= (uint8_t)~bit_mask;
            reader->current_byte <<= 1;
            --reader->bits_remaining;
        }
    }
    return 0;
}

int dm2_suppress_reader_read_bit(DM2_SuppressReader *reader, int *out_bit)
{
    uint8_t data_byte = 0;
    uint8_t mask_byte = 1;
    if (!out_bit) return -1;
    if (dm2_suppress_reader_read(reader, &mask_byte, 1, &data_byte, 0) != 0)
        return -1;
    *out_bit = data_byte & 1;
    return 0;
}

int dm2_suppress_encode(const uint8_t *data, const uint8_t *mask,
                        size_t count, uint8_t *out, size_t out_capacity)
{
    DM2_SuppressWriter writer;
    size_t written = 0;
    size_t flushed = 0;

    dm2_suppress_writer_init(&writer);
    if (dm2_suppress_writer_write(&writer, data, mask, count, out,
                                  out_capacity, &written) != 0 ||
        dm2_suppress_writer_flush(&writer, out + written,
                                  out_capacity - written, &flushed) != 0) {
        return -1;
    }
    return (int)(written + flushed);
}

int dm2_suppress_decode(const uint8_t *in, size_t in_capacity,
                        const uint8_t *mask, size_t count,
                        uint8_t *out, uint8_t fill)
{
    DM2_SuppressReader reader;
    if (!in || !mask || !out) return -1;
    dm2_suppress_reader_init(&reader, in, in_capacity);
    if (dm2_suppress_reader_read(&reader, mask, count, out, fill) != 0) return -1;
    return (int)reader.position;
}

static uint32_t dm2_v1_save_hash_bytes(uint32_t hash,
                                       const uint8_t *bytes,
                                       size_t size)
{
    size_t i;

    if (!bytes && size != 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static uint8_t dm2_v1_save_u8_hash(const uint8_t *bytes, size_t size)
{
    uint32_t hash = dm2_v1_save_hash_bytes(2166136261u, bytes, size);
    hash ^= hash >> 16;
    hash ^= hash >> 8;
    return (uint8_t)(hash ? hash : 1u);
}

int dm2_v1_save_suppress_symbol_receipt(
    DM2_V1_SaveSuppressSymbolReceipt *out_receipt)
{
    const uint8_t source[3] = { 0x81u, 0x00u, 0xD2u };
    const uint8_t mask[3] = { 0x81u, 0x42u, 0xFFu };
    const uint8_t expected[2] = { 0xCDu, 0x20u };
    uint8_t encoded[8] = { 0u };
    uint8_t decoded[3] = { 0u };
    uint8_t decoded_fill[3] = { 0u };
    uint8_t truncated[2] = { 0xCDu, 0x00u };
    uint8_t first_data = 0x80u;
    uint8_t first_mask = 0xC0u;
    uint8_t first_out = 0u;
    uint8_t second_data = 0x0Fu;
    uint8_t second_mask = 0x0Fu;
    uint8_t second_out = 0u;
    DM2_SuppressWriter writer;
    DM2_SuppressReader reader;
    size_t written = 0u;
    size_t flushed = 0u;
    int encoded_size;
    uint32_t hash;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt) return 0;

    dm2_suppress_writer_init(&writer);
    out_receipt->init_ready =
        writer.pending_byte == 0u && writer.pending_bits == 0u;
    encoded_size = dm2_suppress_encode(
        source, mask, sizeof(source), encoded, sizeof(encoded));
    out_receipt->writer_ready =
        encoded_size == (int)sizeof(expected) &&
        memcmp(encoded, expected, sizeof(expected)) == 0;
    out_receipt->flush_ready = out_receipt->writer_ready;
    out_receipt->encoded_size =
        encoded_size > 0 ? (size_t)encoded_size : 0u;

    dm2_suppress_reader_init(&reader, encoded, sizeof(expected));
    out_receipt->reader_ready =
        reader.data == encoded && reader.size == sizeof(expected) &&
        reader.position == 0u && reader.bits_remaining == 0u &&
        dm2_suppress_reader_read(
            &reader, mask, sizeof(mask), decoded, 0u) == 0 &&
        memcmp(decoded, source, sizeof(source)) == 0;
    out_receipt->reader_position_after_decode = reader.position;

    dm2_suppress_writer_init(&writer);
    out_receipt->write_1bit_ready =
        dm2_suppress_writer_write(&writer, &first_data, &first_mask, 1u,
                                  encoded, sizeof(encoded), &written) == 0 &&
        written == 0u && writer.pending_bits == 2u &&
        dm2_suppress_writer_write(&writer, &second_data, &second_mask, 1u,
                                  encoded, sizeof(encoded), &written) == 0 &&
        written == 0u && writer.pending_bits == 6u &&
        dm2_suppress_writer_flush(&writer, encoded, sizeof(encoded),
                                  &flushed) == 0 &&
        flushed == 1u && encoded[0] == 0xBCu &&
        writer.pending_bits == 0u && writer.pending_byte == 0u;
    out_receipt->carry_encoded_byte = encoded[0];

    dm2_suppress_reader_init(&reader, encoded, 1u);
    out_receipt->read_1bit_ready =
        dm2_suppress_reader_read(
            &reader, &first_mask, 1u, &first_out, 0u) == 0 &&
        dm2_suppress_reader_read(
            &reader, &second_mask, 1u, &second_out, 0u) == 0 &&
        first_out == first_data && second_out == second_data &&
        reader.position == 1u;
    out_receipt->first_section_decoded = first_out;
    out_receipt->second_section_decoded = second_out;
    out_receipt->section_carry_ready =
        out_receipt->write_1bit_ready && out_receipt->read_1bit_ready;

    memset(decoded, 0xAA, sizeof(decoded));
    memset(decoded_fill, 0xAA, sizeof(decoded_fill));
    out_receipt->fill_zero_ready =
        dm2_suppress_decode(expected, sizeof(expected), mask, sizeof(mask),
                            decoded, 0u) == (int)sizeof(expected) &&
        decoded[1] == 0x00u;
    out_receipt->fill_one_ready =
        dm2_suppress_decode(expected, sizeof(expected), mask, sizeof(mask),
                            decoded_fill, 1u) == (int)sizeof(expected) &&
        decoded_fill[1] == 0xBDu;
    out_receipt->underflow_rejected =
        dm2_suppress_decode(truncated, 1u, mask, sizeof(mask), decoded,
                            0u) < 0;

    out_receipt->source_vector_hash =
        dm2_v1_save_u8_hash(source, sizeof(source));
    out_receipt->mask_vector_hash =
        dm2_v1_save_u8_hash(mask, sizeof(mask));
    out_receipt->encoded_vector_hash =
        dm2_v1_save_u8_hash(expected, sizeof(expected));
    out_receipt->decoded_vector_hash =
        dm2_v1_save_u8_hash(decoded, sizeof(decoded));
    out_receipt->covered_symbol_mask =
        DM2_V1_SAVE_SUPPRESS_SYMBOL_INIT |
        DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITER |
        DM2_V1_SAVE_SUPPRESS_SYMBOL_FLUSH |
        DM2_V1_SAVE_SUPPRESS_SYMBOL_READER |
        DM2_V1_SAVE_SUPPRESS_SYMBOL_WRITE_1BIT |
        DM2_V1_SAVE_SUPPRESS_SYMBOL_READ_1BIT;

    hash = 2166136261u;
    hash = dm2_v1_save_hash_bytes(
        hash, (const uint8_t *)&out_receipt->covered_symbol_mask,
        sizeof(out_receipt->covered_symbol_mask));
    hash = dm2_v1_save_hash_bytes(hash, source, sizeof(source));
    hash = dm2_v1_save_hash_bytes(hash, mask, sizeof(mask));
    hash = dm2_v1_save_hash_bytes(hash, expected, sizeof(expected));
    hash = dm2_v1_save_hash_bytes(
        hash, &out_receipt->carry_encoded_byte,
        sizeof(out_receipt->carry_encoded_byte));
    hash = dm2_v1_save_hash_bytes(
        hash, &out_receipt->first_section_decoded,
        sizeof(out_receipt->first_section_decoded));
    hash = dm2_v1_save_hash_bytes(
        hash, &out_receipt->second_section_decoded,
        sizeof(out_receipt->second_section_decoded));
    out_receipt->receipt_hash = hash ? hash : 1u;
    out_receipt->valid =
        out_receipt->init_ready &&
        out_receipt->writer_ready &&
        out_receipt->flush_ready &&
        out_receipt->reader_ready &&
        out_receipt->write_1bit_ready &&
        out_receipt->read_1bit_ready &&
        out_receipt->section_carry_ready &&
        out_receipt->fill_zero_ready &&
        out_receipt->fill_one_ready &&
        out_receipt->underflow_rejected &&
        out_receipt->source_vector_hash != 0u &&
        out_receipt->mask_vector_hash != 0u &&
        out_receipt->encoded_vector_hash != 0u &&
        out_receipt->decoded_vector_hash != 0u &&
        out_receipt->receipt_hash != 0u;
    return out_receipt->valid;
}

/* ════════════════════════════════════════════════════════════════
 * Slot manager
 * ════════════════════════════════════════════════════════════════ */

static bool dm2_sl_header_valid(const uint8_t hdr[42])
{
    size_t i;

    /* Original DOS SKSaveNN files use a 42-byte sksave_header_asc record,
     * whose c_hex2a::l_26 field starts at b38. A missing slot is represented
     * in the in-memory dialog list by 0xdeadbeef, not by an on-disk file.
     * The mounted PC corpus has a little-endian version word 1 and a bounded
     * printable save label at b2. DM2_READ_SKSAVE_DUNGEON owns the bytes
     * immediately after this record, so this is only the container gate; the
     * raw dungeon/SUPPRESS parser remains mandatory before a candidate
     * becomes loadable.
     *
     * SKProject: SKULLWIN/dm2data.h::c_hex2a; c_dialog.cpp:115-117,
     * 199-202, 337-343; c_savegame.cpp:2169-2181. */
    if (hdr[0] != 1u || hdr[1] != 0u) return false;
    if (hdr[38] == 0xefu && hdr[39] == 0xbeu &&
        hdr[40] == 0xadu && hdr[41] == 0xdeu) return false;
    for (i = 2u; i < 38u; ++i) {
        if (hdr[i] == 0u) return i != 2u;
        if (hdr[i] < 0x20u || hdr[i] > 0x7eu) return false;
    }
    return false;
}

static void dm2_sl_build_source_header(uint8_t hdr[42], const char *name,
                                       const char *previous_path)
{
    uint8_t previous[42];
    FILE *previous_file;
    size_t nlen;

    memset(hdr, 0, 42u);
    /* SKProject c_savegame.cpp:2169-2181 reads the previous 42-byte header
     * after rotation, retains c_hex2a::l_26, then replaces only w_00/name. */
    if (previous_path &&
        (previous_file = fopen(previous_path, "rb")) != NULL) {
        if (fread(previous, sizeof(previous), 1u, previous_file) == 1u &&
            dm2_sl_header_valid(previous)) {
            memcpy(hdr + 38u, previous + 38u, 4u);
        }
        fclose(previous_file);
    }
    hdr[0] = 1u;
    if (!name) return;
    nlen = strlen(name);
    if (nlen > 35u) nlen = 35u;
    memcpy(hdr + 2u, name, nlen);
}

static FILE *dm2_sl_open_valid_payload(const char *path, int *status)
{
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (status) *status = -2;
        return NULL;
    }

    uint8_t hdr[42];
    if (fread(hdr, 42, 1, f) != 1) {
        fclose(f);
        if (status) *status = -4;
        return NULL;
    }

    /* ReDMCSB LOADSAVE.C F0435 lines 2665-2671 validates the save header
     * before reading payload parts. Keep the authenticated DOS header shape
     * as a container gate only; a full parser still decides loadability. */
    if (!dm2_sl_header_valid(hdr)) {
        fclose(f);
        if (status) *status = -5;
        return NULL;
    }

    if (status) *status = 0;
    return f;
}

typedef enum {
    DM2_SK_CORPUS_MISSING = 0,
    DM2_SK_CORPUS_INVALID = 1,
    DM2_SK_CORPUS_VALID = 2,
} DM2_SKCorpusProbeStatus;

static DM2_SKCorpusProbeStatus dm2_sksave_probe_path(const char *path,
                                                      size_t *out_payload_size)
{
    FILE *f;
    uint8_t hdr[42];
    long end_pos;

    if (out_payload_size) *out_payload_size = 0;
    if (!path || !path[0]) return DM2_SK_CORPUS_MISSING;
    f = fopen(path, "rb");
    if (!f) return DM2_SK_CORPUS_MISSING;
    if (fread(hdr, sizeof(hdr), 1, f) != 1) {
        fclose(f);
        return DM2_SK_CORPUS_INVALID;
    }
    if (!dm2_sl_header_valid(hdr)) {
        fclose(f);
        return DM2_SK_CORPUS_INVALID;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return DM2_SK_CORPUS_INVALID;
    }
    end_pos = ftell(f);
    fclose(f);
    if (end_pos < (long)sizeof(hdr)) return DM2_SK_CORPUS_INVALID;
    if (out_payload_size) {
        *out_payload_size = (size_t)end_pos - sizeof(hdr);
    }
    return DM2_SK_CORPUS_VALID;
}

static void dm2_sksave_corpus_accept(DM2_SKSaveCorpusReceipt *receipt,
                                     const char *path,
                                     size_t payload_size)
{
    if (!receipt) return;
    if (receipt->first_valid_path[0] == '\0' && path) {
        snprintf(receipt->first_valid_path,
                 sizeof(receipt->first_valid_path),
                 "%s", path);
    }
    receipt->total_payload_size += payload_size;
    if (payload_size > receipt->largest_payload_size) {
        receipt->largest_payload_size = payload_size;
    }
}

static uint32_t dm2_sksave_corpus_hash_step(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t dm2_sksave_corpus_payload_hash(const uint8_t *payload,
                                               size_t payload_size,
                                               uint32_t seed)
{
    uint32_t hash = seed ? seed : 0x32434f52u;
    size_t i;
    if (!payload || payload_size == 0u) {
        return hash;
    }
    for (i = 0; i < payload_size; ++i) {
        hash = dm2_sksave_corpus_hash_step(hash, payload[i]);
    }
    return hash;
}

static uint32_t dm2_sksave_corpus_words_hash(const uint16_t *words,
                                             size_t word_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!words) return 0u;
    for (i = 0u; i < word_count; ++i) {
        hash = dm2_sksave_corpus_hash_step(hash, words[i] & 0xffu);
        hash = dm2_sksave_corpus_hash_step(hash, words[i] >> 8);
    }
    return hash;
}

static int dm2_sksave_ascii_lower(int c)
{
    return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

static int dm2_sksave_ascii_equal_ci(const char *a, const char *b)
{
    if (!a || !b) return 0;
    while (*a && *b) {
        if (dm2_sksave_ascii_lower((unsigned char)*a) !=
            dm2_sksave_ascii_lower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

static int dm2_sksave_basename_is_canonical_direct(const char *name)
{
    if (!name) return 0;
    if (strcmp(name, "SKSave.dat") == 0 ||
        strcmp(name, "SKSave.bak") == 0) {
        return 1;
    }
    if (strncmp(name, "SKSave", 6) == 0 &&
        name[6] >= '0' && name[6] <= '9' &&
        name[7] >= '0' && name[7] <= '9' &&
        strcmp(name + 8, ".dat") == 0) {
        return 1;
    }
    return 0;
}

/* Original PC-DOS media commonly spells the four supplied slot saves as
 * lower-case, unpadded `sksave0.dat` … `sksave3.dat`.  SKProject's source
 * constructs the title-cased, two-digit path for its own output, but its
 * file-format gate is independent of that spelling.  Accept only the same
 * SKSave stem, a slot in the source's ten-slot range, and .dat/.bak here;
 * arbitrary header-shaped files remain recursive corpus candidates only. */
static int dm2_sksave_basename_matches_variant(const char *name,
                                               int *out_last_session,
                                               int *out_slot,
                                               int *out_backup)
{
    const char *suffix;
    unsigned int slot = 0u;
    unsigned int digits = 0u;

    if (!name || strlen(name) < 7u ||
        dm2_sksave_ascii_lower((unsigned char)name[0]) != 's' ||
        dm2_sksave_ascii_lower((unsigned char)name[1]) != 'k' ||
        dm2_sksave_ascii_lower((unsigned char)name[2]) != 's' ||
        dm2_sksave_ascii_lower((unsigned char)name[3]) != 'a' ||
        dm2_sksave_ascii_lower((unsigned char)name[4]) != 'v' ||
        dm2_sksave_ascii_lower((unsigned char)name[5]) != 'e') {
        return 0;
    }
    if (dm2_sksave_ascii_equal_ci(name, "SKSave.dat")) {
        if (out_last_session) *out_last_session = 1;
        if (out_slot) *out_slot = -1;
        if (out_backup) *out_backup = 0;
        return 1;
    }
    if (dm2_sksave_ascii_equal_ci(name, "SKSave.bak")) {
        if (out_last_session) *out_last_session = 1;
        if (out_slot) *out_slot = -1;
        if (out_backup) *out_backup = 1;
        return 1;
    }
    while (digits < 2u && name[6u + digits] >= '0' &&
           name[6u + digits] <= '9') {
        slot = slot * 10u + (unsigned int)(name[6u + digits] - '0');
        ++digits;
    }
    if (digits == 0u || slot >= DM2_SLOT_MAX) return 0;
    suffix = name + 6u + digits;
    if (!dm2_sksave_ascii_equal_ci(suffix, ".dat") &&
        !dm2_sksave_ascii_equal_ci(suffix, ".bak")) {
        return 0;
    }
    if (out_last_session) *out_last_session = 0;
    if (out_slot) *out_slot = (int)slot;
    if (out_backup) *out_backup = dm2_sksave_ascii_equal_ci(suffix, ".bak");
    return 1;
}

static int dm2_sksave_basename_is_candidate_ci(const char *name)
{
    return dm2_sksave_basename_matches_variant(name, NULL, NULL, NULL);
}

static int dm2_sksave_root_variant_path(const char *save_base,
                                        int last_session,
                                        unsigned int slot,
                                        int backup,
                                        char *out_path,
                                        size_t out_path_size)
{
    const char *formats[4];
    unsigned int format_count;

    if (!save_base || !out_path || out_path_size == 0u ||
        (!last_session && slot >= DM2_SLOT_MAX)) {
        return 0;
    }
    if (last_session) {
        formats[0] = backup ? "SKSave.bak" : "SKSave.dat";
        formats[1] = backup ? "sksave.bak" : "sksave.dat";
        format_count = 2u;
    } else {
        formats[0] = backup ? "SKSave%02u.bak" : "SKSave%02u.dat";
        formats[1] = backup ? "SKSave%u.bak" : "SKSave%u.dat";
        formats[2] = backup ? "sksave%02u.bak" : "sksave%02u.dat";
        formats[3] = backup ? "sksave%u.bak" : "sksave%u.dat";
        format_count = 4u;
    }
    for (unsigned int i = 0u; i < format_count; ++i) {
        FILE *file;
        if (last_session) {
            snprintf(out_path, out_path_size, "%s/%s", save_base, formats[i]);
        } else {
            snprintf(out_path, out_path_size, "%s/", save_base);
            snprintf(out_path + strlen(out_path),
                     out_path_size - strlen(out_path), formats[i], slot);
        }
        file = fopen(out_path, "rb");
        if (file) {
            fclose(file);
            return 1;
        }
    }
    out_path[0] = '\0';
    return 0;
}

static uint32_t dm2_sksave_corpus_file_hash(const char *path)
{
    FILE *file;
    uint8_t buffer[4096];
    uint32_t hash = 2166136261u;
    size_t read_count;

    if (!path || !path[0] || !(file = fopen(path, "rb"))) {
        return 0u;
    }
    while ((read_count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        hash = dm2_sksave_corpus_payload_hash(buffer, read_count, hash);
    }
    if (ferror(file)) {
        fclose(file);
        return 0u;
    }
    fclose(file);
    return hash;
}

static void dm2_sksave_corpus_classify_payload(
    DM2_SKSaveCorpusReceipt *receipt,
    const char *path,
    size_t payload_size)
{
    FILE *f;
    uint8_t *payload;
    DM2_V1_SaveCandidate candidate;
    int status = 0;
    int importable_kind_ok = 0;
    DM2_SKSaveKind save_kind = DM2_SK_SAVE_KIND_NONE;
    uint32_t initial_file_hash;
    uint32_t source_file_hash;

    if (!receipt || !path || payload_size == 0u ||
        payload_size > (size_t)DM2_SESSION_MAX_SIZE) {
        if (receipt) receipt->import_rejected_candidate_count++;
        return;
    }

    initial_file_hash = dm2_sksave_corpus_file_hash(path);
    if (initial_file_hash == 0u) {
        receipt->import_rejected_candidate_count++;
        return;
    }
    f = dm2_sl_open_valid_payload(path, &status);
    if (!f) {
        receipt->import_rejected_candidate_count++;
        return;
    }
    payload = (uint8_t *)malloc(payload_size);
    if (!payload) {
        fclose(f);
        receipt->import_rejected_candidate_count++;
        return;
    }
    if (fread(payload, 1, payload_size, f) != payload_size ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                            payload_size) != 0) {
        free(payload);
        fclose(f);
        receipt->import_rejected_candidate_count++;
        return;
    }
    source_file_hash = dm2_sksave_corpus_file_hash(path);
    /* The census receipt binds one complete original file to the payload
     * parser. A file changed while being read must not publish a hybrid
     * header/file hash and payload observation. */
    if (source_file_hash == 0u || source_file_hash != initial_file_hash) {
        free(payload);
        fclose(f);
        receipt->import_rejected_candidate_count++;
        return;
    }
    switch (candidate.kind) {
        case DM2_V1_SAVE_CANDIDATE_FIRESTAFF_SESSION:
            receipt->firestaff_session_candidate_count++;
            /* D2RS is a Firestaff decoder fixture, not SKProject's
             * DM2_GAME_LOAD corpus format.  Keep its presence observable,
             * but never publish it as a loadable save candidate. */
            receipt->import_rejected_candidate_count++;
            break;
        case DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE:
            receipt->original_envelope_candidate_count++;
            save_kind = DM2_SK_SAVE_KIND_ORIGINAL_ENVELOPE;
            importable_kind_ok = 1;
            break;
        case DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW:
            receipt->original_raw_candidate_count++;
            save_kind = DM2_SK_SAVE_KIND_ORIGINAL_RAW;
            importable_kind_ok = 1;
            break;
        default:
            receipt->import_rejected_candidate_count++;
            receipt->importable_candidate_count--;
            break;
    }
    if (importable_kind_ok) {
        receipt->importable_candidate_count++;
        receipt->total_importable_payload_size += payload_size;
        if (payload_size > receipt->largest_importable_payload_size) {
            receipt->largest_importable_payload_size = payload_size;
        }
        if (receipt->first_importable_path[0] == '\0') {
            snprintf(receipt->first_importable_path,
                     sizeof(receipt->first_importable_path), "%s", path);
        }
        if (receipt->first_importable_kind == DM2_SK_SAVE_KIND_NONE) {
            receipt->first_importable_kind = save_kind;
            receipt->first_importable_payload_size = payload_size;
        }
        if (receipt->candidate_receipt_count < DM2_SK_CORPUS_RECEIPT_MAX) {
            DM2_SKSaveCandidateReceipt *entry =
                &receipt->candidate_receipts[receipt->candidate_receipt_count++];
            entry->kind = candidate.kind;
            entry->import_rejected = 0;
            entry->payload_size = payload_size;
            entry->payload_hash = dm2_sksave_corpus_payload_hash(
                payload, payload_size, 2166136261u);
            entry->source_file_hash = source_file_hash;
            snprintf(entry->path, sizeof(entry->path), "%s", path);
        }
        receipt->importable_kind_mask |=
            1u << ((unsigned int)candidate.kind & 31u);
        receipt->importable_payload_hash =
            dm2_sksave_corpus_payload_hash(
                payload,
                payload_size,
                receipt->importable_payload_hash
                    ? receipt->importable_payload_hash
                    : 0x32534b43u);
    }
    free(payload);
    fclose(f);
}

static void __attribute__((unused)) dm2_sksave_corpus_probe_candidate(
    DM2_SKSaveCorpusReceipt *receipt,
    const char *path,
    const char *basename,
    int recursive,
    int alternate_name)
{
    DM2_SKCorpusProbeStatus status;
    size_t payload_size = 0u;
    uint8_t before_importable;

    if (!receipt || !path) return;
    status = dm2_sksave_probe_path(path, &payload_size);
    if (status == DM2_SK_CORPUS_VALID) {
        if (recursive) receipt->recursive_candidate_count++;
        if (alternate_name) receipt->alternate_name_candidate_count++;
        dm2_sksave_corpus_accept(receipt, path, payload_size);
        before_importable = receipt->importable_candidate_count;
        dm2_sksave_corpus_classify_payload(receipt, path, payload_size);
        if (receipt->importable_candidate_count > before_importable) {
            if (recursive) receipt->recursive_importable_candidate_count++;
        }
        if (recursive || (basename && !dm2_sksave_basename_is_canonical_direct(basename))) {
            receipt->extra_valid_candidate_count++;
        }
    } else if (status == DM2_SK_CORPUS_INVALID) {
        if (recursive) receipt->recursive_candidate_count++;
        if (alternate_name) receipt->alternate_name_candidate_count++;
        receipt->invalid_candidate_count++;
    }
}

#if !defined(_WIN32)
static int dm2_sksave_is_dot_dir(const char *name)
{
    return name && (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

static void dm2_sksave_corpus_scan_recursive_impl(
    const char *root,
    const char *dir,
    int depth,
    DM2_SKSaveCorpusReceipt *receipt,
    unsigned int *candidate_count)
{
    DIR *d;
    struct dirent *ent;

    if (!root || !dir || !receipt || !candidate_count ||
        *candidate_count >= DM2_SK_CORPUS_RECURSE_CANDIDATE_CAP) {
        if (receipt && candidate_count &&
            *candidate_count >= DM2_SK_CORPUS_RECURSE_CANDIDATE_CAP) {
            receipt->recursive_scan_truncated = 1u;
        }
        return;
    }
    if (depth > DM2_SK_CORPUS_RECURSE_DEPTH) {
        receipt->recursive_scan_truncated = 1u;
        return;
    }
    d = opendir(dir);
    if (!d) return;
    while ((ent = readdir(d)) != NULL &&
           *candidate_count < DM2_SK_CORPUS_RECURSE_CANDIDATE_CAP) {
        char path[512];
        struct stat st;
        DM2_SKCorpusProbeStatus status;
        size_t payload_size = 0u;
        int is_root_exact_canonical;
        int alternate_name;
        int named_candidate;

        if (dm2_sksave_is_dot_dir(ent->d_name)) continue;
        snprintf(path, sizeof(path), "%s/%s", dir, ent->d_name);
        if (stat(path, &st) != 0) continue;
        if (S_ISDIR(st.st_mode)) {
            if (depth + 1 > DM2_SK_CORPUS_RECURSE_DEPTH) {
                receipt->recursive_scan_truncated = 1u;
            } else {
                dm2_sksave_corpus_scan_recursive_impl(root, path, depth + 1,
                                                      receipt, candidate_count);
            }
            continue;
        }
        if (!S_ISREG(st.st_mode)) {
            continue;
        }
        /* Root-level source spellings were already scanned in the direct
         * title/load order above. Recursive pass adds nested or renamed real
         * corpus files without double-counting the supplied DOS corpus. */
        is_root_exact_canonical =
            depth == 0 &&
            strcmp(dir, root) == 0 &&
            dm2_sksave_basename_is_candidate_ci(ent->d_name);
        if (is_root_exact_canonical) continue;
        named_candidate = dm2_sksave_basename_is_candidate_ci(ent->d_name);
        if (!named_candidate) {
            /* SKProject opens canonical names for live resume, but an
             * external original-save corpus is often archived or renamed.
             * Admit such a file only after the same header gate used by
             * DM2_GAME_LOAD and then the existing source-bound payload
             * parser.  Arbitrary files never consume the corpus cap. */
            status = dm2_sksave_probe_path(path, &payload_size);
            if (status != DM2_SK_CORPUS_VALID) {
                continue;
            }
            ++receipt->header_discovered_candidate_count;
        }
        alternate_name = !dm2_sksave_basename_is_canonical_direct(ent->d_name);
        (*candidate_count)++;
        dm2_sksave_corpus_probe_candidate(receipt, path, ent->d_name,
                                          1, alternate_name);
    }
    closedir(d);
}
#endif

static int dm2_sksave_read_valid_payload(const char *path,
                                         uint8_t *out_payload,
                                         size_t out_capacity,
                                         size_t *out_payload_size)
{
    FILE *f;
    int status = 0;
    long payload_start;
    long end_pos;
    size_t payload_size;

    if (out_payload_size) *out_payload_size = 0u;
    if (!path || !out_payload || out_capacity == 0u || !out_payload_size) {
        return -1;
    }
    f = dm2_sl_open_valid_payload(path, &status);
    if (!f) return -1;
    payload_start = ftell(f);
    if (payload_start < 0 || fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    end_pos = ftell(f);
    if (end_pos < payload_start) {
        fclose(f);
        return -1;
    }
    payload_size = (size_t)(end_pos - payload_start);
    if (payload_size == 0u || payload_size > out_capacity ||
        fseek(f, payload_start, SEEK_SET) != 0 ||
        fread(out_payload, 1u, payload_size, f) != payload_size) {
        fclose(f);
        return -1;
    }
    fclose(f);
    *out_payload_size = payload_size;
    return 0;
}

void dm2_sl_init(DM2_SL_State *state, const char *save_base)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    if (save_base && save_base[0]) {
        size_t len = strlen(save_base);
        if (len >= sizeof(state->save_base)) len = sizeof(state->save_base) - 1;
        memcpy(state->save_base, save_base, len);
        state->save_base[len] = '\0';
    } else {
        state->save_base[0] = '.';
        state->save_base[1] = '\0';
    }
    state->initialized = true;
}

bool dm2_sl_slot_occupied(const DM2_SL_State *state, uint8_t slot)
{
    if (!state || slot >= DM2_SLOT_MAX) return false;
    return state->slots[slot].occupied;
}

const char *dm2_sl_slot_name(const DM2_SL_State *state, uint8_t slot)
{
    if (!state || slot >= DM2_SLOT_MAX || !state->slots[slot].occupied)
        return NULL;
    return state->slots[slot].name;
}

bool dm2_sl_scan_slots(DM2_SL_State *state)
{
    if (!state || !state->initialized) return false;

    state->slot_count = 0;
    for (uint8_t i = 0; i < DM2_SLOT_MAX; i++) {
        char path[512];
        const char *base = state->save_base[0] ? state->save_base : ".";

        if (!dm2_sksave_root_variant_path(base, 0, i, 0, path,
                                          sizeof(path))) {
            state->slots[i].occupied = false;
            memset(state->slots[i].name, 0, sizeof(state->slots[i].name));
            state->slots[i].timestamp = 0;
            continue;
        }

        FILE *f = fopen(path, "rb");
        if (!f) continue;

        uint8_t hdr[42];
        if (fread(hdr, 42, 1, f) != 1) {
            fclose(f);
            state->slots[i].occupied = false;
            continue;
        }
        fclose(f);

        if (dm2_sl_header_valid(hdr)) {
            state->slots[i].occupied = true;
            /* Copy slot name (null-terminated, max 33 chars) */
            size_t j;
            for (j = 0; j < DM2_SLOT_NAME_MAX; j++) {
                state->slots[i].name[j] = hdr[2 + j];
                if (hdr[2 + j] == '\0') break;
            }
            state->slots[i].name[DM2_SLOT_NAME_MAX] = '\0';
            /* timestamp field not available from header; leave at 0 */
            state->slot_count++;
        } else {
            state->slots[i].occupied = false;
            memset(state->slots[i].name, 0, sizeof(state->slots[i].name));
        }
    }
    return true;
}

int dm2_sl_save(const char *save_base, uint8_t slot,
                 const char *name,
                 const uint8_t *data, size_t data_size)
{
    if (!save_base || !name || !name[0] || !data || data_size == 0) return -1;
    if (slot >= DM2_SLOT_MAX) return -1;

    char path_dat[256], path_bak[256];
    snprintf(path_dat, sizeof(path_dat), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    snprintf(path_bak, sizeof(path_bak), "%s/SKSave.bak", save_base);

    /* Rotate: existing dat → bak (backup); ignore if dat doesn't exist */
    (void)remove(path_bak);
    (void)rename(path_dat, path_bak);

    FILE *f = fopen(path_dat, "wb");
    if (!f) return -2;

    /* Build the source c_hex2a header. SKSaveNN naming is a Firestaff test
     * helper; the original header itself carries no fabricated slot number. */
    uint8_t hdr[42];
    dm2_sl_build_source_header(hdr, name, path_bak);

    if (fwrite(hdr, 42, 1, f) != 1) { fclose(f); return -3; }
    if (fwrite(data, 1, data_size, f) != data_size) { fclose(f); return -3; }
    fclose(f);
    return 0;
}

int dm2_sl_save_last_session(const char *save_base,
                             const char *name,
                             const uint8_t *data,
                             size_t data_size)
{
    if (!save_base || !name || !name[0] || !data || data_size == 0) return -1;

    char path_dat[256], path_bak[256];
    snprintf(path_dat, sizeof(path_dat), "%s/SKSave.dat", save_base);
    snprintf(path_bak, sizeof(path_bak), "%s/SKSave.bak", save_base);

    /* ReDMCSB/SKWin resume uses SKSave.dat as the direct last-session
     * target and rotates the previous primary to SKSave.bak before write. */
    (void)remove(path_bak);
    (void)rename(path_dat, path_bak);

    FILE *f = fopen(path_dat, "wb");
    if (!f) return -2;

    uint8_t hdr[42];
    dm2_sl_build_source_header(hdr, name, path_bak);

    if (fwrite(hdr, 42, 1, f) != 1) { fclose(f); return -3; }
    if (fwrite(data, 1, data_size, f) != data_size) { fclose(f); return -3; }
    fclose(f);
    return 0;
}

int dm2_sl_load(const char *save_base, uint8_t slot,
                 uint8_t *data, size_t max_size, size_t *out_size)
{
    if (!save_base || !data || !out_size) return -1;
    if (slot >= DM2_SLOT_MAX) return -1;

    char path[256], bak[256];
    int has_primary = dm2_sksave_root_variant_path(
        save_base, 0, slot, 0, path, sizeof(path));

    int status = 0;
    FILE *f = has_primary ? dm2_sl_open_valid_payload(path, &status) : NULL;
    if (!f) {
        /* Try the observed same-slot backup first when the primary is
         * missing, truncated, or corrupt, then preserve the legacy generic
         * SKSave.bak fallback used by the source compatibility path.
         * ReDMCSB LOADSAVE.C F0435 lines 2560-2583 tries the backup save after
         * primary-open failure; Firestaff extends the same safety net to
         * invalid DM2 slot headers so runtime load never accepts stale data. */
        if (dm2_sksave_root_variant_path(save_base, 0, slot, 1, bak,
                                         sizeof(bak))) {
            f = dm2_sl_open_valid_payload(bak, &status);
        }
        if (!f && dm2_sksave_root_variant_path(save_base, 1, 0u, 1, bak,
                                               sizeof(bak))) {
            f = dm2_sl_open_valid_payload(bak, &status);
        }
        if (!f) return status ? status : -2;
    }

    size_t got = fread(data, 1, max_size, f);
    fclose(f);
    *out_size = got;
    return 0;
}

int dm2_sl_load_last_session(const char *save_base,
                             uint8_t *data,
                             size_t max_size,
                             size_t *out_size)
{
    if (!save_base || !data || !out_size) return -1;

    char path[256], bak[256];
    int has_primary = dm2_sksave_root_variant_path(
        save_base, 1, 0u, 0, path, sizeof(path));

    int status = 0;
    FILE *f = has_primary ? dm2_sl_open_valid_payload(path, &status) : NULL;
    if (!f) {
        /* SKWin/DM2 resume path: last-session primary first, backup second,
         * then caller may fall back to a fresh dungeon start. */
        if (dm2_sksave_root_variant_path(save_base, 1, 0u, 1, bak,
                                         sizeof(bak))) {
            f = dm2_sl_open_valid_payload(bak, &status);
        }
        if (!f) return status ? status : -2;
    }

    size_t got = fread(data, 1, max_size, f);
    fclose(f);
    *out_size = got;
    return 0;
}

int dm2_sl_delete(const char *save_base, uint8_t slot)
{
    if (!save_base || slot >= DM2_SLOT_MAX) return -1;
    char path[256], bak[256];
    snprintf(path, sizeof(path), "%s/SKSave%02u.dat", save_base, (unsigned)slot);
    snprintf(bak,  sizeof(bak),  "%s/SKSave.bak",   save_base);
    (void)remove(path);
    (void)remove(bak);
    return 0;
}

/* ════════════════════════════════════════════════════════════════
 * High-level public API
 * ════════════════════════════════════════════════════════════════ */

uint8_t dm2_v1_save_slot_count(void) { return DM2_SLOT_MAX; }

bool dm2_v1_save_slot_valid(uint8_t slot) { return slot < DM2_SLOT_MAX; }

bool dm2_v1_save_has_valid_slot(const char *save_base, uint8_t slot)
{
    if (!save_base) return false;
    char path[256];
    if (!dm2_sksave_root_variant_path(save_base, 0, slot, 0, path,
                                      sizeof(path))) {
        return false;
    }
    FILE *f = fopen(path, "rb");
    if (!f) return false;
    uint8_t hdr[42];
    bool ok = (fread(hdr, 42, 1, f) == 1) && dm2_sl_header_valid(hdr);
    fclose(f);
    return ok;
}

bool dm2_v1_save_has_valid_last_session(const char *save_base)
{
    int status = 0;
    FILE *f;
    char path[256];
    if (!save_base) return false;
    f = dm2_sksave_root_variant_path(save_base, 1, 0u, 0, path,
                                     sizeof(path))
        ? dm2_sl_open_valid_payload(path, &status) : NULL;
    if (!f) {
        f = dm2_sksave_root_variant_path(save_base, 1, 0u, 1, path,
                                         sizeof(path))
            ? dm2_sl_open_valid_payload(path, &status) : NULL;
    }
    if (!f) {
        return false;
    }
    fclose(f);
    return true;
}

bool dm2_v1_sksave_corpus_scan(const char *save_base,
                               DM2_SKSaveCorpusReceipt *out_receipt)
{
    DM2_SKCorpusProbeStatus status;
    size_t payload_size = 0;
    char path[512];

    if (!save_base || !out_receipt) return false;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->recursive_scan_depth_limit = DM2_SK_CORPUS_RECURSE_DEPTH;
    out_receipt->recursive_scan_candidate_cap =
        DM2_SK_CORPUS_RECURSE_CANDIDATE_CAP;

    /* SKWin/DM2 resume probes SKSave.dat before SKSave.bak; keep the same
     * preference so real corpus scans tell the runtime which file would win.
     * The authenticated 42-byte container header is the same initial gate
     * used by slot loads. */
    if (dm2_sksave_root_variant_path(save_base, 1, 0u, 0, path,
                                     sizeof(path))) {
        status = dm2_sksave_probe_path(path, &payload_size);
    } else {
        status = DM2_SK_CORPUS_MISSING;
    }
    if (status == DM2_SK_CORPUS_VALID) {
        out_receipt->has_last_session = true;
        dm2_sksave_corpus_accept(out_receipt, path, payload_size);
        dm2_sksave_corpus_classify_payload(out_receipt, path, payload_size);
    } else if (status == DM2_SK_CORPUS_INVALID) {
        out_receipt->invalid_candidate_count++;
    }

    payload_size = 0;
    if (dm2_sksave_root_variant_path(save_base, 1, 0u, 1, path,
                                     sizeof(path))) {
        status = dm2_sksave_probe_path(path, &payload_size);
    } else {
        status = DM2_SK_CORPUS_MISSING;
    }
    if (status == DM2_SK_CORPUS_VALID) {
        out_receipt->has_last_session_backup = true;
        if (!out_receipt->has_last_session) {
            out_receipt->last_session_uses_backup = true;
            /* GAME_LOAD admits the backup only after the primary fails the
             * same 42-byte header gate. A present primary shadows this
             * record; retain its availability fact above, but do not let a
             * corpus receipt classify it as a second live import. */
            dm2_sksave_corpus_accept(out_receipt, path, payload_size);
            dm2_sksave_corpus_classify_payload(out_receipt, path,
                                               payload_size);
        }
    } else if (status == DM2_SK_CORPUS_INVALID) {
        out_receipt->invalid_candidate_count++;
    }

    for (uint8_t slot = 0; slot < DM2_SLOT_MAX; slot++) {
        payload_size = 0;
        if (dm2_sksave_root_variant_path(save_base, 0, slot, 0, path,
                                         sizeof(path))) {
            status = dm2_sksave_probe_path(path, &payload_size);
        } else {
            status = DM2_SK_CORPUS_MISSING;
        }
        if (status == DM2_SK_CORPUS_VALID) {
            out_receipt->valid_slot_count++;
            out_receipt->valid_slot_mask |= (uint16_t)(1u << slot);
            dm2_sksave_corpus_accept(out_receipt, path, payload_size);
            dm2_sksave_corpus_classify_payload(out_receipt, path, payload_size);
        } else if (status == DM2_SK_CORPUS_INVALID) {
            out_receipt->invalid_candidate_count++;
        }
        payload_size = 0;
        if (dm2_sksave_root_variant_path(save_base, 0, slot, 1, path,
                                         sizeof(path))) {
            status = dm2_sksave_probe_path(path, &payload_size);
        } else {
            status = DM2_SK_CORPUS_MISSING;
        }
        if (status == DM2_SK_CORPUS_VALID) {
            out_receipt->valid_slot_backup_count++;
            dm2_sksave_corpus_accept(out_receipt, path, payload_size);
            dm2_sksave_corpus_classify_payload(out_receipt, path, payload_size);
        } else if (status == DM2_SK_CORPUS_INVALID) {
            out_receipt->invalid_candidate_count++;
        }
    }

#if !defined(_WIN32)
    {
        unsigned int recursive_candidate_count = 0u;
        dm2_sksave_corpus_scan_recursive_impl(save_base, save_base, 0,
                                              out_receipt,
                                              &recursive_candidate_count);
    }
#endif

    return true;
}

bool dm2_v1_distant_environment_timer_corpus_probe(
    const char *save_base,
    DM2_DistantEnvironmentTimerCorpusReceipt *out_receipt)
{
    DM2_SKSaveCorpusReceipt corpus;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return false;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_sksave_corpus_scan(save_base, &corpus)) return false;
    out_receipt->scan_complete = 1;
    out_receipt->has_header_verified_candidate =
        corpus.has_last_session || corpus.has_last_session_backup ||
        corpus.valid_slot_count != 0u || corpus.extra_valid_candidate_count != 0u;
    if (corpus.total_importable_payload_size <= UINT32_MAX) {
        out_receipt->verified_payload_bytes =
            (uint32_t)corpus.total_importable_payload_size;
    }
    /* No skproject-correlated original byte offset or timer tag exists yet.
     * Never scan heuristically or promote a header-valid save to runtime. */
    out_receipt->skipped_missing_live_timer = 1;
    hash = dm2_sksave_corpus_hash_step(hash, (uint32_t)out_receipt->scan_complete);
    hash = dm2_sksave_corpus_hash_step(hash, (uint32_t)out_receipt->has_header_verified_candidate);
    hash = dm2_sksave_corpus_hash_step(hash, (uint32_t)corpus.total_payload_size);
    hash = dm2_sksave_corpus_hash_step(hash, out_receipt->verified_payload_bytes);
    out_receipt->corpus_hash = hash;
    return true;
}

bool dm2_v1_original_timer_format_corpus_probe(
    const char *save_base,
    DM2_OriginalTimerFormatCorpusReceipt *out_receipt)
{
    DM2_SKSaveCorpusReceipt corpus;
    uint32_t hash = 2166136261u;
    uint8_t i;

    if (!out_receipt) return false;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_sksave_corpus_scan(save_base, &corpus)) return false;

    /* skproject/SKULLWIN/c_timer.cpp and c_savegame.cpp establish that timer
     * state exists, but do not identify an original save-record owner or wire
     * layout. Retain only the already parsed, header-verified payload identity
     * so a later trace can bind a precise row without heuristic scanning. */
    out_receipt->scan_complete = 1;
    out_receipt->has_header_verified_candidate =
        corpus.has_last_session || corpus.has_last_session_backup ||
        corpus.valid_slot_count != 0u || corpus.extra_valid_candidate_count != 0u;
    out_receipt->original_candidate_list_complete =
        corpus.candidate_receipt_count == corpus.importable_candidate_count;
    out_receipt->original_candidate_count =
        (uint16_t)(corpus.original_envelope_candidate_count +
                   corpus.original_raw_candidate_count);
    out_receipt->rejected_unowned_candidate_count =
        out_receipt->original_candidate_count;

    for (i = 0u; i < corpus.candidate_receipt_count; ++i) {
        const DM2_SKSaveCandidateReceipt *source =
            &corpus.candidate_receipts[i];
        DM2_SKSaveCandidateReceipt *target;

        if (source->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
            source->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
            continue;
        }
        if (source->payload_size <= UINT32_MAX &&
            source->payload_size <=
                (size_t)(UINT32_MAX -
                         out_receipt->retained_original_payload_bytes)) {
            out_receipt->retained_original_payload_bytes +=
                (uint32_t)source->payload_size;
        } else {
            out_receipt->original_candidate_list_complete = 0;
        }
        if (out_receipt->candidate_receipt_count >=
            DM2_SK_CORPUS_RECEIPT_MAX) {
            out_receipt->original_candidate_list_complete = 0;
            continue;
        }
        target = &out_receipt->candidate_receipts[
            out_receipt->candidate_receipt_count++];
        *target = *source;
        target->import_rejected = 1;
        hash = dm2_sksave_corpus_hash_step(hash, (uint32_t)target->kind);
        hash = dm2_sksave_corpus_hash_step(hash,
                                           (uint32_t)target->payload_size);
        hash = dm2_sksave_corpus_hash_step(hash, target->payload_hash);
    }

    /* Fail closed: no skproject source or original trace binds any retained
     * payload bytes to a timer record. This receipt is never a runtime input. */
    out_receipt->timer_layout_owner_proven = 0;
    out_receipt->matching_timer_record_count = 0;
    hash = dm2_sksave_corpus_hash_step(hash,
                                       out_receipt->retained_original_payload_bytes);
    hash = dm2_sksave_corpus_hash_step(
        hash, (uint32_t)out_receipt->original_candidate_list_complete);
    out_receipt->corpus_hash = hash;
    return true;
}

static int dm2_v1_original_raw_timer_stream_receipt(
    const uint8_t *payload,
    size_t payload_size,
    uint8_t expected_timer_count,
    uint32_t *out_offset,
    uint32_t *out_byte_count,
    uint32_t *out_hash)
{
    DM2_V1_OriginalRawSaveStateReceipt state;
    size_t timer_start;
    size_t timer_end;

    if (out_offset) *out_offset = 0u;
    if (out_byte_count) *out_byte_count = 0u;
    if (out_hash) *out_hash = 0u;
    if (!payload || !out_offset || !out_byte_count || !out_hash ||
        payload_size > UINT32_MAX ||
        !dm2_v1_original_raw_sksave_fixed_state_receipt(payload, payload_size,
                                                         &state) ||
        !state.valid || state.timer_count != expected_timer_count) {
        return 0;
    }
    /* Source: SKProject sksvgame.cpp::DM2_GAME_LOAD.  These offsets bound
     * the original shared bitstream; they intentionally include a partially
     * consumed boundary byte when one exists. */
    timer_start = state.timer_bitstream_offset;
    timer_end = state.record_link_bitstream_offset;
    if (timer_start > UINT32_MAX || timer_end < timer_start ||
        timer_end > payload_size || timer_end - timer_start > UINT32_MAX) {
        return 0;
    }

    *out_offset = (uint32_t)timer_start;
    *out_byte_count = (uint32_t)(timer_end - timer_start);
    *out_hash = dm2_sksave_corpus_payload_hash(
        payload + timer_start, timer_end - timer_start, 2166136261u);
    return 1;
}

bool dm2_v1_original_save_state_corpus_probe(
    const char *save_base,
    DM2_OriginalSaveStateCorpusReceipt *out_receipt)
{
    DM2_SKSaveCorpusReceipt corpus;
    uint32_t hash = 2166136261u;
    uint8_t i;

    if (!out_receipt) return false;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!dm2_v1_sksave_corpus_scan(save_base, &corpus)) return false;

    /* skproject/SKWIN/SkWinCore.cpp GAME_LOAD ^2066:2F8C-319F opens the
     * chosen SKSave, then reads skload_table_60, champions and 10-byte timer
     * entries before READ_SKSAVE_DUNGEON.  Retain only those importer-owned
     * values after the complete file-hash receipt has been revalidated. */
    out_receipt->scan_complete = 1;
    out_receipt->original_candidate_list_complete =
        corpus.candidate_receipt_count == corpus.importable_candidate_count;
    out_receipt->original_candidate_count =
        (uint16_t)(corpus.original_envelope_candidate_count +
                   corpus.original_raw_candidate_count);

    for (i = 0u; i < corpus.candidate_receipt_count; ++i) {
        const DM2_SKSaveCandidateReceipt *source =
            &corpus.candidate_receipts[i];
        DM2_OriginalSaveStateCorpusEntry *target;
        DM2_V1_SaveCandidate candidate;
        uint8_t payload[DM2_SESSION_MAX_SIZE];
        size_t payload_size = 0u;

        if (source->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
            source->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
            continue;
        }
        if (out_receipt->entry_count >= DM2_SK_CORPUS_RECEIPT_MAX) {
            out_receipt->original_candidate_list_complete = 0;
            out_receipt->rejected_candidate_count++;
            continue;
        }
        if (!dm2_v1_sksave_corpus_load_receipted_candidate(
                source, payload, sizeof(payload), &payload_size) ||
            dm2_v1_session_parse_save_candidate(&candidate, payload,
                                                 payload_size) != 0 ||
            candidate.kind != (DM2_V1_SaveCandidateKind)source->kind ||
            (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
             !candidate.dungeon_receipt.valid)) {
            out_receipt->original_candidate_list_complete = 0;
            out_receipt->rejected_candidate_count++;
            continue;
        }

        target = &out_receipt->entries[out_receipt->entry_count++];
        target->candidate = *source;
        target->game_tick = candidate.session.game_tick;
        target->rng_seed = candidate.session.rng_seed;
        target->party_x = candidate.session.party_x;
        target->party_y = candidate.session.party_y;
        target->party_dir = candidate.session.party_dir;
        target->party_map = candidate.session.party_level;
        target->champion_count = candidate.session.champion_count;
        target->timer_count = candidate.session.original_timer_count;
        target->rain_intensity = candidate.session.rain_intensity;
        target->global_flags_hash = dm2_sksave_corpus_payload_hash(
            candidate.session.original_global_flags,
            sizeof(candidate.session.original_global_flags), 2166136261u);
        target->global_bytes_hash = dm2_sksave_corpus_payload_hash(
            candidate.session.original_global_bytes,
            sizeof(candidate.session.original_global_bytes), 2166136261u);
        target->global_words_hash = dm2_sksave_corpus_words_hash(
            candidate.session.original_global_words,
            DM2_GLOBAL_WORDS_SIZE);
        target->spell_effects_hash = dm2_sksave_corpus_payload_hash(
            candidate.session.original_spell_effects,
            sizeof(candidate.session.original_spell_effects), 2166136261u);
        if (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
            uint8_t pool;

            target->raw_dungeon_layout_valid = 1;
            target->raw_dungeon_map_count = candidate.dungeon_receipt.map_count;
            target->raw_dungeon_prefix_hash =
                candidate.dungeon_receipt.prefix_hash;
            target->raw_map_data_hash = candidate.dungeon_receipt.map_data_hash;
            for (pool = 0u; pool < DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT;
                 ++pool) {
                target->raw_db_record_counts[pool] =
                    candidate.dungeon_receipt.db_record_counts[pool];
            }
            if (!dm2_v1_original_raw_timer_stream_receipt(
                    payload, payload_size, target->timer_count,
                    &target->raw_timer_stream_offset,
                    &target->raw_timer_stream_byte_count,
                    &target->raw_timer_stream_hash)) {
                out_receipt->original_candidate_list_complete = 0;
                out_receipt->rejected_candidate_count++;
                --out_receipt->entry_count;
                continue;
            }
        }
        hash = dm2_sksave_corpus_hash_step(hash, target->candidate.source_file_hash);
        hash = dm2_sksave_corpus_hash_step(hash, target->game_tick);
        hash = dm2_sksave_corpus_hash_step(hash, target->rng_seed);
        hash = dm2_sksave_corpus_hash_step(
            hash, ((uint32_t)target->party_x << 16) | target->party_y);
        hash = dm2_sksave_corpus_hash_step(
            hash, ((uint32_t)target->party_dir << 24) |
                  ((uint32_t)target->party_map << 16) |
                  ((uint32_t)target->champion_count << 8) |
                  target->timer_count);
        target->state_hash = dm2_sksave_corpus_hash_step(
            hash, target->rain_intensity);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->global_flags_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->global_bytes_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->global_words_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->spell_effects_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->raw_timer_stream_offset);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->raw_timer_stream_byte_count);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->raw_timer_stream_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, (uint32_t)target->raw_dungeon_layout_valid);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->raw_dungeon_prefix_hash);
        target->state_hash = dm2_sksave_corpus_hash_step(
            target->state_hash, target->raw_map_data_hash);
        for (uint8_t pool = 0u; pool < DM2_ORIGINAL_SAVE_RAW_DB_POOL_COUNT;
             ++pool) {
            target->state_hash = dm2_sksave_corpus_hash_step(
                target->state_hash, target->raw_db_record_counts[pool]);
        }
        hash = target->state_hash;
        out_receipt->parsed_candidate_count++;
    }
    hash = dm2_sksave_corpus_hash_step(
        hash, (uint32_t)out_receipt->original_candidate_list_complete);
    hash = dm2_sksave_corpus_hash_step(hash, out_receipt->original_candidate_count);
    hash = dm2_sksave_corpus_hash_step(hash, out_receipt->rejected_candidate_count);
    out_receipt->corpus_hash = hash;
    return true;
}

bool dm2_v1_sksave_corpus_load_first_importable(
    const char *save_base,
    uint8_t *out_payload,
    size_t out_capacity,
    size_t *out_payload_size,
    DM2_SKSaveCorpusReceipt *out_receipt)
{
    DM2_SKSaveCorpusReceipt local_receipt;
    DM2_SKSaveCorpusReceipt *receipt =
        out_receipt ? out_receipt : &local_receipt;

    if (out_payload_size) *out_payload_size = 0u;
    if (!save_base || !out_payload || out_capacity == 0u ||
        !out_payload_size) {
        return false;
    }
    if (!dm2_v1_sksave_corpus_scan(save_base, receipt) ||
        receipt->importable_candidate_count == 0 ||
        receipt->first_importable_path[0] == '\0') {
        return false;
    }
    for (uint8_t i = 0u; i < receipt->candidate_receipt_count; ++i) {
        if (strcmp(receipt->candidate_receipts[i].path,
                   receipt->first_importable_path) == 0) {
            return dm2_v1_sksave_corpus_load_receipted_candidate(
                &receipt->candidate_receipts[i], out_payload, out_capacity,
                out_payload_size);
        }
    }
    return false;
}

bool dm2_v1_sksave_corpus_load_receipted_candidate(
    const DM2_SKSaveCandidateReceipt *candidate_receipt,
    uint8_t *out_payload,
    size_t out_capacity,
    size_t *out_payload_size)
{
    DM2_V1_SaveCandidate candidate;
    size_t payload_size = 0u;

    if (out_payload_size) *out_payload_size = 0u;
    if (!candidate_receipt || !candidate_receipt->path[0] ||
        !candidate_receipt->source_file_hash || !out_payload ||
        out_capacity == 0u || !out_payload_size ||
        dm2_sksave_corpus_file_hash(candidate_receipt->path) !=
            candidate_receipt->source_file_hash ||
        dm2_sksave_read_valid_payload(candidate_receipt->path, out_payload,
                                      out_capacity, &payload_size) != 0 ||
        payload_size != candidate_receipt->payload_size ||
        dm2_sksave_corpus_payload_hash(out_payload, payload_size,
                                       2166136261u) !=
            candidate_receipt->payload_hash ||
        dm2_v1_session_parse_save_candidate(&candidate, out_payload,
                                             payload_size) != 0 ||
        (int)candidate.kind != candidate_receipt->kind) {
        return false;
    }
    *out_payload_size = payload_size;
    return true;
}

int dm2_v1_save_version_diagnostics(const uint8_t *data, size_t size)
{
    int flags = 0;
    if (!data || size < 4) return flags;
    DM2_SL_State st;
    if (data[0] == 0x00 && data[1] == 0x00) {
        (void)st; /* suppress unused warning */
        flags |= DM2V1_SAVE_DIAG_NULL_FILL;
    }
    if (size >= 10) {
        const uint8_t *p = data + size - 10;
        uint8_t v = p[0];
        bool allsame = (v != 0);
        for (size_t k = 1; k < 10 && allsame; k++) {
            if (p[k] != v) allsame = false;
        }
        if (allsame) flags |= DM2V1_SAVE_DIAG_SUPPRESS_FILL;
    }
    if (size < 44) flags |= DM2V1_SAVE_DIAG_TRUNCATED;
    return flags;
}

int dm2_v1_save_detect_game_version(const uint8_t *header42)
{
    if (!header42) return DM2V1_VERSION_UNKNOWN;
    if (dm2_sl_header_valid(header42)) return DM2V1_VERSION_DM2;
    uint16_t m1 = (uint16_t)header42[38] | ((uint16_t)header42[39] << 8);
    uint16_t m2 = (uint16_t)header42[40] | ((uint16_t)header42[41] << 8);
    if ((m1 == 0x444D || m1 == 0x4D44) &&
        (m2 == 0 || m2 == 0x3156 || m2 == 0x5631)) return DM2V1_VERSION_DM1;
    return DM2V1_VERSION_UNKNOWN;
}

const char *dm2_v1_save_source_evidence(void)
{
    return
        "SKULL.ASM: DM2 save/load entry points, SUPPRESS codec\n"
        "docs/dm2_save_format.md: full format specification\n"
        "docs/dm2_save_slots.md: 10-slot system and DOS header shape\n"
        "docs/dm2_party_state.md: champion squad persistence, SUPPRESS masks\n"
        "SKULL.ASM: WRITE_RECORD_CHECKCODE, WRITE_MINION_ASSOC\n"
        "SKULL.ASM: _2066_33e7 slot picker, GAME_SAVE/GAME_LOAD\n"
        "Firestaff: dm2_v1_sksave_corpus_scan validates real SKSave corpus "
        "candidates before full session import; SKSave.dat shadows "
        "SKSave.bak until its header gate fails\n";
}

/* ════════════════════════════════════════════════════════════════
 * Champion SUPPRESS mask table
 * Source: docs/dm2_party_state.md — _4976_3992 write-mask pattern;
 * 261 bytes of per-field mask values (0x00=skip, 0xFF=all source bits).
 * SKProject's original table1d6356 is not yet catalogued here, so modeled
 * fields retain every bit rather than silently discarding high-bit corpus
 * values. ReDMCSB/SKProject: SKULLWIN/c_savegame.cpp
 * DM2_SUPPRESS_WRITER lines 1596-1659.
 * This mask marks every field that can hold non-zero data in a live
 * champion so SUPPRESS compression achieves source-authentic packing.
 * ════════════════════════════════════════════════════════════════ */

void dm2_suppress_champion_mask(uint8_t mask[261])
{
    if (!mask) return;
    memset(mask, 0, 261);

    /* Name block — 8 + 16 = 24 bytes, retain all source bits. */
    memset(&mask[0],  0xFF, 8);   /* first_name */
    memset(&mask[8],  0xFF, 16);  /* last_name */

    /* Position — 2 + 1 bytes */
    mask[24] = 0xFF; mask[25] = 0xFF;  /* absolute_direction (u16) */
    mask[26] = 0xFF;                   /* squad_position */

    /* HP cur/max — 2+2 + 2+2 */
    mask[27] = 0xFF; mask[28] = 0xFF;   /* cur_hp */
    mask[29] = 0xFF; mask[30] = 0xFF;   /* max_hp */
    /* stamina + mana (2 bytes each, store both LE bytes) */
    mask[31] = 0xFF; mask[32] = 0xFF;  /* stamina */
    mask[33] = 0xFF; mask[34] = 0xFF;  /* mana */

    /* Poison/runes — 1 byte each */
    mask[35] = 0xFF; mask[36] = 0xFF;  /* poison_value, runes_count */
    /* spelled_runes[4] */
    mask[37] = 0xFF; mask[38] = 0xFF; mask[39] = 0xFF; mask[40] = 0xFF;

    /* attributes[7][2]: cur/max pairs — 14 uint16_t = 28 bytes starting at 41 */
    for (int a = 0; a < 7; a++) {
        mask[41 + a*4] = 0xFF; mask[42 + a*4] = 0xFF; /* cur (LE) */
        mask[43 + a*4] = 0xFF; mask[44 + a*4] = 0xFF; /* max (LE) */
    }

    /* food / water — int16_t LE */
    mask[69] = 0xFF; mask[70] = 0xFF; mask[71] = 0xFF; mask[72] = 0xFF;

    /* combat hand state: hand_command[2] × uint32_t, hand_cooldown[2] × u16,
     * hand_defense_class[2] × u8 — 9 fields starting at 73 */
    mask[73] = 0xFF; mask[74] = 0xFF; mask[75] = 0xFF; mask[76] = 0xFF; /* hand_command[0] u32*/
    mask[77] = 0xFF; mask[78] = 0xFF; mask[79] = 0xFF; mask[80] = 0xFF; /* hand_command[1] u32*/
    mask[81] = 0xFF; mask[82] = 0xFF; /* hand_cooldown[0] u16 */
    mask[83] = 0xFF; mask[84] = 0xFF; /* hand_cooldown[1] u16 */
    mask[85] = 0xFF; mask[86] = 0xFF; /* hand_defense_class[0]+[1] */

    /* timer_index, damage_suffered, hero_flag, body_flag — 4 bytes */
    mask[87] = 0xFF; mask[88] = 0xFF; mask[89] = 0xFF; mask[90] = 0xFF;

    /* inventory[30] × uint32_t: bytes 91-210 (120 bytes), store full 32 bits */
    for (int i = 0; i < 30; i++) {
        size_t base = 91 + i * 4;
        mask[base + 0] = 0xFF;
        mask[base + 1] = 0xFF;
        mask[base + 2] = 0xFF;
        mask[base + 3] = 0xFF;
    }
}

int dm2_suppress_encode_champion(const DM2_ChampionRecord *c,
                                  const uint8_t *mask,
                                  uint8_t *out, size_t out_sz)
{
    if (!c || !mask || !out || out_sz < 261) return -1;
    return dm2_suppress_encode((const uint8_t *)c, mask, 261, out, out_sz);
}

int dm2_suppress_decode_champion(const uint8_t *in, size_t in_sz,
                                  const uint8_t *mask,
                                  DM2_ChampionRecord *c,
                                  uint8_t fill)
{
    if (!in || !mask || !c) return -1;
    /* in_sz is the SUPPRESS-encoded stream length; encoded form is
     * typically shorter than 261 bytes since masked nibbles are packed
     * into selected source bits. dm2_suppress_decode detects bit-underflow, so only
     * the empty-input case is rejected here. */
    if (in_sz == 0) return -1;
    return dm2_suppress_decode(in, in_sz, mask, 261, (uint8_t *)c, fill);
}

/* ════════════════════════════════════════════════════════════════
 * DB handle resolution
 * ObjectID: high byte = pool (0-15), low 24 bits = record index.
 * Source: docs/dm2_save_format.md § DB record pools
 * ════════════════════════════════════════════════════════════════ */

uint32_t dm2_db_make_handle(uint8_t pool, uint32_t index)
{
    if (pool >= DM2_DB_POOL_COUNT) return 0;
    return ((uint32_t)pool << 24) | (index & 0x00FFFFFF);
}

bool dm2_db_decode_handle(uint32_t object_id,
                          uint8_t *out_pool,
                          uint32_t *out_index)
{
    uint8_t pool;
    uint32_t idx;

    if (object_id == 0) return false;
    pool = (uint8_t)((object_id >> 24) & 0xFFu);
    idx = object_id & 0x00FFFFFFu;
    if (pool >= DM2_DB_POOL_COUNT) return false;
    if (out_pool) *out_pool = pool;
    if (out_index) *out_index = idx;
    return true;
}

const char *dm2_db_pool_label(uint8_t pool)
{
    switch (pool) {
        case 5: return "WEAPON";
        case 6: return "CLOTH";
        case 7: return "SCROLL";
        case 10: return "MISC";
        default: return "DB";
    }
}

bool dm2_db_format_handle_name(uint32_t object_id,
                               char *out,
                               size_t out_size)
{
    uint8_t pool;
    uint32_t idx;

    if (!out || out_size == 0) return false;
    out[0] = '\0';
    if (!dm2_db_decode_handle(object_id, &pool, &idx)) return false;
    snprintf(out, out_size, "DM2 %s %lu",
             dm2_db_pool_label(pool),
             (unsigned long)idx);
    return out[0] != '\0';
}

bool dm2_db_resolve(uint32_t object_id,
                     const DM2_DB_State *db,
                     uint8_t *out_pool, uint32_t *out_index)
{
    uint8_t pool;
    uint32_t idx;

    if (!dm2_db_decode_handle(object_id, &pool, &idx)) return false;
    if (!db || !db->pools[pool].data) return false;
    if (idx >= db->pools[pool].rec_count) return false;
    if (out_pool)  *out_pool  = pool;
    if (out_index) *out_index = idx;
    return true;
}

bool dm2_db_write_record(uint8_t pool, uint32_t index,
                           FILE *f,
                           const DM2_DB_State *db)
{
    if (!f) return false;
    if (pool >= DM2_DB_POOL_COUNT) return false;
    if (!db) return false;
    const DM2_DB_Pool *p = &db->pools[pool];
    if (!p->data || index >= p->rec_count || p->rec_size == 0) return false;
    size_t offset = (size_t)index * p->rec_size;
    return fwrite(p->data + offset, p->rec_size, 1, f) == 1;
}

/* ════════════════════════════════════════════════════════════════════════
 * Retired D2RS diagnostic state envelope (56 bytes, SUPPRESS-encoded)
 * The original s_savegamebuffer is 60 bytes and is decoded only by the raw
 * SKSave receipt in dm2_v1_new_game.c.
 * Type DM2_GameStateBlock is defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GAME_STATE_BLOCK_SIZE 56

/* Legacy D2RS diagnostic mask (56 bytes). It is never an original-SKSave
 * mask: SKProject dm2data.cpp::table1d631a contains 60 bytes. */
static void dm2_suppress_gamestate_mask(uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE])
{
    /* SKProject SKWIN/SkGlobal.cpp:336-341, _4976_395a. The final explicit
     * zero is the C-string terminator consumed as mask byte 55. */
    static const uint8_t source_mask[DM2_GAME_STATE_BLOCK_SIZE] = {
        0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0x00, 0x00,
        0x07, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x03, 0x00,
        0x3f, 0x00, 0x03, 0x00, 0xff, 0x01, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x07, 0x00,
        0x07, 0x00, 0x03, 0x00, 0x01, 0x00, 0xff, 0x00,
        0x01, 0x00, 0x01, 0x03, 0xff, 0xff, 0xff, 0x03,
        0xff, 0x00, 0x1f, 0x03, 0xff, 0xff, 0xff, 0x00
    };
    if (mask) memcpy(mask, source_mask, sizeof(source_mask));
}

int dm2_suppress_encode_gamestate(const DM2_GameStateBlock *gs,
                                   uint8_t *out, size_t out_sz)
{
    if (!gs || !out || out_sz < DM2_GAME_STATE_BLOCK_SIZE) return -1;
    uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE];
    dm2_suppress_gamestate_mask(mask);
    return dm2_suppress_encode((const uint8_t *)gs, mask,
                               DM2_GAME_STATE_BLOCK_SIZE, out, out_sz);
}

int dm2_suppress_decode_gamestate(const uint8_t *in, size_t in_sz,
                                   DM2_GameStateBlock *gs, uint8_t fill)
{
    if (!in || !gs) return -1;
    /* in_sz is the SUPPRESS-encoded stream length; the encoded form is
     * typically shorter than DM2_GAME_STATE_BLOCK_SIZE (56) bytes since
     * every masked byte is packed into 7 LSBs. dm2_suppress_decode
     * itself detects bit-underflow, so the wrapper only needs to reject
     * the empty-input case. */
    if (in_sz == 0) return -1;
    uint8_t mask[DM2_GAME_STATE_BLOCK_SIZE];
    dm2_suppress_gamestate_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask,
                               DM2_GAME_STATE_BLOCK_SIZE, (uint8_t *)gs, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Global variables (flags/bytes/words) — SUPPRESS encoded
 * Source: docs/dm2_save_format.md § Ingame global flags/bytes/words
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_FLAGS_SIZE  8
#define DM2_GLOBAL_BYTES_SIZE  64
#define DM2_GLOBAL_WORDS_SIZE  64  /* 64 x uint16_t = 128 bytes */

/* SUPPRESS mask for global flags (8 bytes) — all bits stored */
static void dm2_suppress_global_flags_mask(uint8_t mask[DM2_GLOBAL_FLAGS_SIZE])
{
    if (!mask) return;
    memset(mask, 0xFF, DM2_GLOBAL_FLAGS_SIZE);
}

/* SUPPRESS mask for global bytes (64 bytes) — all bytes stored */
static void dm2_suppress_global_bytes_mask(uint8_t mask[DM2_GLOBAL_BYTES_SIZE])
{
    if (!mask) return;
    memset(mask, 0xFF, DM2_GLOBAL_BYTES_SIZE);
}

/* SUPPRESS mask for global words (64 words = 128 bytes) — all words stored */
static void dm2_suppress_global_words_mask(uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2])
{
    if (!mask) return;
    memset(mask, 0xFF, DM2_GLOBAL_WORDS_SIZE * 2);
}

int dm2_suppress_encode_global_flags(const uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!flags || !out) return -1;
    uint8_t mask[DM2_GLOBAL_FLAGS_SIZE];
    dm2_suppress_global_flags_mask(mask);
    return dm2_suppress_encode(flags, mask, DM2_GLOBAL_FLAGS_SIZE, out, out_sz);
}

int dm2_suppress_decode_global_flags(const uint8_t *in, size_t in_sz,
                                     uint8_t flags[DM2_GLOBAL_FLAGS_SIZE],
                                     uint8_t fill)
{
    if (!in || !flags) return -1;
    uint8_t mask[DM2_GLOBAL_FLAGS_SIZE];
    dm2_suppress_global_flags_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_FLAGS_SIZE, flags, fill);
}

int dm2_suppress_encode_global_bytes(const uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!bytes || !out) return -1;
    uint8_t mask[DM2_GLOBAL_BYTES_SIZE];
    dm2_suppress_global_bytes_mask(mask);
    return dm2_suppress_encode(bytes, mask, DM2_GLOBAL_BYTES_SIZE, out, out_sz);
}

int dm2_suppress_decode_global_bytes(const uint8_t *in, size_t in_sz,
                                     uint8_t bytes[DM2_GLOBAL_BYTES_SIZE],
                                     uint8_t fill)
{
    if (!in || !bytes) return -1;
    uint8_t mask[DM2_GLOBAL_BYTES_SIZE];
    dm2_suppress_global_bytes_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_BYTES_SIZE, bytes, fill);
}

int dm2_suppress_encode_global_words(const uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t *out, size_t out_sz)
{
    if (!words || !out) return -1;
    uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2];
    dm2_suppress_global_words_mask(mask);
    return dm2_suppress_encode((const uint8_t *)words, mask,
                               DM2_GLOBAL_WORDS_SIZE * 2, out, out_sz);
}

int dm2_suppress_decode_global_words(const uint8_t *in, size_t in_sz,
                                     uint16_t words[DM2_GLOBAL_WORDS_SIZE],
                                     uint8_t fill)
{
    if (!in || !words) return -1;
    uint8_t mask[DM2_GLOBAL_WORDS_SIZE * 2];
    dm2_suppress_global_words_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_WORDS_SIZE * 2,
                               (uint8_t *)words, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Global spell effects (6 bytes, SUPPRESS)
 * Source: docs/dm2_party_state.md § Global spell effects
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_GLOBAL_SPELL_EFFECTS_SIZE 6

/* SUPPRESS mask for global spell effects (6 bytes) */
static void dm2_suppress_spell_effects_mask(uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE])
{
    if (!mask) return;
    memset(mask, 0xFF, DM2_GLOBAL_SPELL_EFFECTS_SIZE);
}

int dm2_suppress_encode_spell_effects(const uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t *out, size_t out_sz)
{
    if (!effects || !out) return -1;
    uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    dm2_suppress_spell_effects_mask(mask);
    return dm2_suppress_encode(effects, mask, DM2_GLOBAL_SPELL_EFFECTS_SIZE,
                               out, out_sz);
}

int dm2_suppress_decode_spell_effects(const uint8_t *in, size_t in_sz,
                                       uint8_t effects[DM2_GLOBAL_SPELL_EFFECTS_SIZE],
                                       uint8_t fill)
{
    if (!in || !effects) return -1;
    uint8_t mask[DM2_GLOBAL_SPELL_EFFECTS_SIZE];
    dm2_suppress_spell_effects_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_GLOBAL_SPELL_EFFECTS_SIZE,
                               effects, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Timers table (10 bytes per timer, SUPPRESS)
 * Source: docs/dm2_save_format.md § Timers table
 * Types DM2_TimerEntry and DM2_MAX_TIMERS are defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

#define DM2_TIMER_ENTRY_SIZE  10

/* SUPPRESS mask for timer entry (10 bytes) */
static void dm2_suppress_timer_mask(uint8_t mask[DM2_TIMER_ENTRY_SIZE])
{
    if (!mask) return;
    memset(mask, 0, DM2_TIMER_ENTRY_SIZE);
    /* timer_id (2 bytes) */
    mask[0] = 0xFF; mask[1] = 0xFF;
    /* current_tick (2 bytes) */
    mask[2] = 0xFF; mask[3] = 0xFF;
    /* interval_ticks (2 bytes) */
    mask[4] = 0xFF; mask[5] = 0xFF;
    /* flags (2 bytes) */
    mask[6] = 0xFF; mask[7] = 0xFF;
    /* user_data (2 bytes) */
    mask[8] = 0xFF; mask[9] = 0xFF;
}

int dm2_suppress_encode_timer(const DM2_TimerEntry *t,
                               uint8_t *out, size_t out_sz)
{
    if (!t || !out || out_sz < DM2_TIMER_ENTRY_SIZE) return -1;
    uint8_t mask[DM2_TIMER_ENTRY_SIZE];
    dm2_suppress_timer_mask(mask);
    return dm2_suppress_encode((const uint8_t *)t, mask,
                               DM2_TIMER_ENTRY_SIZE, out, out_sz);
}

int dm2_suppress_decode_timer(const uint8_t *in, size_t in_sz,
                               DM2_TimerEntry *t, uint8_t fill)
{
    if (!in || !t) return -1;
    /* in_sz is the SUPPRESS-encoded stream length; encoded form is
     * typically shorter than DM2_TIMER_ENTRY_SIZE (10) bytes since every
     * masked byte is packed into 7 LSBs. dm2_suppress_decode detects
     * bit-underflow, so only the empty-input case is rejected here. */
    if (in_sz == 0) return -1;
    uint8_t mask[DM2_TIMER_ENTRY_SIZE];
    dm2_suppress_timer_mask(mask);
    return dm2_suppress_decode(in, in_sz, mask, DM2_TIMER_ENTRY_SIZE,
                             (uint8_t *)t, fill);
}

/* ════════════════════════════════════════════════════════════════════════
 * Minion association table
 * Source: docs/dm2_party_state.md § Minion Association
 * ════════════════════════════════════════════════════════════════════════ */

/* Minion table is stored raw (not SUPPRESS) as fixed-size records */
size_t dm2_minion_table_size(const DM2_MinionTable *t)
{
    if (!t) return 0;
    /* 8 bytes per entry (2 x uint32_t) + 1 byte count */
    return 1 + (size_t)t->count * sizeof(DM2_MinionAssoc);
}

int dm2_minion_write(const DM2_MinionTable *t, FILE *f)
{
    if (!t || !f) return -1;
    uint8_t count = t->count;
    if (count > DM2_MAX_MINIONS) count = DM2_MAX_MINIONS;
    if (fwrite(&count, 1, 1, f) != 1) return -1;
    if (fwrite(t->entries, sizeof(DM2_MinionAssoc), count, f) != count)
        return -1;
    return 0;
}

int dm2_minion_read(DM2_MinionTable *t, FILE *f)
{
    if (!t || !f) return -1;
    memset(t, 0, sizeof(*t));
    if (fread(&t->count, 1, 1, f) != 1) return -1;
    if (t->count > DM2_MAX_MINIONS) t->count = DM2_MAX_MINIONS;
    if (fread(t->entries, sizeof(DM2_MinionAssoc), t->count, f) != t->count)
        return -1;
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * Leader hand possession (22 bytes in original, here using ObjectID)
 * Source: docs/dm2_party_state.md § Leader Hand Possession
 * Type DM2_LeaderPossession is defined in dm2_v1_save_load.h
 * ════════════════════════════════════════════════════════════════════════ */

int dm2_leader_possession_write(const DM2_LeaderPossession *lp, FILE *f)
{
    if (!lp || !f) return -1;
    /* Write as 4-byte ObjectID */
    uint8_t buf[4];
    buf[0] = (uint8_t)(lp->object & 0xFF);
    buf[1] = (uint8_t)((lp->object >> 8) & 0xFF);
    buf[2] = (uint8_t)((lp->object >> 16) & 0xFF);
    buf[3] = (uint8_t)((lp->object >> 24) & 0xFF);
    return (fwrite(buf, 4, 1, f) == 1) ? 0 : -1;
}

int dm2_leader_possession_read(DM2_LeaderPossession *lp, FILE *f)
{
    if (!lp || !f) return -1;
    uint8_t buf[4];
    if (fread(buf, 4, 1, f) != 1) return -1;
    lp->object = ((uint32_t)buf[0]) | ((uint32_t)buf[1] << 8) |
                 ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * Champion inventory serialization via WRITE_RECORD_CHECKCODE
 * Each inventory slot is an ObjectID handle; chains are followed.
 * Source: docs/dm2_party_state.md § Inventory: The Item Record Chain
 * ════════════════════════════════════════════════════════════════════════ */

int dm2_champion_inventory_write(const uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                   FILE *f)
{
    if (!inventory || !f) return -1;
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        uint8_t buf[4];
        uint32_t h = inventory[i];
        buf[0] = (uint8_t)(h & 0xFF);
        buf[1] = (uint8_t)((h >> 8) & 0xFF);
        buf[2] = (uint8_t)((h >> 16) & 0xFF);
        buf[3] = (uint8_t)((h >> 24) & 0xFF);
        if (fwrite(buf, 4, 1, f) != 1) return -1;
        /* Note: actual WRITE_RECORD_CHECKCODE follows chains.
         * Here we write the handle; chain following would need
         * the full DB state and is handled at higher level. */
    }
    return 0;
}

int dm2_champion_inventory_read(uint32_t inventory[DM2_CHAMPION_INVENTORY_SLOTS],
                                 FILE *f)
{
    if (!inventory || !f) return -1;
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        uint8_t buf[4];
        if (fread(buf, 4, 1, f) != 1) return -1;
        inventory[i] = ((uint32_t)buf[0]) | ((uint32_t)buf[1] << 8) |
                       ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    }
    return 0;
}

/* ════════════════════════════════════════════════════════════════════════
 * PC savegame interoperability
 * Detects DM1 vs DM2 savegames and provides import diagnostics.
 * Source: docs/dm2_save_format.md § DM1 vs DM2 Key Format Differences
 * ════════════════════════════════════════════════════════════════════════ */

/* Detect the type of PC savegame from raw data or file
 * Returns DM2_PC_SAVE_DM2, DM2_PC_SAVE_DM1, or DM2_PC_SAVE_UNKNOWN */
int dm2_pc_save_detect_type(const uint8_t *data, size_t size)
{
    if (!data || size < 42) return DM2_PC_SAVE_UNKNOWN;
    int version = dm2_v1_save_detect_game_version(data);
    if (version == DM2V1_VERSION_DM2) return DM2_PC_SAVE_DM2;
    if (version == DM2V1_VERSION_DM1) return DM2_PC_SAVE_DM1;
    /* Check for DM1-style champion/dungeon file signatures */
    if (size >= 4) {
        /* DM1 CHAMP.DAT starts with champion name or zero */
        /* DM1 DUNGEON.DAT starts with 0x0000 or level data */
        uint16_t magic = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
        if (magic == 0x0000) return DM2_PC_SAVE_DM1;
    }
    return DM2_PC_SAVE_UNKNOWN;
}

/* PC savegame interoperability report */
const char *dm2_pc_save_interoperability_report(const uint8_t *data, size_t size)
{
    static char report[256];
    int type = dm2_pc_save_detect_type(data, size);
    int diag = dm2_v1_save_version_diagnostics(data, size);

    snprintf(report, sizeof(report), "DM2 PC Save Interoperability Report:\n");
    switch (type) {
        case DM2_PC_SAVE_DM2:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: DM2 (native)\n");
            break;
        case DM2_PC_SAVE_DM1:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: DM1 (import supported via DM1->DM2 champion import)\n");
            break;
        default:
            snprintf(report + strlen(report), sizeof(report) - strlen(report),
                     "  Format: Unknown\n");
            break;
    }

    if (diag & DM2V1_SAVE_DIAG_NULL_FILL)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: NULL fill detected\n");
    if (diag & DM2V1_SAVE_DIAG_SUPPRESS_FILL)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: SUPPRESS fill pattern detected\n");
    if (diag & DM2V1_SAVE_DIAG_TRUNCATED)
        snprintf(report + strlen(report), sizeof(report) - strlen(report),
                 "  Diagnostics: File may be truncated\n");

    return report;
}

/* ════════════════════════════════════════════════════════════════════════
 * Save game writing — DM2_GAME_SAVE_MENU flow
 * Source: skproject/SKULLWIN/c_savegame.cpp:2087 DM2_GAME_SAVE_MENU
 *
 * Layout:
 *   42-byte slot header (DOS version/name shape; trailing words opaque)
 *   SUPPRESS-encoded game state block (56 bytes data, table1d631a mask)
 *   SUPPRESS-encoded global flags (8 bytes)
 *   SUPPRESS-encoded global bytes (64 bytes)
 *   SUPPRESS-encoded global words (128 bytes = 64 words)
 *   SUPPRESS-encoded champion records (261 bytes each)
 *   SUPPRESS-encoded timer entries (10 bytes each)
 *   SUPPRESS flush
 * ════════════════════════════════════════════════════════════════════════ */

int dm2_v1_save_game_write(const char *path,
                           const DM2_GameStateBlock *gamestate,
                           const uint8_t global_flags[DM2_GLOBAL_FLAGS_SIZE],
                           const uint8_t global_bytes[DM2_GLOBAL_BYTES_SIZE],
                           const uint16_t global_words[DM2_GLOBAL_WORDS_SIZE],
                           const DM2_ChampionRecord *champions,
                           uint8_t champion_count,
                           const DM2_TimerEntry *timers,
                           uint8_t timer_count,
                           DM2_V1_SaveWriteReceipt *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    (void)path;
    (void)gamestate;
    (void)global_flags;
    (void)global_bytes;
    (void)global_words;
    (void)champions;
    (void)champion_count;
    (void)timers;
    (void)timer_count;

    /* c_savegame.cpp:2169-2204 writes the complete dungeon header, map
     * tables, every DB pool and map data before its SUPPRESS sections. This
     * partial helper has none of those source-owned regions, so producing a
     * file here would be synthetic. Keep the public writer closed until that
     * exact record order is implemented. */
    if (out_receipt) out_receipt->fail_closed = 1;
    return -1;
}

/* ════════════════════════════════════════════════════════════════════════
 * Source evidence for Phase 7
 * ════════════════════════════════════════════════════════════════════════ */

const char *dm2_v1_save_phase7_source_evidence(void)
{
    return
        "SKULL.ASM: _2066_#### save/load entry points\n"
        "SKProject sksvgame.cpp: 60-byte s_savegamebuffer raw receipt\n"
        "Firestaff D2RS: retained diagnostic codec only (not original save I/O)\n"
        "SKULL.ASM: WRITE_RECORD_CHECKCODE for inventory chains\n"
        "SKULL.ASM: WRITE_MINION_ASSOC for minion table\n"
        "docs/dm2_save_format.md: full save format specification\n"
        "docs/dm2_save_slots.md: 10-slot system and DOS header shape\n"
        "docs/dm2_party_state.md: champion squad, inventories, minions\n"
        "docs/dm2_source_lock.md: Phase 7 implementation evidence\n";
}

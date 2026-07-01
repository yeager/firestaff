/*
 * firestaff_ftl_decode.c
 *
 * Read-only FTL container decoder (HUNK_BSS / HUNK_DATA / HUNK_CODE).
 *
 * Scope:
 *   * Parse the 20-byte common header + 12-byte segment headers from
 *     ReDMCSB FTL.H (HEADER + SEGMENTHEADER).
 *   * Validate header invariants documented in greatstone d_ftl.html
 *     (magic 0x6160, Unknown1 == 0x0002, c_6 == 0x01, c_7 == 0x00,
 *     segment type/id uniqueness, segment offset+size in buffer).
 *   * Decode HUNK_DATA (zero-run compression) and HUNK_CODE (0x5223
 *     magic + 1920-word frequency table + nibble-coded stream).
 *
 * The HUNK_CODE algorithm is shared with the Atari ST PAK format
 * (greatstone d_pak.html). It is implemented here against the same
 * nibble grammar documented in ReDMCSB DECOMPCO.C F0913_DecompressPAK
 * (and exposed via the F6049_PRIM_25_FTL_DecompressCODESegment
 * primitive in PRIM2B.C). The PAK decoder in firestaff_pak_decode.c
 * already covers the same grammar for the Atari ST 28-byte executable
 * header; this module covers the FTL HUNK_CODE 8-byte wrapper.
 *
 * What this module deliberately does NOT do:
 *   * It does not interpret the FTL resource area (the assets
 *     themselves, e.g. AMGC/IMGR/INFO/etc. extracted from HUNK_DATA
 *     on Amiga ports) — that is left to existing per-game decoders
 *     such as IMG5 / AMG / HTC.
 *   * It does not bind FTL containers to runtime loading; it produces
 *     fully-decoded bytes only.
 *   * It does not execute the decompressed 68k CODE; the CODE bytes
 *     are exposed as a buffer for static analysis / asset extraction
 *     only, matching the PAK decoder's scope.
 *
 * Provenance:
 *   - ReDMCSB Toolchains/Common/Source/FTL.H
 *   - ReDMCSB Toolchains/Common/Source/DECOMPCO.C F0913_DecompressPAK
 *   - ReDMCSB Toolchains/Common/Source/PRIM2B.C F6049_PRIM_25_FTL_DecompressCODESegment
 *   - Greatstone SCK docs (FTL format page)
 */

#include "firestaff_ftl_decode.h"
#include "firestaff_ftl_hunk_data_zero_run.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ── Big-endian reads (68000 order; Atari ST + Amiga both BE) ─── */

static uint16_t rd16_be(const uint8_t* p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static uint32_t rd32_be(const uint8_t* p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8)  |  (uint32_t)p[3];
}

static void wr16_be(uint8_t* p, uint16_t v) {
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)(v & 0xffu);
}

static void wr32_be(uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)(v >> 24);
    p[1] = (uint8_t)((v >> 16) & 0xffu);
    p[2] = (uint8_t)((v >> 8) & 0xffu);
    p[3] = (uint8_t)(v & 0xffu);
}

/* ── Header parse ─────────────────────────────────────── */

int FirestaffFtl_Parse(const uint8_t* data, size_t data_size,
                       FirestaffFtl* out) {
    if (!data || !out) return -1;
    if (data_size < 0x14u) return -1;

    memset(out, 0, sizeof(*out));

    out->header.magic        = rd16_be(data + 0x00);
    out->header.checksum     = rd16_be(data + 0x02);
    out->header.unknown1     = rd16_be(data + 0x04);
    out->header.c_6          = data[0x06];
    out->header.c_7          = data[0x07];
    out->header.i_8          = rd16_be(data + 0x08);
    out->header.c_0a         = data[0x0a];
    out->header.c_0b         = data[0x0b];
    out->header.c_0c         = data[0x0c];
    out->header.c_0d         = data[0x0d];
    out->header.date1        = rd16_be(data + 0x0e);
    out->header.date2        = rd16_be(data + 0x10);
    out->header.segment_count = rd16_be(data + 0x12);

    /* Reject if magic is wrong. */
    if (out->header.magic != FIRESTAFF_FTL_CONTAINER_MAGIC) return -2;

    /* Reject if the loader invariants (Unknown1 == 2, c_6 == 1, c_7 == 0)
     * are violated. ReDMCSB FTL.H: the loader refuses to load such files. */
    if (out->header.unknown1 != 0x0002u) return -3;
    if (out->header.c_6 != 0x01u) return -3;
    if (out->header.c_7 != 0x00u) return -3;

    if (out->header.segment_count == 0u ||
        out->header.segment_count > 8u) {
        return -4;
    }

    size_t seg_count = (size_t)out->header.segment_count;
    if (data_size < 0x14u + seg_count * 12u) return -4;

    /* Track which (type,id) pairs we've seen so we can detect collisions.
     * ReDMCSB SEGMENTHEADER.Id must be unique within the same type. */
    uint16_t seen_type[8] = {0};
    uint16_t seen_id[8]   = {0};
    int      seen_count   = 0;

    out->segment_count_parsed = (uint16_t)seg_count;

    for (size_t i = 0; i < seg_count; ++i) {
        const uint8_t* sp = data + 0x14u + i * 12u;
        FirestaffFtlSegmentHeader* h = &out->segments[i];
        h->type   = rd16_be(sp + 0u);
        h->id     = rd16_be(sp + 2u);
        h->offset = rd32_be(sp + 4u);
        h->size   = rd32_be(sp + 8u);

        /* Reject if segment offset+size falls outside the input buffer. */
        if ((uint64_t)h->offset + (uint64_t)h->size > (uint64_t)data_size) {
            return -6;
        }

        /* Reject if type/id collides with an earlier segment. */
        for (int j = 0; j < seen_count; ++j) {
            if (seen_type[j] == h->type && seen_id[j] == h->id) {
                return -5;
            }
        }
        seen_type[seen_count] = h->type;
        seen_id[seen_count]   = h->id;
        ++seen_count;

        /* Wire convenience slices for BSS, DATA, CODE. The first
         * segment of each known type wins; later duplicates are
         * recorded only in the segments[] array, which lets callers
         * iterate all segments of a type if needed. */
        const uint8_t* payload = data + h->offset;
        if (h->type == FIRESTAFF_HUNK_BSS && out->hunk_bss.data == NULL) {
            out->hunk_bss.data = payload;
            out->hunk_bss.size = h->size;
        } else if (h->type == FIRESTAFF_HUNK_DATA &&
                   out->hunk_data_raw.data == NULL) {
            out->hunk_data_raw.data = payload;
            out->hunk_data_raw.size = h->size;
        } else if (h->type == FIRESTAFF_HUNK_CODE && out->hunk_code_raw.data == NULL) {
            /* Wire the first CODE segment as a zero-copy view so
             * FirestaffFtl_Decode can decode it without needing
             * the original parse buffer base. */
            out->hunk_code_raw.data = payload;
            out->hunk_code_raw.size = h->size;
        }
    }
    return 0;
}

/* ── HUNK_DATA zero-run decoder ───────────────────────── */

static int hunk_data_expected_size(const FirestaffFtl* ftl,
                                   size_t* out_size) {
    if (!ftl || !out_size) return -1;
    *out_size = 0u;
    if (ftl->hunk_data_raw.data == NULL || ftl->hunk_data_raw.size == 0u) {
        return 0;
    }
    /* Greatstone d_ftl.html: HUNK_BSS offset 4 holds the in-memory
     * size of HUNK_DATA area_1. The bounded Note 7 decoder needs this
     * exact size so it can reject truncated or over-expanded data
     * instead of silently guessing. */
    if (ftl->hunk_bss.data == NULL || ftl->hunk_bss.size < 8u) {
        return -1;
    }
    *out_size = (size_t)rd32_be(ftl->hunk_bss.data + 4u);
    return 0;
}

static int decode_hunk_data(const uint8_t* in,
                            size_t in_size,
                            size_t uncompressed_size,
                            uint8_t** out_buf,
                            size_t* out_size) {
    if (!out_buf || !out_size) return -1;
    *out_buf = NULL;
    *out_size = 0u;
    if (!in && in_size == 0u) {
        return uncompressed_size == 0u ? 0 : -1;
    }
    if (!in) return -1;
    if (uncompressed_size == 0u) return -1;

    uint8_t* buf = (uint8_t*)malloc(uncompressed_size);
    if (!buf) {
        return -1;
    }
    size_t written = 0u;
    FirestaffFtlHunkDataStatus rc =
        FirestaffFtlHunkData_DecompressZeroRun(in,
                                               in_size,
                                               uncompressed_size,
                                               buf,
                                               &written);
    if (rc != FIRESTAFF_FTL_HUNK_DATA_OK ||
        written != uncompressed_size) {
        free(buf);
        return -1;
    }
    *out_buf = buf;
    *out_size = written;
    return 0;
}

/* ── HUNK_CODE 0x5223 nibble decoder ──────────────────── */

/*
 * FTL HUNK_CODE layout (per ReDMCSB DECOMPCO.C F0913_DecompressPAK
 * + the documented F6049_PRIM_25_FTL_DecompressCODESegment call site):
 *
 *   offset 0..1   : 0x5223 signature (big-endian)
 *   offset 2..3   : padding (word, unused by decoder)
 *   offset 4..7   : DecompressedDataWordCount (32-bit big-endian)
 *   offset 8..3847: 1920-word MostFrequentWords table (3840 bytes)
 *   offset 3848.. : nibble-coded compressed code
 *
 * The nibble grammar is identical to the PAK format
 * (firestaff_pak_decode.c):
 *
 *   nibble 0xF (15) -> 4 more nibbles form a 16-bit literal word
 *   nibble 8..E     -> 2 more nibbles form a 12-bit index in [2048..3839];
 *                      word = table[index - 1920]
 *   nibble 0..7     -> 1 more nibble forms an 8-bit index in [0..127];
 *                      word = table[index]
 *
 * Iteration count = DecompressedDataWordCount. Each iteration emits
 * one 16-bit word (2 bytes) to the decoded buffer.
 */

typedef struct {
    const uint8_t* data;
    size_t   size;
    size_t   bit_pos;
} FtlNibbleStream;

static int fns_read_nibble(FtlNibbleStream* ns, uint8_t* out) {
    if ((ns->bit_pos >> 3) >= ns->size) return -1;
    size_t byte_idx = ns->bit_pos >> 3;
    int    bit_off  = ns->bit_pos & 7;
    uint8_t byte = ns->data[byte_idx];
    uint8_t n = (uint8_t)((byte >> (4 - bit_off)) & 0x0F);
    ns->bit_pos += 4;
    *out = n;
    return 0;
}

static int decode_hunk_code(const uint8_t* in, size_t in_size,
                            uint8_t** out_buf, size_t* out_size) {
    if (!in || !out_buf || !out_size) return -1;
    *out_buf = NULL;
    *out_size = 0;

    /* Minimum layout: 8-byte header + 3840-byte table = 3848 bytes. */
    if (in_size < 3848u) return -1;

    /* Validate the 0x5223 signature. ReDMCSB DECOMPCO.C explicitly
     * rejects payloads whose first word is not 0x5223. */
    if (rd16_be(in + 0) != 0x5223u) return -1;

    /* Read the DecompressedDataWordCount (32-bit big-endian at offset 4). */
    uint32_t word_count = rd32_be(in + 4);

    /* Sanity: word_count must fit in size_t and not be pathologically
     * huge. We use the same 256 MB cap as FirestaffPak_Decode. */
    if (word_count > 0x08000000u) return -1; /* 128 M words = 256 MB */

    if (word_count == 0u) {
        /* No iterations: succeed with empty output. */
        return 0;
    }

    /* Output size in bytes: word_count * 2. */
    size_t out_bytes = (size_t)word_count * 2u;
    if (out_bytes == 0u || out_bytes > 0x10000000u) return -1;

    uint8_t* out_data = (uint8_t*)malloc(out_bytes);
    if (!out_data) return -1;
    memset(out_data, 0, out_bytes);

    const uint8_t* table_p = in + 8;
    FtlNibbleStream ns;
    ns.data = in + 3848;
    ns.size = in_size - 3848u;
    ns.bit_pos = 0;

    size_t write_pos = 0;

    for (uint32_t i = 0; i < word_count; ++i) {
        uint8_t nib;
        if (fns_read_nibble(&ns, &nib) != 0) {
            free(out_data);
            return -1;
        }

        if (nib == 0x0Fu) {
            /* Literal escape: 4 nibbles -> 16-bit word */
            uint8_t n1, n2, n3, n4;
            if (fns_read_nibble(&ns, &n1) != 0 ||
                fns_read_nibble(&ns, &n2) != 0 ||
                fns_read_nibble(&ns, &n3) != 0 ||
                fns_read_nibble(&ns, &n4) != 0) {
                free(out_data);
                return -1;
            }
            uint16_t word = (uint16_t)((n1 << 12) | (n2 << 8) | (n3 << 4) | n4);
            if (write_pos + 2 > out_bytes) {
                free(out_data);
                return -1;
            }
            out_data[write_pos++] = (uint8_t)(word >> 8);
            out_data[write_pos++] = (uint8_t)(word & 0xFFu);
        } else if (nib >= 0x08u) {
            /* Long dictionary reference: 12-bit index in [2048..3839] */
            uint8_t n1, n2;
            if (fns_read_nibble(&ns, &n1) != 0 ||
                fns_read_nibble(&ns, &n2) != 0) {
                free(out_data);
                return -1;
            }
            uint32_t v = ((uint32_t)nib << 8) |
                         ((uint32_t)n1 << 4) |
                          (uint32_t)n2;
            if (v < 2048u || v > 3839u) {
                free(out_data);
                return -1;
            }
            uint32_t idx = v - 1920u;
            if (idx >= 1920u) {
                free(out_data);
                return -1;
            }
            uint16_t word = rd16_be(table_p + idx * 2u);
            if (write_pos + 2 > out_bytes) {
                free(out_data);
                return -1;
            }
            out_data[write_pos++] = (uint8_t)(word >> 8);
            out_data[write_pos++] = (uint8_t)(word & 0xFFu);
        } else {
            /* Short dictionary reference: 8-bit index in [0..127] */
            uint8_t n1;
            if (fns_read_nibble(&ns, &n1) != 0) {
                free(out_data);
                return -1;
            }
            uint32_t idx = ((uint32_t)nib << 4) | (uint32_t)n1;
            if (idx >= 128u) {
                free(out_data);
                return -1;
            }
            uint16_t word = rd16_be(table_p + idx * 2u);
            if (write_pos + 2 > out_bytes) {
                free(out_data);
                return -1;
            }
            out_data[write_pos++] = (uint8_t)(word >> 8);
            out_data[write_pos++] = (uint8_t)(word & 0xFFu);
        }
    }

    *out_buf = out_data;
    *out_size = out_bytes;
    return 0;
}

/*
 * Bounded diagnostic helper: validate the front-half of a 0x5223
 * HUNK_CODE payload (header size, magic, word-count sanity) and
 * report the consumed-vs-streamed byte distance, without
 * allocating a decoded buffer.  Operators can use this to probe a
 * real FTL HUNK_CODE payload before deciding to fully decode it,
 * or to surface a "stream ends with N pad bytes" gate that
 * downstream consumers can react to.
 *
 * The pad-byte count is computed by simulating the nibble stream
 * position after exactly DecompressedDataWordCount iterations.
 * Each iteration consumes a fixed count of nibbles:
 *   - short-dict (nibble 0..7):  2 nibbles
 *   - long-dict (nibble 8..E):   3 nibbles
 *   - literal (nibble F):        5 nibbles
 * The total bits consumed by the loop is therefore
 *   consumed_bits = 4 * sum(per-iteration nibble count)
 * which depends on the stream content.  Because nibbles are
 * 4-bit aligned, the consumed_bits value is always a multiple
 * of 4.  The count of consumed bytes is ceil(consumed_bits / 8),
 * and the count of *untouched* bytes after the last consumed
 * nibble is stream_byte_size - consumed_bytes.
 *
 * Important caveat documented for operators: nibble consumption
 * is bitwise (not 16-bit word-wise), so the consumed_bytes value
 * can sit 0..7 bits before a byte boundary, leaving 0..3 unused
 * bits in the last partially-consumed byte.  In practice these
 * bits are zero-filled trailer bytes added by the encoder for
 * byte alignment, and the ReDMCSB DECOMPCO.C 68k reader absorbs
 * them silently via its `swap D2` / `clr.w D2` rotation.  We do
 * NOT hide this; the trailing_pad_bits field exposes 0..3 unused
 * bits in the last byte of the stream for callers that need to
 * round the consumed stream to a full byte.
 *
 * Side effects:  none — does not allocate.
 *
 * Returns 1 if the header/table-validity checks passed (status is
 * OK or OK_WITH_PAD), 0 otherwise (status holds the failure
 * reason and out is still zero-initialized).
 */
int FirestaffFtl_HunkCodeDiagnose(const uint8_t* payload,
                                  size_t payload_size,
                                  FirestaffFtlHunkCodeDiagnostics* out) {
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!payload || payload_size < 3848u) {
        if (out) out->status = FIRESTAFF_HUNK_CODE_TOO_SMALL;
        return 0;
    }

    if (rd16_be(payload + 0) != 0x5223u) {
        if (out) out->status = FIRESTAFF_HUNK_CODE_BAD_MAGIC;
        return 0;
    }

    uint32_t word_count = rd32_be(payload + 4);
    if (word_count > 0x08000000u) {
        if (out) out->status = FIRESTAFF_HUNK_CODE_WORD_COUNT_OOR;
        return 0;
    }

    out->declared_word_count = word_count;
    out->stream_byte_size    = payload_size - 3848u;

    /* word_count == 0 is a valid (empty) stream: nothing
     * consumed, the entire stream is padding. */
    if (word_count == 0u) {
        out->status            = FIRESTAFF_HUNK_CODE_OK;
        out->consumed_bits     = 0u;
        out->trailing_pad_bits = 0u;
        out->pad_byte_count    = out->stream_byte_size;
        return 1;
    }

    /* Walk the same stream the decoder would, tracking the bit
     * position.  We do not write to any output buffer: every
     * branch is followed only to advance bit_pos accurately. */
    FtlNibbleStream ns;
    ns.data     = payload + 3848u;
    ns.size     = payload_size - 3848u;
    ns.bit_pos  = 0u;

    int parse_ok = 1;
    for (uint32_t i = 0; i < word_count; ++i) {
        uint8_t nib;
        if (fns_read_nibble(&ns, &nib) != 0) { parse_ok = 0; break; }

        if (nib == 0x0Fu) {
            uint8_t n1, n2, n3, n4;
            if (fns_read_nibble(&ns, &n1) != 0 ||
                fns_read_nibble(&ns, &n2) != 0 ||
                fns_read_nibble(&ns, &n3) != 0 ||
                fns_read_nibble(&ns, &n4) != 0) { parse_ok = 0; break; }
            /* (decode unused): uint16_t word = (n1<<12)|(n2<<8)|(n3<<4)|n4; */
        } else if (nib >= 0x08u) {
            uint8_t n1, n2;
            if (fns_read_nibble(&ns, &n1) != 0 ||
                fns_read_nibble(&ns, &n2) != 0) { parse_ok = 0; break; }
            /* (decode unused): uint32_t v = ((nib<<8)|(n1<<4)|n2) - 1920; */
        } else {
            uint8_t n1;
            if (fns_read_nibble(&ns, &n1) != 0) { parse_ok = 0; break; }
        }
    }

    if (!parse_ok) {
        out->status = FIRESTAFF_HUNK_CODE_TOO_SMALL; /* truncated mid-stream */
        return 0;
    }

    out->consumed_bits = ns.bit_pos;
    /* consumed_bytes is the smallest byte index touched by the
     * stream.  Because nibbles are 4-bit aligned we round up. */
    size_t consumed_bytes = (out->consumed_bits + 7u) >> 3;
    /* Anything past consumed_bytes is untouched stream bytes. */
    if (consumed_bytes <= out->stream_byte_size) {
        out->pad_byte_count = out->stream_byte_size - consumed_bytes;
    } else {
        out->pad_byte_count = 0u;
    }
    /* Bits in the last touched byte that are unused: because each
     * nibble is 4 bits, the unused tail is always a multiple of 4
     * (0 or 4) right now, so callers should read this as a sentinel
     * rather than assume a fractional count.  We still expose it
     * so a future grammar change to bit-level coding surfaces
     * immediately. */
    out->trailing_pad_bits = (size_t)(out->consumed_bits & 3u);

    out->status = (out->pad_byte_count == 0u)
                      ? FIRESTAFF_HUNK_CODE_OK
                      : FIRESTAFF_HUNK_CODE_OK_WITH_PAD;
    return 1;
}

/* ── Public Decode/Free entry points ──────────────────── */

int FirestaffFtl_Decode(FirestaffFtl* ftl) {
    if (!ftl) return -1;

    /* Free any prior decoded buffers to avoid leaks on re-decode. */
    FirestaffFtl_Free(ftl);

    /* HUNK_DATA: zero-run decompression. The slice was already wired
     * during Parse(); just decode it into a freshly allocated buffer.
     * If HUNK_DATA is absent, this step is a no-op (output stays NULL). */
    if (ftl->hunk_data_raw.data != NULL && ftl->hunk_data_raw.size > 0u) {
        size_t uncompressed_size = 0u;
        int rc = hunk_data_expected_size(ftl, &uncompressed_size);
        if (rc != 0) return -1;
        rc = decode_hunk_data(ftl->hunk_data_raw.data,
                              ftl->hunk_data_raw.size,
                              uncompressed_size,
                              &ftl->hunk_data_decoded,
                              &ftl->hunk_data_decoded_size);
        if (rc != 0) return -1;
    }

    /* HUNK_CODE: locate the first CODE segment via the zero-copy
     * slice wired during Parse() and decode it. We accept CODE
     * segments in two physical shapes:
     *   - compressed (0x5223 magic + 8-byte header + 3840-byte
     *     table + nibble stream): decoded via decode_hunk_code.
     *   - uncompressed (no 0x5223 signature): the entire segment
     *     payload is copied verbatim. This matches the FTL container
     *     parser's existing uncompressed-CODE checksum support
     *     (FIRESTAFF_FTL_CHECK_MATCH on the CODE hunk). */
    if (ftl->hunk_code_raw.data != NULL && ftl->hunk_code_raw.size > 0u) {
        const uint8_t* payload = ftl->hunk_code_raw.data;
        size_t payload_size = ftl->hunk_code_raw.size;
        int rc;

        if (payload_size >= 2u &&
            payload[0] == 0x52u && payload[1] == 0x23u) {
            rc = decode_hunk_code(payload, payload_size,
                                  &ftl->hunk_code, &ftl->hunk_code_size);
        } else {
            /* Uncompressed CODE: copy verbatim. */
            ftl->hunk_code = (uint8_t*)malloc(payload_size);
            if (!ftl->hunk_code) return -1;
            memcpy(ftl->hunk_code, payload, payload_size);
            ftl->hunk_code_size = payload_size;
            rc = 0;
        }
        if (rc != 0) {
            FirestaffFtl_Free(ftl);
            return -1;
        }
    }

    return 0;
}

void FirestaffFtl_Free(FirestaffFtl* ftl) {
    if (!ftl) return;
    free(ftl->hunk_data_decoded);
    ftl->hunk_data_decoded = NULL;
    ftl->hunk_data_decoded_size = 0;
    free(ftl->hunk_code);
    ftl->hunk_code = NULL;
    ftl->hunk_code_size = 0;
}

/* ── Self-tests ───────────────────────────────────────── */

#define ST_FAIL(msg) do {                                               \
    fprintf(stderr, "test_firestaff_ftl_decode FAIL: %s\n", msg);       \
    return 0;                                                            \
} while (0)

#define ST_ASSERT(cond, msg) do {                                        \
    if (!(cond)) { fprintf(stderr, "%s:%d: %s (%s)\n",                   \
                            __FILE__, __LINE__, msg, #cond);             \
                   return 0; }                                           \
} while (0)

/*
 * Test bit writer for synthetic FTL HUNK_CODE payloads.
 * Mirrors the one in firestaff_pak_decode.c but writes into a
 * heap buffer with a known capacity so we can build arbitrary
 * compressed streams for round-trip tests.
 */
typedef struct {
    uint8_t* buf;
    size_t   cap;
    size_t   bit_pos;
} FtlTestBitWriter;

static void ftl_tbw_init(FtlTestBitWriter* w, size_t cap) {
    w->buf = (uint8_t*)calloc(1, cap);
    w->cap = cap;
    w->bit_pos = 0;
}

static void ftl_tbw_write_nibble(FtlTestBitWriter* w, uint8_t nib) {
    if ((w->bit_pos >> 3) >= w->cap) return;
    size_t bi = w->bit_pos >> 3;
    int    bo = w->bit_pos & 7;
    uint8_t byte = w->buf[bi];
    byte |= (uint8_t)((nib & 0x0F) << (4 - bo));
    w->buf[bi] = byte;
    w->bit_pos += 4;
}

/*
 * Build a synthetic FTL container with:
 *   - 20-byte common header
 *   - 1 HUNK_BSS segment (zero payload, just metadata)
 *   - 1 HUNK_DATA segment with a simple literal-only zero-run payload
 *   - 1 HUNK_CODE segment with the supplied compressed code stream
 *
 * Caller owns out_buf and out_size and must keep them alive.
 */
static int build_ftl_synthetic(FtlTestBitWriter* code,
                                uint8_t* out, size_t out_cap,
                                size_t* out_size,
                                uint32_t word_count) {
    /* Layout:
     *   0x00..0x13   : 20-byte FTL header
     *   0x14..0x3F   : 3 x 12-byte segment headers
     *   0x40..0x47   : 8-byte HUNK_BSS payload (zero)
     *   0x48..0x4F   : 8-byte HUNK_DATA payload (8 literal bytes)
     *   0x50..       : HUNK_CODE payload (8-byte header + table + code)
     */
    const size_t bss_off = 0x40u;
    const size_t bss_size = 8u;
    const size_t data_off = bss_off + bss_size;
    const size_t data_size = 8u;
    const size_t code_off = data_off + data_size;
    const size_t code_header = 8u;
    const size_t table_bytes = 3840u;
    const size_t code_nibbles = code->bit_pos / 4u;
    const size_t code_bytes = (code_nibbles + 1u) / 2u; /* ceil(nibbles/8) */
    const size_t code_payload_size = code_header + table_bytes + code_bytes;
    const size_t total = code_off + code_payload_size;

    if (total > out_cap) return -1;
    memset(out, 0, total);

    /* Common header */
    wr16_be(out + 0x00, FIRESTAFF_FTL_CONTAINER_MAGIC);  /* 0x6160 */
    wr16_be(out + 0x02, 0x0000u);                         /* checksum (unused) */
    wr16_be(out + 0x04, 0x0002u);                         /* Unknown1 */
    out[0x06] = 0x01u;                                    /* c_6 */
    out[0x07] = 0x00u;                                    /* c_7 */
    wr16_be(out + 0x08, 0x0007u);                         /* i_8 */
    out[0x0a] = 0x00u;                                    /* c_0a */
    out[0x0b] = 0x01u;                                    /* c_0b */
    out[0x0c] = 0x04u;                                    /* c_0c */
    out[0x0d] = 0x01u;                                    /* c_0d */
    wr16_be(out + 0x0e, 0x0021u);                         /* Date1 (1980-01-01) */
    wr16_be(out + 0x10, 0x0021u);                         /* Date2 */
    wr16_be(out + 0x12, 0x0003u);                         /* SegmentCount */

    /* Segment 0: HUNK_BSS */
    uint8_t* s = out + 0x14u;
    wr16_be(s + 0u, FIRESTAFF_HUNK_BSS);
    wr16_be(s + 2u, 0x0000u);
    wr32_be(s + 4u, (uint32_t)bss_off);
    wr32_be(s + 8u, (uint32_t)bss_size);

    /* Segment 1: HUNK_DATA */
    s = out + 0x14u + 12u;
    wr16_be(s + 0u, FIRESTAFF_HUNK_DATA);
    wr16_be(s + 2u, 0x0000u);
    wr32_be(s + 4u, (uint32_t)data_off);
    wr32_be(s + 8u, (uint32_t)data_size);

    /* Segment 2: HUNK_CODE */
    s = out + 0x14u + 24u;
    wr16_be(s + 0u, FIRESTAFF_HUNK_CODE);
    wr16_be(s + 2u, 0x0000u);
    wr32_be(s + 4u, (uint32_t)code_off);
    wr32_be(s + 8u, (uint32_t)code_payload_size);

    /* HUNK_BSS offset 4: HUNK_DATA area_1 in-memory size. */
    wr32_be(out + bss_off + 4u, 8u);

    /* HUNK_DATA payload: literal pairs "ABCDEFGH" per Greatstone Note 7. */
    uint8_t* data_p = out + data_off;
    memcpy(data_p, "ABCDEFGH", 8);

    /* HUNK_CODE payload: 8-byte header + 3840-byte table + nibble stream */
    uint8_t* code_p = out + code_off;
    wr16_be(code_p + 0u, 0x5223u);                       /* signature */
    wr16_be(code_p + 2u, 0x0000u);                       /* padding */
    wr32_be(code_p + 4u, word_count);                    /* DecompressedDataWordCount */

    /* 1920-word frequency table with deterministic content */
    uint8_t* table_p = code_p + 8;
    for (int i = 0; i < 1920; ++i) {
        uint8_t hi = (uint8_t)((i >> 4) & 0xFFu);
        uint8_t lo = (uint8_t)((i & 0x0Fu) | 0x80u);
        table_p[i * 2 + 0] = hi;
        table_p[i * 2 + 1] = lo;
    }

    /* Nibble stream */
    memcpy(code_p + code_header + table_bytes, code->buf, code_bytes);

    *out_size = total;
    return 0;
}

static int test_parse_minimal(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* One short-dict ref (nibble 0x5 + nibble 0xA = idx 90). */
    ftl_tbw_write_nibble(&code, 0x5);
    ftl_tbw_write_nibble(&code, 0xA);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build_ftl_synthetic");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse returned non-zero");
    ST_ASSERT(ftl.header.magic == 0x6160u, "magic");
    ST_ASSERT(ftl.header.unknown1 == 0x0002u, "unknown1");
    ST_ASSERT(ftl.header.c_6 == 0x01u, "c_6");
    ST_ASSERT(ftl.header.c_7 == 0x00u, "c_7");
    ST_ASSERT(ftl.header.segment_count == 3u, "segment_count");
    ST_ASSERT(ftl.segment_count_parsed == 3u, "parsed");
    ST_ASSERT(ftl.segments[0].type == FIRESTAFF_HUNK_BSS, "seg0 type");
    ST_ASSERT(ftl.segments[1].type == FIRESTAFF_HUNK_DATA, "seg1 type");
    ST_ASSERT(ftl.segments[2].type == FIRESTAFF_HUNK_CODE, "seg2 type");
    ST_ASSERT(ftl.hunk_bss.data != NULL, "hunk_bss wired");
    ST_ASSERT(ftl.hunk_data_raw.data != NULL, "hunk_data_raw wired");
    free(code.buf);
    return 1;
}

static int test_parse_bad_magic(void) {
    uint8_t buf[64] = {0};
    FirestaffFtl ftl;
    int rc = FirestaffFtl_Parse(buf, sizeof(buf), &ftl);
    ST_ASSERT(rc == -2, "bad magic rejected");
    return 1;
}

static int test_parse_bad_invariants(void) {
    uint8_t buf[64] = {0};
    wr16_be(buf + 0x00, FIRESTAFF_FTL_CONTAINER_MAGIC);
    wr16_be(buf + 0x04, 0xFFFFu); /* Unknown1 != 2 */
    FirestaffFtl ftl;
    int rc = FirestaffFtl_Parse(buf, sizeof(buf), &ftl);
    ST_ASSERT(rc == -3, "bad invariants rejected");
    return 1;
}

static int test_parse_truncated(void) {
    uint8_t buf[16] = {0};
    FirestaffFtl ftl;
    int rc = FirestaffFtl_Parse(buf, sizeof(buf), &ftl);
    ST_ASSERT(rc == -1, "truncated rejected");
    return 1;
}

static int test_parse_segment_collision(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    /* Force a collision: seg 1 (DATA) -> same type+id as seg 0 (BSS) */
    wr16_be(buf + 0x14u + 12u + 0u, FIRESTAFF_HUNK_BSS);
    wr16_be(buf + 0x14u + 12u + 2u, 0x0000u);

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    ST_ASSERT(rc == -5, "collision rejected");
    free(code.buf);
    return 1;
}

static int test_parse_segment_out_of_bounds(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    /* Force BSS offset to a huge value */
    wr32_be(buf + 0x14u + 4u, 0xFFFFFF00u);

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    ST_ASSERT(rc == -6, "out-of-bounds rejected");
    free(code.buf);
    return 1;
}

/*
 * Round-trip tests for the HUNK_CODE 0x5223 decoder. Each test
 * builds a synthetic FTL container whose HUNK_CODE carries a
 * known compressed stream, then verifies the decoded output.
 *
 * The synthetic frequency table maps index i to bytes ((i>>4) & 0xFF)
 * and ((i & 0x0F) | 0x80). This makes every index decode to a
 * distinguishable 2-byte word, so the round-trip assertions below
 * are precise.
 */

static int test_decode_hunk_code_short_dict(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* Specific: short-dict idx 90 (0x5A). table[90] = (5, 0x8A). */
    ftl_tbw_write_nibble(&code, 0x5);
    ftl_tbw_write_nibble(&code, 0xA);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse");

    /* Locate the HUNK_CODE segment and decode it directly. */
    const FirestaffFtlSegmentHeader* seg = NULL;
    for (uint16_t i = 0; i < ftl.segment_count_parsed; ++i) {
        if (ftl.segments[i].type == FIRESTAFF_HUNK_CODE) {
            seg = &ftl.segments[i];
            break;
        }
    }
    ST_ASSERT(seg != NULL, "code segment located");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + seg->offset, seg->size,
                          &decoded, &decoded_size);
    if (rc != 0) ST_FAIL("decode_hunk_code");
    ST_ASSERT(decoded_size == 2u, "one word = 2 bytes");
    ST_ASSERT(decoded[0] == 0x05u, "first byte = hi(idx 90)");
    ST_ASSERT(decoded[1] == 0x8Au, "second byte = lo(idx 90)");
    free(decoded);
    free(code.buf);
    return 1;
}

static int test_decode_hunk_code_long_dict(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* Long-dict idx 1150: nibble 0xB + 0xF + 0xE = 0xBFE = 3070.
     * table[1150] = (0x47, 0x8E). */
    ftl_tbw_write_nibble(&code, 0xB);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xE);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse");

    const FirestaffFtlSegmentHeader* seg = NULL;
    for (uint16_t i = 0; i < ftl.segment_count_parsed; ++i) {
        if (ftl.segments[i].type == FIRESTAFF_HUNK_CODE) {
            seg = &ftl.segments[i];
            break;
        }
    }
    ST_ASSERT(seg != NULL, "code segment located");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + seg->offset, seg->size,
                          &decoded, &decoded_size);
    if (rc != 0) ST_FAIL("decode_hunk_code");
    ST_ASSERT(decoded_size == 2u, "one word = 2 bytes");
    ST_ASSERT(decoded[0] == 0x47u, "long-dict first byte");
    ST_ASSERT(decoded[1] == 0x8Eu, "long-dict second byte");
    free(decoded);
    free(code.buf);
    return 1;
}

static int test_decode_hunk_code_literal(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* Literal 0xDEAD: nibbles F, D, E, A, D -> word 0xDEAD */
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xD);
    ftl_tbw_write_nibble(&code, 0xE);
    ftl_tbw_write_nibble(&code, 0xA);
    ftl_tbw_write_nibble(&code, 0xD);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse");

    const FirestaffFtlSegmentHeader* seg = NULL;
    for (uint16_t i = 0; i < ftl.segment_count_parsed; ++i) {
        if (ftl.segments[i].type == FIRESTAFF_HUNK_CODE) {
            seg = &ftl.segments[i];
            break;
        }
    }
    ST_ASSERT(seg != NULL, "code segment located");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + seg->offset, seg->size,
                          &decoded, &decoded_size);
    if (rc != 0) ST_FAIL("decode_hunk_code");
    ST_ASSERT(decoded_size == 2u, "one word = 2 bytes");
    ST_ASSERT(decoded[0] == 0xDEu, "literal first byte");
    ST_ASSERT(decoded[1] == 0xADu, "literal second byte");
    free(decoded);
    free(code.buf);
    return 1;
}

static int test_decode_hunk_code_bad_signature(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 1);
    if (rc != 0) ST_FAIL("build");

    /* Corrupt the 0x5223 signature. */
    buf[0x50u + 0u] = 0x12u;
    buf[0x50u + 1u] = 0x34u;

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + 0x50u, 3848u, &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "bad signature rejected");
    return 1;
}

static int test_decode_hunk_code_truncated(void) {
    uint8_t buf[100] = {0};
    wr16_be(buf + 0u, 0x5223u);
    wr32_be(buf + 4u, 100u); /* claim 100 words */
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_code(buf, sizeof(buf), &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "truncated rejected");
    return 1;
}

static int test_decode_hunk_code_zero_word_count(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 0);
    if (rc != 0) ST_FAIL("build");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + 0x50u, 3848u, &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "zero word count succeeds");
    ST_ASSERT(decoded == NULL, "no allocation");
    ST_ASSERT(decoded_size == 0u, "no output");
    free(code.buf);
    return 1;
}

/*
 * Round-trip: build a mixed stream of short-dict + literal + long-dict
 * commands and verify the decoded bytes match the expected sequence.
 */
static int test_decode_hunk_code_mixed_round_trip(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 8192);
    /* Three commands:
     *   1. short-dict idx 7    -> word (0, 0x87) [from table[7] = (0, 0x87)]
     *   2. literal 0xCAFE      -> word (0xCA, 0xFE)
     *   3. long-dict idx 1920  -> word (0xC0, 0x80) [from table[1920]]
     */
    ftl_tbw_write_nibble(&code, 0x0);
    ftl_tbw_write_nibble(&code, 0x7);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xC);
    ftl_tbw_write_nibble(&code, 0xA);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xE);
    /* long-dict: 12-bit value 1920+128 = 2048 -> nibble 8, n1 0, n2 0 */
    ftl_tbw_write_nibble(&code, 0x8);
    ftl_tbw_write_nibble(&code, 0x0);
    ftl_tbw_write_nibble(&code, 0x0);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 3);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse");

    const FirestaffFtlSegmentHeader* seg = NULL;
    for (uint16_t i = 0; i < ftl.segment_count_parsed; ++i) {
        if (ftl.segments[i].type == FIRESTAFF_HUNK_CODE) {
            seg = &ftl.segments[i];
            break;
        }
    }
    ST_ASSERT(seg != NULL, "code segment located");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + seg->offset, seg->size,
                          &decoded, &decoded_size);
    if (rc != 0) ST_FAIL("decode_hunk_code");
    ST_ASSERT(decoded_size == 6u, "three words = 6 bytes");
    /* idx 7 -> (0, 0x87) */
    ST_ASSERT(decoded[0] == 0x00u, "short-dict hi");
    ST_ASSERT(decoded[1] == 0x87u, "short-dict lo");
    /* literal 0xCAFE */
    ST_ASSERT(decoded[2] == 0xCAu, "literal hi");
    ST_ASSERT(decoded[3] == 0xFEu, "literal lo");
    /* long-dict idx 128 -> table[128] = ((128>>4)&0xFF, (128&0x0F)|0x80)
       = (0x08, 0x80) */
    ST_ASSERT(decoded[4] == 0x08u, "long-dict hi");
    ST_ASSERT(decoded[5] == 0x80u, "long-dict lo");
    free(decoded);
    free(code.buf);
    return 1;
}

/* ── HUNK_DATA zero-run tests ─────────────────────────── */

static int test_decode_hunk_data_literal(void) {
    uint8_t in[] = { 'A','B','C','D','E','F','G','H' };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), 8u, &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "decode ok");
    ST_ASSERT(decoded_size == 8u, "8 bytes decoded");
    ST_ASSERT(decoded[0] == 'A', "[0]");
    ST_ASSERT(decoded[7] == 'H', "[7]");
    free(decoded);
    return 1;
}

static int test_decode_hunk_data_zero_run(void) {
    uint8_t in[] = {
        'A','B',
        0x00u, 0x00u, 0x00u, 0x03u,
        'C','D'
    };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), 9u, &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "decode ok");
    ST_ASSERT(decoded_size == 9u, "2 + 5 + 2 bytes");
    ST_ASSERT(decoded[0] == 'A', "[0]");
    ST_ASSERT(decoded[1] == 'B', "[1]");
    ST_ASSERT(decoded[2] == 0x00u, "[2] zero");
    ST_ASSERT(decoded[6] == 0x00u, "[6] zero");
    ST_ASSERT(decoded[7] == 'C', "[7]");
    ST_ASSERT(decoded[8] == 'D', "[8]");
    free(decoded);
    return 1;
}

static int test_decode_hunk_data_empty(void) {
    uint8_t* decoded = NULL;
    size_t   decoded_size = 99;
    int rc = decode_hunk_data(NULL, 0, 0u, &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "empty ok");
    ST_ASSERT(decoded == NULL, "no alloc");
    ST_ASSERT(decoded_size == 0u, "size 0");
    return 1;
}

static int test_decode_hunk_data_truncated_run_header(void) {
    uint8_t in[] = { 0x00u, 0x00u };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), 4u, &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "truncated run rejected");
    return 1;
}

static int test_decode_hunk_data_odd_input(void) {
    uint8_t in[] = { 'A' };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), 1u, &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "odd input rejected");
    return 1;
}

/*
 * 16-word mixed round-trip test: exercises every nibble-command
 * class and verifies the decoder scales beyond a 1-3 word smoke
 * test.  The synthetic frequency table maps index i to bytes
 * ((i>>4) & 0xFF), ((i & 0x0F) | 0x80), so every word output
 * is predictable.
 */
static int test_decode_hunk_code_long_mixed_round_trip(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* 16 words, chosen so each nibble-command class appears and
     * every chosen index has the expected output word. */
    /* short-dict idx 7 -> (0x00, 0x87) */
    ftl_tbw_write_nibble(&code, 0x0); ftl_tbw_write_nibble(&code, 0x7);
    /* short-dict idx 90 -> (0x05, 0x8A) */
    ftl_tbw_write_nibble(&code, 0x5); ftl_tbw_write_nibble(&code, 0xA);
    /* literal 0xCAFE -> (0xCA, 0xFE) */
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xC);
    ftl_tbw_write_nibble(&code, 0xA);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xE);
    /* long-dict idx 128 -> (0x08, 0x80) */
    ftl_tbw_write_nibble(&code, 0x8);
    ftl_tbw_write_nibble(&code, 0x0);
    ftl_tbw_write_nibble(&code, 0x0);
    /* long-dict idx 1150 -> (0x47, 0x8E) */
    ftl_tbw_write_nibble(&code, 0xB);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xE);
    /* literal 0xDEAD -> (0xDE, 0xAD) */
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xD);
    ftl_tbw_write_nibble(&code, 0xE);
    ftl_tbw_write_nibble(&code, 0xA);
    ftl_tbw_write_nibble(&code, 0xD);
    /* short-dict idx 0 -> (0x00, 0x80) */
    ftl_tbw_write_nibble(&code, 0x0); ftl_tbw_write_nibble(&code, 0x0);
    /* short-dict idx 127 -> (0x07, 0xFF) */
    ftl_tbw_write_nibble(&code, 0x7); ftl_tbw_write_nibble(&code, 0xF);
    /* long-dict idx 1919 -> (0x77, 0x8F); v=3839=0xEFF */
    ftl_tbw_write_nibble(&code, 0xE);
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xF);
    /* literal 0x600D -> (0x60, 0x0D) */
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0x6);
    ftl_tbw_write_nibble(&code, 0x0);
    ftl_tbw_write_nibble(&code, 0x0);
    ftl_tbw_write_nibble(&code, 0xD);
    /* short-dict idx 42 -> (0x02, 0xAA) */
    ftl_tbw_write_nibble(&code, 0x2); ftl_tbw_write_nibble(&code, 0xA);
    /* short-dict idx 99 -> (0x06, 0xA3) */
    ftl_tbw_write_nibble(&code, 0x6); ftl_tbw_write_nibble(&code, 0x3);
    /* long-dict idx 1500 -> table[1500]=(0x5B,0x8C); v=3420=0xD5C */
    ftl_tbw_write_nibble(&code, 0xD);
    ftl_tbw_write_nibble(&code, 0x5);
    ftl_tbw_write_nibble(&code, 0xC);
    /* literal 0xBEEF -> (0xBE, 0xEF) */
    ftl_tbw_write_nibble(&code, 0xF);
    ftl_tbw_write_nibble(&code, 0xB);
    ftl_tbw_write_nibble(&code, 0xE);
    ftl_tbw_write_nibble(&code, 0xE);
    ftl_tbw_write_nibble(&code, 0xF);
    /* short-dict idx 64 -> (0x04, 0xC0) */
    ftl_tbw_write_nibble(&code, 0x4); ftl_tbw_write_nibble(&code, 0x0);
    /* short-dict idx 12 -> (0x00, 0x8C) */
    ftl_tbw_write_nibble(&code, 0x0); ftl_tbw_write_nibble(&code, 0xC);

    /* Build a synthetic FTL container, then decode the
     * HUNK_CODE segment through the public path.  Word count
     * here is exactly 16 (we counted the comments above). */
    static const uint8_t expected[32] = {
        0x00u, 0x87u,             /* short-dict idx 7 */
        0x05u, 0x8Au,             /* short-dict idx 90 */
        0xCAu, 0xFEu,             /* literal 0xCAFE */
        0x08u, 0x80u,             /* long-dict idx 128 */
        0x47u, 0x8Eu,             /* long-dict idx 1150 */
        0xDEu, 0xADu,             /* literal 0xDEAD */
        0x00u, 0x80u,             /* short-dict idx 0 */
        0x07u, 0xFFu,             /* short-dict idx 127 */
        0x77u, 0x8Fu,             /* long-dict idx 1919 */
        0x60u, 0x0Du,             /* literal 0x600D */
        0x02u, 0xAAu,             /* short-dict idx 42 */
        0x06u, 0xA3u,             /* short-dict idx 99 */
        0x5Bu, 0x8Cu,             /* long-dict idx 1500 */
        0xBEu, 0xEFu,             /* literal 0xBEEF */
        0x04u, 0xC0u,             /* short-dict idx 64 */
        0x00u, 0x8Cu              /* short-dict idx 12 */
    };

    /* 16 distinct 2-byte words = 32 bytes of CODE.  The
     * total CODE payload size must accommodate the 8-byte
     * header + 3840-byte table + nibble stream ceiling. */
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 16);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtl ftl;
    rc = FirestaffFtl_Parse(buf, buf_size, &ftl);
    if (rc != 0) ST_FAIL("Parse");

    /* Locate the HUNK_CODE segment and decode it via the
     * parser-wired offset/size, mirroring the other
     * decoder tests. */
    const FirestaffFtlSegmentHeader* seg = NULL;
    for (uint16_t i = 0; i < ftl.segment_count_parsed; ++i) {
        if (ftl.segments[i].type == FIRESTAFF_HUNK_CODE) {
            seg = &ftl.segments[i];
            break;
        }
    }
    ST_ASSERT(seg != NULL, "code segment located");
    ST_ASSERT(seg->size >= 3848u, "CODE segment >= 3848 bytes");

    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    rc = decode_hunk_code(buf + seg->offset, seg->size,
                          &decoded, &decoded_size);
    if (rc != 0) ST_FAIL("decode_hunk_code");
    ST_ASSERT(decoded_size == 32u, "32 bytes decoded");
    ST_ASSERT(memcmp(decoded, expected, 32u) == 0, "decoded bytes match");
    free(decoded);
    free(code.buf);
    return 1;
}

/*
 * Diagnostic helper: an exactly-on-boundary stream reports
 * OK with pad_byte_count = 0.  16 short-dict refs = 32 nibbles
 * = 16 bytes, no padding trailer.
 */
static int test_diagnose_clean_stream(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    for (int i = 0; i < 16; ++i) {
        ftl_tbw_write_nibble(&code, 0x5); /* short-dict idx 5x */
        ftl_tbw_write_nibble(&code, 0xA);
    }
    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 16);
    if (rc != 0) ST_FAIL("build");

    FirestaffFtlHunkCodeDiagnostics d;
    int ok = FirestaffFtl_HunkCodeDiagnose(buf + 0x50u, 3848u + 16u, &d);
    ST_ASSERT(ok == 1, "diagnose succeeded");
    ST_ASSERT(d.status == FIRESTAFF_HUNK_CODE_OK, "OK status");
    ST_ASSERT(d.declared_word_count == 16u, "word count");
    ST_ASSERT(d.stream_byte_size == 16u, "stream byte size");
    ST_ASSERT(d.consumed_bits == 128u, "consumed 32 nibbles = 128 bits");
    ST_ASSERT(d.pad_byte_count == 0u, "no trailer");
    free(code.buf);
    return 1;
}

/*
 * Diagnostic helper: a stream whose content lands inside a
 * byte (not on a byte boundary) reports OK_WITH_PAD with
 * pad_byte_count > 0.  Builds an 8-word stream of literal
 * escapes (= 5 nibbles each = 40 nibbles = 5 bytes; the
 * 4-byte alignment means exactly 0 leftover bits but 0
 * pad bytes), then re-runs with a 1-byte pad trailer to
 * force pad_byte_count > 0.
 */
static int test_diagnose_padded_stream(void) {
    FtlTestBitWriter code; ftl_tbw_init(&code, 4096);
    /* 8 literal 0xC0DE commands -> 40 nibbles = 20 bytes. */
    for (int i = 0; i < 8; ++i) {
        ftl_tbw_write_nibble(&code, 0xF);
        ftl_tbw_write_nibble(&code, 0xC);
        ftl_tbw_write_nibble(&code, 0x0);
        ftl_tbw_write_nibble(&code, 0xD);
        ftl_tbw_write_nibble(&code, 0xE);
    }
    /* Append a 1-byte pad trailer to the stream. */
    ftl_tbw_write_nibble(&code, 0x0); /* nibble in a partial byte */
    ftl_tbw_write_nibble(&code, 0x0);

    uint8_t buf[8192];
    size_t buf_size = 0;
    int rc = build_ftl_synthetic(&code, buf, sizeof(buf), &buf_size, 8);
    if (rc != 0) ST_FAIL("build");

    /* The build helper rounds the code up to ceil(nibbles/2)
     * bytes (one nibble per bit) so the stream is the
     * original 42 nibbles / 4 = 10.5 -> 11 bytes.  The
     * payload is therefore 3848 + 11 = 3859 bytes.  The
     * decoder only needs 40 nibbles for 8 words, leaving
     * 2 unused nibbles = 1 byte of padding. */
    FirestaffFtlHunkCodeDiagnostics d;
    int ok = FirestaffFtl_HunkCodeDiagnose(buf + 0x50u, 3869u, &d);
    ST_ASSERT(ok == 1, "diagnose succeeded");
    ST_ASSERT(d.status == FIRESTAFF_HUNK_CODE_OK_WITH_PAD,
              "OK_WITH_PAD");
    ST_ASSERT(d.stream_byte_size == 21u, "21 stream bytes");
    ST_ASSERT(d.consumed_bits == 160u, "8 literals = 40 nibbles = 160 bits");
    ST_ASSERT(d.pad_byte_count >= 1u, ">=1 pad byte");
    free(code.buf);
    return 1;
}

/*
 * Diagnostic helper: a too-small payload is reported as
 * TOO_SMALL without attempting to read the table or stream.
 */
static int test_diagnose_too_small(void) {
    FirestaffFtlHunkCodeDiagnostics d;
    /* 3847-byte payload: 1 byte short of the 8-byte header +
     * 3840-byte table minimum. */
    uint8_t tiny[3847];
    memset(tiny, 0, sizeof(tiny));
    int ok = FirestaffFtl_HunkCodeDiagnose(tiny, sizeof(tiny), &d);
    ST_ASSERT(ok == 0, "too-small rejected");
    ST_ASSERT(d.status == FIRESTAFF_HUNK_CODE_TOO_SMALL,
              "TOO_SMALL status");
    return 1;
}

/*
 * Diagnostic helper: a payload whose first word is not
 * 0x5223 is reported as BAD_MAGIC without trying to read
 * the word count or stream.
 */
static int test_diagnose_bad_magic(void) {
    uint8_t buf[3848 + 4];
    memset(buf, 0, sizeof(buf));
    /* Bad magic: 0x0000 instead of 0x5223. */
    FirestaffFtlHunkCodeDiagnostics d;
    int ok = FirestaffFtl_HunkCodeDiagnose(buf, sizeof(buf), &d);
    ST_ASSERT(ok == 0, "bad-magic rejected");
    ST_ASSERT(d.status == FIRESTAFF_HUNK_CODE_BAD_MAGIC,
              "BAD_MAGIC status");
    return 1;
}

int FirestaffFtl_SelfTest(void) {
    int total = 0, passed = 0;
#define RUN(name) do { total++; if (name()) passed++; } while (0)
    RUN(test_parse_minimal);
    RUN(test_parse_bad_magic);
    RUN(test_parse_bad_invariants);
    RUN(test_parse_truncated);
    RUN(test_parse_segment_collision);
    RUN(test_parse_segment_out_of_bounds);
    RUN(test_decode_hunk_code_short_dict);
    RUN(test_decode_hunk_code_long_dict);
    RUN(test_decode_hunk_code_literal);
    RUN(test_decode_hunk_code_bad_signature);
    RUN(test_decode_hunk_code_truncated);
    RUN(test_decode_hunk_code_zero_word_count);
    RUN(test_decode_hunk_code_mixed_round_trip);
    RUN(test_decode_hunk_code_long_mixed_round_trip);
    RUN(test_diagnose_clean_stream);
    RUN(test_diagnose_padded_stream);
    RUN(test_diagnose_too_small);
    RUN(test_diagnose_bad_magic);
    RUN(test_decode_hunk_data_literal);
    RUN(test_decode_hunk_data_zero_run);
    RUN(test_decode_hunk_data_empty);
    RUN(test_decode_hunk_data_truncated_run_header);
    RUN(test_decode_hunk_data_odd_input);
#undef RUN
    return (passed == total) ? 0 : -1;
}

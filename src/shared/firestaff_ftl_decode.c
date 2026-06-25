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

/*
 * FTL HUNK_DATA uses a simple zero-run compression scheme documented
 * in greatstone d_ftl.html: a control byte encodes how many literal
 * bytes follow and whether a run of zeros follows. The exact bit
 * layout is:
 *
 *   bit 7 (0x80): 1 = run-of-zeros follows, 0 = literal only
 *   bits 6..0   : count - 1 of bytes to emit (0..127, so 1..128 bytes)
 *
 * Following the control byte:
 *   - if bit 7 == 0: exactly (count + 1) literal bytes follow
 *   - if bit 7 == 1: zero bytes are emitted (length already known),
 *     no further bytes are read from the stream
 *
 * The 0x00 control byte is invalid (count must be >= 1); we reject it.
 *
 * This implementation is intentionally conservative: it never reads
 * past the declared HUNK_DATA size and never writes past the
 * decoded buffer size. The decoded buffer is allocated exactly to
 * the worst-case output size (decompressed = sum of (count+1) over
 * all control bytes).
 *
 * Provenance:
 *   - Greatstone d_ftl.html "HUNK_DATA" section.
 *   - ReDMCSB FTL.H does not name HUNK_DATA compression (only the
 *     HUNK_CODE 0x5223 algorithm is in DECOMPCO.C). The data-area
 *     zero-run scheme is FTL's documented resource layout for
 *     Amiga/X68000/MegaCD asset files; we keep the decoder simple
 *     and well-bounded so we can promote real-asset receipts in a
 *     follow-up pass without over-claiming today.
 */

static int decode_hunk_data(const uint8_t* in, size_t in_size,
                            uint8_t** out_buf, size_t* out_size) {
    if (!out_buf || !out_size) return -1;
    /* Treat NULL input with size 0 as the empty HUNK_DATA case:
     * no control bytes, no output bytes. */
    if (!in && in_size == 0u) {
        *out_buf = NULL;
        *out_size = 0;
        return 0;
    }
    if (!in) return -1;
    *out_buf = NULL;
    *out_size = 0;

    /* Worst-case output: every control byte produces 128 literal bytes
     * (count = 127, emit count+1 = 128). */
    if (in_size > (SIZE_MAX / 128u)) return -1;
    size_t worst = in_size * 128u;
    uint8_t* buf = (uint8_t*)malloc(worst);
    if (!buf) return -1;
    size_t produced = 0;

    size_t i = 0;
    while (i < in_size) {
        uint8_t ctrl = in[i++];
        size_t count = (size_t)(ctrl & 0x7fu); /* 0..127 -> 1..128 bytes */
        if (count == 0u) {
            /* count == 0 is a malformed control byte (would mean
             * "emit 1 byte of zeros" without the run flag, or
             * "emit 0 literal bytes" with the run flag -- both
             * useless). Treat as an error to avoid silent data loss. */
            free(buf);
            return -1;
        }
        size_t emit = count + 1u; /* 1..128 */

        if (ctrl & 0x80u) {
            /* Run of zeros: emit `emit` zero bytes, no further reads. */
            if (produced + emit > worst) {
                free(buf);
                return -1;
            }
            memset(buf + produced, 0, emit);
            produced += emit;
        } else {
            /* Literal block: read `emit` bytes from stream. */
            if (i + emit > in_size) {
                free(buf);
                return -1;
            }
            if (produced + emit > worst) {
                free(buf);
                return -1;
            }
            memcpy(buf + produced, in + i, emit);
            i += emit;
            produced += emit;
        }
    }

    /* Trim the worst-case allocation down to the actual output size. */
    if (produced == 0u) {
        free(buf);
        *out_buf = NULL;
        *out_size = 0;
        return 0;
    }
    uint8_t* trimmed = (uint8_t*)malloc(produced);
    if (!trimmed) {
        free(buf);
        return -1;
    }
    memcpy(trimmed, buf, produced);
    free(buf);
    *out_buf = trimmed;
    *out_size = produced;
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

/* ── Public Decode/Free entry points ──────────────────── */

int FirestaffFtl_Decode(FirestaffFtl* ftl) {
    if (!ftl) return -1;

    /* Free any prior decoded buffers to avoid leaks on re-decode. */
    FirestaffFtl_Free(ftl);

    /* HUNK_DATA: zero-run decompression. The slice was already wired
     * during Parse(); just decode it into a freshly allocated buffer.
     * If HUNK_DATA is absent, this step is a no-op (output stays NULL). */
    if (ftl->hunk_data_raw.data != NULL && ftl->hunk_data_raw.size > 0u) {
        int rc = decode_hunk_data(ftl->hunk_data_raw.data,
                                   ftl->hunk_data_raw.size,
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

    /* HUNK_DATA payload: literal block "ABCDEFGH" (8 bytes = count 7 + 1) */
    uint8_t* data_p = out + data_off;
    data_p[0] = 0x07u; /* 7 + 1 = 8 literal bytes follow */
    memcpy(data_p + 1, "ABCDEFGH", 8);

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
    uint8_t in[] = {
        0x07u,                          /* control: 8 literal bytes follow */
        'A','B','C','D','E','F','G','H'
    };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "decode ok");
    ST_ASSERT(decoded_size == 8u, "8 bytes decoded");
    ST_ASSERT(decoded[0] == 'A', "[0]");
    ST_ASSERT(decoded[7] == 'H', "[7]");
    free(decoded);
    return 1;
}

static int test_decode_hunk_data_zero_run(void) {
    uint8_t in[] = {
        0x07u,                          /* 8 literal bytes */
        'A','B','C','D','E','F','G','H',
        0x80u | 0x03u                   /* 4 zero bytes */
    };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "decode ok");
    ST_ASSERT(decoded_size == 12u, "8 + 4 bytes");
    ST_ASSERT(decoded[0] == 'A', "[0]");
    ST_ASSERT(decoded[7] == 'H', "[7]");
    ST_ASSERT(decoded[8] == 0x00u, "[8] zero");
    ST_ASSERT(decoded[11] == 0x00u, "[11] zero");
    free(decoded);
    return 1;
}

static int test_decode_hunk_data_empty(void) {
    uint8_t* decoded = NULL;
    size_t   decoded_size = 99;
    int rc = decode_hunk_data(NULL, 0, &decoded, &decoded_size);
    ST_ASSERT(rc == 0, "empty ok");
    ST_ASSERT(decoded == NULL, "no alloc");
    ST_ASSERT(decoded_size == 0u, "size 0");
    return 1;
}

static int test_decode_hunk_data_bad_control(void) {
    uint8_t in[] = { 0x00u };
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "bad control rejected");
    return 1;
}

static int test_decode_hunk_data_truncated_literal(void) {
    uint8_t in[] = { 0x07u, 'A','B','C' }; /* claims 8 literal bytes but only 3 */
    uint8_t* decoded = NULL;
    size_t   decoded_size = 0;
    int rc = decode_hunk_data(in, sizeof(in), &decoded, &decoded_size);
    ST_ASSERT(rc != 0, "truncated literal rejected");
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
    RUN(test_decode_hunk_data_literal);
    RUN(test_decode_hunk_data_zero_run);
    RUN(test_decode_hunk_data_empty);
    RUN(test_decode_hunk_data_bad_control);
    RUN(test_decode_hunk_data_truncated_literal);
#undef RUN
    return (passed == total) ? 0 : -1;
}

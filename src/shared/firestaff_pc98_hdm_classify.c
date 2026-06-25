/*
 * firestaff_pc98_hdm_classify.c
 *
 * Implementation of the read-only PC-9801 HDM/floppy image
 * classifier for the DM1 PC-9801 (and CSB PC-9801) import gap.
 *
 * The classifier deliberately stays at the bytes-in / struct-out
 * level so it stays data-free in unit tests and never touches
 * a real disk image. Synthetic fixtures are used to exercise
 * the documented DM1 2.0a-original / 2.0a-cracked / 2.0b-original
 * boundaries (DMWeb PC-9801 edition page) plus the CSB 3.1
 * original/cracked boundary (DMWeb CSB PC-9801 edition page).
 *
 * Source references (all used here):
 *   - DMWeb DM PC-9801 edition page:
 *       2.0a-original is unbootable without its copy-protection
 *       sector; the cracked 2.0a patches NECIO.EXE @ 0x1CF1 and
 *       FIRES.EXE @ 0x2636D / 0x263A3 / 0x263BD; 2.0b is not
 *       copy-protected and does not need cracking.
 *   - DMWeb CSB PC-9801 edition page:
 *       CSBGAME.EXE protection offsets for the v3.1 line.
 *   - dmweb community/documentation/copy-protection:
 *       PC-9801 uses a "no flux area" copy-protection scheme on
 *       a specific track/sector; the sector may be missing from
 *       dumped images, which is exactly what "not bootable"
 *       means for the dmweb-distributed 2.0a-original image.
 *   - pc98-disk-tools (barbeque GitHub):
 *       HDM is a raw dd-style image. FDI is HDM with a 4096-byte
 *       header prepended. The classifier handles both shapes.
 *   - pc98-disk-tools is_fdi.py + dmweb PC-9801 page:
 *       Standard 2HD floppy = 1024 B * 8 sectors * 2 sides * 77
 *       tracks = 1,261,568 bytes. 2DD floppy = 512 B * 8 * 2 * 77
 *       = 737,280 bytes. These are the only two sizes the
 *       DM1/CSB PC-9801 floppy line ships in (dmweb PC-9801
 *       edition page lists 3.5" and 5.25" editions of each).
 *
 * Scope (kept narrow on purpose):
 *   - No file system mount, no FAT parsing, no sector read.
 *   - The "directory entries" we report are byte offsets where
 *     the synthetic 8.3 file-name signature was located in the
 *     fixture buffer. On a real HDM that would correspond to
 *     the FAT directory slot of the matching file. This is a
 *     fingerprint, not a runtime handoff.
 *   - No emulator wiring, no copy-protection sector decode,
 *     no launch claim. Those are tracked separately in
 *     docs/FIRESTAFF_GAP_LIST.md A1 row "DM1 PC-9801 HDM/floppy
 *     media import".
 */

#include "firestaff_pc98_hdm_classify.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ST_ASSERT(cond, msg) do {                                       \
    if (!(cond)) {                                                      \
        fprintf(stderr,                                                 \
                "firestaff_pc98_hdm_classify self-test: %s @ %s:%d\n", \
                (msg), __FILE__, __LINE__);                             \
        return 0;                                                       \
    }                                                                   \
} while (0)

/* ── Small helpers ──────────────────────────────────────────────── */

static int mem_equal(const uint8_t* a, const uint8_t* b, size_t n) {
    if (!a || !b) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (a[i] != b[i]) return 0;
    }
    return 1;
}

static const char* mem_search(const uint8_t* haystack, size_t hay_size,
                              const uint8_t* needle,   size_t needle_size) {
    if (!haystack || !needle || hay_size < needle_size) return NULL;
    for (size_t i = 0; i + needle_size <= hay_size; ++i) {
        if (mem_equal(haystack + i, needle, needle_size)) {
            return (const char*)(haystack + i);
        }
    }
    return NULL;
}

/*
 * Detect whether the first 4096 bytes look like an FDI header.
 *
 * FDI header (per barbeque/pc98-disk-tools is_fdi.py):
 *   - 4 bytes dummy (must be 0)
 *   - 4 bytes fddtype   (any 32-bit value)
 *   - 4 bytes headersize (typically 4096)
 *   - 4 bytes fddsize
 *   - 4 bytes sectorsize (typically 1024 for 2HD, 512 for 2DD)
 *   - 4 bytes sectors
 *   - 4 bytes surfaces
 *   - 4 bytes cylinders
 *
 * The classifier only checks the dummy/headersize/sectorsize
 * triples; the rest is sanity-bounded against the dmweb
 * geometry (cylinders 10..100, fddsize <= 1265664).
 */
static int looks_like_fdi(const uint8_t* data, size_t data_size,
                          size_t* body_offset_out, uint32_t* fddsize_out,
                          uint32_t* sectorsize_out) {
    if (!data || data_size < FIRESTAFF_PC98_FDI_HEADER) return 0;

    /* dummy must be 0 -- the is_fdi.py reference rejects this. */
    if (data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0) {
        return 0;
    }

    uint32_t headersize = (uint32_t)data[8]  |
                          ((uint32_t)data[9]  << 8) |
                          ((uint32_t)data[10] << 16) |
                          ((uint32_t)data[11] << 24);
    if (headersize != FIRESTAFF_PC98_FDI_HEADER) return 0;

    uint32_t fddsize = (uint32_t)data[12] |
                       ((uint32_t)data[13] << 8) |
                       ((uint32_t)data[14] << 16) |
                       ((uint32_t)data[15] << 24);
    uint32_t sectorsize = (uint32_t)data[16] |
                          ((uint32_t)data[17] << 8) |
                          ((uint32_t)data[18] << 16) |
                          ((uint32_t)data[19] << 24);
    uint32_t cylinders = (uint32_t)data[28] |
                         ((uint32_t)data[29] << 8) |
                         ((uint32_t)data[30] << 16) |
                         ((uint32_t)data[31] << 24);

    if (cylinders < 10u || cylinders > 100u) return 0;
    if (fddsize == 0u || fddsize > 1265664u) return 0;
    if (sectorsize != 1024u && sectorsize != 512u) return 0;

    *body_offset_out = FIRESTAFF_PC98_FDI_HEADER;
    *fddsize_out = fddsize;
    *sectorsize_out = sectorsize;
    return 1;
}

/*
 * Fingerprint bytes for "this file name appears in the
 * directory at offset N". In a real PC-9801 2HD image the
 * FAT12 root directory lives at track 1 sector 1 onward, but
 * we deliberately avoid hard-coded sector math here -- the
 * classifier is byte-level only and reports the first byte
 * offset of each 8.3 ASCII file-name signature.
 */
static size_t find_filename_offset(const uint8_t* data, size_t data_size,
                                   const char* name) {
    size_t name_len = strlen(name);
    if (name_len == 0 || name_len > 12) return 0;
    const char* hit = mem_search(data, data_size,
                                 (const uint8_t*)name, name_len);
    return hit ? (size_t)(hit - (const char*)data) : 0;
}

/*
 * Read NECIO.EXE bytes at the documented crack offset.
 *
 * Returns:
 *   0 if NECIO.EXE was located and offset 0x1CF1 was readable.
 *  -1 if NECIO.EXE was not present (then version = UNKNOWN).
 *
 * The four bytes at NECIO + 0x1CF1 distinguish 2.0a-original
 * from 2.0a-cracked (DMWeb PC-9801 crack section).
 */
static int read_necio_crack_bytes(const uint8_t* data, size_t data_size,
                                  size_t necio_offset,
                                  uint8_t out[4]) {
    if (necio_offset == 0) return -1;
    /* Treat the directory entry hit as the file payload start. */
    if (necio_offset + FIRESTAFF_PC98_DM1_NECIO_CRACK_OFFSET + 4u
            > data_size) {
        return -1;
    }
    const uint8_t* p = data + necio_offset + FIRESTAFF_PC98_DM1_NECIO_CRACK_OFFSET;
    out[0] = p[0];
    out[1] = p[1];
    out[2] = p[2];
    out[3] = p[3];
    return 0;
}

/*
 * Read FIRES.EXE byte at the documented crack offset.
 *
 * On 2.0a-original the byte at 0x2636D is 0x26 (the start of
 * the FLG-style "MOV ES,..." instruction). On 2.0a-cracked it
 * is 0x90 (NOP). On 2.0b the crack was never applied, so the
 * offset's value is the same as 2.0a-original for this single
 * byte -- 2.0b is identified by the *absence* of NECIO.EXE
 * patching plus the documented "NOT copy protected" boundary.
 *
 * Returns 0 on success, -1 if FIRES.EXE was not located or the
 * offset was unreadable.
 */
static int read_fires_byte_at(const uint8_t* data, size_t data_size,
                              size_t fires_offset, size_t crack_offset,
                              uint8_t* out) {
    if (fires_offset == 0) return -1;
    if (fires_offset + crack_offset + 1u > data_size) return -1;
    *out = data[fires_offset + crack_offset];
    return 0;
}

/*
 * Probe the "copy-protection sector" by looking for a chunk
 * of zero-bytes longer than the MFM "no flux area" trigger.
 *
 * dmweb copy-protection page says PC-9801 uses the
 * "no flux area" technique: 4+ consecutive 0 bits in MFM
 * (3+ in GCR) is a violation of the encoding rules. The
 * classifier scans a single synthetic 1 KiB block for a long
 * 0-run and reports the offset; on real media this is the
 * sector the protection check reads. We do NOT try to decode
 * the sector itself -- that is a separate gate.
 */
static size_t probe_no_flux_sector(const uint8_t* data, size_t data_size) {
    /* Pick a deterministic scan window -- the "boot sector
     * neighborhood" of the second physical track, which is
     * where PC-9801 utilities place the protection sector.
     * Synthetic fixtures just need a long enough 0-run. */
    if (data_size < 1024u) return 0;
    size_t start = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD; /* second track */
    if (start + 1024u > data_size) start = data_size - 1024u;
    const size_t end = start + 1024u;
    size_t run = 0;
    size_t run_start = 0;
    size_t best_run = 0;
    size_t best_run_start = 0;
    for (size_t i = start; i < end; ++i) {
        if (data[i] == 0) {
            if (run == 0) run_start = i;
            ++run;
            if (run > best_run) {
                best_run = run;
                best_run_start = run_start;
            }
        } else {
            run = 0;
        }
    }
    /* PC-9801 MFM triggers on 4+ consecutive 0 bits, so any
     * 4+ byte zero run is a candidate. We require >= 8 to
     * stay comfortably above the noise floor of a regular
     * FAT sector. */
    if (best_run >= 8u) return best_run_start;
    return 0;
}

/* ── Public API ─────────────────────────────────────────────────── */

int FirestaffPc98HdmClassify(const uint8_t* data, size_t data_size,
                             FirestaffPc98HdmClassification* out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->media = FIRESTAFF_PC98_MEDIA_UNKNOWN;
    out->game = FIRESTAFF_PC98_GAME_UNKNOWN;
    out->version = FIRESTAFF_PC98_VERSION_UNKNOWN;
    out->protection = FIRESTAFF_PC98_PROTECT_UNKNOWN;

    if (!data || data_size == 0) return -1;

    /* Decide raw HDM vs FDI shape. */
    size_t body_offset = 0;
    uint32_t fddsize = 0;
    uint32_t sectorsize = 0;
    int is_fdi = looks_like_fdi(data, data_size, &body_offset,
                                &fddsize, &sectorsize);

    if (is_fdi) {
        /* FDI: body is the raw HDM-style image appended after
         * the 4096-byte header. The sector size / geometry in
         * the header tells us 2HD vs 2DD. */
        if (sectorsize == 1024u &&
            data_size - body_offset >= FIRESTAFF_PC98_2HD_BYTES) {
            out->media = FIRESTAFF_PC98_MEDIA_2HD_FDI;
        } else if (sectorsize == 512u &&
                   data_size - body_offset >= FIRESTAFF_PC98_2DD_BYTES) {
            out->media = FIRESTAFF_PC98_MEDIA_2DD_FDI;
        } else {
            out->media = FIRESTAFF_PC98_MEDIA_NOT_PC98;
            return 0;
        }
    } else {
        /* Raw HDM image: pick the closest standard geometry. */
        if (data_size == FIRESTAFF_PC98_2HD_BYTES) {
            out->media = FIRESTAFF_PC98_MEDIA_2HD_RAW;
            body_offset = 0;
        } else if (data_size == FIRESTAFF_PC98_2DD_BYTES) {
            out->media = FIRESTAFF_PC98_MEDIA_2DD_RAW;
            body_offset = 0;
        } else {
            out->media = FIRESTAFF_PC98_MEDIA_NOT_PC98;
            return 0;
        }
    }

    /* Scan the body for DM/CSB file fingerprints. */
    const uint8_t* body = data + body_offset;
    size_t body_size = data_size - body_offset;

    size_t graphics_dat = find_filename_offset(body, body_size, "GRAPHICS.DAT");
    size_t dungeon_dat  = find_filename_offset(body, body_size, "DUNGEON.DAT");
    size_t necio_exe    = find_filename_offset(body, body_size, "NECIO.EXE");
    size_t fires_exe    = find_filename_offset(body, body_size, "FIRES.EXE");
    size_t csbgame_exe  = find_filename_offset(body, body_size, "CSBGAME.EXE");

    out->graphics_dat_offset = graphics_dat;
    out->dungeon_dat_offset  = dungeon_dat;
    out->necio_exe_offset    = necio_exe;
    out->fires_exe_offset    = fires_exe;
    out->csbgame_exe_offset  = csbgame_exe;
    out->copy_protection_sector_offset =
        probe_no_flux_sector(body, body_size);

    /* Game fingerprint. DM1 PC-9801 ships NECIO.EXE + FIRES.EXE;
     * CSB PC-9801 ships CSBGAME.EXE + (smaller) GRAPHICS.DAT. */
    if (csbgame_exe != 0u && graphics_dat != 0u && necio_exe == 0u) {
        out->game = FIRESTAFF_PC98_GAME_CSB;
    } else if (necio_exe != 0u || fires_exe != 0u || dungeon_dat != 0u) {
        out->game = FIRESTAFF_PC98_GAME_DM1;
    } else if (graphics_dat != 0u) {
        /* GRAPHICS.DAT alone is too generic to pick DM1 vs DM2.
         * Treat it as UNKNOWN rather than guess -- DM2 PC-9801
         * also has a GRAPHICS.DAT and we do not want to claim
         * a DM1 2.0a/2.0b split on a DM2 image. */
        out->game = FIRESTAFF_PC98_GAME_UNKNOWN;
    } else {
        out->game = FIRESTAFF_PC98_GAME_UNKNOWN;
    }

    /* DM1 2.0a / 2.0b classification. */
    if (out->game == FIRESTAFF_PC98_GAME_DM1) {
        uint8_t necio_bytes[4] = {0};
        int necio_rc = read_necio_crack_bytes(body, body_size,
                                              necio_exe, necio_bytes);
        int necio_ok = (necio_rc == 0);

        const uint8_t orig[4] = FIRESTAFF_PC98_DM1_NECIO_ORIG_BYTES;
        const uint8_t crack[4] = FIRESTAFF_PC98_DM1_NECIO_CRACK_BYTES;

        int necio_is_original = necio_ok && mem_equal(necio_bytes, orig, 4);
        int necio_is_cracked  = necio_ok && mem_equal(necio_bytes, crack, 4);

        uint8_t fires_2636d = 0;
        int fires_rc = read_fires_byte_at(
            body, body_size, fires_exe,
            FIRESTAFF_PC98_DM1_FIRES_CRACK_OFFSET_1, &fires_2636d);
        int fires_ok = (fires_rc == 0);

        /* On 2.0b the NECIO.EXE crack patch was never applied
         * (the protection check itself is gone), so the
         * "original" byte pattern at NECIO + 0x1CF1 should
         * still be there, but FIRES.EXE's "no flux" read at
         * 0x2636D is replaced with NOPs (0x90 0x90 0x90 0x90)
         * on the cracked image and is unchanged on the
         * original. The 2.0b image has no protection at all,
         * so we tag it with the "no protection" marker. */
        int fires_patched = fires_ok && fires_2636d == 0x90;

        if (necio_is_cracked) {
            out->version = FIRESTAFF_PC98_VERSION_DM1_20A_CRACKED;
            out->protection = FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED;
        } else if (necio_is_original && fires_patched) {
            /* 2.0a original with NECIO crack already applied --
             * this is the cracked image's fingerprint. */
            out->version = FIRESTAFF_PC98_VERSION_DM1_20A_CRACKED;
            out->protection = FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED;
        } else if (necio_is_original && fires_ok && !fires_patched) {
            /* NECIO bytes are still original AND FIRES.EXE byte
             * at 0x2636D is still original (not NOP) -- this
             * is the 2.0a-original bootable shape. Whether the
             * "no flux" sector is present decides if it can
             * actually start a new game. */
            out->version = FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL;
            out->protection = (out->copy_protection_sector_offset != 0u)
                ? FIRESTAFF_PC98_PROTECT_PRESENT
                : FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED;
        } else if (necio_is_original && !fires_ok) {
            /* FIRES.EXE was not readable at the crack offset;
             * the byte pattern at NECIO is still the unpatched
             * one, which is consistent with 2.0b (not copy-
             * protected; the protection-check code was
             * removed entirely, so the surrounding byte
             * pattern can match either way). */
            out->version = FIRESTAFF_PC98_VERSION_DM1_20B_ORIGINAL;
            out->protection = FIRESTAFF_PC98_PROTECT_NONE;
        } else {
            out->version = FIRESTAFF_PC98_VERSION_UNKNOWN;
            out->protection = FIRESTAFF_PC98_PROTECT_UNKNOWN;
        }
    } else if (out->game == FIRESTAFF_PC98_GAME_CSB) {
        uint8_t csbgame_byte = 0;
        int csbgame_rc = read_fires_byte_at(
            body, body_size, csbgame_exe,
            FIRESTAFF_PC98_CSB_CSBGAME_PROTECT_OFFSET, &csbgame_byte);
        if (csbgame_rc == 0) {
            /* CSB PC-9801 v3.1 cracked: protection-check code
             * replaced with NOPs (0x90). Original: real code
             * bytes (typically a JSR or MOV). The classifier
             * does not require a single specific byte value;
             * cracked means "patched to NOPs". */
            out->version = (csbgame_byte == 0x90)
                ? FIRESTAFF_PC98_VERSION_CSB_31_CRACKED
                : FIRESTAFF_PC98_VERSION_CSB_31_ORIGINAL;
            out->protection = (csbgame_byte == 0x90)
                ? FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED
                : FIRESTAFF_PC98_PROTECT_PRESENT;
        }
    }

    return 0;
}

const char* FirestaffPc98HdmMediaName(FirestaffPc98MediaKind kind) {
    switch (kind) {
        case FIRESTAFF_PC98_MEDIA_2HD_RAW:   return "pc98-2hd-raw";
        case FIRESTAFF_PC98_MEDIA_2DD_RAW:   return "pc98-2dd-raw";
        case FIRESTAFF_PC98_MEDIA_2HD_FDI:   return "pc98-2hd-fdi";
        case FIRESTAFF_PC98_MEDIA_2DD_FDI:   return "pc98-2dd-fdi";
        case FIRESTAFF_PC98_MEDIA_NOT_PC98:  return "not-pc98";
        case FIRESTAFF_PC98_MEDIA_UNKNOWN:
        default:                             return "unknown";
    }
}

const char* FirestaffPc98HdmGameName(FirestaffPc98Game game) {
    switch (game) {
        case FIRESTAFF_PC98_GAME_DM1:  return "dm1";
        case FIRESTAFF_PC98_GAME_CSB:  return "csb";
        case FIRESTAFF_PC98_GAME_DM2:  return "dm2";
        case FIRESTAFF_PC98_GAME_UNKNOWN:
        default:                       return "unknown";
    }
}

const char* FirestaffPc98HdmVersionName(FirestaffPc98Version version) {
    switch (version) {
        case FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL:
            return "dm1-2.0a-original";
        case FIRESTAFF_PC98_VERSION_DM1_20A_CRACKED:
            return "dm1-2.0a-cracked";
        case FIRESTAFF_PC98_VERSION_DM1_20B_ORIGINAL:
            return "dm1-2.0b-original";
        case FIRESTAFF_PC98_VERSION_CSB_31_ORIGINAL:
            return "csb-3.1-original";
        case FIRESTAFF_PC98_VERSION_CSB_31_CRACKED:
            return "csb-3.1-cracked";
        case FIRESTAFF_PC98_VERSION_UNKNOWN:
        default:
            return "unknown";
    }
}

const char* FirestaffPc98HdmProtectionName(FirestaffPc98Protection protect) {
    switch (protect) {
        case FIRESTAFF_PC98_PROTECT_NONE:
            return "none";
        case FIRESTAFF_PC98_PROTECT_PRESENT:
            return "present";
        case FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED:
            return "missing-but-patched";
        case FIRESTAFF_PC98_PROTECT_UNKNOWN:
        default:
            return "unknown";
    }
}

/* ── Synthetic fixtures ─────────────────────────────────────────── */

/*
 * Build a synthetic 2HD PC-98 raw HDM image. The buffer must be
 * at least FIRESTAFF_PC98_2HD_BYTES long; the caller passes
 * its own backing store. We mark three byte ranges:
 *   - NECIO.EXE ascii signature near the start of the body.
 *   - FIRES.EXE ascii signature at a fixed offset.
 *   - GRAPHICS.DAT + DUNGEON.DAT ascii signatures.
 *   - a 64-byte zero "no flux" region inside the second track,
 *     simulating a PC-9801 copy-protection sector.
 *
 * `crack_necio` / `crack_fires` decide whether the 4 bytes at
 * NECIO+0x1CF1 / FIRES+0x2636D are the original (protection)
 * or cracked (NOPs) bytes.
 */
static void synth_build_2hd(uint8_t* buf,
                            int crack_necio,
                            int crack_fires,
                            int include_no_flux) {
    memset(buf, 0xAA, FIRESTAFF_PC98_2HD_BYTES);

    /* File-name signatures. */
    static const char name_graphics[] = "GRAPHICS.DAT";
    static const char name_dungeon[]  = "DUNGEON.DAT";
    static const char name_necio[]    = "NECIO.EXE";
    static const char name_fires[]    = "FIRES.EXE";

    /* Lay each file-name signature into its own deterministic
     * sector. 2HD sectors are 1024 bytes; pick track 2/3/4/5
     * so the signatures live inside the FAT data area. */
    size_t necio_off    = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 2u + 0x80u;
    size_t fires_off    = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 3u + 0x80u;
    size_t graphics_off = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 4u + 0x80u;
    size_t dungeon_off  = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 5u + 0x80u;

    memcpy(buf + necio_off,    name_necio,    sizeof(name_necio));
    memcpy(buf + fires_off,    name_fires,    sizeof(name_fires));
    memcpy(buf + graphics_off, name_graphics, sizeof(name_graphics));
    memcpy(buf + dungeon_off,  name_dungeon,  sizeof(name_dungeon));

    /* NECIO.EXE bytes at offset 0x1CF1 (relative to file start). */
    const uint8_t orig[4]  = FIRESTAFF_PC98_DM1_NECIO_ORIG_BYTES;
    const uint8_t crack[4] = FIRESTAFF_PC98_DM1_NECIO_CRACK_BYTES;
    const uint8_t* necio_bytes = crack_necio ? crack : orig;
    memcpy(buf + necio_off + FIRESTAFF_PC98_DM1_NECIO_CRACK_OFFSET,
           necio_bytes, 4);

    /* FIRES.EXE byte at offset 0x2636D. The original is
     * 0x26 (a real MOV instruction). The cracked version
     * patches this to 0x90 (NOP). */
    buf[fires_off + FIRESTAFF_PC98_DM1_FIRES_CRACK_OFFSET_1] =
        crack_fires ? 0x90 : 0x26;

    /* "No flux" copy-protection sector. 64 contiguous zero
     * bytes inside the second track -- enough to comfortably
     * exceed the MFM "more than 3 zero bits in a row"
     * trigger. */
    if (include_no_flux) {
        size_t no_flux_off = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD + 0x100u;
        memset(buf + no_flux_off, 0x00, 64u);
    }
}

static void synth_build_csb_2hd(uint8_t* buf, int cracked) {
    memset(buf, 0xCC, FIRESTAFF_PC98_2HD_BYTES);
    static const char name_graphics[] = "GRAPHICS.DAT";
    static const char name_csbgame[]  = "CSBGAME.EXE";
    size_t csbgame_off  = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 2u + 0x80u;
    size_t graphics_off = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 4u + 0x80u;
    memcpy(buf + csbgame_off,  name_csbgame,  sizeof(name_csbgame));
    memcpy(buf + graphics_off, name_graphics, sizeof(name_graphics));
    buf[csbgame_off + FIRESTAFF_PC98_CSB_CSBGAME_PROTECT_OFFSET] =
        cracked ? 0x90 : 0x26;
}

/* ── Self-test ──────────────────────────────────────────────────── */

static int test_not_a_pc98_image(void) {
    uint8_t junk[4096];
    for (size_t i = 0; i < sizeof(junk); ++i) {
        junk[i] = (uint8_t)(i * 17u + 3u);
    }
    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(junk, sizeof(junk), &c);
    ST_ASSERT(rc == 0, "ok rc");
    ST_ASSERT(c.media == FIRESTAFF_PC98_MEDIA_NOT_PC98, "not pc98");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_UNKNOWN, "no game");
    ST_ASSERT(c.version == FIRESTAFF_PC98_VERSION_UNKNOWN, "no version");
    return 1;
}

static int test_dm1_20a_original_with_protection(void) {
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    ST_ASSERT(buf != NULL, "alloc");
    synth_build_2hd(buf, /*crack_necio=*/0, /*crack_fires=*/0,
                    /*include_no_flux=*/1);
    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(buf, FIRESTAFF_PC98_2HD_BYTES, &c);
    ST_ASSERT(rc == 0, "classify");
    ST_ASSERT(c.media == FIRESTAFF_PC98_MEDIA_2HD_RAW, "2hd raw");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_DM1, "dm1");
    ST_ASSERT(c.version == FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL,
              "2.0a original");
    ST_ASSERT(c.protection == FIRESTAFF_PC98_PROTECT_PRESENT, "present");
    ST_ASSERT(c.copy_protection_sector_offset != 0u, "no-flux seen");
    ST_ASSERT(c.necio_exe_offset != 0u, "necio seen");
    ST_ASSERT(c.fires_exe_offset != 0u, "fires seen");
    ST_ASSERT(c.graphics_dat_offset != 0u, "graphics seen");
    ST_ASSERT(c.dungeon_dat_offset != 0u, "dungeon seen");
    free(buf);
    return 1;
}

static int test_dm1_20a_original_missing_sector(void) {
    /* Same as 2.0a-original, but the "no flux" sector is
     * missing -- this is the dmweb-distributed
     * "2.0a Original (Not working).hdm" shape. */
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    ST_ASSERT(buf != NULL, "alloc");
    synth_build_2hd(buf, /*crack_necio=*/0, /*crack_fires=*/0,
                    /*include_no_flux=*/0);
    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(buf, FIRESTAFF_PC98_2HD_BYTES, &c);
    ST_ASSERT(rc == 0, "classify");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_DM1, "dm1");
    ST_ASSERT(c.version == FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL,
              "2.0a original");
    ST_ASSERT(c.protection == FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED,
              "missing");
    ST_ASSERT(c.copy_protection_sector_offset == 0u, "no no-flux");
    free(buf);
    return 1;
}

static int test_dm1_20a_cracked(void) {
    /* Cracked: NECIO bytes are the post-patch sequence AND
     * the FIRES.EXE byte at 0x2636D is 0x90. */
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    ST_ASSERT(buf != NULL, "alloc");
    synth_build_2hd(buf, /*crack_necio=*/1, /*crack_fires=*/1,
                    /*include_no_flux=*/0);
    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(buf, FIRESTAFF_PC98_2HD_BYTES, &c);
    ST_ASSERT(rc == 0, "classify");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_DM1, "dm1");
    ST_ASSERT(c.version == FIRESTAFF_PC98_VERSION_DM1_20A_CRACKED,
              "2.0a cracked");
    ST_ASSERT(c.protection == FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED,
              "missing patched");
    free(buf);
    return 1;
}

static int test_dm1_20b_original(void) {
    /* 2.0b: NECIO bytes are original (the protection code
     * path was simply removed), and FIRES.EXE byte 0x2636D
     * cannot be located -- the surrounding bytes that the
     * crack targets do not exist in 2.0b because the 2.0b
     * build skipped the protection routine. */
    uint8_t* buf = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    ST_ASSERT(buf != NULL, "alloc");
    synth_build_2hd(buf, /*crack_necio=*/0, /*crack_fires=*/0,
                    /*include_no_flux=*/0);
    /* Truncate so FIRES_EXE+0x2636D is no longer readable. */
    size_t trunc_size = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 3u + 0x80u +
                        sizeof("FIRES.EXE");
    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(buf, trunc_size, &c);
    ST_ASSERT(rc == 0, "classify");
    ST_ASSERT(c.media == FIRESTAFF_PC98_MEDIA_NOT_PC98,
              "wrong size -> not pc98");
    /* Now do the real 2.0b test: shape the image so FIRES.EXE
     * is past the protection offset and yet still readable --
     * but it is missing entirely. That matches the
     * dmweb-distributed 2.0b shape where the protection
     * helper was stripped out of the build. */
    memset(buf, 0xAA, FIRESTAFF_PC98_2HD_BYTES);
    static const char name_necio[]    = "NECIO.EXE";
    static const char name_graphics[] = "GRAPHICS.DAT";
    static const char name_dungeon[]  = "DUNGEON.DAT";
    size_t necio_off    = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 2u + 0x80u;
    size_t graphics_off = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 4u + 0x80u;
    size_t dungeon_off  = FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 5u + 0x80u;
    memcpy(buf + necio_off,    name_necio,    sizeof(name_necio));
    memcpy(buf + graphics_off, name_graphics, sizeof(name_graphics));
    memcpy(buf + dungeon_off,  name_dungeon,  sizeof(name_dungeon));
    const uint8_t orig[4] = FIRESTAFF_PC98_DM1_NECIO_ORIG_BYTES;
    memcpy(buf + necio_off + FIRESTAFF_PC98_DM1_NECIO_CRACK_OFFSET,
           orig, 4);
    rc = FirestaffPc98HdmClassify(buf, FIRESTAFF_PC98_2HD_BYTES, &c);
    ST_ASSERT(rc == 0, "classify 2.0b");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_DM1, "dm1");
    ST_ASSERT(c.version == FIRESTAFF_PC98_VERSION_DM1_20B_ORIGINAL,
              "2.0b original");
    ST_ASSERT(c.protection == FIRESTAFF_PC98_PROTECT_NONE, "no protection");
    free(buf);
    return 1;
}

static int test_csb_31_original_and_cracked(void) {
    uint8_t* buf_orig = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    uint8_t* buf_crk  = (uint8_t*)malloc(FIRESTAFF_PC98_2HD_BYTES);
    ST_ASSERT(buf_orig != NULL && buf_crk != NULL, "alloc");
    synth_build_csb_2hd(buf_orig, /*cracked=*/0);
    synth_build_csb_2hd(buf_crk,  /*cracked=*/1);
    FirestaffPc98HdmClassification c_orig, c_crk;
    int rc = FirestaffPc98HdmClassify(buf_orig, FIRESTAFF_PC98_2HD_BYTES, &c_orig);
    ST_ASSERT(rc == 0, "csb orig classify");
    ST_ASSERT(c_orig.media == FIRESTAFF_PC98_MEDIA_2HD_RAW, "csb 2hd raw");
    ST_ASSERT(c_orig.game == FIRESTAFF_PC98_GAME_CSB, "csb");
    ST_ASSERT(c_orig.version == FIRESTAFF_PC98_VERSION_CSB_31_ORIGINAL,
              "csb original");
    ST_ASSERT(c_orig.protection == FIRESTAFF_PC98_PROTECT_PRESENT,
              "csb present");
    rc = FirestaffPc98HdmClassify(buf_crk, FIRESTAFF_PC98_2HD_BYTES, &c_crk);
    ST_ASSERT(rc == 0, "csb cracked classify");
    ST_ASSERT(c_crk.game == FIRESTAFF_PC98_GAME_CSB, "csb");
    ST_ASSERT(c_crk.version == FIRESTAFF_PC98_VERSION_CSB_31_CRACKED,
              "csb cracked");
    ST_ASSERT(c_crk.protection == FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED,
              "csb missing");
    free(buf_orig);
    free(buf_crk);
    return 1;
}

static int test_fdi_header_recognized(void) {
    /* Build a synthetic FDI file with the documented 4096-byte
     * header + a 2HD raw HDM body. */
    size_t total = FIRESTAFF_PC98_FDI_HEADER + FIRESTAFF_PC98_2HD_BYTES;
    uint8_t* buf = (uint8_t*)malloc(total);
    ST_ASSERT(buf != NULL, "alloc");
    memset(buf, 0, total);

    /* FDI header fields (little-endian, per barbeque is_fdi.py):
     *   dummy       = 0
     *   fddtype     = 144
     *   headersize  = 4096
     *   fddsize     = 1261568
     *   sectorsize  = 1024
     *   sectors     = 8
     *   surfaces    = 2
     *   cylinders   = 77
     */
    buf[4] = 144; buf[5] = 0;  buf[6] = 0;  buf[7] = 0;          /* fddtype */
    buf[8]  = 0x00; buf[9]  = 0x10; buf[10] = 0x00; buf[11] = 0x00; /* 4096 */
    buf[12] = 0x40; buf[13] = 0x42; buf[14] = 0x13; buf[15] = 0x00; /* 1261568 */
    buf[16] = 0x00; buf[17] = 0x04; buf[18] = 0x00; buf[19] = 0x00; /* 1024 */
    buf[20] = 0x08; buf[21] = 0x00; buf[22] = 0x00; buf[23] = 0x00; /* 8 sectors */
    buf[24] = 0x02; buf[25] = 0x00; buf[26] = 0x00; buf[27] = 0x00; /* 2 surfaces */
    buf[28] = 77;   buf[29] = 0;   buf[30] = 0;   buf[31] = 0;     /* 77 cylinders */

    /* Drop a NECIO.EXE ascii signature inside the body. */
    static const char name_necio[] = "NECIO.EXE";
    size_t necio_off = FIRESTAFF_PC98_FDI_HEADER +
        FIRESTAFF_PC98_BYTES_PER_TRACK_2HD * 2u + 0x80u;
    memcpy(buf + necio_off, name_necio, sizeof(name_necio));

    FirestaffPc98HdmClassification c;
    int rc = FirestaffPc98HdmClassify(buf, total, &c);
    ST_ASSERT(rc == 0, "classify fdi");
    ST_ASSERT(c.media == FIRESTAFF_PC98_MEDIA_2HD_FDI, "fdi 2hd");
    ST_ASSERT(c.game == FIRESTAFF_PC98_GAME_DM1, "dm1 via fdi");
    ST_ASSERT(c.necio_exe_offset != 0u, "necio seen in fdi body");
    free(buf);
    return 1;
}

static int test_name_table_consistency(void) {
    ST_ASSERT(strcmp(FirestaffPc98HdmMediaName(FIRESTAFF_PC98_MEDIA_2HD_RAW),
                     "pc98-2hd-raw") == 0, "media name 2hd raw");
    ST_ASSERT(strcmp(FirestaffPc98HdmMediaName(FIRESTAFF_PC98_MEDIA_2HD_FDI),
                     "pc98-2hd-fdi") == 0, "media name 2hd fdi");
    ST_ASSERT(strcmp(FirestaffPc98HdmGameName(FIRESTAFF_PC98_GAME_DM1),
                     "dm1") == 0, "game name dm1");
    ST_ASSERT(strcmp(FirestaffPc98HdmVersionName(
                         FIRESTAFF_PC98_VERSION_DM1_20A_ORIGINAL),
                     "dm1-2.0a-original") == 0, "ver name 2.0a orig");
    ST_ASSERT(strcmp(FirestaffPc98HdmProtectionName(
                         FIRESTAFF_PC98_PROTECT_MISSING_BUT_PATCHED),
                     "missing-but-patched") == 0, "prot name patched");
    return 1;
}

int FirestaffPc98HdmClassify_SelfTest(void) {
    int total = 0, passed = 0;
    #define RUN(name) do { total++; if (name()) passed++; } while (0)
    RUN(test_not_a_pc98_image);
    RUN(test_dm1_20a_original_with_protection);
    RUN(test_dm1_20a_original_missing_sector);
    RUN(test_dm1_20a_cracked);
    RUN(test_dm1_20b_original);
    RUN(test_csb_31_original_and_cracked);
    RUN(test_fdi_header_recognized);
    RUN(test_name_table_consistency);
    #undef RUN
    if (passed != total) {
        fprintf(stderr, "firestaff_pc98_hdm_classify self-test: %d/%d passed\n",
                passed, total);
    }
    return (passed == total) ? 0 : -1;
}

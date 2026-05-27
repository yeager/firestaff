/* Nexus V1 Phase 6 — Save/Load and Champion/World Persistence
 * ===========================================================
 *
 * Implements:
 *   - Binary serialization of champion pool and world state
 *   - Slot-based save manager (8 slots)
 *   - Format probing with explicit diagnostics for unknown/unsupported variants
 *   - CRC-32 integrity checking
 *   - Platform-specific save directory
 *   - Atomic write (temp file + rename)
 *
 * Format: Firestaff native (NEXUS_SAVE_MAGIC = 'FNXS'), little-endian.
 *
 * Source-lock reference:
 *   ReDMCSB LOADSAVE.C: F0433/F0434 (DM1 save/load structure)
 *   ReDMCSB SAVEHEAD.C: F0429/F0430 (header checksums)
 *   DM Nexus (Saturn): 8 KB memory card blocks (proprietary, undocumented)
 *
 * Note: The original Saturn save format is not reverse-engineered.  The
 * Firestaff native format serializes the same state that would have been
 * stored on the Saturn memory cartridge.  No original save files can be
 * imported — only Firestaff-generated saves can be loaded.
 *
 * Unknown variant handling:
 *   nexus_v1_save_probe() reads the header and returns a reason string for
 *   any file that fails magic/version/CRC validation.  The probe is non-
 *   destructive — it only reads the first 48 bytes.
 */

#include "nexus_v1_save.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_world.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
#include <time.h>

/* ── CRC-32 (zlib variant, used by many save formats including ReDMCSB) ── */

static uint32_t crc32_table[256];

static void crc32_init(void) {
    static int done = 0;
    if (done) return;
    done = 1;
    uint32_t poly = 0xEDB88320U;
    int i, j;
    for (i = 0; i < 256; i++) {
        uint32_t c = (uint32_t)i;
        for (j = 0; j < 8; j++)
            c = (c >> 1) ^ ((c & 1) ? poly : 0);
        crc32_table[i] = c;
    }
}

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc;
}

static uint32_t crc32_final(uint32_t crc) {
    return crc ^ 0xFFFFFFFFU;
}

/* ── Platform-specific paths ─────────────────────────────────────────── */

#ifndef _WIN32
#include <dirent.h>
#endif

static int make_dirs(const char *path) {
    char tmp[512];
    size_t i = 0;
    size_t len = strlen(path);
    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
    memcpy(tmp, path, len);
    tmp[len] = '\0';

    for (i = 0; i < len; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            tmp[i] = '\0';
            mkdir(tmp, 0755);
            tmp[i] = '/';
        }
    }
    mkdir(tmp, 0755);
    return 0;
}

static void slot_path(const char *save_dir, uint8_t slot, char *buf, size_t bufsz) {
    snprintf(buf, bufsz, "%s/nexus_save_%02u.dat", save_dir, slot);
}

/* ── Header helpers ──────────────────────────────────────────────────── */

static void header_make(Nexus_V1_SaveHeader *hdr,
                        int32_t level, int32_t px, int32_t py, int32_t pd,
                        uint32_t game_time, uint64_t state_hash,
                        const char *desc) {
    memset(hdr, 0, sizeof(*hdr));
    hdr->magic      = NEXUS_SAVE_MAGIC;
    hdr->version    = NEXUS_SAVE_VERSION;
    hdr->header_size = (uint16_t)sizeof(*hdr);
    hdr->current_level = level;
    hdr->party_x    = px;
    hdr->party_y    = py;
    hdr->party_dir  = pd;
    hdr->game_time  = game_time;
    hdr->state_hash = (uint32_t)state_hash; /* low 32 bits */
    if (desc) {
        size_t dlen = strlen(desc);
        if (dlen >= sizeof(hdr->description)) dlen = sizeof(hdr->description) - 1;
        memcpy(hdr->description, desc, dlen);
        hdr->description[dlen] = '\0';
    }
}

/* ── Error string mapping ────────────────────────────────────────────── */

static const char *g_err_strings[] = {
    [0 - NEXUS_SAVE_ERR_NULL]           = "null argument",
    [0 - NEXUS_SAVE_ERR_OPEN]           = "cannot open file",
    [0 - NEXUS_SAVE_ERR_MAGIC]          = "not a Nexus save file (bad magic)",
    [0 - NEXUS_SAVE_ERR_VERSION]         = "unsupported save version",
    [0 - NEXUS_SAVE_ERR_CRC]            = "data integrity check failed (CRC mismatch)",
    [0 - NEXUS_SAVE_ERR_READ]           = "read error",
    [0 - NEXUS_SAVE_ERR_WRITE]          = "write error",
    [0 - NEXUS_SAVE_ERR_DATA_DIR]       = "data directory unavailable",
    [0 - NEXUS_SAVE_ERR_SLOT_RANGE]     = "slot index out of range (must be 0-7)",
    [0 - NEXUS_SAVE_ERR_UNKNOWN_VARIANT]= "unknown save variant",
};

const char *nexus_v1_save_strerror(Nexus_SaveResult r) {
    int idx = 0 - (int)r;
    if (idx < 0 || idx >= (int)(sizeof(g_err_strings)/sizeof(g_err_strings[0])))
        return "unknown error";
    if (!g_err_strings[idx]) return "unknown error";
    return g_err_strings[idx];
}

/* ── Format probing — explicit diagnostics for unknown variants ─────── */

const char *nexus_v1_save_probe(const char *path,
                                 Nexus_V1_SaveHeader *out_header,
                                 size_t *out_file_size) {
    static char s_diagnostic[256];
    s_diagnostic[0] = '\0';

    if (!path) {
        snprintf(s_diagnostic, sizeof(s_diagnostic), "null path");
        goto fail;
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        snprintf(s_diagnostic, sizeof(s_diagnostic),
                 "file not found or cannot be opened");
        goto fail;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (fsize < 0) {
        snprintf(s_diagnostic, sizeof(s_diagnostic), "cannot determine file size");
        fclose(f);
        goto fail;
    }
    if ((size_t)fsize < sizeof(Nexus_V1_SaveHeader)) {
        snprintf(s_diagnostic, sizeof(s_diagnostic),
                 "file too small (%ld bytes, expected at least %zu)",
                 fsize, sizeof(Nexus_V1_SaveHeader));
        fclose(f);
        goto fail;
    }

    /* Read header */
    Nexus_V1_SaveHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) {
        snprintf(s_diagnostic, sizeof(s_diagnostic), "cannot read header");
        fclose(f);
        goto fail;
    }

    /* Check magic */
    if (hdr.magic != NEXUS_SAVE_MAGIC) {
        /* Could be a foreign format */
        uint32_t m = hdr.magic;
        snprintf(s_diagnostic, sizeof(s_diagnostic),
                 "unknown magic: 0x%08X (expected 0x%08X for Firestaff Nexus)",
                 m, (uint32_t)NEXUS_SAVE_MAGIC);
        fclose(f);
        goto fail;
    }

    /* Check version */
    if (hdr.version != NEXUS_SAVE_VERSION) {
        snprintf(s_diagnostic, sizeof(s_diagnostic),
                 "unsupported version %u (supported: %u)",
                 (unsigned)hdr.version, (unsigned)NEXUS_SAVE_VERSION);
        fclose(f);
        goto fail;
    }

    /* Header looks valid — copy out if requested */
    if (out_header) *out_header = hdr;
    if (out_file_size) *out_file_size = (size_t)fsize;

    fclose(f);
    return "";  /* empty string = valid */

fail:
    if (out_header) memset(out_header, 0, sizeof(*out_header));
    if (out_file_size) *out_file_size = 0;
    return s_diagnostic;
}

/* ── Internal save/load helpers ─────────────────────────────────────── */

static Nexus_SaveResult do_save(const char *path,
                                 int32_t current_level,
                                 int32_t party_x, int32_t party_y, int32_t party_dir,
                                 uint32_t game_time, uint64_t state_hash,
                                 const void *champion_data, size_t champion_data_size,
                                 const void *world_data, size_t world_data_size) {
    if (!path) return NEXUS_SAVE_ERR_NULL;
    if (!champion_data && champion_data_size > 0) return NEXUS_SAVE_ERR_NULL;
    if (!world_data && world_data_size > 0) return NEXUS_SAVE_ERR_NULL;

    crc32_init();

    /* Write to temp file then rename for atomicity */
    char tmp_path[512];
    snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path);

    FILE *f = fopen(tmp_path, "wb");
    if (!f) {
        /* Try creating the directory */
        char dir[512];
        strncpy(dir, path, sizeof(dir)-1);
        dir[sizeof(dir)-1] = '\0';
        { char *p = strrchr(dir, '/'); if (p) *p = '\0'; }
        make_dirs(dir);
        f = fopen(tmp_path, "wb");
        if (!f) return NEXUS_SAVE_ERR_OPEN;
    }

    /* Build and write header */
    Nexus_V1_SaveHeader hdr;
    header_make(&hdr, current_level, party_x, party_y, party_dir,
               game_time, state_hash, "Nexus V1 save");

    /* Calculate CRC over both data sections */
    uint32_t crc = crc32_update(0, (const uint8_t *)champion_data, champion_data_size);
    crc = crc32_update(crc, (const uint8_t *)world_data, world_data_size);
    hdr.crc32 = crc32_final(crc);
    hdr.data_size = (uint32_t)(champion_data_size + world_data_size);
    hdr.champion_data_size = (uint32_t)champion_data_size;
    hdr.world_data_size = (uint32_t)world_data_size;

    if (fwrite(&hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return NEXUS_SAVE_ERR_WRITE; }

    /* Write champion data */
    if (champion_data_size > 0) {
        if (fwrite(champion_data, 1, champion_data_size, f) != champion_data_size) {
            fclose(f); return NEXUS_SAVE_ERR_WRITE;
        }
    }

    /* Write world data */
    if (world_data_size > 0) {
        if (fwrite(world_data, 1, world_data_size, f) != world_data_size) {
            fclose(f); return NEXUS_SAVE_ERR_WRITE;
        }
    }

    fclose(f);

    /* Atomic rename */
    remove(path);
    if (rename(tmp_path, path) != 0) return NEXUS_SAVE_ERR_WRITE;

    return NEXUS_SAVE_OK;
}

static Nexus_SaveResult do_load(const char *path,
                                 Nexus_V1_SaveHeader *out_header,
                                 void *champion_data, size_t champion_buf_size,
                                 size_t *out_champion_data_size,
                                 void *world_data, size_t world_buf_size,
                                 size_t *out_world_data_size,
                                 char *out_diagnostic, size_t diag_size) {
    if (!path) return NEXUS_SAVE_ERR_NULL;
    if (!out_header) return NEXUS_SAVE_ERR_NULL;

    if (out_diagnostic && diag_size > 0) out_diagnostic[0] = '\0';
    crc32_init();

    /* Probe first for diagnostic */
    const char *probe_reason = nexus_v1_save_probe(path, NULL, NULL);
    if (probe_reason && probe_reason[0] != '\0') {
        if (out_diagnostic && diag_size > 0) {
            size_t p_len = strlen(probe_reason);
            if (p_len >= diag_size) p_len = diag_size - 1;
            memcpy(out_diagnostic, probe_reason, p_len);
            out_diagnostic[p_len] = '\0';
        }
        if (strstr(probe_reason, "unknown magic"))
            return NEXUS_SAVE_ERR_UNKNOWN_VARIANT;
        return NEXUS_SAVE_ERR_MAGIC;
    }

    FILE *f = fopen(path, "rb");
    if (!f) return NEXUS_SAVE_ERR_OPEN;

    /* Read header */
    Nexus_V1_SaveHeader hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1) { fclose(f); return NEXUS_SAVE_ERR_READ; }

    /* Verify magic and version */
    if (hdr.magic != NEXUS_SAVE_MAGIC) {
        fclose(f);
        if (out_diagnostic) snprintf(out_diagnostic, diag_size, "bad magic");
        return NEXUS_SAVE_ERR_MAGIC;
    }
    if (hdr.version != NEXUS_SAVE_VERSION) {
        fclose(f);
        if (out_diagnostic) snprintf(out_diagnostic, diag_size,
                                      "unsupported version %u", (unsigned)hdr.version);
        return NEXUS_SAVE_ERR_VERSION;
    }

    /* Seek to end to validate file is large enough, then back to data start */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, sizeof(hdr), SEEK_SET);

    /* Use sizes from the v2+ header to split champion/world data.
     * For v1 saves (no individual sizes), fall back to assuming equal split
     * of the combined data_size — this is approximate but better than using
     * the caller's arbitrary buffer sizes. */
    size_t champ_size = (hdr.version >= 2) ? hdr.champion_data_size : (hdr.data_size / 2);
    size_t world_size = (hdr.version >= 2) ? hdr.world_data_size : (hdr.data_size - champ_size);

    /* For safety, cap champion read to available space */
    size_t read_champ = (champ_size <= champion_buf_size) ? champ_size : champion_buf_size;
    size_t read_world = (world_size <= world_buf_size) ? world_size : world_buf_size;

    if (champion_data && read_champ > 0) {
        if (fread(champion_data, 1, read_champ, f) != read_champ) {
            fclose(f); return NEXUS_SAVE_ERR_READ;
        }
    }
    if (world_data && read_world > 0) {
        if (fread(world_data, 1, read_world, f) != read_world) {
            fclose(f); return NEXUS_SAVE_ERR_READ;
        }
    }

    if (out_champion_data_size) *out_champion_data_size = read_champ;
    if (out_world_data_size) *out_world_data_size = read_world;

    /* Verify CRC */
    if (hdr.crc32 != 0) {
        uint32_t crc = crc32_update(0, (const uint8_t *)champion_data, read_champ);
        crc = crc32_update(crc, (const uint8_t *)world_data, read_world);
        crc = crc32_final(crc);
        if (crc != hdr.crc32) {
            fclose(f);
            if (out_diagnostic) snprintf(out_diagnostic, diag_size, "CRC mismatch");
            return NEXUS_SAVE_ERR_CRC;
        }
    }

    fclose(f);
    *out_header = hdr;
    return NEXUS_SAVE_OK;
}

/* ── Default save directory ─────────────────────────────────────────── */

void nexus_v1_save_default_dir(char *buf, size_t bufsz) {
    const char *home = getenv("HOME");
    const char *appdata = getenv("APPDATA");
    const char *base = appdata ? appdata : (home ? home : ".");
    const char *sub = 
#if defined(_WIN32) || defined(_WIN64)
        "Firestaff\\nexus\\saves\\";
#else
        "Library/Application Support/Firestaff/nexus/saves/";
#endif
    snprintf(buf, bufsz, "%s/%s", base, sub);
}

/* ── Save manager ────────────────────────────────────────────────────── */

void nexus_v1_save_init(Nexus_V1_SaveManager *mgr, const char *save_dir) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    if (save_dir && save_dir[0]) {
        size_t len = strlen(save_dir);
        if (len >= sizeof(mgr->save_dir)) len = sizeof(mgr->save_dir) - 1;
        memcpy(mgr->save_dir, save_dir, len);
        mgr->save_dir[len] = '\0';
    } else {
        nexus_v1_save_default_dir(mgr->save_dir, sizeof(mgr->save_dir));
    }
    make_dirs(mgr->save_dir);
    mgr->initialized = 1;
    mgr->slot_count = NEXUS_SAVE_MAX_SLOTS;
}

int nexus_v1_save_scan(Nexus_V1_SaveManager *mgr) {
    int i;
    if (!mgr || !mgr->initialized) return -1;

    for (i = 0; i < NEXUS_SAVE_MAX_SLOTS; i++) {
        char path[512];
        slot_path(mgr->save_dir, (uint8_t)i, path, sizeof(path));

        FILE *f = fopen(path, "rb");
        if (!f) {
            mgr->slots[i].occupied = 0;
            memset(&mgr->slots[i].header, 0, sizeof(mgr->slots[i].header));
            mgr->slots[i].label[0] = '\0';
            mgr->slots[i].timestamp = 0;
            continue;
        }

        Nexus_V1_SaveHeader hdr;
        if (fread(&hdr, sizeof(hdr), 1, f) == 1 && hdr.magic == NEXUS_SAVE_MAGIC) {
            mgr->slots[i].occupied = 1;
            mgr->slots[i].header = hdr;
            mgr->slots[i].slot_index = (uint8_t)i;

            struct stat st;
            if (fstat(fileno(f), &st) == 0)
                mgr->slots[i].timestamp = (uint32_t)st.st_mtime;
            else
                mgr->slots[i].timestamp = 0;

            snprintf(mgr->slots[i].label, sizeof(mgr->slots[i].label),
                     "Level %d (%d,%d) — %s",
                     (int)hdr.current_level,
                     (int)hdr.party_x,
                     (int)hdr.party_y,
                     hdr.description[0] ? hdr.description : "Nexus V1 save");
        } else {
            mgr->slots[i].occupied = 0;
            memset(&mgr->slots[i].header, 0, sizeof(mgr->slots[i].header));
            mgr->slots[i].label[0] = '\0';
            mgr->slots[i].timestamp = 0;
        }
        fclose(f);
    }
    return 0;
}

const Nexus_V1_SaveSlot *nexus_v1_save_get_slot(const Nexus_V1_SaveManager *mgr,
                                                 uint8_t slot) {
    if (!mgr || slot >= NEXUS_SAVE_MAX_SLOTS) return NULL;
    if (!mgr->slots[slot].occupied) return NULL;
    return &mgr->slots[slot];
}

/* ── Public save/load API ────────────────────────────────────────────── */

Nexus_SaveResult nexus_v1_save(Nexus_V1_SaveManager *mgr,
                                uint8_t slot,
                                int32_t current_level,
                                int32_t party_x, int32_t party_y, int32_t party_dir,
                                uint32_t game_time,
                                uint64_t state_hash,
                                const void *champion_data,
                                size_t champion_data_size,
                                const void *world_data,
                                size_t world_data_size) {
    if (!mgr || !mgr->initialized) return NEXUS_SAVE_ERR_DATA_DIR;
    if (slot >= NEXUS_SAVE_MAX_SLOTS) return NEXUS_SAVE_ERR_SLOT_RANGE;

    char path[512];
    slot_path(mgr->save_dir, slot, path, sizeof(path));

    Nexus_SaveResult r = do_save(path, current_level, party_x, party_y, party_dir,
                                  game_time, state_hash,
                                  champion_data, champion_data_size,
                                  world_data, world_data_size);
    if (r == NEXUS_SAVE_OK) {
        /* Update slot cache */
        mgr->slots[slot].occupied = 1;
        header_make(&mgr->slots[slot].header, current_level, party_x, party_y, party_dir,
                    game_time, state_hash, "Nexus V1 save");
        mgr->slots[slot].slot_index = slot;
        mgr->slots[slot].timestamp = (uint32_t)time(NULL);
        snprintf(mgr->slots[slot].label, sizeof(mgr->slots[slot].label),
                 "Level %d (%d,%d)", (int)current_level, (int)party_x, (int)party_y);
    }
    return r;
}

Nexus_SaveResult nexus_v1_load(Nexus_V1_SaveManager *mgr,
                                uint8_t slot,
                                Nexus_V1_SaveHeader *out_header,
                                void *champion_data, size_t champion_buf_size,
                                size_t *out_champion_data_size,
                                void *world_data, size_t world_buf_size,
                                size_t *out_world_data_size,
                                char *out_diagnostic, size_t diag_size) {
    if (!mgr || !mgr->initialized) return NEXUS_SAVE_ERR_DATA_DIR;
    if (slot >= NEXUS_SAVE_MAX_SLOTS) return NEXUS_SAVE_ERR_SLOT_RANGE;

    char path[512];
    slot_path(mgr->save_dir, slot, path, sizeof(path));
    return do_load(path, out_header,
                   champion_data, champion_buf_size, out_champion_data_size,
                   world_data, world_buf_size, out_world_data_size,
                   out_diagnostic, diag_size);
}

Nexus_SaveResult nexus_v1_save_delete(Nexus_V1_SaveManager *mgr, uint8_t slot) {
    if (!mgr || !mgr->initialized) return NEXUS_SAVE_ERR_DATA_DIR;
    if (slot >= NEXUS_SAVE_MAX_SLOTS) return NEXUS_SAVE_ERR_SLOT_RANGE;

    char path[512];
    slot_path(mgr->save_dir, slot, path, sizeof(path));

    if (remove(path) == 0) {
        mgr->slots[slot].occupied = 0;
        memset(&mgr->slots[slot].header, 0, sizeof(mgr->slots[slot].header));
        mgr->slots[slot].label[0] = '\0';
        mgr->slots[slot].timestamp = 0;
        return NEXUS_SAVE_OK;
    }
    return NEXUS_SAVE_ERR_OPEN;
}

/* ── Slot-free convenience API ───────────────────────────────────────── */

Nexus_SaveResult nexus_v1_save_to_path(const char *path,
                                        int32_t current_level,
                                        int32_t party_x, int32_t party_y, int32_t party_dir,
                                        uint32_t game_time,
                                        uint64_t state_hash,
                                        const void *champion_data,
                                        size_t champion_data_size,
                                        const void *world_data,
                                        size_t world_data_size) {
    return do_save(path, current_level, party_x, party_y, party_dir,
                   game_time, state_hash,
                   champion_data, champion_data_size,
                   world_data, world_data_size);
}

Nexus_SaveResult nexus_v1_load_from_path(const char *path,
                                          Nexus_V1_SaveHeader *out_header,
                                          void *champion_data,
                                          size_t champion_buf_size,
                                          size_t *out_champion_data_size,
                                          void *world_data,
                                          size_t world_buf_size,
                                          size_t *out_world_data_size,
                                          char *out_diagnostic,
                                          size_t diag_size) {
    return do_load(path, out_header,
                   champion_data, champion_buf_size, out_champion_data_size,
                   world_data, world_buf_size, out_world_data_size,
                   out_diagnostic, diag_size);
}

/* ── High-level save/load (automatic serialization) ─────────────────── */

Nexus_SaveResult nexus_v1_save_full(Nexus_V1_SaveManager *mgr, uint8_t slot,
                                     int32_t current_level,
                                     int32_t party_x, int32_t party_y, int32_t party_dir,
                                     uint32_t game_time,
                                     uint64_t state_hash,
                                     const void *champion_pool,
                                     const void *world) {
    if (!champion_pool || !world) return NEXUS_SAVE_ERR_NULL;

    /* Serialize champion pool */
    size_t champ_size = nexus_v1_champion_pool_serialize_size((const Nexus_V1_ChampionPool *)champion_pool);
    uint8_t *champ_buf = (uint8_t *)malloc(champ_size);
    if (!champ_buf) return NEXUS_SAVE_ERR_WRITE;
    nexus_v1_champion_pool_serialize((const Nexus_V1_ChampionPool *)champion_pool, champ_buf, champ_size);

    /* Serialize world */
    size_t world_size = nexus_v1_world_serialize_size((const Nexus_V1_World *)world);
    uint8_t *world_buf = (uint8_t *)malloc(world_size);
    if (!world_buf) { free(champ_buf); return NEXUS_SAVE_ERR_WRITE; }
    nexus_v1_world_serialize((const Nexus_V1_World *)world, world_buf, world_size);

    Nexus_SaveResult r = nexus_v1_save(mgr, slot, current_level,
                                        party_x, party_y, party_dir,
                                        game_time, state_hash,
                                        champ_buf, champ_size,
                                        world_buf, world_size);
    free(champ_buf);
    free(world_buf);
    return r;
}

Nexus_SaveResult nexus_v1_load_full(Nexus_V1_SaveManager *mgr, uint8_t slot,
                                      Nexus_V1_SaveHeader *out_header,
                                      void *champion_pool,
                                      void *world,
                                      char *out_diagnostic, size_t diag_size) {
    if (!champion_pool || !world) return NEXUS_SAVE_ERR_NULL;
    if (!mgr || !mgr->initialized) return NEXUS_SAVE_ERR_DATA_DIR;
    if (slot >= NEXUS_SAVE_MAX_SLOTS) return NEXUS_SAVE_ERR_SLOT_RANGE;

    /* First load into temp buffers so we can get the exact sizes */
    size_t champ_size_max = nexus_v1_champion_pool_serialize_size((const Nexus_V1_ChampionPool *)champion_pool);
    size_t world_size_max = nexus_v1_world_serialize_size((const Nexus_V1_World *)world);

    uint8_t *champ_buf = (uint8_t *)malloc(champ_size_max);
    if (!champ_buf) return NEXUS_SAVE_ERR_READ;
    uint8_t *world_buf = (uint8_t *)malloc(world_size_max);
    if (!world_buf) { free(champ_buf); return NEXUS_SAVE_ERR_READ; }

    size_t champ_read = 0, world_read = 0;
    Nexus_SaveResult r = nexus_v1_load(mgr, slot, out_header,
                                        champ_buf, champ_size_max, &champ_read,
                                        world_buf, world_size_max, &world_read,
                                        out_diagnostic, diag_size);
    if (r != NEXUS_SAVE_OK) {
        free(champ_buf);
        free(world_buf);
        return r;
    }

    /* Deserialize into caller's structures */
    int ok = nexus_v1_champion_pool_deserialize((Nexus_V1_ChampionPool *)champion_pool,
                                                 champ_buf, champ_read);
    if (ok != 0) {
        free(champ_buf);
        free(world_buf);
        return NEXUS_SAVE_ERR_READ;
    }
    ok = nexus_v1_world_deserialize((Nexus_V1_World *)world, world_buf, world_read);
    if (ok != 0) {
        free(champ_buf);
        free(world_buf);
        return NEXUS_SAVE_ERR_READ;
    }

    free(champ_buf);
    free(world_buf);
    return NEXUS_SAVE_OK;
}

Nexus_SaveResult nexus_v1_save_full_to_path(const char *path,
                                             int32_t current_level,
                                             int32_t party_x, int32_t party_y, int32_t party_dir,
                                             uint32_t game_time,
                                             uint64_t state_hash,
                                             const void *champion_pool,
                                             const void *world) {
    if (!champion_pool || !world) return NEXUS_SAVE_ERR_NULL;

    size_t champ_size = nexus_v1_champion_pool_serialize_size((const Nexus_V1_ChampionPool *)champion_pool);
    uint8_t *champ_buf = (uint8_t *)malloc(champ_size);
    if (!champ_buf) return NEXUS_SAVE_ERR_WRITE;
    nexus_v1_champion_pool_serialize((const Nexus_V1_ChampionPool *)champion_pool, champ_buf, champ_size);

    size_t world_size = nexus_v1_world_serialize_size((const Nexus_V1_World *)world);
    uint8_t *world_buf = (uint8_t *)malloc(world_size);
    if (!world_buf) { free(champ_buf); return NEXUS_SAVE_ERR_WRITE; }
    nexus_v1_world_serialize((const Nexus_V1_World *)world, world_buf, world_size);

    Nexus_SaveResult r = nexus_v1_save_to_path(path, current_level,
                                                party_x, party_y, party_dir,
                                                game_time, state_hash,
                                                champ_buf, champ_size,
                                                world_buf, world_size);
    free(champ_buf);
    free(world_buf);
    return r;
}

Nexus_SaveResult nexus_v1_load_full_from_path(const char *path,
                                                Nexus_V1_SaveHeader *out_header,
                                                void *champion_pool,
                                                void *world,
                                                char *out_diagnostic, size_t diag_size) {
    if (!champion_pool || !world) return NEXUS_SAVE_ERR_NULL;

    size_t champ_size_max = nexus_v1_champion_pool_serialize_size((const Nexus_V1_ChampionPool *)champion_pool);
    size_t world_size_max = nexus_v1_world_serialize_size((const Nexus_V1_World *)world);

    uint8_t *champ_buf = (uint8_t *)malloc(champ_size_max);
    if (!champ_buf) return NEXUS_SAVE_ERR_READ;
    uint8_t *world_buf = (uint8_t *)malloc(world_size_max);
    if (!world_buf) { free(champ_buf); return NEXUS_SAVE_ERR_READ; }

    size_t champ_read = 0, world_read = 0;
    Nexus_SaveResult r = nexus_v1_load_from_path(path, out_header,
                                                  champ_buf, champ_size_max, &champ_read,
                                                  world_buf, world_size_max, &world_read,
                                                  out_diagnostic, diag_size);
    if (r != NEXUS_SAVE_OK) {
        free(champ_buf);
        free(world_buf);
        return r;
    }

    int ok = nexus_v1_champion_pool_deserialize((Nexus_V1_ChampionPool *)champion_pool,
                                                 champ_buf, champ_read);
    if (ok != 0) {
        free(champ_buf);
        free(world_buf);
        return NEXUS_SAVE_ERR_READ;
    }
    ok = nexus_v1_world_deserialize((Nexus_V1_World *)world, world_buf, world_read);
    if (ok != 0) {
        free(champ_buf);
        free(world_buf);
        return NEXUS_SAVE_ERR_READ;
    }

    free(champ_buf);
    free(world_buf);
    return NEXUS_SAVE_OK;
}
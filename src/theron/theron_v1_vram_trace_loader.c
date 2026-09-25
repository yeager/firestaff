/*
 * theron_v1_vram_trace_loader.c — Load real PCE VRAM/VCE snapshots
 *
 * Loads binary VRAM (64KB) and VCE (1KB) dumps captured from Mednafen
 * save states into the viewport's tile/palette system. This enables
 * rendering with authentic game data instead of synthetic placeholders.
 *
 * VRAM layout (word-addressed, 64KB = 32K words):
 *   BAT bits 0..10 are hardware tile indices.  Each index addresses one
 *   32-byte background pattern from VRAM byte zero; there is no relocatable
 *   host-side tile base.  A historical fixture used a synthetic $1000 base,
 *   but applying it to retail captures shifts every selected pattern.
 *
 * VCE layout: 512 × 16-bit LE words, BGR333 format.
 */

#include "theron_v1_vram_trace_loader.h"
#include "theron_v1_mednafen_vdc_io_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint32_t theron_vram_trace_fnv1a_file(const char *path) {
    FILE *file;
    uint8_t buffer[4096];
    size_t count;
    uint32_t hash = 0x811c9dc5u;

    if (!path || !(file = fopen(path, "rb"))) return 0u;
    while ((count = fread(buffer, 1u, sizeof(buffer), file)) != 0u) {
        for (size_t i = 0u; i < count; ++i) {
            hash ^= buffer[i];
            hash *= 0x01000193u;
        }
    }
    if (ferror(file)) hash = 0u;
    fclose(file);
    return hash;
}

int theron_v1_vram_trace_load_raw(Theron_V1_Viewport *vp,
                                  const uint8_t *vram_data, int vram_size,
                                  const uint8_t *vce_data, int vce_size) {
    if (!vp || !vram_data || !vce_data) return -1;
    /* A raw snapshot is a complete VDC/VCE capture, not a prefix view into a
     * larger container.  The file-backed path already enforces these exact
     * lengths; keep the in-memory API equally strict so callers cannot
     * accidentally admit concatenated or container-tainted bytes. */
    if (vram_size != THERON_VRAM_SIZE || vce_size != THERON_VCE_SIZE) return -1;

    if (!vp->vram_trace_data) {
        vp->vram_trace_data = (uint8_t *)malloc(THERON_VRAM_SIZE);
        if (!vp->vram_trace_data) return -1;
    }
    if (!vp->vce_trace_data) {
        vp->vce_trace_data = (uint8_t *)malloc(THERON_VCE_SIZE);
        if (!vp->vce_trace_data) {
            free(vp->vram_trace_data);
            vp->vram_trace_data = NULL;
            return -1;
        }
    }

    memcpy(vp->vram_trace_data, vram_data, THERON_VRAM_SIZE);
    memcpy(vp->vce_trace_data, vce_data, THERON_VCE_SIZE);

    for (int i = 0; i < THERON_VDC_BAT_MAX_WORDS; ++i)
        vp->bat_atlas_indices[i] = -1;
    vp->vce_palette_relation_verified = 0;
    vp->bat_palette_group_mask = 0;

    tqr_palette_load_group(&vp->palette, vce_data, 0, 512);

    vp->vram_trace_loaded = 1;
    return 0;
}

int theron_v1_vram_trace_load_files(Theron_V1_Viewport *vp,
                                    const char *vram_path,
                                    const char *vce_path) {
    if (!vp || !vram_path || !vce_path) return -1;

    FILE *fv = fopen(vram_path, "rb");
    if (!fv) return -1;
    fseek(fv, 0, SEEK_END);
    long vram_sz = ftell(fv);
    /* The capture contract is an exact raw VDC snapshot.  Do not silently
     * truncate a concatenated/contaminated file into an apparently valid
     * source bank. */
    if (vram_sz != THERON_VRAM_SIZE) { fclose(fv); return -1; }
    fseek(fv, 0, SEEK_SET);

    uint8_t *vram = (uint8_t *)malloc(THERON_VRAM_SIZE);
    if (!vram) { fclose(fv); return -1; }
    if (fread(vram, 1, THERON_VRAM_SIZE, fv) != THERON_VRAM_SIZE) {
        free(vram); fclose(fv); return -1;
    }
    fclose(fv);

    FILE *fc = fopen(vce_path, "rb");
    if (!fc) { free(vram); return -1; }
    fseek(fc, 0, SEEK_END);
    long vce_sz = ftell(fc);
    if (vce_sz != THERON_VCE_SIZE) {
        free(vram);
        fclose(fc);
        return -1;
    }
    fseek(fc, 0, SEEK_SET);

    uint8_t *vce = (uint8_t *)malloc(THERON_VCE_SIZE);
    if (!vce) { free(vram); fclose(fc); return -1; }
    if (fread(vce, 1, THERON_VCE_SIZE, fc) != THERON_VCE_SIZE) {
        free(vram); free(vce); fclose(fc); return -1;
    }
    fclose(fc);

    int rc = theron_v1_vram_trace_load_raw(vp, vram, THERON_VRAM_SIZE,
                                           vce, THERON_VCE_SIZE);
    free(vram);
    free(vce);
    return rc;
}

int theron_v1_vram_trace_load_verified_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    uint32_t expected_vram_fnv1a,
    uint32_t expected_vce_fnv1a) {
    if (!vp || !vram_path || !vce_path || expected_vram_fnv1a == 0u ||
        expected_vce_fnv1a == 0u ||
        theron_vram_trace_fnv1a_file(vram_path) != expected_vram_fnv1a ||
        theron_vram_trace_fnv1a_file(vce_path) != expected_vce_fnv1a) {
        return -1;
    }
    return theron_v1_vram_trace_load_files(vp, vram_path, vce_path);
}

int theron_v1_vram_trace_load_known_capture_files(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path) {
    /* Complete FNV-1a identities from authenticated external captures.  The
     * old pair is retained for backwards-compatible capture replay.  The
     * later pairs are the US dungeon, US interactive, JP startup and US
     * cold-start screen-space receipts respectively.  Hash admission is the
     * only new capability here; no VDC frame is interpreted as a dungeon
     * square or object record. */
    static const struct {
        uint32_t vram;
        uint32_t vce;
    } known[] = {
        {0xf11c6b2au, 0xea83f117u},
        {0x5c830cc2u, 0x6fb303b5u},
        {0x4f15b98cu, 0x71cc9b11u},
        {0x8ae1e419u, 0x4e48c361u},
        {0x1a37c99bu, 0x71cc9b11u},
        /* 2026-08-09 authenticated active-dungeon screen capture. */
        {0x105dcffbu, 0xea83f117u},
        /* 2026-08-10 corrected cold-start transport capture.  This pair
         * owns only the captured VDC/VCE bitmap and palette banks; the
         * dungeon square/material consumer remains deliberately closed. */
        {0x4a2186a2u, 0xaa11c4f2u},
        /* 2026-08-10 manual dungeon capture from the authenticated US
         * Track 02/System Card session.  Its VCE snapshot is the same
         * source-owned palette bank, while the VRAM image is a later
         * screen-state snapshot. */
        {0x5d20ebc7u, 0xea83f117u},
        /* 2026-08-11 bounded replay from the authenticated US Track 02
         * medium/System Card pair.  This is a new screen-space VDC image
         * with the previously admitted source VCE bank; it authorizes only
         * bitmap/tile/palette replay, never square or object semantics. */
        {0x42a483acu, 0x6fb303b5u},
        /* 2026-08-12 autoload combat capture.  The session is negative for
         * gameplay handoff, but its complete VDC/VCE snapshots are real
         * source-owned screen bytes and may be replayed screen-space only. */
        {0x411960ebu, 0x6fb303b5u},
        /* 2026-08-09 clean external replay.  The transition receipt for
         * this run is intentionally negative, so this pair authorizes only
         * the complete source VDC/VCE screen route, not level or gameplay
         * ownership. */
        {0xa449538au, 0xea83f117u},
        /* 2026-08-13 authenticated CUE/state replay through the patched
         * Mednafen capture producer.  This is a distinct live VRAM image
         * paired with the already admitted source VCE bank; it remains a
         * screen-space-only route. */
        {0x8165c4d4u, 0xea83f117u},
        /* 2026-08-13 authenticated RAM/VDC replay.  The capture has a
         * complete source VDC/VCE pair, but its $2600 window is still an
         * initialization/readback loop and does not identify a level,
         * object, square, HUD or gameplay consumer. */
        {0x087da136u, 0x5376a91bu},
        /* 2026-08-14 external-disk r25 direct-provenance capture.  Its
         * source-LBA/$611D/$C3A0 join is admitted only as screen-space
         * capture data; level/object/gameplay semantics remain closed. */
        {0xe08b571du, 0x298f9642u},
        /* 2026-08-14 r30 state replay.  The runtime spawn/level join is
         * negative, but the complete VDC/VCE pair is still an authenticated
         * screen-space frame and may be replayed without semantic promotion. */
        {0xee9374fau, 0xc17c0a95u},
        /* 2026-09-25 authentic US Akutuba-save replay. The original BRAM,
         * Track 02, System Card, and instrumented emulator produced this
         * complete screen pair; its receipt has no CD-to-RAM join or level
         * transition, so this admits pixels only. */
        {0x59ef2648u, 0x6fb303b5u}
    };

    if (!vp || !vram_path || !vce_path) return -1;
    for (size_t i = 0u; i < sizeof(known) / sizeof(known[0]); ++i) {
        if (theron_v1_vram_trace_load_verified_files(
                vp, vram_path, vce_path, known[i].vram, known[i].vce) == 0) {
            return 0;
        }
    }
    return -1;
}

static int theron_v1_vdc_state_load_verified(
    Theron_V1_Viewport *vp, const char *path, uint32_t expected_fnv1a) {
    FILE *file;
    char header[64];
    char row[256];
    unsigned bxr, byr, mwr, hsr, hdr, vsr, vdr, vcr, cr;
    static const uint16_t widths[4] = {32u, 64u, 128u, 128u};
    int trailing;

    if (!vp || !path || !path[0] || expected_fnv1a == 0u ||
        theron_vram_trace_fnv1a_file(path) != expected_fnv1a ||
        !(file = fopen(path, "r"))) return -1;
    if (!fgets(header, sizeof(header), file) ||
        strcmp(header, "FIRESTAFF_THERON_VDC_STATE_V1\n") != 0 ||
        !fgets(row, sizeof(row), file) ||
        sscanf(row,
               "vdc=0 bxr=%4x byr=%4x mwr=%4x hsr=%4x hdr=%4x vsr=%4x vdr=%4x vcr=%4x cr=%4x",
               &bxr, &byr, &mwr, &hsr, &hdr, &vsr, &vdr, &vcr, &cr) != 9) {
        fclose(file);
        return -1;
    }
    do {
        trailing = fgetc(file);
    } while (trailing == '\n' || trailing == '\r' || trailing == ' ' ||
             trailing == '\t');
    fclose(file);
    if (trailing != EOF || bxr > 0x03ffu || byr > 0x01ffu ||
        vdr > 0x01ffu) return -1;

    vp->vdc_bxr = (uint16_t)bxr;
    vp->vdc_byr = (uint16_t)byr;
    vp->vdc_mwr = (uint16_t)mwr;
    vp->vdc_hdr = (uint16_t)hdr;
    vp->vdc_vdr = (uint16_t)vdr;
    vp->vdc_cr = (uint16_t)cr;
    vp->vdc_bat_width = widths[(mwr >> 4) & 3u];
    vp->vdc_bat_height = (mwr & 0x40u) ? 64u : 32u;
    vp->vdc_display_width = (uint16_t)(((hdr & 0x7fu) + 1u) * 8u);
    vp->vdc_display_height = (uint16_t)((vdr & 0x01ffu) + 1u);
    /* The first admitted bundle is deliberately exact.  Later geometries
     * require their own authenticated triple and framebuffer audit. */
    if (vp->vdc_bat_width != 64u || vp->vdc_bat_height != 64u ||
        vp->vdc_display_width != 320u || vp->vdc_display_height != 200u)
        return -1;
    vp->vdc_state_loaded = 1;
    (void)hsr;
    (void)vsr;
    (void)vcr;
    return 0;
}

int theron_v1_vram_trace_load_known_capture_bundle(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    const char *vdc_state_path,
    const char *vdc_sat_path) {
    /* Legacy source compatibility only.  Four files cannot prove that the
     * snapshot and producer stream share an endpoint; production requires
     * theron_v1_vram_trace_load_known_atomic_capture_bundle(). */
    (void)vp;
    (void)vram_path;
    (void)vce_path;
    (void)vdc_state_path;
    (void)vdc_sat_path;
    return -1;
}

int theron_v1_vram_trace_load_known_atomic_capture_bundle(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    const char *vdc_state_path,
    const char *vdc_sat_path,
    const char *vdc_io_path) {
    Theron_V1VdcIoTrace trace;
    Theron_V1VdcIoVramReplayReceipt replay;
    static const struct {
        uint32_t vram_fnv1a;
        uint32_t vce_fnv1a;
        uint32_t vdc_state_fnv1a;
        uint32_t sat_fnv1a;
        uint32_t vdc_io_fnv1a;
        uint32_t vwr_commit_count;
        uint32_t written_word_count;
    } known[] = {
        /* Stable authenticated active-dungeon frame. */
        {0x6cb9f191u, 0x6fb303b5u, 0x61478026u, 0x80f4c57bu,
         0x282eac4au, 25890u, 8816u},
        /* Same original savestate/media after scripted RIGHT at emulated
         * frame 1 for ten frames. The original input poll observed raw
         * $0020 / nibble $3d before this distinct atomic screen formed. */
        {0x7dd24066u, 0x6fb303b5u, 0x61478026u, 0x7e6da7e3u,
         0xb96a56e5u, 25890u, 8784u},
        /* Same source state with LEFT $0080 / nibble $37. */
        {0xe4261226u, 0x6fb303b5u, 0x61478026u, 0x484aaf93u,
         0x2e37df95u, 25890u, 8784u},
        /* 2026-09-25 authentic US Akutuba-save replay. Exact final VDC
         * snapshot boundary and write replay match the source VRAM image;
         * zero CD-to-RAM receipts and no game-owned $E009 dispatch mean
         * this remains screen-space only, with no level semantics. */
        {0x59ef2648u, 0x6fb303b5u, 0x21d291e3u, 0xf8cc0675u,
         0x49d2ae18u, 27556u, 9360u}
    };
    FILE *sat_file;
    size_t known_index;
    int replay_verified;

    if (!vp || !vram_path || !vce_path || !vdc_state_path ||
        !vdc_sat_path || !vdc_io_path) {
        return -1;
    }
    for (known_index = 0u;
         known_index < sizeof(known) / sizeof(known[0]);
         ++known_index) {
        if (theron_vram_trace_fnv1a_file(vram_path) ==
                known[known_index].vram_fnv1a &&
            theron_vram_trace_fnv1a_file(vce_path) ==
                known[known_index].vce_fnv1a &&
            theron_vram_trace_fnv1a_file(vdc_state_path) ==
                known[known_index].vdc_state_fnv1a &&
            theron_vram_trace_fnv1a_file(vdc_sat_path) ==
                known[known_index].sat_fnv1a &&
            theron_vram_trace_fnv1a_file(vdc_io_path) ==
                known[known_index].vdc_io_fnv1a)
            break;
    }
    if (known_index == sizeof(known) / sizeof(known[0]) ||
        !theron_v1_mednafen_vdc_io_trace_load_file(vdc_io_path, &trace))
        return -1;
    replay_verified = theron_v1_mednafen_vdc_io_verify_vram_snapshot(
        &trace, vram_path, &replay);
    theron_v1_mednafen_vdc_io_trace_free(&trace);
    if (!replay_verified ||
        replay.vwr_commit_count != known[known_index].vwr_commit_count ||
        replay.written_word_count != known[known_index].written_word_count ||
        replay.matched_word_count != known[known_index].written_word_count ||
        replay.mismatched_word_count != 0u ||
        replay.semantic_publication_allowed) return -1;

    if (theron_v1_vram_trace_load_verified_files(
            vp, vram_path, vce_path, known[known_index].vram_fnv1a,
            known[known_index].vce_fnv1a) != 0 ||
        theron_v1_vdc_state_load_verified(vp, vdc_state_path,
                                          known[known_index].vdc_state_fnv1a) != 0 ||
        theron_vram_trace_fnv1a_file(vdc_sat_path) !=
            known[known_index].sat_fnv1a ||
        !(sat_file = fopen(vdc_sat_path, "rb"))) {
        if (vp->vram_trace_loaded) theron_v1_vram_trace_unload(vp);
        return -1;
    }
    vp->vdc_sat_trace_data = (uint8_t *)malloc(THERON_VDC_SAT_SIZE);
    if (!vp->vdc_sat_trace_data ||
        fread(vp->vdc_sat_trace_data, 1, THERON_VDC_SAT_SIZE, sat_file) !=
            THERON_VDC_SAT_SIZE || fgetc(sat_file) != EOF) {
        fclose(sat_file);
        theron_v1_vram_trace_unload(vp);
        return -1;
    }
    fclose(sat_file);
    vp->vdc_sat_loaded = 1;
    return 0;
}

int theron_v1_vram_trace_load_known_atomic_input_capture_bundle(
    Theron_V1_Viewport *vp,
    const char *vram_path,
    const char *vce_path,
    const char *vdc_state_path,
    const char *vdc_sat_path,
    const char *vdc_io_path,
    const char *input_path,
    const char *transition_path) {
    uint16_t mask;
    uint16_t result;
    uint32_t input_hash;
    uint32_t transition_hash;
    uint32_t vram_hash;
    uint32_t io_hash;
    if (!vp || !input_path || !transition_path) return -1;
    input_hash = theron_vram_trace_fnv1a_file(input_path);
    transition_hash = theron_vram_trace_fnv1a_file(transition_path);
    vram_hash = theron_vram_trace_fnv1a_file(vram_path);
    io_hash = theron_vram_trace_fnv1a_file(vdc_io_path);
    if (input_hash == 0xd5791561u && transition_hash == 0x71f9ac0cu &&
        vram_hash == 0x7dd24066u && io_hash == 0xb96a56e5u) {
        mask = 0x0020u;
        result = 0x003du;
    } else if (input_hash == 0x811e9c8fu &&
               transition_hash == 0x71822191u &&
               vram_hash == 0xe4261226u && io_hash == 0x2e37df95u) {
        mask = 0x0080u;
        result = 0x0037u;
    } else {
        return -1;
    }
    if (theron_v1_vram_trace_load_known_atomic_capture_bundle(
            vp, vram_path, vce_path, vdc_state_path, vdc_sat_path,
            vdc_io_path) != 0)
        return -1;
    vp->vdc_input_screen_relation_verified = 1;
    vp->vdc_source_input_mask = mask;
    vp->vdc_source_input_result = result;
    vp->vdc_source_input_hold_frames = 10u;
    return 0;
}

int theron_v1_vram_trace_load_tqtr(Theron_V1_Viewport *vp,
                                   const char *tqtr_path) {
    if (!vp || !tqtr_path) return -1;

    FILE *f = fopen(tqtr_path, "rb");
    if (!f) return -1;

    /* TQTR header: "TQTR" (4) + version (4) + screen_type (4) +
     * vram_size (4) + vce_size (4) + cdram_size (4) + sysram_size (4) = 28 */
    uint8_t hdr[28];
    if (fread(hdr, 1, 28, f) != 28) { fclose(f); return -1; }
    if (memcmp(hdr, "TQTR", 4) != 0) { fclose(f); return -1; }

    uint32_t vram_sz = hdr[12] | (hdr[13] << 8) | (hdr[14] << 16) | (hdr[15] << 24);
    uint32_t vce_sz  = hdr[16] | (hdr[17] << 8) | (hdr[18] << 16) | (hdr[19] << 24);

    if (vram_sz < THERON_VRAM_SIZE || vce_sz < THERON_VCE_SIZE) {
        fclose(f); return -1;
    }

    uint8_t *vram = (uint8_t *)malloc(THERON_VRAM_SIZE);
    uint8_t *vce  = (uint8_t *)malloc(THERON_VCE_SIZE);
    if (!vram || !vce) { free(vram); free(vce); fclose(f); return -1; }

    if (fread(vram, 1, THERON_VRAM_SIZE, f) != THERON_VRAM_SIZE ||
        /* TQTR may carry an extended VRAM segment.  The VCE segment starts
         * after the complete declared VRAM span, not after the 64 KiB slice
         * Firestaff consumes.  Skipping the extension keeps the palette
         * aligned with the source container instead of admitting shifted
         * colours as a seemingly valid capture. */
        (vram_sz > THERON_VRAM_SIZE &&
         fseek(f, (long)(vram_sz - THERON_VRAM_SIZE), SEEK_CUR) != 0) ||
        fread(vce, 1, THERON_VCE_SIZE, f) != THERON_VCE_SIZE) {
        free(vram); free(vce); fclose(f); return -1;
    }
    fclose(f);

    int rc = theron_v1_vram_trace_load_raw(vp, vram, THERON_VRAM_SIZE,
                                           vce, THERON_VCE_SIZE);
    free(vram);
    free(vce);
    return rc;
}

void theron_v1_vram_trace_unload(Theron_V1_Viewport *vp) {
    if (!vp) return;
    free(vp->vram_trace_data);
    free(vp->vce_trace_data);
    free(vp->vdc_sat_trace_data);
    free(vp->vdc_source_frame);
    vp->vram_trace_data = NULL;
    vp->vce_trace_data = NULL;
    vp->vdc_sat_trace_data = NULL;
    vp->vdc_source_frame = NULL;
    vp->vram_trace_loaded = 0;
    vp->vdc_state_loaded = 0;
    vp->vdc_sat_loaded = 0;
    vp->vdc_input_screen_relation_verified = 0;
    vp->vdc_source_input_mask = 0u;
    vp->vdc_source_input_result = 0u;
    vp->vdc_source_input_hold_frames = 0u;
    vp->host_palette_source_count = 0;
    vp->vce_palette_relation_verified = 0;
    vp->bat_palette_group_mask = 0;
    for (int i = 0; i < THERON_VDC_BAT_MAX_WORDS; ++i)
        vp->bat_atlas_indices[i] = -1;
    tqr_palette_free_tiles(&vp->palette);
}

static int theron_v1_vram_trace_populate_tiles_with_base(
    Theron_V1_Viewport *vp, int bat_start_word, int bat_w, int bat_h,
    int tile_base_byte) {
    if (!vp || !vp->vram_trace_loaded || !vp->vram_trace_data) return -1;
    /* The authenticated MWR may select a 64x64 BAT. */
    if (bat_start_word < 0 || bat_w <= 0 || bat_h <= 0 ||
        bat_w > 64 || bat_h > 64 ||
        bat_start_word >= THERON_VDC_BAT_MAX_WORDS ||
        bat_start_word + bat_w * bat_h > THERON_VDC_BAT_MAX_WORDS) {
        return -1;
    }

    const uint8_t *vram = vp->vram_trace_data;
    int tile_map[2048][TQR_PALETTE_GROUPS];

    /* A BAT word is the source-owned VDC mapping: bits 0..10 select the
     * background tile and bits 12..15 select its palette group.  Keep one
     * atlas entry per (tile, palette) pair because the same source tile can
     * be displayed with multiple VCE groups.  Do not scan the whole VRAM or
     * invent group 0 for every non-zero tile; that loses the actual scene
     * binding carried by the captured BAT. */
    for (int tile = 0; tile < 2048; ++tile) {
        for (int group = 0; group < TQR_PALETTE_GROUPS; ++group) {
            tile_map[tile][group] = -1;
        }
    }

    if (tile_base_byte < 0 || tile_base_byte >= THERON_VRAM_SIZE)
        return -1;

    tqr_palette_free_tiles(&vp->palette);
    for (int i = 0; i < THERON_VDC_BAT_MAX_WORDS; ++i)
        vp->bat_atlas_indices[i] = -1;

    int loaded = 0;
    uint16_t palette_group_mask = 0;
    for (int y = 0; y < bat_h; ++y) {
        for (int x = 0; x < bat_w; ++x) {
            int bat_word = bat_start_word + y * bat_w + x;
            uint16_t bat = (uint16_t)vram[bat_word * 2] |
                           ((uint16_t)vram[bat_word * 2 + 1] << 8);
            int tile_index = (int)(bat & 0x0FFFu);
            int pal_group = (int)((bat >> 12) & 0x0Fu);
            int off;
            int atlas_index;

            /* HuC6270 BAT exposes twelve tile bits, but the retail PCE has
             * 64 KiB VRAM: only 2,048 32-byte background patterns exist.
             * Mednafen treats a set bit 11 as an unmapped background read.
             * Masking it away aliases the cell to unrelated real graphics
             * and produces a colourful but false frame. */
            if (tile_index >= 2048) continue;
            off = tile_base_byte + tile_index * THERON_VRAM_TILE_BYTES;

            if (off < tile_base_byte ||
                off + THERON_VRAM_TILE_BYTES > 0xFE00 ||
                tile_map[tile_index][pal_group] >= 0) {
                if (off >= tile_base_byte &&
                    off + THERON_VRAM_TILE_BYTES <= 0xFE00 &&
                    tile_map[tile_index][pal_group] >= 0) {
                    vp->bat_atlas_indices[bat_word] =
                        (int16_t)tile_map[tile_index][pal_group];
                }
                continue;
            }
            atlas_index = tqr_tile_load_from_data(
                &vp->palette, vram + off, 4, pal_group, "vram_trace");
            if (atlas_index < 0) return -1;
            palette_group_mask |= (uint16_t)(1u << pal_group);
            vp->palette.tiles[atlas_index].vram_index = (uint16_t)tile_index;
            tile_map[tile_index][pal_group] = atlas_index;
            vp->bat_atlas_indices[bat_word] = (int16_t)atlas_index;
            ++loaded;
        }
    }

    /* The VCE snapshot is the exact source of the 16-entry groups selected
     * by the BAT words above.  Recheck the native little-endian words before
     * publishing the relation so a future caller cannot accidentally treat
     * a decoded tile atlas with an unrelated palette as source-bound. */
    if (loaded > 0 && vp->vce_trace_data) {
        int relation_ok = 1;
        for (int group = 0; group < TQR_PALETTE_GROUPS && relation_ok; ++group) {
            if ((palette_group_mask & (uint16_t)(1u << group)) == 0) continue;
            for (int color = 0; color < TQR_PALETTE_GROUP_SIZE; ++color) {
                int entry = group * TQR_PALETTE_GROUP_SIZE + color;
                uint16_t native = (uint16_t)vp->vce_trace_data[entry * 2] |
                                   ((uint16_t)vp->vce_trace_data[entry * 2 + 1] << 8);
                if (vp->palette.entries[entry].bgr333 != native) {
                    relation_ok = 0;
                    break;
                }
            }
        }
        vp->bat_palette_group_mask = palette_group_mask;
        vp->vce_palette_relation_verified = relation_ok;
    }

    return loaded;
}

int theron_v1_vram_trace_populate_tiles(Theron_V1_Viewport *vp,
                                        int bat_start_word,
                                        int bat_w, int bat_h) {
    /* HuC6270 background patterns are addressed directly by the BAT tile
     * index.  Never reintroduce the old synthetic fixture offset here: it
     * can still produce in-bounds bytes and therefore looks superficially
     * successful while rendering unrelated retail patterns. */
    return theron_v1_vram_trace_populate_tiles_with_base(
        vp, bat_start_word, bat_w, bat_h, 0);
}

int theron_v1_vram_trace_bat_atlas_index(const Theron_V1_Viewport *vp,
                                         int bat_word) {
    if (!vp || !vp->vram_trace_loaded || bat_word < 0 ||
        bat_word >= THERON_VDC_BAT_MAX_WORDS)
        return -1;
    return vp->bat_atlas_indices[bat_word];
}

int theron_v1_vram_trace_palette_relation_verified(
    const Theron_V1_Viewport *vp) {
    return vp && vp->vram_trace_loaded &&
           vp->vce_palette_relation_verified &&
           vp->bat_palette_group_mask != 0;
}

int theron_v1_vram_trace_render_bat_preview(Theron_V1_Viewport *vp,
                                            int bat_start_word,
                                            int bat_w,
                                            int bat_h,
                                            int dst_x,
                                            int dst_y) {
    int copied = 0;

    if (!vp || !vp->vram_trace_loaded || !vp->fb.data ||
        bat_start_word < 0 || bat_w <= 0 || bat_h <= 0 ||
        bat_w > 64 || bat_h > 32 || bat_start_word >= 2048 ||
        bat_start_word + (bat_h - 1) * 64 + (bat_w - 1) >= 2048 ||
        dst_x < 0 || dst_y < 0 || dst_x + bat_w * TQR_TILE_DIM > vp->fb.w ||
        dst_y + bat_h * TQR_TILE_DIM > vp->fb.h) {
        return -1;
    }

    for (int y = 0; y < bat_h; ++y) {
        for (int x = 0; x < bat_w; ++x) {
            int bat_word = bat_start_word + y * 64 + x;
            int atlas_index = vp->bat_atlas_indices[bat_word];
            const TQR_Tile *tile;
            uint8_t decoded_tile[64];

            if (atlas_index < 0 || atlas_index >= vp->palette.tile_count)
                continue;
            tile = &vp->palette.tiles[atlas_index];
            if (!tile->data) continue;
            /* The atlas keeps the source-owned PCE planar bytes.  Decode
             * them before preview/presentation; treating the 32 raw 4bpp
             * bytes as 64 indexed pixels produces a plausible-looking but
             * incorrect screen and bypasses the real bitmap decoder. */
            tqr_decode_tile(decoded_tile, tile->data, tile->bpp);
            for (int row = 0; row < TQR_TILE_DIM; ++row) {
                uint8_t *dst = vp->fb.data +
                    (dst_y + y * TQR_TILE_DIM + row) * vp->fb.stride +
                    dst_x + x * TQR_TILE_DIM;
                const uint8_t *src = decoded_tile + row * TQR_TILE_DIM;
                uint8_t palette_base = (uint8_t)(tile->pal_group *
                                                 TQR_PALETTE_GROUP_SIZE);
                for (int px = 0; px < TQR_TILE_DIM; ++px) {
                    /* BAT bits 12..15 select the VCE group.  Keep that
                     * source-owned group in the indexed frame; copying only
                     * the four-bit tile value silently collapsed every real
                     * palette group into group zero. */
                    /* HuC6260 shares background colour zero across all BG
                     * palette groups.  Group-local entry zero is not drawn;
                     * Mednafen resolves it to VCE entry 0. */
                    dst[px] = src[px] == 0u
                        ? 0u
                        : (uint8_t)(palette_base + src[px]);
                }
            }
            ++copied;
        }
    }
    return copied;
}

int theron_v1_vram_trace_render_authenticated_screen(Theron_V1_Viewport *vp) {
    if (!vp || !vp->vram_trace_loaded || !vp->fb.data) {
        return -1;
    }
    if (vp->vdc_state_loaded) {
        int rendered = 0;
        int bat_width = vp->vdc_bat_width;
        int bat_height = vp->vdc_bat_height;
        size_t frame_pixels;
        if (bat_width <= 0 || bat_height <= 0 ||
            vp->vdc_display_width > vp->fb.w ||
            vp->vdc_display_height > vp->fb.h || !vp->vdc_sat_loaded ||
            !vp->vdc_sat_trace_data) return -1;
        frame_pixels = (size_t)vp->vdc_display_width *
                       (size_t)vp->vdc_display_height;
        if (!vp->vdc_source_frame) {
            vp->vdc_source_frame = (uint16_t *)malloc(
                frame_pixels * sizeof(*vp->vdc_source_frame));
            if (!vp->vdc_source_frame) return -1;
        }
        memset(vp->fb.data, 0,
               (size_t)vp->fb.stride * (size_t)vp->fb.h);
        memset(vp->vdc_source_frame, 0,
               frame_pixels * sizeof(*vp->vdc_source_frame));
        for (int y = 0; y < vp->vdc_display_height; ++y) {
            unsigned sy = ((unsigned)vp->vdc_byr + (unsigned)y) &
                          (unsigned)(bat_height * TQR_TILE_DIM - 1);
            for (int x = 0; x < vp->vdc_display_width; ++x) {
                unsigned sx = ((unsigned)vp->vdc_bxr + (unsigned)x) &
                              (unsigned)(bat_width * TQR_TILE_DIM - 1);
                int bat_word = (int)((sy >> 3) * (unsigned)bat_width +
                                     (sx >> 3));
                int atlas_index = vp->bat_atlas_indices[bat_word];
                const TQR_Tile *tile;
                uint8_t decoded[64];
                uint8_t pixel;
                if (atlas_index < 0 || atlas_index >= vp->palette.tile_count)
                    continue;
                tile = &vp->palette.tiles[atlas_index];
                if (!tile->data) continue;
                tqr_decode_tile(decoded, tile->data, tile->bpp);
                pixel = decoded[(sy & 7u) * 8u + (sx & 7u)];
                vp->vdc_source_frame[y * vp->vdc_display_width + x] = pixel == 0u
                    ? 0u
                    : (uint16_t)(tile->pal_group * TQR_PALETTE_GROUP_SIZE +
                                 pixel);
                ++rendered;
            }
        }
        /* HuC6270 SAT composition.  The hardware selects the first sixteen
         * matching SAT pieces for each scanline, then draws them in reverse
         * order so the lower SAT number wins.  A 32-pixel sprite consumes
         * two pieces, exactly as Mednafen's FetchSpriteData path does. */
        for (int y = 0; y < vp->vdc_display_height; ++y) {
            struct SpritePiece {
                uint16_t flags, pattern, palette;
                int x, row;
            } active[16];
            int active_count = 0;
            for (int sprite = 0; sprite < 64 && active_count < 16; ++sprite) {
                const uint8_t *s = vp->vdc_sat_trace_data + sprite * 8;
                uint16_t syw = (uint16_t)s[0] | ((uint16_t)s[1] << 8);
                uint16_t sxw = (uint16_t)s[2] | ((uint16_t)s[3] << 8);
                uint16_t pn = (uint16_t)s[4] | ((uint16_t)s[5] << 8);
                uint16_t flags = (uint16_t)s[6] | ((uint16_t)s[7] << 8);
                static const int heights[4] = {16, 32, 64, 64};
                static const unsigned masks[4] = {~0u, ~2u, ~6u, ~6u};
                int sy = (int)(syw & 0x03ffu) - 0x40;
                int height = heights[(flags >> 12) & 3u];
                int width = (flags & 0x0100u) ? 32 : 16;
                int row;
                unsigned base;
                if (y < sy || y >= sy + height) continue;
                row = y - sy;
                if (flags & 0x8000u) row = height - 1 - row;
                base = ((pn >> 1) & 0x03ffu) & masks[(flags >> 12) & 3u];
                base |= (unsigned)(row & 0x30) >> 3;
                if (width == 32) base &= ~1u;
                for (int half = 0; half < width / 16 && active_count < 16;
                     ++half) {
                    unsigned pattern = base | (unsigned)half;
                    int piece_x = (int)(sxw & 0x03ffu) - 0x20 + half * 16;
                    if ((flags & 0x0800u) && width == 32) pattern ^= 1u;
                    active[active_count].flags = flags;
                    active[active_count].pattern = (uint16_t)pattern;
                    active[active_count].palette = (uint16_t)((flags & 0xfu) << 4);
                    active[active_count].x = piece_x;
                    active[active_count].row = row & 15;
                    ++active_count;
                }
            }
            for (int ai = active_count - 1; ai >= 0; --ai) {
                const struct SpritePiece *sp = &active[ai];
                size_t word_base = (size_t)sp->pattern * 64u +
                                   (size_t)sp->row;
                uint16_t planes[4];
                if (word_base + 48u >= THERON_VRAM_SIZE / 2u) continue;
                for (int plane = 0; plane < 4; ++plane) {
                    size_t byte = (word_base + (size_t)plane * 16u) * 2u;
                    planes[plane] = (uint16_t)vp->vram_trace_data[byte] |
                        ((uint16_t)vp->vram_trace_data[byte + 1] << 8);
                }
                for (int px = 0; px < 16; ++px) {
                    int bit = (sp->flags & 0x0800u) ? px : 15 - px;
                    unsigned pixel = 0;
                    int dx = sp->x + px;
                    uint16_t *dst;
                    if (dx < 0 || dx >= vp->vdc_display_width) continue;
                    for (int plane = 0; plane < 4; ++plane)
                        pixel |= ((planes[plane] >> bit) & 1u) << plane;
                    if (!pixel) continue;
                    dst = &vp->vdc_source_frame[
                        y * vp->vdc_display_width + dx];
                    if ((*dst & 0x0fu) == 0u || (sp->flags & 0x0080u))
                        *dst = (uint16_t)(0x100u | sp->palette | pixel);
                }
            }
        }
        /* M11 is an 8-bit indexed surface.  Compact only the source entries
         * actually present in this exact frame; this is an identity remap,
         * not quantisation or generated colour data. */
        vp->host_palette_source_count = 0;
        for (size_t p = 0; p < frame_pixels; ++p) {
            uint16_t source = vp->vdc_source_frame[p] & 0x01ffu;
            uint16_t host;
            for (host = 0; host < vp->host_palette_source_count; ++host)
                if (vp->host_palette_source_indices[host] == source) break;
            if (host == vp->host_palette_source_count) {
                if (host >= 256u) return -1;
                vp->host_palette_source_indices[host] = source;
                ++vp->host_palette_source_count;
            }
            vp->fb.data[p] = (uint8_t)host;
        }
        return rendered;
    }
    if (vp->fb.w < TQR_VIEWPORT_W || vp->fb.h < TQR_VIEWPORT_H)
        return -1;
    /* The capture is a native 256x224 VDC screen. BAT cells are laid out in
     * the source's 64-cell stride; the admitted screen window is the first
     * 32 columns by 28 rows. Keep this separate from the unresolved
     * T520/T600 square-to-tile consumer. */
    return theron_v1_vram_trace_render_bat_preview(
        vp, 0, TQR_VIEWPORT_W / TQR_TILE_DIM,
        TQR_VIEWPORT_H / TQR_TILE_DIM, 0, 0);
}

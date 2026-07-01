/*
 * csb_v1_save_real_artifact_boundary_pc34_compat.c
 *
 * CSB V1 save/load real-artifact boundary evidence gate.
 * See include/csb_v1_save_real_artifact_boundary_pc34_compat.h
 * for scope and source-lock citations.
 *
 * The implementation is deliberately small and follows the
 * existing csb_v1_csbwin_save_loader_boundary_pc34_compat /
 * csb_v1_csbgraphics_dat_real_scan pattern: pure evidence
 * gate that drives the live F0435-shaped save/load entry
 * points against bytes derived from a real binary DUNGEON.DAT.
 *
 * Source-lock boundary (see header for the full chain):
 *   - ReDMCSB LOADSAVE.C: F0435_STARTEND_LoadGame lines
 *     ~2665-2724 (header + GameID validation before GLOBAL_DATA).
 *   - ReDMCSB SAVEHEAD.C: F0429/F0430 (header obfuscation/checksum).
 *   - ReDMCSB SAVEUTIL.C: F0417 (XOR obfuscation + checksum).
 *   - ReDMCSB DEFS.H: C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX.
 *   - ReDMCSB DUNGEON.C: F0148/F0151/F0156/F0161 (real-data
 *     level/offset/square parsing used to derive the save
 *     GameID/seed/party_x).
 */

#include "csb_v1_save_real_artifact_boundary_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Status-name helper ─────────────────────────────────────────────── */

const char *csb_v1_save_real_artifact_status_name(int status_code)
{
    switch (status_code) {
    case CSB_V1_SAVE_REAL_OK:                     return "OK";
    case CSB_V1_SAVE_REAL_ERR_NULL:               return "null-arg";
    case CSB_V1_SAVE_REAL_ERR_DUNGEON_PARSE:      return "dungeon-parse";
    case CSB_V1_SAVE_REAL_ERR_HEADER_BUILD:       return "header-build";
    case CSB_V1_SAVE_REAL_ERR_SAVE_WRITE:         return "save-write";
    case CSB_V1_SAVE_REAL_ERR_VERIFY_REJECT:      return "verify-reject";
    case CSB_V1_SAVE_REAL_ERR_VERIFY_OK_MISMATCH: return "verify-ok-mismatch";
    case CSB_V1_SAVE_REAL_ERR_LOAD_HEADER:        return "load-header";
    case CSB_V1_SAVE_REAL_ERR_LOAD_PREFIX:        return "load-prefix";
    case CSB_V1_SAVE_REAL_ERR_PREFIX_MISMATCH:    return "prefix-mismatch";
    case CSB_V1_SAVE_REAL_ERR_PARTY_BUILD:        return "party-build";
    case CSB_V1_SAVE_REAL_ERR_GAMEID_MISMATCH:    return "gameid-mismatch";
    default:                                       return "unknown";
    }
}

const char *csb_v1_save_real_artifact_source_evidence(void)
{
    return
        "ReDMCSB LOADSAVE.C F0435_STARTEND_LoadGame lines ~2665-2724\n"
        "ReDMCSB SAVEHEAD.C F0429/F0430 header obfuscation/checksum\n"
        "ReDMCSB SAVEUTIL.C F0417_SAVEUTIL_GetChecksumAndObfuscate\n"
        "ReDMCSB DEFS.H C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX\n"
        "ReDMCSB DUNGEON.C F0148/F0151/F0156/F0161 real-data parse\n"
        "ReDMCSB DECOMPDU.C F0455 (FTL dungeon decompression)\n"
        "CSBWin SaveGame.cpp 512-byte XOR header contract";
}

/* ── Real-data GameID derivation ────────────────────────────────────── */

/* FTL decompression signature check (matches
 * csb_decode_dungeon_if_needed in csb_v1_dungeon_loader_pc34_compat.c).
 * Returns 1 if the buffer starts with the documented FTL
 * compressed-dungeon signature, 0 otherwise. */
static int is_ftl_compressed_signature(const uint8_t *dat, int dat_size)
{
    if (!dat || dat_size < 8) return 0;
    if (dat[0] == 0x81 && dat[1] == 0x04) return 1;
    if (dat[0] == 0x04 && dat[1] == 0x81) return 1;
    return 0;
}

uint16_t csb_v1_save_real_artifact_derive_game_id(
    const uint8_t *dat, int dat_size)
{
    uint32_t sum;
    int i;

    if (!dat || dat_size < 8) return 0;

    /* Deterministic mix: FNV-1a 32-bit over the first 32 bytes
     * of the dungeon payload after the FTL 8-byte header (if
     * compressed), or the first 32 raw bytes (if already
     * decompressed). This survives both layouts without
     * depending on the FTL decompressor, so a probe can call
     * this BEFORE csb_v1_dungeon_load() and use the GameID for
     * the save header it builds from the same bytes. */
    if (is_ftl_compressed_signature(dat, dat_size)) {
        if (dat_size < 8 + 32) return 0;
        sum = 0x01000193u;
        for (i = 0; i < 32; ++i) {
            sum = (sum * 0x01000193u) ^ (uint32_t)dat[8 + i];
        }
    } else {
        sum = 0x01000193u;
        for (i = 0; i < 32 && i < dat_size; ++i) {
            sum = (sum * 0x01000193u) ^ (uint32_t)dat[i];
        }
    }
    /* Fold to 16 bits and reserve sentinel values. */
    {
        uint16_t gid = (uint16_t)((sum ^ (sum >> 16)) & 0xFFFFu);
        if (gid == 0u) gid = 0x0001u;
        if (gid == 0xFFFFu) gid = 0xFFFEu;
        return gid;
    }
}

/* ── Real-data party-state builder ──────────────────────────────────── */

/* Build a deterministic CSB_V1_SaveHeader + bounded state
 * prefix from the real binary DUNGEON.DAT bytes.
 *
 * - hdr_out: populated with the F0435-shaped header.
 * - state_out / state_cap: caller-owned scratch; the gate
 *   writes prefix_size bytes here.
 * - state_size_out: number of bytes written.
 *
 * Returns 0 on success, non-zero on error. */
static int build_real_party_state(
    const CSB_V1_DungeonData *dungeon,
    const uint8_t *dat,
    int dat_size,
    uint16_t expected_game_id,
    int prefix_size,
    CSB_V1_SaveHeader *hdr_out,
    uint8_t *state_out,
    int state_cap,
    int *state_size_out)
{
    /* Party pose derived from real dungeon metadata so the
     * round-tripped prefix is reproducible from the same
     * DUNGEON.DAT. We deliberately use small bounded values:
     * party_x = level_offsets[0] mod level_widths[0]
     * party_y = 0
     * party_z = 0 (CSB uses 0..11 floor index for level)
     * party_dir = 0
     * champion_count = 1 (minimum to satisfy the v2.x
     * importer range check)
     * dungeon_seed = FNV-1a of the first 16 bytes of the
     * post-FTL payload (or raw bytes). */
    int party_x, party_y, party_z, party_dir, champ_count;
    uint32_t dungeon_seed;
    int i;

    if (!dungeon || !dat || !hdr_out || !state_out || !state_size_out) return -1;
    if (dungeon->level_count < 1) return -2;
    if (dungeon->level_widths[0] < 1) return -2;
    if (prefix_size <= 0) return -3;
    if (state_cap < prefix_size) return -4;

    party_x = dungeon->level_offsets[0] % dungeon->level_widths[0];
    party_y = 0;
    party_z = 0;
    party_dir = 0;
    champ_count = 1;

    /* Deterministic dungeon_seed from the bytes. We
     * intentionally derive a DIFFERENT seed than the GameID
     * derivation above so the F0435 header has two distinct
     * real-data fields for the verdict to compare. */
    {
        const uint8_t *src;
        int src_count;
        if (is_ftl_compressed_signature(dat, dat_size)) {
            src = dat + 8;
            src_count = dat_size - 8;
        } else {
            src = dat;
            src_count = dat_size;
        }
        if (src_count < 1) return -2;
        dungeon_seed = 0x01000193u;
        for (i = 0; i < 16 && i < src_count; ++i) {
            dungeon_seed = (dungeon_seed * 0x01000193u) ^ (uint32_t)src[i];
        }
        if (dungeon_seed == 0u) dungeon_seed = 0xA5A5A5A5u;
    }

    if (csb_v1_save_header_build(
            hdr_out,
            CSB_V1_SAVE_MAGIC_CSB,
            expected_game_id,
            dungeon_seed,
            party_x, party_y, party_z,
            party_dir,
            champ_count,
            /* game_time = dungeon_seed low 24 bits, mixed with
             * level_count so two different real DUNGEON.DATs
             * never collide on every header field. */
            (uint32_t)((dungeon_seed & 0x00FFFFFFu)
                       ^ ((uint32_t)dungeon->level_count * 0x100u)),
            /* play_time_ms = dungeon_seed high 24 bits + level
             * count, ensures non-zero play time on real data. */
            (uint32_t)(((dungeon_seed >> 8) & 0x00FFFFFFu)
                       + (uint32_t)dungeon->level_count * 1000u))
        != 0) {
        return -5;
    }

    /* Deterministic state prefix: FNV-1a stream of the dungeon
     * metadata mixed with the GameID. The same real DUNGEON.DAT
     * + the same GameID + the same prefix_size produce the same
     * bytes across runs; a different prefix_size or a
     * different DUNGEON.DAT produces different bytes. The probe
     * can therefore assert that the loaded prefix byte-for-byte
     * matches the saved prefix. */
    {
        uint32_t h = 0x01000193u ^ (uint32_t)expected_game_id
                              ^ (uint32_t)party_x
                              ^ ((uint32_t)party_y << 8)
                              ^ ((uint32_t)party_z << 16)
                              ^ dungeon_seed;
        for (i = 0; i < prefix_size; ++i) {
            /* Mix in a dungeon-derived byte every 4 bytes so
             * the prefix isn't pure LCG noise. */
            if ((i & 3) == 0 && dungeon->raw_data && dungeon->raw_size > 0) {
                int di = (i >> 2) % dungeon->raw_size;
                h = (h * 0x01000193u) ^ (uint32_t)dungeon->raw_data[di];
            }
            h = (h * 0x01000193u) ^ (uint32_t)(0x21u + (i * 17u));
            state_out[i] = (uint8_t)((h >> ((i & 3) * 8)) & 0xFFu);
        }
    }

    *state_size_out = prefix_size;
    return 0;
}

/* ── Boundary check entry point ─────────────────────────────────────── */

int csb_v1_save_real_artifact_boundary_check(
    const CSB_V1_SaveRealArtifactConfig *cfg,
    CSB_V1_SaveRealArtifactVerdict *out)
{
    CSB_V1_DungeonData dungeon;
    uint8_t *state_buf = NULL;
    CSB_V1_SaveHeader hdr;
    CSB_V1_SaveHeader loaded_hdr;
    uint8_t loaded_prefix[256];
    int state_size = 0;
    int rc;
    int parsed_level_count;
    uint16_t derived_game_id;
    uint16_t effective_game_id;

    /* Sentinel messages for the failure paths. Kept as static
     * strings so the verdict record can carry a non-NULL
     * status_message even when the gate short-circuits. */
    static const char k_msg_null[] = "null config or verdict";
    static const char k_msg_dungeon_parse[] = "csb_v1_dungeon_load failed on real bytes";
    static const char k_msg_party_build[] = "build_real_party_state failed";
    static const char k_msg_header_build[] = "csb_v1_save_header_build failed";
    static const char k_msg_save_write[] = "csb_v1_save_game failed";
    static const char k_msg_verify_reject[] = "verify_same did not return CSB_V1_LOAD_OK";
    static const char k_msg_verify_mismatch[] = "verify_foreign did not return DIFFERENT_GAME";
    static const char k_msg_load_header[] = "header-only load returned non-OK";
    static const char k_msg_load_prefix[] = "bounded prefix load returned non-OK";
    static const char k_msg_prefix_mismatch[] = "loaded prefix bytes do not match saved bytes";
    static const char k_msg_gameid_mismatch[] = "loaded header GameID does not match built header GameID";

    if (!cfg || !out) return CSB_V1_SAVE_REAL_ERR_NULL;
    if (!cfg->dat || cfg->dat_size <= 0) return CSB_V1_SAVE_REAL_ERR_NULL;

    memset(out, 0, sizeof(*out));
    out->status_code    = CSB_V1_SAVE_REAL_ERR_NULL;
    out->status_message = k_msg_null;

    /* Resolve effective GameID: caller override wins, else
     * derive from the real bytes. The derived value is
     * surfaced in out->derived_game_id for the probe / docs. */
    if (cfg->expected_game_id != 0u) {
        derived_game_id = cfg->expected_game_id;
    } else {
        derived_game_id = csb_v1_save_real_artifact_derive_game_id(
            cfg->dat, cfg->dat_size);
    }
    if (derived_game_id == 0u) {
        out->status_code    = CSB_V1_SAVE_REAL_ERR_PARTY_BUILD;
        out->status_message = "derive_game_id returned 0 (bytes too small?)";
        return out->status_code;
    }
    effective_game_id = derived_game_id;
    out->derived_game_id = effective_game_id;

    /* Step 1: parse the real DUNGEON.DAT. csb_v1_dungeon_load
     * handles both FTL-compressed and decompressed inputs. */
    memset(&dungeon, 0, sizeof(dungeon));
    rc = csb_v1_dungeon_load(&dungeon, cfg->dat, cfg->dat_size);
    if (rc != 0 || dungeon.level_count < 1) {
        csb_v1_dungeon_free(&dungeon);
        out->parsed_level_count = 0;
        out->status_code       = CSB_V1_SAVE_REAL_ERR_DUNGEON_PARSE;
        out->status_message    = k_msg_dungeon_parse;
        return out->status_code;
    }
    parsed_level_count = dungeon.level_count;
    out->parsed_level_count = parsed_level_count;

    /* Step 2: build the F0435-shaped header + bounded state
     * prefix from the real dungeon metadata + GameID. Skip
     * the save/load roundtrip if out_path is NULL (the probe
     * may want to verify just the header-build + GameID
     * derivation path on hosts where /tmp is unwritable). */
    if (cfg->prefix_size > 0 && cfg->prefix_size <= (int)sizeof(loaded_prefix)) {
        state_buf = (uint8_t *)malloc((size_t)cfg->prefix_size);
        if (!state_buf) {
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_PARTY_BUILD;
            out->status_message = "malloc state_buf failed";
            return out->status_code;
        }
        rc = build_real_party_state(
            &dungeon, cfg->dat, cfg->dat_size,
            effective_game_id, cfg->prefix_size,
            &hdr, state_buf, cfg->prefix_size, &state_size);
        if (rc != 0) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_PARTY_BUILD;
            out->status_message = k_msg_party_build;
            return out->status_code;
        }
        out->header_build_code = 0;
    } else {
        /* Header-only path: build the header without a state
         * prefix so the probe can still verify the
         * header-build + GameID derivation contract. We
         * re-derive the dungeon_seed directly here so the
         * header still carries real-data-derived fields. */
        const uint8_t *src;
        int src_count;
        uint32_t d_seed = 0x01000193u;
        int header_only_i;
        if (is_ftl_compressed_signature(cfg->dat, cfg->dat_size)) {
            src = cfg->dat + 8;
            src_count = cfg->dat_size - 8;
        } else {
            src = cfg->dat;
            src_count = cfg->dat_size;
        }
        if (src_count < 1) {
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_PARTY_BUILD;
            out->status_message = k_msg_party_build;
            return out->status_code;
        }
        for (header_only_i = 0;
             header_only_i < 16 && header_only_i < src_count;
             ++header_only_i) {
            d_seed = (d_seed * 0x01000193u) ^ (uint32_t)src[header_only_i];
        }
        if (d_seed == 0u) d_seed = 0xA5A5A5A5u;
        rc = csb_v1_save_header_build(
            &hdr,
            CSB_V1_SAVE_MAGIC_CSB,
            effective_game_id,
            d_seed,
            0, 0, 0, 0, 1,
            (uint32_t)((d_seed & 0x00FFFFFFu)
                       ^ ((uint32_t)parsed_level_count * 0x100u)),
            (uint32_t)(((d_seed >> 8) & 0x00FFFFFFu)
                       + (uint32_t)parsed_level_count * 1000u));
        if (rc != 0) {
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_HEADER_BUILD;
            out->status_message = k_msg_header_build;
            return out->status_code;
        }
        out->header_build_code = 0;
    }

    /* Step 3: write the save if out_path is set. */
    if (cfg->out_path && state_buf && state_size > 0) {
        rc = csb_v1_save_game(cfg->out_path, state_buf, state_size, &hdr);
        out->save_write_code = rc;
        if (rc != CSB_V1_SAVE_OK) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_SAVE_WRITE;
            out->status_message = k_msg_save_write;
            return out->status_code;
        }

        /* Step 4a: verify_same — same magic + same GameID must
         * pass the F0435 loader-boundary check. */
        rc = csb_v1_save_verify_compatible(cfg->out_path,
                                           CSB_V1_SAVE_MAGIC_CSB,
                                           effective_game_id);
        out->verify_same_code = rc;
        out->verify_same_match = (rc == CSB_V1_LOAD_OK) ? 1 : 0;
        if (rc != CSB_V1_LOAD_OK) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_VERIFY_REJECT;
            out->status_message = k_msg_verify_reject;
            return out->status_code;
        }

        /* Step 4b: verify_foreign — different GameID must
         * return CSB_V1_LOAD_ERR_DIFFERENT_GAME (the F0435
         * "different game" half of the contract). */
        rc = csb_v1_save_verify_compatible(cfg->out_path,
                                           CSB_V1_SAVE_MAGIC_CSB,
                                           (uint16_t)(effective_game_id ^ 0xFFFFu));
        out->verify_foreign_code = rc;
        out->verify_foreign_match =
            (rc == CSB_V1_LOAD_ERR_DIFFERENT_GAME) ? 1 : 0;
        if (rc != CSB_V1_LOAD_ERR_DIFFERENT_GAME) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_VERIFY_OK_MISMATCH;
            out->status_message = k_msg_verify_mismatch;
            return out->status_code;
        }

        /* Step 5a: header-only load — direct csb_v1_load_game
         * with state=NULL, max_size=0. */
        memset(&loaded_hdr, 0, sizeof(loaded_hdr));
        rc = csb_v1_load_game(cfg->out_path, NULL, 0, &loaded_hdr);
        out->load_header_code = rc;
        if (rc != CSB_V1_LOAD_OK) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_LOAD_HEADER;
            out->status_message = k_msg_load_header;
            return out->status_code;
        }

        /* Step 5b: bounded-prefix load — round-trip the
         * prefix_size bytes back out. */
        memset(loaded_prefix, 0xA5u, sizeof(loaded_prefix));
        rc = csb_v1_load_game(cfg->out_path, loaded_prefix,
                              cfg->prefix_size, NULL);
        out->load_prefix_code = rc;
        if (rc != CSB_V1_LOAD_OK) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_LOAD_PREFIX;
            out->status_message = k_msg_load_prefix;
            return out->status_code;
        }

        /* Step 6: compare the loaded prefix bytes to the saved
         * prefix bytes. csb_v1_load_game's bounded-prefix path
         * (F0435 lines ~2724+) returns the requested max_size
         * bytes verbatim, so this is the byte-for-byte
         * proof that the F0435 prefix handoff survives the
         * disk round-trip on real bytes. */
        if (memcmp(loaded_prefix, state_buf, (size_t)cfg->prefix_size) != 0) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            out->status_code    = CSB_V1_SAVE_REAL_ERR_PREFIX_MISMATCH;
            out->status_message = k_msg_prefix_mismatch;
            return out->status_code;
        }

        /* Surface the loaded header fields for the probe. */
        out->loaded_magic         = loaded_hdr.Magic;
        out->loaded_game_id       = loaded_hdr.GameID;
        out->loaded_dungeon_seed  = loaded_hdr.DungeonSeed;
        out->loaded_party_x       = loaded_hdr.PartyMapX;
        out->loaded_party_y       = loaded_hdr.PartyMapY;
        out->loaded_party_z       = loaded_hdr.PartyMapZ;
        out->loaded_party_dir     = loaded_hdr.PartyDirection;
        out->loaded_champion_count = loaded_hdr.ChampionCount;

        /* Header-match flag: every field must equal its
         * build-time counterpart. This is the F0435-shaped
         * field-equality contract. */
        out->header_match =
            (loaded_hdr.Magic          == hdr.Magic)          &&
            (loaded_hdr.GameID         == hdr.GameID)         &&
            (loaded_hdr.DungeonSeed    == hdr.DungeonSeed)    &&
            (loaded_hdr.PartyMapX      == hdr.PartyMapX)      &&
            (loaded_hdr.PartyMapY      == hdr.PartyMapY)      &&
            (loaded_hdr.PartyMapZ      == hdr.PartyMapZ)      &&
            (loaded_hdr.PartyDirection == hdr.PartyDirection) &&
            (loaded_hdr.ChampionCount  == hdr.ChampionCount);
        if (!out->header_match) {
            free(state_buf);
            csb_v1_dungeon_free(&dungeon);
            /* The F0435 GameID check already passed (the
             * verify_same_code is CSB_V1_LOAD_OK above), so a
             * header_match=0 here means the loaded fields
             * differ from the built fields in a way the
             * loader did not surface as an error. We treat
             * that as a game-id mismatch (the most likely
             * field to drift silently is GameID, which the
             * F0435 contract requires to be byte-equal). */
            out->status_code    = CSB_V1_SAVE_REAL_ERR_GAMEID_MISMATCH;
            out->status_message = k_msg_gameid_mismatch;
            return out->status_code;
        }

        out->prefix_match = 1;
    } else {
        /* No out_path / no prefix: header-build only. Mark the
         * downstream flags as not-applicable (0). */
        out->save_write_code     = 0; /* skipped, not failed */
        out->verify_same_code    = 0; /* skipped */
        out->verify_foreign_code = 0; /* skipped */
        out->load_header_code    = 0; /* skipped */
        out->load_prefix_code    = 0; /* skipped */
        out->header_match        = 0; /* no round-trip ran */
        out->prefix_match        = 0; /* no round-trip ran */
    }

    free(state_buf);
    csb_v1_dungeon_free(&dungeon);
    out->status_code    = CSB_V1_SAVE_REAL_OK;
    out->status_message = "OK";
    return out->status_code;
}

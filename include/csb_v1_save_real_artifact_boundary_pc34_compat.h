/*
 * csb_v1_save_real_artifact_boundary_pc34_compat.h
 *
 * CSB V1 save/load real-artifact boundary evidence gate.
 *
 * Closes the open slice in the 2026-06-22 CSB V1 save runtime
 * boundary follow-up:
 *   "Real save compatibility artifacts (binary DUNGEON.DAT +
 *    party-state save/load handoff through the live F0435
 *    STARTEND_LoadGame-shaped flow)"
 *
 * Where the data-free csb_v1_save_runtime_boundary_pc34_compat
 * gate proves the F0435-shaped header + bounded-prefix
 * roundtrip on synthetic bytes, this module proves the same
 * roundtrip is **boundary-clean** when the bytes come from a
 * real binary DUNGEON.DAT + a CSB_V1_PartyState buffer
 * produced by the live csb_v1_dungeon_load() pipeline.
 *
 * The module is intentionally narrow: it is an evidence gate,
 * not an importer, decoder, or runtime binding. It:
 *
 *   1. Accepts an in-memory binary DUNGEON.DAT buffer.
 *   2. Calls csb_v1_dungeon_load() to populate the level /
 *      width / height / offsets metadata.
 *   3. Builds a deterministic CSB_V1_SaveHeader + bounded
 *      state prefix from the dungeon metadata + a real-data
 *      GameID derived from the bytes (so the header is
 *      byte-faithful to the F0435 + SAVEHEAD.C F0429/F0430
 *      contract).
 *   4. Calls csb_v1_save_game() to write a save file.
 *   5. Calls csb_v1_save_verify_compatible() with the SAME
 *      magic/GameID to prove the loader-boundary contract.
 *   6. Calls csb_v1_load_game() to round-trip the prefix
 *      back out, then asserts the prefix bytes match.
 *   7. Calls csb_v1_save_verify_compatible() with a
 *      deliberately foreign GameID to prove the F0435
 *      "different game" rejection half of the contract.
 *
 * The boundary check is fail-closed: any non-OK return code or
 * prefix mismatch stamps a specific failure code + message.
 * The probe also runs the synthetic-fixture path so the
 * verdict contract is proven even on hosts without
 * user-staged CSB DUNGEON.DAT.
 *
 * Source references:
 *   - ReDMCSB LOADSAVE.C: F0435_STARTEND_LoadGame lines
 *     ~2665-2724 (header + GameID validation before GLOBAL_DATA).
 *   - ReDMCSB SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful +
 *     F0430_IsWriteObfuscatedSaveHeaderSuccessful.
 *   - ReDMCSB SAVEUTIL.C: F0417_SAVEUTIL_GetChecksumAndObfuscate
 *     (XOR obfuscation + checksum contract).
 *   - ReDMCSB DEFS.H: C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX.
 *   - CSBWin/SaveGame.cpp: save file I/O + DM1→CSB import path
 *     + 512-byte XOR header.
 *
 * Non-claims:
 *   - No M11/M12 runtime binding. The gate produces a save
 *     file on disk + a verdict record; a launcher / M11 can
 *     read the verdict and decide what to do.
 *   - No full F0435 STARTEND_LoadGame runtime replay (the
 *     full runtime requires a live world model + tick + draw
 *     cycle, which is the next slice of the gap).
 *   - No real CSB save file vendoring. The module reads
 *     real bytes from the operator-supplied DUNGEON.DAT and
 *     synthesizes the save from those bytes; it does not
 *     vendor or transcribe original CSB save bytes.
 *
 * Disjoint from:
 *   - csb_v1_save_runtime_boundary_pc34_compat (data-free
 *     synthetic fixture gate; this is the real-bytes
 *     counterpart).
 *   - csb_v1_csbwin_save_loader_boundary_pc34_compat
 *     (CSBWin/DM1 v2.x "CSBGAME\0" importer contract; this
 *     is the F0435 / F0433 512-byte obfuscated header
 *     contract).
 *   - csb_v1_import_csb_save_buffer (Champions GAP 3 v2.0/v2.1
 *     champion-record importer; this gate exercises the
 *     F0435-shaped prefix path, not the CSBGAME roster
 *     path).
 */
#ifndef FIRESTAFF_CSB_V1_SAVE_REAL_ARTIFACT_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_SAVE_REAL_ARTIFACT_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Status codes returned by the boundary check. Mirrors the
 * existing CSB_V1_LOAD_* / CSB_V1_SAVE_* codes plus a few
 * gate-specific ones for the real-artifact path. */
enum {
    CSB_V1_SAVE_REAL_OK                     = 0,
    CSB_V1_SAVE_REAL_ERR_NULL               = -1,
    CSB_V1_SAVE_REAL_ERR_DUNGEON_PARSE      = -2,
    CSB_V1_SAVE_REAL_ERR_HEADER_BUILD       = -3,
    CSB_V1_SAVE_REAL_ERR_SAVE_WRITE         = -4,
    CSB_V1_SAVE_REAL_ERR_VERIFY_REJECT      = -5,
    CSB_V1_SAVE_REAL_ERR_VERIFY_OK_MISMATCH = -6,
    CSB_V1_SAVE_REAL_ERR_LOAD_HEADER        = -7,
    CSB_V1_SAVE_REAL_ERR_LOAD_PREFIX        = -8,
    CSB_V1_SAVE_REAL_ERR_PREFIX_MISMATCH    = -9,
    CSB_V1_SAVE_REAL_ERR_PARTY_BUILD        = -10,
    CSB_V1_SAVE_REAL_ERR_GAMEID_MISMATCH    = -11
};

/* Configuration for the real-artifact boundary check.
 *
 * - dat / dat_size: the binary DUNGEON.DAT bytes (may be
 *   FTL-compressed; csb_v1_dungeon_load() handles that).
 * - out_path: where csb_v1_save_game() should write the
 *   produced save file. If NULL, no save is written and the
 *   gate stops after header build (the header_build + verify
 *   verdict + GameID-rejection verdict are still produced).
 * - prefix_size: how many bytes of the saved state prefix to
 *   read back through csb_v1_load_game(). Must be > 0 if
 *   out_path != NULL; the gate treats 0 as a guard that
 *   short-circuits the round-trip step.
 * - expected_game_id: the GameID the header is built with and
 *   the loader-boundary check uses. If 0, the gate derives
 *   a real-data GameID from the dungeon bytes (FNV-1a mix of
 *   the post-FTL payload's first 32 bytes). */
typedef struct {
    const uint8_t   *dat;
    int              dat_size;
    const char      *out_path;
    int              prefix_size;
    uint16_t         expected_game_id;
} CSB_V1_SaveRealArtifactConfig;

/* Verdict record produced by the boundary check.
 *
 * The probe compares the load-returned fields against the
 * build-supplied fields and stamps each comparison as 1
 * (match) or 0 (mismatch). The header_match / prefix_match
 * flags are the headline result the launcher / M11 can
 * quote. The detailed fields (magic, game_id, dungeon_seed,
 * party_x, party_y, party_z, party_dir, champion_count)
 * survive the F0435-shaped roundtrip if and only if the
 * matching flags are 1. */
typedef struct {
    /* Step-level codes */
    int header_build_code;     /* csb_v1_save_header_build rc (0=ok) */
    int save_write_code;       /* csb_v1_save_game rc */
    int verify_same_code;      /* csb_v1_save_verify_compatible rc
                                *   when called with the expected
                                *   magic/GameID */
    int verify_foreign_code;   /* ... with a deliberately wrong
                                *   GameID; expected negative
                                *   CSB_V1_LOAD_ERR_DIFFERENT_GAME */
    int load_header_code;      /* csb_v1_load_game(NULL,0) rc */
    int load_prefix_code;      /* csb_v1_load_game(buf,prefix_size) rc */

    /* Round-tripped header fields (post-load) */
    uint32_t loaded_magic;
    uint16_t loaded_game_id;
    uint32_t loaded_dungeon_seed;
    int16_t  loaded_party_x;
    int16_t  loaded_party_y;
    int16_t  loaded_party_z;
    uint16_t loaded_party_dir;
    uint16_t loaded_champion_count;

    /* Comparison flags */
    int header_match;          /* 1 iff loaded_header_code == OK and
                                *   every loaded_* field equals its
                                *   build-time counterpart. */
    int prefix_match;          /* 1 iff load_prefix_code == OK and
                                *   prefix_size bytes of the loaded
                                *   prefix match the saved prefix. */
    int verify_same_match;     /* 1 iff verify_same_code ==
                                *   CSB_V1_LOAD_OK */
    int verify_foreign_match;  /* 1 iff verify_foreign_code ==
                                *   CSB_V1_LOAD_ERR_DIFFERENT_GAME */

    /* Real-data GameID the gate derived from the dungeon
     * bytes (deterministic, survives both header build and
     * the loader-boundary contract check). Surfaced for the
     * probe / docs. */
    uint16_t derived_game_id;

    /* Number of dungeon levels the real DUNGEON.DAT parses
     * to (>=1 on a successful parse, 0 on parse error). */
    int      parsed_level_count;

    /* Final status code (CSB_V1_SAVE_REAL_*). */
    int status_code;

    /* Last-failure human-readable reason string, used by
     * the probe to print which step broke. May be NULL. */
    const char *status_message;
} CSB_V1_SaveRealArtifactVerdict;

/* Run the boundary check. Populates `out` (must be non-NULL)
 * with the verdict record. Returns the same value as
 * out->status_code. */
int csb_v1_save_real_artifact_boundary_check(
    const CSB_V1_SaveRealArtifactConfig *cfg,
    CSB_V1_SaveRealArtifactVerdict *out);

/* Derive a deterministic GameID from a binary DUNGEON.DAT
 * buffer (FTL-compressed or decompressed). Uses FNV-1a mix of
 * the first 32 bytes of the *decompressed* dungeon payload
 * (or the first 32 raw bytes if the buffer isn't FTL-compressed)
 * so the same real bytes always produce the same GameID.
 *
 * Returns 0 if dat is NULL or dat_size is too small. The
 * returned GameID is in the [0x0001, 0xFFFE] range so it
 * never collides with sentinel 0x0000 / 0xFFFF values used by
 * CSB header validation paths. */
uint16_t csb_v1_save_real_artifact_derive_game_id(
    const uint8_t *dat, int dat_size);

/* Public result-name helper so the probe / launcher can
 * print a stable label for each status code. */
const char *csb_v1_save_real_artifact_status_name(int status_code);

/* Source-evidence citation string. */
const char *csb_v1_save_real_artifact_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_SAVE_REAL_ARTIFACT_BOUNDARY_PC34_COMPAT_H */

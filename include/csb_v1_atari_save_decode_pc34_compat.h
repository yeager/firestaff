/*
 * CSB Atari ST saved-game decoder boundary.
 *
 * DMWeb's Saved Game Files specification describes MINI.DAT as CSB's main
 * dungeon stored in the native save format.  This module validates and
 * locates the source-owned dungeon payload; it deliberately does not invent
 * a Firestaff save container or silently substitute Prison's DUNGEON.DAT.
 */
#ifndef FIRESTAFF_CSB_V1_ATARI_SAVE_DECODE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_ATARI_SAVE_DECODE_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_ATARI_SAVE_OK = 0,
    CSB_V1_ATARI_SAVE_ERR_NULL = -1,
    CSB_V1_ATARI_SAVE_ERR_TRUNCATED = -2,
    CSB_V1_ATARI_SAVE_ERR_BLOCK2_CHECKSUM = -3,
    CSB_V1_ATARI_SAVE_ERR_BLOCK3_CHECKSUM = -4,
    CSB_V1_ATARI_SAVE_ERR_ITEM16_CHECKSUM = -5,
    CSB_V1_ATARI_SAVE_ERR_CHARACTER_CHECKSUM = -6,
    CSB_V1_ATARI_SAVE_ERR_TIMER_CHECKSUM = -7,
    CSB_V1_ATARI_SAVE_ERR_TIMER_QUEUE_CHECKSUM = -8,
    CSB_V1_ATARI_SAVE_ERR_COUNTS = -9
} CSB_V1_AtariSaveDecodeResult;

typedef struct {
    uint32_t game_time;
    uint32_t random_seed;
    int16_t leader_hand_thing;
    int16_t champion_count;
    int16_t party_x;
    int16_t party_y;
    int16_t party_direction;
    int16_t party_map_index;
    int16_t timer_count;
    int16_t item16_count;
    int16_t timer_capacity;
    int16_t item16_capacity;
    size_t dungeon_offset;
    size_t dungeon_size;
} CSB_V1_AtariSaveInfo;

/* The documented GAMEBLOCK2 fields that can be updated without inventing
 * ITEM16, character, timer or dungeon bytes. Every other source section is
 * preserved verbatim by the bounded writeback routine below. */
typedef struct {
    uint32_t game_time;
    uint32_t random_seed;
    int16_t leader_hand_thing;
    int16_t party_x;
    int16_t party_y;
    int16_t party_direction;
    int16_t party_map_index;
} CSB_V1_AtariSaveGameBlock2Patch;

/* Decode original big-endian Atari ST CSB save data, including MINI.DAT.
 * The output points to no owned memory: dungeon_offset/dungeon_size refer to
 * the caller's input buffer. */
int csb_v1_atari_save_decode_pc34_compat(const uint8_t *bytes,
                                         size_t size,
                                         CSB_V1_AtariSaveInfo *out_info);

/* Decode the native CSB GAMEBLOCK2/character section into the runtime party
 * shape. This consumes original bytes only; unowned timer/ITEM16 bodies stay
 * outside this champion boundary. */
int csb_v1_atari_save_decode_party_pc34_compat(
    const uint8_t *bytes, size_t size, CSB_V1_PartyState *out_party,
    CSB_V1_AtariSaveInfo *out_info);

/* Decode and hand the authenticated MINI.DAT dungeon bytes directly to the
 * existing memory-backed CSB dungeon loader.  The caller owns `out_dungeon`
 * and releases it with csb_v1_dungeon_free(). */
int csb_v1_atari_save_load_dungeon_pc34_compat(
    const uint8_t *bytes, size_t size, CSB_V1_DungeonData *out_dungeon,
    CSB_V1_AtariSaveInfo *out_info);

/* Re-encrypt an authenticated original CSB save after changing only the
 * documented GAMEBLOCK2 state above. The caller supplies an equally large
 * output buffer. The routine preserves the original dungeon and every
 * unowned encrypted section byte-for-byte, recomputes the source checksums,
 * and verifies the result before returning it. */
int csb_v1_atari_save_patch_gameblock2_pc34_compat(
    const uint8_t *bytes, size_t size,
    const CSB_V1_AtariSaveGameBlock2Patch *patch,
    uint8_t *out_bytes, size_t out_size,
    CSB_V1_AtariSaveInfo *out_info);

/* Extends the bounded GAMEBLOCK2 patch with fields already owned by
 * csb_v1_atari_save_decode_party_pc34_compat(). The source champion count
 * must match exactly. Unknown bytes in each 800-byte champion record, ITEM16,
 * timers, timer queue and the embedded dungeon remain source-preserved. */
int csb_v1_atari_save_patch_gameblock2_and_party_pc34_compat(
    const uint8_t *bytes, size_t size,
    const CSB_V1_AtariSaveGameBlock2Patch *patch,
    const CSB_V1_PartyState *party,
    uint8_t *out_bytes, size_t out_size,
    CSB_V1_AtariSaveInfo *out_info);

const char *csb_v1_atari_save_decode_source_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif

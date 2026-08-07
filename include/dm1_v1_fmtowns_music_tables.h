#ifndef DM1_V1_FMTOWNS_MUSIC_TABLES_H
#define DM1_V1_FMTOWNS_MUSIC_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Source-locked FM Towns DM1 music tables recovered from EDM.EXP.
 *
 * Symbols located via SYM1 table lookup (see
 * dm1_v1_fmtowns_edm_sym1). Byte values read directly from EDM.EXP
 * initialised data segment at the SYM1-recorded vaddrs.
 *
 * Verified 2026-08-07 against shipping HMA-240 disc.
 */

/* LEVEL_SONGS @ 0x3fbcc: 16 words mapping level index -> song index.
 * Last entry (0xffff) is the sentinel terminator.
 *
 * Byte-verified: 00 00 06 00 07 00 0c 00 09 00 04 00 03 00 0a 00
 *                0d 00 0e 00 0f 00 08 00 11 00 01 00 00 00 ff ff
 * As words:      0, 6, 7, 0xc, 9, 4, 3, 0xa, 0xd, 0xe, 0xf, 8,
 *                0x11, 1, 0, 0xffff */
#define DM1_V1_FMTOWNS_LEVEL_SONG_COUNT   15U
#define DM1_V1_FMTOWNS_LEVEL_SONG_END     0xffffU

extern const uint16_t
    dm1_v1_fmtowns_level_songs[DM1_V1_FMTOWNS_LEVEL_SONG_COUNT];

/* Title screen graphic asset indices, byte-verified from EDM.EXP:
 *
 *   TITLE_PRESENTS  @ 0x28f4a = 0x000c (asset 12)
 *   TITLE_DUNGEON   @ 0x28f4c = 0x000d (asset 13)
 *   TITLE_MASTER    @ 0x28f4e = 0x000e (asset 14) */
#define DM1_V1_FMTOWNS_TITLE_PRESENTS_ASSET  12U
#define DM1_V1_FMTOWNS_TITLE_DUNGEON_ASSET   13U
#define DM1_V1_FMTOWNS_TITLE_MASTER_ASSET    14U

/* Icon palette selector indices, byte-verified from EDM.EXP:
 *   ICON_PAL @ 0x28f44 = { 0x09, 0x0a, 0x0b } (3 words). */
#define DM1_V1_FMTOWNS_ICON_PAL_COUNT  3U
extern const uint16_t
    dm1_v1_fmtowns_icon_pal[DM1_V1_FMTOWNS_ICON_PAL_COUNT];

/* Music runtime defaults, byte-verified from EDM.EXP:
 *   DM_MUSIC @ 0x3fa80 = { 0x03, 0x04, 0x10, 0x0b } (4 bytes). */
#define DM1_V1_FMTOWNS_DM_MUSIC_COUNT  4U
extern const uint8_t
    dm1_v1_fmtowns_dm_music_defaults[DM1_V1_FMTOWNS_DM_MUSIC_COUNT];

/* MUSIC_ON initial state (byte-verified 0x0001 at 0x3fbf0). */
#define DM1_V1_FMTOWNS_MUSIC_ON_DEFAULT  1U

/* SPELL_COSTS @ 0x24388: 32-byte table indexed by spell*mana-level.
 * Byte-verified:
 *   01 02 03 04 05 06 02 03 04 05 06 07 04 05 06 07
 *   07 09 02 02 03 04 06 07 08 0c 10 14 18 1c 00 00
 *
 * The last two bytes are 0x00 padding. */
#define DM1_V1_FMTOWNS_SPELL_COSTS_COUNT  32U
extern const uint8_t
    dm1_v1_fmtowns_spell_costs[DM1_V1_FMTOWNS_SPELL_COSTS_COUNT];

/* Look up the CDDA-song index for `level` (0..14). Returns the
 * song index on success, LEVEL_SONG_END for out-of-range. */
uint16_t dm1_v1_fmtowns_level_song_for_level_pc34(unsigned int level);

/* PLAYER_COLOR @ 0x291b8: 8-word table. First 4 words are the
 * per-champion palette indices used by DRAW_ICN_BUTTON to tint
 * icon backgrounds when a champion is selected. Bytes at +8..
 * are adjacent unrelated data.
 * Byte-verified: 07 0b 08 0e 05 05 04 06 */
#define DM1_V1_FMTOWNS_PLAYER_COLOR_COUNT  8U
extern const uint8_t
    dm1_v1_fmtowns_player_color[DM1_V1_FMTOWNS_PLAYER_COLOR_COUNT];

/* SPELL_MULT @ 0x243a0: 8-byte per-spell-class multiplier that
 * modifies the base SPELL_COSTS look-up. Byte-verified head:
 * 08 0c 10 14 18 1c 00 00 (bytes 6-7 are 0 padding).
 * Beyond +7 the table interleaves with SPELL_MSE_LIST-adjacent
 * data; consumers must not read past index 7. */
#define DM1_V1_FMTOWNS_SPELL_MULT_COUNT  8U
extern const uint8_t
    dm1_v1_fmtowns_spell_mult[DM1_V1_FMTOWNS_SPELL_MULT_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_MUSIC_TABLES_H */

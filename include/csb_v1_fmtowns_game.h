#ifndef FIRESTAFF_CSB_V1_FMTOWNS_GAME_H
#define FIRESTAFF_CSB_V1_FMTOWNS_GAME_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_boot.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_fmtowns_portrait.h"
#include "csb_v1_fmtowns_switch.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT 10u
#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH 32u
#define CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT 32u
#define CSB_V1_FMTOWNS_GAME_MUSIC_TABLE_BYTES \
    (CSB_V1_FMTOWNS_GAME_MUSIC_MAP_COUNT * \
     CSB_V1_FMTOWNS_GAME_MUSIC_MAP_WIDTH * \
    CSB_V1_FMTOWNS_GAME_MUSIC_MAP_HEIGHT)

/* ReDMCSB MUSIC.C's F31E/F31J G2038 table maps C0_MUSIC_ENTRANCE to the
 * physical first audio track.  This is distinct from the per-square G4099
 * music map. */
#define CSB_V1_FMTOWNS_GAME_ENTRANCE_CDDA_TRACK 2u

#define CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT 6u
#define CSB_V1_FMTOWNS_UTILITY_MENU_POOL_CAPACITY 76u
#define CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT 16u
#define CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_RECORD_BYTES 68u
#define CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_BYTES 420u
#define CSB_V1_FMTOWNS_UTILITY_GAME_SOURCE_TITLE_BYTES 23u
#define CSB_V1_FMTOWNS_UTILITY_GAME_SOURCE_CHOICES_BYTES 40u
#define CSB_V1_FMTOWNS_UTILITY_GAME_SAVE_PROMPT_BYTES 42u
#define CSB_V1_FMTOWNS_UTILITY_DIALOG_OK_BYTES 3u
#define CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES 247u
#define CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_BYTES 324u
/* F0689 reaches the end of C06's 31x75 IMG2 command stream after 290 bytes.
 * The remaining verified carrier span precedes the following executable data
 * and must not be treated as image commands. */
#define CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_STREAM_BYTES 290u
#define CSB_V1_FMTOWNS_STARTUP_ACTIVE_GROUP_CAPACITY 60u
/* F31 retail user saves may retain the live F0196 allocation, which is
 * larger than the MINI.DAT bootstrap allocation.  ReDMCSB GROUP.C F0196
 * reserves 110 entries; do not let the MINI.DAT's 60-entry seed cap an
 * otherwise authenticated user campaign. */
#define CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY 110u
#define CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT 4u
#define CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES 464u
#define CSB_V1_FMTOWNS_UTILITY_PORTRAIT_CATALOG_CAPACITY 24u
#define CSB_V1_FMTOWNS_UTILITY_PORTRAIT_FILENAME_CAPACITY 13u
#define CSB_V1_FMTOWNS_UTILITY_SAVE_MAPPING_CAPACITY 32u

/* ReDMCSB DEFS.H command ordinals consumed by CEDT006.C's C06 loop. */
typedef enum CSB_V1_FmtownsUtilityMenuAction {
    CSB_V1_FMTOWNS_UTILITY_ACTION_NONE = 0,
    CSB_V1_FMTOWNS_UTILITY_ACTION_REVERT = 8,
    CSB_V1_FMTOWNS_UTILITY_ACTION_UNDO = 9,
    CSB_V1_FMTOWNS_UTILITY_ACTION_LOAD_CHAMPIONS = 11,
    CSB_V1_FMTOWNS_UTILITY_ACTION_SAVE_CHAMPIONS = 12,
    CSB_V1_FMTOWNS_UTILITY_ACTION_MAKE_NEW_ADVENTURE = 13,
    CSB_V1_FMTOWNS_UTILITY_ACTION_QUIT = 17
} CSB_V1_FmtownsUtilityMenuAction;

typedef struct CSB_V1_FmtownsUtilityMenuHitBox {
    CSB_V1_FmtownsUtilityMenuAction action;
    int16_t left;
    int16_t right;
    int16_t top;
    int16_t bottom;
} CSB_V1_FmtownsUtilityMenuHitBox;

/*
 * FM Towns Game-program admission.
 *
 * SWITCHTW.EXP does not enter a shared PC startup program.  AUTOEXEC.BAT
 * maps its Japanese/English Game exits to CHTWJ.EXP/CHTWE.EXP respectively.
 * ReDMCSB Toolchains/Common/Source/COMPILE.H:367-385 identifies both as
 * C03_GAME and binds them to the distinct F31J/F31E media sets; STARTUP1.C
 * line 163 then owns the F0435 load loop and ENTRANCE.C line 85 owns F0807.
 */
typedef struct CSB_V1_FmtownsGameHandoffReceipt {
    int valid;
    int executable_verified;
    int language_matches_profile;
    int game_program_is_c03_game;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t executable_size;
    uint32_t executable_fnv1a;
    /* ENTRANCE.C F0806 selects C28_ENTRANCE_CSB after SWITCHTW exits.
     * These are authenticated G8174 COLOR_DEF bytes from this exact
     * CHTWE/CHTWJ executable, not the preceding C26_SWITCH DAC state. */
    int entrance_palette_verified;
    uint32_t entrance_palette_source_offset;
    uint8_t entrance_palette_rgb6[16][3];
    /* PALETTE.C F0390 selects one of C00_LIGHT0..C05_LIGHT5 after Entrance.
     * F31 stores their G8151..G8156 COLOR_DEF rows contiguously. */
    int dungeon_palettes_verified;
    uint32_t dungeon_palettes_source_offset;
    uint8_t dungeon_palette_rgb6[6][16][3];
    char executable_name[16];
    char executable_path[512];
    /* Non-owning view for packed FM Towns media.  Loose development trees
     * continue to use executable_path; packed callers retain the original
     * member in this bounded view instead of extracting it. */
    const uint8_t *executable_bytes;
    size_t executable_bytes_size;
    char graphics_md5[33];
    char dungeon_md5[33];
    /* The CD's MINI.DAT is an authenticated F31 bootstrap resource.  It is
     * deliberately recorded separately from a user save: it must not enter
     * the Atari/Amiga GAMEBLOCK Resume decoder. */
    int startup_mini_verified;
    uint32_t startup_mini_size;
    uint32_t startup_mini_fnv1a;
    char startup_mini_path[512];
    const uint8_t *startup_mini_bytes;
    size_t startup_mini_bytes_size;
    /* F0435 reopens the selected CD DUNGEON.DAT; it is not stored in the
     * user-created CSBGAME.DAT. */
    const uint8_t *startup_dungeon_bytes;
    size_t startup_dungeon_bytes_size;
    char startup_dungeon_path[512];
    /* F0435 reads and authenticates the F31 512-byte header with the CSB
     * key at word 29.  This proves the selected seed is a native C5-format
     * FM Towns save header. The following receipt fields describe the
     * verified body; csb_v1_fmtowns_game_load_startup_state() decodes and
     * applies that seed atomically to the live F31 runtime. */
    int startup_mini_header_verified;
    uint16_t startup_mini_header_key;
    uint8_t startup_mini_header_format_id;
    uint16_t startup_mini_header_platform;
    uint16_t startup_mini_header_dungeon_id;
    /* CEDTINCD.C F7051 then reads these five native F31 save parts. Their
     * checksums admit the seed body; the receipt itself remains descriptive,
     * while the state loader transfers the body and F7063 dungeon tail as
     * one candidate without using the Atari/Amiga decoder. */
    int startup_mini_save_parts_verified;
    uint16_t startup_mini_party_champion_count;
    uint16_t startup_mini_event_count;
    uint16_t startup_mini_first_unused_event_index;
    uint16_t startup_mini_current_active_group_count;
    uint16_t startup_mini_event_maximum_count;
    uint16_t startup_mini_active_group_capacity;
    uint32_t startup_mini_game_time;
    int16_t startup_mini_party_map_x;
    int16_t startup_mini_party_map_y;
    int16_t startup_mini_party_direction;
    int16_t startup_mini_party_map_index;
    uint32_t startup_mini_verified_save_body_offset;
    /* F7063 consumes the raw dungeon tail after the four F31 portraits and
     * compares its source byte-sum footer. The receipt does not mutate a
     * runtime; the state loader materializes this exact tail and the apply
     * API commits it together with party, events and active groups. */
    int startup_mini_dungeon_tail_verified;
    uint8_t startup_mini_dungeon_map_count;
    uint16_t startup_mini_dungeon_column_count;
    uint16_t startup_mini_dungeon_tail_checksum;
    uint8_t startup_mini_first_map_offset_x;
    uint8_t startup_mini_first_map_offset_y;
    uint32_t startup_mini_dungeon_tail_offset;
    uint32_t startup_mini_dungeon_tail_size;
    int music_table_verified;
    uint32_t music_table_source_offset;
    uint32_t music_table_size;
    uint32_t music_table_fnv1a;
    const char *source_evidence;
} CSB_V1_FmtownsGameHandoffReceipt;

/* Exact F31 F0435 save-state material retained before any runtime install.
 * The event heap is expanded from the original ten-byte EVENT representation
 * (DEFS.H EVENT, with the G20-only padding absent); active-group bytes stay
 * raw until their C04 dungeon owners have been resolved. */
typedef struct CSB_V1_FmtownsStartupState {
    int valid;
    CSB_V1_PartyState party;
    uint32_t game_time;
    int16_t party_map_index;
    uint16_t active_group_capacity;
    uint16_t active_group_count;
    uint8_t active_groups[CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY][16];
    /* F0183/F0184 address groups by their C04 table index, not a THING
     * handle. Resolve every nonnegative saved index back to exactly one
     * original C04 square before a future runtime install may use it. */
    uint16_t active_group_resolved_count;
    struct {
        int valid;
        uint16_t group_thing_index;
        uint16_t group_thing;
        int map_index;
        int map_x;
        int map_y;
    } active_group_owners[CSB_V1_FMTOWNS_USER_SAVE_ACTIVE_GROUP_CAPACITY];
    struct DM1_EventQueue_V1 timeline_queue;
    CSB_V1_DungeonData dungeon;
} CSB_V1_FmtownsStartupState;

/* A user-created FM Towns CSBGAME.DAT is deliberately distinct from the
 * CD's MINI.DAT bootstrap image.  F0435 reads the five obfuscated parts and
 * four portrait rasters from the save disk, then F0434 continues on that
 * same save-file handle with the F7063 checksummed dungeon stream.  It is
 * therefore a self-contained original dungeon state, not a CDATA/CJDATA
 * DUNGEON.DAT substitute. */
typedef struct CSB_V1_FmtownsUserSaveReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_size;
    uint32_t source_fnv1a;
    char source_path[512];
    char dungeon_path[512];
    const uint8_t *dungeon_bytes;
    size_t dungeon_bytes_size;
    uint16_t header_key;
    uint16_t platform;
    uint16_t dungeon_id;
    uint16_t party_champion_count;
    uint16_t event_count;
    uint16_t first_unused_event_index;
    uint16_t current_active_group_count;
    uint16_t event_maximum_count;
    uint16_t active_group_capacity;
    uint32_t game_time;
    int16_t party_map_x;
    int16_t party_map_y;
    int16_t party_direction;
    int16_t party_map_index;
    uint32_t portraits_offset;
    uint32_t dungeon_tail_offset;
    uint32_t dungeon_tail_size;
    uint16_t dungeon_tail_checksum;
    int recovered_from_backup;
    const char *source_evidence;
} CSB_V1_FmtownsUserSaveReceipt;

/* Four raw external F31 portraits immediately follow F0435's five verified
 * MINI.DAT save parts.  CEDT019.C F2124 converts these original planar bytes
 * before CEDT006.C draws them; no party-model portrait is a substitute. */
typedef struct CSB_V1_FmtownsStartupPortraitReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_file_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    uint8_t source_bytes[CSB_V1_FMTOWNS_STARTUP_PORTRAIT_COUNT]
                        [CSB_V1_FMTOWNS_STARTUP_PORTRAIT_BYTES];
    const char *source_evidence;
} CSB_V1_FmtownsStartupPortraitReceipt;

/* AUTOEXEC.BAT exit 2/5 enters a different C06_CEDT program. This receipt
 * admits that program only; it does not pretend its editor UI is C03_GAME. */
typedef struct CSB_V1_FmtownsUtilityHandoffReceipt {
    int valid;
    int executable_verified;
    int language_matches_profile;
    int utility_program_is_c06_cedt;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t executable_size;
    uint32_t executable_fnv1a;
    char executable_name[16];
    char executable_path[512];
    /* Non-owning view for packed FM Towns media.  Loose development trees
     * continue to use executable_path; packed callers retain the original
     * member in this bounded view instead of extracting it. */
    const uint8_t *executable_bytes;
    size_t executable_bytes_size;
    /* Bounded Phar Lap level-1 P3 envelope from the verified C06 image.
     * This exposes the real native entry point without claiming to emulate
     * its TBIOS menu, editor pixels, or save transactions. */
    int p3_header_verified;
    uint32_t p3_header_size;
    uint32_t p3_load_image_offset;
    uint32_t p3_load_image_size;
    uint32_t p3_initial_eip;
    /* CEDT027.C's C09_ICON table is retained from this exact hash-verified
     * F31 executable: 16 indexed RGB6 entries followed by its 0xFF sentinel.
     * It is not a host palette approximation. */
    int icon_palette_verified;
    uint32_t icon_palette_file_offset;
    uint8_t icon_palette_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    /* CEDT018.C's F31 mirror and file-picker-arrow bitmaps.  These are read
     * from the selected C06 executable, never carried as host artwork. */
    int static_art_verified;
    uint32_t mirror_bitmap_file_offset;
    uint32_t mirror_bitmap_fnv1a;
    uint8_t mirror_bitmap[CSB_V1_FMTOWNS_UTILITY_MIRROR_BITMAP_BYTES];
    uint32_t file_picker_arrows_file_offset;
    uint32_t file_picker_arrows_fnv1a;
    uint8_t file_picker_arrows[CSB_V1_FMTOWNS_UTILITY_FILE_PICKER_ARROWS_BYTES];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityHandoffReceipt;

/* CEDT001.C F7000 expands the source-owned #CHAMP_NAME# file operation.
 * This receipt retains the exact mapping from the selected UTILE/UTILJ
 * image; it is not a host path and cannot be used without the authenticated
 * C06 handoff. */
typedef struct CSB_V1_FmtownsUtilitySaveMappingReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_file_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    char template_bytes[CSB_V1_FMTOWNS_UTILITY_SAVE_MAPPING_CAPACITY];
    const char *source_evidence;
} CSB_V1_FmtownsUtilitySaveMappingReceipt;

/* C06's first menu is retained as source bytes.  Japanese remains Shift-JIS
 * until the native Towns text path is decoded; callers must not replace it
 * with translated host strings. */
typedef struct CSB_V1_FmtownsUtilityMenuReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_virtual_offset;
    uint32_t source_file_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    uint16_t label_offsets[CSB_V1_FMTOWNS_UTILITY_MENU_ACTION_COUNT];
    uint8_t source_bytes[CSB_V1_FMTOWNS_UTILITY_MENU_POOL_CAPACITY];
    int icon_palette_verified;
    uint32_t icon_palette_file_offset;
    uint8_t icon_palette_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityMenuReceipt;

/* C06's pre-editor source chooser is a separate UTILE string group.  Keep
 * it apart from the six bottom-row action labels: the strings carry both the
 * initial C0_GAME_SOURCE choice and the later A: medium prompt. */
typedef struct CSB_V1_FmtownsUtilityGameSourceReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t title_file_offset;
    uint32_t choices_file_offset;
    uint32_t save_prompt_file_offset;
    uint32_t ok_file_offset;
    uint32_t source_fnv1a;
    uint8_t title[CSB_V1_FMTOWNS_UTILITY_GAME_SOURCE_TITLE_BYTES];
    uint8_t choices[CSB_V1_FMTOWNS_UTILITY_GAME_SOURCE_CHOICES_BYTES];
    uint8_t save_prompt[CSB_V1_FMTOWNS_UTILITY_GAME_SAVE_PROMPT_BYTES];
    uint8_t ok[CSB_V1_FMTOWNS_UTILITY_DIALOG_OK_BYTES];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityGameSourceReceipt;

/* The C06 interface/scroll font is retained only after it has been read from
 * the exact hash-verified UTILE/UTILJ program selected by SWITCHTW.  The
 * bytes are not a host-font substitute: ReDMCSB CEDT019.C:18 and
 * CEDTFNT.C:44 identify the 420-byte source object and its native consumer.
 * The raw offsets differ between F31E and F31J. */
typedef struct CSB_V1_FmtownsUtilityFontReceipt {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    CSB_V1_VariantId variant_id;
    uint32_t source_file_offset;
    uint32_t source_size;
    uint32_t source_fnv1a;
    uint8_t source_bytes[CSB_V1_FMTOWNS_UTILITY_INTERFACE_FONT_BYTES];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityFontReceipt;

/* The F31 CD ships actual selectable C06 .CMP portraits in PORTRAIT/. The
 * catalogue records only files that pass PORTRAIT.C's native header/pixel
 * admission; an absent or untrusted directory produces no invented rows. */
typedef struct CSB_V1_FmtownsUtilityPortraitCatalogEntry {
    char filename[CSB_V1_FMTOWNS_UTILITY_PORTRAIT_FILENAME_CAPACITY];
    char source_path[512];
    const uint8_t *source_bytes;
    size_t source_bytes_size;
    uint32_t source_fnv1a;
    CSB_V1_FmtownsPortraitReceipt portrait;
} CSB_V1_FmtownsUtilityPortraitCatalogEntry;

typedef struct CSB_V1_FmtownsUtilityPortraitCatalog {
    int valid;
    CSB_V1_FmtownsSwitchLanguage language;
    uint16_t entry_count;
    uint16_t rejected_entry_count;
    char source_directory[512];
    CSB_V1_FmtownsUtilityPortraitCatalogEntry
        entries[CSB_V1_FMTOWNS_UTILITY_PORTRAIT_CATALOG_CAPACITY];
    const char *source_evidence;
} CSB_V1_FmtownsUtilityPortraitCatalog;

/* CEDT008.C F7083/F7084 selector state.  The selector is deliberately
 * catalog-bound: it can move only across already admitted PORTRAIT records
 * and cannot turn a filename, row number, or generated entry into a load
 * candidate.  The catalog remains owned by the caller. */
typedef struct CSB_V1_FmtownsUtilityPortraitSelector {
    int valid;
    uint16_t selected_index;
    uint16_t entry_count;
    uint32_t catalog_fnv1a;
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog;
    const char *source_evidence;
} CSB_V1_FmtownsUtilityPortraitSelector;

/* Admit precisely the F31E/F31J executable selected by SWITCHTW.  A valid
 * CSB profile alone is deliberately insufficient: this gate also checks the
 * exact retail program identity before the entrance/HUD session is opened. */
int csb_v1_fmtowns_game_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt);

/* Admit an external, user-selected F31 C5 save after the matching retail
 * C03 program has already passed csb_v1_fmtowns_game_handoff_open().  Unlike
 * the bundled MINI.DAT seed, a user save has no fixed retail hash: F7061,
 * all five F7057 parts and F7063 must therefore validate it as one native
 * F0435 candidate.  This is load-only; it does not claim F0433 write-back. */
int csb_v1_fmtowns_game_user_save_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    const char *save_path,
    CSB_V1_FmtownsGameHandoffReceipt *out_receipt);

/* Copy only the F7063-verified dungeon bytes, excluding the trailing F7059
 * checksum word. This does not transfer ownership to a runtime or imply that
 * F31 save restoration is complete. */
int csb_v1_fmtowns_game_copy_verified_dungeon_tail(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint8_t *out_bytes, size_t out_size);

/* Serialize the live F31 source-layout dungeon plus its native two-byte
 * little-endian checksum trailer. `out_size` must equal raw_size + 2; the
 * source loader re-admits the raw dungeon bytes before the trailer is added.
 * No host or synthetic dungeon format is accepted. */
int csb_v1_fmtowns_game_encode_dungeon_tail(
    const CSB_V1_DungeonData *dungeon, uint8_t *out_bytes, size_t out_size);

/* Decode the authenticated F31 MINI.DAT champion/party save part into the
 * live CSB representation.  This is intentionally limited to the original
 * 4*319-byte CHAMPION array and the source-owned global party pose; callers
 * must not treat it as a general CSBWin or Atari save decoder.  ReDMCSB
 * LOADSAVE.C F0435 copies this exact part into M516_CHAMPIONS/G0407 after
 * F7057 has validated its checksum and deobfuscated it. */
int csb_v1_fmtowns_game_load_startup_party(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_PartyState *out_party);

/* Preserve the F31 source portraits as their exact on-disk planar payload.
 * Presentation code must apply the recovered native conversion path itself;
 * this admission API never invents decoded pixels. */
int csb_v1_fmtowns_game_load_startup_portraits(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_FmtownsStartupPortraitReceipt *out_receipt);

/* Materialize the authenticated F7063-checked dungeon tail as an original
 * CSB dungeon.  The caller owns the returned allocation through
 * csb_v1_dungeon_free(); no base-package or fixture dungeon is substituted. */
int csb_v1_fmtowns_game_load_startup_dungeon(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_DungeonData *out_dungeon);

/* Decode all five F0435 save parts plus the F7063 dungeon tail as one
 * receipt-bound candidate. It does not modify a runtime profile; callers
 * must resolve every raw ACTIVE_GROUP owner before committing it. */
int csb_v1_fmtowns_game_load_startup_state(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    CSB_V1_FmtownsStartupState *out_state);
void csb_v1_fmtowns_game_startup_state_free(
    CSB_V1_FmtownsStartupState *state);

/* Commit a fully resolved F31 candidate in one ownership transfer. The call
 * fails without modifying runtime unless dungeon, party, timeline and every
 * saved ACTIVE_GROUP owner are all present. */
int csb_v1_fmtowns_game_apply_startup_state(
    CSB_V1_FmtownsStartupState *state, CSB_V1_RuntimeProfile *runtime);

/* F0435 user-save admission and state transfer.  The file has no expected
 * retail hash: each real save differs.  All five native checksums, format,
 * platform, dungeon identity, exact part extent and portrait extent must
 * validate before it can affect a runtime. */
int csb_v1_fmtowns_game_user_save_open(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path,
    CSB_V1_FmtownsUserSaveReceipt *out_receipt);

/* F0435 recovery for the original F31 save-disk slot only.  When the
 * selected CSBGAME.DAT cannot pass the complete native reader, a validated
 * sibling CSBGAME.BAK is renamed back to the canonical slot before it is
 * admitted.  Arbitrary user filenames never receive an invented .BAK rule. */
int csb_v1_fmtowns_game_user_save_open_or_restore_backup(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path,
    CSB_V1_FmtownsUserSaveReceipt *out_receipt);

int csb_v1_fmtowns_game_load_user_save_state(
    const CSB_V1_FmtownsUserSaveReceipt *receipt,
    CSB_V1_FmtownsStartupState *out_state);

/* F7051/F2124 keeps the four native planar portraits immediately after the
 * five authenticated save parts. Recheck the complete selected slot before
 * exposing those bytes to C06; no decoded or host portrait substitute is
 * accepted. */
int csb_v1_fmtowns_game_load_user_save_portraits(
    const CSB_V1_FmtownsUserSaveReceipt *receipt,
    CSB_V1_FmtownsStartupPortraitReceipt *out_receipt);

/* F0433/F0435 native F31 write path. The writer starts from the authenticated
 * existing slot, patches only fields owned by the live F31 runtime, rebuilds
 * all five keyed parts and the F7062 header, and replaces the slot atomically.
 * If the authenticated save owns a save-specific dungeon-tail shape distinct
 * from the full CD dungeon, that tail is retained byte-for-byte; the live tail
 * is rebuilt only when its native source layout matches the admitted slot. */
int csb_v1_fmtowns_game_write_user_save(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path);

/* CEDTINC8.C F7052's first-save path.  Materialize a new canonical F31
 * CSBGAME.DAT from the already verified selected MINI.DAT, then run the same
 * F0433/F7052 writer used for an admitted slot.  The destination must not
 * exist; a sibling staging file is removed on every failure. */
int csb_v1_fmtowns_game_create_user_save_from_startup(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const char *save_path);

/* C06_CEDT F7001 -> F7052 starts from the same authenticated MINI.DAT as
 * C03, but replaces its editor-owned champion records and the four original
 * planar portrait blocks before the first M746/CSBGAME.DAT is published.
 * The destination must be absent, just as in F7052's create path. */
int csb_v1_fmtowns_game_create_utility_user_save_from_startup(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path);

/* Subsequent C06 F7052 saves reopen the already admitted M746 slot, patch
 * only the utility-owned party and portrait fields, and retain the native
 * F0433/F7062 write/backup transaction. */
int csb_v1_fmtowns_game_write_utility_user_save(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FmtownsGameHandoffReceipt *game_receipt,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    const char *save_path);

int csb_v1_fmtowns_utility_handoff_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityHandoffReceipt *out_receipt);

/* Bind F7000's real portrait-save filename template from the selected C06
 * executable.  No destination is invented when this receipt is absent. */
int csb_v1_fmtowns_utility_save_mapping_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilitySaveMappingReceipt *out_receipt);

int csb_v1_fmtowns_utility_menu_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityMenuReceipt *out_receipt);

int csb_v1_fmtowns_utility_game_source_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityGameSourceReceipt *out_receipt);

/* Read the native C06 interface font from the verified F31 executable.
 * The call deliberately fails on an unrecognized executable or source span;
 * callers must not replace it with a generated or PC34 font. */
int csb_v1_fmtowns_utility_font_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityFontReceipt *out_receipt);

/* Enumerate genuine F31 C06 portrait records under the selected CD root.
 * This is a read-only FILE_PICKER catalogue; it cannot create a .CMP file
 * and does not treat a filename as provenance. */
int csb_v1_fmtowns_utility_portrait_catalog_open(
    const CSB_V1_BootProfile *profile,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityPortraitCatalog *out_catalog);

/* CEDT008.C's NEW DISK command reopens the same F7002 picker against F7000's
 * dynamic portrait medium.  Firestaff maps that source medium only after the
 * verified #CHAMP_NAME# receipt is present; its .CMP records receive the
 * normal native admission checks and never become CD-media provenance. */
int csb_v1_fmtowns_utility_portrait_medium_catalog_open(
    const CSB_V1_FmtownsUtilitySaveMappingReceipt *mapping,
    CSB_V1_FmtownsSwitchLanguage language,
    CSB_V1_FmtownsUtilityPortraitCatalog *out_catalog);

/* Bind the native selector to one admitted catalogue entry.  The selected
 * index is a source-catalogue index, not a host path or a synthetic row. */
int csb_v1_fmtowns_utility_portrait_selector_open(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t initial_index,
    CSB_V1_FmtownsUtilityPortraitSelector *out_selector);

/* F7084 arrow movement.  Movement is bounded by the admitted catalogue;
 * there is no wraparound and no fallback entry at either end. */
int csb_v1_fmtowns_utility_portrait_selector_move(
    CSB_V1_FmtownsUtilityPortraitSelector *selector,
    int direction);

/* Revalidate the selected catalogue entry through the existing F7002
 * transaction. */
int csb_v1_fmtowns_utility_portrait_selector_load(
    const CSB_V1_FmtownsUtilityPortraitSelector *selector,
    CSB_V1_PartyState *party, uint16_t selected_champion,
    CSB_V1_FmtownsStartupPortraitReceipt *portraits);

/* Legacy internal serializer for existing admitted CMP records.  F7001 does
 * not dispatch this batch operation: its reachable PORTRAIT choice is
 * csb_v1_fmtowns_utility_save_selected_portrait() below. */
int csb_v1_fmtowns_utility_save_portraits(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits);

/* F7000_SavePortrait / CEDTDATA.C M747_FILE_ID_SAVE_CMP writes precisely
 * one selected champion to the dynamic portrait medium, named
 * `2:\\#CHAMP_NAME#.CMP` by the original file table.  Firestaff maps that
 * medium to ~/.firestaff/portraits on macOS/Linux and INSTALLDIR\\portraits
 * on Windows.  The destination is created on demand; source CD/catalogue
 * files are never overwritten. */
int csb_v1_fmtowns_utility_save_selected_portrait(
    const CSB_V1_FmtownsUtilitySaveMappingReceipt *mapping,
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    const CSB_V1_PartyState *party,
    const CSB_V1_FmtownsStartupPortraitReceipt *portraits,
    uint16_t selected_champion, char *out_path, size_t out_path_size);

/* F7002_ReadCMP / CEDT001.C: import one already-selected, authenticated
 * .CMP record into the currently selected party slot.  The selector itself
 * remains owned by C06; this transaction accepts only an index returned by
 * the source-owned catalogue and never opens a host path or invents a row. */
int csb_v1_fmtowns_utility_load_portrait(
    const CSB_V1_FmtownsUtilityPortraitCatalog *catalog,
    uint16_t catalog_index, CSB_V1_PartyState *party,
    uint16_t selected_champion,
    CSB_V1_FmtownsStartupPortraitReceipt *portraits);

/* Decode the original C06 mouse target in its 320x200 source coordinate
 * space. ReDMCSB CEDTDATA.C lines 128-165 defines these F31E/F31J boxes,
 * and CEDT006.C lines 1401-1529 dispatches the resulting command ordinal.
 * Edges are inclusive, matching MOUSE2_INPUT. */
int csb_v1_fmtowns_utility_menu_action_at(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    int16_t source_x, int16_t source_y,
    CSB_V1_FmtownsUtilityMenuHitBox *out_hit_box);

/* Copy the receipt-bound F31 C06 editor C09_ICON palette in native six-bit
 * RGB. CEDT018.C first blacks the curtain, applies C09_ICON, then restores
 * it; CEDT027.C defines the table. A receipt is required so the live route
 * never falls back to a hand-copied host palette. */
int csb_v1_fmtowns_utility_icon_palette_rgb6(
    const CSB_V1_FmtownsUtilityMenuReceipt *receipt,
    uint8_t out_rgb6[CSB_V1_FMTOWNS_UTILITY_ICON_PALETTE_COLOR_COUNT][3]);

/* Return the native F31 music selector (zero means no selector) for one
 * source map square. The value is the exact byte passed to F0719_PlayMusicTrack
 * by ReDMCSB MUSIC.C F0743; it is not a synthesized physical CD track number. */
int csb_v1_fmtowns_game_music_track_at(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint32_t map_index,
    uint32_t map_x,
    uint32_t map_y,
    uint8_t *out_track);

/* Resolve ENTRANCE.C F0806/F0807's C0_MUSIC_ENTRANCE through the admitted
 * F31 Game executable. It never borrows a PC/Amiga cue or returns a track
 * for an unverified handoff. */
int csb_v1_fmtowns_game_entrance_music_track(
    const CSB_V1_FmtownsGameHandoffReceipt *receipt,
    uint8_t *out_track);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_FMTOWNS_GAME_H */

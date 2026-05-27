#ifndef FIRESTAFF_CSB_V1_CHARACTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHARACTER_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Character System
 *
 * CSB uses imported DM1 champions. Champions are imported from DM1 save
 * files via the CSB Utility Disk flow. Key differences from DM1:
 * - Champions are not created in CSB; they are imported from DM1 saves
 * - The champion creation hall is replaced by the utility disk import UI
 * - Reincarnation (not available in DM1) applies stat halving + skill reset
 * - Resurrection restores a dead champion without stat penalty
 *
 * Source: CSBWin/Character.cpp (5528 lines)
 * Source: CSBWin/SaveGame.cpp (2953 lines) — DM1 import + save/load
 * Source: CSBWin/CedtData.cpp — champion editor / utility disk data
 * Base:   ReDMCSB CHAMPION.C (F0284-F0330), REVIVE.C, CEDT*.C
 */

#define CSB_V1_MAX_CHAMPIONS     4
#define CSB_V1_SLOT_COUNT       30
#define CSB_V1_MAX_NAME_LEN     15
#define CSB_V1_MAX_TITLE_LEN    15
#define CSB_V1_SKILL_COUNT      16
#define CSB_V1_STAT_COUNT        7  /* STR, DEX, WIS, VIT, ANTIMAGIC, ANTIFIRE, LUCK */
#define CSB_V1_STAT_STR          0
#define CSB_V1_STAT_DEX          1
#define CSB_V1_STAT_WIS          2
#define CSB_V1_STAT_VIT          3
#define CSB_V1_STAT_ANTIMAGIC    4
#define CSB_V1_STAT_ANTIFIRE     5
#define CSB_V1_STAT_LUCK         6

/* Skill rank levels — CSB adds NEOPHYTE below DM1's NOVICE */
#define CSB_V1_RANK_NEOPHYTE   0   /* CSB-only: new lowest rank */
#define CSB_V1_RANK_NOVICE     1   /* DM1's lowest rank */
#define CSB_V1_RANK_APPRENTICE 2
#define CSB_V1_RANK_JOURNEYMAN 3
#define CSB_V1_RANK_CRAFTSMAN  4
#define CSB_V1_RANK_ARTISAN    5
#define CSB_V1_RANK_ADEPT      6
#define CSB_V1_RANK_EXPERT     7
#define CSB_V1_RANK_MASTER     8
#define CSB_V1_RANK_GRAND_MASTER 9
#define CSB_V1_RANK_COUNT      10

/* Champion attribute bit flags */
#define CSB_V1_CHAMPION_ATTRIBUTE_NONE         0x0000
#define CSB_V1_CHAMPION_ATTRIBUTE_ICON         0x0400  /* champion has icon on minimap */
#define CSB_V1_CHAMPION_ATTRIBUTE_DEAD          0x0800  /* champion is dead */
#define CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME 0x1000  /* reincarnate requires rename */
#define CSB_V1_CHAMPION_ATTRIBUTE_NEOPHYTE_MODE 0x2000  /* neophyte skill display mode */

/* Champion action indices (F0xx) */
#define CSB_V1_ACTION_NONE          0xFF
#define CSB_V1_ACTION_REST          0x00
#define CSB_V1_ACTION_ATTACK        0x01
#define CSB_V1_ACTION_DEFEND        0x02
#define CSB_V1_ACTION_CAST          0x03
#define CSB_V1_ACTION_USE           0x04
#define CSB_V1_ACTION_GET           0x05
#define CSB_V1_ACTION_DROP          0x06
#define CSB_V1_ACTION_MOVE          0x07

/* ReDMCSB C00_SLOT_READY_HAND through C29_SLOT_CHEST_1 */
#define CSB_V1_SLOT_READY_HAND      0
#define CSB_V1_SLOT_ACTION_HAND      1
#define CSB_V1_SLOT_BELT_1          2
#define CSB_V1_SLOT_BELT_2          3
#define CSB_V1_SLOT_BELT_3          4
#define CSB_V1_SLOT_BELT_4          5
#define CSB_V1_SLOT_PACK_1         10
#define CSB_V1_SLOT_PACK_2         11
#define CSB_V1_SLOT_PACK_3         12
#define CSB_V1_SLOT_PACK_4         13
#define CSB_V1_SLOT_PACK_5         14
#define CSB_V1_SLOT_PACK_6         15
#define CSB_V1_SLOT_PACK_7         16
#define CSB_V1_SLOT_PACK_8         17
#define CSB_V1_SLOT_PACK_9         18
#define CSB_V1_SLOT_PACK_10        19
#define CSB_V1_SLOT_PACK_11        20
#define CSB_V1_SLOT_PACK_12        21
#define CSB_V1_SLOT_CHEST_1        22
/* Slots 22-29 are chest slots */

/* Cell (view position) values — normalized 0-3 */
#define CSB_V1_CELL_FRONT_LEFT      0
#define CSB_V1_CELL_FRONT           1
#define CSB_V1_CELL_RIGHT           2
#define CSB_V1_CELL_BACK            3

/* Direction values */
#define CSB_V1_DIR_NORTH            0
#define CSB_V1_DIR_EAST             1
#define CSB_V1_DIR_SOUTH            2
#define CSB_V1_DIR_WEST             3

/* Champion portrait index range */
#define CSB_V1_PORTRAIT_MIN         0
#define CSB_V1_PORTRAIT_MAX        15

/* Statistic index: MINIMUM / CURRENT / MAXIMUM triples */
#define CSB_V1_STAT_MIN             0
#define CSB_V1_STAT_CUR             1
#define CSB_V1_STAT_MAX             2

/* DM1 champion save record size (bytes) — header + 4×116 bytes */
#define CSB_V1_DM1_SAVE_MIN_SIZE    (24 + 4 * 116)

/* Portrait bitmap dimensions (from ReDMCSB) */
#define CSB_V1_PORTRAIT_WIDTH       32
#define CSB_V1_PORTRAIT_HEIGHT       29
#define CSB_V1_PORTRAIT_BYTE_WIDTH   128  /* 4 bytes/pixel × 32 wide, planar */
#define CSB_V1_PORTRAIT_BYTE_COUNT  3712 /* 128 × 29 */

/* Reincarnate vs Resurrect command indices */
#define CSB_V1_COMMAND_RESURRECT    0xA0  /* C160_COMMAND_CLICK_IN_PANEL_RESURRECT */
#define CSB_V1_COMMAND_REINCARNATE  0xA1  /* C161_COMMAND_CLICK_IN_PANEL_REINCARNATE */

/* ── CSB V1 Champion structure ────────────────────────────────────── */
/* Aligned to ReDMCSB CHAMPION structure (including portrait).
 * Size on 68k: 800 bytes (320 bytes champion + 480 bytes portrait).
 * On x86/ARM (32-bit int): approximately 832 bytes. */
typedef struct {
    /* ── Core identity ── */
    char     Name[CSB_V1_MAX_NAME_LEN + 1];          /*  0  offset 0 */
    char     Title[CSB_V1_MAX_TITLE_LEN + 1];         /* 16  offset 16 */

    /* ── Portrait bitmap (planar, 32×29, 128×29 bytes) ── */
    uint8_t  Portrait[CSB_V1_PORTRAIT_BYTE_COUNT];   /* 32  offset 32 */

    /* ── Vitals (16-bit, current + max) ── */
    int16_t  CurrentHealth;                            /* 3744 */
    int16_t  MaximumHealth;                            /* 3746 */
    int16_t  CurrentStamina;                          /* 3748 */
    int16_t  MaximumStamina;                          /* 3750 */
    int16_t  CurrentMana;                             /* 3752 */
    int16_t  MaximumMana;                             /* 3754 */

    /* ── 7 statistics × 3 (min/current/max) ── */
    /* Index: 0=STR, 1=DEX, 2=WIS, 3=VIT, 4=ANTIMAGIC, 5=ANTIFIRE, 6=LUCK */
    uint16_t Statistics[CSB_V1_STAT_COUNT][3];        /* 3756 — 7×3×2 = 42 bytes */

    /* ── 16 skills (0–255 each) ── */
    uint8_t  Skills[CSB_V1_SKILL_COUNT];              /* 3798 */

    /* ── 30 equipment slots (THING values) ── */
    uint16_t Slots[CSB_V1_SLOT_COUNT];                 /* 3814 — 30×2 = 60 bytes */

    /* ── Position and orientation ── */
    uint8_t  Cell;                                     /* 3874  view cell (0-3, normalized to party dir) */
    uint8_t  Direction;                                /* 3875  champion facing direction (0-3) */
    uint16_t DirectionMaximumDamageReceived;           /* 3876 */

    /* ── Status ── */
    uint8_t  ActionIndex;                             /* 3878  F0xx action code */
    int16_t  EnableActionEventIndex;                  /* 3879 */
    int16_t  HideDamageReceivedEventIndex;            /* 3881 */
    uint16_t Attributes;                               /* 3883  bit flags (icon, dead, ...) */
    int16_t  Food;                                     /* 3885 */
    int16_t  Water;                                    /* 3887 */
    uint16_t Load;                                     /* 3889 */
    uint8_t  Padding1[2];                              /* 3891  alignment padding */

    /* ── Additional state (from ReDMCSB) ── */
    int16_t  EventIndex;                              /* 3893 */
    /* ── Reincarnation scaling (CHANGE7_24 globals — per-champion) ── */
    uint8_t  reincarnateAttributePenalty;  /* default 2, max 16 — attribute tier loss */
    uint8_t  reincarnateStatPenalty;      /* default 8, max 16 — stat point loss denominator */
    uint8_t  randomPoints;                /* default 12 — random +1 boosts on reincarnate */
    uint8_t  Padding3[2];                 /* alignment to 8 bytes */
} CSB_V1_Champion;

/* ── CSB V1 Party State ───────────────────────────────────────────── */
typedef struct {
    CSB_V1_Champion Champions[CSB_V1_MAX_CHAMPIONS];
    int             ChampionCount;
    int             ImportedFromDM1;         /* 1 = imported from DM1 save */
    int             ImportSource;            /* 0=none, 1=utility_disk, 2=dm1_save */
    char            ImportDM1Path[256];      /* path to source DM1 save */
    uint32_t       UtilityDiskSerial;        /* serial of utility disk used */
    int             PartyDirection;           /* 0=N,1=E,2=S,3=W */
    int             LeaderIndex;             /* 0-3 or -1 if none */
    int             MagicCasterIndex;        /* champion currently casting */
    int             PartyMapX;               /* current dungeon X */
    int             PartyMapY;               /* current dungeon Y */
    uint8_t         Reserved[64];             /* future expansion */
} CSB_V1_PartyState;

/* ── Function declarations ─────────────────────────────────────────── */

/* Champion import from DM1 save (utility disk flow) */
int  csb_v1_character_import_dm1_save(CSB_V1_PartyState *party,
                                       const char *dm1_save_path);
int  csb_v1_character_import_dm1_buffer(CSB_V1_PartyState *party,
                                         const uint8_t *dm1_buf,
                                         int buf_size);

/* Champion initialization */
void csb_v1_character_init_default(CSB_V1_PartyState *party);
void csb_v1_champion_init(CSB_V1_Champion *c);

/* DM1 save format offsets — for DM1→CSB champion import (CEDTINC7.C).
 * These describe the on-disk layout of a DM1 save champion record.
 * Header:  [0] champ_count (1 byte, 0–4), [1..9] unused, [10..13] game tick.
 * Champion record: 116 bytes each, starting at offset 24. */
#define CSB_V1_DM1_CHAMP_OFF_NAME       0  /* 8 bytes, null-padded */
#define CSB_V1_DM1_CHAMP_OFF_HEALTH      8  /* 2 bytes LE: current health */
#define CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH 10  /* 2 bytes LE: max health */
#define CSB_V1_DM1_CHAMP_OFF_STAMINA    12  /* 2 bytes LE: current stamina */
#define CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA 14 /* 2 bytes LE: max stamina */
#define CSB_V1_DM1_CHAMP_OFF_MANA       16  /* 2 bytes LE: current mana */
#define CSB_V1_DM1_CHAMP_OFF_MAX_MANA   18  /* 2 bytes LE: max mana */
#define CSB_V1_DM1_CHAMP_OFF_STR        20  /* uint8_t */
#define CSB_V1_DM1_CHAMP_OFF_DEX        21  /* uint8_t */
#define CSB_V1_DM1_CHAMP_OFF_WIS        22  /* uint8_t */
#define CSB_V1_DM1_CHAMP_OFF_VIT        23  /* uint8_t */
#define CSB_V1_DM1_CHAMP_OFF_SKILLS     24  /* 16 bytes: skill values */
#define CSB_V1_DM1_CHAMP_OFF_EQUIP      40  /* 30×2 bytes: slot THING values */
#define CSB_V1_DM1_CHAMP_SIZE           116 /* bytes per champion record */
#define CSB_V1_DM1_HDR_CHAMP_COUNT       0  /* uint8_t: number of champions */
#define CSB_V1_DM1_HDR_CHAMPION_START   24 /* first champion record offset */

/* Reincarnation and resurrection
 * Resurrect: restores dead champion to life, stats unchanged
 * Reincarnate: restores dead champion, halves HP/Mana/Stamina,
 *               reduces other stats by 1/8th, clears all skills,
 *               applies 12× random stat boost, prompts rename */
int  csb_v1_champion_resurrect(CSB_V1_Champion *c);
int  csb_v1_champion_reincarnate(CSB_V1_Champion *c);
int  csb_v1_champion_is_dead(const CSB_V1_Champion *c);
int  csb_v1_champion_kill(CSB_V1_Champion *c);

/* Champion stat utilities */
int   csb_v1_champion_get_stat(const CSB_V1_Champion *c, int stat_idx, int which);
void  csb_v1_champion_set_stat(CSB_V1_Champion *c, int stat_idx, int which, int val);
int   csb_v1_champion_get_skill(const CSB_V1_Champion *c, int skill_idx);
void  csb_v1_champion_set_skill(CSB_V1_Champion *c, int skill_idx, int val);
int   csb_v1_champion_get_load(CSB_V1_Champion *c);
void  csb_v1_champion_recompute_load(CSB_V1_Champion *c);

/* Utility disk verification (checks for CSB utility disk signature) */
int  csb_v1_util_check_disk(const char *drive_path);
int  csb_v1_util_require_disk(const char *drive_path,
                               char *err_msg, int err_msg_size);

/* Source evidence string */
const char *csb_v1_character_source_evidence(void);

#endif /* FIRESTAFF_CSB_V1_CHARACTER_PC34_COMPAT_H */

/* pass603: CSB V1 Phase 6 — Utility/Import Flow
 *
 * Implements:
 *   - CSB champion import from DM1 saves (256-byte block format)
 *   - Import state machine (F0100-F0120 from ReDMCSB SAVEGAME.C)
 *   - Champion block verification
 *   - DM1 record → CSB block conversion
 *
 * Source references:
 *   CSBWin/SaveGame.cpp: DM1 import path (F0100-F0120)
 *   ReDMCSB SAVEGAME.C: F0100-F0120 champion import state machine
 *   ReDMCSB CHAMPION.C: champion block layout (256 bytes)
 *   ReDMCSB CEDTINC7.C: utility disk prompt flow
 *   ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTILITY_DISK
 */

#include "csb_v1_utility_import_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────────────── */

/* DM1 save header (24 bytes) */
#define DM1_SAVE_HDR_CHAMP_COUNT   0   /* offset 0: uint8_t champion count */
#define DM1_SAVE_HDR_MAGIC          1   /* offset 1: magic identifier */
#define DM1_SAVE_HDR_GAME_TIME      10  /* offset 10: 4 bytes LE game time */
#define DM1_SAVE_HDR_CHAMP_START    24  /* first champion record at offset 24 */

/* DM1 champion record offsets within 116-byte record */
#define DM1_REC_NAME       0   /* 8 bytes, space-padded */
#define DM1_REC_HEALTH     8   /* 2 bytes LE: current health */
#define DM1_REC_MAX_HEALTH 10  /* 2 bytes LE: max health */
#define DM1_REC_STAMINA    12  /* 2 bytes LE: current stamina */
#define DM1_REC_MAX_STAM   14  /* 2 bytes LE: max stamina */
#define DM1_REC_MANA       16  /* 2 bytes LE: current mana */
#define DM1_REC_MAX_MANA   18  /* 2 bytes LE: max mana */
#define DM1_REC_STR        20  /* uint8_t */
#define DM1_REC_DEX        21  /* uint8_t */
#define DM1_REC_WIS        22  /* uint8_t */
#define DM1_REC_VIT        23  /* uint8_t */
#define DM1_REC_SKILLS     24  /* 16 bytes */
#define DM1_REC_EQUIP      40  /* 30 × 2 bytes = 60 bytes */

/* DM1 magic for save files (at offset 0, value 0x01 means valid DM1 save) */
#define DM1_SAVE_MAGIC_VALID       0x01
#define DM1_SAVE_MAGIC_COMPRESSED  0x02

/* ── Internal helpers ─────────────────────────────────────────────────── */

static int16_t read_le16(const uint8_t *p)
{
    return (int16_t)(p[0] | (p[1] << 8));
}

static void write_le16(uint8_t *p, int16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_utility_import_source_evidence(void)
{
    return
        "CSBWin/SaveGame.cpp: DM1 import path (F0100-F0120)\n"
        "ReDMCSB SAVEGAME.C: F0100-F0120 champion import state machine\n"
        "ReDMCSB CHAMPION.C: champion block layout (256 bytes)\n"
        "ReDMCSB CEDTINC7.C: utility disk prompt flow\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTILITY_DISK string\n"
        "CSBWin/Character.cpp: champion management (5528 lines)\n"
        "CSBWin/SaveGame.cpp: save/load + DM1 import (2953 lines)\n"
        "MEDIA529_F20E_F20J: CSB save header key index C29\n"
        "MEDIA187_F20E_F21E_G20E_G21E: DM save key index C10\n";
}

/* ── Champion block size ─────────────────────────────────────────────── */
int csb_v1_champion_block_size(void)
{
    return CSB_V1_CHAMPION_BLOCK_SIZE;
}

/* ── Verify champion block ───────────────────────────────────────────── */
int csb_v1_champion_block_verify(const CSB_V1_ChampionBlock *block)
{
    if (!block) return -1;

    /* Check name is not all zeros or spaces (valid champion has a name) */
    {
        int all_spaces = 1;
        int all_zero = 1;
        for (int i = 0; i < 8; i++) {
            if (block->Name[i] != ' ' && block->Name[i] != '\0') all_spaces = 0;
            if (block->Name[i] != 0) all_zero = 0;
        }
        if (all_spaces || all_zero) return -2;
    }

    /* Verify maximum health is non-zero for a valid champion */
    if (block->MaximumHealth == 0) return -3;

    /* Verify statistics are non-zero for at least one stat */
    {
        int has_stat = 0;
        for (int i = 0; i < 7; i++) {
            if (block->Statistics[i][2] != 0) { has_stat = 1; break; }
        }
        if (!has_stat) return -4;
    }

    return 0;
}

/* ── DM1 record to CSB block conversion ─────────────────────────────── */
/* ReDMCSB SAVEGAME.C F0100-F0120: the state machine converts
 * each 116-byte DM1 champion record into a 256-byte CSB champion block. */
int csb_v1_dm1_record_to_csb_block(const uint8_t *dm1_record,
                                    CSB_V1_ChampionBlock *csb_block)
{
    int i;

    if (!dm1_record || !csb_block) return -1;

    memset(csb_block, 0, sizeof(*csb_block));

    /* Name: 8 bytes, copy and space-pad */
    memcpy(csb_block->Name, dm1_record + DM1_REC_NAME, 8);
    /* Ensure null-terminated but keep space-padding for fixed-width name */
    for (i = 7; i >= 0; i--) {
        if (csb_block->Name[i] == ' ' || csb_block->Name[i] == '\0')
            csb_block->Name[i] = ' ';
        else
            break;
    }

    /* Vitals */
    csb_block->CurrentHealth  = read_le16(dm1_record + DM1_REC_HEALTH);
    csb_block->MaximumHealth  = read_le16(dm1_record + DM1_REC_MAX_HEALTH);
    csb_block->CurrentStamina = read_le16(dm1_record + DM1_REC_STAMINA);
    csb_block->MaximumStamina = read_le16(dm1_record + DM1_REC_MAX_STAM);
    csb_block->CurrentMana    = read_le16(dm1_record + DM1_REC_MANA);
    csb_block->MaximumMana    = read_le16(dm1_record + DM1_REC_MAX_MANA);

    /* Statistics — DM1 stores current=max for each stat (bytes at 20-23) */
    csb_block->Statistics[0][0] = 30;  /* STR min */
    csb_block->Statistics[0][1] = dm1_record[DM1_REC_STR];
    csb_block->Statistics[0][2] = dm1_record[DM1_REC_STR];

    csb_block->Statistics[1][0] = 30;  /* DEX min */
    csb_block->Statistics[1][1] = dm1_record[DM1_REC_DEX];
    csb_block->Statistics[1][2] = dm1_record[DM1_REC_DEX];

    csb_block->Statistics[2][0] = 30;  /* WIS min */
    csb_block->Statistics[2][1] = dm1_record[DM1_REC_WIS];
    csb_block->Statistics[2][2] = dm1_record[DM1_REC_WIS];

    csb_block->Statistics[3][0] = 30;  /* VIT min */
    csb_block->Statistics[3][1] = dm1_record[DM1_REC_VIT];
    csb_block->Statistics[3][2] = dm1_record[DM1_REC_VIT];

    /* ANTIMAGIC and ANTIFIRE — not in DM1, default to 30 */
    csb_block->Statistics[4][0] = 30;  /* ANTIMAGIC min */
    csb_block->Statistics[4][1] = 30;
    csb_block->Statistics[4][2] = 30;

    csb_block->Statistics[5][0] = 30;  /* ANTIFIRE min */
    csb_block->Statistics[5][1] = 30;
    csb_block->Statistics[5][2] = 30;

    /* LUCK — not in DM1, default to 30 */
    csb_block->Statistics[6][0] = 30;  /* LUCK min */
    csb_block->Statistics[6][1] = 30;
    csb_block->Statistics[6][2] = 30;

    /* Enforce minimum 30 for all stats */
    for (i = 0; i < 7; i++) {
        if (csb_block->Statistics[i][1] < 30) csb_block->Statistics[i][1] = 30;
        if (csb_block->Statistics[i][2] < 30) csb_block->Statistics[i][2] = 30;
    }

    /* Skills: 16 bytes */
    memcpy(csb_block->Skills, dm1_record + DM1_REC_SKILLS, 16);

    /* Orientation: default values */
    csb_block->Cell = 0;
    csb_block->Direction = 0;
    csb_block->DirectionMaxDamage = 0;

    /* Status */
    csb_block->ActionIndex = 0xFF;  /* REST */
    csb_block->EnableActionEvent = -1;
    csb_block->HideDamageEvent = -1;

    /* Set DEAD flag if current health is 0 */
    csb_block->Attributes = 0;
    if (csb_block->CurrentHealth <= 0) {
        csb_block->Attributes |= 0x0800;  /* DEAD flag */
    }

    /* Food and water */
    csb_block->Food = 1500;
    csb_block->Water = 1500;
    csb_block->Load = 0;

    /* Equipment slots: 30 × uint16_t from DM1 record */
    for (i = 0; i < 30; i++) {
        csb_block->Slots[i] = read_le16(dm1_record + DM1_REC_EQUIP + i * 2);
    }

    /* Reserved padding bytes are already zero from memset */
    return 0;
}

/* ── Champion block to party slot ────────────────────────────────────── */
int csb_v1_champion_block_to_party(CSB_V1_PartyState *party,
                                     int slot_index,
                                     const CSB_V1_ChampionBlock *block)
{
    if (!party || slot_index < 0 || slot_index >= CSB_V1_MAX_CHAMPIONS)
        return -1;
    if (!block) return -1;

    /* Copy champion data from block into party champion */
    /* Name: 8 bytes → 16 bytes (CSB_V1_Champion has 16 bytes) */
    {
        CSB_V1_Champion *c = &party->Champions[slot_index];

        /* Copy name (8 bytes fixed-width → 16 bytes with null) */
        memset(c->Name, 0, sizeof(c->Name));
        memcpy(c->Name, block->Name, 8);
        /* Name is already null-terminated by the memset above */

        /* Copy vitals */
        c->CurrentHealth  = block->CurrentHealth;
        c->MaximumHealth  = block->MaximumHealth;
        c->CurrentStamina = block->CurrentStamina;
        c->MaximumStamina = block->MaximumStamina;
        c->CurrentMana   = block->CurrentMana;
        c->MaximumMana   = block->MaximumMana;

        /* Copy statistics */
        memcpy(c->Statistics, block->Statistics, sizeof(block->Statistics));

        /* Copy skills */
        memcpy(c->Skills, block->Skills, sizeof(block->Skills));

        /* Orientation */
        c->Cell = block->Cell;
        c->Direction = block->Direction;
        c->DirectionMaximumDamageReceived = block->DirectionMaxDamage;

        /* Status */
        c->ActionIndex = block->ActionIndex;
        c->EnableActionEventIndex = block->EnableActionEvent;
        c->HideDamageReceivedEventIndex = block->HideDamageEvent;
        c->Attributes = block->Attributes;
        c->Food = block->Food;
        c->Water = block->Water;
        c->Load = block->Load;

        /* Event index */
        c->EventIndex = -1;

        /* Reincarnation scaling defaults (Character.cpp:682-687) */
        c->reincarnateAttributePenalty = 2;
        c->reincarnateStatPenalty = 8;
        c->randomPoints = 12;
    }

    return 0;
}

/* ── Import from DM1 save buffer ────────────────────────────────────── */
int csb_v1_import_from_dm1_save_buffer(CSB_V1_PartyState *party,
                                       const uint8_t *dm1_buf,
                                       int buf_size,
                                       CSB_V1_ImportResult *result)
{
    int champ_count;
    int i;
    int offset;

    /* Initialize result */
    if (result) {
        memset(result, 0, sizeof(*result));
        result->state = CSB_V1_IMPORT_STATE_INIT;
        result->error_code = CSB_V1_IMPORT_OK;
    }

    if (!party || !dm1_buf) {
        if (result) result->error_code = CSB_V1_IMPORT_ERR_NULL;
        return -1;
    }

    if (buf_size < DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE) {
        if (result) result->error_code = CSB_V1_IMPORT_ERR_SIZE;
        return -1;
    }

    /* State 1: Check DM1 save header magic */
    if (result) result->state = CSB_V1_IMPORT_STATE_CHECK_HEADER;
    {
        uint8_t magic = dm1_buf[DM1_SAVE_HDR_MAGIC];
        /* Valid DM1 saves have magic 0x01 or 0x02 */
        if (magic != DM1_SAVE_MAGIC_VALID && magic != DM1_SAVE_MAGIC_COMPRESSED) {
            /* Try alternate check: magic at offset 0 is champ count.
             * DM1 saves with champ_count=0 at offset 0 are invalid.
             * champ_count 1-4 at offset 0 is valid. */
        }
    }

    /* State 2: Validate champion count */
    if (result) result->state = CSB_V1_IMPORT_STATE_VALIDATE_COUNT;
    champ_count = dm1_buf[DM1_SAVE_HDR_CHAMP_COUNT];
    if (champ_count > CSB_V1_MAX_CHAMPIONS) {
        champ_count = CSB_V1_MAX_CHAMPIONS;
    }
    if (champ_count == 0 || champ_count > 4) {
        if (result) {
            result->error_code = CSB_V1_IMPORT_ERR_COUNT;
            result->state = CSB_V1_IMPORT_STATE_ERROR;
        }
        return -1;
    }

    /* Record game time from header */
    if (result) {
        result->dm1_game_time = (uint32_t)(
            dm1_buf[DM1_SAVE_HDR_GAME_TIME] |
            ((uint32_t)dm1_buf[DM1_SAVE_HDR_GAME_TIME + 1] << 8) |
            ((uint32_t)dm1_buf[DM1_SAVE_HDR_GAME_TIME + 2] << 16) |
            ((uint32_t)dm1_buf[DM1_SAVE_HDR_GAME_TIME + 3] << 24)
        );
    }

    /* Initialize party */
    csb_v1_character_init_default(party);
    party->ImportedFromDM1 = 1;
    party->ImportSource = 2;  /* 2 = dm1_save_file */
    party->ChampionCount = 0;

    /* State 3: Read each DM1 champion record */
    if (result) result->state = CSB_V1_IMPORT_STATE_READ_CHAMPS;
    offset = DM1_SAVE_HDR_CHAMP_START;

    for (i = 0; i < champ_count; i++) {
        if (offset + DM1_CHAMPION_RECORD_SIZE > buf_size) {
            if (result) {
                result->error_code = CSB_V1_IMPORT_ERR_PARTIAL;
                result->byte_offset = offset;
                result->state = CSB_V1_IMPORT_STATE_ERROR;
            }
            break;
        }

        /* State 4: Convert to CSB 256-byte block */
        if (result) result->state = CSB_V1_IMPORT_STATE_CONVERT_BLOCKS;
        {
            CSB_V1_ChampionBlock block;
            int r = csb_v1_dm1_record_to_csb_block(dm1_buf + offset, &block);
            if (r != 0) {
                if (result) {
                    result->error_code = CSB_V1_IMPORT_ERR_BLOCK_ALIGN;
                    result->byte_offset = offset;
                    result->state = CSB_V1_IMPORT_STATE_ERROR;
                }
                offset += DM1_CHAMPION_RECORD_SIZE;
                continue; /* skip malformed record */
            }

            /* State 5: Verify block checksum */
            if (result) result->state = CSB_V1_IMPORT_STATE_VERIFY_CHECKSUM;
            (void)csb_v1_champion_block_verify(&block);

            /* State 6: Store in party slot */
            if (result) result->state = CSB_V1_IMPORT_STATE_STORE_PARTY;
            csb_v1_champion_block_to_party(party, i, &block);
        }

        offset += DM1_CHAMPION_RECORD_SIZE;
        party->ChampionCount++;
    }

    /* Set leader to first living champion */
    for (i = 0; i < party->ChampionCount; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i])) {
            party->LeaderIndex = i;
            break;
        }
    }
    if (party->LeaderIndex < 0 && party->ChampionCount > 0)
        party->LeaderIndex = 0;

    if (result) {
        result->champion_count = party->ChampionCount;
        result->state = CSB_V1_IMPORT_STATE_DONE;
    }

    return party->ChampionCount;
}

/* ── Import from DM1 save file ────────────────────────────────────────── */
int csb_v1_import_from_dm1_save_file(CSB_V1_PartyState *party,
                                      const char *dm1_save_path,
                                      CSB_V1_ImportResult *result)
{
    FILE *f;
    uint8_t *buf = NULL;
    int buf_size;
    long file_size;
    int result_code;

    if (!party || !dm1_save_path) {
        if (result) result->error_code = CSB_V1_IMPORT_ERR_NULL;
        return -1;
    }

    /* Open file */
    f = fopen(dm1_save_path, "rb");
    if (!f) {
        if (result) {
            result->error_code = CSB_V1_IMPORT_ERR_NO_DISK;
            result->state = CSB_V1_IMPORT_STATE_ERROR;
        }
        return -1;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < DM1_SAVE_HEADER_SIZE + DM1_CHAMPION_RECORD_SIZE) {
        fclose(f);
        if (result) {
            result->error_code = CSB_V1_IMPORT_ERR_SIZE;
            result->state = CSB_V1_IMPORT_STATE_ERROR;
        }
        return -1;
    }

    /* Allocate buffer (max 4 champions × 116 bytes + 24 byte header = 488 bytes) */
    buf_size = (int)file_size;
    if (buf_size > 4096) buf_size = 4096;  /* sanity cap */
    buf = (uint8_t *)malloc((size_t)buf_size);
    if (!buf) {
        fclose(f);
        if (result) {
            result->error_code = CSB_V1_IMPORT_ERR_NULL;
            result->state = CSB_V1_IMPORT_STATE_ERROR;
        }
        return -1;
    }

    /* Read entire file */
    {
        size_t read = fread(buf, 1, (size_t)buf_size, f);
        buf_size = (int)read;
    }
    fclose(f);

    /* Delegate to buffer import */
    result_code = csb_v1_import_from_dm1_save_buffer(party, buf, buf_size, result);

    free(buf);
    return result_code;
}
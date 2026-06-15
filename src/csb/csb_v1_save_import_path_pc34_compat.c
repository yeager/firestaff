/*
 * csb_v1_save_import_path_pc34_compat.c
 *
 * CSB V1 Champion Transfer/Import (Champions GAP 3).
 * Source-locked per ReDMCSB CHARACTER.C / CHAMPION.C
 * ReadingChampion()/WritingChampion(), DEFS.H:1289
 * (CSBGAME.DAT magic), CEDT006.C:101-118 (save-section
 * dispatch), Character.cpp:14 (reincarnation globals).
 *
 * v2 implements a real CSB v2.0/v2.1 roster importer on top
 * of the CSB_V1_PartyState data layer.  See the header for
 * the container layout.
 */
#include "csb_v1_save_import_path_pc34_compat.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Little-endian readers/writers ────────────────────────── */
static unsigned int rd_u32(const unsigned char* p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8)
         | ((unsigned)p[2] << 16) | ((unsigned)p[3] << 24);
}
static int rd_i16(const unsigned char* p) {
    int v = (int)((unsigned)p[0] | ((unsigned)p[1] << 8));
    if (v & 0x8000) v -= 0x10000;
    return v;
}
static unsigned rd_u16(const unsigned char* p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}
static void wr_u32(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xFF);
    p[1] = (unsigned char)((v >> 8) & 0xFF);
    p[2] = (unsigned char)((v >> 16) & 0xFF);
    p[3] = (unsigned char)((v >> 24) & 0xFF);
}
static void wr_i16(unsigned char* p, int v) {
    p[0] = (unsigned char)(v & 0xFF);
    p[1] = (unsigned char)((v >> 8) & 0xFF);
}
static void wr_u16(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)(v & 0xFF);
    p[1] = (unsigned char)((v >> 8) & 0xFF);
}

/* ── Variant detection (retained from v1) ─────────────────── */
CSB_V1_SaveVariant csb_v1_detect_save_variant(
    const unsigned char* header, int headerLen) {
    static const unsigned char kDm1Magic[8] = {
        'R','D','M','C','S','B','1','5'
    };
    if (!header || headerLen < 8) {
        return CSB_V1_SAVE_VARIANT_UNKNOWN;
    }
    if (memcmp(header, kDm1Magic, 8) == 0) {
        return CSB_V1_SAVE_VARIANT_DM1_PC34;
    }
    if (memcmp(header, "CSBGAME\0", 8) == 0) {
        unsigned int v;
        if (headerLen < 12) return CSB_V1_SAVE_VARIANT_UNKNOWN;
        v = rd_u32(header + CSB_SAVE_HDR_OFF_VERSION);
        if (v == CSB_SAVE_VERSION_V20) return CSB_V1_SAVE_VARIANT_CSB_V20;
        if (v == CSB_SAVE_VERSION_V21) return CSB_V1_SAVE_VARIANT_CSB_V21;
        return CSB_V1_SAVE_VARIANT_UNKNOWN;
    }
    return CSB_V1_SAVE_VARIANT_UNKNOWN;
}

int csb_v1_save_import_path_implemented(void) {
    return 1;
}

/* ── CHANGE7_24 reincarnation stat-cap penalty ────────────────
 * Source-locked per CSB:REVIVE.C CHANGE7_24 + Character.cpp:14
 * (reincarnateAttributePenalty=2, reincarnateStatPenalty=8).
 * A champion stored in a CSB save with the "reincarnated" flag
 * already lost stats in-game; on import we re-assert the cap so
 * a tampered/older save cannot smuggle un-penalised reincarnated
 * champions back in.  Mirrors csb_v1_champion_reincarnate's
 * attribute rule (cur/max reduced by 1/8th down to min) and the
 * HP/MP/STA halving, but operates on a CSB_V1_Champion.  Luck
 * (index 6) is exempt. */
static void csb_save_apply_reincarnation_cap(CSB_V1_Champion* c) {
    int i;
    if (!c) return;
    if (c->MaximumHealth  > 1) c->MaximumHealth  = (int16_t)(c->MaximumHealth  >> 1);
    if (c->MaximumStamina > 1) c->MaximumStamina = (int16_t)(c->MaximumStamina >> 1);
    if (c->MaximumMana    > 1) c->MaximumMana    = (int16_t)(c->MaximumMana    >> 1);
    if (c->CurrentHealth  > c->MaximumHealth)  c->CurrentHealth  = c->MaximumHealth;
    if (c->CurrentStamina > c->MaximumStamina) c->CurrentStamina = c->MaximumStamina;
    if (c->CurrentMana    > c->MaximumMana)    c->CurrentMana    = c->MaximumMana;

    for (i = CSB_V1_STAT_STR; i <= CSB_V1_STAT_ANTIFIRE; ++i) {
        int cur = (int)c->Statistics[i][CSB_V1_STAT_CUR];
        int max_ = (int)c->Statistics[i][CSB_V1_STAT_MAX];
        int min_ = (int)c->Statistics[i][CSB_V1_STAT_MIN];
        int ref = (cur > max_) ? cur : max_;
        int eighth = ref / 8;            /* reincarnateStatPenalty = 8 */
        int nv = cur - eighth;
        if (nv < min_) nv = min_;
        c->Statistics[i][CSB_V1_STAT_CUR] = (uint16_t)nv;
        if (max_ - eighth >= min_) {
            c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)(max_ - eighth);
        } else {
            c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)min_;
        }
    }
    /* Reincarnated champions need a rename in the original UI. */
    c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME;
}

/* ── Parse one CSB champion record ────────────────────────── */
static void parse_csb_champion_record(const unsigned char* rec,
                                      CSB_V1_Champion* c) {
    int i;
    int reincarnated;

    csb_v1_champion_init(c);

    /* Identity (16 bytes, NUL-padded). */
    {
        char name[CSB_V1_MAX_NAME_LEN + 1];
        int n = CSB_V1_MAX_NAME_LEN;
        memcpy(name, rec + CSB_SAVE_CH_OFF_NAME, (size_t)n);
        name[n] = '\0';
        /* trim trailing spaces */
        while (n > 0 && (name[n-1] == ' ' || name[n-1] == '\0')) {
            name[--n] = '\0';
        }
        memcpy(c->Name, name, sizeof(c->Name));
        c->Name[CSB_V1_MAX_NAME_LEN] = '\0';
    }

    reincarnated = rec[CSB_SAVE_CH_OFF_REINCARNATED] ? 1 : 0;

    /* Vitals. */
    c->CurrentHealth  = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_CUR_HP);
    c->MaximumHealth  = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_MAX_HP);
    c->CurrentStamina = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_CUR_STA);
    c->MaximumStamina = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_MAX_STA);
    c->CurrentMana    = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_CUR_MANA);
    c->MaximumMana    = (int16_t)rd_i16(rec + CSB_SAVE_CH_OFF_MAX_MANA);

    /* 7 stats current/max rows.  Min is fixed at 30 per the
     * DM1/CSB import convention (see csb_v1_character import). */
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        int cur = rd_i16(rec + CSB_SAVE_CH_OFF_STAT_CUR + i * 2);
        int max_ = rd_i16(rec + CSB_SAVE_CH_OFF_STAT_MAX + i * 2);
        if (cur < 30) cur = 30;
        if (max_ < cur) max_ = cur;
        c->Statistics[i][CSB_V1_STAT_MIN] = 30;
        c->Statistics[i][CSB_V1_STAT_CUR] = (uint16_t)cur;
        c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)max_;
    }

    /* Skills. */
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        c->Skills[i] = rec[CSB_SAVE_CH_OFF_SKILLS + i];
    }

    /* Equipped slots. */
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        c->Slots[i] = (uint16_t)rd_u16(rec + CSB_SAVE_CH_OFF_SLOTS + i * 2);
    }

    /* Dead flag. */
    if (rec[CSB_SAVE_CH_OFF_DEAD] ||
        (c->CurrentHealth <= 0 && c->MaximumHealth <= 0)) {
        c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    }

    /* CSB-only reincarnation penalty (HoC delta, CHANGE7_24). */
    if (reincarnated) {
        csb_save_apply_reincarnation_cap(c);
    }

    csb_v1_champion_recompute_load(c);
}

/* ── Buffer importer ──────────────────────────────────────── */
int csb_v1_import_csb_save_buffer(CSB_V1_PartyState* party,
                                  const unsigned char* buf,
                                  long len) {
    CSB_V1_SaveVariant variant;
    int champCount;
    int i;
    long need;

    if (!party || !buf) return CSB_SAVE_IMPORT_ERR_NULL;
    if (len < CSB_SAVE_HEADER_SIZE) return CSB_SAVE_IMPORT_ERR_TRUNCATED;

    if (memcmp(buf, "CSBGAME\0", CSB_SAVE_MAGIC_LEN) != 0) {
        return CSB_SAVE_IMPORT_ERR_BAD_MAGIC;
    }
    variant = csb_v1_detect_save_variant(buf, (int)len);
    if (variant != CSB_V1_SAVE_VARIANT_CSB_V20 &&
        variant != CSB_V1_SAVE_VARIANT_CSB_V21) {
        return CSB_SAVE_IMPORT_ERR_VERSION;
    }

    champCount = buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT];
    if (champCount < 1 || champCount > CSB_V1_MAX_CHAMPIONS) {
        return CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS;
    }

    need = (long)CSB_SAVE_HEADER_SIZE
         + (long)champCount * CSB_SAVE_CHAMP_SIZE;
    if (len < need) return CSB_SAVE_IMPORT_ERR_TRUNCATED;

    csb_v1_character_init_default(party);
    party->ImportedFromDM1 = 0; /* this is a CSB save, not DM1 */
    party->ImportSource = CSB_SAVE_IMPORT_SOURCE;
    /* Record the variant in Reserved[0] as the import stamp so a
     * subsequent edit does not re-import (caller checks
     * ImportSource == CSB_SAVE_IMPORT_SOURCE). */
    party->Reserved[0] = (unsigned char)
        (variant == CSB_V1_SAVE_VARIANT_CSB_V21 ? 0x21 : 0x20);

    for (i = 0; i < champCount; ++i) {
        const unsigned char* rec = buf + CSB_SAVE_HEADER_SIZE
                                 + (long)i * CSB_SAVE_CHAMP_SIZE;
        parse_csb_champion_record(rec, &party->Champions[i]);
    }
    party->ChampionCount = champCount;

    party->LeaderIndex = -1;
    for (i = 0; i < champCount; ++i) {
        if (!csb_v1_champion_is_dead(&party->Champions[i])) {
            party->LeaderIndex = i;
            break;
        }
    }
    if (party->LeaderIndex < 0) party->LeaderIndex = 0;

    return champCount;
}

/* ── File importer ────────────────────────────────────────── */
int csb_v1_import_csb_save_file(CSB_V1_PartyState* party,
                                const char* path) {
    FILE* f;
    unsigned char* buf;
    long size;
    int rc;

    if (!party || !path) return CSB_SAVE_IMPORT_ERR_NULL;
    f = fopen(path, "rb");
    if (!f) return CSB_SAVE_IMPORT_ERR_IO;

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return CSB_SAVE_IMPORT_ERR_IO; }
    size = ftell(f);
    if (size < 0) { fclose(f); return CSB_SAVE_IMPORT_ERR_IO; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return CSB_SAVE_IMPORT_ERR_IO; }

    buf = (unsigned char*)malloc((size_t)(size > 0 ? size : 1));
    if (!buf) { fclose(f); return CSB_SAVE_IMPORT_ERR_IO; }
    if (fread(buf, 1, (size_t)size, f) != (size_t)size) {
        free(buf); fclose(f);
        return CSB_SAVE_IMPORT_ERR_IO;
    }
    fclose(f);

    rc = csb_v1_import_csb_save_buffer(party, buf, size);
    free(buf);
    return rc;
}

/* ── Save builder (round-trip inverse) ────────────────────── */
long csb_v1_build_csb_save_buffer(const CSB_V1_PartyState* party,
                                  unsigned int version,
                                  unsigned char* out,
                                  long outCapacity) {
    int champCount;
    int i;
    long need;

    if (!party || !out) return CSB_SAVE_IMPORT_ERR_NULL;
    if (version != CSB_SAVE_VERSION_V20 && version != CSB_SAVE_VERSION_V21) {
        return CSB_SAVE_IMPORT_ERR_VERSION;
    }
    champCount = party->ChampionCount;
    if (champCount < 1 || champCount > CSB_V1_MAX_CHAMPIONS) {
        return CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS;
    }
    need = (long)CSB_SAVE_HEADER_SIZE
         + (long)champCount * CSB_SAVE_CHAMP_SIZE;
    if (outCapacity < need) return CSB_SAVE_IMPORT_ERR_TRUNCATED;

    memset(out, 0, (size_t)need);
    memcpy(out + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
    wr_u32(out + CSB_SAVE_HDR_OFF_VERSION, version);
    out[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = (unsigned char)champCount;
    wr_u32(out + CSB_SAVE_HDR_OFF_GAME_ID, 0u);

    for (i = 0; i < champCount; ++i) {
        const CSB_V1_Champion* c = &party->Champions[i];
        unsigned char* rec = out + CSB_SAVE_HEADER_SIZE
                           + (long)i * CSB_SAVE_CHAMP_SIZE;
        int s;
        memcpy(rec + CSB_SAVE_CH_OFF_NAME, c->Name, CSB_V1_MAX_NAME_LEN);
        rec[CSB_SAVE_CH_OFF_REINCARNATED] = 0; /* exported as already-capped */
        rec[CSB_SAVE_CH_OFF_DEAD] =
            (c->Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD) ? 1 : 0;
        wr_i16(rec + CSB_SAVE_CH_OFF_CUR_HP,   c->CurrentHealth);
        wr_i16(rec + CSB_SAVE_CH_OFF_MAX_HP,   c->MaximumHealth);
        wr_i16(rec + CSB_SAVE_CH_OFF_CUR_STA,  c->CurrentStamina);
        wr_i16(rec + CSB_SAVE_CH_OFF_MAX_STA,  c->MaximumStamina);
        wr_i16(rec + CSB_SAVE_CH_OFF_CUR_MANA, c->CurrentMana);
        wr_i16(rec + CSB_SAVE_CH_OFF_MAX_MANA, c->MaximumMana);
        for (s = 0; s < CSB_V1_STAT_COUNT; ++s) {
            wr_i16(rec + CSB_SAVE_CH_OFF_STAT_CUR + s * 2,
                   (int)c->Statistics[s][CSB_V1_STAT_CUR]);
            wr_i16(rec + CSB_SAVE_CH_OFF_STAT_MAX + s * 2,
                   (int)c->Statistics[s][CSB_V1_STAT_MAX]);
        }
        for (s = 0; s < CSB_V1_SKILL_COUNT; ++s) {
            rec[CSB_SAVE_CH_OFF_SKILLS + s] = c->Skills[s];
        }
        for (s = 0; s < CSB_V1_SLOT_COUNT; ++s) {
            wr_u16(rec + CSB_SAVE_CH_OFF_SLOTS + s * 2, c->Slots[s]);
        }
    }
    return need;
}

/* ── Legacy entry point (OMFATTANDE-stub compatibility) ───── */
int csb_v1_import_csb_save(const char* path) {
    CSB_V1_PartyState party;
    if (!path) return CSB_SAVE_IMPORT_ERR_NULL;
    return csb_v1_import_csb_save_file(&party, path);
}

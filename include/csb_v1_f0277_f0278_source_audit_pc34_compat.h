#ifndef FIRESTAFF_CSB_V1_F0277_F0278_SOURCE_AUDIT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0277_F0278_SOURCE_AUDIT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * Evidence-only bindings for ReDMCSB COPYPRO6.C F0277 and CHAMPRST.C F0278.
 * These APIs never write CPSE, champion, leader-hand, DSA, or runtime state.
 */

enum {
    CSB_V1_F0277_PC34_SECTOR_BYTES = 509,
    CSB_V1_F0277_FUZZY_OFFSET = 20,
    CSB_V1_F0277_FUZZY_BYTE_COUNT = 489,
    CSB_V1_F0277_FUZZY_WORD_COUNT = 32,
    CSB_V1_F0278_MAX_PARTY_CHAMPIONS = 4,
    CSB_V1_F0278_THING_NONE = 0xffff,
    CSB_V1_F0278_ICON_NONE = 0xffff,
    CSB_V1_F0278_CHAMPION_DIRTY_MASK = 0xff80
};

typedef struct CSB_V1_F0277RawSectorPc34 {
    const uint8_t *sector_bytes;
    size_t sector_size;
    const uint16_t *prior_fuzzy_words;
    size_t prior_fuzzy_word_count;
    uint32_t raw_capture_identity;
    int csb_pc34_platform;
    int authenticated;
} CSB_V1_F0277RawSectorPc34;

typedef struct CSB_V1_F0277FuzzyReceiptPc34 {
    int admitted;
    int platform_authenticated;
    int raw_shape_valid;
    int analysis_intentionally_unexecuted;
    uint32_t raw_capture_identity;
    uint32_t sector_fingerprint;
    const char *source_evidence;
} CSB_V1_F0277FuzzyReceiptPc34;

typedef struct CSB_V1_F0278ChampionRawStatePc34 {
    int new_game;
    int party_count;
    const uint16_t *champion_attributes;
    size_t champion_attribute_count;
    uint16_t leader_hand_thing;
    uint16_t leader_hand_icon;
    int leader_index;
    int magic_caster_index;
    uint32_t raw_state_identity;
    int authenticated;
} CSB_V1_F0278ChampionRawStatePc34;

typedef struct CSB_V1_F0278ResetPlanPc34 {
    int admitted;
    int clears_leader_hand;
    int restores_leader_hand;
    int marks_empty_hand;
    int clears_champion_dirty_attributes;
    int redraw_is_not_invoked;
    int leader_restore_is_not_invoked;
    int magic_caster_restore_is_not_invoked;
    uint16_t champion_dirty_mask;
    uint32_t raw_state_identity;
    const char *source_evidence;
} CSB_V1_F0278ResetPlanPc34;

int csb_v1_f0277_fuzzy_bits_raw_receipt_pc34(
    const CSB_V1_F0277RawSectorPc34 *raw,
    CSB_V1_F0277FuzzyReceiptPc34 *out);

int csb_v1_f0278_champion_reset_plan_pc34(
    const CSB_V1_F0278ChampionRawStatePc34 *raw,
    CSB_V1_F0278ResetPlanPc34 *out);

#endif

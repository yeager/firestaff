#ifndef FIRESTAFF_DM1_V1_LIVE_ACTION_EFFECTS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LIVE_ACTION_EFFECTS_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB: MENU.C F0407, PROJEXPL.C F0231 and TIMELINE.C F0253.
 * This is the DM1-owned live receipt for action results.  It keeps a
 * completed action in the same tick-addressed runtime domain as spells and
 * projectiles, rather than leaving presentation to inspect a stale emission. */
enum {
    DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34 = 16,
    DM1_V1_LIVE_ACTION_EFFECT_DAMAGE_PC34 = 1,
    DM1_V1_LIVE_ACTION_EFFECT_MISS_PC34 = 2,
    DM1_V1_LIVE_ACTION_EFFECT_DOOR_PC34 = 3,
    DM1_V1_LIVE_ACTION_EFFECT_ACTION_LOCK_PC34 = 4,
    DM1_V1_LIVE_ACTION_EFFECT_SPELL_PC34 = 5
};

enum {
    DM1_V1_ACTION_HUD_PRESENTATION_NONE_PC34 = 0,
    DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34 = 1,
    DM1_V1_ACTION_HUD_PRESENTATION_MISS_PC34 = 2,
    DM1_V1_ACTION_HUD_PRESENTATION_DOOR_PC34 = 3,
    DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34 = 4,
    DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34 = 5,
    DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34 = 6,
    DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34 = 7,
    DM1_V1_ACTION_HUD_PRESENTATION_SPELL_FAILURE_PC34 = 8
};

enum {
    DM1_V1_ACTION_HUD_LAYOUT_NONE_PC34 = 0,
    DM1_V1_ACTION_HUD_LAYOUT_CREATURE_DAMAGE_PC34 = 1,
    DM1_V1_ACTION_HUD_LAYOUT_ACTION_MENU_ROW_PC34 = 2,
    DM1_V1_ACTION_HUD_LAYOUT_SPELL_AREA_PC34 = 3,
    DM1_V1_ACTION_HUD_LAYOUT_MESSAGE_LINE_PC34 = 4
};

enum {
    DM1_V1_ACTION_HUD_TEXT_CAPACITY_PC34 = 64
};

typedef struct {
    int kind;
    int championIndex;
    int actionIndex;
    int damage;
    int combatOutcome;
    int defenseDelta;
    int doorAffected;
    unsigned char remainingTicks;
    uint32_t sourceTick;
    uint32_t serial;
} DM1_V1_LiveActionEffectPc34;

typedef struct {
    DM1_V1_LiveActionEffectPc34 entries[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
    int count;
    uint32_t nextSerial;
    uint32_t lastAdvancedTick;
} DM1_V1_LiveActionEffectsPc34;

typedef struct {
    int kind;
    int championIndex;
    int actionIndex;
    int damage;
    int combatOutcome;
    int defenseDelta;
    int doorAffected;
    int disabledTicks;
    uint32_t sourceTick;
} DM1_V1_LiveActionEffectInputPc34;

typedef struct {
    int valid;
    int materialized;
    int replaced;
    int slot;
} DM1_V1_LiveActionEffectReceiptPc34;

typedef struct {
    int valid;
    int advanced;
    int expiredCount;
    int expiredChampionIndex[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
    int expiredActionIndex[DM1_V1_LIVE_ACTION_EFFECT_CAPACITY_PC34];
} DM1_V1_LiveActionEffectsAdvancePlanPc34;

typedef struct {
    int failureType;
    int messageColor;
    int printsLineFeed;
    int printsChampionName;
    int appendsBaseSkillName;
    int clearsSymbolsOnCastClick;
    int redrawsAvailableSymbols;
    int redrawsChampionSymbols;
    const char *messageBeforeSkill;
    const char *messageAfterSkill;
} DM1_V1_SpellFailureHudFeedbackPc34;

typedef struct {
    int valid;
    int drawable;
    int sourceEffectKind;
    int presentationKind;
    int layoutKind;
    int championIndex;
    int actionIndex;
    int spellKind;
    int spellType;
    int spellPowerOrdinal;
    int damage;
    int combatOutcome;
    int remainingTicks;
    int textColor;
    int requiresSourceFont;
    /*
     * Concrete ReDMCSB material owners.  Presentation consumers must load
     * these GRAPHICS.DAT entries and zones; `text` is deliberately not a
     * rendering input for action/spell HUD feedback.
     */
    int requiredPrimaryGraphicId;
    int requiredSecondaryGraphicId;
    int requiredPrimaryZoneId;
    int requiredSecondaryZoneId;
    int requiredFontGraphicId;
    int requiresRealActionMenuLayout;
    int requiresRealSpellAreaLayout;
    int suppressSyntheticFallback;
    int printsLineFeed;
    int printsChampionName;
    int appendsBaseSkillName;
    int clearsSymbolsOnCastClick;
    int redrawsAvailableSymbols;
    int redrawsChampionSymbols;
    uint32_t sourceTick;
    uint32_t serial;
    /* Retained only for ABI compatibility with older diagnostic callers.
     * Source-owned action/spell presentation always leaves this empty. */
    char text[DM1_V1_ACTION_HUD_TEXT_CAPACITY_PC34];
    const char *messageBeforeSkill;
    const char *messageAfterSkill;
    const char *sourceAnchor;
} DM1_V1_ActionSpellHudPresentationReceiptPc34;

void dm1_v1_live_action_effects_reset_pc34(DM1_V1_LiveActionEffectsPc34 *effects);
int dm1_v1_live_action_effect_materialize_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    const DM1_V1_LiveActionEffectInputPc34 *input,
    DM1_V1_LiveActionEffectReceiptPc34 *outReceipt);
int dm1_v1_live_action_effects_advance_pc34(
    DM1_V1_LiveActionEffectsPc34 *effects,
    uint32_t tick,
    DM1_V1_LiveActionEffectsAdvancePlanPc34 *outPlan);
int dm1_v1_live_action_effect_hud_presentation_pc34(
    const DM1_V1_LiveActionEffectPc34 *effect,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *outReceipt);
int dm1_v1_live_action_spell_failure_hud_presentation_f0412_pc34(
    const DM1_V1_SpellFailureHudFeedbackPc34 *feedback,
    int championIndex,
    DM1_V1_ActionSpellHudPresentationReceiptPc34 *outReceipt);
const char *dm1_v1_live_action_effects_source_evidence_pc34(void);

#endif

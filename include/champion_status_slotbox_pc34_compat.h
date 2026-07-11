#ifndef REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H
#define REDMCSB_CHAMPION_STATUS_SLOTBOX_PC34_COMPAT_H

typedef struct ChampionStatusSlotBoxCompat {
    unsigned int slotBoxIndex;
    unsigned int championIndex;
    unsigned int handSlot;
    unsigned int commandId;
    unsigned int zoneIndex;
    unsigned int left;
    unsigned int right;
    unsigned int top;
    unsigned int bottom;
    const char* evidence;
} ChampionStatusSlotBoxCompat;

typedef struct ChampionStatusNameBoxCompat {
    unsigned int championIndex;
    unsigned int commandId;
    unsigned int left;
    unsigned int right;
    unsigned int top;
    unsigned int bottom;
    unsigned int fillWidth;
    unsigned int fillHeight;
    const char* evidence;
} ChampionStatusNameBoxCompat;

typedef struct ChampionStatusRedrawPlanCompat {
    unsigned int championIndex;
    unsigned int sourceAttributes;
    unsigned int isInventoryChampion;
    unsigned int currentHealth;
    unsigned int statusBoxRedrawn;
    unsigned int redrawNameTitle;
    unsigned int redrawStatistics;
    unsigned int redrawWounds;
    unsigned int redrawActionHand;
    unsigned int redrawViewport;
    unsigned int drawDeadStatusBox;
    unsigned int drawActionIconBeforeClear;
    unsigned int clearAllDirtyFlagsAtEnd;
    const char* evidence;
} ChampionStatusRedrawPlanCompat;

typedef struct ChampionStatusRectCompat {
    int x;
    int y;
    int w;
    int h;
} ChampionStatusRectCompat;

enum {
    CHAMPION_STATUS_COMPAT_CHAMPION_COUNT = 4,
    CHAMPION_STATUS_COMPAT_STATUS_BOX_ZONE_ID_BASE = 151,
    CHAMPION_STATUS_COMPAT_STATUS_NAME_CLEAR_ZONE_ID_BASE = 159,
    CHAMPION_STATUS_COMPAT_STATUS_NAME_TEXT_ZONE_ID_BASE = 163,
    CHAMPION_STATUS_COMPAT_BAR_GRAPH_ZONE_ID_BASE = 187,
    CHAMPION_STATUS_COMPAT_BAR_VALUE_ZONE_ID_BASE = 195,
    CHAMPION_STATUS_COMPAT_DAMAGE_ZONE_ID_BASE = 167,
    CHAMPION_STATUS_COMPAT_INVENTORY_DAMAGE_ZONE_ID_BASE = 179,
    CHAMPION_STATUS_COMPAT_HAND_PARENT_ZONE_ID_BASE = 207,
    CHAMPION_STATUS_COMPAT_HAND_ZONE_ID_BASE = 211,
    CHAMPION_STATUS_COMPAT_SLOT_X_BASE = 0,
    CHAMPION_STATUS_COMPAT_SLOT_Y = 0,
    CHAMPION_STATUS_COMPAT_SLOT_W = 67,
    CHAMPION_STATUS_COMPAT_SLOT_H = 29,
    CHAMPION_STATUS_COMPAT_SLOT_STEP = 69,
    CHAMPION_STATUS_COMPAT_NAME_CLEAR_W = 43,
    CHAMPION_STATUS_COMPAT_NAME_CLEAR_H = 7,
    CHAMPION_STATUS_COMPAT_NAME_TEXT_X = 1,
    CHAMPION_STATUS_COMPAT_NAME_TEXT_W = 42,
    CHAMPION_STATUS_COMPAT_HAND_READY_X = 4,
    CHAMPION_STATUS_COMPAT_HAND_ACTION_X = 24,
    CHAMPION_STATUS_COMPAT_HAND_Y = 10,
    CHAMPION_STATUS_COMPAT_HAND_W = 16,
    CHAMPION_STATUS_COMPAT_HAND_H = 16,
    CHAMPION_STATUS_COMPAT_HAND_ICON_INSET = 1,
    CHAMPION_STATUS_COMPAT_HAND_SLOT_BOX_W = 18,
    CHAMPION_STATUS_COMPAT_HAND_SLOT_BOX_H = 18,
    CHAMPION_STATUS_COMPAT_BAR_REGION_X = 43,
    CHAMPION_STATUS_COMPAT_BAR_REGION_W = 24,
    CHAMPION_STATUS_COMPAT_BAR_REGION_H = 29,
    CHAMPION_STATUS_COMPAT_BAR_CONTAINER_W = 4,
    CHAMPION_STATUS_COMPAT_BAR_CONTAINER_H = 25,
    CHAMPION_STATUS_COMPAT_BAR_HP_CX = 5,
    CHAMPION_STATUS_COMPAT_BAR_STAMINA_CX = 12,
    CHAMPION_STATUS_COMPAT_BAR_MANA_CX = 19,
    CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SHIELD = 37,
    CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_FIRESHIELD = 38,
    CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SPELLSHIELD = 39,
    CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX = 7,
    CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX_DEAD = 8,
    CHAMPION_STATUS_COMPAT_COLOR_DARK_GRAY_CLEAR = 1,
    CHAMPION_STATUS_COMPAT_COLOR_GOLD = 9,
    CHAMPION_STATUS_COMPAT_COLOR_YELLOW = 11,
    CHAMPION_STATUS_COMPAT_COLOR_DARKEST_GRAY = 12,
    CHAMPION_STATUS_COMPAT_COLOR_LIGHTEST_GRAY = 13
};

static inline int CHAMPION_Compat_StatusBoxZoneId(int championSlot) {
    if (championSlot < 0 ||
        championSlot >= CHAMPION_STATUS_COMPAT_CHAMPION_COUNT) {
        return 0;
    }
    return CHAMPION_STATUS_COMPAT_STATUS_BOX_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_StatusBoxZone(int championSlot,
                                                ChampionStatusRectCompat* out) {
    if (!out || !CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    out->x = CHAMPION_STATUS_COMPAT_SLOT_X_BASE +
             championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP;
    out->y = CHAMPION_STATUS_COMPAT_SLOT_Y;
    out->w = CHAMPION_STATUS_COMPAT_SLOT_W;
    out->h = CHAMPION_STATUS_COMPAT_SLOT_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusBarGraphZoneId(int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_BAR_GRAPH_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_StatusBarGraphRegionZone(
    int championSlot,
    ChampionStatusRectCompat* out) {
    if (!out || !CHAMPION_Compat_StatusBarGraphZoneId(championSlot)) {
        return 0;
    }
    out->x = championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP +
             CHAMPION_STATUS_COMPAT_BAR_REGION_X;
    out->y = CHAMPION_STATUS_COMPAT_SLOT_Y;
    out->w = CHAMPION_STATUS_COMPAT_BAR_REGION_W;
    out->h = CHAMPION_STATUS_COMPAT_BAR_REGION_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusBarZoneId(int statIndex) {
    if (statIndex < 0 || statIndex > 2) return 0;
    return CHAMPION_STATUS_COMPAT_BAR_VALUE_ZONE_ID_BASE + statIndex * 4;
}

static inline int CHAMPION_Compat_StatusBarValueZoneId(int championSlot,
                                                       int statIndex) {
    if (!CHAMPION_Compat_StatusBarGraphZoneId(championSlot) ||
        !CHAMPION_Compat_StatusBarZoneId(statIndex)) {
        return 0;
    }
    return CHAMPION_Compat_StatusBarZoneId(statIndex) + championSlot;
}

static inline int CHAMPION_Compat_StatusBarX(int statIndex) {
    static const int kCenters[3] = {
        CHAMPION_STATUS_COMPAT_BAR_HP_CX,
        CHAMPION_STATUS_COMPAT_BAR_STAMINA_CX,
        CHAMPION_STATUS_COMPAT_BAR_MANA_CX
    };
    if (statIndex < 0 || statIndex > 2) return -1;
    return CHAMPION_STATUS_COMPAT_BAR_REGION_X +
           kCenters[statIndex] -
           CHAMPION_STATUS_COMPAT_BAR_CONTAINER_W / 2;
}

static inline int CHAMPION_Compat_StatusBarZone(int championSlot,
                                                int statIndex,
                                                ChampionStatusRectCompat* out) {
    int x;
    if (!out ||
        !CHAMPION_Compat_StatusBarValueZoneId(championSlot, statIndex)) {
        return 0;
    }
    x = CHAMPION_Compat_StatusBarX(statIndex);
    if (x < 0) return 0;
    out->x = championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP + x;
    out->y = CHAMPION_STATUS_COMPAT_SLOT_Y;
    out->w = CHAMPION_STATUS_COMPAT_BAR_CONTAINER_W;
    out->h = CHAMPION_STATUS_COMPAT_BAR_CONTAINER_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusHandParentZoneId(int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_HAND_PARENT_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_StatusHandZoneId(int championSlot,
                                                   int handIndex) {
    if (!CHAMPION_Compat_StatusHandParentZoneId(championSlot) ||
        handIndex < 0 || handIndex > 1) {
        return 0;
    }
    return CHAMPION_STATUS_COMPAT_HAND_ZONE_ID_BASE + championSlot * 2 +
           handIndex;
}

static inline int CHAMPION_Compat_StatusHandZone(int championSlot,
                                                 int handIndex,
                                                 ChampionStatusRectCompat* out) {
    if (!out || !CHAMPION_Compat_StatusHandZoneId(championSlot, handIndex)) {
        return 0;
    }
    out->x = championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP +
             (handIndex ? CHAMPION_STATUS_COMPAT_HAND_ACTION_X
                        : CHAMPION_STATUS_COMPAT_HAND_READY_X);
    out->y = CHAMPION_STATUS_COMPAT_HAND_Y;
    out->w = CHAMPION_STATUS_COMPAT_HAND_W;
    out->h = CHAMPION_STATUS_COMPAT_HAND_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusHandIconZone(
    int championSlot,
    int handIndex,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat hand;
    if (!out ||
        !CHAMPION_Compat_StatusHandZone(championSlot, handIndex, &hand)) {
        return 0;
    }
    out->x = hand.x + CHAMPION_STATUS_COMPAT_HAND_ICON_INSET;
    out->y = hand.y + CHAMPION_STATUS_COMPAT_HAND_ICON_INSET;
    out->w = 16;
    out->h = 16;
    return 1;
}

static inline int CHAMPION_Compat_StatusHandSlotBoxZone(
    int championSlot,
    int handIndex,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat hand;
    if (!out ||
        !CHAMPION_Compat_StatusHandZone(championSlot, handIndex, &hand)) {
        return 0;
    }
    out->x = hand.x;
    out->y = hand.y;
    out->w = CHAMPION_STATUS_COMPAT_HAND_SLOT_BOX_W;
    out->h = CHAMPION_STATUS_COMPAT_HAND_SLOT_BOX_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusNameClearZoneId(int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_STATUS_NAME_CLEAR_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_StatusNameTextZoneId(int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_STATUS_NAME_TEXT_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_StatusNameZone(int championSlot,
                                                 ChampionStatusRectCompat* out) {
    if (!out || !CHAMPION_Compat_StatusNameClearZoneId(championSlot)) {
        return 0;
    }
    out->x = championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP;
    out->y = CHAMPION_STATUS_COMPAT_SLOT_Y;
    out->w = CHAMPION_STATUS_COMPAT_NAME_CLEAR_W;
    out->h = CHAMPION_STATUS_COMPAT_NAME_CLEAR_H;
    return 1;
}

static inline int CHAMPION_Compat_StatusNameTextZone(
    int championSlot,
    ChampionStatusRectCompat* out) {
    if (!out || !CHAMPION_Compat_StatusNameTextZoneId(championSlot)) {
        return 0;
    }
    out->x = championSlot * CHAMPION_STATUS_COMPAT_SLOT_STEP +
             CHAMPION_STATUS_COMPAT_NAME_TEXT_X;
    out->y = CHAMPION_STATUS_COMPAT_SLOT_Y;
    out->w = CHAMPION_STATUS_COMPAT_NAME_TEXT_W;
    out->h = CHAMPION_STATUS_COMPAT_NAME_CLEAR_H;
    return 1;
}

static inline int CHAMPION_Compat_DamageIndicatorZoneId(int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_DAMAGE_ZONE_ID_BASE + championSlot;
}

static inline int CHAMPION_Compat_DamageIndicatorZone(
    int championSlot,
    int indicatorW,
    int indicatorH,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat box;
    if (!out || indicatorW <= 0 || indicatorH <= 0 ||
        !CHAMPION_Compat_DamageIndicatorZoneId(championSlot) ||
        !CHAMPION_Compat_StatusBoxZone(championSlot, &box)) {
        return 0;
    }
    out->x = box.x + (box.w - indicatorW) / 2;
    out->y = box.y + (box.h - indicatorH) / 2;
    out->w = indicatorW;
    out->h = indicatorH;
    return 1;
}

static inline int CHAMPION_Compat_InventoryDamageIndicatorZoneId(
    int championSlot) {
    if (!CHAMPION_Compat_StatusBoxZoneId(championSlot)) return 0;
    return CHAMPION_STATUS_COMPAT_INVENTORY_DAMAGE_ZONE_ID_BASE +
           championSlot;
}

static inline int CHAMPION_Compat_InventoryDamageIndicatorZone(
    int championSlot,
    int indicatorW,
    int indicatorH,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat box;
    if (!out || indicatorW <= 0 || indicatorH <= 0 ||
        !CHAMPION_Compat_InventoryDamageIndicatorZoneId(championSlot) ||
        !CHAMPION_Compat_StatusBoxZone(championSlot, &box)) {
        return 0;
    }
    out->x = box.x + 7;
    out->y = box.y;
    out->w = indicatorW;
    out->h = indicatorH;
    return 1;
}

static inline int CHAMPION_Compat_DamageNumberOrigin(
    int championSlot,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat box;
    ChampionStatusRectCompat damage;
    if (!out ||
        !CHAMPION_Compat_StatusBoxZone(championSlot, &box) ||
        !CHAMPION_Compat_DamageIndicatorZone(championSlot, 45, 7,
                                             &damage)) {
        return 0;
    }
    out->x = box.x + box.w / 2 - 4;
    out->y = damage.y;
    out->w = 0;
    out->h = 0;
    return 1;
}

static inline int CHAMPION_Compat_DamageNumberOriginPc34(
    int championSlot,
    int damageAmount,
    int inventoryChampion,
    ChampionStatusRectCompat* out) {
    ChampionStatusRectCompat box;
    int digitOffset;
    if (!out || damageAmount <= 0 ||
        !CHAMPION_Compat_StatusBoxZone(championSlot, &box)) {
        return 0;
    }
    if (damageAmount < 10) {
        digitOffset = inventoryChampion ? 21 : 19;
    } else if (damageAmount < 100) {
        digitOffset = inventoryChampion ? 18 : 16;
    } else {
        digitOffset = inventoryChampion ? 15 : 13;
    }
    out->x = box.x + digitOffset;
    out->y = box.y + (inventoryChampion ? 16 : 5);
    out->w = 0;
    out->h = 0;
    return 1;
}

static inline int CHAMPION_Compat_StatusShieldBorderGraphics(
    int fireShieldDefense,
    int spellShieldDefense,
    int partyShieldDefense,
    int outGraphics[3]) {
    int appended[3];
    int appendCount = 0;
    int drawCount = 0;
    int i;

    /* ReDMCSB CHAMDRAW.C F0292 lines 792-804 appends fire, spell,
     * party and then decrements BorderCount while drawing. */
    if (fireShieldDefense > 0) {
        appended[appendCount++] =
            CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_FIRESHIELD;
    }
    if (spellShieldDefense > 0) {
        appended[appendCount++] =
            CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SPELLSHIELD;
    }
    if (partyShieldDefense > 0) {
        appended[appendCount++] =
            CHAMPION_STATUS_COMPAT_GFX_BORDER_PARTY_SHIELD;
    }
    if (outGraphics) {
        for (i = appendCount - 1; i >= 0; --i) {
            outGraphics[drawCount++] = appended[i];
        }
    }
    return appendCount;
}

static inline int CHAMPION_Compat_StatusNameColor(int present,
                                                  int currentHealth,
                                                  int leader) {
    if (!present) return -1;
    /* ReDMCSB CHAMDRAW.C F0292: dead names use C13 lightest gray;
     * living leader uses C11 yellow; other living champions use C09 gold. */
    if (currentHealth == 0) {
        return CHAMPION_STATUS_COMPAT_COLOR_LIGHTEST_GRAY;
    }
    return leader ? CHAMPION_STATUS_COMPAT_COLOR_YELLOW
                  : CHAMPION_STATUS_COMPAT_COLOR_GOLD;
}

static inline int CHAMPION_Compat_StatusNameClearColor(void) {
    return CHAMPION_STATUS_COMPAT_COLOR_DARK_GRAY_CLEAR;
}

static inline int CHAMPION_Compat_StatusBoxFillColor(void) {
    return CHAMPION_STATUS_COMPAT_COLOR_DARKEST_GRAY;
}

static inline int CHAMPION_Compat_StatusBoxGraphicId(void) {
    return CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX;
}

static inline int CHAMPION_Compat_DeadStatusBoxGraphicId(void) {
    return CHAMPION_STATUS_COMPAT_GFX_STATUS_BOX_DEAD;
}

static inline int CHAMPION_Compat_StatusBoxBaseGraphic(int present,
                                                       int currentHealth) {
    if (!present) return 0;
    return currentHealth == 0
        ? CHAMPION_Compat_DeadStatusBoxGraphicId()
        : 0;
}

unsigned int CHAMPION_Compat_GetStatusSlotBoxCount(void);
int CHAMPION_Compat_GetStatusSlotBox(unsigned int slotBoxIndex, ChampionStatusSlotBoxCompat* outBox);
const char* CHAMPION_Compat_GetStatusSlotBoxEvidence(void);

unsigned int CHAMPION_Compat_GetStatusNameBoxCount(void);
int CHAMPION_Compat_GetStatusNameBox(unsigned int championIndex, ChampionStatusNameBoxCompat* outBox);
const char* CHAMPION_Compat_GetStatusNameBoxEvidence(void);

int CHAMPION_Compat_GetStatusRedrawPlan(unsigned int championIndex,
                                        unsigned int attributes,
                                        unsigned int isInventoryChampion,
                                        unsigned int currentHealth,
                                        ChampionStatusRedrawPlanCompat* outPlan);
const char* CHAMPION_Compat_GetStatusRedrawPlanEvidence(void);

#endif

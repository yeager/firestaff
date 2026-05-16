#ifndef FIRESTAFF_CAMPAIGN_M12_H
#define FIRESTAFF_CAMPAIGN_M12_H

/*
 * Campaign Mode — M12 launcher feature.
 *
 * Manages campaign save slots with per-slot metadata: campaign name,
 * creation/modification dates, selected game ID, dungeon progress
 * (current level, monsters defeated by type), and play time.
 *
 * Campaign data is stored as JSON files in dataDir/campaigns/.
 * Each slot maps to campaign-{0..N}.json.
 *
 * Lifecycle:
 *   1. M12_Campaign_Init()    — reset state
 *   2. M12_Campaign_Scan()    — discover existing campaigns on disk
 *   3. HandleInput / UI       — slot selection, create, delete, load
 *   4. M12_Campaign_Save()    — persist the active campaign to disk
 */

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Limits ──────────────────────────────────────────────────────── */
#define CAMPAIGN_MAX_SLOTS       8
#define CAMPAIGN_NAME_MAX        64
#define CAMPAIGN_GAME_ID_MAX     32
#define CAMPAIGN_PATH_MAX        512
#define CAMPAIGN_VISIBLE_SLOTS   6

/* ── Monster type tracking (mirrors bestiary categories) ─────────── */
typedef enum {
    M12_CAMPAIGN_MONSTER_HUMANOID = 0,
    M12_CAMPAIGN_MONSTER_UNDEAD,
    M12_CAMPAIGN_MONSTER_BEAST,
    M12_CAMPAIGN_MONSTER_CONSTRUCT,
    M12_CAMPAIGN_MONSTER_DEMON,
    M12_CAMPAIGN_MONSTER_DRAGON,
    M12_CAMPAIGN_MONSTER_TYPE_COUNT
} M12_CampaignMonsterType;

/* ── Campaign slot status ────────────────────────────────────────── */
typedef enum {
    CAMPAIGN_SLOT_EMPTY = 0,
    CAMPAIGN_SLOT_ACTIVE,
    CAMPAIGN_SLOT_COMPLETED,
    CAMPAIGN_SLOT_CORRUPT
} M12_CampaignSlotStatus;

/* ── Sub-view for the campaign UI ────────────────────────────────── */
typedef enum {
    CAMPAIGN_VIEW_SLOT_LIST = 0,  /* Slot selection grid          */
    CAMPAIGN_VIEW_DETAIL,         /* Selected slot detail/actions  */
    CAMPAIGN_VIEW_CREATE,         /* Name entry for new campaign   */
    CAMPAIGN_VIEW_CONFIRM_DELETE  /* Delete confirmation prompt    */
} M12_CampaignView;

/* ── Progress snapshot per dungeon level ─────────────────────────── */
typedef struct {
    int levelReached;                   /* Highest dungeon level visited   */
    int levelsCleared;                  /* Levels fully explored           */
    int totalMonsterKills;              /* Aggregate kills                 */
    int killsByType[M12_CAMPAIGN_MONSTER_TYPE_COUNT]; /* Per-type kills    */
    int itemsCollected;                 /* Unique items picked up          */
    int secretsFound;                   /* Hidden areas discovered         */
    int championsRecruited;             /* Party members acquired          */
    int deathCount;                     /* Total party wipes               */
} M12_CampaignProgress;

/* ── Single campaign slot ────────────────────────────────────────── */
typedef struct {
    M12_CampaignSlotStatus status;
    char name[CAMPAIGN_NAME_MAX];           /* Player-chosen campaign name */
    char gameId[CAMPAIGN_GAME_ID_MAX];      /* "dm1", "csb", "dm2", etc.  */
    char filePath[CAMPAIGN_PATH_MAX];       /* Full path to campaign JSON  */
    time_t createdAt;                       /* First creation timestamp    */
    time_t modifiedAt;                      /* Last save timestamp         */
    int    playTimeSeconds;                 /* Accumulated play time       */
    M12_CampaignProgress progress;
} M12_CampaignSlot;

/* ── Campaign state (owned by the menu system) ───────────────────── */
typedef struct {
    M12_CampaignSlot slots[CAMPAIGN_MAX_SLOTS];
    int  slotCount;             /* Number of discovered/created slots  */
    int  selectedIndex;         /* Currently highlighted slot          */
    int  scrollOffset;          /* First visible slot in the list      */
    M12_CampaignView view;      /* Current sub-view                    */
    int  scanned;               /* 1 after initial disk scan           */

    /* Name entry state (for CAMPAIGN_VIEW_CREATE) */
    char newName[CAMPAIGN_NAME_MAX];
    int  newNameLen;
    int  newGameIndex;          /* Index into game list (0=DM1, etc.)  */
} M12_CampaignState;

/* ── Initialization ──────────────────────────────────────────────── */

/** Reset all campaign state to zeroes. */
void M12_Campaign_Init(M12_CampaignState* state);

/* ── Disk I/O ────────────────────────────────────────────────────── */

/**
 * Scan dataDir/campaigns/ for campaign-*.json files and populate
 * the slot list.  Returns the number of slots found.
 */
int M12_Campaign_Scan(M12_CampaignState* state, const char* dataDir);

/**
 * Save the given slot to its campaign JSON file.
 * Returns 0 on success, -1 on error.
 */
int M12_Campaign_Save(const M12_CampaignSlot* slot);

/**
 * Create a new campaign in the first empty slot.
 * Writes the initial JSON file to dataDir/campaigns/.
 * Returns the slot index on success, -1 if full or on error.
 */
int M12_Campaign_Create(M12_CampaignState* state, const char* dataDir,
                        const char* name, const char* gameId);

/**
 * Delete the campaign at the given slot index.
 * Removes the JSON file from disk and clears the slot.
 * Returns 0 on success, -1 on error.
 */
int M12_Campaign_Delete(M12_CampaignState* state, int slotIndex);

/* ── Menu input handling ─────────────────────────────────────────── */

/**
 * Handle menu navigation input within the campaign UI.
 * Returns 1 if a campaign was selected for launch (caller reads
 * slots[selectedIndex]).
 */
int M12_Campaign_HandleInput(M12_CampaignState* state, int input);

/* ── Name entry helpers ──────────────────────────────────────────── */

/** Append a printable character to the new campaign name. */
void M12_Campaign_NameAppend(M12_CampaignState* state, char ch);

/** Remove the last character from the new campaign name. */
void M12_Campaign_NameBackspace(M12_CampaignState* state);

/* ── Query ───────────────────────────────────────────────────────── */

/** Get the currently selected slot, or NULL if none. */
const M12_CampaignSlot* M12_Campaign_GetSelected(
    const M12_CampaignState* state);

/** Format play time as "HH:MM:SS" into outBuf (at least 9 bytes). */
void M12_Campaign_FormatPlayTime(int totalSeconds, char* outBuf, int outSize);

/** Return a human-readable label for a monster type enum. */
const char* M12_Campaign_MonsterTypeName(M12_CampaignMonsterType type);

/** Return the game ID string for a game index (0=dm1, 1=csb, etc.). */
const char* M12_Campaign_GameIdByIndex(int index);

/** Return a human-readable label for a slot status. */
const char* M12_Campaign_SlotStatusLabel(M12_CampaignSlotStatus status);

/** Return the number of available (empty) slots. */
int M12_Campaign_EmptySlotCount(const M12_CampaignState* state);

/* ── Progress update (called by engine during gameplay) ──────────── */

/**
 * Record a monster kill of the given type in the active campaign.
 * Increments both per-type and total counters.
 */
void M12_Campaign_RecordKill(M12_CampaignSlot* slot,
                             M12_CampaignMonsterType type);

/**
 * Update the highest dungeon level reached if newLevel exceeds
 * the current record.
 */
void M12_Campaign_UpdateLevel(M12_CampaignSlot* slot, int newLevel);

/**
 * Add elapsed seconds to the campaign's play time counter.
 */
void M12_Campaign_AddPlayTime(M12_CampaignSlot* slot, int seconds);

/* ── Rendering ───────────────────────────────────────────────────── */

/**
 * Draw the campaign mode UI into a framebuffer (text-mode stub).
 * The modern renderer handles the actual visual layout.
 */
void M12_Campaign_Draw(const M12_CampaignState* state,
                       unsigned char* fb, int fbWidth, int fbHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CAMPAIGN_M12_H */

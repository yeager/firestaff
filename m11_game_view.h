#ifndef FIRESTAFF_M11_GAME_VIEW_H
#define FIRESTAFF_M11_GAME_VIEW_H

#include "menu_startup_m12.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_magic_pc34_compat.h"
#include "asset_loader_m11.h"
#include "audio_sdl_m11.h"
#include "font_m11.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    M11_GAME_VIEW_PATH_CAPACITY = 512
};

typedef enum {
    M11_GAME_INPUT_IGNORED = 0,
    M11_GAME_INPUT_REDRAW = 1,
    M11_GAME_INPUT_RETURN_TO_MENU = 2
} M11_GameInputResult;

typedef enum {
    M11_GAME_SOURCE_BUILTIN_CATALOG = 0,
    M11_GAME_SOURCE_CUSTOM_DUNGEON,
    M11_GAME_SOURCE_DIRECT_DUNGEON
} M11_GameSourceKind;

typedef struct {
    const char* title;
    const char* gameId;
    const char* dataDir;
    const char* sourceId;
    const char* dungeonPath;
    int languageIndex;
    int rendererBackend;
    M11_GameSourceKind sourceKind;
} M11_GameLaunchSpec;

enum {
    M11_RUNTIME_CATALOG_MAX_ENTRIES = 1024,
    M11_RUNTIME_CATALOG_MSGID_CAPACITY = 96,
    M11_RUNTIME_CATALOG_MSGSTR_CAPACITY = 128
};

typedef struct {
    char msgid[M11_RUNTIME_CATALOG_MSGID_CAPACITY];
    char msgstr[M11_RUNTIME_CATALOG_MSGSTR_CAPACITY];
} M11_RuntimeCatalogEntry;

typedef struct {
    M11_RuntimeCatalogEntry entries[M11_RUNTIME_CATALOG_MAX_ENTRIES];
    int entryCount;
    int loaded;
} M11_RuntimeCatalog;

enum {
    M11_MESSAGE_LOG_CAPACITY = 6,
    M11_MESSAGE_MAX_LENGTH = 80
};

typedef struct {
    char text[M11_MESSAGE_MAX_LENGTH];
    unsigned char color;
} M11_LogEntry;

typedef struct {
    M11_LogEntry entries[M11_MESSAGE_LOG_CAPACITY];
    int writeIndex;
    int count;
} M11_MessageLog;

typedef struct {
    int active;
    int startedFromLauncher;
    char title[64];
    char sourceId[32];
    M11_GameSourceKind sourceKind;
    char dungeonPath[M11_GAME_VIEW_PATH_CAPACITY];
    char lastAction[32];
    char lastOutcome[64];
    char inspectTitle[64];
    char inspectDetail[128];
    /* Pass 42: V1-chrome-mode reroute bookkeeping.  Last payload
     * pushed into the message log from m11_set_status /
     * m11_set_inspect_readout, used to suppress back-to-back
     * duplicate pushes (the renderer may call the setters with
     * identical strings across frames).  Not part of the save
     * format; zero-initialised by memset in M11_GameView_Init. */
    char chromeRerouteLastStatus[96];
    char chromeRerouteLastInspect[128];
    uint32_t lastWorldHash;
    struct TickResult_Compat lastTickResult;
    struct GameWorld_Compat world;
    struct ChampionMirrorCatalog_Compat mirrorCatalog;
    int mirrorCatalogAvailable;
    /* Source-backed champion mirror candidate panel.  Mirrors the
     * G0299_ui_CandidateChampionOrdinal + M568_PANEL_RESURRECT_REINCARNATE
     * flow at M11 state level: selecting a front-cell mirror records
     * its DUNGEON.DAT mirror ordinal, then confirm/cancel consumes it. */
    int candidateMirrorOrdinal;
    int candidateMirrorPanelActive;
    M11_MessageLog messageLog;
    int resting;
    int partyDead;
    uint32_t exploredBits[32]; /* 32 * 32 = 1024 cells tracked per level */

    /* Asset loader for GRAPHICS.DAT-backed rendering */
    M11_AssetLoader assetLoader;
    int assetsAvailable; /* 1 if assetLoader is ready */
    M11_FontState originalFont; /* DM1 font from GRAPHICS.DAT */
    int originalFontAvailable;
    M11_AudioState audioState;
    int audioEventCount;

    /* ── Per-map ornament index cache ──
     * In DM1, each map has a per-map wall/floor/door ornament index
     * table stored in the DUNGEON.DAT metadata area.  The ordinal
     * from the sensor thing is looked up in this table to get the
     * actual ornament graphic index.  We cache these per-map tables
     * here, loaded lazily from DUNGEON.DAT the first time a map is
     * visited.  Ref: ReDMCSB G0261_auc_CurrentMapWallOrnamentIndices. */
    int wallOrnamentIndices[32][16];   /* [mapIndex][ordinal] -> graphic index */
    int floorOrnamentIndices[32][16];  /* [mapIndex][ordinal] -> graphic index */
    int doorOrnamentIndices[32][16];
    int ornamentCacheLoaded[32];       /* 1 if loaded for this map */

    /* Torch fuel burn-down tracking.
     * Each weapon index that is a lit torch has its remaining fuel
     * tracked here.  Fuel decreases by 1 each game tick.  When it
     * reaches 0, the torch is extinguished.  A torch's light
     * contribution is scaled by its fuel fraction. */
    enum { M11_TORCH_FUEL_CAPACITY = 256 };
    int torchFuel[256];          /* remaining fuel per weapon index */
    int torchFuelInitialized[256]; /* 1 if fuel has been set for this index */

    /* Spell casting UI state */
    int spellPanelOpen;          /* 1 when rune entry panel is visible */
    int spellRuneRow;            /* current rune row (0..3) = power/element/form/class */
    struct RuneSequence_Compat spellBuffer; /* runes entered so far */

    /* ── Creature animation state ── */
    /* Global animation tick — incremented each game tick, drives all
     * creature frame cycling and attack flash timers. */
    uint32_t animTick;

    /* Damage-flash timer.  Set to M11_DAMAGE_FLASH_DURATION when the
     * party takes creature melee damage.  Decremented each tick.
     * While > 0, the viewport border flashes red. */
    int damageFlashTimer;

    /* Per-champion damage indicator state.
     * When a champion takes damage, the corresponding timer is set to
     * M11_DAMAGE_FLASH_DURATION and the amount is recorded.  While > 0,
     * the GRAPHICS.DAT damage-to-champion overlay (graphic 15, 45×7)
     * is drawn on top of the champion's status box with the damage
     * number.  Ref: ReDMCSB CHAMPION.C F0291. */
    int championDamageTimer[4];  /* per-slot countdown */
    int championDamageAmount[4]; /* last damage dealt */

    /* Front-cell attack indicator timer.  Set to M11_ATTACK_CUE_DURATION
     * when a creature in the front cell (depth 0) attacks.  Decremented
     * each tick.  While > 0, draw slash-mark overlay on the viewport. */
    int attackCueTimer;

    /* Creature type that last attacked (for attack-cue sprite). */
    int attackCueCreatureType;

    /* Creature-hit overlay state (GRAPHICS.DAT graphic 14).
     * When the party deals melee damage to a creature, this timer is set
     * and the damage amount recorded.  While > 0, graphic 14 is drawn
     * centered on the viewport with the damage number overlaid.
     * Ref: ReDMCSB MELEE.C — C014_GRAPHIC_DAMAGE_TO_CREATURE. */
    int creatureHitOverlayTimer;
    int creatureHitDamageAmount;

    /* ── Full-screen overlay state ── */
    int mapOverlayActive;        /* 1 when full-screen map is displayed */
    int inventoryPanelActive;    /* 1 when full inventory grid is displayed */
    int inventorySelectedSlot;   /* currently highlighted slot index (-1 = none) */

    /* ── Endgame / dialog flow state ── */
    /* Set to 1 when ORCH_GAME_WON / EMIT_GAME_WON fires.  Blocks all
     * gameplay input; only ESC (return to menu) is accepted. */
    int gameWon;
    uint32_t gameWonTick;  /* tick when victory was detected */

    /* Dialog box overlay for text-plaque inspection.
     * When dialogOverlayActive is 1, a styled dialog panel is rendered
     * on top of the viewport showing dialogOverlayText.  The user
     * dismisses it with any key or click. */
    int dialogOverlayActive;
    char dialogOverlayText[128];
    int dialogChoiceCount;
    int dialogSelectedChoice;
    char dialogChoices[4][32];
    char localeCode[8];
    M11_RuntimeCatalog localizedCatalog;
    M11_RuntimeCatalog englishCatalog;

    /* V1 presentation mode: when showDebugHUD is 0 (default), the
     * in-game screen omits developer-facing metadata, keybinding
     * helpers, tick counters, and diagnostic square summaries.
     * Set to 1 via FIRESTAFF_DEBUG_HUD=1 environment variable. */
    int showDebugHUD;

    /* Acting-champion ordinal.  Mirrors DM1
     * G0506_ui_ActingChampionOrdinal exactly: 0 = no champion is
     * acting (idle action area with four action-hand icon cells,
     * F0387 icon-mode branch).  1..N = champion at index N-1 has
     * been activated by a click on their action-hand cell and the
     * action area is in menu mode (F0387 menu-mode branch,
     * rendering graphic 10 + champion name + up to three action
     * names from the item's ActionSet).
     *
     * Toggled by M11_GameView_HandlePointer when a click falls
     * inside an action-hand icon cell.  Cleared automatically
     * when the acting champion dies, is replaced, or the party
     * enters rest — matches F0388_MENUS_ClearActingChampion
     * semantics (the subset we currently model).
     *
     * Ref: ReDMCSB MENU.C F0389_MENUS_SetActingChampion,
     *      F0388_MENUS_ClearActingChampion,
     *      ACTIDRAW.C F0387_MENUS_DrawActionArea menu-mode branch. */
    unsigned int actingChampionOrdinal;
} M11_GameViewState;

/* Spell casting API */
int M11_GameView_OpenSpellPanel(M11_GameViewState* state);
int M11_GameView_CloseSpellPanel(M11_GameViewState* state);
int M11_GameView_EnterRune(M11_GameViewState* state, int symbolIndex);
int M11_GameView_CastSpell(M11_GameViewState* state);
int M11_GameView_ClearSpell(M11_GameViewState* state);

void M11_GameView_Init(M11_GameViewState* state);
void M11_GameView_Shutdown(M11_GameViewState* state);
int M11_GameView_Start(M11_GameViewState* state, const M11_GameLaunchSpec* spec);
int M11_GameView_OpenSelectedMenuEntry(M11_GameViewState* state,
                                       const M12_StartupMenuState* menuState);
int M11_GameView_StartDm1(M11_GameViewState* state, const char* dataDir);
int M11_GameView_GetQuickSavePath(const M11_GameViewState* state,
                                  char* out,
                                  size_t outSize);
int M11_GameView_QuickSave(M11_GameViewState* state);
int M11_GameView_QuickLoad(M11_GameViewState* state);
M11_GameInputResult M11_GameView_AdvanceIdleTick(M11_GameViewState* state);
M11_GameInputResult M11_GameView_HandleInput(M11_GameViewState* state,
                                             M12_MenuInput input);
M11_GameInputResult M11_GameView_HandlePointer(M11_GameViewState* state,
                                               int x,
                                               int y,
                                               int primaryButton);
void M11_GameView_Draw(const M11_GameViewState* state,
                       unsigned char* framebuffer,
                       int framebufferWidth,
                       int framebufferHeight);
int M11_GameView_PickupItem(M11_GameViewState* state);
int M11_GameView_DropItem(M11_GameViewState* state);
int M11_GameView_CountChampionItems(const M11_GameViewState* state, int championIndex);
void M11_MessageLog_Push(M11_MessageLog* log, const char* text, unsigned char color);
int M11_GameView_GetMessageLogCount(const M11_GameViewState* state);
const char* M11_GameView_GetMessageLogEntry(const M11_GameViewState* state, int reverseIndex);

/* Post-move environmental transition check (pits, teleporters).
 * Returns 1 if a transition occurred. */
int M11_GameView_CheckPostMoveTransitions(M11_GameViewState* state);

/* Query the computed skill level for a champion via the lifecycle layer.
 * Returns the effective level (>= 0), or -1 on error. */
int M11_GameView_GetSkillLevel(const M11_GameViewState* state,
                               int championIndex,
                               int skillIndex);

/* Use the item in the active champion's hand slot (potions, flasks).
 * Returns 1 if an item was consumed/used. */
int M11_GameView_UseItem(M11_GameViewState* state);

/* Process tick emissions: log events, award XP, apply level-ups.
 * Normally called internally after each tick advance; exposed for
 * probe-level verification of emission-driven XP integration. */
void M11_GameView_ProcessTickEmissions(M11_GameViewState* state);

/* Compute the party's current light level (0..255).
 * Combines magical light from FUL/MAGIC_TORCH spells with lit torch
 * items in hand slots.  Used by the viewport renderer for dynamic
 * depth dimming and exposed for probe-level verification. */
int M11_GameView_GetLightLevel(const M11_GameViewState* state);

/* Torch fuel constants.  INITIAL is the starting fuel for a freshly-lit
 * torch (approximately 1500 game ticks).  Each tick burns 1 unit. */
#define M11_TORCH_INITIAL_FUEL  1500
#define M11_FLAMITT_INITIAL_FUEL 2500

/* Query remaining fuel for a weapon index (0 if untracked or exhausted). */
int M11_GameView_GetTorchFuel(const M11_GameViewState* state, int weaponIndex);

/* Update torch fuel for all lit torches (called once per tick). */
void M11_GameView_UpdateTorchFuel(M11_GameViewState* state);

/* ── Creature animation API ── */

/* Durations in game ticks. */
#define M11_DAMAGE_FLASH_DURATION   4
#define M11_ATTACK_CUE_DURATION     3
#define M11_CREATURE_HIT_OVERLAY_DURATION 5  /* ticks to show graphic-14 overlay */
#define M11_CREATURE_ANIM_PERIOD    6  /* ticks per idle-frame cycle */

/* Advance the animation frame counter and decrement timers.
 * Called once per game tick (from AdvanceIdleTick). */
void M11_GameView_TickAnimation(M11_GameViewState* state);

/* Signal that the party just took creature melee damage.
 * Starts the damage-flash and attack-cue timers.
 * creatureType: type index of the attacking creature (for cue sprite). */
void M11_GameView_NotifyDamageFlash(M11_GameViewState* state,
                                    int creatureType);

/* Query animation state (for probes). */
int M11_GameView_GetDamageFlashTimer(const M11_GameViewState* state);
int M11_GameView_GetAttackCueTimer(const M11_GameViewState* state);

/* Trigger a per-champion damage indicator overlay (GRAPHICS.DAT graphic 15).
 * The indicator displays for M11_DAMAGE_FLASH_DURATION ticks. */
void M11_GameView_NotifyChampionDamage(M11_GameViewState* state,
                                       int championSlot,
                                       int damageAmount);
uint32_t M11_GameView_GetAnimTick(const M11_GameViewState* state);

/* Signal that the party just dealt melee damage to a creature.
 * Starts the graphic-14 viewport overlay timer. */
void M11_GameView_NotifyCreatureHit(M11_GameViewState* state,
                                    int damageAmount);
int M11_GameView_GetCreatureHitOverlayTimer(const M11_GameViewState* state);

/* Return the idle-animation frame index (0 or 1) for a given
 * creature type based on the current animTick. */
int M11_GameView_CreatureAnimFrame(const M11_GameViewState* state,
                                   int creatureType);

/* Return the attack-cue creature type (-1 if no active attack). */
int M11_GameView_GetAttackCueCreatureType(const M11_GameViewState* state);

/* ── Dialog / endgame query API ── */

/* Return 1 if the game has been won (Firestaff placed correctly). */
int M11_GameView_IsGameWon(const M11_GameViewState* state);

/* Return the tick at which the game was won (0 if not won). */
uint32_t M11_GameView_GetGameWonTick(const M11_GameViewState* state);

/* Return 1 if a dialog overlay is currently displayed. */
int M11_GameView_IsDialogOverlayActive(const M11_GameViewState* state);

/* Dismiss the dialog overlay if active.  Returns 1 if dismissed. */
int M11_GameView_DismissDialogOverlay(M11_GameViewState* state);

/* Show a dialog overlay with the given text.  Returns 1 on success. */
int M11_GameView_ShowDialogOverlay(M11_GameViewState* state,
                                   const char* text);

/* Show a dialog overlay with up to four source-style choice labels. */
int M11_GameView_ShowDialogOverlayChoices(M11_GameViewState* state,
                                          const char* text,
                                          const char* choice1,
                                          const char* choice2,
                                          const char* choice3,
                                          const char* choice4);

/* Return 1..4 after a choice is selected, or 0 if none was selected. */
int M11_GameView_GetDialogSelectedChoice(const M11_GameViewState* state);

/* ── Full-screen map overlay API ── */

/* Toggle the full-screen map overlay.  Returns 1 if now visible. */
int M11_GameView_ToggleMapOverlay(M11_GameViewState* state);

/* Return 1 if the map overlay is currently displayed. */
int M11_GameView_IsMapOverlayActive(const M11_GameViewState* state);

/* ── Full inventory panel API ── */

/* Toggle the full inventory panel.  Returns 1 if now visible. */
int M11_GameView_ToggleInventoryPanel(M11_GameViewState* state);

/* Return 1 if the inventory panel is currently displayed. */
int M11_GameView_IsInventoryPanelActive(const M11_GameViewState* state);

/* Return the currently selected inventory slot index (-1 = none). */
int M11_GameView_GetInventorySelectedSlot(const M11_GameViewState* state);

/* Return human-readable label for an inventory slot index. */
const char* M11_GameView_SlotName(int slotIndex);

/* ── Action-menu API (DM1 F0387 menu-mode) ── */

/* Return the currently acting champion ordinal (DM1 G0506).
 * 0 = no champion is acting (idle icon-cell mode).
 * 1..N = champion at index N-1 has been activated and the action
 * area is in menu mode. */
unsigned int M11_GameView_GetActingChampionOrdinal(const M11_GameViewState* state);

/* Set the acting champion by index (0..CHAMPION_MAX_PARTY-1).
 * Returns 1 on success, 0 if the champion slot is empty/dead/out
 * of range.  Mirrors F0389_MENUS_SetActingChampion for the bounded
 * subset we currently model (action-area state only; does not yet
 * emit the full champion-draw refresh or icon flag). */
int M11_GameView_SetActingChampion(M11_GameViewState* state, int championIndex);

/* Clear the acting champion, returning the action area to idle
 * icon-cell mode.  Mirrors F0388_MENUS_ClearActingChampion for
 * the bounded action-area subset. */
void M11_GameView_ClearActingChampion(M11_GameViewState* state);

/* Resolve the DM1 ActionSet indices for the currently acting
 * champion.  Returns the 3 action-name indices (0..43, or 255 =
 * C0xFF_ACTION_NONE) into outIndices[0..2].  Returns 1 when the
 * action area is in menu mode and the indices were filled,
 * 0 when idle (outIndices left untouched).
 *
 * Ref: ReDMCSB MENU.C G0489_as_Graphic560_ActionSets,
 *      F0389_MENUS_SetActingChampion action-set lookup. */
int M11_GameView_GetActingActionIndices(const M11_GameViewState* state,
                                        unsigned char outIndices[3]);

/* Look up the DM1 action name for an action index (0..43, or 255).
 * Returns an empty string for C0xFF_ACTION_NONE and a pointer to a
 * static string otherwise.  Verbatim names from
 * G0490_ac_Graphic560_ActionNames (ReDMCSB MENU.C). */
const char* M11_GameView_GetActionName(unsigned char actionIndex);

/* Trigger the DM1 action-row click pathway for the currently acting
 * champion.  Mirrors F0391_MENUS_DidClickTriggerAction for the
 * bounded V1 slice we currently model:
 *
 *   - Returns 0 immediately when no champion is acting or when the
 *     chosen row resolves to C0xFF_ACTION_NONE (DM1 aborts without
 *     clearing the menu so the player can pick a valid row).
 *   - Emits a player-facing "CHAMPION: ACTION" log line in cyan.
 *   - For melee-contact actions (CHOP, PUNCH, KICK, STAB, SWING,
 *     HIT, THRUST, SLASH, BASH, JAB, STUN, HACK, BERZERK, CLEAVE,
 *     MELEE) advances one CMD_ATTACK tick through the existing M10
 *     orchestrator, which resolves damage, creature hit overlays
 *     and combat emissions exactly as the keyboard strike path.
 *   - ALWAYS clears the acting champion at the end (F0391 /
 *     F0388_MENUS_ClearActingChampion semantics), closing the
 *     action menu and restoring idle icon-cell presentation.
 *
 * actionListIndex must be 0..2.  Returns 1 when a tick-level
 * action was committed, 0 otherwise (including the NONE-row early
 * exit).  Ref: ReDMCSB MENU.C F0391, F0407; ACTIDRAW.C F0387. */
int M11_GameView_TriggerActionRow(M11_GameViewState* state,
                                  int actionListIndex);

/* Probe-visibility helpers for DM1 projectile / spell action
 * downstream effects.  These let probes observe that action-menu
 * projectile rows (FIREBALL / LIGHTNING / DISPELL / INVOKE /
 * SHOOT / THROW) actually spawn a projectile into world.projectiles
 * without needing to wire a full ObjectInfo action-hand harness.
 *
 * GetProjectileCount returns the count of live entries in
 * GameWorld.projectiles (wraps world.projectiles.count).
 *
 * TriggerNonMeleeActionByIndex invokes the F0407-style handler
 * for a chosen action-name index (0..43) against the acting
 * champion, bypassing the ActionSet-from-hand resolution.  This
 * is a test helper that mirrors exactly the same dispatch path
 * M11_GameView_TriggerActionRow uses when F0391 selects a
 * non-melee action from the champion's action list: it emits
 * the "CHAMPION: ACTION" log line, runs the bounded V1 handler
 * (which may spawn a projectile, adjust MagicState, etc.),
 * advances a CMD_NONE tick, and clears the acting champion.
 * Melee-contact actions are not routed here (they require the
 * CMD_ATTACK orchestrator path, which already has coverage via
 * the row-click probe invariants).
 *
 * Returns 1 when the handler reported AL1245_B_ActionPerformed=
 * TRUE (i.e. the projectile was spawned, mana was deducted, or
 * a deterministic effect was applied), 0 otherwise.
 *
 * Ref: ReDMCSB MENU.C F0407_MENUS_IsActionPerformed. */
int M11_GameView_GetProjectileCount(const M11_GameViewState* state);
int M11_GameView_TriggerNonMeleeActionByIndex(M11_GameViewState* state,
                                              int championIndex,
                                              int actionIndex);

/* V1 projectile cycle probe hook: drive one tick of the V1
 * projectile advance over all live projectiles.  Normally invoked
 * from M11_GameView_ProcessTickEmissions each orchestrator tick;
 * exposed here so probes can deterministically step projectiles
 * without replaying the full orchestrator pipeline.  Calls
 * F0811_PROJECTILE_Advance_Compat per live slot, applies the new
 * state or despawns on impact (with explosion spawn + damage +
 * log cue as the bounded V1 slice).  Safe on empty lists. */
void M11_GameView_AdvanceProjectilesOnce(M11_GameViewState* state);

/* V1 explosion cycle probe hook: drive one tick of the V1 explosion
 * advance over all live explosion slots.  Normally invoked from
 * M11_GameView_ProcessTickEmissions each orchestrator tick right after
 * the projectile advance; exposed here so probes can deterministically
 * step explosion aftermath without replaying the orchestrator pipeline.
 * Calls F0822_EXPLOSION_Advance_Compat per live slot, applies its
 * champion/group damage actions, and either despawns (one-shot: fire-
 * ball, lightning) or commits the advanced state with the new
 * currentFrame / decayed attack (persistent: poison cloud, smoke).
 * Safe on empty lists. */
void M11_GameView_AdvanceExplosionsOnce(M11_GameViewState* state);

/* Probe shim for the internal m11_summarize_square_things helper so
 * probes can verify that runtime-only projectiles / explosions show
 * up in the viewport cell summary (this is what drives the sprite
 * being drawn as the projectile travels across cells). */
int M11_GameView_CountCellProjectiles(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);
int M11_GameView_CountCellExplosions(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY);

/* Return DM1 G0215 projectile scale units (out of 32) for a viewport
 * depth and relative sub-cell. Exposed so probes can pin D2/D3 source
 * scaling without depending on screenshot pixel dimensions. */
int M11_GameView_GetProjectileSourceScaleUnits(int depthIndex,
                                               int relativeCell);
int M11_GameView_GetProjectileAspectFirstNative(int aspectIndex);
unsigned int M11_GameView_GetProjectileAspectGraphicInfo(int aspectIndex);
int M11_GameView_GetProjectileAspectBitmapDelta(int aspectIndex, int relativeDir);
int M11_GameView_GetProjectileGraphicForAspect(int aspectIndex, int relativeDir);
int M11_GameView_GetProjectileAspectFlipFlags(int aspectIndex,
                                              int relativeDir,
                                              int relativeCell,
                                              int mapX,
                                              int mapY);

/* Resolve a dungeon thing type/subtype to its M612 viewport object
 * native graphic index using G0237 object-info -> G0209 object-aspect
 * source data. Returns 0 for unsupported inputs. */
unsigned int M11_GameView_GetObjectSpriteIndex(int thingType, int subtype);

/* Return DM1 G2030 object scale units for source object distance/cell
 * scale bucket 0..4. Out-of-range inputs clamp like the renderer. */
int M11_GameView_GetObjectSourceScaleUnits(int scaleIndex);
int M11_GameView_GetObjectSourceScaleIndex(int depthIndex, int relativeCell);
int M11_GameView_GetC2500ObjectZonePoint(int scaleIndex,
                                         int relativeCell,
                                         int* outX,
                                         int* outY);
int M11_GameView_GetC2900ProjectileZonePoint(int scaleIndex,
                                             int relativeCell,
                                             int* outX,
                                             int* outY);
int M11_GameView_GetWallSetGraphicIndex(int wallSet, int wallSet0GraphicIndex);
int M11_GameView_GetViewportRect(int* outX, int* outY, int* outW, int* outH);
int M11_GameView_GetObjectIconIndexForThing(const M11_GameViewState* state,
                                            unsigned short thingId);
int M11_GameView_GetC3200CreatureZonePoint(int coordSet,
                                           int depthIndex,
                                           int visibleCount,
                                           int slotIndex,
                                           int* outX,
                                           int* outY);
int M11_GameView_GetC3200CreatureSideZonePoint(int coordSet,
                                               int depthIndex,
                                               int sideHint,
                                               int visibleCount,
                                               int slotIndex,
                                               int* outX,
                                               int* outY);
void M11_GameView_GetObjectPileShiftIndices(int pileIndex,
                                            int* outXIndex,
                                            int* outYIndex);
int M11_GameView_GetObjectShiftValue(int shiftSet, int shiftIndex);
unsigned int M11_GameView_GetObjectAspectGraphicInfo(int aspectIndex);
int M11_GameView_GetObjectAspectCoordinateSet(int aspectIndex);
int M11_GameView_ObjectUsesFlipOnRight(int thingType, int subtype,
                                       int relativeCell);
int M11_GameView_GetCreaturePaletteChange(int depthPaletteIndex,
                                          int paletteIndex);

/* ── Creature aspect query API (for probes) ── */

/* Return the coordinate set index (0-10) for a creature type. */
int M11_GameView_GetCreatureCoordinateSet(int creatureType);

/* Return the transparent color index for a creature type. */
int M11_GameView_GetCreatureTransparentColor(int creatureType);

/* Return front-cell viewport placement for a creature duplicate using
 * original Graphic558 center/bottom coordinates.  Exposed for probe
 * verification of original-data-backed coordinate extraction. */
void M11_GameView_GetCreatureFrontSlotPoint(int coordSet,
                                            int depthIndex,
                                            int visibleCount,
                                            int slotIndex,
                                            int* outCenterX,
                                            int* outBottomY);

/* Return the front-pose GRAPHICS.DAT sprite index for a creature at a
 * given depth. Returns 0 for invalid inputs. */
unsigned int M11_GameView_GetCreatureSpriteForDepth(int creatureType, int depthIndex);

/* Return the GRAPHICS.DAT sprite index selected for an actual view using
 * creature direction vs party direction and optional attack state.
 * outMirror receives whether the side pose should be mirrored. */
unsigned int M11_GameView_GetCreatureSpriteForView(int creatureType,
                                                   int depthIndex,
                                                   int creatureDir,
                                                   int partyDir,
                                                   int attacking,
                                                   int* outMirror);

/* Query replacement color palette indices for a creature type.
 * Returns 1 if the creature uses replacement colors, 0 if not.
 * outReplDst9/outReplDst10 receive the target palette indices. */
int M11_GameView_GetCreatureReplacementColors(int creatureType,
                                               int* outReplDst9,
                                               int* outReplDst10);

/* Return the CREATURE_INFO GraphicInfo bitfield for a creature type.
 * Values are extracted verbatim from ReDMCSB
 * G0243_as_Graphic559_CreatureInfo[].GraphicInfo (DM1 PC v3.4).
 * Returns 0 for invalid creature types.
 * See m11_game_view.c for M11_CREATURE_GI_MASK_* bit definitions. */
unsigned int M11_GameView_GetCreatureGraphicInfo(int creatureType);

/* Boolean queries on the source-backed GraphicInfo table.  These mirror
 * the DEFS.H flags that control whether dedicated side/back/attack
 * bitmaps exist and whether the FRONT bitmap should be mirrored when
 * falling back. */
int M11_GameView_CreatureHasSideBitmap(int creatureType);
int M11_GameView_CreatureHasBackBitmap(int creatureType);
int M11_GameView_CreatureHasAttackBitmap(int creatureType);
int M11_GameView_CreatureHasFlipNonAttack(int creatureType);
int M11_GameView_CreatureHasFlipAttack(int creatureType);

/* Source-backed queries for MASK0x0003_ADDITIONAL,
 * MASK0x0080_SPECIAL_D2_FRONT, MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT,
 * MASK0x0400_FLIP_DURING_ATTACK, and the M052/M053 offset amplitude
 * fields of CREATURE_INFO.GraphicInfo.  Reference:
 * ReDMCSB DUNVIEW.C F097 (_LoadGraphics) native-bitmap allocation loop
 * and F1512-render offset computation, plus F0179_GROUP_GetCreature-
 * AspectUpdateTime (GROUP2.C) for runtime flip behaviour. */
int M11_GameView_GetCreatureAdditional(int creatureType);
int M11_GameView_CreatureHasSpecialD2Front(int creatureType);
int M11_GameView_CreatureHasD2FrontIsFlippedFront(int creatureType);
int M11_GameView_CreatureHasFlipDuringAttack(int creatureType);
int M11_GameView_GetCreatureMaxHorizontalOffset(int creatureType);
int M11_GameView_GetCreatureMaxVerticalOffset(int creatureType);

/* Total GRAPHICS.DAT native-bitmap slots F097_xxxx_DUNGEONVIEW_LoadGraphics
 * allocates for this creature in order:
 *   [Front] [Side?] [Back?] [SpecialD2?] [Attack?] [AdditionalFront x N?]
 * The SpecialD2 slot is present when MASK0x0080_SPECIAL_D2_FRONT is set
 * and MASK0x0100_SPECIAL_D2_FRONT_IS_FLIPPED_FRONT is clear (guarded by
 * the C06_COMPILE_DM10aEN..DM13bFR block in DUNVIEW.C; BUG0_00 notes
 * this slot is allocated but never read by the renderer).
 * Additional-front slots are only allocated when MASK0x0004_FLIP_NON_ATTACK
 * is clear. */
int M11_GameView_GetCreatureNativeBitmapCount(int creatureType);

/* Total derived-bitmap cache slots F460_xxxx_START_CalculateDerivedBitmap-
 * CacheSizes reserves for this creature (Front D3 + Front D2 always,
 * +2 for each pose with a dedicated bitmap, +3 per additional front). */
int M11_GameView_GetCreatureDerivedBitmapCount(int creatureType);

/* Return the floor ornament ordinal for a viewport cell position.
 * relForward/relSide are relative to party position/facing. */
int M11_GameView_GetFloorOrnamentOrdinal(const M11_GameViewState* state,
                                         int relForward, int relSide);

/* Source endgame title placement helper for probes.  Mirrors
 * ENDGAME.C:F0444_STARTEND_Endgame spacing: title starts after
 * Champion.Name, with an extra character gap unless the title begins
 * with ',', ';', or '-'. */
int M11_GameView_EndgameTitleXForSourceText(const char* name, const char* title);

int M11_GameView_GetMirrorCatalogCount(const M11_GameViewState* state);
int M11_GameView_GetMirrorNameByOrdinal(const M11_GameViewState* state,
                                        int mirrorOrdinal,
                                        char* outName,
                                        int outSize);
int M11_GameView_GetMirrorTitleByOrdinal(const M11_GameViewState* state,
                                         int mirrorOrdinal,
                                         char* outTitle,
                                         int outSize);
int M11_GameView_RecruitChampionByMirrorOrdinal(M11_GameViewState* state,
                                                int mirrorOrdinal);
int M11_GameView_RecruitChampionByMirrorName(M11_GameViewState* state,
                                             const char* name);
int M11_GameView_GetFrontMirrorOrdinal(const M11_GameViewState* state);
int M11_GameView_SelectFrontMirrorCandidate(M11_GameViewState* state);
int M11_GameView_ConfirmMirrorCandidate(M11_GameViewState* state,
                                        int reincarnate);
int M11_GameView_CancelMirrorCandidate(M11_GameViewState* state);
int M11_GameView_GetV1StatusNameColor(const M11_GameViewState* state,
                                      int championSlot);
int M11_GameView_GetV1StatusNameClearColor(void);
int M11_GameView_GetV1StatusBoxFillColor(void);
int M11_GameView_GetV1StatusBoxZoneId(int championSlot);
int M11_GameView_GetV1StatusBoxZone(int championSlot,
                                    int* outX,
                                    int* outY,
                                    int* outW,
                                    int* outH);
int M11_GameView_GetV1StatusNameClearZoneId(int championSlot);
int M11_GameView_GetV1StatusNameTextZoneId(int championSlot);
int M11_GameView_GetV1StatusNameZone(int championSlot,
                                     int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH);
int M11_GameView_GetV1StatusNameTextZone(int championSlot,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
int M11_GameView_GetV1StatusHandParentZoneId(int championSlot);
int M11_GameView_GetV1StatusHandZoneId(int championSlot,
                                       int handIndex);
int M11_GameView_GetV1StatusHandZone(int championSlot,
                                     int handIndex,
                                     int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH);
int M11_GameView_GetV1StatusHandIconZone(int championSlot,
                                         int handIndex,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
int M11_GameView_GetV1StatusHandSlotBoxZone(int championSlot,
                                            int handIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH);
int M11_GameView_GetV1StatusBarGraphZoneId(int championSlot);
int M11_GameView_GetV1StatusBarZoneId(int statIndex);
int M11_GameView_GetV1StatusBarValueZoneId(int championSlot,
                                           int statIndex);
int M11_GameView_GetV1StatusBarZone(int championSlot,
                                    int statIndex,
                                    int* outX,
                                    int* outY,
                                    int* outW,
                                    int* outH);
int M11_GameView_GetV1ChampionBarColor(int championSlot);
int M11_GameView_GetV1StatusBarBlankColor(void);
int M11_GameView_GetV1StatusHandSlotGraphic(const M11_GameViewState* state,
                                            int championSlot,
                                            int handIndex);
int M11_GameView_GetV1SlotBoxNormalGraphicId(void);
int M11_GameView_GetV1SlotBoxWoundedGraphicId(void);
int M11_GameView_GetV1SlotBoxActingHandGraphicId(void);
int M11_GameView_GetV1StatusHandIconIndex(const M11_GameViewState* state,
                                          int championSlot,
                                          int handIndex);
int M11_GameView_GetV1StatusShieldBorderGraphic(const M11_GameViewState* state);
int M11_GameView_GetV1PartyShieldBorderGraphicId(void);
int M11_GameView_GetV1FireShieldBorderGraphicId(void);
int M11_GameView_GetV1SpellShieldBorderGraphicId(void);
int M11_GameView_GetV1StatusShieldBorderZone(int championSlot,
                                             int* outX,
                                             int* outY,
                                             int* outW,
                                             int* outH);
int M11_GameView_GetV1StatusBoxBaseGraphic(const M11_GameViewState* state,
                                           int championSlot);
int M11_GameView_GetV1StatusBoxGraphicId(void);
int M11_GameView_GetV1DeadStatusBoxGraphicId(void);
int M11_GameView_GetV1PoisonLabelZone(int championSlot,
                                      int labelW,
                                      int labelH,
                                      int* outX,
                                      int* outY,
                                      int* outW,
                                      int* outH);
int M11_GameView_GetV1DamageIndicatorZone(int championSlot,
                                          int indicatorW,
                                          int indicatorH,
                                          int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH);
int M11_GameView_GetV1DamageNumberOrigin(int championSlot,
                                         int* outX,
                                         int* outY);
int M11_GameView_GetV1PoisonLabelGraphicId(void);
int M11_GameView_GetV1ChampionSmallDamageGraphicId(void);
int M11_GameView_GetV1ChampionBigDamageGraphicId(void);
int M11_GameView_GetV1CreatureDamageGraphicId(void);
int M11_GameView_GetV1MovementArrowsZoneId(void);
int M11_GameView_GetV1MovementArrowZoneId(int arrowIndex);
int M11_GameView_GetV1MovementArrowZone(int arrowIndex,
                                         int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
int M11_GameView_GetV1ScreenZoneId(void);
int M11_GameView_GetV1ScreenZone(int* outX,
                                  int* outY,
                                  int* outW,
                                  int* outH);
int M11_GameView_GetV1ScreenCenteredDialogZoneId(void);
int M11_GameView_GetV1ScreenCenteredDialogZone(int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH);
int M11_GameView_GetV1ExplosionPatternD0CZoneId(void);
int M11_GameView_GetV1ExplosionPatternD0CZone(int* outX,
                                              int* outY,
                                              int* outW,
                                              int* outH);
int M11_GameView_GetV1ViewportCenteredTextZoneId(void);
int M11_GameView_GetV1ViewportCenteredTextZone(int contentW,
                                               int contentH,
                                               int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH);
int M11_GameView_GetV1MessageAreaZoneId(void);
int M11_GameView_GetV1MessageAreaZone(int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH);
int M11_GameView_GetV1ViewportZoneId(void);
int M11_GameView_GetV1ViewportZone(int* outX,
                                   int* outY,
                                   int* outW,
                                   int* outH);
int M11_GameView_GetV1LeaderHandObjectNameZoneId(void);
int M11_GameView_GetV1LeaderHandObjectNameZone(int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH);
int M11_GameView_GetV1ActionAreaZoneId(void);
int M11_GameView_GetV1ActionAreaZone(int* outX,
                                        int* outY,
                                        int* outW,
                                        int* outH);
int M11_GameView_GetV1SpellAreaZoneId(void);
int M11_GameView_GetV1SpellAreaZone(int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH);
int M11_GameView_GetV1ActionSpellStripZone(int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH);
int M11_GameView_GetV1SpellCasterPanelZoneId(void);
int M11_GameView_GetV1SpellCasterPanelZone(int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH);
int M11_GameView_GetV1SpellCasterTabZoneId(void);
int M11_GameView_GetV1SpellCasterTabZone(int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH);
int M11_GameView_GetV1ActionAreaGraphicId(void);
int M11_GameView_GetV1ActionMenuGraphicZoneId(int actionRowCount);
int M11_GameView_GetV1ActionMenuGraphicZone(int actionRowCount,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH);
int M11_GameView_GetV1ActionAreaClearColor(void);
int M11_GameView_GetV1ActionResultZoneId(void);
int M11_GameView_GetV1ActionResultZone(int* outX,
                                       int* outY,
                                       int* outW,
                                       int* outH);
int M11_GameView_GetV1ActionPassZoneId(void);
int M11_GameView_GetV1ActionPassZone(int* outX,
                                     int* outY,
                                     int* outW,
                                     int* outH);
int M11_GameView_GetV1SpellAreaBackgroundGraphicId(void);
int M11_GameView_GetV1ChampionPortraitGraphicId(void);
int M11_GameView_GetV1ChampionIconGraphicId(void);
int M11_GameView_GetV1ChampionIconZoneId(int championSlot);
int M11_GameView_GetV1ChampionIconZone(int championSlot,
                                        int* outX,
                                        int* outY,
                                        int* outW,
                                        int* outH);
int M11_GameView_GetV1InventoryPanelGraphicId(void);
int M11_GameView_GetV1InventoryPanelZoneId(void);
int M11_GameView_GetV1InventoryPanelZone(int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH);
int M11_GameView_GetV1EndgameTheEndGraphicId(void);
int M11_GameView_GetV1EndgameChampionMirrorGraphicId(void);
int M11_GameView_GetV1DialogBackdropGraphicId(void);
int M11_GameView_GetV1DialogVersionTextOrigin(int* outX, int* outY);
int M11_GameView_GetV1DialogChoicePatchZone(int choiceCount,
                                             int* outSrcX,
                                             int* outSrcY,
                                             int* outW,
                                             int* outH,
                                             int* outDstX,
                                             int* outDstY);
int M11_GameView_GetV1DialogMessageZone(int choiceCount,
                                          int* outX,
                                          int* outY,
                                          int* outW,
                                          int* outH);
int M11_GameView_GetV1DialogMessageWidth(int choiceCount);
int M11_GameView_GetV1DialogSingleChoiceMessageTextY(int lineCount);
int M11_GameView_GetV1DialogMultiChoiceMessageTextY(int lineCount);
int M11_GameView_GetV1DialogChoiceTextZoneId(int choiceCount,
                                              int choiceIndex);
int M11_GameView_GetV1DialogChoiceTextZone(int choiceCount,
                                            int choiceIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH);
int M11_GameView_GetV1DialogChoiceButtonZoneId(int choiceCount,
                                                int choiceIndex);
int M11_GameView_GetV1DialogChoiceHitZone(int choiceCount,
                                           int choiceIndex,
                                           int* outX,
                                           int* outY,
                                           int* outW,
                                           int* outH);
int M11_GameView_GetV1FoodLabelGraphicId(void);
int M11_GameView_GetV1WaterLabelGraphicId(void);
int M11_GameView_GetV1FoodBarZoneId(void);
int M11_GameView_GetV1FoodBarZone(int* outX,
                                  int* outY,
                                  int* outW,
                                  int* outH,
                                  int* outSrcY);
int M11_GameView_GetV1FoodWaterPanelZoneId(void);
int M11_GameView_GetV1FoodWaterPanelZone(int* outX,
                                         int* outY,
                                         int* outW,
                                         int* outH,
                                         int* outSrcY);
int M11_GameView_GetV1SpellAreaLinesGraphicId(void);
int M11_GameView_GetV1SpellAvailableSymbolParentZoneId(int symbolIndex);
int M11_GameView_GetV1SpellAvailableSymbolZoneId(int symbolIndex);
int M11_GameView_GetV1SpellChampionSymbolZoneId(int symbolIndex);
int M11_GameView_GetV1SpellCastZoneId(void);
int M11_GameView_GetV1SpellRecantZoneId(void);
int M11_GameView_GetV1SpellLabelCellSourceZone(int selectedLine,
                                                int* outX,
                                                int* outY,
                                                int* outW,
                                                int* outH);
int M11_GameView_GetV1ActionMenuHeaderZoneId(void);
int M11_GameView_GetV1ActionMenuHeaderZone(int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH);
int M11_GameView_GetV1ActionMenuRowCount(void);
int M11_GameView_GetV1ActionMenuRowBaseZoneId(int rowIndex);
int M11_GameView_GetV1ActionMenuRowZoneId(int rowIndex);
int M11_GameView_GetV1ActionMenuRowZone(int rowIndex,
                                            int* outX,
                                            int* outY,
                                            int* outW,
                                            int* outH);
int M11_GameView_GetV1ActionMenuTextInset(int* outX,
                                           int* outY);
int M11_GameView_GetV1ActionMenuTextOrigin(int rowIndex,
                                               int* outX,
                                               int* outY);
int M11_GameView_GetV1ActionMenuHeaderFillColor(void);
int M11_GameView_GetV1ActionMenuHeaderTextColor(void);
int M11_GameView_GetV1ActionMenuRowFillColor(void);
int M11_GameView_GetV1ActionMenuRowTextColor(void);
int M11_GameView_GetV1ActionIconParentZoneId(void);
int M11_GameView_GetV1ActionIconCellZoneId(int championSlot);
int M11_GameView_GetV1ActionIconCellZone(int championSlot,
                                             int* outX,
                                             int* outY,
                                             int* outW,
                                             int* outH);
int M11_GameView_GetV1ActionIconInnerZoneId(int championSlot);
int M11_GameView_GetV1ActionIconInnerZone(int championSlot,
                                              int* outX,
                                              int* outY,
                                              int* outW,
                                              int* outH);
int M11_GameView_GetV1ObjectIconSourceZone(int iconIndex,
                                               int* outGraphicIndex,
                                               int* outX,
                                               int* outY,
                                               int* outW,
                                               int* outH);
int M11_GameView_MapV1ActionIconPaletteColor(int colorIndex,
                                             int applyActionPalette);
int M11_GameView_ShouldHatchV1ActionIconCells(const M11_GameViewState* state);
int M11_GameView_GetV1ActionIconCellBackdropColor(const M11_GameViewState* state,
                                                  int championSlot);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_VIEW_H */

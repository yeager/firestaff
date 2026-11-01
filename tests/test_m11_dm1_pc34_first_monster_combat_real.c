/* Original PC 3.4 HoC-to-first-monster action regression. */
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_floor_ornament_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"
#include "asset_loader_m11.h"
#include "menu_input_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>

static unsigned int group_health_total(const struct DungeonGroup_Compat *group)
{
    unsigned int total = 0;
    unsigned int i;
    if (!group) return 0;
    for (i = 0; i <= group->count && i < 4u; ++i)
        total += group->health[i];
    return total;
}

static int expected_ai_state_for_group_behavior(unsigned char behavior)
{
    switch (behavior) {
        case DM1_BEHAVIOR_ATTACK: return AI_STATE_ATTACK;
        case DM1_BEHAVIOR_APPROACH: return AI_STATE_APPROACH;
        case DM1_BEHAVIOR_FLEE: return AI_STATE_FLEE;
        case DM1_BEHAVIOR_WANDER:
        default: return AI_STATE_WANDER;
    }
}

static int verify_saved_champion_hud_icons(M11_GameViewState *state)
{
    unsigned char framebuffer[320 * 200];
    const M11_AssetSlot *iconStrip;
    int slot;
    if (!state || state->world.party.championCount != CHAMPION_MAX_PARTY)
        return 0;
    iconStrip = M11_AssetLoader_Load(&state->assetLoader,
                                     DM1_GFX_CHAMPION_ICONS);
    if (!iconStrip || !iconStrip->loaded || !iconStrip->pixels ||
        iconStrip->width != DM1_CHAMPION_ICON_WIDTH * CHAMPION_MAX_PARTY ||
        iconStrip->height != DM1_CHAMPION_ICON_HEIGHT) {
        fputs("FAIL: authentic PC34 champion icon strip did not load\n", stderr);
        return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        const struct ChampionState_Compat *champion =
            &state->world.party.champions[slot];
        DM1_V1_LayoutZoneRectPc34 rect;
        DM1_ChampionPanel_IconBitmapModel model;
        unsigned int spritePixels = 0;
        unsigned int mismatches = 0;
        int x, y;
        if (!champion->present || champion->hp.current == 0 ||
            !dm1_v1_champion_icon_rect_pc34(slot, &rect) ||
            !DM1_ChampionPanel_BuildIconBitmapModel(
                slot, (int)champion->direction & 3,
                state->world.party.direction & 3,
                (int)state->world.magic.event71CountInvisibility, &model)) {
            fprintf(stderr,
                    "FAIL: authentic PC34 saved champion %d is not eligible for HUD icon rendering\n",
                    slot);
            return 0;
        }
        if (model.applyInvisibilityPalette || rect.w < model.width ||
            rect.h < model.height) {
            fprintf(stderr,
                    "FAIL: authentic PC34 saved champion %d has unsupported HUD comparison state\n",
                    slot);
            return 0;
        }
        for (y = 0; y < model.height; ++y) {
            for (x = 0; x < model.width; ++x) {
                const unsigned char source = iconStrip->pixels[
                    (model.sourceY + y) * iconStrip->width +
                    model.sourceX + x];
                const unsigned char expected = source == model.transparentColor
                    ? (unsigned char)model.fillColor : source;
                const unsigned char actual =
                    framebuffer[(rect.y + y) * 320 + rect.x + x];
                if (source != model.transparentColor) ++spritePixels;
                if (actual != expected) ++mismatches;
            }
        }
        if (spritePixels == 0 || mismatches != 0) {
            fprintf(stderr,
                    "FAIL: authentic PC34 saved champion %d HUD icon differs from GRAPHICS.DAT: sprite-pixels=%u mismatches=%u rect=%d,%d %dx%d\n",
                    slot, spritePixels, mismatches, rect.x, rect.y,
                    rect.w, rect.h);
            return 0;
        }
        printf("PASS: authentic PC34 saved champion %d HUD icon matches GRAPHICS.DAT (%u sprite pixels)\n",
               slot, spritePixels);
    }
    return 1;
}

typedef struct {
    uint64_t cursor;
    uint64_t dropped;
    unsigned int playerAttacks;
    unsigned int playerDamage;
    unsigned int monsterMoves;
    unsigned int monsterAttacks;
    unsigned int group169Attacks;
    unsigned int championDamage;
    unsigned int championHpLoss;
    int sequenceError;
} RuntimeEventAudit;

static void collect_runtime_events(const struct GameWorld_Compat* world,
                                   RuntimeEventAudit* audit)
{
    DM1RuntimeEventReceipt_Compat events[DM1_RUNTIME_EVENT_RECEIPT_CAPACITY];
    uint64_t dropped = 0;
    size_t count;
    size_t i;
    if (!world || !audit) return;
    do {
        count = DM1_RuntimeEventReceiptsSince_Compat(
            world, &audit->cursor, events,
            sizeof(events) / sizeof(events[0]), &dropped);
        audit->dropped += dropped;
        for (i = 0; i < count; ++i) {
            const DM1RuntimeEventReceipt_Compat* event = &events[i];
            if (event->sequence != audit->cursor - count + i + 1u)
                audit->sequenceError = 1;
            switch (event->kind) {
            case DM1_RUNTIME_EVENT_PLAYER_ATTACK:
                ++audit->playerAttacks;
                if (event->damage > 0 && event->groupIndex == 169)
                    ++audit->playerDamage;
                break;
            case DM1_RUNTIME_EVENT_CREATURE_MOVED:
                ++audit->monsterMoves;
                break;
            case DM1_RUNTIME_EVENT_CREATURE_ATTACK:
                ++audit->monsterAttacks;
                if (event->groupIndex == 169) ++audit->group169Attacks;
                break;
            case DM1_RUNTIME_EVENT_CHAMPION_DAMAGED:
                ++audit->championDamage;
                if (event->damage > 0 && event->hpAfter < event->hpBefore)
                    ++audit->championHpLoss;
                break;
            default:
                break;
            }
        }
    } while (count == sizeof(events) / sizeof(events[0]));
}

static M11_GameInputResult audited_input(M11_GameViewState* state,
                                         M12_MenuInput input,
                                         RuntimeEventAudit* audit)
{
    M11_GameInputResult result = M11_GameView_HandleInput(state, input);
    collect_runtime_events(&state->world, audit);
    return result;
}

static M11_GameInputResult audited_pointer(M11_GameViewState* state,
                                          int x, int y, int buttonMask,
                                          RuntimeEventAudit* audit)
{
    M11_GameInputResult result = M11_GameView_HandlePointerButton(
        state, x, y, buttonMask);
    collect_runtime_events(&state->world, audit);
    return result;
}

static void audited_idle_tick(M11_GameViewState* state,
                              RuntimeEventAudit* audit)
{
    (void)M11_GameView_AdvanceIdleTick(state);
    collect_runtime_events(&state->world, audit);
}

static int verify_hoc_plate_changes_indexed_view(M11_GameViewState* state,
                                                  int relativeForward,
                                                  int partyX,
                                                  int expectedGraphicIndex)
{
    unsigned char framebuffer[320 * 200];
    unsigned char rendered[320 * 200];
    uint8_t palette[256][3];
    struct DungeonThings_Compat* things;
    unsigned short sensorThing;
    struct DungeonSensor_Compat* plateSensor = NULL;
    DM1_FloorOrnamentRenderPlanPc34 plan = {0};
    unsigned char savedOrdinal;
    int safety = 0;
    int mapX = -1, mapY = -1;
    int ordinal;
    int globalOrnamentIndex;
    if (!state || state->world.party.mapIndex != 0 ||
        state->world.party.mapX != partyX || state->world.party.mapY != 9 ||
        state->world.party.direction != 3) {
        if (state) {
            fprintf(stderr, "plate view pose map=%d party=(%d,%d,d%d)\n",
                    state->world.party.mapIndex, state->world.party.mapX,
                    state->world.party.mapY, state->world.party.direction);
        }
        return 0;
    }
    /* Probe the exact authentic plate square through normal viewport geometry. */
    if (!M11_GameView_ProbeViewportCellClass(state, relativeForward, 0,
            &mapX, &mapY, NULL, NULL, NULL, NULL, NULL) ||
        mapX != 6 || mapY != 9) {
        fprintf(stderr, "plate target probe at D%d=(%d,%d)\n",
                relativeForward, mapX, mapY);
        return 0;
    }
    ordinal = M11_GameView_GetFloorOrnamentOrdinal(
        state, relativeForward, 0);
    if (ordinal <= 0 || ordinal > 16) {
        fprintf(stderr, "plate target has invalid floor ornament ordinal: %d\n",
                ordinal);
        return 0;
    }
    globalOrnamentIndex = state->floorOrnamentIndices[0][ordinal - 1];
    if (globalOrnamentIndex != 1 ||
        !dm1_v1_floor_ornament_render_plan_pc34(
            relativeForward, 0, globalOrnamentIndex, &plan) ||
        plan.blit.graphicIndex != expectedGraphicIndex) {
        fprintf(stderr,
                "HoC plate selected unexpected material at D%dC (ordinal=%d global=%d graphic=%d expected=%d)\n",
                relativeForward, ordinal, globalOrnamentIndex,
                plan.blit.graphicIndex, expectedGraphicIndex);
        return 0;
    }
    things = state->world.things;
    if (!things || !things->sensors) {
        fputs("plate sensor table missing\n", stderr);
        return 0;
    }
    sensorThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, things, 0, mapX, mapY);
    while (sensorThing != THING_ENDOFLIST && sensorThing != THING_NONE &&
           safety++ < 64) {
        if (THING_GET_TYPE(sensorThing) == THING_TYPE_SENSOR) {
            int index = THING_GET_INDEX(sensorThing);
            if (index >= 0 && index < things->sensorCount) {
                plateSensor = &things->sensors[index];
                break;
            }
        }
        sensorThing = F0512_DUNGEON_GetThingNext_Compat(things, sensorThing);
    }
    if (!plateSensor || plateSensor->ornamentOrdinal != ordinal) {
        fprintf(stderr,
                "plate sensor missing or ordinal mismatch (sensor=%d ordinal=%d)\n",
                plateSensor ? (int)plateSensor->ornamentOrdinal : -1, ordinal);
        return 0;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    memcpy(rendered, framebuffer, sizeof(rendered));
    /* Re-render the authentic world with only this sensor's ornament
     * suppressed to recover the original floor pixels beneath the plate. */
    savedOrdinal = plateSensor->ornamentOrdinal;
    plateSensor->ornamentOrdinal = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    plateSensor->ornamentOrdinal = savedOrdinal;
    {
        int x, y;
        unsigned int changed = 0;
        unsigned int outsideZone = 0;
        unsigned int lowContrast = 0;
        unsigned int minRgbDelta = 766u;
        int zoneLeft = plan.blit.dstX;
        int zoneRight = zoneLeft + plan.blit.width;
        int zoneTop = 33 + plan.blit.dstY;
        int zoneBottom = zoneTop + plan.blit.height;
        /* Ignore index-only changes that collapse to the same color. Every
         * source-zone pixel must differ visibly from the authentic PC34
         * palette-mapped floor with the sensor ornament suppressed. */
        if (!M11_Render_CopyIndexedPaletteRgb8(palette)) {
            fputs("PC34 indexed presentation palette is unavailable\n", stderr);
            return 0;
        }
        for (y = 33; y < 168; ++y) for (x = 48; x < 272; ++x) {
            size_t at = (size_t)y * 320u + (size_t)x;
            if (rendered[at] != framebuffer[at]) {
                unsigned int rgbDelta = 0;
                int channel;
                ++changed;
                if (x < zoneLeft || x >= zoneRight ||
                    y < zoneTop || y >= zoneBottom) {
                    ++outsideZone;
                }
                for (channel = 0; channel < 3; ++channel) {
                    int a = palette[rendered[at]][channel];
                    int b = palette[framebuffer[at]][channel];
                    rgbDelta += (unsigned int)(a > b ? a - b : b - a);
                }
                if (rgbDelta < minRgbDelta) minRgbDelta = rgbDelta;
                if (rgbDelta < 60u) ++lowContrast;
            }
        }
        if (changed == 0u || outsideZone != 0u || lowContrast != 0u) {
            fprintf(stderr,
                    "HoC plate RGB contribution invalid at D%dC: pixels=%u outside-zone=%u low-contrast=%u min-RGB-L1=%u\n",
                    relativeForward, changed, outsideZone, lowContrast,
                    minRgbDelta);
            return 0;
        }
        printf("PASS: authentic PC34 HoC pressure plate (6,9) selects GRAPHICS.DAT %04d and changes %u in-zone pixels at D%dC from (%d,9), minimum RGB-L1 contrast=%u\n",
               expectedGraphicIndex, changed, relativeForward, partyX,
               minRgbDelta);
    }
    return 1;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    const char *originalSave = getenv("FIRESTAFF_DM1_PC34_ORIGINAL_SAVE");
    const char *shieldText = getenv("FIRESTAFF_TEST_PARTY_SHIELD");
    M11_GameViewState state;
    static const M12_MenuInput route_to_c127[] = {
        M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
        M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_TURN_RIGHT, M12_MENU_INPUT_TURN_RIGHT
    };
    static const M12_MenuInput route_to_scroll[] = {
        M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN
    };
    size_t i;
    int tick;
    unsigned char actions[3];
    unsigned short first;
    int clickX, clickY, clickW, clickH;
    uint32_t attackBeforeDoor;
    uint32_t attackAfterFirstEncounter;
    uint32_t encounterTickStart;
    unsigned short initialChampionHp;
    unsigned short hpAfterFirstEncounter;
    unsigned int naturalDamage;
    unsigned int monsterHpBeforePunch;
    unsigned int monsterHpAfterPunch;
    int repeatedGroup169Attacks = 0;
    RuntimeEventAudit eventAudit = {0};
    long partyShieldDefense = shieldText ? strtol(shieldText, NULL, 10) : 0;

    if (partyShieldDefense < 0 || partyShieldDefense > 100) {
        fputs("FAIL: test party shield must be between 0 and 100\n", stderr);
        return 2;
    }

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 77;
    }
    {
        FILE *media = fopen(archive, "rb");
        if (!media) {
            puts("SKIP: original PC 3.4 archive is unavailable");
            return 77;
        }
        fclose(media);
    }
    /* When a real PC34 save is available, verify F0435 restores the saved
     * four-champion formation cells from each 319-byte C2 champion record.
     * The source writes Direction at byte 28 and Cell at byte 29; losing the
     * latter collapses the whole party into cell zero and breaks melee reach. */
    if (originalSave && originalSave[0]) {
        M11_GameViewState resumed;
        unsigned int formationCells = 0u;
        int slot;
        FILE *save = fopen(originalSave, "rb");
        if (!save) {
            fprintf(stderr, "FAIL: authentic PC34 save is unavailable: %s\n",
                    originalSave);
            return 1;
        }
        fclose(save);
        M11_GameView_Init(&resumed);
        if (!M11_GameView_StartDm1(&resumed, archive) ||
            !M11_GameView_LoadDm1SavePath(&resumed, originalSave, NULL) ||
            resumed.world.party.championCount != 4) {
            fprintf(stderr,
                    "FAIL: authentic PC34 save did not restore four champions (%s %s)\n",
                    resumed.lastAction, resumed.lastOutcome);
            M11_GameView_Shutdown(&resumed);
            return 1;
        }
        for (slot = 0; slot < resumed.world.party.championCount; ++slot) {
            const unsigned int cell = resumed.world.party.champions[slot].cell;
            if (cell > 3u || (formationCells & (1u << cell)) != 0u) {
                fprintf(stderr,
                        "FAIL: authentic PC34 save has invalid/duplicate champion cell %u at slot %d\n",
                        cell, slot);
                M11_GameView_Shutdown(&resumed);
                return 1;
            }
            formationCells |= 1u << cell;
        }
        if (formationCells != 0x0fu) {
            fprintf(stderr,
                    "FAIL: authentic PC34 save restored incomplete formation mask 0x%x\n",
                    formationCells);
            M11_GameView_Shutdown(&resumed);
            return 1;
        }
        if (!verify_saved_champion_hud_icons(&resumed)) {
            M11_GameView_Shutdown(&resumed);
            return 1;
        }
        {
            int active = 0;
            int group169AttackRestored = 0;
            for (slot = 0; slot < resumed.world.creatureAICount; ++slot) {
                const struct CreatureAIState_Compat *ai =
                    &resumed.world.creatureAI[slot];
                const int groupIndex = ai->reserved0;
                unsigned char behavior;
                if (groupIndex < 0 || groupIndex >=
                        resumed.world.things->groupCount) {
                    continue;
                }
                behavior = resumed.world.things->groups[groupIndex].behavior;
                if (ai->stateKind !=
                    expected_ai_state_for_group_behavior(behavior)) {
                    fprintf(stderr,
                            "FAIL: authentic PC34 save group %d behavior=%u "
                            "restored AI state=%d expected=%d\n",
                            groupIndex, (unsigned int)behavior, ai->stateKind,
                            expected_ai_state_for_group_behavior(behavior));
                    M11_GameView_Shutdown(&resumed);
                    return 1;
                }
                ++active;
                if (groupIndex == 169 &&
                    behavior == DM1_BEHAVIOR_ATTACK &&
                    ai->stateKind == AI_STATE_ATTACK) {
                    group169AttackRestored = 1;
                }
            }
            if (resumed.world.things->groupCount > 169 &&
                resumed.world.things->groups[169].behavior ==
                    DM1_BEHAVIOR_ATTACK && !group169AttackRestored) {
                fputs("FAIL: authentic save lost group 169's attack state\n",
                      stderr);
                M11_GameView_Shutdown(&resumed);
                return 1;
            }
            printf("PASS: authentic PC34 save restores source behavior for %d active monster groups\n",
                   active);
        }
        M11_GameView_Shutdown(&resumed);
        puts("PASS: authentic PC34 save restores all four source champion cells");
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, archive) || !state.world.dungeon ||
        !state.world.things || state.world.dungeon->header.mapCount < 1 ||
        state.world.things->scrollCount != 35 ||
        state.world.pc34F0306FirstOperandFirst != 0 ||
        !state.vblankTiming.pc34PitInputWait) {
        fputs("FAIL: canonical PC34 dungeon did not load\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Replay the same 28-command PC-3.4 keypad route as hoc_route in the
     * native CLI regression, using production runtime inputs. The direct M11
     * test advances five idle ticks after each input so one command completes
     * before the next; this is deterministic runtime pacing, not a claim that
     * the CLI script has identical tick timing. Start from the source
     * bootstrap pose; no save or pose is fabricated. */
    for (tick = 0; tick < 5; ++tick)
        audited_idle_tick(&state, &eventAudit);
    for (i = 0u; i < sizeof(route_to_c127) / sizeof(route_to_c127[0]); ++i) {
        M11_GameInputResult result =
            audited_input(&state, route_to_c127[i], &eventAudit);
        if (result == M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: authentic PC34 Hall route input was ignored\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            audited_idle_tick(&state, &eventAudit);
    }
    if (audited_pointer(&state, 112, 83, M11_DM1_MOUSE_MASK_LEFT,
            &eventAudit) == M11_GAME_INPUT_IGNORED ||
        !state.candidateMirrorPanelActive ||
        state.candidateMirrorOrdinal != 5) {
        fputs("FAIL: source route did not open C127 ordinal 5\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (audited_pointer(&state, 130, 115, M11_DM1_MOUSE_MASK_LEFT,
            &eventAudit) == M11_GAME_INPUT_IGNORED ||
        state.world.party.championCount != 1) {
        fputs("FAIL: C040 did not recruit the authentic C127 candidate\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    initialChampionHp = state.world.party.champions[0].hp.current;
    for (i = 0u; i < sizeof(route_to_scroll) / sizeof(route_to_scroll[0]); ++i) {
        if (i == 13u) {
            if (audited_input(&state, M12_MENU_INPUT_TURN_LEFT, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not turn toward the Hall plate\n", stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            if (!verify_hoc_plate_changes_indexed_view(&state, 1, 7, 396)) {
                fputs("FAIL: authentic PC34 HoC plate did not change indexed pixels at D1C\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            if (audited_input(&state, M12_MENU_INPUT_DOWN, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not step back for the D2C plate view\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            if (!verify_hoc_plate_changes_indexed_view(&state, 2, 8, 394)) {
                fputs("FAIL: authentic PC34 HoC plate did not change indexed pixels at D2C\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            if (audited_input(&state, M12_MENU_INPUT_DOWN, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not step back for the D3C plate view\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            if (!verify_hoc_plate_changes_indexed_view(&state, 3, 9, 392)) {
                fputs("FAIL: authentic PC34 HoC plate did not change indexed pixels at D3C\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            if (audited_input(&state, M12_MENU_INPUT_UP, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not return from D3C to D2C\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            if (audited_input(&state, M12_MENU_INPUT_UP, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not return to the HoC plate approach\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            if (state.world.party.mapX != 7 || state.world.party.mapY != 9) {
                fputs("FAIL: authentic route did not return to the HoC plate approach\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            if (audited_input(&state, M12_MENU_INPUT_TURN_RIGHT, &eventAudit) ==
                    M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic route could not restore heading from the Hall plate\n", stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
        }
        M11_GameInputResult result =
            audited_input(&state, route_to_scroll[i], &eventAudit);
        if (result == M11_GAME_INPUT_IGNORED) {
            fputs("FAIL: authentic Hall route to scroll was ignored\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        for (tick = 0; tick < 5; ++tick)
            audited_idle_tick(&state, &eventAudit);
        /* The authentic PC34 Hall route steps on the C003 floor-party plate
         * at (6,9), which opens the door in (5,9). Reaching the next cell
         * through production movement verifies both F0276 activation and
         * the door transition without assigning the party pose in the test. */
        if (i == 13u &&
            (state.world.party.mapIndex != 0 ||
             state.world.party.mapX != 6 || state.world.party.mapY != 9)) {
            fputs("FAIL: authentic route did not activate the Hall floor plate at (6,9)\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (i == 14u &&
            (state.world.party.mapIndex != 0 ||
             state.world.party.mapX != 5 || state.world.party.mapY != 9)) {
            fputs("FAIL: authentic route did not pass through the opened Hall door at (5,9)\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    if (state.world.party.mapIndex != 0 || state.world.party.mapX != 4 ||
        state.world.party.mapY != 15 || state.world.party.direction != 0) {
        fputs("FAIL: production movement did not reach authentic HoC (4,15)\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Continue the source route through the first pressure/door passage.
     * ReDMCSB COMMAND.C:2155 dispatches movement to CLIKMENU.C F0366;
     * CLIKVIEW.C:437 dispatches the visible front-cell C080 click. */
    {
        static const M12_MenuInput to_first_door[] = {
            M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_UP,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_STRAFE_RIGHT,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_TURN_RIGHT,
            M12_MENU_INPUT_TURN_RIGHT
        };
        for (i = 0; i < sizeof(to_first_door) / sizeof(to_first_door[0]); ++i) {
            if (audited_input(&state, to_first_door[i], &eventAudit) ==
                M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: authentic first-door route input ignored\n", stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
        }
    }
    /* Controlled in-memory spell state lets the same original group attack
     * exercise the live M11 body-shield snapshot layer. F0446 uses the same
     * 0..100 range for party ShieldDefense (ReDMCSB ENDGAME.C:805-812). */
    state.world.magic.partyShieldDefense =
        (unsigned short)partyShieldDefense;
    attackBeforeDoor = state.world.lifecycle.lastCreatureAttackTime;
    if (audited_pointer(&state, 167, 81, M11_DM1_MOUSE_MASK_LEFT,
            &eventAudit) == M11_GAME_INPUT_IGNORED) {
        fputs("FAIL: authentic first-door button click ignored\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (tick = 0; tick < 30; ++tick)
        audited_idle_tick(&state, &eventAudit);
    if (audited_input(&state, M12_MENU_INPUT_UP, &eventAudit) ==
        M11_GAME_INPUT_IGNORED) {
        fputs("FAIL: first-door passage input ignored\n", stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (tick = 0; tick < 5; ++tick)
        audited_idle_tick(&state, &eventAudit);
    if (partyShieldDefense == 0) {
        /* The authentic group can emit a source attack that misses. Keep
         * advancing the real timeline until its ordinary attack lands. */
        for (tick = 0; tick < 1500 &&
             state.world.party.champions[0].hp.current >= initialChampionHp &&
             !state.partyDead && !state.world.partyDead; ++tick)
            audited_idle_tick(&state, &eventAudit);
    }
    if (state.world.party.mapIndex != 1 || state.world.party.mapX != 6 ||
        state.world.party.mapY < 0 || state.world.party.mapY >=
            (int)state.world.dungeon->maps[1].height ||
        state.world.party.direction != 2 ||
        (partyShieldDefense == 0 &&
         state.world.party.champions[0].hp.current >= initialChampionHp) ||
        state.world.lifecycle.lastCreatureAttackTime == attackBeforeDoor ||
        state.world.things->groupCount <= 169 ||
        state.world.things->groups[169].health[0] == 0) {
        fputs("FAIL: authentic map-1 group did not naturally attack after arrival\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    naturalDamage = state.world.party.champions[0].hp.current < initialChampionHp
        ? (unsigned int)(initialChampionHp -
                         state.world.party.champions[0].hp.current)
        : 0u;
    /* Compare the authenticated baseline damage before shielding subsequent
     * retries, so a source miss cannot kill the party during PUNCH coverage. */
    state.world.magic.partyShieldDefense = 100u;
    {
        int group169Active = 0;
        int group169StateKind = -1;
        int group169X = -1;
        int group169Y = -1;
        for (i = 0; i < (size_t)state.world.creatureAICount; ++i) {
            const struct CreatureAIState_Compat* ai = &state.world.creatureAI[i];
            if (ai->reserved0 == 169 && ai->groupMapIndex == 1) {
                group169Active = 1;
                group169StateKind = ai->stateKind;
                group169X = ai->groupMapX;
                group169Y = ai->groupMapY;
                break;
            }
        }
        if (!group169Active || state.world.pc34ActiveGroupSourceCount <= 0 ||
            group169StateKind != AI_STATE_ATTACK ||
            state.world.things->groups[169].behavior != DM1_BEHAVIOR_ATTACK) {
            fputs("FAIL: original group #169 did not enter natural attack behavior\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        first = F0511_DUNGEON_GetSquareFirstThing_Compat(state.world.dungeon,
            state.world.things, 1, group169X, group169Y);
        if (first != 0x10a9u) {
            fputs("FAIL: active original group #169 lost its map square\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (abs(group169X - state.world.party.mapX) +
                abs(group169Y - state.world.party.mapY) > 1) {
            fputs("FAIL: original group #169 did not approach the party\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (group169X != state.world.party.mapX ||
            group169Y != state.world.party.mapY + 1) {
            fputs("FAIL: original group #169 is not in the party's forward cell\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        const int32_t ninjaXpBefore =
            state.world.lifecycle.champions[0].skills20[1].experience;
        const int32_t fightXpBefore =
            state.world.lifecycle.champions[0].skills20[9].experience;
        const int32_t ninjaXpStart = ninjaXpBefore;
        const int32_t fightXpStart = fightXpBefore;
        monsterHpBeforePunch = group_health_total(
            &state.world.things->groups[169]);
        monsterHpAfterPunch = monsterHpBeforePunch;
        /* ReDMCSB MENU.C:803-840, F0391 selects an action-list row and
         * dispatches F0407 before clearing the acting champion. PUNCH uses
         * source randomness and can miss, so retry through the ordinary
         * cooldown/action-menu path until the original group takes damage. */
        for (int punchAttempt = 0;
             punchAttempt < 24 && monsterHpAfterPunch >= monsterHpBeforePunch;
             ++punchAttempt) {
            int waitGuard = 0;
            while (state.actionDisabledTicks[0] > 0 && waitGuard++ < 256)
                audited_idle_tick(&state, &eventAudit);
            if (state.actionDisabledTicks[0] > 0 ||
                !M11_GameView_GetV1ActionIconCellZone(0,
                    &clickX, &clickY, &clickW, &clickH) ||
                audited_pointer(&state,
                    clickX + clickW / 2, clickY + clickH / 2,
                    M11_DM1_MOUSE_MASK_LEFT, &eventAudit) == M11_GAME_INPUT_IGNORED ||
                state.actingChampionOrdinal != 1 ||
                !M11_GameView_GetActingActionIndices(&state, actions) ||
                actions[0] != 6 || actions[1] != 7 || actions[2] != 8 ||
                !M11_GameView_GetV1ActionMenuRowZone(0,
                    &clickX, &clickY, &clickW, &clickH) ||
                audited_pointer(&state,
                    clickX + clickW / 2, clickY + clickH / 2,
                    M11_DM1_MOUSE_MASK_LEFT, &eventAudit) == M11_GAME_INPUT_IGNORED) {
                fputs("FAIL: ordinary pointer route could not repeat first-monster PUNCH\n",
                      stderr);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            for (tick = 0; tick < 5; ++tick)
                audited_idle_tick(&state, &eventAudit);
            monsterHpAfterPunch = group_health_total(
                &state.world.things->groups[169]);
        }
        /* PUNCH awards 8 base experience to skill 9 (FIGHT) and mirrors it
         * to skill 1 (NINJA), even on an ordinary miss. */
        if (state.world.lifecycle.champions[0].skills20[1].experience <= ninjaXpStart ||
            state.world.lifecycle.champions[0].skills20[9].experience <= fightXpStart ||
            state.world.lifecycle.champions[0].skills20[1].experience - ninjaXpStart !=
                state.world.lifecycle.champions[0].skills20[9].experience - fightXpStart) {
            fputs("FAIL: live PUNCH did not award matching Ninja/Fight XP\n", stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        if (monsterHpAfterPunch >= monsterHpBeforePunch) {
            fprintf(stderr,
                    "FAIL: authentic PC34 PUNCH missed all %d source retries against group #169 (%u HP)\n",
                    24, monsterHpBeforePunch);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    {
        struct ChampionState_Compat saved = state.world.party.champions[0];
        struct ChampionState_Compat *champ =
            &state.world.party.champions[0];
        int feetIcon;
        int expectedMaxLoad;
        int withStaleCache;
        int withFreshCache;

        /* F0309 feeds F0312 from live stamina, wounds and footwear. Exercise
         * the real loaded HoC champion after changing those source fields,
         * and ensure neither the M11 cache nor an F0312 call relies on its
         * previous maxLoad value. */
        champ->stamina.current = champ->stamina.maximum / 4u;
        champ->wounds = (unsigned short)(champ->wounds | (1u << 4));
        feetIcon = dm1_v1_dungeon_get_object_icon_index_pc34(
            state.world.things, champ->inventory[CHAMPION_SLOT_FEET],
            state.world.party.direction);
        champ->maxLoad = 1u;
        withStaleCache = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, champ, 0, 80);
        expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
            champ, feetIcon, state.world.pc34F0306FirstOperandFirst);
        champ->maxLoad = (unsigned short)expectedMaxLoad;
        withFreshCache = F0312_CHAMPION_HandStrengthBaseline_Compat(
            &state.world, champ, 0, 80);
        if (withStaleCache != withFreshCache) {
            fputs("FAIL: F0312 used stale maxLoad after live stat changes\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        champ->maxLoad = 1u;
        audited_idle_tick(&state, &eventAudit);
        expectedMaxLoad = F0309_CHAMPION_GetMaximumLoadForSource_Compat(
            champ, dm1_v1_dungeon_get_object_icon_index_pc34(
                state.world.things, champ->inventory[CHAMPION_SLOT_FEET],
                state.world.party.direction),
            state.world.pc34F0306FirstOperandFirst);
        if ((int)champ->maxLoad != expectedMaxLoad) {
            fputs("FAIL: DM1 PC34 did not refresh cached F0309 maxLoad\n",
                  stderr);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        *champ = saved;
    }
    /* F0207 attacks once when the source creature enters its attack
     * animation. ReDMCSB GROUP.C:2344 then clears the C38 attack latch so the
     * per-creature timeline can attack again. Prove the recurrence while the
     * original group is still alive, after exercising the player's action
     * path. A full shield keeps this long source-time probe from ending the
     * run through champion death. */
    if (state.world.things->groups[169].health[0] == 0) {
        fputs("FAIL: first monster died before the repeated-attack probe\n",
              stderr);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    attackAfterFirstEncounter = state.world.lifecycle.lastCreatureAttackTime;
    encounterTickStart = state.world.gameTick;
    hpAfterFirstEncounter = state.world.party.champions[0].hp.current;
    state.world.magic.partyShieldDefense = 100u;
    for (tick = 0; tick < 1500; ++tick) {
        int emissionIndex;
        if (state.partyDead || state.world.partyDead) break;
        audited_idle_tick(&state, &eventAudit);
        for (emissionIndex = 0;
             emissionIndex < state.lastTickResult.emissionCount;
             ++emissionIndex) {
            const struct TickEmission_Compat *emission =
                &state.lastTickResult.emissions[emissionIndex];
            if (emission->kind == EMIT_CREATURE_ATTACK &&
                emission->payload[0] == 169) {
                ++repeatedGroup169Attacks;
            }
        }
    }
    if (state.world.lifecycle.lastCreatureAttackTime <=
            attackAfterFirstEncounter || repeatedGroup169Attacks == 0) {
        fprintf(stderr,
            "FAIL: source group #169 did not attack again in 21 seconds "
            "(attacks=%d group=%u hp=%u->%u gameTick=%u->%u "
            "attackTick=%u->%u)\n",
            repeatedGroup169Attacks,
            (unsigned int)state.world.things->groups[169].health[0],
            (unsigned int)hpAfterFirstEncounter,
            (unsigned int)state.world.party.champions[0].hp.current,
            (unsigned int)encounterTickStart,
            (unsigned int)state.world.gameTick,
            (unsigned int)attackAfterFirstEncounter,
            (unsigned int)state.world.lifecycle.lastCreatureAttackTime);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (eventAudit.dropped != 0 || eventAudit.sequenceError ||
        eventAudit.playerAttacks == 0 || eventAudit.playerDamage == 0 ||
        eventAudit.monsterMoves == 0 || eventAudit.monsterAttacks == 0 ||
        eventAudit.group169Attacks == 0 ||
        (naturalDamage > 0 && eventAudit.championHpLoss == 0)) {
        fprintf(stderr,
            "FAIL: authentic PC34 debug receipts missed runtime events "
            "(dropped=%llu sequenceError=%d player=%u playerDamage=%u "
            "monsterMove=%u monsterAttack=%u group169=%u championDamage=%u "
            "championHpLoss=%u naturalDamage=%u)\n",
            (unsigned long long)eventAudit.dropped, eventAudit.sequenceError,
            eventAudit.playerAttacks, eventAudit.playerDamage,
            eventAudit.monsterMoves, eventAudit.monsterAttacks,
            eventAudit.group169Attacks, eventAudit.championDamage,
            eventAudit.championHpLoss, naturalDamage);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    printf("PASS: authentic PC34 combat and debug receipts "
           "player=%u playerDamage=%u monsterMoves=%u monsterAttacks=%u "
           "group169Attacks=%u championHpLoss=%u naturalDamage=%u "
           "partyShield=%ld\n",
           eventAudit.playerAttacks, eventAudit.playerDamage,
           eventAudit.monsterMoves, eventAudit.monsterAttacks,
           eventAudit.group169Attacks, eventAudit.championHpLoss,
           naturalDamage, partyShieldDefense);
    return 0;
}

/*
 * firestaff_theron_v2_overlay_seed_gate_probe.c — Theron V2 HUD seed gate
 *
 * Data-free probe for the V1→V2 presentation snapshot helper. It proves
 * the enhanced overlay can be seeded from a synthetic Theron V1 world
 * without mutating source-locked V1 runtime state, and that V1 original
 * mode returns a hidden overlay that writes no pixels.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T520  party placement / leader direction
 *   THQUEST.ASM T600  UI overlay zones
 *   THQUEST.ASM T800  champion persistence / stats
 *   THQUEST.ASM T900  object database / rune magic
 *   sibling: dm2_v2_hud_runtime.c presentation snapshot pattern
 */

#include "theron_v2_hud_overlay_pc34.h"
#include "theron_v1_viewport.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FB_W TQR_FB_W
#define FB_H TQR_FB_H

static int s_pass = 0;
static int s_fail = 0;

static void check(const char *name, int cond)
{
    if (cond) {
        printf("  PASS: %s\n", name);
        ++s_pass;
    } else {
        printf("  FAIL: %s\n", name);
        ++s_fail;
    }
}

static int fb_nonzero_count(const uint8_t *fb, size_t count)
{
    int n = 0;
    for (size_t i = 0; i < count; ++i) {
        if (fb[i] != 0U) {
            ++n;
        }
    }
    return n;
}

static void seed_champion(Theron_V1_Champion *c,
                          int hp, int max_hp,
                          int stamina, int max_stamina,
                          int mana, int max_mana,
                          uint8_t alive)
{
    memset(c, 0, sizeof(*c));
    c->health = (int16_t)hp;
    c->max_health = (int16_t)max_hp;
    c->stamina = (int16_t)stamina;
    c->max_stamina = (int16_t)max_stamina;
    c->mana = (int16_t)mana;
    c->max_mana = (int16_t)max_mana;
    c->alive = alive;
}

static void seed_world(Theron_V1_World *world)
{
    memset(world, 0, sizeof(*world));
    world->current_dungeon = 4;
    world->quest_items_in_dungeon = 5;
    world->world_tick = 0x123456789ABCDEF0ULL;
    world->state_hash = 0xC0FFEE1234ULL;
    world->progression.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    world->progression.quest_items_collected =
        (uint8_t)(THERON_QUEST_ITEM_1_SACRED_AMPLIFIER |
                  THERON_QUEST_ITEM_3_FLAME_ORBS |
                  THERON_QUEST_ITEM_5_WAYWARD_RIBBON);
    world->progression.quest_items_in_current_dungeon = 2;
    world->party.champion_count = 2;
    world->party.active_slot = 1;
    world->party.leader_dir = 2;
    world->party.leader_x = 11;
    world->party.leader_y = 12;
    world->party.gold = 777U;

    seed_champion(&world->party.champions[0], 20, 40, 30, 60, 15, 30, 1);
    seed_champion(&world->party.champions[1], 90, 100, 80, 100, 0, 50, 1);
}

int main(void)
{
    Theron_V1_World world;
    uint8_t before[sizeof(world)];
    uint8_t fb_a[FB_W * FB_H];
    uint8_t fb_b[FB_W * FB_H];
    Theron_V2_HudOverlay hud;

    printf("=== Theron V2 HUD overlay seed gate probe ===\n");

    seed_world(&world);
    memcpy(before, &world, sizeof(world));

    printf("\n[ V1 original gate ]\n");
    memset(&hud, 0xA5, sizeof(hud));
    check("V1 gate returns V1_SKIPPED",
          theron_v2_hud_seed_from_v1_world(&hud, &world, 0) ==
              THERON_V2_HUD_SEED_V1_SKIPPED);
    check("V1 gate hides overlay",
          hud.visible == false && hud.opacity == 0);
    check("V1 gate disables top/stats/action sub-surfaces",
          hud.top_bar_visible == false &&
          hud.stats_bar_visible == false &&
          hud.action_strip.visible == false);
    check("V1 gate does not mutate V1 world",
          memcmp(before, &world, sizeof(world)) == 0);
    memset(fb_a, 0, sizeof(fb_a));
    theron_v2_hud_render(&hud, fb_a, FB_W, FB_H);
    check("V1 hidden overlay render writes no pixels",
          fb_nonzero_count(fb_a, sizeof(fb_a)) == 0);

    printf("\n[ V2 seed snapshot ]\n");
    check("V2 gate returns V2_READY",
          theron_v2_hud_seed_from_v1_world(&hud, &world, 1) ==
              THERON_V2_HUD_SEED_V2_READY);
    check("V2 gate keeps V1 world byte-identical",
          memcmp(before, &world, sizeof(world)) == 0);
    check("direction seeded from V1 leader_dir",
          hud.compass.direction == 2);
    check("dungeon progress prefers progression.current_dungeon",
          hud.dungeon_progress.current_dungeon == THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
          hud.dungeon_progress.total_dungeons == THERON_DUNGEON_COUNT);
    check("quest item counter seeded from current dungeon progress",
          hud.quest_items.collected == 2 && hud.quest_items.total == 5);
    check("relic counter counts the 7-bit collected mask",
          hud.relic_counter.relics_found == 3 &&
          hud.relic_counter.relics_required == THERON_QUEST_ITEM_COUNT);
    check("action strip defaults to MOVE",
          hud.action_strip.icons[THERON_V2_ACTION_MOVE].active == true &&
          hud.action_strip.icons[THERON_V2_ACTION_ATTACK].active == false);
    check("champion 0 bars use V1 percentages",
          hud.champion_bars[0].hp_pct == 50 &&
          hud.champion_bars[0].stamina_pct == 50 &&
          hud.champion_bars[0].mana_pct == 50 &&
          hud.champion_bars[0].leader == false &&
          hud.champion_bars[0].spell_ready == true);
    check("champion 1 active slot is leader and has no spell-ready mana",
          hud.champion_bars[1].leader == true &&
          hud.champion_bars[1].hp_pct == 90 &&
          hud.champion_bars[1].mana_pct == 0 &&
          hud.champion_bars[1].spell_ready == false);
    check("empty champion slots stay zeroed",
          hud.champion_bars[2].hp_pct == 0 &&
          hud.champion_bars[3].hp_pct == 0);

    memset(fb_a, 0, sizeof(fb_a));
    memset(fb_b, 0, sizeof(fb_b));
    theron_v2_hud_render(&hud, fb_a, FB_W, FB_H);
    check("V2 seeded overlay paints pixels",
          fb_nonzero_count(fb_a, sizeof(fb_a)) > 0);
    check("reseed does not mutate V1 world",
          theron_v2_hud_seed_from_v1_world(&hud, &world, 1) ==
              THERON_V2_HUD_SEED_V2_READY &&
          memcmp(before, &world, sizeof(world)) == 0);
    theron_v2_hud_render(&hud, fb_b, FB_W, FB_H);
    check("V2 seeded render is deterministic for this snapshot",
          memcmp(fb_a, fb_b, sizeof(fb_a)) == 0);

    printf("\n[ Names and invalid inputs ]\n");
    check("gate name V1_SKIPPED",
          strcmp(theron_v2_hud_seed_gate_name(THERON_V2_HUD_SEED_V1_SKIPPED),
                 "V1_SKIPPED") == 0);
    check("gate name V2_READY",
          strcmp(theron_v2_hud_seed_gate_name(THERON_V2_HUD_SEED_V2_READY),
                 "V2_READY") == 0);
    check("NULL overlay returns INVALID",
          theron_v2_hud_seed_from_v1_world(NULL, &world, 1) ==
              THERON_V2_HUD_SEED_INVALID);
    check("NULL world returns INVALID",
          theron_v2_hud_seed_from_v1_world(&hud, NULL, 1) ==
              THERON_V2_HUD_SEED_INVALID);
    check("unknown gate name is stable",
          strcmp(theron_v2_hud_seed_gate_name((Theron_V2_HudSeedGate)99),
                 "UNKNOWN") == 0);

    printf("\n=== Results: %d passed, %d failed ===\n", s_pass, s_fail);
    return s_fail ? 1 : 0;
}

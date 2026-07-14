#include <stdio.h>
#include <string.h>

#include "redmcsb_f0804_draw_magic_map_pc34_compat.h"

typedef struct TestState {
    RedmcsbF0804Square_Compat square;
    int icons[32];
    int iconCount;
    int fillCount;
    int fillColor;
    int random;
    int16_t next[16];
    int16_t types[16];
    int16_t slots[16];
    int16_t aspects[16];
    int16_t explosion[16];
} TestState;

static int get_square(void *ctx, int16_t x, int16_t y, int16_t direction,
                      RedmcsbF0804Square_Compat *out) {
    TestState *state = ctx;
    (void)x;
    (void)y;
    (void)direction;
    *out = state->square;
    return 1;
}
static void fill(void *ctx, int16_t x, int16_t y, int16_t width, int16_t height, int16_t color) {
    TestState *state = ctx;
    (void)x; (void)y; (void)width; (void)height;
    ++state->fillCount;
    state->fillColor = color;
}
static void icon(void *ctx, int16_t value, int16_t x, int16_t y) {
    TestState *state = ctx;
    (void)x; (void)y;
    state->icons[state->iconCount++] = value;
}
static int16_t random_four(void *ctx) { return (int16_t)((TestState *)ctx)->random; }
static int16_t next_thing(void *ctx, int16_t thing) { return ((TestState *)ctx)->next[thing]; }
static int16_t type(void *ctx, int16_t thing) { return ((TestState *)ctx)->types[thing]; }
static int16_t slot(void *ctx, int16_t thing) { return ((TestState *)ctx)->slots[thing]; }
static int16_t aspect(void *ctx, int16_t value) { return ((TestState *)ctx)->aspects[value]; }
static int16_t explosion(void *ctx, int16_t thing) { return ((TestState *)ctx)->explosion[thing]; }

static int expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    TestState state;
    RedmcsbF0804Request_Compat request;
    RedmcsbF0804Callbacks_Compat callbacks;
    memset(&state, 0, sizeof(state));
    memset(&request, 0, sizeof(request));
    memset(&callbacks, 0, sizeof(callbacks));
    state.square.element = REDMCSB_F0804_ELEMENT_WALL;
    state.square.square = (6 << 5) | 1;
    state.square.firstThing = REDMCSB_F0804_THING_END_OF_LIST;
    state.random = 3;
    request.x = 3; request.y = 4; request.partyX = 3; request.partyY = 4;
    request.iconX = 20; request.iconY = 30; request.iconWidth = 7; request.iconHeight = 7;
    request.zoneMarginX = 1; request.zoneMarginY = 2; request.gameTime = 2;
    request.champion.ninjaSpellCount = 1;
    callbacks.context = &state; callbacks.getSquare = get_square; callbacks.fillZone = fill;
    callbacks.drawIcon = icon; callbacks.randomFour = random_four; callbacks.getNextThing = next_thing;
    callbacks.getThingType = type; callbacks.getProjectileSlot = slot;
    callbacks.getProjectileAspect = aspect; callbacks.getExplosionType = explosion;
    if (!expect(F0804_DrawMagicMap_Compat(&request, &callbacks) == 1, "dispatch succeeds") ||
        !expect(state.fillCount == 1 && state.fillColor == 8, "party marker is source red") ||
        !expect(state.iconCount == 1 && state.icons[0] == 1, "imaginary wall blink uses icon one")) return 1;

    memset(&state, 0, sizeof(state));
    state.square.element = REDMCSB_F0804_ELEMENT_TELEPORTER;
    state.square.firstThing = 1;
    state.types[1] = REDMCSB_F0804_THING_PROJECTILE;
    state.next[1] = 2;
    state.types[2] = REDMCSB_F0804_THING_EXPLOSION;
    state.next[2] = REDMCSB_F0804_THING_END_OF_LIST;
    state.slots[1] = 1;
    state.aspects[1] = -11;
    state.explosion[2] = 6;
    state.random = 3;
    request.partyX = -1; request.partyY = -1; request.gameTime = 0;
    request.champion.ninjaSpellCount = 0; request.champion.wizardSpellCount = 1;
    callbacks.context = &state;
    if (!expect(F0804_DrawMagicMap_Compat(&request, &callbacks) == 1, "spell overlay dispatch succeeds") ||
        !expect(state.iconCount == 3, "teleporter plus projectile plus explosion") ||
        !expect(state.icons[0] == 9, "teleporter uses M004_RANDOM(4)+6") ||
        !expect(state.icons[1] == 51, "fireball source icon includes thing cell") ||
        !expect(state.icons[2] == 71, "poison explosion source icon uses random range")) return 1;

    puts("PASS redmcsb F0804 DrawMagicMap source lock");
    return 0;
}

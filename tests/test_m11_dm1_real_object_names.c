#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int rect_is_color(const unsigned char *framebuffer,
                         int x0, int y0, int width, int height,
                         unsigned char color)
{
    int x;
    int y;

    if (!framebuffer) return 0;
    for (y = y0; y < y0 + height; ++y) {
        for (x = x0; x < x0 + width; ++x) {
            if (framebuffer[y * 320 + x] != color) return 0;
        }
    }
    return 1;
}

int main(void)
{
    /* The object-name table and SND3 bank below are PC 3.4 source material.
     * Do not let an arbitrary data-root select a nearby unpacked edition: the
     * real-media gate must name the authenticated archive explicitly. */
    const char *data_dir = getenv("FIRESTAFF_DM1_PC34_ARCHIVE");
    M11_GameViewState state;
    unsigned short thing;
    char name[64];
    unsigned char framebuffer[320 * 200];
    unsigned char foreign_font[M11_FONT_BITMAP_BYTES];
    size_t cursorPixels = 0u;
    int slotX = 0;
    int slotY = 0;
    int slotW = 0;
    int slotH = 0;
    int statusSourceX = 0;
    int statusSourceY = 0;
    int statusSourceW = 0;
    int statusSourceH = 0;
    int statusDestinationX = 0;
    int statusDestinationY = 0;
    int statusDestinationW = 0;
    int statusDestinationH = 0;
    int y;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_DM1_PC34_ARCHIVE is not set");
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        fprintf(stderr, "DM1 real-data launch failed: %s\n", data_dir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!state.dm1ObjectNameTableValid) {
        fprintf(stderr, "DM1 real GRAPHICS.DAT did not bind the M564 name table\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (state.audioState.originalSnd3LoadedCount != M11_AUDIO_ORIGINAL_SOUND_COUNT) {
        fprintf(stderr,
                "DM1 real GRAPHICS.DAT did not bind all SND3 sounds: %d/%d\n",
                state.audioState.originalSnd3LoadedCount,
                M11_AUDIO_ORIGINAL_SOUND_COUNT);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!state.world.things || state.world.things->weaponCount <= 0) {
        fprintf(stderr, "DM1 real DUNGEON.DAT contains no weapon record\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ReDMCSB OBJECT.C F0031/F0033 resolves the visible name from the
     * icon-indexed M564 stream, not from the decoded subtype catalog. */
    thing = (unsigned short)(THING_TYPE_WEAPON << 10);
    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(&state, thing)) {
        fprintf(stderr, "could not move first real weapon to leader hand\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(name, 0, sizeof(name));
    if (!DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(
            &state, name, (int)sizeof(name)) || name[0] == '\0' ||
        strncmp(name, "WEAPON ", 7) == 0) {
        fprintf(stderr, "real M564 leader-hand name invalid: '%s'\n", name);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Exercise the real C546 eye route after M564, C020/C029, and M653
     * have all been bound from this exact archive.  This is deliberately
     * not a fixture-name test: the object description must remain tied to
     * the admitted live Thing and original source material. */
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;
    state.inventoryPanelActive = 1;
    if (M11_GameView_HandlePointer(&state, 20, 54, 1) !=
            M11_GAME_INPUT_REDRAW ||
        !state.v1ObjectDescriptionPanelActive ||
        state.v1ObjectDescriptionThing != thing ||
        !state.v1ObjectDescriptionSourceMaterialValid ||
        !M11_GameView_IsDialogOverlayActive(&state)) {
        fprintf(stderr, "real C546 eye route did not admit original object panel\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (rect_is_color(framebuffer, 48, 33, 144, 73, 0u)) {
        fprintf(stderr, "real C546 object-description panel did not draw\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!M11_GameView_DismissDialogOverlay(&state)) {
        fprintf(stderr, "real C546 object-description overlay did not dismiss\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(&state, thing)) {
        fprintf(stderr, "could not restore real weapon after C546 eye route\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* OBJECT.C F0034 writes the M564-selected name into C017 with M653.
     * A byte-identical 768-byte raster from another GRAPHICS.DAT identity
     * must not appear as a plausible source-owned object label. */
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (rect_is_color(framebuffer, 233, 33, 87, 6, 0u)) {
        fprintf(stderr, "real F0034 M653 leader-hand name did not draw\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memcpy(foreign_font, state.originalFont.bitmap, sizeof(foreign_font));
    if (!M11_Font_LoadFromRawBitmap(&state.originalFont, 694,
                                    foreign_font, sizeof(foreign_font))) {
        fprintf(stderr, "foreign F0034 M653 fixture did not load\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.originalFontAvailable = 1;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (!rect_is_color(framebuffer, 233, 33, 87, 6, 0u)) {
        fprintf(stderr, "foreign font drew the F0034 source name\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                      state.assetLoader.fileState,
                                      state.assetLoader.runtimeState)) {
        fprintf(stderr, "PC34 M653 font could not be restored\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.originalFontAvailable = 1;

    /* ReDMCSB IO.C F0702 replaces the host arrow with the held source
     * object. Verify the final indexed framebuffer, not merely the transient
     * leader-hand state or the name resolver. */
    memset(framebuffer, 0, sizeof(framebuffer));
    state.pointerPositionKnown = 1;
    state.pointerX = 120;
    state.pointerY = 80;
    M11_GameView_DrawLeaderHandCursor(&state, framebuffer,
                                      320, 200);
    for (y = state.pointerY - 6;
         y < state.pointerY - 6 + 18 && y < 200; ++y) {
        int x;
        for (x = state.pointerX - 8;
             x < state.pointerX - 8 + 18 && x < 320; ++x) {
            if (framebuffer[y * 320 + x] != 0u) {
                ++cursorPixels;
            }
        }
    }
    if (cursorPixels == 0u) {
        fprintf(stderr, "real F0702 held-object cursor did not write pixels\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    /* ReDMCSB CLIKVIEW.C F0373 puts a floor pickup in the transient mouse
     * hand first.  Use a real decoded weapon record to verify the keyboard
     * pickup route does not skip that hand and silently write an inventory
     * slot. */
    state.world.party.championCount = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].hp.current = 100;
    state.world.party.champions[0].hp.maximum = 100;

    /* ReDMCSB CLIKVIEW.C F0374 must also be reachable from the generic drop
     * command: a real object held by G4055 is not a champion inventory slot. */
    if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(&state, thing) ||
        !M11_GameView_DropItem(&state) ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != THING_NONE ||
        !M11_GameView_PickupItem(&state) ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing) {
        fprintf(stderr, "real leader-hand DropItem route failed detail=%s\n",
                state.inspectDetail);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);

    state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] = thing;
    {
        int dropped = M11_GameView_DropItem(&state);
        if (!dropped ||
            state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] !=
                THING_NONE ||
            !M11_GameView_PickupItem(&state) ||
            DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing) {
            fprintf(stderr,
                    "real floor pickup skipped the source mouse-hand route drop=%d map=%d x=%d y=%d detail=%s\n",
                    dropped, state.world.party.mapIndex, state.world.party.mapX,
                    state.world.party.mapY, state.inspectDetail);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    /* The actual SDL path sends a button release after this C508 action-hand
     * click.  The exchange itself belongs to the press.  Repeating it at
     * release used to return the real object to the slot (or make it appear
     * thrown), which is the inventory-loss symptom reported for HoC.  Keep
     * the regression tied to the admitted PC 3.4 object record and the
     * original C507..C536 geometry, rather than a stand-in object corpus. */
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
    state.inventoryPanelActive = 1;
    state.world.party.activeChampionIndex = 0;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = thing;
    if (!M11_GameView_GetV1InventorySourceSlotBoxZone(
            9, &slotX, &slotY, &slotW, &slotH) ||
        M11_GameView_HandlePointer(&state, slotX + slotW / 2,
                                   33 + slotY + slotH / 2, 1) !=
            M11_GAME_INPUT_REDRAW ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing ||
        state.world.party.champions[0]
                 .inventory[CHAMPION_SLOT_ACTION_HAND] != THING_NONE ||
        M11_GameView_HandlePointerButtonRelease(
            &state, slotX + slotW / 2, 33 + slotY + slotH / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != thing ||
        state.world.party.champions[0]
                 .inventory[CHAMPION_SLOT_ACTION_HAND] != THING_NONE) {
        fprintf(stderr,
                "real PC3.4 C508 press/release repeated its inventory exchange\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* C211..C218 are a different F0302 route from C507..C536.  Bind the
     * press/release hand-to-hand move to the same admitted PC 3.4 Thing so
     * an ordinary HoC placement cannot regress into a drop, a swap-back, or
     * a host-only inventory shortcut. */
    DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
    state.world.party.championCount = 2;
    state.world.party.champions[1].present = 1;
    state.world.party.champions[1].hp.current = 100;
    state.world.party.champions[1].hp.maximum = 100;
    state.world.party.champions[1].inventory[CHAMPION_SLOT_HAND_LEFT] =
        THING_NONE;
    state.world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] = thing;
    if (!M11_GameView_GetV1StatusHandIconZone(
            1, 1, &statusSourceX, &statusSourceY,
            &statusSourceW, &statusSourceH) ||
        !M11_GameView_GetV1StatusHandIconZone(
            1, 0, &statusDestinationX, &statusDestinationY,
            &statusDestinationW, &statusDestinationH) ||
        M11_GameView_HandlePointer(
            &state, statusSourceX + statusSourceW / 2,
            statusSourceY + statusSourceH / 2, 1) != M11_GAME_INPUT_REDRAW ||
        M11_GameView_HandlePointerButtonRelease(
            &state, statusDestinationX + statusDestinationW / 2,
            statusDestinationY + statusDestinationH / 2,
            DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
        state.world.party.champions[1]
                 .inventory[CHAMPION_SLOT_HAND_LEFT] != thing ||
        state.world.party.champions[1]
                 .inventory[CHAMPION_SLOT_ACTION_HAND] != THING_NONE ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != THING_NONE) {
        fprintf(stderr,
                "real PC3.4 C211..C218 hand placement did not commit once\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    M11_GameView_Shutdown(&state);
    printf("ok: real DM1 M564 leader-hand name = %s; F0702 cursor pixels=%zu; floor pickup -> mouse hand\n",
           name, cursorPixels);
    return 0;
}

#include "entrance_frontend_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define FB_W 320u
#define FB_H 200u
#define DOOR_W 128u
#define DOOR_H 161u

static unsigned char pixel_at(const unsigned char* pixels,
                              unsigned int w,
                              unsigned int x,
                              unsigned int y) {
    return pixels[y * w + x];
}

static void fill_viewport(unsigned char* frame, unsigned char value) {
    unsigned int x;
    unsigned int y;
    for (y = 33u; y < 33u + 136u; ++y) {
        for (x = 0u; x < 224u; ++x) {
            frame[y * FB_W + x] = value;
        }
    }
}

static void fill_door(unsigned char* door, unsigned char base) {
    unsigned int x;
    unsigned int y;
    for (y = 0u; y < DOOR_H; ++y) {
        for (x = 0u; x < DOOR_W; ++x) {
            door[y * DOOR_W + x] = (unsigned char)(base + (x & 15u));
        }
    }
}

int main(void) {
    unsigned char framebuffer[FB_W * FB_H];
    unsigned char entrance[FB_W * FB_H];
    unsigned char dungeon[FB_W * FB_H];
    unsigned char leftDoor[DOOR_W * DOOR_H];
    unsigned char rightDoor[DOOR_W * DOOR_H];
    EntranceCompatDoorStep door;
    EntranceCompatCompositePixels pixels;
    int ok = 1;

    memset(framebuffer, 0xee, sizeof(framebuffer));
    memset(entrance, 1, sizeof(entrance));
    memset(dungeon, 9, sizeof(dungeon));
    fill_viewport(dungeon, 7);
    fill_door(leftDoor, 32);
    fill_door(rightDoor, 64);

    ok &= ENTRANCE_Compat_GetDoorAnimationStep(16u, &door);
    memset(&pixels, 0, sizeof(pixels));
    pixels.entranceScreen = entrance;
    pixels.entranceWidth = FB_W;
    pixels.entranceHeight = FB_H;
    pixels.dungeonFrame = dungeon;
    pixels.dungeonFrameWidth = FB_W;
    pixels.dungeonFrameHeight = FB_H;
    pixels.leftDoor = leftDoor;
    pixels.leftDoorWidth = DOOR_W;
    pixels.leftDoorHeight = DOOR_H;
    pixels.rightDoor = rightDoor;
    pixels.rightDoorWidth = DOOR_W;
    pixels.rightDoorHeight = DOOR_H;

    ok &= ENTRANCE_Compat_CompositeDoorOpeningFrame(framebuffer,
                                                    FB_W,
                                                    FB_H,
                                                    &pixels,
                                                    &door);

    if (pixel_at(framebuffer, FB_W, 250u, 40u) != 1u) {
        fprintf(stderr, "right-side entrance background leaked: got %u want 1\n",
                (unsigned int)pixel_at(framebuffer, FB_W, 250u, 40u));
        ok = 0;
    }
    if (pixel_at(framebuffer, FB_W, 80u, 50u) != 7u) {
        fprintf(stderr, "door aperture did not show viewport: got %u want 7\n",
                (unsigned int)pixel_at(framebuffer, FB_W, 80u, 50u));
        ok = 0;
    }
    if (pixel_at(framebuffer, FB_W, 10u, 50u) !=
        pixel_at(leftDoor, DOOR_W, door.leftSourceX + 10u, 22u)) {
        fprintf(stderr, "left door strip mismatch\n");
        ok = 0;
    }
    if (pixel_at(framebuffer, FB_W, 180u, 50u) !=
        pixel_at(rightDoor, DOOR_W, door.rightSourceX + (180u - door.rightBoxX), 22u)) {
        fprintf(stderr, "right door strip mismatch\n");
        ok = 0;
    }

    printf("entranceDoorOpeningCompositeOk=%d\n", ok);
    return ok ? 0 : 1;
}

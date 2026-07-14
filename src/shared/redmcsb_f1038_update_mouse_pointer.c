#include "redmcsb_f1038_update_mouse_pointer.h"

static void redmcsb_f1038_change_all_sprites(
    redmcsb_f1038_update_mouse_pointer_state *state,
    const uint16_t *images)
{
    uint16_t sprite_index;

    for (sprite_index = 0; sprite_index < REDMCSB_F1038_SPRITE_COUNT;
         sprite_index++) {
        state->change_sprite(
            state->context, sprite_index,
            images + (sprite_index * REDMCSB_F1038_SPRITE_IMAGE_WORDS));
    }
}

void redmcsb_f1038_update_mouse_pointer(
    redmcsb_f1038_update_mouse_pointer_state *state)
{
    int16_t mouse_x;
    int16_t mouse_y;
    int16_t pointer_type;
    long sprite_x;
    long sprite_y;

    if (!state->active) {
        return;
    }

    mouse_x = state->mouse_x;
    mouse_y = state->mouse_y;
    pointer_type = state->get_pointer_type(state->context, mouse_x, mouse_y);

    if (pointer_type != REDMCSB_F1038_POINTER_NONE &&
        state->pointer_type != pointer_type) {
        redmcsb_f1038_change_all_sprites(state, state->sprite_images);
    }

    if (state->last_mouse_x != mouse_x || state->last_mouse_y != mouse_y ||
        state->pointer_type != pointer_type) {
        state->last_mouse_x = mouse_x;
        state->last_mouse_y = mouse_y;
        sprite_x = (long)mouse_x - 1L -
                   (long)state->hotspots[pointer_type + 1][0];
        sprite_y = (long)mouse_y -
                   (long)state->hotspots[pointer_type + 1][1];
        state->move_sprite(state->context, 0, sprite_x, sprite_y);
        state->move_sprite(state->context, 1, sprite_x, sprite_y);
        state->move_sprite(state->context, 2,
                           sprite_x + 16L, sprite_y);
        state->move_sprite(state->context, 3,
                           sprite_x + 16L, sprite_y);
    }

    if (state->pointer_type != pointer_type) {
        redmcsb_f1038_change_all_sprites(
            state,
            state->sprite_images +
                ((pointer_type + 1) * REDMCSB_F1038_POINTER_BANK_WORDS));
        state->pointer_type = pointer_type;
    }
}

const char *redmcsb_f1038_update_mouse_pointer_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/IO.C:785-1071 defines "
           "F1038_UpdateMousePointer. The portable Amiga branch at "
           "IO.C:1041-1071 obtains G3114_/G3112_, calls "
           "F1041_GetMousePointerType, conditionally loads G3203_ base "
           "images, moves sprites 0-3 with G3115_[type + 1] offsets, then "
           "loads bank (type + 1) * 160 and records "
           "G3107_i_GetMousePointerType. IO.C:502-509 defines the "
           "G3115_ no-pointer and pointer hotspot rows; AMIGA.H:91-97 "
           "defines the 40-word SPRITEIMAGE layout.";
}

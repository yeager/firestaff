#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f1038_update_mouse_pointer.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int16_t resolved_pointer_type;
    unsigned int change_count;
    unsigned int move_count;
    uint16_t changed_sprites[8];
    const uint16_t *changed_images[8];
    uint16_t moved_sprites[4];
    long moved_x[4];
    long moved_y[4];
} test_context;

static int16_t get_pointer_type(void *context, int16_t x, int16_t y)
{
    (void)y;
    (void)x;
    test_context *test = context;

    assert(x == 80);
    assert(y == 40);
    return test->resolved_pointer_type;
}

static void change_sprite(void *context, uint16_t sprite_index,
                          const uint16_t *image)
{
    test_context *test = context;
    unsigned int call = test->change_count++;

    assert(call < 8U);
    test->changed_sprites[call] = sprite_index;
    test->changed_images[call] = image;
}

static void move_sprite(void *context, uint16_t sprite_index, long x, long y)
{
    test_context *test = context;
    unsigned int call = test->move_count++;

    assert(call < 4U);
    test->moved_sprites[call] = sprite_index;
    test->moved_x[call] = x;
    test->moved_y[call] = y;
}

int main(void)
{
    int16_t hotspots[3][4] = {{3, 5, 0, 0}, {7, 11, 0, 0}, {13, 17, 0, 0}};
    uint16_t images[3 * REDMCSB_F1038_POINTER_BANK_WORDS];
    test_context test;
    redmcsb_f1038_update_mouse_pointer_state state;
    const char *evidence;
    (void)evidence;
    unsigned int index;

    memset(&test, 0, sizeof(test));
    memset(images, 0, sizeof(images));
    test.resolved_pointer_type = 1;
    for (index = 0; index < 3U; index++) {
        images[index * REDMCSB_F1038_POINTER_BANK_WORDS] = (uint16_t)index;
    }
    state.active = true;
    state.mouse_x = 80;
    state.mouse_y = 40;
    state.pointer_type = 0;
    state.last_mouse_x = -1;
    state.last_mouse_y = -1;
    state.hotspots = hotspots;
    state.sprite_images = images;
    state.get_pointer_type = get_pointer_type;
    state.change_sprite = change_sprite;
    state.move_sprite = move_sprite;
    state.context = &test;

    redmcsb_f1038_update_mouse_pointer(&state);

    assert(test.change_count == 8U);
    for (index = 0; index < 4U; index++) {
        assert(test.changed_sprites[index] == index);
        assert(test.changed_images[index] ==
               images + (index * REDMCSB_F1038_SPRITE_IMAGE_WORDS));
        assert(test.changed_sprites[index + 4U] == index);
        assert(test.changed_images[index + 4U] ==
               images + (2 * REDMCSB_F1038_POINTER_BANK_WORDS) +
                            (index * REDMCSB_F1038_SPRITE_IMAGE_WORDS));
    }
    assert(test.move_count == 4U);
    assert(test.moved_sprites[0] == 0U);
    assert(test.moved_sprites[1] == 1U);
    assert(test.moved_sprites[2] == 2U);
    assert(test.moved_sprites[3] == 3U);
    assert(test.moved_x[0] == 66L);
    assert(test.moved_y[0] == 23L);
    assert(test.moved_x[1] == 66L);
    assert(test.moved_y[1] == 23L);
    assert(test.moved_x[2] == 82L);
    assert(test.moved_y[2] == 23L);
    assert(test.moved_x[3] == 82L);
    assert(test.moved_y[3] == 23L);
    assert(state.pointer_type == 1);
    assert(state.last_mouse_x == 80);
    assert(state.last_mouse_y == 40);

    memset(&test, 0, sizeof(test));
    test.resolved_pointer_type = REDMCSB_F1038_POINTER_NONE;
    redmcsb_f1038_update_mouse_pointer(&state);
    assert(test.change_count == 4U);
    assert(test.move_count == 4U);
    for (index = 0; index < 4U; index++) {
        assert(test.changed_sprites[index] == index);
        assert(test.changed_images[index] ==
               images + (index * REDMCSB_F1038_SPRITE_IMAGE_WORDS));
    }
    assert(state.pointer_type == REDMCSB_F1038_POINTER_NONE);

    evidence = redmcsb_f1038_update_mouse_pointer_source_evidence();
    assert(strstr(evidence, "IO.C:1041-1071") != NULL);
    assert(strstr(evidence, "IO.C:502-509") != NULL);
    assert(strstr(evidence, "AMIGA.H:91-97") != NULL);
    puts("ok: ReDMCSB F1038 Amiga mouse-pointer update sequence");
    return 0;
}

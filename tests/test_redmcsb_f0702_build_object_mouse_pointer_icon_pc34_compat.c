#include "redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static __attribute__((unused)) uint8_t get_pixel(const uint8_t *bitmap, size_t width, size_t x, size_t y)
{
    const uint8_t value = bitmap[(y * (width / 2U)) + (x / 2U)];

    return (x & 1U) == 0U ? (uint8_t)(value >> 4) : (uint8_t)(value & 0x0fU);
}

static void set_pixel(uint8_t *bitmap, size_t width, size_t x, size_t y, uint8_t color)
{
    uint8_t *const value = &bitmap[(y * (width / 2U)) + (x / 2U)];

    if ((x & 1U) == 0U) {
        *value = (uint8_t)((*value & 0x0fU) | (color << 4));
    } else {
        *value = (uint8_t)((*value & 0xf0U) | color);
    }
}

static void test_builds_pc34_shadow_then_object(void)
{
    uint8_t object[REDMCSB_F0702_OBJECT_BITMAP_BYTES_PC34];
    uint8_t pointer[REDMCSB_F0702_POINTER_BITMAP_BYTES_PC34];
    (void)pointer;

    memset(object, 0xcc, sizeof(object));
    set_pixel(object, 16U, 0U, 0U, 5U);
    set_pixel(object, 16U, 1U, 0U, 12U);
    set_pixel(object, 16U, 15U, 15U, 7U);

    assert(redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat(
        object, sizeof(object), pointer, sizeof(pointer)));
    /* IO.C fills 18x18 with color C12 before either F0132 blit. */
    assert(get_pixel(pointer, 18U, 17U, 0U) == 12U);
    /* The object is on top and its C12 pixels are transparent. */
    assert(get_pixel(pointer, 18U, 0U, 0U) == 5U);
    assert(get_pixel(pointer, 18U, 1U, 0U) == 12U);
    assert(get_pixel(pointer, 18U, 15U, 15U) == 7U);
    /* Shadow mapping makes non-C12 pixels zero, but C12 transparent. */
    assert(get_pixel(pointer, 18U, 2U, 2U) == 0U);
    assert(get_pixel(pointer, 18U, 3U, 2U) == 12U);
    assert(get_pixel(pointer, 18U, 17U, 17U) == 0U);
}

static void test_rejects_undersized_input_without_writing_target(void)
{
    uint8_t object[REDMCSB_F0702_OBJECT_BITMAP_BYTES_PC34] = { 0 };
    (void)object;
    uint8_t pointer[REDMCSB_F0702_POINTER_BITMAP_BYTES_PC34];
    uint8_t before[sizeof(pointer)];

    memset(pointer, 0xa5, sizeof(pointer));
    memcpy(before, pointer, sizeof(pointer));
    assert(!redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat(
        object, sizeof(object) - 1U, pointer, sizeof(pointer)));
    assert(memcmp(pointer, before, sizeof(pointer)) == 0);
    assert(!redmcsb_f0702_build_object_mouse_pointer_icon_pc34_compat(
        object, sizeof(object), pointer, sizeof(pointer) - 1U));
    assert(memcmp(pointer, before, sizeof(pointer)) == 0);
}

int main(void)
{
    test_builds_pc34_shadow_then_object();
    test_rejects_undersized_input_without_writing_target();
    return 0;
}

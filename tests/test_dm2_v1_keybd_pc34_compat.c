/*
 * test_dm2_v1_keybd_pc34_compat.c — unit tests for DM2 keyboard handling.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_keybd_pc34_compat.h"

static void test_init(void)
{
    DM2_V1_Keybd kbd;
    memset(&kbd, 0xFF, sizeof(kbd));
    dm2_v1_keybd_init(&kbd);
    assert(kbd.queueidx_in == 0);
    assert(kbd.queueidx_out == 0);
    assert(kbd.queuecnt == 0);
    assert(!dm2_v1_has_key(&kbd));
    printf("  PASS test_init\n");
}

static void test_putkey_getkey(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);
    /* Put Allegro key 0x01 (KEY_A), should translate to scancode 0x1e */
    dm2_v1_keybd_putkey(&kbd, 0x01);
    assert(dm2_v1_has_key(&kbd));
    int16_t key = dm2_v1_keybd_getkey(&kbd);
    assert(key == 0x1e);
    assert(!dm2_v1_has_key(&kbd));
    printf("  PASS test_putkey_getkey\n");
}

static void test_queue_full(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);
    for (int i = 0; i < DM2_V1_KBD_QUEUE_LENGTH; i++)
        dm2_v1_keybd_putkey(&kbd, 0x01);
    assert(kbd.queuecnt == DM2_V1_KBD_QUEUE_LENGTH);
    /* 11th should be dropped */
    dm2_v1_keybd_putkey(&kbd, 0x02);
    assert(kbd.queuecnt == DM2_V1_KBD_QUEUE_LENGTH);
    printf("  PASS test_queue_full\n");
}

static void test_queue_wrap(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);
    /* Fill and drain several times to wrap */
    for (int cycle = 0; cycle < 3; cycle++) {
        for (int i = 0; i < DM2_V1_KBD_QUEUE_LENGTH; i++)
            dm2_v1_keybd_putkey(&kbd, 0x01);
        for (int i = 0; i < DM2_V1_KBD_QUEUE_LENGTH; i++) {
            int16_t k = dm2_v1_keybd_getkey(&kbd);
            assert(k >= 0);
        }
        assert(!dm2_v1_has_key(&kbd));
    }
    printf("  PASS test_queue_wrap\n");
}

static void test_has_key(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);
    assert(!dm2_v1_has_key(&kbd));
    dm2_v1_keybd_putkey(&kbd, 0x01);
    assert(dm2_v1_has_key(&kbd));
    dm2_v1_keybd_getkey(&kbd);
    assert(!dm2_v1_has_key(&kbd));
    printf("  PASS test_has_key\n");
}

static void test_getkey_translated_cursor_keys(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);

    /* Manually insert pre-translated cursor codes */
    kbd.kbdqueue[0] = 0x1048;
    kbd.kbdqueue[1] = 0x104b;
    kbd.kbdqueue[2] = 0x104d;
    kbd.kbdqueue[3] = 0x1050;
    kbd.kbdqueue[4] = 0x124b;
    kbd.kbdqueue[5] = 0x124d;
    kbd.queuecnt = 6;

    assert(dm2_v1_getkey_translated(&kbd) == 0x4c);
    assert(dm2_v1_getkey_translated(&kbd) == 0x4b);
    assert(dm2_v1_getkey_translated(&kbd) == 0x4d);
    assert(dm2_v1_getkey_translated(&kbd) == 0x50);
    assert(dm2_v1_getkey_translated(&kbd) == 0x4f);
    assert(dm2_v1_getkey_translated(&kbd) == 0x51);
    printf("  PASS test_getkey_translated_cursor_keys\n");
}

static void test_getkey_empty(void)
{
    DM2_V1_Keybd kbd;
    dm2_v1_keybd_init(&kbd);
    assert(dm2_v1_keybd_getkey(&kbd) == -1);
    assert(dm2_v1_getkey_translated(&kbd) == -1);
    printf("  PASS test_getkey_empty\n");
}

static void test_translate_ascii_unshifted(void)
{
    /* Scancode 0x1e = 'a' */
    assert(dm2_v1_keybd_translate_ascii(0x1e) == 'a');
    /* Scancode 0x10 = 'q' */
    assert(dm2_v1_keybd_translate_ascii(0x10) == 'q');
    /* Scancode 0x39 = space */
    assert(dm2_v1_keybd_translate_ascii(0x39) == ' ');
    printf("  PASS test_translate_ascii_unshifted\n");
}

static void test_translate_ascii_shifted(void)
{
    /* Scancode 0x1e with shift flag 0x200 = 'A' */
    assert(dm2_v1_keybd_translate_ascii(0x21e) == 'A');
    /* Scancode 0x02 with shift = '!' */
    assert(dm2_v1_keybd_translate_ascii(0x202) == '!');
    printf("  PASS test_translate_ascii_shifted\n");
}

static void test_translate_ascii_ctrl(void)
{
    /* Scancode 0x1e with ctrl flag 0x400 = 'a' & 0x1f = 0x01 */
    assert(dm2_v1_keybd_translate_ascii(0x41e) == 0x01);
    printf("  PASS test_translate_ascii_ctrl\n");
}

int main(void)
{
    printf("test_dm2_v1_keybd_pc34_compat\n");
    test_init();
    test_putkey_getkey();
    test_queue_full();
    test_queue_wrap();
    test_has_key();
    test_getkey_translated_cursor_keys();
    test_getkey_empty();
    test_translate_ascii_unshifted();
    test_translate_ascii_shifted();
    test_translate_ascii_ctrl();
    printf("All tests passed.\n");
    return 0;
}

#ifndef FIRESTAFF_DM2_V1_KEYBD_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_KEYBD_PC34_COMPAT_H

/*
 * dm2_v1_keybd_pc34_compat.h — DM2 keyboard input handling.
 *
 * Ports c_keybd from skproject c_keybd.cpp: circular key queue,
 * Allegro-to-DOS scancode translation, and ASCII lookup tables.
 *
 * Source: skproject/SKWINSPX/src/v4/c_keybd.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_KBD_QUEUE_LENGTH 10

typedef struct {
    int16_t queueidx_in, queueidx_out, queuecnt;
    int16_t kbdqueue[DM2_V1_KBD_QUEUE_LENGTH];
} DM2_V1_Keybd;

/* Allegro key to DOS scancode mapping */
extern const int16_t dm2_v1_keytab[128];

/* Unshifted and shifted ASCII tables */
extern const uint8_t dm2_v1_DRVB_table1[128];
extern const uint8_t dm2_v1_DRVB_table2[128];

void dm2_v1_keybd_init(DM2_V1_Keybd *kbd);
void dm2_v1_keybd_putkey(DM2_V1_Keybd *kbd, int16_t keycode);
int16_t dm2_v1_keybd_getkey(DM2_V1_Keybd *kbd);  /* non-blocking, returns -1 if empty */
bool dm2_v1_has_key(const DM2_V1_Keybd *kbd);
int16_t dm2_v1_getkey_translated(DM2_V1_Keybd *kbd);
int16_t dm2_v1_keybd_translate_ascii(int16_t scancode);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_KEYBD_PC34_COMPAT_H */

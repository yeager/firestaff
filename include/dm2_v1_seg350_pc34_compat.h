#ifndef FIRESTAFF_DM2_V1_SEG350_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_SEG350_PC34_COMPAT_H

/*
 * dm2_v1_seg350_pc34_compat.h — DM2 segment 350 helpers.
 *
 * Ports c_4b3wp and c_350 init functions from skproject c_350.cpp.
 * These structs hold runtime state for the DM2 presentation layer.
 *
 * Source: skproject/SKWINSPX/src/v4/c_350.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── c_4b3wp entry (nested in c_350) ─────────────────────────────────── */

typedef struct {
    int8_t b_00, b_01, b_02, b_03;
    int16_t w_04, w_06, w_08;
    void *xp_0a;
} DM2_V1_Seg350Entry;

/* ── c_350 main struct ───────────────────────────────────────────────── */

typedef struct {
    int16_t v1e054c;
    void *v1e054e, *v1e0552, *creatures, *v1e055a, *v1e055e;
    uint8_t v1e0562[12];           /* timer struct, 12 bytes */
    int8_t v1e056e, v1e056f, v1e0570, v1e0571;
    int16_t v1e0572, v1e0574, v1e0576, v1e0578;
    int16_t v1e057a, v1e057c, v1e057e, v1e0580;
    int16_t v1e0582, v1e0584, v1e0586;
    void *v1e0588;
    int8_t v1e058c, v1e058d;
    int8_t v1e058e[0x80];
    uint8_t v1e060e[8 * 12];       /* buttons[8], 12 bytes each */
    int8_t v1e066e[5];
    int8_t v1e0673, v1e0674, v1e0675;
    int8_t v1e0676[2];
    uint8_t v1e0678[0x10 * 16];    /* sized structs[16], 16 bytes each */
    DM2_V1_Seg350Entry v1e07d8;
    int16_t *v1e07e6;
    int8_t v1e07ea, v1e07eb, v1e07ec, v1e07ed;
    void *v1e07ee[0x2a];
    uint8_t v1e0896;
    void *v1e0898;
} DM2_V1_Seg350;

/* ── Functions ───────────────────────────────────────────────────────── */

void dm2_v1_seg350_entry_init(DM2_V1_Seg350Entry *e);
void dm2_v1_seg350_init(DM2_V1_Seg350 *s);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_SEG350_PC34_COMPAT_H */

#ifndef FIRESTAFF_DM1_V1_G0487_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_G0487_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 560 init var
 * G0487_as_Graphic560_Spells[M530_SPELL_COUNT] (SPELL struct array).
 *
 * G0487 is the 25-entry spell definition table for PC 3.4 EN. Each
 * SPELL entry is {Symbols (4 bytes), BaseRequiredSkillLevel (1 byte),
 * SkillIndex (1 byte), Attributes (2 bytes)} = 8 bytes/entry × 25
 * = 200 bytes total.
 *
 * Read sites: MENU.C F0452_SPELLS_GetSpellCastResult +
 * F0456_SPELLS_ApplySpell + F0412_MENUS_GetChampionSpellCastResult.
 *
 * Disjoint from pass784+ non-mirror-candidate contract gates.
 * Disjoint from pass784-1034.
 */

#define DM1_V1_G0487_PC34_COMPAT_SIZE 200

typedef struct DM1_V1_G0487ResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_G0487_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int spell0ShieldSymbols;
    int allSkillsInValidRange;
    int allBaseReqInByteRange;
    int allAttrsInUint16Range;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsMinusOne;
} DM1_V1_G0487ResultPc34;

const unsigned char *
dm1_v1_g0487_table_pc34(void);

int
dm1_v1_g0487_size_pc34(void);

int
dm1_v1_g0487_get_pc34(int spell_index, int byte_offset);

int
dm1_v1_g0487_run_pc34(
    DM1_V1_G0487ResultPc34 *out);

#endif

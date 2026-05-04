#ifndef FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_SPELL_LO = 0,
    DM1_SPELL_UM,
    DM1_SPELL_ON,
    DM1_SPELL_EE,
    DM1_SPELL_PAL,
    DM1_SPELL_MON,
    DM1_SPELL_SYMBOL_COUNT
};

enum {
    DM1_SPELL_PRIEST = 0,
    DM1_SPELL_WIZARD,
    DM1_SPELL_CATEGORY_COUNT
};

typedef struct {
    int symbols[4];
    int symbolCount;
    int category;
    int power;
    int manaCost;
    int castTime;
    int championIndex;
} M11_SpellCast;

typedef struct {
    M11_SpellCast pending[4];
    int pendingCount;
    int cooldowns[4];
} M11_SpellState;

void m11_spell_init(M11_SpellState* s);
int m11_spell_add_symbol(M11_SpellState* s, int championIdx, int symbol);
int m11_spell_cast(M11_SpellState* s, int championIdx);
void m11_spell_clear(M11_SpellState* s, int championIdx);
void m11_spell_tick(M11_SpellState* s, int tickMs);
int m11_spell_get_power(const M11_SpellCast* cast);
const char* m11_spell_symbol_name(int symbol);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SPELL_CASTING_PC34_COMPAT_H */
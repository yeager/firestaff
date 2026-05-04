#include "dm1_v1_spell_casting_pc34_compat.h"
#include <string.h>

void m11_spell_init(M11_SpellState* s) {
    if (!s) return;
    memset(s, 0, sizeof(M11_SpellState));
}

int m11_spell_add_symbol(M11_SpellState* s, int championIdx, int symbol) {
    if (!s || championIdx < 0 || championIdx >= 4) return 0;
    
    M11_SpellCast* cast = &s->pending[championIdx];
    
    if (cast->symbolCount < 4) {
        cast->symbols[cast->symbolCount] = symbol;
        cast->symbolCount++;
        return 1;
    }
    
    return 0;
}

int m11_spell_cast(M11_SpellState* s, int championIdx) {
    if (!s || championIdx < 0 || championIdx >= 4) return 0;
    
    if (s->cooldowns[championIdx] > 0) {
        return 0;
    }
    
    M11_SpellCast* cast = &s->pending[championIdx];
    
    int power = 0;
    for (int i = 0; i < cast->symbolCount; i++) {
        power += cast->symbols[i];
    }
    power *= 10;
    
    cast->power = power;
    cast->manaCost = power / 2;
    cast->castTime = power * 5;
    cast->championIndex = championIdx;
    
    s->pendingCount++;
    
    return 1;
}

void m11_spell_clear(M11_SpellState* s, int championIdx) {
    if (!s || championIdx < 0 || championIdx >= 4) return;
    
    memset(&s->pending[championIdx], 0, sizeof(M11_SpellCast));
}

void m11_spell_tick(M11_SpellState* s, int tickMs) {
    if (!s) return;
    
    for (int i = 0; i < 4; i++) {
        if (s->cooldowns[i] > 0) {
            s->cooldowns[i] -= tickMs;
            if (s->cooldowns[i] < 0) {
                s->cooldowns[i] = 0;
            }
        }
        
        if (s->pending[i].castTime > 0) {
            s->pending[i].castTime -= tickMs;
            if (s->pending[i].castTime < 0) {
                s->pending[i].castTime = 0;
            }
        }
    }
}

int m11_spell_get_power(const M11_SpellCast* cast) {
    if (!cast) return 0;
    
    int power = 0;
    for (int i = 0; i < cast->symbolCount; i++) {
        power += cast->symbols[i];
    }
    return power * 10;
}

const char* m11_spell_symbol_name(int symbol) {
    switch (symbol) {
        case DM1_SPELL_LO: return "Lo";
        case DM1_SPELL_UM: return "Um";
        case DM1_SPELL_ON: return "On";
        case DM1_SPELL_EE: return "Ee";
        case DM1_SPELL_PAL: return "Pal";
        case DM1_SPELL_MON: return "Mon";
        default: return "Unknown";
    }
}
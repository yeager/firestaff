
#ifndef FIRESTAFF_SPELL_REF_H
#define FIRESTAFF_SPELL_REF_H

typedef struct {
    const char *name;
    const char *symbols;    /* spell symbol sequence */
    const char *description;
    int power_level;        /* 1-6 */
    const char *class_name; /* Priest/Wizard */
    int mana_cost;
} FS_SpellEntry;

int fs_spell_count(void);
const FS_SpellEntry *fs_spell_get(int index);

#endif


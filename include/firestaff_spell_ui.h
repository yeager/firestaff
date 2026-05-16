
#ifndef FIRESTAFF_SPELL_UI_H
#define FIRESTAFF_SPELL_UI_H
#include <stdint.h>

/* DM1 spell casting UI.
 * Spells are cast by selecting rune symbols:
 *   1. Power rune (Lo/Um/On/Ee/Pal/Mon)
 *   2. Element rune (Ya/Vi/Oh/Ful/Des/Zo)
 *   3. Form rune (optional: Ven/Ew/Kath/Ir/Bro/Gor)
 *   4. Alignment rune (optional: Ku/Ros/Dain/Neta/Ra/Sar)
 *
 * The spell panel shows 6 rune buttons per category.
 * Click sequence builds the spell, then cast with action button. */

#define SPELL_MAX_RUNES 4
#define RUNE_BUTTON_SIZE 16

typedef enum {
    SPELL_STATE_IDLE = 0,
    SPELL_STATE_POWER,     /* selecting power rune */
    SPELL_STATE_ELEMENT,   /* selecting element */
    SPELL_STATE_FORM,      /* selecting form (optional) */
    SPELL_STATE_ALIGN,     /* selecting alignment (optional) */
    SPELL_STATE_READY,     /* spell composed, ready to cast */
} FS_SpellState;

typedef struct {
    FS_SpellState state;
    int power_rune;        /* -1 = not selected */
    int element_rune;
    int form_rune;
    int align_rune;
    int caster_index;      /* which champion is casting */
    int visible;
    int cursor_pos;        /* 0-5 within current rune category */
} FS_SpellUI;

void fs_spell_init(FS_SpellUI *sp);
void fs_spell_open(FS_SpellUI *sp, int champion_index);
void fs_spell_close(FS_SpellUI *sp);
void fs_spell_select_rune(FS_SpellUI *sp, int rune_index);
void fs_spell_cancel_last(FS_SpellUI *sp);
int fs_spell_is_ready(const FS_SpellUI *sp);
void fs_spell_render(const FS_SpellUI *sp, uint8_t *framebuffer);

/* Rune names for display */
const char *fs_spell_power_name(int index);
const char *fs_spell_element_name(int index);

#endif


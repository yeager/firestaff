
#include "firestaff_spell_ui.h"
#include <string.h>
#include <stdio.h>

static const char *g_power_names[] = {"Lo","Um","On","Ee","Pal","Mon"};
static const char *g_element_names[] = {"Ya","Vi","Oh","Ful","Des","Zo"};
static const char *g_form_names[] = {"Ven","Ew","Kath","Ir","Bro","Gor"};
static const char *g_align_names[] = {"Ku","Ros","Dain","Neta","Ra","Sar"};

/* Rune symbol colors */
static const uint8_t g_rune_colors[] = {15, 14, 12, 4, 9, 13};

void fs_spell_init(FS_SpellUI *sp) {
    if (!sp) return;
    memset(sp, 0, sizeof(*sp));
    sp->power_rune = sp->element_rune = sp->form_rune = sp->align_rune = -1;
}

void fs_spell_open(FS_SpellUI *sp, int champion_index) {
    if (!sp) return;
    fs_spell_init(sp);
    sp->caster_index = champion_index;
    sp->visible = 1;
    sp->state = SPELL_STATE_POWER;
}

void fs_spell_close(FS_SpellUI *sp) {
    if (sp) { sp->visible = 0; sp->state = SPELL_STATE_IDLE; }
}

void fs_spell_select_rune(FS_SpellUI *sp, int rune_index) {
    if (!sp || rune_index < 0 || rune_index > 5) return;
    switch (sp->state) {
    case SPELL_STATE_POWER:
        sp->power_rune = rune_index;
        sp->state = SPELL_STATE_ELEMENT;
        break;
    case SPELL_STATE_ELEMENT:
        sp->element_rune = rune_index;
        sp->state = SPELL_STATE_READY; /* form/align optional */
        break;
    case SPELL_STATE_FORM:
        sp->form_rune = rune_index;
        sp->state = SPELL_STATE_READY;
        break;
    case SPELL_STATE_ALIGN:
        sp->align_rune = rune_index;
        sp->state = SPELL_STATE_READY;
        break;
    default: break;
    }
}

void fs_spell_cancel_last(FS_SpellUI *sp) {
    if (!sp) return;
    if (sp->state == SPELL_STATE_READY || sp->align_rune >= 0) {
        sp->align_rune = -1;
        sp->state = (sp->form_rune >= 0) ? SPELL_STATE_READY : SPELL_STATE_FORM;
    } else if (sp->form_rune >= 0) {
        sp->form_rune = -1;
        sp->state = SPELL_STATE_READY;
    } else if (sp->element_rune >= 0) {
        sp->element_rune = -1;
        sp->state = SPELL_STATE_ELEMENT;
    } else if (sp->power_rune >= 0) {
        sp->power_rune = -1;
        sp->state = SPELL_STATE_POWER;
    }
}

int fs_spell_is_ready(const FS_SpellUI *sp) {
    return sp && sp->state == SPELL_STATE_READY;
}

const char *fs_spell_power_name(int i) { return (i>=0&&i<6) ? g_power_names[i] : "?"; }
const char *fs_spell_element_name(int i) { return (i>=0&&i<6) ? g_element_names[i] : "?"; }

/* Render spell panel overlay */
void fs_spell_render(const FS_SpellUI *sp, uint8_t *fb) {
    int i, px, py;
    const char **names = NULL;
    if (!sp || !fb || !sp->visible) return;

    /* Dim lower portion of screen for spell panel overlay */
    for (py = 150; py < 200; py++)
        for (px = 0; px < 320; px++)
            fb[py * 320 + px] = (fb[py * 320 + px] > 3) ? (fb[py * 320 + px] - 3) : 0;

    /* Spell panel at bottom: 6 rune buttons */
    switch (sp->state) {
    case SPELL_STATE_POWER: names = g_power_names; break;
    case SPELL_STATE_ELEMENT: names = g_element_names; break;
    case SPELL_STATE_FORM: names = g_form_names; break;
    case SPELL_STATE_ALIGN: names = g_align_names; break;
    default: break;
    }

    if (!names && sp->state == SPELL_STATE_READY) {
        /* Show "CAST" button */
        int bx = 120, by = 170;
        for (py = by; py < by + 20 && py < 200; py++)
            for (px = bx; px < bx + 80 && px < 320; px++)
                fb[py * 320 + px] = 12; /* red cast button */
        return;
    }

    if (!names) return;

    /* Draw 6 rune buttons in a row */
    for (i = 0; i < 6; i++) {
        int bx = 20 + i * 48;
        int by = 170;
        uint8_t color = g_rune_colors[i];
        uint8_t border = (i == sp->cursor_pos) ? 15 : 7;

        /* Button background */
        for (py = by; py < by + RUNE_BUTTON_SIZE && py < 200; py++)
            for (px = bx; px < bx + RUNE_BUTTON_SIZE * 2 && px < 320; px++)
                fb[py * 320 + px] = color;
        /* Border if selected */
        if (i == sp->cursor_pos) {
            for (px = bx; px < bx + RUNE_BUTTON_SIZE * 2 && px < 320; px++) {
                if (by >= 0) fb[by * 320 + px] = border;
                if (by + RUNE_BUTTON_SIZE - 1 < 200)
                    fb[(by + RUNE_BUTTON_SIZE - 1) * 320 + px] = border;
            }
        }
    }

    /* Show selected runes so far */
    {
        int sy = 155;
        if (sp->power_rune >= 0) {
            /* Small indicator dot */
            for (py = sy; py < sy + 4 && py < 200; py++)
                for (px = 20; px < 28 && px < 320; px++)
                    fb[py * 320 + px] = g_rune_colors[sp->power_rune];
        }
        if (sp->element_rune >= 0) {
            for (py = sy; py < sy + 4 && py < 200; py++)
                for (px = 35; px < 43 && px < 320; px++)
                    fb[py * 320 + px] = g_rune_colors[sp->element_rune];
        }
    }
}


/*
 * DM1 V1 champion-panel name-box clip contract gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206):
 *  - CHAMDRAW.C F0292:750 L0868_i_ChampionStatusBoxX =
 *    P0615_ui_ChampionIndex * C69_CHAMPION_STATUS_BOX_SPACING.
 *  - CHAMDRAW.C F0292:818-833 dead-champion name strip:
 *      F0053_TEXT_PrintToLogicalScreen(1 + L0868, 5,
 *          C13_COLOR_LIGHTEST_GRAY, C01_COLOR_DARK_GRAY, Name)
 *        (MEDIA001, "1+L0868" = L0868+1 optimization prefix)
 *      F0053_TEXT_PrintToLogicalScreen(L0868 + 1, 5,
 *          C13_COLOR_LIGHTEST_GRAY, C01_COLOR_DARK_GRAY, Name)
 *        (MEDIA224, the "Machine code size reduced" branch)
 *      F0650_PrintCenteredTextToScreenZone(
 *          P0615_ui_ChampionIndex + C163_ZONE_FIRST_CHAMPION_NAME,
 *          C13_COLOR_LIGHTEST_GRAY, C01_COLOR_DARK_GRAY, Name)
 *        (MEDIA529 zone-centered, used by Amiga/console ports)
 *  - CHAMDRAW.C F0292:843-895 live non-inventory NAME_TITLE branch:
 *      M770_BOX_TOP(L0871_ai_Box) = 0;
 *      M771_BOX_BOTTOM(L0871_ai_Box) = 6;
 *      M769_BOX_RIGHT(L0871_ai_Box) = (M768_BOX_LEFT(L0871_ai_Box) =
 *          L0868_i_ChampionStatusBoxX) + 42;
 *      M524_FillScreenBox(L0871_ai_Box, C01_COLOR_DARK_GRAY);
 *      F0053_TEXT_PrintToLogicalScreen(L0868 + 1, 5,
 *          AL0864_i_ColorIndex, C01_COLOR_DARK_GRAY, Name);
 *  - CHAMDRAW.C F0292:845 (MEDIA049 PC) color cascade:
 *      AL0864_i_ColorIndex = (championIndex == leaderIndex)
 *          ? C09_COLOR_GOLD : C13_COLOR_LIGHTEST_GRAY;
 *  - CHAMDRAW.C F0292:855-871 inventory viewport name/title:
 *      F0052_TEXT_PrintToViewport(3, 7, AL0864_i_ColorIndex, Name);
 *      L0869_i_ChampionTitleX = 6 * strlen(Name) + 3;
 *      if (Title[0] not in {',', ';', '-'})
 *          L0869_i_ChampionTitleX += 6;
 *      F0052_TEXT_PrintToViewport(L0869_i_ChampionTitleX, 7,
 *          AL0864_i_ColorIndex, Title);
 *  - DEFS.H:623 / DEFS.H:660 CHAMPION_INCLUDING_PORTRAIT.Name[8]
 *    and CHAMPION.Name[8] - the 8-byte field bounds the visible
 *    name to 7 chars + NUL.
 *  - DEFS.H:2157 C69_CHAMPION_STATUS_BOX_SPACING = 69.
 *  - DEFS.H:2079 C01_COLOR_DARK_GRAY = 1.
 *  - DEFS.H:2087 C09_COLOR_GOLD = 9.
 *  - DEFS.H:2091 C13_COLOR_LIGHTEST_GRAY = 13.
 *  - DEFS.H:3787 C159_ZONE_CHAMPION_0_STATUS_BOX_NAME = 159.
 *  - DEFS.H:3791 C163_ZONE_FIRST_CHAMPION_NAME = 163.
 *
 * The lane is intentionally non-duplicative with the existing
 * F0292 NAME_TITLE strip x-anchor edge inside
 * `test_dm1_v1_champion_panel_hud_pc34_compat` (which only pins
 * the slot-3 left/right/print-x/width bytes 207/249/208/43 and
 * not the 6px/char font width, the Name[8] 7-char byte cap, the
 * L0869 title-X = 6*strlen+3 formula, the dead-champion x=L0868+1
 * path, the punctuation-passthrough +6 increment, or the leader
 * color cascade), the F0292 -> F0354 dispatch predicate
 * (`dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat`),
 * the portrait-box geometry
 * (`dm1_v1_champion_panel_f0354_box_variants_pc34_compat`), the
 * portrait-state redraw matrix
 * (`dm1_v1_champion_panel_portrait_state_redraw_pc34_compat` +
 * `dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat`),
 * the mouth/eye press release gate, the food/water status-box
 * gate, the HUD recompute gate, the action-hand slot-priority
 * gate, the action-cell slotbox gate, the status-hand slot-pixels
 * gate, the hand-slot priority source lock, the spell-area
 * overlay gate, the panel shield border pixel slice, the panel
 * pressing mouth/eye statusbox, the inventory champion switch
 * hand carry, and any chest/inventory/mirror runtime regression.
 *
 * No bitmap sampling, no GRAPHICS.DAT / DUNGEON.DAT load, no
 * real-asset parity claim.
 */

#include "firestaff/dm1/v1/champion_panel/name_box_clip_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. ReDMCSB CHAMDRAW.C F0292:750 anchors "
    "L0868_i_ChampionStatusBoxX = P0615_ui_ChampionIndex * "
    "C69_CHAMPION_STATUS_BOX_SPACING, and ReDMCSB DEFS.H:2157 anchors "
    "C69_CHAMPION_STATUS_BOX_SPACING=69. ReDMCSB CHAMDRAW.C F0292:818-833 "
    "is the dead-champion name strip with three sibling code paths: "
    "MEDIA001 prints at (1+L0868, 5) with C13/C01 colors, MEDIA224 prints "
    "at (L0868+1, 5) with C13/C01 colors (the same physical pixel as the "
    "MEDIA001 path on PC 3.4), and MEDIA529 zone-centers on "
    "C163_ZONE_FIRST_CHAMPION_NAME+championIndex with C13/C01 colors. "
    "ReDMCSB CHAMDRAW.C F0292:843-895 is the live non-inventory NAME_TITLE "
    "branch: it fills the 7-row-tall 43-pixel name box at (L0868, 0).."
    "(L0868+42, 6) with C01_COLOR_DARK_GRAY and prints the name at "
    "(L0868+1, 5) with the AL0864_i_ColorIndex foreground and "
    "C01_COLOR_DARK_GRAY background. ReDMCSB CHAMDRAW.C F0292:845 (MEDIA049 "
    "PC) sets AL0864_i_ColorIndex to C09_COLOR_GOLD when championIndex "
    "equals G0411_i_LeaderIndex and to C13_COLOR_LIGHTEST_GRAY otherwise. "
    "ReDMCSB CHAMDRAW.C F0292:855-871 is the inventory viewport name/title "
    "branch: F0052 prints the name at (3, 7), L0869_i_ChampionTitleX is "
    "computed as 6 * strlen(Name) + 3, the title receives an additional +6 "
    "(one 6-px glyph) when Title[0] is not one of {',', ';', '-'}, and "
    "F0052 prints the title at (L0869, 7). ReDMCSB DEFS.H:623 and "
    "DEFS.H:660 anchor the champion Name[8] struct field, which bounds "
    "the visible name to 7 chars + NUL. ReDMCSB DEFS.H:2079/2087/2091 "
    "anchor C01_COLOR_DARK_GRAY=1, C09_COLOR_GOLD=9, and "
    "C13_COLOR_LIGHTEST_GRAY=13. ReDMCSB DEFS.H:3787/3791 anchor "
    "C159_ZONE_CHAMPION_0_STATUS_BOX_NAME=159 and "
    "C163_ZONE_FIRST_CHAMPION_NAME=163. ReDMCSB CHAMDRAW.C F0292:884 "
    "calls F0053_TEXT_PrintToLogicalScreen at L0868 + 1, 5 "
    "(L0868 + 1 left pad, y=5). ReDMCSB CHAMDRAW.C F0292:879 sets "
    "M770_BOX_TOP=0 and M771_BOX_BOTTOM=6. ReDMCSB CHAMDRAW.C F0292:881 "
    "sets M769_BOX_RIGHT = (M768_BOX_LEFT = L0868_i_ChampionStatusBoxX) + 42 "
    "and M524_FillScreenBox with C01_COLOR_DARK_GRAY. The 6-px/char PC 3.4 "
    "font width multiplied by 7 = 42 exactly matches the byte-coord right "
    "inset. This fixture is the name-box clip contract for CHAMDRAW.C "
    "F0292; the F0292 -> F0354 dispatch predicate is pinned separately "
    "by the existing portrait-box blit dispatch gate.";

static uint32_t hash_step(uint32_t hash, unsigned int value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (uint32_t)((value >> (i * 8)) & 0xffu);
        hash *= UINT32_C(16777619);
    }
    return hash;
}

static bool is_valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index < DM1_V1_CPNBC_CHAMPION_COUNT_PC34;
}

static bool title_first_char_is_punct(int title_first_char)
{
    return title_first_char == ',' || title_first_char == ';' ||
           title_first_char == '-';
}

int dm1_v1_cpnbc_compute_inventory_title_x_pc34(
    const char *name, int title_first_char)
{
    int baseline;
    int increment;
    int name_len;

    /* CHAMDRAW.C F0292:856 L0869 = 6 * strlen(Name) + 3. The Name
     * field is capped at Name[8] in DEFS.H:623, so anything past 7
     * chars is treated as a clip overflow (not visible in the
     * formula). For a NULL name the strlen is 0, the baseline is
     * the constant 3, and the punctuation passthrough +6 decision
     * still applies. */
    if (!name) {
        name_len = 0;
    } else {
        name_len = (int)strnlen(name, DM1_V1_CPNBC_NAME_FIELD_BYTES_PC34);
        if (name_len > DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34) {
            name_len = DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
        }
    }
    baseline = name_len * DM1_V1_CPNBC_GLYPH_WIDTH_PC34 +
               DM1_V1_CPNBC_INVENTORY_TITLE_X_BASELINE_PC34;
    increment =
        title_first_char_is_punct(title_first_char)
            ? 0
            : DM1_V1_CPNBC_INVENTORY_TITLE_PUNCT_INC_PC34;
    return baseline + increment;
}

bool dm1_v1_cpnbc_title_passthrough_punctuation_pc34(int title_first_char)
{
    return title_first_char_is_punct(title_first_char);
}

int dm1_v1_cpnbc_name_field_text_pixels_pc34(const char *name)
{
    int visible_chars;

    if (!name) {
        return 0;
    }
    /* The Name[8] field carries 7 visible chars + NUL. strnlen
     * returns up to 8 (the NUL), so we explicitly clip to 7. */
    visible_chars = (int)strnlen(name, DM1_V1_CPNBC_NAME_FIELD_BYTES_PC34);
    if (visible_chars > DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34) {
        visible_chars = DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
    }
    return visible_chars * DM1_V1_CPNBC_GLYPH_WIDTH_PC34;
}

static void init_box(DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    memset(out_box, 0, sizeof(*out_box));
    out_box->name_field_bytes = DM1_V1_CPNBC_NAME_FIELD_BYTES_PC34;
    out_box->name_visible_chars =
        DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
    out_box->status_box_x_anchor = 0;
    out_box->name_zone_index = 0;
    out_box->name_box_left = 0;
    out_box->name_box_top = 0;
    out_box->name_box_right = 0;
    out_box->name_box_bottom = 0;
    out_box->name_box_width = 0;
    out_box->name_box_height = 0;
    out_box->name_print_x = 0;
    out_box->name_print_y = 0;
    out_box->inventory_name_print_x = 0;
    out_box->inventory_name_print_y = 0;
    out_box->inventory_title_x = 0;
    out_box->inventory_title_x_after_punct = 0;
    out_box->color_fg = 0;
    out_box->color_bg = 0;
    out_box->glyphs_that_fit_in_name_box = 0;
    out_box->title_passthrough_no_increment = false;
}

static uint32_t hash_box(const DM1_V1_CPNBC_ChampionNameBoxPc34Compat *box)
{
    uint32_t hash = UINT32_C(2166136261);

    if (!box) {
        return hash;
    }
    hash = hash_step(hash, (unsigned int)box->champion_index);
    hash = hash_step(hash, box->is_inventory_champion ? 1u : 0u);
    hash = hash_step(hash, box->is_dead ? 1u : 0u);
    hash = hash_step(hash, box->leader_index_match ? 1u : 0u);
    hash = hash_step(hash, (unsigned int)box->leader_index);
    hash = hash_step(hash, (unsigned int)box->name_box_left);
    hash = hash_step(hash, (unsigned int)box->name_box_top);
    hash = hash_step(hash, (unsigned int)box->name_box_right);
    hash = hash_step(hash, (unsigned int)box->name_box_bottom);
    hash = hash_step(hash, (unsigned int)box->name_box_width);
    hash = hash_step(hash, (unsigned int)box->name_box_height);
    hash = hash_step(hash, (unsigned int)box->name_print_x);
    hash = hash_step(hash, (unsigned int)box->name_print_y);
    hash = hash_step(hash, (unsigned int)box->name_zone_index);
    hash = hash_step(hash, (unsigned int)box->inventory_name_print_x);
    hash = hash_step(hash, (unsigned int)box->inventory_name_print_y);
    hash = hash_step(hash, (unsigned int)box->inventory_title_x);
    hash = hash_step(hash, (unsigned int)box->inventory_title_x_after_punct);
    hash = hash_step(hash, (unsigned int)box->color_fg);
    hash = hash_step(hash, (unsigned int)box->color_bg);
    hash = hash_step(hash, (unsigned int)box->title_first_char);
    hash = hash_step(hash, box->title_passthrough_no_increment ? 1u : 0u);
    hash = hash_step(hash, (unsigned int)box->name_field_bytes);
    hash = hash_step(hash, (unsigned int)box->name_visible_chars);
    hash = hash_step(hash, (unsigned int)box->name_clip_text_pixels);
    hash = hash_step(hash, (unsigned int)box->glyphs_that_fit_in_name_box);
    hash = hash_step(hash, (unsigned int)box->status_box_x_anchor);
    hash = hash_step(hash, box->reached_f0292_name_box ? 1u : 0u);
    hash = hash_step(hash, box->box_in_range ? 1u : 0u);
    hash = hash_step(hash, box->zone_in_range ? 1u : 0u);
    hash = hash_step(hash, box->color_cascade_correct ? 1u : 0u);
    hash = hash_step(hash, box->name_field_clip_holds ? 1u : 0u);
    hash = hash_step(hash, (unsigned int)box->variant);
    return hash;
}

void dm1_v1_cpnbc_build_status_box_live_pc34(
    int champion_index,
    int leader_index,
    const char *name,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    init_box(out_box);

    out_box->champion_index = champion_index;
    out_box->leader_index = leader_index;
    out_box->is_inventory_champion = false;
    out_box->is_dead = false;
    out_box->leader_index_match =
        is_valid_champion_index(champion_index) &&
        is_valid_champion_index(leader_index) &&
        champion_index == leader_index;

    out_box->reached_f0292_name_box = is_valid_champion_index(champion_index);
    out_box->status_box_x_anchor =
        champion_index * DM1_V1_CPNBC_STATUS_BOX_SPACING_PC34;

    /* CHAMDRAW.C F0292:879-884: M770_BOX_TOP=0, M771_BOX_BOTTOM=6,
     * M768_BOX_LEFT=L0868, M769_BOX_RIGHT=L0868+42. The 42 is the
     * 7-char * 6-px/char byte-coord right inset. */
    out_box->name_box_left = out_box->status_box_x_anchor;
    out_box->name_box_top = 0;
    out_box->name_box_right =
        out_box->status_box_x_anchor +
        DM1_V1_CPNBC_NAME_BOX_RIGHT_INSET_PC34;
    out_box->name_box_bottom = DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34 - 1;
    out_box->name_box_width = DM1_V1_CPNBC_NAME_BOX_LEFT_TO_RIGHT_PC34;
    out_box->name_box_height = DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34;
    out_box->name_print_x =
        out_box->status_box_x_anchor + DM1_V1_CPNBC_NAME_BOX_LEFT_PAD_PC34;
    out_box->name_print_y = DM1_V1_CPNBC_NAME_BOX_PRINT_Y_PC34;

    out_box->name_zone_index =
        DM1_V1_CPNBC_STATUS_BOX_NAME_ZONE_BASE_PC34 + champion_index;
    out_box->zone_in_range = is_valid_champion_index(champion_index) &&
                              out_box->name_zone_index ==
                                  DM1_V1_CPNBC_STATUS_BOX_NAME_ZONE_BASE_PC34 +
                                      champion_index;

    out_box->box_in_range =
        is_valid_champion_index(champion_index) &&
        (out_box->name_box_right - out_box->name_box_left + 1) ==
            DM1_V1_CPNBC_NAME_BOX_LEFT_TO_RIGHT_PC34 &&
        (out_box->name_box_bottom - out_box->name_box_top + 1) ==
            DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34 &&
        out_box->name_print_x - out_box->name_box_left ==
            DM1_V1_CPNBC_NAME_BOX_LEFT_PAD_PC34 &&
        out_box->name_box_width / DM1_V1_CPNBC_GLYPH_WIDTH_PC34 ==
            DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;

    /* CHAMDRAW.C F0292:845 MEDIA049 PC color cascade. */
    out_box->color_fg = out_box->leader_index_match
                            ? DM1_V1_CPNBC_COLOR_LEADER_NAME_PC34
                            : DM1_V1_CPNBC_COLOR_NONLEADER_NAME_PC34;
    out_box->color_bg = DM1_V1_CPNBC_COLOR_NAME_FILL_PC34;
    out_box->color_cascade_correct =
        (out_box->color_fg == DM1_V1_CPNBC_COLOR_LEADER_NAME_PC34 ||
         out_box->color_fg == DM1_V1_CPNBC_COLOR_NONLEADER_NAME_PC34) &&
        out_box->color_bg == DM1_V1_CPNBC_COLOR_NAME_FILL_PC34;

    out_box->glyphs_that_fit_in_name_box =
        DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
    out_box->name_clip_text_pixels =
        dm1_v1_cpnbc_name_field_text_pixels_pc34(name);
    out_box->name_field_clip_holds =
        out_box->name_clip_text_pixels <= out_box->name_box_width;

    out_box->variant = out_box->reached_f0292_name_box
                           ? DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_LIVE_PC34
                           : DM1_V1_CPNBC_VARIANT_PC34_NAME_BOX_NOT_REACHED_PC34;

    out_box->hash = hash_box(out_box);
}

void dm1_v1_cpnbc_build_status_box_dead_pc34(
    int champion_index,
    const char *name,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box)
{
    if (!out_box) {
        return;
    }
    init_box(out_box);

    out_box->champion_index = champion_index;
    out_box->leader_index = -1;
    out_box->is_inventory_champion = false;
    out_box->is_dead = true;
    out_box->leader_index_match = false;

    out_box->reached_f0292_name_box = is_valid_champion_index(champion_index);
    out_box->status_box_x_anchor =
        champion_index * DM1_V1_CPNBC_STATUS_BOX_SPACING_PC34;

    /* CHAMDRAW.C F0292:818-833 dead-champion name strip. On PC 3.4
     * both MEDIA001 (1 + L0868) and MEDIA224 (L0868 + 1) print at
     * the same physical pixel column = L0868 + 1 (the +1 left pad
     * is the contract). The lane pins the x = L0868 + 1 anchor. */
    out_box->name_print_x =
        out_box->status_box_x_anchor + DM1_V1_CPNBC_NAME_BOX_LEFT_PAD_PC34;
    out_box->name_print_y = DM1_V1_CPNBC_NAME_BOX_PRINT_Y_PC34;
    out_box->name_zone_index =
        DM1_V1_CPNBC_FIRST_CHAMPION_NAME_ZONE_PC34 + champion_index;
    out_box->zone_in_range =
        is_valid_champion_index(champion_index) &&
        out_box->name_zone_index ==
            DM1_V1_CPNBC_FIRST_CHAMPION_NAME_ZONE_PC34 + champion_index;

    /* The dead-champion name strip lives inside the same 43-wide
     * name box at the top of the status box (the dead-champion
     * blit C008 fills the full 67-wide status box first, but the
     * name text is constrained to the same 43-wide name box at the
     * top). */
    out_box->name_box_left = out_box->status_box_x_anchor;
    out_box->name_box_top = 0;
    out_box->name_box_right =
        out_box->status_box_x_anchor +
        DM1_V1_CPNBC_NAME_BOX_RIGHT_INSET_PC34;
    out_box->name_box_bottom = DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34 - 1;
    out_box->name_box_width = DM1_V1_CPNBC_NAME_BOX_LEFT_TO_RIGHT_PC34;
    out_box->name_box_height = DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34;
    out_box->box_in_range =
        is_valid_champion_index(champion_index) &&
        (out_box->name_box_right - out_box->name_box_left + 1) ==
            DM1_V1_CPNBC_NAME_BOX_LEFT_TO_RIGHT_PC34 &&
        (out_box->name_box_bottom - out_box->name_box_top + 1) ==
            DM1_V1_CPNBC_NAME_BOX_HEIGHT_PC34 &&
        out_box->name_print_x - out_box->name_box_left ==
            DM1_V1_CPNBC_NAME_BOX_LEFT_PAD_PC34;

    out_box->color_fg = DM1_V1_CPNBC_DEAD_NAME_FG_PC34;
    out_box->color_bg = DM1_V1_CPNBC_DEAD_NAME_BG_PC34;
    out_box->color_cascade_correct =
        out_box->color_fg == DM1_V1_CPNBC_DEAD_NAME_FG_PC34 &&
        out_box->color_bg == DM1_V1_CPNBC_DEAD_NAME_BG_PC34;

    out_box->glyphs_that_fit_in_name_box =
        DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
    out_box->name_clip_text_pixels =
        dm1_v1_cpnbc_name_field_text_pixels_pc34(name);
    out_box->name_field_clip_holds =
        out_box->name_clip_text_pixels <= out_box->name_box_width;

    out_box->variant = out_box->reached_f0292_name_box
                           ? DM1_V1_CPNBC_VARIANT_PC34_STATUS_BOX_DEAD_PC34
                           : DM1_V1_CPNBC_VARIANT_PC34_NAME_BOX_NOT_REACHED_PC34;

    out_box->hash = hash_box(out_box);
}

void dm1_v1_cpnbc_build_inventory_viewport_pc34(
    int champion_index,
    int leader_index,
    const char *name,
    int title_first_char,
    DM1_V1_CPNBC_ChampionNameBoxPc34Compat *out_box)
{
    int title_x_baseline;
    int title_x_after_punct;

    if (!out_box) {
        return;
    }
    init_box(out_box);

    out_box->champion_index = champion_index;
    out_box->leader_index = leader_index;
    out_box->is_inventory_champion = true;
    out_box->is_dead = false;
    out_box->leader_index_match =
        is_valid_champion_index(champion_index) &&
        is_valid_champion_index(leader_index) &&
        champion_index == leader_index;

    out_box->reached_f0292_name_box = is_valid_champion_index(champion_index);
    out_box->status_box_x_anchor =
        champion_index * DM1_V1_CPNBC_STATUS_BOX_SPACING_PC34;

    /* CHAMDRAW.C F0292:855-871 inventory viewport branch. */
    out_box->inventory_name_print_x =
        DM1_V1_CPNBC_INVENTORY_NAME_PRINT_X_PC34;
    out_box->inventory_name_print_y =
        DM1_V1_CPNBC_INVENTORY_NAME_PRINT_Y_PC34;
    out_box->title_first_char = title_first_char;
    out_box->title_passthrough_no_increment =
        title_first_char_is_punct(title_first_char);
    title_x_baseline = dm1_v1_cpnbc_compute_inventory_title_x_pc34(
        name, title_first_char);
    title_x_after_punct =
        title_first_char_is_punct(title_first_char)
            ? title_x_baseline
            : title_x_baseline -
                  DM1_V1_CPNBC_INVENTORY_TITLE_PUNCT_INC_PC34;
    out_box->inventory_title_x = title_x_baseline;
    out_box->inventory_title_x_after_punct = title_x_after_punct;

    /* The inventory name/title print does not redraw the status
     * box name strip; the box fields are zeroed by init_box. The
     * F0292 inventory branch sets MASK0x4000_VIEWPORT instead of
     * MASK0x0080_NAME_TITLE. */
    out_box->name_box_left = 0;
    out_box->name_box_top = 0;
    out_box->name_box_right = 0;
    out_box->name_box_bottom = 0;
    out_box->name_box_width = 0;
    out_box->name_box_height = 0;
    out_box->name_print_x = 0;
    out_box->name_print_y = 0;
    out_box->box_in_range = is_valid_champion_index(champion_index);

    out_box->color_fg = out_box->leader_index_match
                            ? DM1_V1_CPNBC_COLOR_LEADER_NAME_PC34
                            : DM1_V1_CPNBC_COLOR_NONLEADER_NAME_PC34;
    out_box->color_bg = 0; /* F0052 takes no background color. */
    out_box->color_cascade_correct =
        out_box->color_fg == DM1_V1_CPNBC_COLOR_LEADER_NAME_PC34 ||
        out_box->color_fg == DM1_V1_CPNBC_COLOR_NONLEADER_NAME_PC34;

    out_box->glyphs_that_fit_in_name_box =
        DM1_V1_CPNBC_NAME_FIELD_VISIBLE_CHARS_PC34;
    out_box->name_clip_text_pixels =
        dm1_v1_cpnbc_name_field_text_pixels_pc34(name);
    /* The inventory branch does not redraw the status box name
     * strip, so the name-field clip holds vacuously for the
     * viewport path (the strip is not redrawn at all). The
     * inventory viewport column 3 + 6*strlen+3 contract is pinned
     * separately through inventory_name_print_x / inventory_title_x.
     */
    out_box->name_field_clip_holds = true;

    out_box->variant = out_box->reached_f0292_name_box
                           ? DM1_V1_CPNBC_VARIANT_PC34_INVENTORY_VIEWPORT_PC34
                           : DM1_V1_CPNBC_VARIANT_PC34_NAME_BOX_NOT_REACHED_PC34;

    out_box->hash = hash_box(out_box);
}

void dm1_v1_cpnbc_build_model_pc34(
    int leader_index,
    const char *name,
    DM1_V1_CPNBC_ModelPc34Compat *out_model)
{
    int i;
    int effective_leader;

    if (!out_model) {
        return;
    }
    dm1_v1_cpnbc_default_pc34(out_model);

    effective_leader = is_valid_champion_index(leader_index) ? leader_index : 0;

    for (i = 0; i < DM1_V1_CPNBC_CHAMPION_COUNT_PC34; ++i) {
        dm1_v1_cpnbc_build_status_box_live_pc34(
            i, effective_leader, name, &out_model->champions[i]);
    }

    out_model->seven_char_name = name;
    out_model->punct_title = NULL;
    out_model->non_punct_title = NULL;

    out_model->deterministic_hash = UINT32_C(2166136261);
    for (i = 0; i < DM1_V1_CPNBC_CHAMPION_COUNT_PC34; ++i) {
        out_model->deterministic_hash =
            hash_step(out_model->deterministic_hash,
                      out_model->champions[i].hash);
    }
}

void dm1_v1_cpnbc_default_pc34(DM1_V1_CPNBC_ModelPc34Compat *out_model)
{
    if (!out_model) {
        return;
    }
    memset(out_model, 0, sizeof(*out_model));
    out_model->contract_only = true;
    out_model->disjoint_from_f0354_box_variants_pc34_compat = true;
    out_model->disjoint_from_portrait_box_blit_dispatch_pc34_compat = true;
    out_model->disjoint_from_portrait_box_redraw_states_pc34_compat = true;
    out_model->disjoint_from_portrait_state_redraw_pc34_compat = true;
    out_model->disjoint_from_mouth_eye_release_pc34_compat = true;
    out_model->disjoint_from_food_water_status_box_pc34_compat = true;
    out_model->disjoint_from_hud_food_water_recompute_pc34_compat = true;
    out_model->disjoint_from_action_hand_slot_priority_pc34_compat = true;
    out_model->disjoint_from_champion_panel_pixels_runtime_probe = true;
    out_model->disjoint_from_champion_panel_status_states_runtime_probe = true;
    out_model->disjoint_from_champion_panel_status_hand_slot_pixels_source_lock =
        true;
    out_model->disjoint_from_champion_panel_partial_party_pixel_probe = true;
    out_model->disjoint_from_champion_panel_status_box_asset_slice_probe =
        true;
    out_model->disjoint_from_champion_panel_recompute_runtime = true;
    out_model->disjoint_from_champion_panel_leader_rotation_pixel_slice =
        true;
    out_model->disjoint_from_champion_panel_action_cell_slotbox_runtime =
        true;
    out_model->disjoint_from_champion_panel_hand_slot_priority_source_lock =
        true;
    out_model->disjoint_from_champion_panel_shield_border_pixel = true;
    out_model
        ->disjoint_from_champion_panel_pressing_mouth_eye_statusbox_pc34_compat =
        true;
    out_model->disjoint_from_inventory_champion_switch_hand_carry_pc34_compat =
        true;
    out_model->disjoint_from_spell_area_overlay_pc34_compat = true;
    out_model->disjoint_from_hud_pc34_compat_name_strip_x_anchor_only = true;
    out_model->seven_char_name = "ABCDEFG";
    out_model->punct_title = "Z";
    out_model->non_punct_title = "Z";
    out_model->deterministic_hash = UINT32_C(2166136261);
}

const char *dm1_v1_cpnbc_source_evidence_pc34(void)
{
    return s_source_evidence;
}

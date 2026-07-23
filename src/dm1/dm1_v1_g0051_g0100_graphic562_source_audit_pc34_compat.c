#include "dm1_v1_g0051_g0100_graphic562_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner) { number##u, anchor, owner, 1, 1, 1 }

static const DM1_V1_G0051G0100SourceAuditPc34 k_audit[] = {
    ROW(51, "DATA.C:89 G0051", "dm1_v1_underscore_character_string_pc34_compat"),
    ROW(52, "DATA.C:90 G0052", "dm1_v1_rename_champion_input_character_string_pc34_compat"),
    ROW(53, "DATA.C:91 G0053", "dm1_v1_reincarnate_special_characters_pc34_compat"),
    ROW(54, "DATA.C:92 G0054", "dm1_v1_box_champion_icons_pc34_compat"),
    ROW(55, "DATA.C:93 G0055", "dm1_v1_bar_graph_masks_pc34_compat"),
    ROW(56, "DATA.C:99 G0056", "dm1_v1_bar_graph_byte_offsets_pc34_compat"),
    ROW(57, "DATA.C:105 G0057", "dm1_v1_slot_drop_order_pc34_compat"),
    ROW(58, "DATA.C:106 G0058", "fail_closed: source-useless CPSE state"),
    ROW(59, "DATA.C:107 G0059", "dm1_v1_square_type_to_event_type_pc34_compat"),
    ROW(60, "DATA.C:108 G0060", "dm1_v1_sound_pc34_compat"),
    ROW(61, "DATA.C:109 G0061", "dm1_v1_box_screen_top_pc34_compat"),
    ROW(62, "DATA.C:110 G0062", "dm1_v1_box_screen_right_pc34_compat"),
    ROW(63, "DATA.C:111 G0063", "dm1_v1_box_screen_bottom_pc34_compat"),
    ROW(64, "CEDT019.C:34 G0064", "dm1_v1_print_text_masks2_pc34_compat"),
    ROW(65, "CEDT019.C:35 G0065", "dm1_v1_print_text_masks1_pc34_compat"),
    ROW(66, "DATA.C:114 G0066", "dm1_v1_line_feed_character_string_pc34_compat"),
    ROW(67, "DATA.C:115 G0067", "fail_closed: no verified DM1 owner"),
    ROW(68, "CHAMPION.C:30 G0068", "fail_closed: source floppy/CPSE state"),
    ROW(69, "DUNVIEW.C:25 G0069", "fail_closed: source floppy/CPSE state"),
    ROW(70, "DUNVIEW.C:35 G0070", "fail_closed: source floppy/CPSE state"),
    ROW(71, "DUNVIEW.C:36 G0071", "fail_closed: source floppy/CPSE state"),
    ROW(72, "DUNVIEW.C:37 G0072", "fail_closed: source floppy/CPSE state"),
    ROW(73, "DUNVIEW.C:42 G0073", "fail_closed: source floppy/CPSE state"),
    ROW(74, "DEFS.H:5449 G0074", "dm1_v1_viewport_3d_pc34_compat"),
    ROW(75, "DEFS.H:5447 G0075", "fail_closed: no verified projectile palette owner"),
    ROW(76, "DUNVIEW.C:49 G0076", "dm1_v1_viewport_3d_pc34_compat"),
    ROW(77, "DUNVIEW.C:50 G0077", "dm1_v1_endgame_system_pc34_compat"),
    ROW(78, "DUNVIEW.C:64 G0078", "fail_closed: source floppy/CPSE state"),
    ROW(79, "DUNVIEW.C:88 G0079", "dm1_v1_viewport_d3c_stairs_pit_dispatch_pc34_compat"),
    ROW(80, "no numbered G0080 declaration in audit corpus", "fail_closed: no source owner"),
    ROW(81, "DUNVIEW.C:99 G0081", "fail_closed: source floppy/CPSE state"),
    ROW(82, "DUNVIEW.C:100 G0082", "fail_closed: source code-patch state"),
    ROW(83, "DEFS.H:5409 G0083", "fail_closed: no verified graphic result owner"),
    ROW(84, "DEFS.H:5410 G0084", "fail_closed: no verified floor bitmap owner"),
    ROW(85, "DEFS.H:5411 G0085", "fail_closed: no verified ceiling bitmap owner"),
    ROW(86, "DEFS.H:5414 G0086", "fail_closed: no verified viewport black-area owner"),
    ROW(87, "DEFS.H:5415 G0087", "dm1_v1_f0128_viewport_pc34_compat"),
    ROW(88, "no numbered G0088 declaration in audit corpus", "fail_closed: no source owner"),
    ROW(89, "DUNVIEW.C:270 G0089", "fail_closed: source code-patch state"),
    ROW(90, "DEFS.H:5433 G0090", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(91, "DEFS.H:5434 G0091", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(92, "DEFS.H:5435 G0092", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(93, "DEFS.H:5436 G0093", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(94, "DEFS.H:5437 G0094", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(95, "DEFS.H:5438 G0095", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(96, "DEFS.H:5439 G0096", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(97, "DEFS.H:5440 G0097", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(98, "DEFS.H:5441 G0098", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(99, "DEFS.H:5442 G0099", "dm1_v1_f0461_start_allocate_flipped_wall_bitmaps_pc34_compat"),
    ROW(100, "DUNVIEW.C:356 G0100", "fail_closed: source code-patch state")
};

#undef ROW

const DM1_V1_G0051G0100SourceAuditPc34 *
dm1_v1_g0051_g0100_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0051G0100SourceAuditPc34 *
dm1_v1_g0051_g0100_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0051_g0100_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DATA.C, DUNVIEW.C, DEFS.H, CHAMPION.C, and CEDT019.C are "
           "the authority for G0051-G0100. This inventory binds only existing "
           "source-named DM1 owners; CPSE/floppy/code-patch and unbound bitmap state "
           "remain fail closed without authentic raw PC34 material. The audit does not "
           "render or synthesize behavior.";
}

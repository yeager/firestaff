#include "dm1_v1_g0201_g0250_graphic558_559_source_audit_pc34_compat.h"

#define ROW(number, anchor, owner) { number##u, anchor, owner, 1, 1, 1 }

static const DM1_V1_G0201G0250SourceAuditPc34 k_audit[] = {
    ROW(201, "DUNVIEW.C:478 G0201", "dm1_v1_g0201_pc34_compat"),
    ROW(202, "DUNVIEW.C:479 G0202", "dm1_v1_g0202_pc34_compat"),
    ROW(203, "DUNVIEW.C:480 G0203", "dm1_v1_g0203_pc34_compat"),
    ROW(204, "DUNVIEW.C:481 G0204", "dm1_v1_g0204_pc34_compat"),
    ROW(205, "DUNVIEW.C:482 G0205", "dm1_v1_g0205_pc34_compat"),
    ROW(206, "DUNVIEW.C:483 G0206", "dm1_v1_g0206_pc34_compat"),
    ROW(207, "DUNVIEW.C:484 G0207", "dm1_v1_g0207_pc34_compat"),
    ROW(208, "DUNVIEW.C:485 G0208", "dm1_v1_g0208_pc34_compat"),
    ROW(209, "DUNVIEW.C:486 G0209", "fail_closed: no verified object-aspect owner"),
    ROW(210, "DUNVIEW.C:487 G0210", "dm1_v1_projectile_explosion_render_pc34_compat"),
    ROW(211, "DUNVIEW.C:488 G0211", "dm1_v1_explosion_bitmap_viewport_pc34_compat"),
    ROW(212, "DUNVIEW.C:489 G0212", "dm1_v1_g0212_pc34_compat"),
    ROW(213, "DUNVIEW.C:490 G0213", "dm1_v1_g0213_pc34_compat"),
    ROW(214, "DUNVIEW.C:491 G0214", "dm1_v1_g0214_pc34_compat"),
    ROW(215, "DUNVIEW.C:492 G0215", "dm1_v1_projectile_explosion_render_pc34_compat"),
    ROW(216, "DUNVIEW.C:493 G0216", "dm1_v1_projectile_explosion_render_pc34_compat"),
    ROW(217, "DUNVIEW.C:494 G0217", "dm1_v1_creature_viewport_pc34_compat"),
    ROW(218, "DUNVIEW.C:495 G0218", "fail_closed: no verified object-coordinate owner"),
    ROW(219, "DUNVIEW.C:496 G0219", "dm1_v1_creature_render_pc34_compat"),
    ROW(220, "DUNVIEW.C:497 G0220", "dm1_v1_g0220_pc34_compat"),
    ROW(221, "DUNVIEW.C:510 G0221", "dm1_v1_g0221_pc34_compat"),
    ROW(222, "DUNVIEW.C:511 G0222", "dm1_v1_g0222_pc34_compat"),
    ROW(223, "DUNVIEW.C:512 G0223", "dm1_v1_g0223_pc34_compat"),
    ROW(224, "DUNVIEW.C:513 G0224", "dm1_v1_g0224_pc34_compat"),
    ROW(225, "DUNVIEW.C:514 G0225", "fail_closed: no verified explosion-coordinate owner"),
    ROW(226, "DUNVIEW.C:515 G0226", "fail_closed: no verified explosion-coordinate owner"),
    ROW(227, "DUNVIEW.C:516 G0227", "fail_closed: no verified rebirth-coordinate owner"),
    ROW(228, "DUNVIEW.C:517 G0228", "fail_closed: no verified rebirth-coordinate owner"),
    ROW(229, "DUNVIEW.C:518 G0229", "fail_closed: no verified Graphic558 anchor owner"),
    ROW(230, "DUNVIEW.C:1928 G0230", "fail_closed: no verified current-floor owner"),
    ROW(231, "DUNVIEW.C:1929 G0231", "fail_closed: no verified current-wall owner"),
    ROW(232, "DUNVIEW.C:2295 G0232", "fail_closed: no verified first-call owner"),
    ROW(233, "DUNGEON.C:6 G0233", "dm1_v1_dungeon_square_structs_pc34_compat"),
    ROW(234, "DUNGEON.C:7 G0234", "dm1_v1_dungeon_square_structs_pc34_compat"),
    ROW(235, "CEDT005.C:64 G0235", "dm1_v1_dungeon_thing_data_pc34_compat"),
    ROW(236, "DUNGEON.C:9 G0236", "fail_closed: no verified additional-thing-count owner"),
    ROW(237, "DUNGEON.C:10 G0237", "dm1_v1_dungeon_thing_data_pc34_compat"),
    ROW(238, "DUNGEON.C:11 G0238", "dm1_v1_dungeon_weapon_info_pc34_compat"),
    ROW(239, "DUNGEON.C:12 G0239", "dm1_v1_armour_defense_f0143_pc34_compat"),
    ROW(240, "DUNGEON.C:13 G0240", "fail_closed: source CPSD state"),
    ROW(241, "DUNGEON.C:14 G0241", "fail_closed: no verified junk-info owner"),
    ROW(242, "DUNGEON.C:15 G0242", "fail_closed: no verified food-amount owner"),
    ROW(243, "DUNGEON.C:16 G0243", "dm1_v1_creature_attributes_f0144_pc34_compat"),
    ROW(244, "DUNGEON.C:17 G0244", "fail_closed: no verified creature-sound owner"),
    ROW(245, "DUNGEON.C:18 G0245", "fail_closed: no verified possession owner"),
    ROW(246, "DUNGEON.C:19 G0246", "fail_closed: no verified possession owner"),
    ROW(247, "DUNGEON.C:20 G0247", "fail_closed: no verified possession owner"),
    ROW(248, "DUNGEON.C:21 G0248", "fail_closed: no verified possession owner"),
    ROW(249, "DUNGEON.C:22 G0249", "fail_closed: no verified possession owner"),
    ROW(250, "DUNGEON.C:23 G0250", "fail_closed: no verified possession owner")
};

#undef ROW

const DM1_V1_G0201G0250SourceAuditPc34 *
dm1_v1_g0201_g0250_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0201G0250SourceAuditPc34 *
dm1_v1_g0201_g0250_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0201_g0250_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DUNVIEW.C:478-2295, DUNGEON.C:6-23, and CEDT005.C:64 are "
           "the authority for G0201-G0250. Existing source-named owners are retained; "
           "unbound coordinate, state, and possession tables remain fail closed without "
           "authentic raw PC34 material. The audit does not render or synthesize behavior.";
}

#ifndef FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_BOTH_BACKDROPS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_VIEWPORT_CUSTOM_BACKGROUNDS_BOTH_BACKDROPS_PC34_COMPAT_H

/* CSB V1 CustomBackgrounds both-backdrops contract anchors:
 * - ReDMCSB DUNVIEW.C F0128:8318-8542 viewport order, including the first
 *   room 0 and second room 2 CustomBackgrounds-equivalent dispatch order.
 * - ReDMCSB DUNVIEW.C F0128:8337-8339 backdrop keep-out before cell draws.
 * - ReDMCSB DUNVIEW.C F0098:2962-3002 base floor/ceiling draw/reset.
 * - ReDMCSB DEFS.H:2088 C10_COLOR_FLESH transparency.
 * - ReDMCSB DEFS.H:2595-2611 I34E view-square constants.
 * - CSB-lineage Viewport.cpp:6840-6841 first/second dispatch.
 * - CSB-lineage Viewport.cpp:6503-6551 CustomBackgrounds.
 * - CSB-lineage Viewport.cpp:6521-6524 default-skin fallback.
 */

typedef struct {
    int contract_only;
    int first_room_num;
    int first_relative_forward;
    int first_relative_side;
    int first_keep_out_ordinal;
    int second_room_num;
    int second_relative_forward;
    int second_relative_side;
    int second_keep_out_ordinal;
    int first_dispatched_before_second;
    int rooms_distinct;
    int sides_distinct;
    int keep_out_ordinals_distinct;
    int f0098_drawn_before_either_backdrop;
    int c10_transparency_preserved_across_both;
    int both_non_overlapping;
    int first_routes_through_f0107_f0108_f0111_f0115;
    int second_routes_through_f0107_f0108_f0111_f0115;
    int first_routes_through_f0107;
    int first_routes_through_f0108;
    int first_routes_through_f0111;
    int first_routes_through_f0115;
    int second_routes_through_f0107;
    int second_routes_through_f0108;
    int second_routes_through_f0111;
    int second_routes_through_f0115;
    int order_includes_f0098_then_first_then_second;
    const char *redmcsb_f0128_anchor;
    const char *redmcsb_f0098_anchor;
    const char *redmcsb_f0128_keep_out_anchor;
    const char *redmcsb_defs_c10_anchor;
    const char *csb_lineage_first_dispatch_anchor;
    const char *csb_lineage_second_dispatch_anchor;
    const char *non_overlap_note;
    const char *source_summary;
} CSB_V1_CustomBackgroundsBothBackdropsContract;

const CSB_V1_CustomBackgroundsBothBackdropsContract *
csb_v1_viewport_custom_backgrounds_both_backdrops_contract_pc34(void);

#endif

#ifndef FIRESTAFF_CSB_V1_CHAOS_CAST_TARGETING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHAOS_CAST_TARGETING_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#define CSB_V1_CHAOS_TARGET_CHAMPION_NONE       -1
#define CSB_V1_CHAOS_TARGET_MAX_CHAMPIONS        4
#define CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES   576
#define CSB_V1_CHAOS_TARGET_SCREEN_BYTES     32000
#define CSB_V1_CHAOS_TARGET_C048_BYTE_WIDTH     48
#define CSB_V1_CHAOS_TARGET_C160_BYTE_WIDTH    160
#define CSB_V1_CHAOS_TARGET_LINE2_OFFSET      8112
#define CSB_V1_CHAOS_TARGET_C2_AVAILABLE         2
#define CSB_V1_CHAOS_TARGET_C3_CHAMPION          3

#define CSB_V1_CHAOS_TARGET_COMMAND_SPELL_AREA 100

#define CSB_V1_CHAOS_TARGET_READY                0
#define CSB_V1_CHAOS_TARGET_INVALID             -1
#define CSB_V1_CHAOS_TARGET_REJECTED            -2
#define CSB_V1_CHAOS_TARGET_NO_CASTER           -3
#define CSB_V1_CHAOS_TARGET_BAD_FRAME           -4

typedef struct {
    int current_health;
    int symbol_step;
    char symbols[5];
    char name[16];
} CSB_V1_ChaosTargetChampion;

typedef struct {
    int command_id;
    int command_gate_admitted;
    int candidate_champion_ordinal;
    int requested_call_frame;
} CSB_V1_ChaosTargetCommand;

typedef struct {
    int magic_caster_champion_index;
    int party_champion_count;
    CSB_V1_ChaosTargetChampion champions[CSB_V1_CHAOS_TARGET_MAX_CHAMPIONS];
    uint8_t spell_area_line[CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES];
    uint8_t spell_area_lines[3][CSB_V1_CHAOS_TARGET_SPELL_LINE_BYTES];
    uint8_t screen[CSB_V1_CHAOS_TARGET_SCREEN_BYTES];
    int spell_area_line_kind;
    int spell_area_line_visible;
    int spell_area_line1_caster;
    int last_controls_champion_index;
    int last_blit_offset;
    int last_blit_stride;
    int last_blit_width;
    int last_blit_rows;
    int line1_blits;
    int clear_line_count;
    int dispatch_frame_index;
    int dispatch_started;
    int dsa_queue_count;
    int ex_gosub_depth;
    int dsa_buffer_head;
    int dsa_active_caster;
    int cooldown_ticks;
    int cooldown_tick_decrements;
} CSB_V1_ChaosCastTargetingState;

void csb_v1_chaos_cast_targeting_init(CSB_V1_ChaosCastTargetingState *state);
int csb_v1_chaos_cast_targeting_set_champion(
    CSB_V1_ChaosCastTargetingState *state, int champion_index,
    int current_health, int symbol_step, const char *symbols,
    const char *name);
int csb_v1_chaos_cast_targeting_build_spell_area_line(
    CSB_V1_ChaosCastTargetingState *state, int spell_area_bitmap_line);
int csb_v1_chaos_cast_targeting_set_magic_caster_and_draw_spell_area(
    CSB_V1_ChaosCastTargetingState *state, int champion_index);
int csb_v1_chaos_cast_targeting_clear_completed_spell_line(
    CSB_V1_ChaosCastTargetingState *state);
int csb_v1_chaos_cast_targeting_accept_spell_area_command(
    CSB_V1_ChaosCastTargetingState *state,
    const CSB_V1_ChaosTargetCommand *command);
int csb_v1_chaos_cast_targeting_count_nonzero_line_bytes(
    const CSB_V1_ChaosCastTargetingState *state);
int csb_v1_chaos_cast_targeting_count_nonzero_line1_screen_bytes(
    const CSB_V1_ChaosCastTargetingState *state);
const char *csb_v1_chaos_cast_targeting_anchor_text(void);

#endif

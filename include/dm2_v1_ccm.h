
#ifndef FIRESTAFF_DM2_V1_CCM_H
#define FIRESTAFF_DM2_V1_CCM_H

/*
 * dm2_v1_ccm.h - DM2 V1 CCM (Creature Command Machine) Parity
 *
 * DM2 Phase 5 (creature/combat parity) source-lock: advanced CCM.
 *
 * CCM is the Creature Command Machine — a small interpreter that
 * executes per-tick creature AI opcodes.  Each creature has a
 * CCM state (b_1a register) that holds the next opcode to execute.
 *
 * Source-lock anchors (DM2 decompilation via skproject):
 *   skproject/SKULLWIN/c_creature.cpp         - DM2_PROCEED_CCM dispatch
 *   skproject/SKULLWIN/c_ai.cpp              - DM2_THINK_CREATURE
 *   skproject/SKULLWIN/SKWinGlobal.h:42      - max creature count
 *   ReDMCSB GROUP.C:1695-1770                 - F0207 creature attack
 *   ReDMCSB GROUP.C:2376-2387                 - F0209 visible row/col
 *   ReDMCSB PROJEXPL.C:76-92                  - F0212 projectile live
 *   skproject/SKULLWIN/c_creature.cpp:130     - DM2_PROCEED_CCM
 *
 * DM2 CCM opcodes implemented (representative subset):
 *   0x00 WALK_NOW          - movement dispatch
 *   0x01 ATTACK_HANDLER    - delegate to attack path (melee/spell)
 *   0x02 WALK_CONT         - movement continuation
 *   0x05 SPECIAL_ACTION    - branch to CCM06/CCM0B/CCM0C
 *   0x09 STEAL_ITEM        - thief-type item theft
 *   0x0A MERCHANT_BEHAVIOR - merchant/shop behavior
 *   0x0D SHOOT_ITEM        - ranged throw/pickup (projectile dispatch)
 *   0x0F KILL_ON_TIMER_POS - delayed-position kill
 *   0x13 ROTATES_TARGET    - reorient another creature
 *   0x15 CAST_SPELL        - monster spellcasting
 *   0x17 CREATURE_ATTACKS_PARTY - fallback attack
 *   0x26 EXPLODE_OR_SUMMON - self-destruct or spawn minion
 *
 * The remaining 200+ opcodes in skproject's full CCM are STUB'd in
 * this implementation (returns DM2_CCM_RESULT_UNKNOWN_OPCODE).
 *
 * V1 invariant: CCM execution never mutates party state directly;
 * mutations go through the creature AI + projectile dispatch path.
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Constants ───────────────────────────────────────────────────── */
#define DM2_CCM_MAX_OPCODES        32    /* implemented opcodes */
#define DM2_CCM_STACK_SIZE         16    /* operand stack depth */
#define DM2_CCM_FLAG_COUNT         16    /* flag registers */

/* CCM opcodes (subset) */
typedef enum {
    DM2_CCM_OP_NOP              = 0x00,
    DM2_CCM_OP_WALK_NOW         = 0x00,  /* alias per skproject: 0x00 = walk */
    DM2_CCM_OP_ATTACK_HANDLER   = 0x01,
    DM2_CCM_OP_WALK_CONT        = 0x02,
    DM2_CCM_OP_SPECIAL_ACTION   = 0x05,
    DM2_CCM_OP_STEAL_ITEM       = 0x09,
    DM2_CCM_OP_MERCHANT_BEHAVIOR= 0x0A,
    DM2_CCM_OP_SHOOT_ITEM       = 0x0D,
    DM2_CCM_OP_KILL_ON_TIMER_POS= 0x0F,
    DM2_CCM_OP_ROTATES_TARGET   = 0x13,
    DM2_CCM_OP_CAST_SPELL       = 0x15,
    DM2_CCM_OP_CREATURE_ATTACKS_PARTY = 0x17,
    DM2_CCM_OP_EXPLODE_OR_SUMMON = 0x26,
    /* Aliases for state-machine register names from dm2_v1_creature.h */
    DM2_CCM_OP_HALT             = 0xFF,
} DM2_CCM_Opcode;

/* Result codes */
typedef enum {
    DM2_CCM_RESULT_OK = 0,
    DM2_CCM_RESULT_HALTED,
    DM2_CCM_RESULT_STACK_OVERFLOW,
    DM2_CCM_RESULT_STACK_UNDERFLOW,
    DM2_CCM_RESULT_UNKNOWN_OPCODE,
    DM2_CCM_RESULT_BAD_ARG,
    DM2_CCM_RESULT_NOT_READY,  /* e.g. timer not elapsed */
} DM2_CCM_Result;

/* CCM state (per-creature) */
typedef struct {
    int   pc;                          /* program counter */
    int   halted;
    int   stack[DM2_CCM_STACK_SIZE];
    int   stack_top;
    int   flags[DM2_CCM_FLAG_COUNT];
    int   last_step_tick_ms;
    int   target_id;
    int   target_x, target_y, target_level;
    int   step_count;
    /* Result of last execution */
    int   last_opcode;
    int   last_result;
} DM2_V1_CCMState;

/* Opcode descriptor */
typedef struct {
    int   opcode;       /* DM2_CCM_OP_* */
    const char *name;
    int   arg_count;    /* operands consumed */
    int   stubbed;      /* 1 = not implemented, returns UNKNOWN_OPCODE */
} DM2_V1_CCMOpcodeDef;

/* ── Catalog ────────────────────────────────────────────────────── */
int  dm2_v1_ccm_get_opcode_count(void);
const DM2_V1_CCMOpcodeDef *dm2_v1_ccm_get_opcode_def(int opcode);
const char *dm2_v1_ccm_get_opcode_name(int opcode);

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_ccm_reset_state(DM2_V1_CCMState *state);
void dm2_v1_ccm_init_state(DM2_V1_CCMState *state);

/* ── Stack ──────────────────────────────────────────────────────── */
int  dm2_v1_ccm_stack_push(DM2_V1_CCMState *state, int value);
int  dm2_v1_ccm_stack_pop(DM2_V1_CCMState *state, int *out_value);
int  dm2_v1_ccm_stack_peek(const DM2_V1_CCMState *state, int *out_value);
int  dm2_v1_ccm_stack_size(const DM2_V1_CCMState *state);

/* ── Step / run ─────────────────────────────────────────────────── */
int  dm2_v1_ccm_step(DM2_V1_CCMState *state, int opcode,
                     const int *args, int arg_count, int now_ms);
int  dm2_v1_ccm_run(DM2_V1_CCMState *state, int now_ms);

/* ── Flags ──────────────────────────────────────────────────────── */
int  dm2_v1_ccm_flag_get(const DM2_V1_CCMState *state, int flag_id);
void dm2_v1_ccm_flag_set(DM2_V1_CCMState *state, int flag_id, int value);

/* ── Observability ──────────────────────────────────────────────── */
int  dm2_v1_ccm_total_steps(void);
int  dm2_v1_ccm_total_unknown(void);
int  dm2_v1_ccm_total_halted(void);

const char *dm2_v1_ccm_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CCM_H */

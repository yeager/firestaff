
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
 * Opcode numbering is aligned to the skproject b_1a dispatch matrix
 * (skproject/SKULLWIN/c_creature.cpp:2930-3212 DM2_PROCEED_CCM, bound
 * verbatim in dm2_v1_ccm_dispatch_pc34_compat): every enum value below
 * IS the creature command byte the source compare chain routes to the
 * named handler group.  Notably:
 *   0x01/0x02/0x09 WALK_NOW            (skip00387)
 *   0x03/0x04      CCM03
 *   0x05           JUMPS
 *   0x06/0x07      CCM06
 *   0x08/0x26      CREATURE_ATTACKS_PARTY (skip00388)
 *   0x0A           STEAL_FROM_CHAMPION
 *   0x0B           CCM0B (stub: handler body unproven)
 *   0x0C/0x0D      CCM0C (stub: handler body unproven)
 *   0x0E/0x0F      SHOOT_ITEM
 *   0x13           KILL_ON_TIMER_POSITION
 *   0x15/0x16      ROTATES_TARGET_CREATURE
 *   0x17           PLACE_MERCHANDISE
 *   0x18           TAKE_MERCHANDISE
 *   0x19/0x29/0x2A/0x2D/0x2E PUTS_DOWN_ITEM (skip00386)
 *   0x1A/0x2B/0x2C TAKES_ITEM (skip00389)
 *   0x27/0x28      CAST_SPELL
 *   0x2F-0x31      ACTIVATES_WALL (stub)
 *   0x35-0x3A      USES_LADDER_HOLE (stub)
 *   0x3B/0x3C      TRANSFORM (stub)
 *   0x3D-0x40      EXPLODE_OR_SUMMON
 *   0x55           DM2_1B7D5 (stub)
 *
 * Bytes the source compare chain routes to NO handler (0x00, 0x10-0x12,
 * 0x14, 0x1B-0x25, 0x32-0x34, 0x41-0x54, 0x56-0xFE) are not table
 * entries; stepping them returns DM2_CCM_RESULT_UNKNOWN_OPCODE, matching
 * the source's "no branch taken" (RG3L stays 0) fail-closed behaviour.
 * 0xFF HALT is a Firestaff-internal control byte, not a source b_1a.
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
#define DM2_CCM_MAX_OPCODES        48    /* source-matrix table rows */
#define DM2_CCM_STACK_SIZE         16    /* operand stack depth */
#define DM2_CCM_FLAG_COUNT         24    /* flag registers */
#define DM2_CCM_MAX_PROGRAM_OPS    64    /* bounded imported CCM bytecode */
#define DM2_CCM_MAX_PROGRAM_ARGS    3    /* current max operand width */

/* CCM opcodes — values ARE the skproject b_1a command bytes
 * (c_creature.cpp:2930-3212 DM2_PROCEED_CCM compare chain). */
typedef enum {
    DM2_CCM_OP_WALK_NOW          = 0x01,  /* WALK_NOW (skip00387) */
    DM2_CCM_OP_WALK_CONT         = 0x02,  /* WALK_NOW group; walk continuation */
    DM2_CCM_OP_CCM03             = 0x03,  /* DM2_CREATURE_CCM03 */
    DM2_CCM_OP_CCM03_PHASE       = 0x04,  /* DM2_CREATURE_CCM03 paired phase */
    DM2_CCM_OP_JUMPS             = 0x05,  /* DM2_CREATURE_JUMPS */
    DM2_CCM_OP_CCM06             = 0x06,  /* DM2_CREATURE_CCM06 */
    DM2_CCM_OP_CCM06_ALT         = 0x07,  /* DM2_CREATURE_CCM06 alternate phase */
    DM2_CCM_OP_ATTACKS_PARTY     = 0x08,  /* DM2_CREATURE_ATTACKS_PARTY (skip00388) */
    DM2_CCM_OP_WALK_NOW_09       = 0x09,  /* WALK_NOW (skip00387) */
    DM2_CCM_OP_STEAL_FROM_CHAMPION = 0x0A, /* DM2_CREATURE_STEAL_FROM_CHAMPION */
    DM2_CCM_OP_CCM0B             = 0x0B,  /* DM2_CREATURE_CCM0B (stub) */
    DM2_CCM_OP_CCM0C             = 0x0C,  /* DM2_CREATURE_CCM0C (stub) */
    DM2_CCM_OP_CCM0C_ALT         = 0x0D,  /* DM2_CREATURE_CCM0C (stub) */
    DM2_CCM_OP_SHOOT_ITEM        = 0x0E,  /* DM2_CREATURE_SHOOT_ITEM */
    DM2_CCM_OP_SHOOT_ITEM_ALT    = 0x0F,  /* DM2_CREATURE_SHOOT_ITEM */
    DM2_CCM_OP_KILL_ON_TIMER_POSITION = 0x13, /* DM2_CREATURE_KILL_ON_TIMER_POSITION */
    DM2_CCM_OP_ROTATES_TARGET_CREATURE = 0x15, /* DM2_CREATURE_ROTATES_TARGET_CREATURE */
    DM2_CCM_OP_ROTATES_TARGET_16 = 0x16,  /* ROTATES_TARGET_CREATURE paired state */
    DM2_CCM_OP_PLACE_MERCHANDISE = 0x17,  /* DM2_PLACE_MERCHANDISE */
    DM2_CCM_OP_TAKE_MERCHANDISE  = 0x18,  /* DM2_TAKE_MERCHANDISE */
    DM2_CCM_OP_PUTS_DOWN_ITEM    = 0x19,  /* DM2_CREATURE_PUTS_DOWN_ITEM (skip00386) */
    DM2_CCM_OP_TAKES_ITEM        = 0x1A,  /* DM2_CREATURE_TAKES_ITEM (skip00389) */
    DM2_CCM_OP_ATTACKS_PARTY_26  = 0x26,  /* DM2_CREATURE_ATTACKS_PARTY (skip00388) */
    DM2_CCM_OP_CAST_SPELL        = 0x27,  /* DM2_CREATURE_CAST_SPELL */
    DM2_CCM_OP_CAST_SPELL_28     = 0x28,  /* DM2_CREATURE_CAST_SPELL */
    DM2_CCM_OP_PUTS_DOWN_ITEM_29 = 0x29,  /* PUTS_DOWN_ITEM (skip00386) */
    DM2_CCM_OP_PUTS_DOWN_ITEM_2A = 0x2A,  /* PUTS_DOWN_ITEM (skip00386) */
    DM2_CCM_OP_TAKES_ITEM_2B     = 0x2B,  /* TAKES_ITEM (skip00389) */
    DM2_CCM_OP_TAKES_ITEM_2C     = 0x2C,  /* TAKES_ITEM (skip00389) */
    DM2_CCM_OP_PUTS_DOWN_ITEM_2D = 0x2D,  /* PUTS_DOWN_ITEM (skip00386) */
    DM2_CCM_OP_PUTS_DOWN_ITEM_2E = 0x2E,  /* PUTS_DOWN_ITEM (skip00386) */
    DM2_CCM_OP_ACTIVATES_WALL    = 0x2F,  /* DM2_CREATURE_ACTIVATES_WALL (stub) */
    DM2_CCM_OP_ACTIVATES_WALL_30 = 0x30,  /* ACTIVATES_WALL (stub) */
    DM2_CCM_OP_ACTIVATES_WALL_31 = 0x31,  /* ACTIVATES_WALL (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE  = 0x35,  /* DM2_CREATURE_USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE_36 = 0x36, /* USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE_37 = 0x37, /* USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE_38 = 0x38, /* USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE_39 = 0x39, /* USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_USES_LADDER_HOLE_3A = 0x3A, /* USES_LADDER_HOLE (stub) */
    DM2_CCM_OP_TRANSFORM         = 0x3B,  /* DM2_CREATURE_TRANSFORM (stub) */
    DM2_CCM_OP_TRANSFORM_3C      = 0x3C,  /* TRANSFORM (stub) */
    DM2_CCM_OP_EXPLODE_OR_SUMMON = 0x3D,  /* DM2_CREATURE_EXPLODE_OR_SUMMON */
    DM2_CCM_OP_EXPLODE_OR_SUMMON_3E = 0x3E, /* EXPLODE_OR_SUMMON */
    DM2_CCM_OP_EXPLODE_OR_SUMMON_3F = 0x3F, /* EXPLODE_OR_SUMMON */
    DM2_CCM_OP_EXPLODE_OR_SUMMON_40 = 0x40, /* EXPLODE_OR_SUMMON */
    DM2_CCM_OP_1B7D5             = 0x55,  /* DM2_1B7D5 (stub) */
    /* Firestaff-internal control byte, not a source b_1a value. */
    DM2_CCM_OP_HALT              = 0xFF,
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
    int   next_state;                  /* requested b_1a writeback, -1 = none */
    int   step_count;
    /* Result of last execution */
    int   last_opcode;
    int   last_result;
} DM2_V1_CCMState;

/* Opcode descriptor */
typedef struct {
    int   opcode;       /* DM2_CCM_OP_* (source b_1a command byte) */
    const char *name;
    int   arg_count;    /* operands consumed */
    int   stubbed;      /* 1 = not implemented, returns UNKNOWN_OPCODE */
    /* Source handler group of this command byte; values match
     * DM2_V1_CcmSourceHandler in dm2_v1_ccm_dispatch_pc34_compat.h
     * (kept as int to avoid header coupling).  -1 for the
     * Firestaff-internal HALT control byte. */
    int   source_group;
} DM2_V1_CCMOpcodeDef;

typedef struct {
    uint8_t opcode;
    uint8_t arg_count;
    int args[DM2_CCM_MAX_PROGRAM_ARGS];
} DM2_V1_CCMProgramOp;

typedef struct {
    DM2_V1_CCMProgramOp ops[DM2_CCM_MAX_PROGRAM_OPS];
    int count;
} DM2_V1_CCMProgram;

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
/*
 * Compatibility entry point with no source command stream argument.
 *
 * SKProject's DM2_PROCEED_CCM obtains b_1a plus its operand ownership from
 * the live creature/CCM record; a program counter is never an opcode.
 * Consequently this entry point always fails closed with UNKNOWN_OPCODE and
 * leaves the CCM state untouched apart from last_result.  Production callers
 * must first import the authenticated command stream and use
 * dm2_v1_ccm_run_program().
 */
int  dm2_v1_ccm_run(DM2_V1_CCMState *state, int now_ms);
int  dm2_v1_ccm_decode_program(const uint8_t *bytes, size_t byte_count,
                               DM2_V1_CCMProgram *out_program);
int  dm2_v1_ccm_run_program(DM2_V1_CCMState *state,
                             const DM2_V1_CCMProgram *program,
                             int now_ms);

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

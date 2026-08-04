/*
 * dm2_v1_creature_ai_decision_pc34_compat.c — DM2 creature AI decision/selection.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 *
 * DM2_DECIDE_NEXT_XACT (c_ai.cpp:4445-4492):
 *   Walks the 7-byte action table starting at row_index. Skips rows
 *   whose opcode is negative (opcode -10 = set register, writing byte 2
 *   into creature word at 0x0e + 2*arg1 for arg1 < 2). Returns the
 *   opcode from the first non-negative row. Sets xact_arg0/arg1 from
 *   bytes 3 and 4.
 *
 * DM2_14cd_08f5 (c_ai.cpp:4375-4443):
 *   Post-XACT result handler. Reads the XACT return code and the
 *   current row's branch bytes (byte 1 for -2, byte 2 for others).
 *   Branch codes: -2/-3 = clear table_index; -5 = row+1, -7/-6 = relative
 *   jump; -8 = row+2; other = set row directly. Returns 1 if state changed.
 *
 * DM2_14cd_0389 (c_ai.cpp:3931-3978):
 *   Validates the creature's current target. Checks v1e07d8 state fields
 *   and, if table_index is set, calls the action handler (DM2_14cd_0f0a)
 *   to see if the target is reachable. Fail-closed: requires live dungeon.
 *
 * DM2_14cd_0457 (c_ai.cpp:3979-4079):
 *   Target selection. Walks the candidate array, adjusting priorities and
 *   compacting negative-priority entries. Fail-closed: requires live data.
 *
 * DM2_14cd_0067 (c_ai.cpp:4495-4720):
 *   Behavior selection. Modifies ai_flags based on random bits and state,
 *   then walks the behavior table (6-byte entries) matching flag patterns.
 *   Fail-closed: requires live random, GDAT, global var queries.
 *
 * DM2_SELECT_CREATURE_37FC (c_ai.cpp:4723-4740):
 *   Wrapper resolving GDAT creature word, then calling DM2_14cd_0067.
 *
 * DM2_14cd_0550 (c_ai.cpp:4080-4175):
 *   Action handler invocation. Fail-closed: calls DM2_14cd_0f0a.
 *
 * DM2_14cd_0276 (c_ai.cpp:4176-4230):
 *   Prepares action context from a behavior entry. Extracts v1e07d8 fields.
 *   Memory allocation portion is fail-closed.
 */

#include "dm2_v1_creature_ai_decision_pc34_compat.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* DM2_DECIDE_NEXT_XACT (c_ai.cpp:4445-4492)                         */
/* Full logic translation — pure table walker.                        */
/* ------------------------------------------------------------------ */

int dm2_v1_decide_next_xact(const DM2_V1_DecideNextXactRequest *req,
                             DM2_V1_DecideNextXactReceipt *receipt)
{
    int8_t row_idx;
    int budget;
    int16_t w0e, w10;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    if (!req->table || req->table_row_count <= 0) {
        receipt->valid = 1;
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    row_idx = req->row_index;
    w0e = req->creature_w0e;
    w10 = req->creature_w10;

    /* c_ai.cpp:4460-4481 — walk rows, skip negative opcodes */
    budget = req->table_row_count;
    for (;;) {
        const DM2_V1_ActionTableRow *row;
        int idx = (int)(uint8_t)row_idx;

        if (idx >= req->table_row_count || --budget < 0) {
            receipt->fail_closed = 1;
            return 0;
        }

        row = &req->table[idx];

        /* c_ai.cpp:4465 — if opcode >= 0, this is the terminal row */
        if (row->opcode >= 0)  {
            /* c_ai.cpp:4483-4491 — write back and extract args */
            receipt->new_row_index = row_idx;
            receipt->action_opcode = row->opcode;
            receipt->xact_arg0 = (int16_t)row->arg3;
            receipt->xact_arg1 = (int16_t)row->arg4;
            receipt->new_creature_w0e = w0e;
            receipt->new_creature_w10 = w10;
            return 1;
        }

        /* c_ai.cpp:4467-4479 — opcode -10 (0xf6): set register */
        if (row->opcode == -10) {
            int8_t reg_idx = row->arg1;
            if (reg_idx >= 0 && reg_idx < 2) {
                int16_t val = (int16_t)row->arg2;
                /* Register 0 = creature word 0x0e, register 1 = word 0x10 */
                if (reg_idx == 0)
                    w0e = val;
                else
                    w10 = val;
                receipt->reg_writes++;
            }
        }

        /* c_ai.cpp:4480 — advance to next row */
        row_idx++;
    }
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_08f5 (c_ai.cpp:4375-4443) — Post-XACT result handler     */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_post_xact_result(const DM2_V1_PostXactResultRequest *req,
                                DM2_V1_PostXactResultReceipt *receipt)
{
    int8_t tbl_idx, row_idx;
    int8_t branch;
    int state_changed;
    const DM2_V1_ActionTableRow *row;
    int row_offset;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->table || req->table_row_count <= 0) {
        receipt->fail_closed = 1;
        return 0;
    }

    tbl_idx = req->table_index;
    row_idx = req->row_index;

    row_offset = (int)(uint8_t)row_idx;
    if (row_offset >= req->table_row_count) {
        receipt->fail_closed = 1;
        return 0;
    }

    row = &req->table[row_offset];

    /* c_ai.cpp:4390-4400 — select branch byte based on return code */
    if (req->xact_return_code == -2) {
        /* -2 = success: use byte 2 (arg2) */
        branch = row->arg2;
    } else {
        /* other (typically -3 = fail): use byte 1 (arg1) */
        branch = row->arg1;
    }

    /* c_ai.cpp:4402-4406 — if branch is -2 or -3, clear table_index */
    if (branch == -2 || branch == -3) {
        receipt->new_table_index = -1;
        receipt->new_row_index = 0;
        receipt->state_changed = 1;
        return 1;
    }

    state_changed = 0;

    /* c_ai.cpp:4408-4434 — decode branch codes */
    if (branch <= -5 || branch >= -3) {
        /* Not in the -8..-5 special range (or > -5):
         * c_ai.cpp:4410-4414 — direct row set, state changed if different */
        /* Actually: the condition is (RG1Blo > 0xfb || RG1Blo < 0xf8)
         * which means branch > -5 (signed) or branch < -8 (signed).
         * That covers: branch >= -4 or branch <= -9. */
        if (branch > -5 || branch < -8) {
            state_changed = (row_idx != branch) ? 1 : 0;
            row_idx = branch;
        } else {
            /* Special codes in [-8, -5] range */
            /* c_ai.cpp:4417-4434 */
            if (branch == -7) {
                /* -7 (0xf9): skip handled below with -8 */
                /* Actually branch == -7 => RG1Blo == 0xf9 => not -7, not -8 */
                /* Let me re-read: -5=0xfb, -6=0xfa, -7=0xf9, -8=0xf8 */
                /* 4417: if (RG1Blo != 0xf9) => not -7 */
                /* 4419: if (RG1Blo != 0xf8) => not -8 */
                /* 4422: if (RG1Blo != 0xfb) => not -5 */
                /* So: -7 => row_idx += 2 (line 4432) */
                row_idx = (int8_t)(row_idx + 2);
                state_changed = 1;
            } else if (branch == -8) {
                /* -8 (0xf8): row += 2 (line 4432) */
                row_idx = (int8_t)(row_idx + 2);
                state_changed = 1;
            } else if (branch == -5) {
                /* -5 (0xfb): row = row + 1 (line 4426-4428) */
                /* c_ai.cpp:4422-4426: -5 => RG2L=1, row = row + 1 */
                row_idx = (int8_t)(row_idx + 1);
                state_changed = 1;
            } else {
                /* -6 (0xfa): row = row + (-1) (line 4424, 4428) */
                row_idx = (int8_t)(row_idx - 1);
                state_changed = 1;
            }
        }
    }

    /* c_ai.cpp:4438-4441 — write back */
    receipt->new_table_index = tbl_idx;
    receipt->new_row_index = row_idx;
    receipt->state_changed = state_changed;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0389 (c_ai.cpp:3931-3978) — Target validation             */
/* Fail-closed: requires live dungeon data for DM2_14cd_0f0a call.    */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_validate_target(const DM2_V1_ValidateTargetRequest *req,
                               DM2_V1_ValidateTargetReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:3943 — if any of the three state bytes fail checks, target invalid */
    if (req->v1e07d8_b00 == 0 || req->v1e07d8_b01 == 0 || req->v1e07d8_b03 == -1) {
        receipt->target_valid = 0;
        receipt->table_index = (int8_t)0xff;
        return 1;
    }

    /* c_ai.cpp:3948 — if creature_b12 == -1, no current action table */
    if (req->creature_b12 == -1) {
        receipt->target_valid = 0;
        receipt->table_index = (int8_t)0xff;
        return 1;
    }

    /* c_ai.cpp:3953-3960 — would call DM2_14cd_0f0a with action table row
     * bytes 5/6. Fail-closed: we cannot execute the handler without live data. */
    receipt->fail_closed = 1;
    receipt->table_index = req->creature_b12;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0457 (c_ai.cpp:3979-4079) — Target selection              */
/* Fail-closed: requires DM2_MIN, DM2_COPY_MEMORY, live state.       */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_select_target(const DM2_V1_SelectTargetRequest *req,
                              DM2_V1_SelectTargetReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (req->candidate_count <= 0) {
        receipt->new_candidate_count = 0;
        receipt->selected_index = -1;
        return 1;
    }

    if (!req->candidates) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:3996-4077 — complex target priority adjustment, compaction,
     * and best-match selection. Requires DM2_MIN, DM2_COPY_MEMORY, and
     * live candidate data. Fail-closed. */
    receipt->fail_closed = 1;
    receipt->new_candidate_count = req->candidate_count;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0067 (c_ai.cpp:4495-4720) — Behavior selection            */
/* Fail-closed: requires DM2_RAND, DM2_GET_GLOB_VAR, live state.     */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_select_behavior(const DM2_V1_SelectBehaviorRequest *req,
                               DM2_V1_SelectBehaviorReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->behavior_table || req->behavior_entry_count <= 0) {
        receipt->fail_closed = 1;
        return 0;
    }

    if (!req->mode_flags) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:4513-4720 — complex flag mutation using random_seed, ai_flags,
     * creature state, mode_flags table, and behavior table walk with
     * DM2_GET_GLOB_VAR calls. Fail-closed: too many external dependencies. */
    receipt->fail_closed = 1;
    receipt->behavior_index = -1;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_SELECT_CREATURE_37FC (c_ai.cpp:4723-4740) — Mode selection     */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_select_creature_mode(const DM2_V1_SelectCreatureModeRequest *req,
                                    DM2_V1_SelectCreatureModeReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:4729-4733 — resolve v1e0584 if -1 */
    receipt->v1e0584 = req->v1e0584;
    if (req->v1e0584 == -1) {
        if (!req->query_gdat) {
            receipt->fail_closed = 1;
            return 0;
        }
        receipt->v1e0584 = req->query_gdat(req->gdat_ctx,
                                            req->creature_spec_b04, 1);
    }

    /* c_ai.cpp:4734 — call DM2_14cd_0067 with table1d6190[v1e0584] */
    if (receipt->v1e0584 < 0 ||
        receipt->v1e0584 >= req->behavior_table_count ||
        !req->behavior_tables) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* Forward to behavior selection — fail-closed due to dependencies */
    {
        DM2_V1_SelectBehaviorRequest breq = req->behavior_req;
        breq.behavior_table = req->behavior_tables[receipt->v1e0584];
        breq.v1e0584 = receipt->v1e0584;

        int r = dm2_v1_ai_select_behavior(&breq, &receipt->behavior_receipt);
        if (!r || receipt->behavior_receipt.fail_closed) {
            receipt->fail_closed = 1;
            return 0;
        }

        receipt->v1e0586 = receipt->behavior_receipt.behavior_index;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0550 (c_ai.cpp:4080-4175) — Action handler invocation     */
/* Fail-closed: requires DM2_14cd_0f0a, DM2_RAND16.                  */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_invoke_action_handler(const DM2_V1_InvokeActionHandlerRequest *req,
                                     DM2_V1_InvokeActionHandlerReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->entry || !req->action_tables || req->action_table_count <= 0) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:4098-4171 — iterates behavior entries with random probability
     * checks (DM2_RAND16), resolves action table rows, and calls
     * DM2_14cd_0f0a. Fail-closed. */
    receipt->fail_closed = 1;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0276 (c_ai.cpp:4176-4230) — Action context preparation    */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_prepare_action_context(const DM2_V1_PrepareActionContextRequest *req,
                                      DM2_V1_PrepareActionContextReceipt *receipt)
{
    int8_t b06;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->entry || req->entry_size < 0x1a) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:4186-4200 — extract fields from behavior entry */
    b06 = (int8_t)req->entry[6];
    receipt->v1e07d8_b00 = (b06 > 0) ? b06 : 0;  /* MAX(0, byte_at(entry, 6)) */
    receipt->v1e07d8_b01 = receipt->v1e07d8_b00;
    receipt->v1e07d8_w08 = (uint16_t)req->entry[4] | ((uint16_t)req->entry[5] << 8);
    receipt->v1e07d8_b03 = (int8_t)req->entry[7];
    receipt->v1e07d8_w04 = (uint16_t)req->entry[8] | ((uint16_t)req->entry[9] << 8);
    receipt->v1e07d8_w06 = (uint16_t)req->entry[0xa] | ((uint16_t)req->entry[0xb] << 8);
    receipt->v1e07d8_b02 = (int8_t)req->entry[0x11];

    /* c_ai.cpp:4200 — xp_0a = pointer_at(entry + 0x12) — architecture-dependent,
     * cannot be portably extracted. Documented in receipt. */

    /* c_ai.cpp:4202-4227 — if b00 > 0, allocates memory via DM2_ALLOCATION11 etc.
     * Fail-closed for the allocation part. */
    receipt->needs_allocation = (receipt->v1e07d8_b00 > 0) ? 1 : 0;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0684 (c_ai.cpp:4231-4374) — AI action table lookup        */
/* Fail-closed: requires DM2_14cd_0389, DM2_14cd_062e, DM2_14cd_0550,*/
/* DM2_14cd_0457, DM2_14cd_0276, DM2_FIND_WALK_PATH.                 */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_find_action_table(const DM2_V1_FindActionTableRequest *req,
                                 DM2_V1_FindActionTableReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->mode_flags) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:4246-4371 — orchestrates target validation, action handler
     * invocation, target selection, path finding, and context preparation.
     * Every sub-call requires live dungeon state. Fail-closed with -3. */
    receipt->fail_closed = 1;
    receipt->result_table_index = -3;

    return 1;
}


#include "csb_v1_chaos_magic_pc34_compat.h"
#include "csb_v1_csbwin_512_xor_pad_classify.h"
#include <stdlib.h>
#include <string.h>

/* pass603: CSB V1 Chaos magic / DSA system
 *
 * Source-locked to:
 *   CSBWin/Chaos.cpp: InitializeE (line 584)
 *   CSBWin/Chaos.cpp: _CALL0-_CALL9 (lines 60-69) — DSA call dispatch
 *   CSBWin/DSA.cpp: DSA interpreter core (5806 lines)
 *   CSBWin/CSBCode.cpp: _DisplayChaosStrikesBack (line 9196)
 *   CSBWin/CSBCode.cpp: StartChaos (line 11414)
 */

#define CSB_V1_DSA_FLAG_COUNT 256
#define CSB_V1_DSA_LOADED_BYTECODE_MAGIC 0x43534244u /* 'CSBD' */

static int csb_v1_csbwin_dsa_sign_extend(uint16_t value, int bits);

static int csb_v1_dsa_has_operands(const CSB_V1_DSAScript *script, int count) {
    return script && count >= 0 && script->pc <= script->bytecode_len &&
           count <= script->bytecode_len - script->pc;
}

static int csb_v1_dsa_flag_is_valid(int flag) {
    return flag >= 0 && flag < CSB_V1_DSA_FLAG_COUNT;
}

static int csb_v1_dsa_target_is_valid(const CSB_V1_DSAScript *script, int target) {
    return script && target >= 0 && target < script->bytecode_len;
}

static int csb_v1_dsa_reject_malformed_at(CSB_V1_DSAScript *script, int pc) {
    if (script) {
        script->pc = pc;
        script->active = 0;
    }
    return 0;
}

static void csb_v1_dsa_record_dispatch(CSB_V1_ChaosMagicState *state,
    CSB_V1_DSADispatchKind kind, int opcode, int operand, int op_pc)
{
    if (!state) return;
    state->dispatch_count++;
    state->last_dispatch.kind = kind;
    state->last_dispatch.opcode = opcode;
    state->last_dispatch.operand = operand;
    state->last_dispatch.op_pc = op_pc;
}

static uint32_t csb_v1_read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void csb_v1_dsa_free_imported_actions(CSB_V1_ChaosMagicState *state) {
    int i;
    if (!state) return;
    for (i = 0; i < state->imported_action_count; ++i) {
        free(state->imported_actions[i].program_words);
    }
    free(state->imported_actions);
    state->imported_actions = NULL;
    state->imported_action_count = 0;
}

void csb_v1_chaos_init(CSB_V1_ChaosMagicState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

void csb_v1_chaos_cleanup(CSB_V1_ChaosMagicState *state) {
    if (!state) return;
    if (state->loaded_bytecode_magic == CSB_V1_DSA_LOADED_BYTECODE_MAGIC) {
        free(state->loaded_bytecode);
    }
    csb_v1_dsa_free_imported_actions(state);
    memset(state, 0, sizeof(*state));
}

int csb_v1_chaos_import_extended_save_dsas(CSB_V1_ChaosMagicState *state,
    const uint8_t *bytes, int size)
{
    CSB_V1_CSBWinExtendedDSAReport report;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    CSB_V1_DSAImportedAction *actions = NULL;
    CSB_V1_CSBWinDSAImportedHeader
        headers[CSB_V1_MAX_DSA_SCRIPTS];
    size_t offset;
    int action_count = 0;
    uint16_t dsa_ordinal;

    if (!state || !bytes || size <= 0) return -1;
    if (csb_v1_csbwin_512_inspect_extended_dsa_section(bytes, (size_t)size,
            &report, &features) != CSB_V1_CSBWIN_EXTENDED_OK || !report.valid) {
        return -1;
    }
    if (report.action_count > 4096u || report.program_word_count >
        CSB_V1_MAX_DSA_BYTECODE_BYTES / 2u) {
        return -1;
    }
    if (report.action_count != 0u) {
        actions = calloc((size_t)report.action_count, sizeof(*actions));
        if (!actions) return -1;
    }
    memset(headers, 0, sizeof(headers));

    offset = features.extension_payload_offset;
    for (dsa_ordinal = 0u; dsa_ordinal < features.dsa_count; ++dsa_ordinal) {
        uint32_t dsa_id;
        uint32_t persistent_state;
        uint32_t local_state;
        uint32_t group_id;
        uint32_t state_slot_count;
        uint32_t non_empty_states;
        uint32_t state_ordinal;
        if (offset > (size_t)size || (size_t)size - offset < 108u) goto reject;
        dsa_id = csb_v1_read_le32(bytes + offset);
        offset += 4u + 80u;
        persistent_state = csb_v1_read_le32(bytes + offset);
        offset += 4u;
        local_state = csb_v1_read_le32(bytes + offset);
        offset += 4u;
        group_id = csb_v1_read_le32(bytes + offset);
        offset += 4u;
        state_slot_count = csb_v1_read_le32(bytes + offset);
        offset += 4u;
        if (dsa_id < CSB_V1_MAX_DSA_SCRIPTS) {
            headers[dsa_id].valid = 1;
            headers[dsa_id].persistent_state = persistent_state;
            headers[dsa_id].local_state = local_state;
            headers[dsa_id].group_id = group_id;
            headers[dsa_id].state_slot_count = state_slot_count;
        }
        offset += 4u; /* first displayed state */
        non_empty_states = csb_v1_read_le32(bytes + offset);
        offset += 4u;
        for (state_ordinal = 0u; state_ordinal < non_empty_states; ++state_ordinal) {
            uint32_t state_index;
            uint32_t program_count;
            uint32_t action_ordinal;
            if (offset > (size_t)size || (size_t)size - offset < 8u) goto reject;
            state_index = csb_v1_read_le32(bytes + offset);
            offset += 4u;
            program_count = csb_v1_read_le32(bytes + offset);
            offset += 4u;
            for (action_ordinal = 0u; action_ordinal < program_count; ++action_ordinal) {
                uint32_t words;
                size_t byte_count;
                int word;
                if (action_count >= (int)report.action_count ||
                    offset > (size_t)size || (size_t)size - offset < 8u) goto reject;
                actions[action_count].dsa_id = (uint8_t)dsa_id;
                actions[action_count].state_index = state_index;
                actions[action_count].column = csb_v1_read_le32(bytes + offset);
                offset += 4u;
                words = csb_v1_read_le32(bytes + offset);
                offset += 4u;
                byte_count = (size_t)words * 2u;
                if (words > CSB_V1_MAX_DSA_BYTECODE_BYTES / 2u ||
                    byte_count > (size_t)size - offset) goto reject;
                if (words != 0u) {
                    actions[action_count].program_words = calloc(words, sizeof(uint16_t));
                    if (!actions[action_count].program_words) goto reject;
                    for (word = 0; word < (int)words; ++word) {
                        actions[action_count].program_words[word] =
                            (uint16_t)bytes[offset + (size_t)word * 2u] |
                            ((uint16_t)bytes[offset + (size_t)word * 2u + 1u] << 8);
                    }
                }
                actions[action_count].program_word_count = (int)words;
                ++action_count;
                offset += byte_count;
            }
        }
    }
    if (action_count != (int)report.action_count ||
        offset != report.dsa_payload_offset + report.dsa_payload_size) goto reject;

    csb_v1_dsa_free_imported_actions(state);
    memcpy(state->imported_headers, headers, sizeof(headers));
    state->imported_actions = actions;
    state->imported_action_count = action_count;
    return action_count;

reject:
    if (actions) {
        int i;
        for (i = 0; i < (int)report.action_count; ++i) free(actions[i].program_words);
    }
    free(actions);
    return -1;
}

const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal)
{
    int i;
    if (!state || dsa_id < 0 || dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        action_ordinal < 0) return NULL;
    for (i = 0; i < state->imported_action_count; ++i) {
        const CSB_V1_DSAImportedAction *action = &state->imported_actions[i];
        if (action->dsa_id == (uint8_t)dsa_id &&
            action->state_index == state_index) {
            if (action_ordinal-- == 0) return action;
        }
    }
    return NULL;
}

const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action_column(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column)
{
    int i;
    if (!state || dsa_id < 0 || dsa_id >= CSB_V1_MAX_DSA_SCRIPTS) return NULL;
    /* CSBWin DSA.cpp:5717-5740 DSAState::Program/ProgramSize scans the
     * serialized action order and takes the first exact column match. */
    for (i = 0; i < state->imported_action_count; ++i) {
        const CSB_V1_DSAImportedAction *action = &state->imported_actions[i];
        if (action->dsa_id == (uint8_t)dsa_id &&
            action->state_index == state_index && action->column == column) {
            return action;
        }
    }
    return NULL;
}

const CSB_V1_DSAImportedAction *
csb_v1_chaos_resolve_imported_master_filter_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint16_t actuator_word2,
    uint32_t input_column, uint32_t *out_state_index, int *out_action_ordinal)
{
    const CSB_V1_CSBWinDSAImportedHeader *header;
    const CSB_V1_DSAImportedAction *action;
    uint32_t state_index;
    int i;
    int ordinal = 0;

    if (!state || dsa_id < 0 || dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        !out_state_index || !out_action_ordinal) return NULL;
    header = &state->imported_headers[dsa_id];
    /* DSA.cpp FindMaster explicitly leaves LocalState 3 unimplemented; the
     * other source state stores are distinct ownership models.  This bridge
     * admits only a master whose state is DB3::DSAstate(). */
    if (!header->valid || header->local_state != 0u) return NULL;
    state_index = (uint32_t)((actuator_word2 >> 12) & 0x0fu);
    if (state_index >= header->state_slot_count) return NULL;
    action = csb_v1_chaos_find_imported_action_column(
        state, dsa_id, state_index, input_column);
    if (!action) return NULL;
    for (i = 0; i < state->imported_action_count; ++i) {
        const CSB_V1_DSAImportedAction *candidate =
            &state->imported_actions[i];
        if (candidate->dsa_id == (uint8_t)dsa_id &&
            candidate->state_index == state_index) {
            if (candidate == action) {
                *out_state_index = state_index;
                *out_action_ordinal = ordinal;
                return action;
            }
            ++ordinal;
        }
    }
    return NULL;
}

CSB_V1_CSBWinDSAJumpResult
csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAJumpDispatch *out_dispatch)
{
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAJumpDispatch candidate;
    uint16_t command;
    int cursor = 1;
    int relative_state;
    uint32_t target_state;
    uint32_t target_column = 0u;

    if (!state || !out_dispatch || dsa_id < 0 ||
        dsa_id >= CSB_V1_MAX_DSA_SCRIPTS) {
        return CSB_V1_CSBWIN_DSA_JUMP_NOT_AUTHENTICATED;
    }
    action = csb_v1_chaos_find_imported_action_column(state, dsa_id,
                                                        state_index, column);
    if (!action) return CSB_V1_CSBWIN_DSA_JUMP_NOT_FOUND;
    if (!action->program_words || action->program_word_count < 1) {
        return CSB_V1_CSBWIN_DSA_JUMP_MALFORMED;
    }

    command = action->program_words[0];
    if ((command & 0x3fu) != CSB_V1_CSBWIN_DSACMD_JUMP) {
        return CSB_V1_CSBWIN_DSA_JUMP_NOT_JUMP;
    }
    /* CSBWin Data.h:2090-2116 and DSA.cpp:812-849: DSACMD_JUMP has
     * row[6], column-present[1], nextState[3], then optional extensions in
     * next-state, row, column order. This resolver cannot activate filters
     * or mutate the world. */
    relative_state = csb_v1_csbwin_dsa_sign_extend(
        (uint16_t)(command >> 13), 3);
    if (relative_state == -4) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_JUMP_MALFORMED;
        }
        relative_state = (int)(int16_t)action->program_words[cursor++];
    }
    target_state = (uint32_t)((command >> 6) & 0x3fu);
    if (target_state == 63u) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_JUMP_MALFORMED;
        }
        target_state = action->program_words[cursor++];
    }
    if ((command & 0x1000u) != 0u) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_JUMP_MALFORMED;
        }
        target_column = action->program_words[cursor++];
    }
    if (cursor != action->program_word_count) {
        return CSB_V1_CSBWIN_DSA_JUMP_MALFORMED;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.source_state = state_index;
    candidate.source_column = column;
    candidate.continuation_state = (int)state_index + relative_state;
    candidate.target_state = target_state;
    candidate.target_column = target_column;
    candidate.words_consumed = (uint16_t)cursor;
    *out_dispatch = candidate;
    return CSB_V1_CSBWIN_DSA_JUMP_OK;
}

CSB_V1_CSBWinDSAGosubResult
csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAGosubDispatch *out_dispatch)
{
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSAGosubDispatch candidate;
    uint16_t command;
    int cursor = 1;
    int relative_state;
    uint32_t target_state;
    uint32_t target_column = 0u;

    if (!state || !out_dispatch || dsa_id < 0 ||
        dsa_id >= CSB_V1_MAX_DSA_SCRIPTS) {
        return CSB_V1_CSBWIN_DSA_GOSUB_NOT_AUTHENTICATED;
    }
    action = csb_v1_chaos_find_imported_action_column(state, dsa_id,
                                                        state_index, column);
    if (!action) return CSB_V1_CSBWIN_DSA_GOSUB_NOT_FOUND;
    if (!action->program_words || action->program_word_count < 1) {
        return CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED;
    }

    command = action->program_words[0];
    if ((command & 0x3fu) != CSB_V1_CSBWIN_DSACMD_GOSUB) {
        return CSB_V1_CSBWIN_DSA_GOSUB_NOT_GOSUB;
    }
    /* CSBWin Data.h:2093-2119 and DSA.cpp:764-808: DSACMD_GOSUB
     * decodes the same row/column transfer form as JUMP, but retains the
     * outer next-state and enters Execute() one frame deeper. */
    relative_state = csb_v1_csbwin_dsa_sign_extend(
        (uint16_t)(command >> 13), 3);
    if (relative_state == -4) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED;
        }
        /* EX_GOSUB assigns this ui16 source word directly to i32 rather than
         * applying the signed conversion used by LOAD and variable commands.
         * ReDMCSB: CSBWin DSA.cpp:772-780. */
        relative_state = (int)action->program_words[cursor++];
    }
    target_state = (uint32_t)((command >> 6) & 0x3fu);
    if (target_state == 63u) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED;
        }
        target_state = action->program_words[cursor++];
    }
    if ((command & 0x1000u) != 0u) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED;
        }
        target_column = action->program_words[cursor++];
    }
    if (cursor != action->program_word_count) {
        return CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED;
    }

    memset(&candidate, 0, sizeof(candidate));
    candidate.source_state = state_index;
    candidate.source_column = column;
    candidate.continuation_state = (int)state_index + relative_state;
    candidate.target_state = target_state;
    candidate.target_column = target_column;
    candidate.subroutine_depth_delta = 1u;
    candidate.words_consumed = (uint16_t)cursor;
    *out_dispatch = candidate;
    return CSB_V1_CSBWIN_DSA_GOSUB_OK;
}

typedef struct {
    int final_state;
    int subroutine_depth;
} CSB_V1_CSBWinDSAExecuteReturnFrame;

CSB_V1_CSBWinDSAExecuteResult
csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, int initial_subroutine_depth,
    CSB_V1_CSBWinDSAExecuteReceipt *out_receipt)
{
    CSB_V1_CSBWinDSAExecuteReturnFrame return_frames[
        CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH];
    CSB_V1_CSBWinDSAExecuteReceipt candidate;
    uint32_t current_state = state_index;
    uint32_t current_column = column;
    int final_state = -1;
    int current_depth = initial_subroutine_depth;
    int return_depth = 0;

    if (!state || !out_receipt || dsa_id < 0 ||
        dsa_id >= CSB_V1_MAX_DSA_SCRIPTS || initial_subroutine_depth < 0) {
        return CSB_V1_CSBWIN_DSA_EXECUTE_NOT_AUTHENTICATED;
    }
    if (initial_subroutine_depth >= CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH) {
        return CSB_V1_CSBWIN_DSA_EXECUTE_DEPTH_LIMIT;
    }
    memset(&candidate, 0, sizeof(candidate));
    candidate.source_state = state_index;
    candidate.source_column = column;
    candidate.initial_subroutine_depth = initial_subroutine_depth;
    candidate.final_state = -1;
    candidate.maximum_subroutine_depth = (uint8_t)initial_subroutine_depth;

    /* CSBWin DSA.cpp:5053-5293 Execute. The transfer subset keeps Execute's
     * file-order Program(state,column) selection, JUMP frame transfer, and
     * GOSUB nested-frame return behavior without promoting any world/filter
     * command. RETURN is not a bytecode: Execute returns when the selected
     * state/column has no Program, and EX_GOSUB ignores that child return
     * value before its caller continues. */
    for (;;) {
        const CSB_V1_DSAImportedAction *action =
            csb_v1_chaos_find_imported_action_column(state, dsa_id,
                                                       current_state,
                                                       current_column);
        uint16_t opcode;

        if (!action) {
            int returned_state = final_state == -1 ? (int)current_state : final_state;
            ++candidate.return_count;
            candidate.returned_by_missing_program = 1;
            if (return_depth == 0) {
                candidate.final_state = returned_state;
                *out_receipt = candidate;
                return CSB_V1_CSBWIN_DSA_EXECUTE_OK;
            }
            --return_depth;
            ++candidate.frame_pop_count;
            final_state = return_frames[return_depth].final_state;
            current_depth = return_frames[return_depth].subroutine_depth;
            /* A complete GOSUB action has no remaining caller words. */
            if (return_depth == 0) {
                candidate.final_state = final_state;
                *out_receipt = candidate;
                return CSB_V1_CSBWIN_DSA_EXECUTE_OK;
            }
            continue;
        }
        if (!action->program_words || action->program_word_count < 1) {
            return CSB_V1_CSBWIN_DSA_EXECUTE_MALFORMED;
        }
        opcode = (uint16_t)(action->program_words[0] & 0x3fu);
        if (candidate.transfer_count >= CSB_V1_CSBWIN_DSA_EXECUTE_MAX_TRANSFERS) {
            return CSB_V1_CSBWIN_DSA_EXECUTE_TRANSFER_LIMIT;
        }
        if (opcode == CSB_V1_CSBWIN_DSACMD_JUMP) {
            CSB_V1_CSBWinDSAJumpDispatch dispatch;
            CSB_V1_CSBWinDSAJumpResult rc =
                csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
                    state, dsa_id, current_state, current_column, &dispatch);
            if (rc == CSB_V1_CSBWIN_DSA_JUMP_MALFORMED) {
                return CSB_V1_CSBWIN_DSA_EXECUTE_MALFORMED;
            }
            if (rc != CSB_V1_CSBWIN_DSA_JUMP_OK) {
                return CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED;
            }
            if (final_state == -1 && dispatch.continuation_state >= 0) {
                final_state = dispatch.continuation_state;
            }
            ++candidate.transfer_count;
            candidate.words_consumed = (uint16_t)(candidate.words_consumed +
                                                   dispatch.words_consumed);
            current_state = dispatch.target_state;
            current_column = dispatch.target_column;
            continue;
        }
        if (opcode == CSB_V1_CSBWIN_DSACMD_GOSUB) {
            CSB_V1_CSBWinDSAGosubDispatch dispatch;
            CSB_V1_CSBWinDSAGosubResult rc =
                csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
                    state, dsa_id, current_state, current_column, &dispatch);
            if (rc == CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED) {
                return CSB_V1_CSBWIN_DSA_EXECUTE_MALFORMED;
            }
            if (rc != CSB_V1_CSBWIN_DSA_GOSUB_OK) {
                return CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED;
            }
            if (return_depth >= CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH - 1 ||
                current_depth >= CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH - 1) {
                return CSB_V1_CSBWIN_DSA_EXECUTE_DEPTH_LIMIT;
            }
            if (final_state == -1 && dispatch.continuation_state >= 0) {
                final_state = dispatch.continuation_state;
            }
            return_frames[return_depth].final_state = final_state;
            return_frames[return_depth].subroutine_depth = current_depth;
            ++return_depth;
            ++candidate.frame_push_count;
            ++candidate.transfer_count;
            candidate.words_consumed = (uint16_t)(candidate.words_consumed +
                                                   dispatch.words_consumed);
            ++current_depth;
            if ((uint8_t)current_depth > candidate.maximum_subroutine_depth) {
                candidate.maximum_subroutine_depth = (uint8_t)current_depth;
            }
            current_state = dispatch.target_state;
            current_column = dispatch.target_column;
            final_state = -1;
            continue;
        }
        return CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED;
    }
}

static int csb_v1_csbwin_dsa_sign_extend(uint16_t value, int bits) {
    uint16_t sign = (uint16_t)(1u << (bits - 1));
    return (int)((value ^ sign) - sign);
}

__attribute__((unused))
static int csb_v1_csbwin_dsa_location_level(uint32_t location)
{
    return (int)((location >> 10) & 0x3fu);
}

__attribute__((unused))
static int csb_v1_csbwin_dsa_location_x(uint32_t location)
{
    return (int)((location >> 5) & 0x1fu);
}

__attribute__((unused))
static int csb_v1_csbwin_dsa_location_y(uint32_t location)
{
    return (int)(location & 0x1fu);
}

static int csb_v1_csbwin_dsa_core_subcode_supported(uint16_t subcode,
                                                    int *requires_runtime_owner)
{
    if (requires_runtime_owner) *requires_runtime_owner = 0;
    switch (subcode) {
    case 1u: case 2u: case 3u: case 4u: case 5u: case 6u: case 7u:
    case 11u: case 12u: case 13u: case 14u: case 15u: case 16u: case 17u:
    case 18u: case 19u: case 20u: case 21u: case 22u: case 23u: case 24u: case 25u:
    case 26u: case 27u: case 28u: case 29u: case 30u: case 31u: case 32u:
    case 37u: case 38u: case 39u: case 40u: case 41u: case 48u: case 50u:
    case 59u: case 67u: case 68u: case 70u: case 97u: case 98u: case 99u: case 108u: case 129u:
    case 133u: case 136u: case 139u:
        return 1;
    case 8u: case 33u: case 34u: case 35u: case 36u: case 42u: case 44u: case 45u:
    case 46u: case 49u: case 51u: case 52u: case 53u: case 54u: case 55u:
    case 56u: case 57u: case 58u: case 60u: case 62u: case 63u: case 64u: case 65u:
    case 66u: case 69u: case 71u: case 72u: case 73u: case 74u: case 75u: case 76u:
    case 95u: case 96u:
    case 77u: case 78u: case 92u: case 100u: case 101u: case 102u: case 106u:
    case 47u: case 90u: case 103u: case 104u: case 105u: case 107u: case 109u: case 110u: case 112u: case 113u: case 114u:
    case 115u: case 116u: case 117u: case 118u: case 123u: case 124u: case 125u:
    case 121u: case 122u: case 130u: case 131u: case 132u: case 134u: case 135u: case 137u:
    case 138u:
        if (requires_runtime_owner) *requires_runtime_owner = 1;
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_csbwin_dsa_stack_push(uint32_t *stack, int *depth,
                                         uint32_t value);

/* CSBWin DSA.cpp EX_AMPERSAND expands I_Indirect from the live parameter
 * array before it enters the normal stack-word dispatcher.  Firestaff keeps
 * the parameter count outside that array, so this adapter is deliberately
 * strict: only the direct operations which already own a transactional
 * runtime callback may be selected. Its local-variable rewrite targets the
 * action-local DSAVARS bank, exactly as CSBWin ProcessDSATimer does. */
static CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_expand_indirect(uint32_t *stack, int *depth,
    uint32_t *variables, uint8_t *variable_state, uint32_t *parameters,
    int parameter_count, uint16_t *out_subcode,
    int *out_effective_parameter_count)
{
    uint16_t direct_subcode;
    uint32_t stack_count;
    uint32_t return_parameter_count;
    int cursor;
    int i;

    if (!stack || !depth || !variables || !variable_state || !parameters ||
        !out_subcode ||
        !out_effective_parameter_count || parameter_count < 3 ||
        parameter_count > 26) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    switch (parameters[0]) {
    case 84u: direct_subcode = 63u; break;  /* I_Monster! -> Monster! */
    case 85u: direct_subcode = 66u; break;  /* I_Char! -> Char! */
    case 86u: direct_subcode = 73u; break;  /* I_Move -> Move */
    case 81u: direct_subcode = 68u; break;  /* I_CreateCloud -> CreateCloud */
    case 83u: direct_subcode = 62u; break;  /* I_TeleportParty -> TeleportParty */
    case 93u: direct_subcode = 95u; break;  /* I_DelMon -> DelMon */
    case 94u: direct_subcode = 96u; break;  /* I_InsMon -> InsMon */
    case 87u: direct_subcode = 76u; break;  /* I_Copy -> Copy */
    case 88u: direct_subcode = 58u; break;  /* I_Cell! -> Cell! */
    case 111u: direct_subcode = 110u; break; /* I_CausePoison */
    case 126u: direct_subcode = 118u; break; /* I_SwapCharacter */
    default:
        return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
    }

    stack_count = parameters[1];
    if (stack_count > (uint32_t)(parameter_count - 3)) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    /* Source starts at pDSAparameters[1], then pushes P3..P(2+n) in
     * reverse order. Firestaff's zero-based payload is P1..Pn. */
    cursor = 2 + (int)stack_count;
    for (i = 0; i < (int)stack_count; ++i) {
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, parameters[--cursor])) {
            return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        }
    }
    cursor += (int)stack_count;
    if (cursor >= parameter_count) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    /* CSBWin's DSAVARS belongs to this ProcessDSATimer action. Apply the
     * source parameter-backed rewrite to the staged action-local bank before
     * the selected direct stack word observes it. */
    if (parameters[cursor++] != 0u) {
        uint32_t variable_count;
        uint32_t variable_index;

        if (parameter_count < 4) {
            return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        }
        variable_count = parameters[2];
        variable_index = parameters[3];
        if (variable_index > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            variable_count >
                CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - variable_index ||
            variable_count > (uint32_t)(parameter_count - cursor)) {
            return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        }
        for (i = 0; i < (int)variable_count; ++i) {
            variables[variable_index + (uint32_t)i] = parameters[cursor++];
            variable_state[variable_index + (uint32_t)i] = 1u;
        }
    }
    if (cursor >= parameter_count) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    return_parameter_count = parameters[cursor++];
    if (return_parameter_count > 26u ||
        return_parameter_count > (uint32_t)(parameter_count - cursor)) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    for (i = 0; i < (int)return_parameter_count; ++i) {
        parameters[i] = parameters[cursor++];
    }
    *out_subcode = direct_subcode;
    *out_effective_parameter_count = (int)return_parameter_count;
    return CSB_V1_CSBWIN_DSA_STACK_OK;
}

static CSB_V1_CSBWinDSACoreVerifyResult
csb_v1_csbwin_dsa_verify_message_words(
    const CSB_V1_DSAImportedAction *action, uint16_t command,
    uint8_t opcode, int *cursor, CSB_V1_CSBWinDSACoreProgramReceipt *receipt)
{
    int next_state;
    uint8_t delay_kind;
    uint8_t target_kind;
    int extra_words = 0;

    if (!action || !cursor || !receipt) {
        return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    }
    if (opcode != CSB_V1_CSBWIN_DSACMD_MESSAGE &&
        opcode != CSB_V1_CSBWIN_DSACMD_MESSAGE32 &&
        opcode != CSB_V1_CSBWIN_DSACMD_DESSAGE32) {
        return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
    }
    next_state = csb_v1_csbwin_dsa_sign_extend(
        (uint16_t)((command >> 12) & 0x0fu), 4);
    delay_kind = (uint8_t)((command >> 8) & 0x03u);
    target_kind = (uint8_t)((command >> 10) & 0x03u);
    if (next_state == -8) ++extra_words;
    if (delay_kind == 3u) ++extra_words;
    if (target_kind == 2u) {
        extra_words +=
            (opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE) ? 1 : 2;
    }
    if (extra_words > action->program_word_count - *cursor) {
        return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    }
    *cursor += extra_words;
    receipt->timer_core = 1;
    receipt->message_core = 1;
    receipt->requires_runtime_owner = 1;
    return CSB_V1_CSBWIN_DSA_CORE_OK;
}

static CSB_V1_CSBWinDSACoreVerifyResult
csb_v1_csbwin_dsa_verify_copyteleporter_words(
    const CSB_V1_DSAImportedAction *action, uint16_t command,
    uint8_t opcode, int *cursor, CSB_V1_CSBWinDSACoreProgramReceipt *receipt)
{
    int next_state;
    uint8_t source_kind;
    uint8_t destination_kind;
    int extra_words = 0;

    if (!action || !cursor || !receipt) {
        return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    }
    if (opcode != CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER &&
        opcode != CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32) {
        return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
    }
    source_kind = (uint8_t)((command >> 6) & 0x03u);
    destination_kind = (uint8_t)((command >> 8) & 0x03u);
    next_state = csb_v1_csbwin_dsa_sign_extend(
        (uint16_t)((command >> 10) & 0x3fu), 6);
    if (next_state == -32) ++extra_words;
    if (source_kind == 2u) {
        extra_words +=
            opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER ? 1 : 2;
    }
    if (destination_kind == 2u) {
        extra_words +=
            opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER ? 1 : 2;
    }
    if (extra_words > action->program_word_count - *cursor) {
        return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    }
    *cursor += extra_words;
    receipt->requires_runtime_owner = 1;
    receipt->dungeon_mutation_core = 1;
    return CSB_V1_CSBWIN_DSA_CORE_OK;
}

static int csb_v1_csbwin_dsa_subcode_is_arithmetic(uint16_t subcode)
{
    switch (subcode) {
    case 1u: case 2u: case 3u: case 4u: case 5u: case 6u: case 7u:
    case 11u: case 12u: case 13u: case 14u: case 15u: case 16u: case 17u:
    case 18u: case 19u: case 20u: case 21u: case 22u: case 24u: case 25u:
    case 27u: case 28u: case 29u: case 30u: case 31u: case 32u: case 37u:
    case 50u: case 67u: case 68u: case 70u: case 108u:
        return 1;
    default:
        return 0;
    }
}

/* DSA.cpp:2613-2699 keeps these boolean-producing stack operators distinct
 * from arithmetic and transfer operators.  Preserve that distinction in the
 * source receipt so the runtime can bind the exact accepted family. */
static int csb_v1_csbwin_dsa_subcode_is_comparison(uint16_t subcode)
{
    switch (subcode) {
    case 5u:  /* STKOP_Equal */
    case 27u: /* STKOP_Less */
    case 29u: /* STKOP_NotEqual */
    case 32u: /* STKOP_Not */
    case 50u: /* STKOP_ULess */
        return 1;
    default:
        return 0;
    }
}

/* CSBWin DSA.cpp:2613-2699: these values either produce a boolean or
 * combine boolean masks which later drive QUESTION/CASE/JUMP selection. */
static int csb_v1_csbwin_dsa_subcode_is_conditional(uint16_t subcode)
{
    switch (subcode) {
    case 5u:   /* STKOP_Equal */
    case 19u:  /* STKOP_And */
    case 22u:  /* STKOP_Or */
    case 27u:  /* STKOP_Less */
    case 29u:  /* STKOP_NotEqual */
    case 32u:  /* STKOP_Not */
    case 50u:  /* STKOP_ULess */
    case 70u:  /* STKOP_Xor */
    case 98u:  /* STKOP_JumpGear */
    case 99u:  /* STKOP_GosubGear */
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_csbwin_dsa_subcode_is_timer_family(uint16_t subcode)
{
    switch (subcode) {
    case 110u: /* STKOP_CausePoison may schedule TT_75 through runtime. */
    case 138u: /* STKOP_ModifyMessage remaps the current ProcessTimers call. */
        return 1;
    default:
        return 0;
    }
}

static int csb_v1_csbwin_dsa_subcode_is_dungeon_mutation(uint16_t subcode)
{
    switch (subcode) {
    case 33u:  /* STKOP_FalsePit */
    case 34u:  /* STKOP_GeneratorDelayStore */
    case 45u:  /* STKOP_StoreExCellFlg */
    case 52u:  /* STKOP_SetCurse */
    case 54u:  /* STKOP_SetCharges */
    case 56u:  /* STKOP_SetBroken */
    case 58u:  /* STKOP_CellStore */
    case 63u:  /* STKOP_MonsterStore */
    case 73u:  /* STKOP_Move */
    case 95u:  /* STKOP_DelMon */
    case 96u:  /* STKOP_InsMon */
    case 66u:  /* STKOP_CharStore */
    case 75u:  /* STKOP_SetPoisoned */
    case 76u:  /* STKOP_Copy */
    case 102u: /* STKOP_MissileInfoStore */
    case 110u: /* STKOP_CausePoison */
    case 113u: /* STKOP_ExperiencePlus */
    case 118u: /* STKOP_SwapCharacter */
    case 125u: /* STKOP_SetSubType */
    case 132u: /* STKOP_SetSkin */
    case 135u: /* STKOP_TalentsStore */
        return 1;
    default:
        return 0;
    }
}

CSB_V1_CSBWinDSACoreVerifyResult
csb_v1_csbwin_dsa_verify_authenticated_core_program(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSACoreProgramReceipt *out_receipt)
{
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_CSBWinDSACoreProgramReceipt receipt;
    int cursor = 0;
    int saw_stack_command = 0;

    if (!out_receipt) return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    memset(&receipt, 0, sizeof(receipt));
    receipt.unsupported_opcode = 0xffu;
    *out_receipt = receipt;
    if (!state || dsa_id < 0 || dsa_id >= CSB_V1_MAX_DSA_SCRIPTS ||
        action_ordinal < 0) {
        return CSB_V1_CSBWIN_DSA_CORE_NOT_AUTHENTICATED;
    }
    action = csb_v1_chaos_find_imported_action(
        state, dsa_id, state_index, action_ordinal);
    if (!action || !action->program_words || action->program_word_count < 1) {
        return CSB_V1_CSBWIN_DSA_CORE_NOT_AUTHENTICATED;
    }

    while (cursor < action->program_word_count) {
        uint16_t command = action->program_words[cursor++];
        uint8_t opcode = (uint8_t)(command & 0x3fu);
        ++receipt.command_count;
        if (opcode == CSB_V1_CSBWIN_DSACMD_JUMP ||
            opcode == CSB_V1_CSBWIN_DSACMD_GOSUB) {
            uint16_t words_consumed = 0u;

            if (receipt.command_count != 1u) {
                receipt.unsupported_opcode = opcode;
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
            }
            if (opcode == CSB_V1_CSBWIN_DSACMD_JUMP) {
                CSB_V1_CSBWinDSAJumpDispatch dispatch;
                CSB_V1_CSBWinDSAJumpResult rc =
                    csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
                        state, dsa_id, state_index, action->column,
                        &dispatch);
                if (rc == CSB_V1_CSBWIN_DSA_JUMP_MALFORMED) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                if (rc != CSB_V1_CSBWIN_DSA_JUMP_OK) {
                    receipt.unsupported_opcode = opcode;
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
                }
                words_consumed = dispatch.words_consumed;
            } else {
                CSB_V1_CSBWinDSAGosubDispatch dispatch;
                CSB_V1_CSBWinDSAGosubResult rc =
                    csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
                        state, dsa_id, state_index, action->column,
                        &dispatch);
                if (rc == CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                if (rc != CSB_V1_CSBWIN_DSA_GOSUB_OK) {
                    receipt.unsupported_opcode = opcode;
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
                }
                words_consumed = dispatch.words_consumed;
            }
            if (words_consumed != (uint16_t)action->program_word_count) {
                receipt.unsupported_opcode = opcode;
                receipt.words_consumed = words_consumed;
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
            }
            receipt.transfer_only = 1;
            receipt.words_consumed = words_consumed;
            receipt.valid = 1;
            *out_receipt = receipt;
            return CSB_V1_CSBWIN_DSA_CORE_OK;
        }
        saw_stack_command = 1;
        if (opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE ||
            opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE32 ||
            opcode == CSB_V1_CSBWIN_DSACMD_DESSAGE32) {
            CSB_V1_CSBWinDSACoreVerifyResult message_rc =
                csb_v1_csbwin_dsa_verify_message_words(
                    action, command, opcode, &cursor, &receipt);
            if (message_rc != CSB_V1_CSBWIN_DSA_CORE_OK) {
                *out_receipt = receipt;
                return message_rc;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER ||
                   opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32) {
            CSB_V1_CSBWinDSACoreVerifyResult copy_rc =
                csb_v1_csbwin_dsa_verify_copyteleporter_words(
                    action, command, opcode, &cursor, &receipt);
            if (copy_rc != CSB_V1_CSBWIN_DSA_CORE_OK) {
                *out_receipt = receipt;
                return copy_rc;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_LOAD) {
            uint8_t selector = (uint8_t)((command >> 6) & 0x1fu);
            int next_state =
                csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 11), 5);
            if (next_state == -16) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
            if (selector == CSB_V1_CSBWIN_DSA_LOAD_ABS32) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL;
            }
            if (selector == CSB_V1_CSBWIN_DSA_LOAD_INTEGER ||
                selector == CSB_V1_CSBWIN_DSA_LOAD_ABS) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            } else if (selector == CSB_V1_CSBWIN_DSA_LOAD_INTEGER32) {
                if (cursor + 1 >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                cursor += 2;
            } else if (selector != CSB_V1_CSBWIN_DSA_LOAD_DOLLAR &&
                       selector > 25u) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_FETCH) {
            int next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            receipt.query_core = 1;
            receipt.requires_runtime_owner = 1;
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_NOOP ||
                   opcode == CSB_V1_CSBWIN_DSACMD_EQUAL) {
            int next_state =
                csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 6), 10);
            if (opcode == CSB_V1_CSBWIN_DSACMD_EQUAL) {
                receipt.conditional_core = 1;
                receipt.comparison_core = 1;
            }
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_QUESTION) {
            int next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)((command >> 6) & 0x0fu), 4);
            uint8_t if_command = (uint8_t)((command >> 11) & 0x03u);
            uint8_t else_command = (uint8_t)((command >> 14) & 0x03u);
            uint8_t if_column = (uint8_t)((command >> 10) & 0x01u);
            uint8_t else_column = (uint8_t)((command >> 13) & 0x01u);
            int extension_count;
            receipt.conditional_core = 1;
            if (next_state == -2) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
            extension_count = (if_command != 0u) + if_column +
                (else_command != 0u) + else_column;
            if (extension_count > action->program_word_count - cursor) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
            }
            cursor += extension_count;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_CASE) {
            int next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            uint16_t case_count;

            /* Data.h:2208-2237 and DSA.cpp:981-1025: CASE carries a
             * signed NextState, ui16 count, then exact ui32 key / packed
             * state-column pairs. Admit the whole source span before it can
             * consume a stack value or select a target action. */
            receipt.conditional_core = 1;
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
            if (cursor >= action->program_word_count) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
            }
            case_count = action->program_words[cursor++];
            if ((size_t)case_count >
                (size_t)(action->program_word_count - cursor) / 4u) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
            }
            cursor += (int)case_count * 4;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_OVERRIDE) {
            uint8_t what = (uint8_t)((command >> 6) & 0x07u);
            uint8_t value = (uint8_t)((command >> 9) & 0x07u);
            int next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 12), 4);

            /* Data.h:2047-2069 and DSA.cpp:1038-1067: only OVERRIDE_P is
             * legal. EX_OVERRIDE reads its optional Override_Pos word before
             * the raw ui16 MAXSTATE extension. */
            receipt.conditional_core = 1;
            if (what != 1u) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL;
            }
            if (value == 7u) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
            if (next_state == -8) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_STORE) {
            uint8_t selector = (uint8_t)((command >> 6) & 0x1fu);
            int next_state =
                csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 11), 5);
            if (selector > 25u) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL;
            }
            if (next_state == -16) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_VARIABLEFETCH ||
                   opcode == CSB_V1_CSBWIN_DSACMD_VARIABLESTORE ||
                   opcode == CSB_V1_CSBWIN_DSACMD_GLOBALFETCH ||
                   opcode == CSB_V1_CSBWIN_DSACMD_GLOBALSTORE) {
            uint8_t index = (uint8_t)((command >> 6) & 0x7fu);
            int relative_state =
                csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 13), 3);
            if (index >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_SOURCE_ILLEGAL;
            }
            receipt.variable_core = 1;
            if (relative_state == -4) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND ||
                   opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND2) {
            uint16_t subcode = (uint16_t)((command >> 6) & 0x7fu);
            int next_state =
                csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 13), 3);
            int requires_runtime_owner = 0;
            if (next_state == -4) {
                if (cursor >= action->program_word_count) {
                    *out_receipt = receipt;
                    return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
                }
                ++cursor;
            }
            if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND2) {
                subcode = (uint16_t)(subcode + 128u);
            }
            if (!csb_v1_csbwin_dsa_core_subcode_supported(
                    subcode, &requires_runtime_owner)) {
                receipt.unsupported_opcode = opcode;
                receipt.unsupported_subcode = subcode;
                *out_receipt = receipt;
                return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
            }
            if (requires_runtime_owner) receipt.requires_runtime_owner = 1;
            if (csb_v1_csbwin_dsa_subcode_is_arithmetic(subcode)) {
                receipt.arithmetic_core = 1;
            }
            if (csb_v1_csbwin_dsa_subcode_is_comparison(subcode)) {
                receipt.comparison_core = 1;
            }
            if (csb_v1_csbwin_dsa_subcode_is_conditional(subcode)) {
                receipt.conditional_core = 1;
            }
            if (csb_v1_csbwin_dsa_subcode_is_timer_family(subcode)) {
                receipt.timer_core = 1;
            }
            if (subcode == 42u) receipt.message_core = 1;
            if (subcode == 47u || subcode == 103u || subcode == 104u || subcode == 115u || subcode == 121u ||
                subcode == 122u) receipt.text_display_core = 1;
            if (subcode == 69u) receipt.sound_core = 1;
            if (subcode == 134u || subcode == 135u || subcode == 118u ||
                subcode == 65u || subcode == 66u) receipt.champion_core = 1;
            if ((subcode >= 51u && subcode <= 56u) || subcode == 74u ||
                subcode == 75u || subcode == 124u || subcode == 125u ||
                subcode == 76u) receipt.object_core = 1;
            if (subcode == 60u || subcode == 64u || subcode == 65u ||
                subcode == 106u || subcode == 116u || subcode == 117u ||
                subcode == 129u) receipt.query_core = 1;
            if (csb_v1_csbwin_dsa_subcode_is_dungeon_mutation(subcode)) {
                receipt.dungeon_mutation_core = 1;
            }
        } else {
            receipt.unsupported_opcode = opcode;
            *out_receipt = receipt;
            return CSB_V1_CSBWIN_DSA_CORE_UNSUPPORTED;
        }
    }
    if (!saw_stack_command) {
        *out_receipt = receipt;
        return CSB_V1_CSBWIN_DSA_CORE_MALFORMED;
    }
    receipt.stack_core = 1;
    receipt.words_consumed = (uint16_t)cursor;
    receipt.valid = 1;
    *out_receipt = receipt;
    return CSB_V1_CSBWIN_DSA_CORE_OK;
}

CSB_V1_CSBWinDSALoadResult csb_v1_csbwin_dsa_execute_load_action(
    const CSB_V1_DSAImportedAction *action,
    const CSB_V1_CSBWinDSALoadContext *context,
    CSB_V1_CSBWinDSALoadExecution *out_execution)
{
    uint16_t command;
    uint8_t selector;
    int next_state;
    int words = 1;
    uint32_t value = 0u;

    if (!action || !context || !out_execution || !action->program_words ||
        action->program_word_count < 1) return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
    command = action->program_words[0];
    if ((command & 0x3fu) != CSB_V1_CSBWIN_DSACMD_LOAD) {
        return CSB_V1_CSBWIN_DSA_LOAD_NOT_LOAD;
    }
    selector = (uint8_t)((command >> 6) & 0x1fu);
    next_state = csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 11), 5);
    if (next_state == -16) {
        if (action->program_word_count < 2) return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
        next_state = (int)(int16_t)action->program_words[words++];
    }
    switch (selector) {
    case CSB_V1_CSBWIN_DSA_LOAD_INTEGER:
    case CSB_V1_CSBWIN_DSA_LOAD_ABS:
        if (action->program_word_count != words + 1) {
            return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
        }
        value = action->program_words[words++];
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_DOLLAR:
        if (action->program_word_count != words) {
            return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
        }
        value = context->master_location;
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_INTEGER32:
        if (action->program_word_count != words + 2) {
            return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
        }
        value = (uint32_t)action->program_words[words] |
            ((uint32_t)action->program_words[words + 1] << 16);
        words += 2;
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_ABS32:
        /* CSBWin DSA.cpp:1074-1189 has no case for Data.h's selector 29:
         * its default takes the source illegal-command path. */
        return CSB_V1_CSBWIN_DSA_LOAD_SOURCE_ILLEGAL;
    default:
        if (selector > 25u || action->program_word_count != words) {
            return CSB_V1_CSBWIN_DSA_LOAD_MALFORMED;
        }
        if (context->parameters && context->parameter_count > (int)selector) {
            value = context->parameters[selector];
        }
        break;
    }
    out_execution->value = value;
    out_execution->next_state = next_state;
    out_execution->words_consumed = (uint16_t)words;
    out_execution->selector = selector;
    return CSB_V1_CSBWIN_DSA_LOAD_OK;
}

CSB_V1_CSBWinDSALoadStoreResult
csb_v1_csbwin_dsa_execute_authenticated_load_store_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSALoadStoreContext *context,
    CSB_V1_CSBWinDSALoadStoreExecution *out_execution)
{
    const CSB_V1_DSAImportedAction *action;
    CSB_V1_DSAImportedAction load_action;
    CSB_V1_CSBWinDSALoadContext load_context;
    CSB_V1_CSBWinDSALoadExecution load_execution;
    uint16_t store_command;
    uint8_t load_selector;
    uint8_t store_selector;
    int store_next_state;
    int cursor;

    if (!state || !context || !out_execution || !context->parameters ||
        context->parameter_count < 0) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED;
    }
    action = csb_v1_chaos_find_imported_action(state, dsa_id, state_index,
                                                 action_ordinal);
    if (!action || !action->program_words || action->program_word_count < 2) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_NOT_AUTHENTICATED;
    }

    /* DSA.cpp:1074-1188 consumes LOAD first. Delimit it before admitting
     * exactly one dependent EX_STORE command. */
    load_selector = (uint8_t)((action->program_words[0] >> 6) & 0x1fu);
    cursor = 1;
    if (((action->program_words[0] >> 11) & 0x1fu) == 0x10u) ++cursor;
    switch (load_selector) {
    case CSB_V1_CSBWIN_DSA_LOAD_INTEGER:
    case CSB_V1_CSBWIN_DSA_LOAD_ABS:
        ++cursor;
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_INTEGER32:
        cursor += 2;
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_DOLLAR:
        break;
    default:
        if (load_selector > 25u) {
            return CSB_V1_CSBWIN_DSA_LOAD_STORE_SOURCE_ILLEGAL;
        }
        break;
    }
    if (cursor >= action->program_word_count) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED;
    }
    load_action = *action;
    load_action.program_word_count = cursor;
    load_context.master_location = context->master_location;
    load_context.parameters = context->parameters;
    load_context.parameter_count = context->parameter_count;
    switch (csb_v1_csbwin_dsa_execute_load_action(&load_action, &load_context,
            &load_execution)) {
    case CSB_V1_CSBWIN_DSA_LOAD_OK:
        break;
    case CSB_V1_CSBWIN_DSA_LOAD_SOURCE_ILLEGAL:
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_SOURCE_ILLEGAL;
    default:
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED;
    }

    store_command = action->program_words[cursor++];
    if ((store_command & 0x3fu) != CSB_V1_CSBWIN_DSACMD_STORE) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_UNSUPPORTED;
    }
    store_selector = (uint8_t)((store_command >> 6) & 0x1fu);
    if (store_selector > 25u) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_SOURCE_ILLEGAL;
    }
    store_next_state = csb_v1_csbwin_dsa_sign_extend(
        (uint16_t)(store_command >> 11), 5);
    if (store_next_state == -16) {
        if (cursor >= action->program_word_count) {
            return CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED;
        }
        store_next_state = (int)(int16_t)action->program_words[cursor++];
    }
    if (cursor != action->program_word_count) {
        return CSB_V1_CSBWIN_DSA_LOAD_STORE_UNSUPPORTED;
    }
    if ((int)store_selector < context->parameter_count) {
        context->parameters[store_selector] = load_execution.value;
    }
    out_execution->value = load_execution.value;
    out_execution->load_next_state = load_execution.next_state;
    out_execution->next_state = store_next_state;
    out_execution->words_consumed = (uint16_t)cursor;
    out_execution->load_selector = load_execution.selector;
    out_execution->store_selector = store_selector;
    return CSB_V1_CSBWIN_DSA_LOAD_STORE_OK;
}

static int csb_v1_csbwin_dsa_stack_push(uint32_t *stack, int *depth,
    uint32_t value)
{
    if (!stack || !depth || *depth < 0 ||
        *depth >= CSB_V1_CSBWIN_DSA_STACK_CAPACITY) return 0;
    stack[(*depth)++] = value;
    return 1;
}

static int csb_v1_csbwin_dsa_stack_pop(uint32_t *stack, int *depth,
    uint32_t *out_value)
{
    if (!stack || !depth || !out_value || *depth < 1) return 0;
    *out_value = stack[--(*depth)];
    return 1;
}

static uint32_t csb_v1_csbwin_dsa_arithmetic_rshift(uint32_t value,
    uint32_t count)
{
    if (count == 0u) return value;
    if ((value & 0x80000000u) == 0u) return value >> count;
    return (value >> count) | (~0u << (32u - count));
}

/* CSBWin executes STKOP_SetSkin against the live SKIN_CACHE.  Firestaff's
 * authenticated action boundary must instead retain those source writes
 * until every following source word has been accepted.  The runtime binds
 * this queue to a candidate EXPOOL profile, so publishing the queue remains
 * atomic with parameter/global commits. */
#define CSB_V1_CSBWIN_DSA_PENDING_SKIN_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_EXCELL_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_GENERATOR_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_MONSTER_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_CELL_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_OBJECT_PROPERTY_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_MISSILE_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_EXPERIENCE_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_SWAPS 100
#define CSB_V1_CSBWIN_DSA_PENDING_POISON_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_CLOUD_REQUESTS 100
#define CSB_V1_CSBWIN_DSA_PENDING_PARTY_TELEPORTS 100
#define CSB_V1_CSBWIN_DSA_PENDING_MONSTER_GROUP_MUTATIONS 100
#define CSB_V1_CSBWIN_DSA_PENDING_OBJECT_MOVES 100
#define CSB_V1_CSBWIN_DSA_PENDING_ACTUATOR_COPIES 100
#define CSB_V1_CSBWIN_DSA_PENDING_SOUND_REQUESTS 100
#define CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS 100
#define CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS 100
#define CSB_V1_CSBWIN_DSA_PENDING_GLOBAL_TEXT_STORES 100
#define CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_WRITES 100
#define CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_PALETTE_WRITES 100
#define CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT 10
#define CSB_V1_CSBWIN_DSA_TEXT_BYTES 1001

typedef struct {
    uint32_t location;
    uint8_t before;
    uint8_t skin;
} CSB_V1_CSBWinDSAPendingSkinWrite;

typedef struct {
    uint32_t location;
    uint32_t flags;
    uint32_t before[8];
} CSB_V1_CSBWinDSAPendingExCellWrite;

typedef struct {
    uint32_t location;
    int delay;
    int expected_delay;
    /* GeneratorDelay@ only finds source DB3 actuator type six.  A stored
     * type-zero fallback must therefore not change a later fetch. */
    int has_generator;
} CSB_V1_CSBWinDSAPendingGeneratorWrite;

typedef struct {
    uint16_t thing;
    uint32_t before[8];
    uint32_t values[8];
    uint8_t write_mask;
} CSB_V1_CSBWinDSAPendingMonsterWrite;

typedef struct {
    uint32_t location;
    uint32_t before[5];
    uint32_t values[5];
    uint8_t write_mask;
    uint8_t false_pit;
} CSB_V1_CSBWinDSAPendingCellWrite;

typedef struct {
    uint16_t thing;
    CSB_V1_CSBWinDSAObjectProperty property;
    uint32_t value;
} CSB_V1_CSBWinDSAPendingObjectPropertyWrite;

typedef struct {
    uint16_t thing;
    uint32_t expected_values[4];
    uint32_t values[4];
} CSB_V1_CSBWinDSAPendingMissileWrite;

typedef struct {
    int32_t character_selector;
    uint32_t values[59];
    uint32_t word_count;
} CSB_V1_CSBWinDSAPendingCharacterWrite;

typedef struct {
    int32_t character_selector;
    int32_t skill_number;
    int32_t experience;
} CSB_V1_CSBWinDSAPendingExperienceWrite;

typedef struct {
    int32_t party_index;
    int32_t fingerprint;
} CSB_V1_CSBWinDSAPendingCharacterSwap;

typedef struct {
    int32_t character_selector;
    int32_t poison_value;
} CSB_V1_CSBWinDSAPendingPoisonWrite;

typedef struct {
    int32_t cloud_type;
    int32_t size;
    uint32_t location;
} CSB_V1_CSBWinDSAPendingCloudRequest;
typedef struct {
    uint32_t destination_location;
} CSB_V1_CSBWinDSAPendingPartyTeleport;

typedef struct {
    uint32_t location;
    uint32_t operand;
    int insert_monster;
} CSB_V1_CSBWinDSAPendingMonsterGroupMutation;

typedef struct {
    int32_t source_type;
    uint32_t source_object_mask;
    uint32_t source_position_mask;
    uint32_t source_location;
    uint32_t source_depth;
    int32_t destination_type;
    uint32_t destination_object_mask;
    uint32_t destination_position_mask;
    uint32_t destination_location;
    uint32_t destination_depth;
} CSB_V1_CSBWinDSAPendingObjectMove;

typedef struct {
    uint16_t thing;
    uint16_t source_thing;
    uint8_t payload[6];
} CSB_V1_CSBWinDSAPendingActuatorCopy;
typedef struct {
    int32_t sound_number;
    int32_t volume;
    int32_t flags;
} CSB_V1_CSBWinDSAPendingSoundRequest;
typedef struct {
    int32_t location;
    int32_t index;
    int32_t color;
} CSB_V1_CSBWinDSAPendingDescriptionRequest;
typedef struct {
    uint32_t location;
    int32_t color;
} CSB_V1_CSBWinDSAPendingSayTextRequest;
typedef struct {
    char text[CSB_V1_CSBWIN_DSA_TEXT_BYTES];
    int32_t color;
} CSB_V1_CSBWinDSAPendingDisplayTextRequest;
typedef struct {
    uint32_t global_index;
    char text[CSB_V1_CSBWIN_DSA_TEXT_BYTES];
} CSB_V1_CSBWinDSAPendingGlobalTextStore;
typedef struct {
    uint32_t overlay_number;
    uint32_t parameters[4];
} CSB_V1_CSBWinDSAPendingOverlayWrite;
typedef struct {
    uint32_t overlay_number;
    uint32_t parameters[3];
} CSB_V1_CSBWinDSAPendingOverlayPaletteWrite;
typedef struct {
    uint32_t delay;
    uint32_t action;
    uint32_t target_location;
    int message_route;
} CSB_V1_CSBWinDSAPendingSwitchAction;
typedef struct {
    uint32_t delay;
    uint32_t message_type;
    uint32_t target_location;
    uint32_t parameter_count;
    uint32_t parameters[29];
} CSB_V1_CSBWinDSAPendingParameterMessage;
typedef struct {
    uint32_t source_location;
    uint32_t destination_location;
} CSB_V1_CSBWinDSAPendingTeleporterCopy;

static int csb_v1_csbwin_dsa_pending_object_property_lookup(
    const CSB_V1_CSBWinDSAPendingObjectPropertyWrite *writes,
    int write_count,
    uint16_t thing,
    CSB_V1_CSBWinDSAObjectProperty property,
    uint32_t *out_value)
{
    int i;

    if (!writes || !out_value || write_count < 0) return 0;
    for (i = write_count - 1; i >= 0; --i) {
        if (writes[i].thing == thing && writes[i].property == property) {
            *out_value = writes[i].value;
            return 1;
        }
    }
    return 0;
}

static int csb_v1_csbwin_dsa_pending_skin_lookup(
    const CSB_V1_CSBWinDSAPendingSkinWrite *writes,
    int write_count,
    uint32_t location,
    uint8_t *out_skin)
{
    int i;

    if (!writes || !out_skin || write_count < 0) return 0;
    for (i = write_count - 1; i >= 0; --i) {
        if (writes[i].location == location) {
            *out_skin = writes[i].skin;
            return 1;
        }
    }
    return 0;
}

static int csb_v1_csbwin_dsa_pending_actuator_copy_lookup(
    const CSB_V1_CSBWinDSAPendingActuatorCopy *copies, int copy_count,
    uint16_t thing, uint8_t out_payload[6])
{
    int i;

    if (!copies || !out_payload || copy_count < 0) return 0;
    for (i = copy_count - 1; i >= 0; --i) {
        if (copies[i].thing == thing) {
            memcpy(out_payload, copies[i].payload, sizeof(copies[i].payload));
            return 1;
        }
    }
    return 0;
}

static int csb_v1_csbwin_dsa_decode_target_operand(
    const CSB_V1_DSAImportedAction *action, int *cursor, uint8_t target_kind,
    int thirty_two, const uint32_t *parameters, int parameter_count,
    uint32_t *stack, int *depth, uint32_t *out_location)
{
    uint32_t low;

    if (!action || !cursor || !parameters || !stack || !depth ||
        !out_location) {
        return 0;
    }
    switch (target_kind) {
    case 0u:
        *out_location = parameter_count > 0 ? parameters[0] : 0u;
        return 1;
    case 1u:
        *out_location = parameter_count > 1 ? parameters[1] : 0u;
        return 1;
    case 2u:
        if (*cursor >= action->program_word_count) return 0;
        low = action->program_words[(*cursor)++];
        if (thirty_two) {
            if (*cursor >= action->program_word_count) return 0;
            low |= (uint32_t)action->program_words[(*cursor)++] << 16;
        }
        *out_location = low;
        return 1;
    case 3u:
        return csb_v1_csbwin_dsa_stack_pop(stack, depth, out_location);
    default:
        return 0;
    }
}

static CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_execute_stack_subcode(uint16_t subcode, uint32_t *stack,
    int *depth, int *forced_state, uint32_t *variables,
    uint8_t *variable_state, uint32_t *parameters, int parameter_count,
    CSB_V1_CSBWinDSAStackContext *context,
    CSB_V1_CSBWinDSAPendingSkinWrite *pending_skin_writes,
    int *pending_skin_write_count,
    CSB_V1_CSBWinDSAPendingExCellWrite *pending_excell_writes,
    int *pending_excell_write_count,
    CSB_V1_CSBWinDSAPendingGeneratorWrite *pending_generator_writes,
    int *pending_generator_write_count,
    CSB_V1_CSBWinDSAPendingMonsterWrite *pending_monster_writes,
    int *pending_monster_write_count,
    CSB_V1_CSBWinDSAPendingCellWrite *pending_cell_writes,
    int *pending_cell_write_count,
    CSB_V1_CSBWinDSAPendingObjectPropertyWrite *pending_object_property_writes,
    int *pending_object_property_write_count, int *staged_saves_disabled,
    uint32_t *staged_random_state,
    CSB_V1_CSBWinDSAPendingMissileWrite *pending_missile_writes,
    int *pending_missile_write_count,
    CSB_V1_CSBWinDSAPendingCharacterWrite *pending_character_writes,
    int *pending_character_write_count,
    CSB_V1_CSBWinDSAPendingExperienceWrite *pending_experience_writes,
    int *pending_experience_write_count,
    CSB_V1_CSBWinDSAPendingCharacterSwap *pending_character_swaps,
    int *pending_character_swap_count,
    CSB_V1_CSBWinDSAPendingPoisonWrite *pending_poison_writes,
    int *pending_poison_write_count,
    CSB_V1_CSBWinDSAPendingCloudRequest *pending_cloud_requests,
    int *pending_cloud_request_count,
    CSB_V1_CSBWinDSAPendingPartyTeleport *pending_party_teleports,
    int *pending_party_teleport_count,
    CSB_V1_CSBWinDSAPendingMonsterGroupMutation *pending_monster_group_mutations,
    int *pending_monster_group_mutation_count,
    CSB_V1_CSBWinDSAPendingObjectMove *pending_object_moves,
    int *pending_object_move_count,
    CSB_V1_CSBWinDSAPendingActuatorCopy *pending_actuator_copies,
    int *pending_actuator_copy_count,
    CSB_V1_CSBWinDSAPendingSoundRequest *pending_sound_requests,
    int *pending_sound_request_count, int *discard_text_requested,
    int *adjust_skills_parameters_requested,
    uint32_t pending_adjust_skills_parameters[5],
    CSB_V1_CSBWinDSAPendingDescriptionRequest *pending_descriptions,
    int *pending_description_count,
    CSB_V1_CSBWinDSAPendingSayTextRequest *pending_say_text_requests,
    int *pending_say_text_request_count,
    CSB_V1_CSBWinDSAPendingDisplayTextRequest *pending_display_text_requests,
    int *pending_display_text_request_count,
    char local_text[CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT]
                   [CSB_V1_CSBWIN_DSA_TEXT_BYTES],
    CSB_V1_CSBWinDSAPendingGlobalTextStore *pending_global_text_stores,
    int *pending_global_text_store_count,
    CSB_V1_CSBWinDSAPendingOverlayWrite *pending_overlay_writes,
    int *pending_overlay_write_count,
    CSB_V1_CSBWinDSAPendingOverlayPaletteWrite *pending_overlay_palette_writes,
    int *pending_overlay_palette_write_count,
    CSB_V1_CSBWinDSAPendingParameterMessage *pending_parameter_messages,
    int *pending_parameter_message_count)
{
    uint32_t v;
    uint32_t w;
    uint32_t count;
    uint32_t result;
    uint32_t aux;
    uint8_t skin;
    int32_t sv;
    int32_t sw;
    int64_t delta_x;
    int64_t delta_y;
    int info_a;
    int info_b;
    uint8_t actuator_payload[6];
    uint8_t destination_payload[6];

    if (!stack || !depth || !forced_state || !variables || !variable_state ||
        !parameters || !context ||
        !pending_skin_writes || !pending_skin_write_count ||
        !pending_excell_writes || !pending_excell_write_count ||
        !pending_generator_writes || !pending_generator_write_count ||
        !pending_monster_writes || !pending_monster_write_count ||
        !pending_cell_writes || !pending_cell_write_count ||
        !pending_object_property_writes ||
        !pending_object_property_write_count ||
        !staged_saves_disabled || !staged_random_state ||
        !pending_missile_writes || !pending_missile_write_count ||
        !pending_character_writes || !pending_character_write_count ||
        !pending_experience_writes || !pending_experience_write_count ||
        !pending_character_swaps || !pending_character_swap_count ||
        !pending_poison_writes || !pending_poison_write_count ||
        !pending_party_teleports || !pending_party_teleport_count ||
        !pending_monster_group_mutations || !pending_monster_group_mutation_count ||
        !pending_object_moves || !pending_object_move_count ||
        !pending_actuator_copies || !pending_actuator_copy_count ||
        !pending_sound_requests || !pending_sound_request_count ||
        !adjust_skills_parameters_requested || !pending_adjust_skills_parameters ||
        !pending_descriptions || !pending_description_count ||
        !pending_say_text_requests || !pending_say_text_request_count ||
        !pending_display_text_requests || !pending_display_text_request_count ||
        !local_text || !pending_global_text_stores ||
        !pending_global_text_store_count ||
        !pending_overlay_writes || !pending_overlay_write_count ||
        !pending_overlay_palette_writes || !pending_overlay_palette_write_count ||
        !pending_parameter_messages || !pending_parameter_message_count ||
        !discard_text_requested ||
        *pending_skin_write_count < 0 || *pending_excell_write_count < 0 ||
        *pending_generator_write_count < 0 ||
        *pending_monster_write_count < 0 ||
        *pending_cell_write_count < 0 ||
        *pending_object_property_write_count < 0 ||
        *pending_missile_write_count < 0 ||
        *pending_character_write_count < 0 ||
        *pending_experience_write_count < 0 ||
        *pending_character_swap_count < 0 ||
        *pending_poison_write_count < 0 ||
        *pending_party_teleport_count < 0 ||
        *pending_monster_group_mutation_count < 0 ||
        *pending_object_move_count < 0 ||
        *pending_actuator_copy_count < 0 ||
        *pending_say_text_request_count < 0 ||
        *pending_display_text_request_count < 0 ||
        *pending_sound_request_count < 0 ||
        *pending_global_text_store_count < 0 ||
        *pending_overlay_write_count < 0 ||
        *pending_overlay_palette_write_count < 0 ||
        *pending_parameter_message_count < 0 ||
        parameter_count < 0 ||
        parameter_count > 26) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    switch (subcode) {
    case 1u: /* STKOP_Plus */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w + v)) goto underflow;
        break;
    case 123u: /* STKOP_ObjectID, CSBWin DSA.cpp:2733-2738. */
        if (!context->most_recent_interesting_object_valid ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                           context->most_recent_interesting_object)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 2u: /* STKOP_Roll */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            count >= (uint32_t)*depth) goto underflow;
        v = stack[*depth - (int)count - 1];
        memmove(&stack[*depth - (int)count - 1],
                &stack[*depth - (int)count], (size_t)count * sizeof(*stack));
        stack[*depth - 1] = v;
        break;
    case 67u: /* STKOP_Random, CSBWin DSA.cpp:2721-2731. */
        if (!context->random_state_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (v == 0u) v = 1u;
        *staged_random_state = *staged_random_state * 0xbb40e62du + 11u;
        w = (*staged_random_state >> 8) & 0x00ffffffu;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                                           (w & 0xffffu) % v)) {
            return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        }
        break;
    case 68u: /* STKOP_CreateCloud, CSBWin DSA.cpp:2740-2786. */
        /* Source pops size, type, then packed Location. Invalid cloud types
         * are a silent CSBWin no-op; valid requests are held until every
         * later bytecode word succeeds. The runtime callback owns the exact
         * DB15/FluxCage/timer allocation and is required up front. */
        if (!context->create_cloud ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        switch ((int32_t)w) {
        case 0: case 3: case 4: case 7: case 40: case 50:
            if (*pending_cloud_request_count >=
                CSB_V1_CSBWIN_DSA_PENDING_CLOUD_REQUESTS) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            pending_cloud_requests[*pending_cloud_request_count].cloud_type =
                (int32_t)w;
            pending_cloud_requests[*pending_cloud_request_count].size =
                (int32_t)v;
            pending_cloud_requests[*pending_cloud_request_count].location = count;
            ++*pending_cloud_request_count;
            break;
        default:
            break;
        }
        break;
    case 62u: /* STKOP_TeleportParty, CSBWin DSA.cpp:4583-4606. */
        /* The original pops exactly one packed LOCATIONREL then constructs a
         * party/object DB1 teleporter with facingMode=0, audible=0. Keep that
         * request staged until the complete DSA action has been accepted. */
        if (!context->teleport_party ||
            *pending_party_teleport_count >=
                CSB_V1_CSBWIN_DSA_PENDING_PARTY_TELEPORTS ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_party_teleports[*pending_party_teleport_count]
            .destination_location = v;
        ++*pending_party_teleport_count;
        break;
    case 95u: /* STKOP_DelMon, CSBWin DSA.cpp:1742-1748. */
    case 96u: /* STKOP_InsMon, CSBWin DSA.cpp:1751-1757. */
        /* Both source operations pop operand then LOCATIONREL.  They are
         * illegal inside a DSA filter and are deferred until the complete
         * authenticated action has passed.  The runtime callback owns DB4,
         * ITEM16 and TIMER mutation; a missing owner is never emulated. */
        if (!context->mutate_monster_group ||
            *pending_monster_group_mutation_count >=
                CSB_V1_CSBWIN_DSA_PENDING_MONSTER_GROUP_MUTATIONS ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_monster_group_mutations[*pending_monster_group_mutation_count]
            .location = w;
        pending_monster_group_mutations[*pending_monster_group_mutation_count]
            .operand = v;
        pending_monster_group_mutations[*pending_monster_group_mutation_count]
            .insert_monster = subcode == 96u;
        ++*pending_monster_group_mutation_count;
        break;
    case 73u: /* STKOP_Move, CSBWin DSA.cpp:4664-4693. */
        /* MoveObject validates the whole source/destination pair before it
         * performs either mutation.  Keep its ten source operands queued in
         * the same order until every later DSA word has succeeded. */
        if (!context->move_object ||
            *pending_object_move_count >= CSB_V1_CSBWIN_DSA_PENDING_OBJECT_MOVES ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_object_moves[*pending_object_move_count].destination_depth = v;
        pending_object_moves[*pending_object_move_count].destination_location = w;
        pending_object_moves[*pending_object_move_count].destination_position_mask = count;
        if ((count & 0x0fu) != 0u &&
            ((count & 0x0fu) & ((count & 0x0fu) - 1u)) != 0u) {
            uint32_t choice_count = 0u;
            uint32_t choice;
            uint32_t bit;

            if (!context->random_state_valid) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            for (bit = 0u; bit < 4u; ++bit) {
                if ((count & (1u << bit)) != 0u) ++choice_count;
            }
            *staged_random_state =
                *staged_random_state * 0xbb40e62du + 11u;
            choice = ((*staged_random_state >> 8) & 0x00ffffffu) % choice_count;
            for (bit = 0u; bit < 4u; ++bit) {
                if ((count & (1u << bit)) == 0u) continue;
                if (choice-- == 0u) {
                    pending_object_moves[*pending_object_move_count]
                        .destination_position_mask = 1u << bit;
                    break;
                }
            }
        }
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) goto underflow;
        pending_object_moves[*pending_object_move_count].destination_object_mask = v;
        pending_object_moves[*pending_object_move_count].destination_type = (int32_t)w;
        pending_object_moves[*pending_object_move_count].source_depth = count;
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) goto underflow;
        pending_object_moves[*pending_object_move_count].source_location = v;
        pending_object_moves[*pending_object_move_count].source_position_mask = w;
        pending_object_moves[*pending_object_move_count].source_object_mask = count;
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        pending_object_moves[*pending_object_move_count].source_type = (int32_t)v;
        ++*pending_object_move_count;
        break;
    case 69u: /* STKOP_Sound, CSBWin DSA.cpp:4612-4624. */
        if (!context->play_sound ||
            *pending_sound_request_count >= CSB_V1_CSBWIN_DSA_PENDING_SOUND_REQUESTS ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_sound_requests[*pending_sound_request_count].sound_number = (int32_t)count;
        pending_sound_requests[*pending_sound_request_count].volume = (int32_t)w;
        pending_sound_requests[*pending_sound_request_count].flags = (int32_t)v;
        ++*pending_sound_request_count;
        break;
    case 101u: /* STKOP_MissileInfoFetch, CSBWin DSA.cpp:2795-2822. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        {
            uint32_t missile_values[4] = {
                UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
            };
            if (v <= UINT16_MAX) {
                if (!context->get_missile_info) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                if (context->get_missile_info(context->dungeon_user,
                                              (uint16_t)v,
                                              missile_values) < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
            }
            if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                                               missile_values[0]) ||
                !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                               missile_values[1]) ||
                !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                               missile_values[2]) ||
                !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                               missile_values[3])) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
        }
        break;
    case 102u: /* STKOP_MissileInfoStore, CSBWin DSA.cpp:2824-2846. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        if (v <= UINT16_MAX) {
            uint32_t missile_values[4] = {
                UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX
            };
            uint32_t source_missile_values[4];
            int pending = -1;
            int source_result;
            for (sv = 0; sv < *pending_missile_write_count; ++sv) {
                if (pending_missile_writes[sv].thing == (uint16_t)v) {
                    pending = sv;
                }
            }
            if (pending >= 0) {
                memcpy(missile_values, pending_missile_writes[pending].values,
                       sizeof(missile_values));
            } else {
                if (!context->get_missile_info) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                source_result = context->get_missile_info(
                    context->dungeon_user, (uint16_t)v, missile_values);
                if (source_result < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                if (source_result == 0) break;
                memcpy(source_missile_values, missile_values,
                       sizeof(source_missile_values));
            }
            if (!csb_v1_csbwin_dsa_stack_pop(stack, depth,
                                              &missile_values[3]) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, depth,
                                              &missile_values[2]) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, depth,
                                              &missile_values[1])) {
                goto underflow;
            }
            if (pending < 0) {
                if ((!context->commit_missile_info &&
                     !context->set_missile_info) ||
                    *pending_missile_write_count >=
                        CSB_V1_CSBWIN_DSA_PENDING_MISSILE_WRITES) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                pending = *pending_missile_write_count;
                pending_missile_writes[pending].thing = (uint16_t)v;
                memcpy(pending_missile_writes[pending].expected_values,
                       source_missile_values,
                       sizeof(source_missile_values));
                ++*pending_missile_write_count;
            }
            memcpy(pending_missile_writes[pending].values, missile_values,
                   sizeof(missile_values));
        }
        break;
    case 3u: /* STKOP_Pick */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            count >= (uint32_t)*depth ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, stack[*depth - 1 - (int)count])) {
            goto underflow;
        }
        break;
    case 4u: /* STKOP_Neg */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, 0u - v)) goto underflow;
        break;
    case 5u: /* STKOP_Equal */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w == v ? 1u : 0u)) goto underflow;
        break;
    case 6u: /* STKOP_Poke */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            count > (uint32_t)*depth) goto underflow;
        stack[*depth - (int)count] = v;
        break;
    case 7u: /* STKOP_Drop */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        break;
    case 11u: /* STKOP_Dup */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 12u: /* STKOP_1Minus */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v - 1u)) goto underflow;
        break;
    case 13u: /* STKOP_Swap */
        if (*depth < 2) goto underflow;
        v = stack[*depth - 2]; stack[*depth - 2] = stack[*depth - 1]; stack[*depth - 1] = v;
        break;
    case 14u: /* STKOP_1Plus */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v + 1u)) goto underflow;
        break;
    case 15u: /* STKOP_Over */
        if (*depth < 2 || !csb_v1_csbwin_dsa_stack_push(stack, depth, stack[*depth - 2])) goto underflow;
        break;
    case 16u: /* STKOP_MinusRoll */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            count >= (uint32_t)*depth) goto underflow;
        v = stack[*depth - 1];
        memmove(&stack[*depth - (int)count], &stack[*depth - (int)count - 1],
                (size_t)count * sizeof(*stack));
        stack[*depth - (int)count] = v;
        break;
    case 17u: /* STKOP_Rot */
        if (*depth < 3) goto underflow;
        v = stack[*depth - 3]; stack[*depth - 3] = stack[*depth - 2];
        stack[*depth - 2] = stack[*depth - 1]; stack[*depth - 1] = v;
        break;
    case 18u: /* STKOP_MinusRot */
        if (*depth < 3) goto underflow;
        v = stack[*depth - 1]; stack[*depth - 1] = stack[*depth - 2];
        stack[*depth - 2] = stack[*depth - 3]; stack[*depth - 3] = v;
        break;
    case 19u: /* STKOP_And */
    case 22u: /* STKOP_Or */
    case 70u: /* STKOP_Xor */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                subcode == 19u ? (w & v) : (subcode == 22u ? (w | v) : (w ^ v)))) goto underflow;
        break;
    case 71u: /* STKOP_MonBlk, CSBWin DSA.cpp:4625-4636. */
        if (!context->monster_move_inhibit_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        context->monster_move_inhibit[0] = (uint8_t)(v & 1u);
        context->monster_move_inhibit[1] = (uint8_t)(v & 2u);
        context->monster_move_inhibit[2] = (uint8_t)(v & 4u);
        context->monster_move_inhibit[3] = (uint8_t)(v & 8u);
        break;
    case 92u: /* STKOP_SetAdjustSkillsParameters, DSA.cpp:3034-3043. */
        if (!context->set_adjust_skills_parameters) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        for (sv = 4; sv >= 0; --sv) {
            if (!csb_v1_csbwin_dsa_stack_pop(
                    stack, depth, &pending_adjust_skills_parameters[sv])) {
                goto underflow;
            }
        }
        *adjust_skills_parameters_requested = 1;
        break;
    case 72u: /* STKOP_Describe, DSA.cpp:4639-4661; Character.cpp:3797-3832. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            goto underflow;
        }
        /* Character.cpp accepts only phrase slots 0..7; invalid slots are a
         * source no-op and therefore require no DB2/phrase owner. */
        if ((int32_t)w < 0 || w > 7u) break;
        if (!context->describe ||
            *pending_description_count >=
                CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_descriptions[*pending_description_count].location = (int32_t)count;
        pending_descriptions[*pending_description_count].index = (int32_t)w;
        pending_descriptions[*pending_description_count].color = (int32_t)v;
        ++*pending_description_count;
        break;
    case 47u: /* STKOP_Say, DSA.cpp:3134-3160. */
        /* CSBWin pops color then the packed location, checks that location,
         * finds its first DB2 text record and feeds the decoded text to the
         * scrolling-text owner.  Location validation and DB2 decoding remain
         * in that owner so an absent dungeon/text bank cannot fabricate text. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (!context->say_text ||
            *pending_say_text_request_count >=
                CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_say_text_requests[*pending_say_text_request_count].location = w;
        pending_say_text_requests[*pending_say_text_request_count].color =
            (int32_t)v;
        ++*pending_say_text_request_count;
        break;
    case 42u: /* STKOP_Message, DSA.cpp:3046-3090. */
        /* Stack order is target, message type, parameter count, delay.  The
         * source queues a TT_ParameterMessage plus exactly the first N DSA
         * parameters in EXPOOL.  Reject an unowned/excessive body rather than
         * padding a fake parameter array. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &aux)) goto underflow;
        if (w > 29u) break;
        if (!context->queue_parameter_message ||
            w > (uint32_t)parameter_count ||
            *pending_parameter_message_count >=
                CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (context->override_state_valid && context->override_p) {
            aux = context->override_position;
            context->override_p = 0;
        }
        pending_parameter_messages[*pending_parameter_message_count].delay = v;
        pending_parameter_messages[*pending_parameter_message_count].parameter_count = w;
        pending_parameter_messages[*pending_parameter_message_count].message_type = count;
        pending_parameter_messages[*pending_parameter_message_count].target_location = aux;
        if (w > 0u) {
            memcpy(pending_parameter_messages[*pending_parameter_message_count]
                       .parameters, parameters, w * sizeof(parameters[0]));
        }
        ++*pending_parameter_message_count;
        break;
    case 104u: /* STKOP_TextSay, DSA.cpp:3240-3254. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        /* DSADBANK::GetText returns an empty string for invalid slots. Keep
         * that source no-op semantics, but never emit it via a fabricated UI. */
        if (w >= CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT) break;
        if (!context->display_text ||
            *pending_display_text_request_count >=
                CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        memcpy(pending_display_text_requests[*pending_display_text_request_count]
                   .text, local_text[w], CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        pending_display_text_requests[*pending_display_text_request_count]
            .text[CSB_V1_CSBWIN_DSA_TEXT_BYTES - 1u] = '\0';
        pending_display_text_requests[*pending_display_text_request_count]
            .color = (int32_t)v;
        ++*pending_display_text_request_count;
        break;
    case 20u: /* STKOP_Shift */
    case 31u: /* STKOP_RShift */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        sv = (int32_t)v;
        count = (sv < 0 ? (0u - (uint32_t)sv) : (uint32_t)sv) & 31u;
        if (subcode == 20u) {
            v = sv >= 0 ? w << count : csb_v1_csbwin_dsa_arithmetic_rshift(w, count);
        } else {
            v = sv >= 0 ? csb_v1_csbwin_dsa_arithmetic_rshift(w, count) : w << count;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 21u: /* STKOP_Comp */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, ~v)) goto underflow;
        break;
    case 24u: /* STKOP_Slash */
    case 25u: /* STKOP_Percent */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        sv = (int32_t)v; sw = (int32_t)w;
        if (sv == 0 || (sw == INT32_MIN && sv == -1)) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        v = (uint32_t)(subcode == 24u ? sw / sv : sw % sv);
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 26u: /* STKOP_SetNewState */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        *forced_state = (int32_t)v;
        break;
    case 36u: /* STKOP_DisableSaves */
        /* CSBWin DSA.cpp:2946-2955 assigns the SaveGame.cpp policy gate.
         * Retain it locally until the complete authenticated action succeeds. */
        if (!context->saves_disabled_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        *staged_saves_disabled = v != 0u;
        break;
    case 78u: /* STKOP_Palette, CSBWin DSA.cpp:2931-2944. */
        /* SetOverlayPalette consumes density, onum, p2, p1. Its owner checks
         * the real selected package surface and palette on commit. */
        if (!context->set_overlay_palette ||
            *pending_overlay_palette_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_PALETTE_WRITES ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &result)) {
            goto underflow;
        }
        pending_overlay_palette_writes[*pending_overlay_palette_write_count]
            .overlay_number = w;
        pending_overlay_palette_writes[*pending_overlay_palette_write_count]
            .parameters[0] = result;
        pending_overlay_palette_writes[*pending_overlay_palette_write_count]
            .parameters[1] = count;
        pending_overlay_palette_writes[*pending_overlay_palette_write_count]
            .parameters[2] = v;
        ++*pending_overlay_palette_write_count;
        break;
    case 49u: /* STKOP_Mastery, CSBWin DSA.cpp:3389-3409. */
        /* Keep DetermineMastery's sleeping, temporary-XP, and possession
         * rules with the live CHARDESC owner.  The core only preserves the
         * source stack order and its invalid-character/skill zero result. */
        if (!context->get_mastery ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = context->get_mastery(context->dungeon_user, count, w, v, &result);
        if (sv < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                                          sv == 0 ? 0u : result)) {
            goto underflow;
        }
        break;
    case 23u: /* STKOP_2Dup, CSBWin DSA.cpp:2430-2437. */
        /* The source reads the top pair without removing it, then pushes the
         * same pair again. Check all capacity up front so a malformed action
         * cannot leave even the private staged stack half-updated. */
        if (*depth < 2) goto underflow;
        if (*depth > CSB_V1_CSBWIN_DSA_STACK_CAPACITY - 2) {
            return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        }
        stack[*depth] = stack[*depth - 2];
        stack[*depth + 1] = stack[*depth - 1];
        *depth += 2;
        break;
    case 27u: /* STKOP_Less */
    case 50u: /* STKOP_ULess */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                subcode == 27u ? ((int32_t)w < (int32_t)v) : (w < v))) goto underflow;
        break;
    case 51u: /* STKOP_GetCurse */
    case 53u: /* STKOP_GetCharges */
    case 55u: /* STKOP_GetBroken */
    case 74u: /* STKOP_GetPoisoned */
    case 124u: /* STKOP_GetSubType */
        /* CSBWin DSA.cpp:3411-3675 reads only DB5/DB6/DB8/DB10 fields.
         * An invalid Thing is a source zero; an unavailable real dungeon is
         * not replaced by a fixture and therefore remains unsupported. */
        if (!context->get_object_property ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            CSB_V1_CSBWinDSAObjectProperty property;
            int resolved;

            switch (subcode) {
            case 51u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE; break;
            case 53u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES; break;
            case 55u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN; break;
            case 74u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED; break;
            default: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE; break;
            }
            w = 0u;
            if (v > UINT16_MAX) {
                resolved = 0;
            } else if (csb_v1_csbwin_dsa_pending_object_property_lookup(
                           pending_object_property_writes,
                           *pending_object_property_write_count,
                           (uint16_t)v, property, &w)) {
                resolved = 1;
            } else {
                resolved = context->get_object_property(
                    context->dungeon_user, (uint16_t)v, property, &w);
            }
            if (resolved < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                                               resolved > 0 ? w : 0u)) {
                goto underflow;
            }
        }
        break;
    case 52u: /* STKOP_SetCurse */
    case 54u: /* STKOP_SetCharges */
    case 56u: /* STKOP_SetBroken */
    case 75u: /* STKOP_SetPoisoned */
    case 125u: /* STKOP_SetSubType */
        /* The source setters are silent no-ops for an invalid DB type. Keep
         * that behavior, but defer a valid raw-record mutation until the
         * complete authenticated action has been consumed. */
        if (!context->get_object_property || !context->set_object_property ||
            !context->normalize_object_property ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            CSB_V1_CSBWinDSAObjectProperty property;
            int resolved;
            int pending = -1;

            switch (subcode) {
            case 52u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE; w = w != 0u; break;
            case 54u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES; break;
            case 56u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN; w = w != 0u; break;
            case 75u: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED; w = w != 0u; break;
            default: property = CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE; break;
            }
            if (v > UINT16_MAX) break;
            resolved = context->get_object_property(
                context->dungeon_user, (uint16_t)v, property, &count);
            if (resolved < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (resolved == 0) break;
            resolved = context->normalize_object_property(
                context->dungeon_user, (uint16_t)v, property, w, &w);
            if (resolved < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (resolved == 0) break;
            for (count = 0u;
                 count < (uint32_t)*pending_object_property_write_count;
                 ++count) {
                if (pending_object_property_writes[count].thing == (uint16_t)v &&
                    pending_object_property_writes[count].property == property) {
                    pending = (int)count;
                }
            }
            if (pending >= 0) {
                pending_object_property_writes[pending].value = w;
            } else if (*pending_object_property_write_count >=
                       CSB_V1_CSBWIN_DSA_PENDING_OBJECT_PROPERTY_WRITES) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            } else {
                pending_object_property_writes[
                    *pending_object_property_write_count].thing = (uint16_t)v;
                pending_object_property_writes[
                    *pending_object_property_write_count].property = property;
                pending_object_property_writes[
                    *pending_object_property_write_count].value = w;
                ++*pending_object_property_write_count;
            }
        }
        break;
    case 28u: /* STKOP_2Drop */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        break;
    case 29u: /* STKOP_NotEqual */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w == v ? 0u : 1u)) goto underflow;
        break;
    case 30u: /* STKOP_2Pick */
        if (*depth < 3 || !csb_v1_csbwin_dsa_stack_push(stack, depth, stack[*depth - 3])) goto underflow;
        break;
    case 32u: /* STKOP_Not */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v == 0u ? 1u : 0u)) goto underflow;
        break;
    case 33u: /* STKOP_FalsePit, CSBWin DSA.cpp:2859-2876. */
        /* The source changes only CELLFLAG bit zero on a real roomPIT. Reuse
         * Cell!'s staged source image so a later action failure cannot write
         * the byte map. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        {
            uint32_t cell_values[5] = { 0u, 0u, 0u, 0u, 0u };
            uint32_t cell_before[5];
            int pending = -1;
            int resolved;
            for (sv = 0; sv < *pending_cell_write_count; ++sv) {
                if (pending_cell_writes[sv].location == w) pending = sv;
            }
            if (pending >= 0) {
                memcpy(cell_values, pending_cell_writes[pending].values,
                       sizeof(cell_values));
            } else if (!context->get_cell_info ||
                       !context->get_cell_info(context->dungeon_user, w,
                                               cell_values)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            memcpy(cell_before, cell_values, sizeof(cell_before));
            if (!context->resolve_cell_store || !context->set_cell_info) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            resolved = context->resolve_cell_store(context->dungeon_user, w,
                                                   3u /* roomPIT */);
            if (resolved < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (resolved == 0) break;
            cell_values[1] = (cell_values[1] & ~1u) | (v != 0u ? 1u : 0u);
            if (pending >= 0) {
                memcpy(pending_cell_writes[pending].values, cell_values,
                       sizeof(cell_values));
                pending_cell_writes[pending].write_mask |= 1u << 1;
                pending_cell_writes[pending].false_pit = 1u;
            } else if (*pending_cell_write_count >=
                       CSB_V1_CSBWIN_DSA_PENDING_CELL_WRITES) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            } else {
                pending_cell_writes[*pending_cell_write_count].location = w;
                memcpy(pending_cell_writes[*pending_cell_write_count].before,
                       cell_before, sizeof(cell_before));
                memcpy(pending_cell_writes[*pending_cell_write_count].values,
                       cell_values, sizeof(cell_values));
                pending_cell_writes[*pending_cell_write_count].write_mask =
                    1u << 1;
                pending_cell_writes[*pending_cell_write_count].false_pit = 1u;
                ++*pending_cell_write_count;
            }
        }
        break;
    case 37u: /* STKOP_Gear */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w * v)) goto underflow;
        break;
    case 38u: /* STKOP_Fetch */
        /* CSBWin DSA.cpp:2473-2480 routes this through DSADBANK::Var.
         * An undefined local therefore yields source zero without changing
         * its definition state; no external variable bank is involved. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        if (variable_state[v] != 1u) variables[v] = 0u;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, variables[v])) {
            goto underflow;
        }
        break;
    case 39u: /* STKOP_Store */
        /* DSA.cpp:2481-2488 pops index before the value and defines the
         * selected DSAVARS cell.  The local bank remains transaction-local. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        variables[v] = w;
        variable_state[v] = 1u;
        break;
    case 60u: /* STKOP_MonsterFetch */
        /* CSBWin DSA.cpp:3992-4048 consumes Thing, DSAVARS destination, and
         * requested count. It initializes the exact eight DB4 result words,
         * then copies at most eight into the caller action's local bank. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (v > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            break;
        }
        {
            uint32_t monster_values[8] = { 0u, UINT32_MAX, 0u, 0u,
                                           0u, 0u, 0u, 0u };
            int pending = -1;
            uint32_t scan;
            if (w <= UINT16_MAX) {
                for (scan = 0u;
                     scan < (uint32_t)*pending_monster_write_count;
                     ++scan) {
                    if (pending_monster_writes[scan].thing == (uint16_t)w) {
                        pending = (int)scan;
                    }
                }
            }
            if (pending >= 0) {
                memcpy(monster_values, pending_monster_writes[pending].values,
                       sizeof(monster_values));
                if (!context->monster_invisible_enabled) {
                    monster_values[6] &= ~1u;
                }
                if (!context->monster_size4_enabled) {
                    monster_values[6] &= ~14u;
                }
            } else if (w <= UINT16_MAX) {
                if (!context->get_monster_info ||
                    !context->get_monster_info(context->dungeon_user,
                                                (uint16_t)w,
                                                monster_values)) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
            }
            if (count > 8u) count = 8u;
            for (sv = 0; sv < (int32_t)count; ++sv) {
                variables[v + (uint32_t)sv] = monster_values[sv];
                variable_state[v + (uint32_t)sv] = 1u;
            }
        }
        break;
    case 63u: /* STKOP_MonsterStore */
        /* CSBWin DSA.cpp:4075-4125 updates selected DB4 hit-point, feature,
         * and alternate-graphic fields only. The real record remains owned
         * by the runtime, so retain its source write until this whole action
         * has passed the authenticated bytecode boundary. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (w > UINT16_MAX) break;
        {
            uint32_t monster_values[8] = { 0u, UINT32_MAX, 0u, 0u,
                                           0u, 0u, 0u, 0u };
            uint32_t monster_before[8];
            uint8_t write_mask = 0u;
            int pending = -1;
            uint32_t i;

            for (i = 0u; i < (uint32_t)*pending_monster_write_count; ++i) {
                if (pending_monster_writes[i].thing == (uint16_t)w) {
                    pending = (int)i;
                }
            }
            if (pending >= 0) {
                memcpy(monster_values, pending_monster_writes[pending].values,
                       sizeof(monster_values));
            } else if (!context->get_monster_info ||
                       !context->get_monster_info(context->dungeon_user,
                                                   (uint16_t)w,
                                                   monster_values)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            if (monster_values[1] == UINT32_MAX) break;
            memcpy(monster_before, monster_values, sizeof(monster_before));
            if (count > 2u) {
                uint32_t hp_count = count - 2u;
                if (hp_count > 4u) hp_count = 4u;
                if (v > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - 3u ||
                    hp_count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v - 2u) {
                    return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
                }
                for (i = 0u; i < hp_count; ++i) {
                    monster_values[2u + i] =
                        (uint32_t)(uint16_t)variables[v + 2u + i];
                    write_mask |= (uint8_t)(1u << (2u + i));
                }
            }
            if (count > 6u) {
                if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
                    v + 6u >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
                    return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
                }
                monster_values[6] = variables[v + 6u] & 0x0fu;
                write_mask |= 1u << 6;
            }
            if (count > 7u) {
                if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
                    v + 7u >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
                    return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
                }
                monster_values[7] = variables[v + 7u] & 0x07u;
                write_mask |= 1u << 7;
            }
            if (write_mask == 0u) break;
            if (pending >= 0) {
                pending_monster_writes[pending].write_mask |= write_mask;
                memcpy(pending_monster_writes[pending].values, monster_values,
                       sizeof(monster_values));
            } else {
                if (!context->set_monster_info ||
                    *pending_monster_write_count >=
                        CSB_V1_CSBWIN_DSA_PENDING_MONSTER_WRITES) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                pending_monster_writes[*pending_monster_write_count].thing =
                    (uint16_t)w;
                memcpy(pending_monster_writes[*pending_monster_write_count].before,
                       monster_before, sizeof(monster_before));
                memcpy(pending_monster_writes[*pending_monster_write_count].values,
                       monster_values, sizeof(monster_values));
                pending_monster_writes[*pending_monster_write_count].write_mask =
                    write_mask;
                ++*pending_monster_write_count;
            }
        }
        break;
    case 64u: /* STKOP_PartyFetch, CSBWin DSA.cpp:4127-4165. */
        /* The original takes (DSAVARS index, requested count), then copies a
         * coherent twelve-word GAMEBLOCK2 snapshot. In particular, do not
         * synthesize PartySleeping from an unrelated Firestaff flag. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        if (v > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            break;
        }
        {
            uint32_t party_values[12];

            if (!context->get_party_info ||
                context->get_party_info(context->dungeon_user, party_values) <= 0) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            if (count > 12u) count = 12u;
            for (w = 0u; w < count; ++w) {
                variables[v + w] = party_values[w];
                variable_state[v + w] = 1u;
            }
        }
        break;
    case 65u: /* STKOP_CharFetch, CSBWin DSA.cpp:4167-4253. */
        /* CHAR@ consumes character selector, DSAVARS index, then count. The
         * source zeroes the result for an oversized destination or an invalid
         * character, but still copies the part that fits below DSAVARS[100].
         * The real owner resolves hand selector four, Wings, PendingDamage,
         * attributes, and skills as one coherent CHARDESC image. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        {
            uint32_t character_values[59] = { 0u };
            int result_available = 0;

            if ((uint32_t)(v + count) <=
                CSB_V1_CSBWIN_DSA_VARIABLE_COUNT && count != 0u &&
                v <= 99u && (int32_t)w >= 0) {
                if (!context->get_character_info) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                result_available = context->get_character_info(
                    context->dungeon_user, (int32_t)w, character_values);
                if (result_available < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                if (result_available == 0) {
                    memset(character_values, 0, sizeof(character_values));
                }
            }
            if (count > 59u) count = 59u;
            for (result = 0u; result < count; ++result) {
                if ((uint32_t)(v + result) > 99u) break;
                variables[v + result] = character_values[result];
                variable_state[v + result] = 1u;
            }
        }
        break;
    case 66u: /* STKOP_CharStore, CSBWin DSA.cpp:4426-4529. */
        /* CHAR! consumes character selector, DSAVARS index, then count. The
         * live CHARDESC owner resolves selector four, the party bounds, and
         * PotentialCharacterOrdinal. It also applies the source food/health/
         * mana/stamina/water/attribute/talent rules to a candidate only. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if ((uint32_t)(v + count) >
                CSB_V1_CSBWIN_DSA_VARIABLE_COUNT || (int32_t)w < 0) {
            break;
        }
        if (count > 59u) count = 59u;
        if (count != 0u) {
            CSB_V1_CSBWinDSAPendingCharacterWrite *write;
            uint32_t i;
            int prepared;

            if (!context->prepare_character_store ||
                !context->set_character_info ||
                *pending_character_write_count >=
                    CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_WRITES) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            write = &pending_character_writes[*pending_character_write_count];
            write->character_selector = (int32_t)w;
            write->word_count = count;
            for (i = 0u; i < count; ++i) {
                write->values[i] = variable_state[v + i] == 1u ?
                    variables[v + i] : 0u;
            }
            prepared = context->prepare_character_store(
                context->dungeon_user, write->character_selector,
                write->values, count);
            if (prepared < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (prepared == 0) break;
            /* DSA.cpp changes Var(index+2) when it clamps the requested
             * health. The candidate gives that exact local result back to a
             * later PARAM! in this same authenticated action. */
            for (i = 0u; i < count; ++i) {
                variables[v + i] = write->values[i];
                variable_state[v + i] = 1u;
            }
            ++*pending_character_write_count;
        }
        break;
    case 57u: /* STKOP_CellFetch */
        /* CSBWin DSA.cpp:3676-3835 zeroes the requested DSAVARS run, then
         * overlays the real CELLFLAG/DB0/DB1 data for a valid LOCATIONREL.
         * The runtime supplies only an original byte-map cell and its first
         * matching record; no inferred room or object layout is accepted. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if ((int32_t)v < 0 || v > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            break;
        }
        for (sv = 0; sv < (int32_t)count; ++sv) {
            variables[v + (uint32_t)sv] = 0u;
            variable_state[v + (uint32_t)sv] = 1u;
        }
        if (count == 0u) break;
        {
            uint32_t cell_values[5] = { 0u, 0u, 0u, 0u, 0u };
            int pending = -1;
            for (sv = 0; sv < *pending_cell_write_count; ++sv) {
                if (pending_cell_writes[sv].location == w) pending = sv;
            }
            if (pending >= 0) {
                memcpy(cell_values, pending_cell_writes[pending].values,
                       sizeof(cell_values));
            } else if (!context->get_cell_info ||
                !context->get_cell_info(context->dungeon_user, w,
                                         cell_values)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            if (count > 5u) count = 5u;
            for (sv = 0; sv < (int32_t)count; ++sv) {
                variables[v + (uint32_t)sv] = cell_values[sv];
            }
        }
        break;
    case 58u: /* STKOP_CellStore */
        /* CSBWin DSA.cpp:3837-3956 changes only an existing source CELLFLAG
         * and (where applicable) its first DB0/DB1 record. Keep a full
         * post-write Cell@ image locally until bytecode acceptance. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if ((int32_t)v < 0 || v > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v || count == 0u) {
            break;
        }
        {
            uint32_t cell_values[5] = { 0u, 0u, 0u, 0u, 0u };
            uint32_t input_values[5] = { 0u, 0u, 0u, 0u, 0u };
            uint32_t cell_before[5];
            uint8_t write_mask = 0u;
            int pending = -1;
            int resolved;
            uint32_t i;

            for (i = 0u; i < (uint32_t)*pending_cell_write_count; ++i) {
                if (pending_cell_writes[i].location == w) pending = (int)i;
            }
            if (pending >= 0) {
                memcpy(cell_values, pending_cell_writes[pending].values,
                       sizeof(cell_values));
            } else if (!context->get_cell_info ||
                       !context->get_cell_info(context->dungeon_user, w,
                                               cell_values)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            memcpy(cell_before, cell_values, sizeof(cell_before));
            for (i = 0u; i < count && i < 5u; ++i) {
                input_values[i] = variables[v + i];
            }
            if (input_values[0] != cell_values[0]) break;
            if (!context->resolve_cell_store || !context->set_cell_info) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            resolved = context->resolve_cell_store(context->dungeon_user, w,
                                                   cell_values[0]);
            if (resolved < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (resolved == 0) break;
            if (count < 2u) break;
            switch (cell_values[0]) {
            case 0u: /* roomSTONE */
                cell_values[1] = input_values[1] & 0x0fu;
                write_mask = 1u << 1;
                break;
            case 1u: /* roomOPEN */
                cell_values[1] = input_values[1] & 0x01u;
                write_mask = 1u << 1;
                break;
            case 2u: /* roomSTAIRS: source no-op */
                break;
            case 3u: /* roomPIT */
                cell_values[1] = input_values[1] & 0x0du;
                write_mask = 1u << 1;
                break;
            case 6u: /* roomFALSEWALL */
                cell_values[1] = input_values[1] & 0x05u;
                write_mask = 1u << 1;
                break;
            case 5u: /* roomTELEPORTER */
                cell_values[1] = input_values[1] & 0x0cu;
                write_mask = 1u << 1;
                if (count >= 3u) {
                    cell_values[2] = input_values[2] & 0x07u;
                    write_mask |= 1u << 2;
                }
                if (count >= 4u) {
                    cell_values[3] = input_values[3] & 0x03u;
                    write_mask |= 1u << 3;
                }
                break;
            case 4u: /* roomDOOR */
                cell_values[1] = (cell_values[1] & 0x01u) |
                    (input_values[1] & 0x1eu);
                write_mask = 1u << 1;
                if (count >= 3u) {
                    if (cell_values[2] == 5u && input_values[2] == 0u) {
                        cell_values[2] = 0u;
                    } else if (cell_values[2] == 4u && input_values[2] == 5u) {
                        cell_values[2] = 5u;
                    }
                    write_mask |= 1u << 2;
                }
                if (count >= 4u) {
                    cell_values[3] = input_values[3] & 0x01u;
                    write_mask |= 1u << 3;
                }
                if (count >= 5u) {
                    cell_values[4] = input_values[4] & 0x0fu;
                    write_mask |= 1u << 4;
                }
                break;
            default:
                break;
            }
            if (write_mask == 0u) break;
            if (pending >= 0) {
                memcpy(pending_cell_writes[pending].values, cell_values,
                       sizeof(cell_values));
                pending_cell_writes[pending].write_mask |= write_mask;
            } else if (*pending_cell_write_count >=
                       CSB_V1_CSBWIN_DSA_PENDING_CELL_WRITES) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            } else {
                pending_cell_writes[*pending_cell_write_count].location = w;
                memcpy(pending_cell_writes[*pending_cell_write_count].before,
                       cell_before, sizeof(cell_before));
                memcpy(pending_cell_writes[*pending_cell_write_count].values,
                       cell_values, sizeof(cell_values));
                pending_cell_writes[*pending_cell_write_count].write_mask =
                    write_mask;
                ++*pending_cell_write_count;
            }
        }
        break;
    case 130u: /* STKOP_DSAInfoFetch, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:2439-2471 returns the four DB3 type-47 fields or
         * four -1 words for every invalid indirect object. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        sv = -1;
        sw = -1;
        info_a = -1;
        info_b = -1;
        if (context->get_dsa_info &&
            context->get_dsa_info(context->wing_user, (uint16_t)v,
                                  &sv, &sw, &info_a, &info_b)) {
            /* callback populated all four source fields */
        } else {
            info_a = -1;
            info_b = -1;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sw) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)info_a) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)info_b)) goto underflow;
        break;
    case 48u: /* STKOP_Loc2AbsCoord */
        /* CSBWin DSA.cpp:3253-3268 constructs LOCATIONREL from the source
         * packed integer, assigns it to LOCATIONABS, then pushes level, X,
         * Y, and position in that order.  The packed layout is the same
         * LOCATIONREL::Integer form already used by master_location. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                            (v >> 10) & 0x3fu) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                            (v >> 5) & 0x1fu) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, v & 0x1fu) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                            (v >> 16) & 0x03u)) {
            goto underflow;
        }
        break;
    case 59u: /* STKOP_GlobalFetch */
        /* CSBWin DSA.cpp:3958-3973 recognizes selector one as the live
         * party LOCATIONREL.  Keep the source packing used by Loc2AbsCoord:
         * facing bits 16..17, level 10..15, X 5..9, and Y 0..4. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        if (v == 1u) {
            if (!context->party_location_valid) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            v = ((uint32_t)(context->party_direction & 3) << 16) |
                ((uint32_t)(context->party_level & 0x3f) << 10) |
                ((uint32_t)(context->party_x & 0x1f) << 5) |
                (uint32_t)(context->party_y & 0x1f);
        } else {
            v = 0u;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 44u: /* STKOP_FetchExCellFlg */
        /* CSBWin DSA.cpp:3270-3297 queries one eight-word
         * EDT_ExtendedCellFlags record and folds the selected Y bit into
         * eight result bits.  EXPOOL remains runtime-owned. */
        if (!context->get_excell_flags ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            uint32_t cell_words[8];
            uint32_t flags = 0u;

            int pending = -1;
            for (count = 0u; count < (uint32_t)*pending_excell_write_count;
                 ++count) {
                if (pending_excell_writes[count].location == v) {
                    pending = (int)count;
                }
            }
            if (pending >= 0) {
                uint32_t staged_flags = pending_excell_writes[pending].flags;
                memset(cell_words, 0, sizeof(cell_words));
                for (count = 0u; count < 8u; ++count) {
                    if ((staged_flags & 1u) != 0u) {
                        cell_words[count] = 1u << (v & 31u);
                    }
                    staged_flags >>= 1;
                }
            } else if (context->get_excell_flags(context->excell_user, v,
                                                  cell_words) <= 0) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            for (count = 0u; count < 8u; ++count) {
                if ((cell_words[count] & (1u << (v & 31u))) != 0u) {
                    flags |= 0x100u;
                }
                flags >>= 1;
            }
            if (!csb_v1_csbwin_dsa_stack_push(stack, depth, flags)) {
                goto underflow;
            }
        }
        break;
    case 45u: /* STKOP_StoreExCellFlg */
        /* CSBWin DSA.cpp:3298-3328 pops the location then the eight-bit
         * flag payload and delegates the DB11 transaction to EXPOOL::Write.
         * The caller-owned runtime candidate performs that full write. */
        if (!context->get_excell_flags || !context->set_excell_flags ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            *pending_excell_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_EXCELL_WRITES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_excell_writes[*pending_excell_write_count].location = v;
        pending_excell_writes[*pending_excell_write_count].flags = w;
        if (context->get_excell_flags(context->excell_user, v,
                                      pending_excell_writes[
                                          *pending_excell_write_count].before) <= 0) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++*pending_excell_write_count;
        break;
    case 46u: /* STKOP_ChPoss */
        /* CSBWin DSA.cpp:3330-3356 normalizes the source character/slot
         * selectors, then reads an existing CHARDESC possession or cursor
         * hand. The runtime owns both source surfaces. */
        if (!context->get_champion_possession ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if ((int32_t)w > 4) w = 0u;
        if ((int32_t)w == 4) {
            w = (uint32_t)(int32_t)context->party_leader_index;
        }
        if (v > 29u) v = 0u;
        sv = -1;
        if (context->get_champion_possession(context->dungeon_user,
                                              (int32_t)w, v, &sv) < 0) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            goto underflow;
        }
        break;
    case 77u: /* STKOP_MonPoss */
        /* CSBWin DSA.cpp:3358-3386 follows DB4.possession2 and the real
         * DBCOMMON next links.  The runtime owns that loaded chain; absent
         * or malformed original dungeon data is not replaced with a list. */
        if (!context->get_monster_possession ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = -1;
        if (w <= UINT16_MAX &&
            context->get_monster_possession(context->dungeon_user,
                                             (uint16_t)w, v, &sv) < 0) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            goto underflow;
        }
        break;
    case 109u: /* STKOP_ThisCell */
    case 107u: /* STKOP_Neighbors */
        /* CSBWin DSA.cpp:2210-2309,4819-4830 calls ExamineCell for the
         * master square or its four cardinal neighbors.  The callback reads
         * only the caller-owned original byte-map and DB chains. */
        if (!context->inspect_cells ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        count = 0u;
        if (context->inspect_cells(context->dungeon_user, w, v,
                                   subcode == 109u ? 4u : 0u,
                                   subcode == 109u ? 4u : 3u,
                                   &count) < 0 ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 8u: /* STKOP_Type */
        /* CSBWin DSA.cpp EX_TYPE:1388-1511 maps one validated original
         * Thing record to dbType * 10000 plus its source type bits. */
        if (!context->get_thing_type ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = -1;
        if (context->get_thing_type(context->dungeon_user, (int32_t)v,
                                    &sv) < 0 ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 114u: /* STKOP_IsCarried */
        /* DSA.cpp EX_IsCarried:1613-1652 searches actual character slots,
         * cursor hand, and recursively linked DB9 contents. */
        if (!context->is_carried ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = -1;
        if (context->is_carried(context->dungeon_user, (int32_t)w,
                                (int32_t)v, &sv) < 0 ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 112u: /* STKOP_MultiplierFetch, CSBWin DSA.cpp:3974-3990. */
        if (!context->get_level_multiplier ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = 1;
        if (context->get_level_multiplier(context->dungeon_user, (int32_t)v,
                                          &sv) < 0 ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 100u: /* STKOP_GeneratorDelayFetch */
        /* CSBWin DSA.cpp:4724-4735 queries the first type-six DB3 generator
         * at this real dungeon location and pushes its disableTime, or -1. */
        if (!context->get_generator_delay ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = -1;
        for (count = 0u; count < (uint32_t)*pending_generator_write_count;
             ++count) {
            if (pending_generator_writes[count].location == v &&
                pending_generator_writes[count].has_generator) {
                sv = pending_generator_writes[count].delay;
            }
        }
        if ((sv == -1 && !context->get_generator_delay(
                              context->dungeon_user, v, &sv)) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, (uint32_t)sv)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 34u: /* STKOP_GeneratorDelayStore */
        /* CSBWin DSA.cpp:2876-2915 resolves a real DB3 chain, changes the
         * first type-six actuator or otherwise the first type-zero actuator,
         * and silently ignores stone/empty cells.  Query first to validate
         * the caller-owned chain, then defer its write until this complete
         * authenticated action is known-good. */
        if (!context->get_generator_delay ||
            (!context->commit_generator_delay &&
             !context->set_generator_delay) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            *pending_generator_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_GENERATOR_WRITES ||
            !context->get_generator_delay(context->dungeon_user, v, &sv)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_generator_writes[*pending_generator_write_count].location = v;
        /* DB3 disableTime is CSBWin's unsigned BITS8_15 field. */
        pending_generator_writes[*pending_generator_write_count].delay =
            (int)(uint8_t)w;
        pending_generator_writes[*pending_generator_write_count].expected_delay = sv;
        pending_generator_writes[*pending_generator_write_count].has_generator =
            sv != -1;
        ++*pending_generator_write_count;
        break;
    case 35u: /* STKOP_Overlay, CSBWin DSA.cpp:2916-2930. */
        /* SelectOverlay pops onum, p4, p3, p2, p1. It is intentionally
         * package-owned: a missing CSBGRAPHICS surface is rejected instead
         * of synthesizing a host overlay. */
        if (!context->set_overlay ||
            *pending_overlay_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_WRITES ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &result) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &aux)) {
            goto underflow;
        }
        pending_overlay_writes[*pending_overlay_write_count].overlay_number = v;
        pending_overlay_writes[*pending_overlay_write_count].parameters[0] = aux;
        pending_overlay_writes[*pending_overlay_write_count].parameters[1] = result;
        pending_overlay_writes[*pending_overlay_write_count].parameters[2] = count;
        pending_overlay_writes[*pending_overlay_write_count].parameters[3] = w;
        ++*pending_overlay_write_count;
        break;
    case 105u: /* STKOP_MonLandD */
        /* CSBWin DSA.cpp:4769-4790 exposes the source movement callback's
         * first three words as LOCATIONREL and the Manhattan distance to its
         * party words. Keep these DSAVARS local until the complete imported
         * action is accepted; normal timer and non-filter routes are denied. */
        if (!context->movement_filter_active || parameter_count < 7 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (v > 98u || v + 1u >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) break;
        variables[v] = 1024u * parameters[0] + 32u * parameters[1] +
            parameters[2];
        variable_state[v] = 1u;
        delta_x = (int64_t)(int32_t)parameters[1] -
            (int64_t)(int32_t)parameters[5];
        delta_y = (int64_t)(int32_t)parameters[2] -
            (int64_t)(int32_t)parameters[6];
        if (delta_x < INT32_MIN || delta_x > INT32_MAX ||
            delta_y < INT32_MIN || delta_y > INT32_MAX) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        if (delta_x < 0) delta_x = -delta_x;
        if (delta_y < 0) delta_y = -delta_y;
        if (delta_x + delta_y > INT32_MAX) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        variables[v + 1u] = (uint32_t)(delta_x + delta_y - 1);
        variable_state[v + 1u] = 1u;
        break;
    case 97u: /* STKOP_TimeFetch */
        /* CSBWin DSA.cpp:2512-2518 pushes the live d.Time value. */
        if (!context->game_time_valid ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                           context->game_time)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 106u: /* STKOP_CountInjury */
        /* CSBWin DSA.cpp:4798-4817 pops injuryMask then champion mask,
         * ignores dead champions, and sums each selected wound bit. */
        if (!context->party_champions_valid ||
            context->party_champion_count < 0 ||
            context->party_champion_count > 4 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        count = 0u;
        for (sv = 0; sv < context->party_champion_count; ++sv, w >>= 1) {
            uint32_t injuries;

            if ((w & 1u) == 0u || context->party_champion_health[sv] <= 0) {
                continue;
            }
            injuries = (uint32_t)context->party_champion_wounds[sv] & v;
            injuries = (injuries & 0x55555555u) +
                ((injuries >> 1) & 0x55555555u);
            injuries = (injuries & 0x33333333u) +
                ((injuries >> 2) & 0x33333333u);
            injuries = (injuries + (injuries >> 4)) & 0x0f0f0f0fu;
            injuries += injuries >> 8;
            count += injuries & 0x3fu;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, count)) goto underflow;
        break;
    case 40u: /* STKOP_ParamFetch */
        /* CSBWin DSA.cpp:2956-2999 copies the first N source parameters to
         * DSAVARS starting at I.  Missing logical parameters become zero and
         * retain DVT_NonParameter so a later PARAM! does not manufacture a
         * caller value.  Firestaff's callback owns only parameter_count
         * words, therefore malformed DSAVARS coordinates fail closed. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (w > 100u) break;
        if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            w > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        for (count = 0u; count < w; ++count) {
            if (count < (uint32_t)parameter_count) {
                variables[v + count] = parameters[count];
                variable_state[v + count] = 1u;
            } else {
                variables[v + count] = 0u;
                variable_state[v + count] = 2u; /* DVT_NonParameter */
            }
        }
        break;
    case 41u: /* STKOP_ParamStore */
        /* DSA.cpp:3000-3044 copies DSAVARS back to the supplied parameter
         * list except cells marked DVT_NonParameter by PARAM@.  The source
         * caller owns the list's actual extent, so never widen it here. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if ((int32_t)w < 0 || w > 100u) break;
        if (w > (uint32_t)parameter_count ||
            v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT ||
            w > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        }
        for (count = 0u; count < w; ++count) {
            if (variable_state[v + count] != 2u) {
                if (variable_state[v + count] != 1u) {
                    variables[v + count] = 0u;
                }
                parameters[count] = variables[v + count];
            }
        }
        break;
    case 139u: /* STKOP_NumParam, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:4949-4955 pushes pDSAparameters[0].  Firestaff's
         * authenticated filter context keeps that source count separately
         * from its A..Z payload, so no world or filter state is invented. */
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                                           (uint32_t)parameter_count)) {
            goto underflow;
        }
        break;
    case 108u: /* STKOP_BitCount */
        /* DSA.cpp:4832-4848 sums the eight four-bit entries in bitCounts.
         * Use the equivalent exact population count here; this stays wholly
         * inside the authenticated source stack and needs no world owner. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) goto underflow;
        v = (v & 0x55555555u) + ((v >> 1) & 0x55555555u);
        v = (v & 0x33333333u) + ((v >> 2) & 0x33333333u);
        v = (v + (v >> 4)) & 0x0f0f0f0fu;
        v += v >> 8;
        v += v >> 16;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v & 0x3fu)) {
            goto underflow;
        }
        break;
    case 110u: /* STKOP_CausePoison, CSBWin DSA.cpp:4348-4362. */
        /* Source stack order is poison value then character index. Poisoning
         * combines DamageCharacter, portrait flags, and a possible TT_75
         * timer, so only the source runtime candidate may decide eligibility
         * and publish the coupled result. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if ((int32_t)v < 0) break;
        if (!context->prepare_cause_poison ||
            !context->commit_cause_poison ||
            *pending_poison_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_POISON_WRITES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            int prepared = context->prepare_cause_poison(
                context->dungeon_user, (int32_t)v, (int32_t)w);

            if (prepared < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (prepared > 0) {
                CSB_V1_CSBWinDSAPendingPoisonWrite *write =
                    &pending_poison_writes[*pending_poison_write_count];

                write->character_selector = (int32_t)v;
                write->poison_value = (int32_t)w;
                ++*pending_poison_write_count;
            }
        }
        break;
    case 115u: /* STKOP_DiscardText, CSBWin DSA.cpp:3161-3167. */
        if (!context->discard_text) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        *discard_text_requested = 1;
        break;
    case 103u: /* STKOP_TextFetch, CSBWin DSA.cpp:3168-3193. */
        /* The source copies a decoded DB2 text string into its fresh local
         * DSA bank. An invalid source record is a no-op; absent real data is
         * not replaced with fabricated text. */
        if (!context->read_text ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (v >= CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT) break;
        memset(local_text[v], 0, CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        sv = context->read_text(context->text_user, w, local_text[v],
                                CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        if (sv < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        local_text[v][CSB_V1_CSBWIN_DSA_TEXT_BYTES - 1u] = '\0';
        break;
    case 121u: /* STKOP_GlobalTextStore, CSBWin DSA.cpp:3194-3203. */
        /* Global text belongs to the CSBWin text owner. Stage a whole slot
         * copy so a later malformed word cannot publish a partial action. */
        if (!context->set_global_text ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (w >= CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT) break;
        if (*pending_global_text_store_count >=
            CSB_V1_CSBWIN_DSA_PENDING_GLOBAL_TEXT_STORES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_global_text_stores[*pending_global_text_store_count]
            .global_index = v;
        memcpy(pending_global_text_stores[*pending_global_text_store_count]
                   .text, local_text[w], CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        ++*pending_global_text_store_count;
        break;
    case 122u: /* STKOP_CharNameFetch, CSBWin DSA.cpp:3204-3238. */
        /* CSBWin checks party then wing CHARDESCs by the low sixteen bits of
         * fingerprint. Missing characters deliberately leave an empty local
         * slot; an unavailable source owner rejects the complete action. */
        if (!context->read_character_name ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        v &= 0xffffu;
        if (v >= CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT) break;
        memset(local_text[v], 0, CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        sv = context->read_character_name(context->text_user, (uint16_t)w,
                                          local_text[v],
                                          CSB_V1_CSBWIN_DSA_TEXT_BYTES);
        if (sv < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        local_text[v][CSB_V1_CSBWIN_DSA_TEXT_BYTES - 1u] = '\0';
        break;
    case 113u: /* STKOP_ExperiencePlus, CSBWin DSA.cpp:4542-4557. */
        /* The original pops experience, skill, then character and delegates
         * all skill hierarchy, XP caps, and LevelUp effects to AddToSkill.
         * Keep those coupled CHARDESC changes in one runtime-owned candidate
         * and publish them only after the full source program is accepted. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) goto underflow;
        if ((int32_t)v <= 0) break;
        if (!context->prepare_experience_plus ||
            !context->add_experience_plus ||
            *pending_experience_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_EXPERIENCE_WRITES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            int prepared = context->prepare_experience_plus(
                context->dungeon_user, (int32_t)count, (int32_t)w,
                (int32_t)v);

            if (prepared < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (prepared > 0) {
                CSB_V1_CSBWinDSAPendingExperienceWrite *write =
                    &pending_experience_writes[*pending_experience_write_count];

                write->character_selector = (int32_t)count;
                write->skill_number = (int32_t)w;
                write->experience = (int32_t)v;
                ++*pending_experience_write_count;
            }
        }
        break;
    case 118u: /* STKOP_SwapCharacter, CSBWin DSA.cpp:4413-4425. */
        /* CSBWin pushes SwapCharacter(index,fingerprint)'s result directly.
         * The roster owner retains delete/add/swap selection, Wings lookup,
         * and the source error codes; only a successful candidate is queued
         * until this complete authenticated action has been consumed. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (!context->prepare_character_swap ||
            !context->commit_character_swap ||
            *pending_character_swap_count >=
                CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_SWAPS) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        {
            uint32_t swap_result = 0u;
            int prepared = context->prepare_character_swap(
                context->dungeon_user, (int32_t)w, (int32_t)v, &swap_result);

            if (prepared < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (!csb_v1_csbwin_dsa_stack_push(stack, depth, swap_result)) {
                goto underflow;
            }
            if (prepared > 0) {
                CSB_V1_CSBWinDSAPendingCharacterSwap *swap =
                    &pending_character_swaps[*pending_character_swap_count];

                swap->party_index = (int32_t)w;
                swap->fingerprint = (int32_t)v;
                ++*pending_character_swap_count;
            }
        }
        break;
    case 129u: /* STKOP_PartyDistance, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:4057-4072 pops LOCATIONREL and compares it with
         * d.partyLevel/X/Y.  The source returns Manhattan distance on the
         * same level, otherwise the negative absolute level distance. */
        if (!context->party_location_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = (int)((v >> 10) & 0x3fu);
        sw = context->party_level - sv;
        if (sw == 0) {
            int dx = context->party_x - (int)((v >> 5) & 0x1fu);
            int dy = context->party_y - (int)(v & 0x1fu);

            if (dx < 0) dx = -dx;
            if (dy < 0) dy = -dy;
            v = (uint32_t)(dx + dy);
        } else {
            if (sw < 0) sw = -sw;
            v = (uint32_t)(-sw);
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 133u: /* STKOP_ThisDSAId, reached via AMPERSAND2 + 128 */
        /* DSA.cpp:4822-4828 pushes exPkt.m_RNslave.ConvertToInteger().
         * The runner admits that raw Thing identity only after the runtime
         * binding has verified the source type-47 actuator. */
        if (!context->dsa_slave_thing_valid ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth,
                                           context->dsa_slave_thing)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 136u: /* STKOP_VSET, reached via AMPERSAND2 + 128 */
        /* DSA.cpp:4850-4887 consumes N, destination, and source from the
         * source stack.  It clamps the two DSAVARS spans to 100 cells, then
         * either fills a constant or memmoves values and definition states. */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (v >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) break;
        if (count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v) {
            count = CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - v;
        }
        if ((int32_t)w < 0) {
            if (w == 0x80000000u) {
                return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            }
            for (sv = 0; sv < (int32_t)count; ++sv) {
                variables[v + (uint32_t)sv] = 0u - w;
                variable_state[v + (uint32_t)sv] = 1u;
            }
        } else if (w > 99u) {
            for (sv = 0; sv < (int32_t)count; ++sv) {
                variables[v + (uint32_t)sv] = w - 100u;
                variable_state[v + (uint32_t)sv] = 1u;
            }
        } else {
            if (count > CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - w) {
                count = CSB_V1_CSBWIN_DSA_VARIABLE_COUNT - w;
            }
            memmove(variables + v, variables + w,
                    (size_t)count * sizeof(*variables));
            memmove(variable_state + v, variable_state + w,
                    (size_t)count * sizeof(*variable_state));
        }
        break;
    case 138u: /* STKOP_ModifyMessage, reached via AMPERSAND2 + 128 */
        /* DSA.cpp:4931-4947 pops SET, CLEAR, then TOGGLE and clamps each
         * source value above MSG_TOGGLE to MSG_TOGGLE. These values belong
         * to one ProcessTimers invocation, never to save data or a host
         * default, so publish them only after this complete action passes. */
        if (!context->timer_type_modifiers_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        context->timer_type_modifiers[0] = (uint8_t)(v > 3u ? 3u : v);
        context->timer_type_modifiers[1] = (uint8_t)(w > 3u ? 3u : w);
        context->timer_type_modifiers[2] = (uint8_t)(count > 3u ? 3u : count);
        break;
    case 137u: /* STKOP_Jitter, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:4898-4929 pops overlay-Y, overlay-X, graphic-Y,
         * then graphic-X. Each changed source value raises jitterChanged;
         * retain the whole render-context mutation until the authenticated
         * action is complete. */
        if (!context->jitter_state_valid ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &result)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (context->y_overlay_jitter != (int32_t)v) {
            context->y_overlay_jitter = (int32_t)v;
            context->jitter_changed = 1;
        }
        if (context->x_overlay_jitter != (int32_t)w) {
            context->x_overlay_jitter = (int32_t)w;
            context->jitter_changed = 1;
        }
        if (context->y_graphic_jitter != (int32_t)count) {
            context->y_graphic_jitter = (int32_t)count;
            context->jitter_changed = 1;
        }
        if (context->x_graphic_jitter != (int32_t)result) {
            context->x_graphic_jitter = (int32_t)result;
            context->jitter_changed = 1;
        }
        break;
    case 117u: /* STKOP_WhoHasTalent */
        /* DSA.cpp:4363-4380 evaluates every source party CHARDESC and
         * returns its bitmask when all requested talent bits are present. */
        if (!context->party_champions_valid ||
            context->party_champion_count < 0 ||
            context->party_champion_count > 4 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        w = 0u;
        for (sv = 0; sv < context->party_champion_count; ++sv) {
            if ((context->party_champion_talents[sv] & v) == v) {
                w |= 1u << sv;
            }
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, w)) goto underflow;
        break;
    case 76u: /* STKOP_Copy, CSBWin DSA.cpp:4696-4721. */
        /* The source pops destination before source, casts both words to
         * Thing indices, accepts DB3 only, and copies exactly sizeof(DB3)-2
         * bytes. Keep the result action-local until every later source word
         * has been accepted; later COPY commands see the staged DB3 image. */
        if (!context->get_actuator_payload ||
            (!context->copy_actuator_payload &&
             !context->set_actuator_payload) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (!csb_v1_csbwin_dsa_pending_actuator_copy_lookup(
                pending_actuator_copies, *pending_actuator_copy_count,
                (uint16_t)w, actuator_payload)) {
            sv = context->get_actuator_payload(context->dungeon_user,
                                                (uint16_t)w,
                                                actuator_payload);
            if (sv < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (sv == 0) break;
        }
        if (!csb_v1_csbwin_dsa_pending_actuator_copy_lookup(
                pending_actuator_copies, *pending_actuator_copy_count,
                (uint16_t)v, destination_payload)) {
            sv = context->get_actuator_payload(context->dungeon_user,
                                                (uint16_t)v,
                                                destination_payload);
            if (sv < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (sv == 0) break;
        }
        if (*pending_actuator_copy_count >=
            CSB_V1_CSBWIN_DSA_PENDING_ACTUATOR_COPIES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_actuator_copies[*pending_actuator_copy_count].thing =
            (uint16_t)v;
        pending_actuator_copies[*pending_actuator_copy_count].source_thing =
            (uint16_t)w;
        memcpy(pending_actuator_copies[*pending_actuator_copy_count].payload,
               actuator_payload, sizeof(actuator_payload));
        ++*pending_actuator_copy_count;
        break;
    case 116u: /* STKOP_WhereIsChar */
        /* CSBWin DSA.cpp:4383-4411 scans the party fingerprints (retaining
         * the final matching index), then asks EXPOOL for EDT_Character|fp. */
        if (!context->party_champions_valid ||
            context->party_champion_count < 0 ||
            context->party_champion_count > 4 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        w = 4u;
        for (sv = 0; sv < context->party_champion_count; ++sv) {
            if (context->party_champion_fingerprints[sv] == (uint16_t)v) {
                w = (uint32_t)sv;
            }
        }
        if (w == 4u) {
            int wing_state;

            if (!context->has_wing_character) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            wing_state = context->has_wing_character(context->wing_user,
                                                      (uint16_t)v);
            if (wing_state < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (wing_state > 0) w = 5u;
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, w)) goto underflow;
        break;
    case 134u: /* STKOP_TalentsFetch, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:4243-4283 maps index four to d.HandChar and
         * returns the selected CHARDESC talents.  High-bit indices name a
         * CHARDESC restored from the runtime-owned EDT_Character wing bank. */
        if (!context->party_champions_valid ||
            context->party_champion_count < 0 ||
            context->party_champion_count > 4 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = (int32_t)v;
        if (sv == 4) sv = context->party_leader_index;
        if (((uint32_t)sv & 0x10000u) != 0u) {
            if (!context->get_wing_talents ||
                context->get_wing_talents(context->wing_user,
                                          (uint16_t)sv, &v) < 0) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
        } else if (sv < 0 || sv >= context->party_champion_count) {
            v = 0u;
        } else {
            v = context->party_champion_talents[sv];
        }
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth, v)) goto underflow;
        break;
    case 135u: /* STKOP_TalentsStore, reached via AMPERSAND2 + 128 */
        /* DSA.cpp:4291-4338 pops character index then talents. Party data is
         * runner-owned; wing writes retain CHARDESC::SaveToWings ownership. */
        if (!context->party_champions_valid ||
            context->party_champion_count < 0 ||
            context->party_champion_count > 4 ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        sv = (int32_t)v;
        if (sv == 4) sv = context->party_leader_index;
        if (((uint32_t)sv & 0x10000u) != 0u) {
            uint32_t old_talents = 0u;
            int wing_result;

            if (!context->get_wing_talents || !context->set_wing_talents) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            wing_result = context->get_wing_talents(context->wing_user,
                                                     (uint16_t)sv,
                                                     &old_talents);
            if (wing_result < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            if (wing_result > 0 && old_talents != w) {
                if (context->set_wing_talents(context->wing_user,
                                              (uint16_t)sv, w) < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                ++context->wing_talents_store_count;
                context->last_wing_talents_fingerprint = (uint16_t)sv;
                context->last_wing_talents_before = old_talents;
                context->last_wing_talents_after = w;
            }
        } else if (sv >= 0 && sv < context->party_champion_count) {
            context->party_champion_talents[sv] = w;
        }
        break;
    case 131u: /* STKOP_GetSkin, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:3107-3120 pops the packed five/five/six-bit
         * location, reads the loaded SKIN_CACHE, then pushes its byte. */
        if (!context->get_skin ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            (!csb_v1_csbwin_dsa_pending_skin_lookup(
                 pending_skin_writes, *pending_skin_write_count, v, &skin) &&
             !context->get_skin(context->skin_user, v, &skin)) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, skin)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        break;
    case 132u: /* STKOP_SetSkin, reached via AMPERSAND2 + 128 */
        /* CSBWin DSA.cpp:3122-3135 pops location first, then the skin byte.
         * Retain the write locally until the complete authenticated action
         * has consumed every later word. GETSKIN below observes this exact
         * action-local state, as it would after CSBWin's immediate write. */
        if (!context->get_skin || !context->set_skin ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            *pending_skin_write_count >=
                CSB_V1_CSBWIN_DSA_PENDING_SKIN_WRITES) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_skin_writes[*pending_skin_write_count].location = v;
        if (!csb_v1_csbwin_dsa_pending_skin_lookup(
                pending_skin_writes, *pending_skin_write_count, v, &skin) &&
            !context->get_skin(context->skin_user, v, &skin)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        pending_skin_writes[*pending_skin_write_count].before = skin;
        pending_skin_writes[*pending_skin_write_count].skin = (uint8_t)w;
        ++*pending_skin_write_count;
        break;
    default:
        return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
    }
    return CSB_V1_CSBWIN_DSA_STACK_OK;

underflow:
    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
}

CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_execute_authenticated_stack_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSAStackContext *context,
    CSB_V1_CSBWinDSAStackExecution *out_execution)
{
    const CSB_V1_DSAImportedAction *action;
    uint32_t stack[CSB_V1_CSBWIN_DSA_STACK_CAPACITY];
    uint32_t parameters[26] = { 0u };
    uint32_t variables[CSB_V1_CSBWIN_DSA_VARIABLE_COUNT] = { 0u };
    uint32_t global_variables[CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY] = { 0u };
    uint8_t variable_state[CSB_V1_CSBWIN_DSA_VARIABLE_COUNT] = { 0u };
    CSB_V1_CSBWinDSAPendingSkinWrite
        pending_skin_writes[CSB_V1_CSBWIN_DSA_PENDING_SKIN_WRITES];
    CSB_V1_CSBWinDSAPendingExCellWrite
        pending_excell_writes[CSB_V1_CSBWIN_DSA_PENDING_EXCELL_WRITES];
    CSB_V1_CSBWinDSAPendingGeneratorWrite
        pending_generator_writes[CSB_V1_CSBWIN_DSA_PENDING_GENERATOR_WRITES];
    CSB_V1_CSBWinDSAPendingMonsterWrite
        pending_monster_writes[CSB_V1_CSBWIN_DSA_PENDING_MONSTER_WRITES];
    CSB_V1_CSBWinDSAPendingCellWrite
        pending_cell_writes[CSB_V1_CSBWIN_DSA_PENDING_CELL_WRITES];
    CSB_V1_CSBWinDSAPendingObjectPropertyWrite
        pending_object_property_writes[
            CSB_V1_CSBWIN_DSA_PENDING_OBJECT_PROPERTY_WRITES];
    CSB_V1_CSBWinDSAPendingMissileWrite
        pending_missile_writes[CSB_V1_CSBWIN_DSA_PENDING_MISSILE_WRITES];
    CSB_V1_CSBWinDSAPendingCharacterWrite
        pending_character_writes[CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_WRITES];
    CSB_V1_CSBWinDSAPendingExperienceWrite
        pending_experience_writes[CSB_V1_CSBWIN_DSA_PENDING_EXPERIENCE_WRITES];
    CSB_V1_CSBWinDSAPendingCharacterSwap
        pending_character_swaps[CSB_V1_CSBWIN_DSA_PENDING_CHARACTER_SWAPS];
    CSB_V1_CSBWinDSAPendingPoisonWrite
        pending_poison_writes[CSB_V1_CSBWIN_DSA_PENDING_POISON_WRITES];
    CSB_V1_CSBWinDSAPendingCloudRequest
        pending_cloud_requests[CSB_V1_CSBWIN_DSA_PENDING_CLOUD_REQUESTS];
    CSB_V1_CSBWinDSAPendingPartyTeleport
        pending_party_teleports[CSB_V1_CSBWIN_DSA_PENDING_PARTY_TELEPORTS];
    CSB_V1_CSBWinDSAPendingMonsterGroupMutation pending_monster_group_mutations[
        CSB_V1_CSBWIN_DSA_PENDING_MONSTER_GROUP_MUTATIONS];
    CSB_V1_CSBWinDSAPendingObjectMove
        pending_object_moves[CSB_V1_CSBWIN_DSA_PENDING_OBJECT_MOVES];
    CSB_V1_CSBWinDSAPendingActuatorCopy
        pending_actuator_copies[CSB_V1_CSBWIN_DSA_PENDING_ACTUATOR_COPIES];
    CSB_V1_CSBWinDSAPendingSoundRequest
        pending_sound_requests[CSB_V1_CSBWIN_DSA_PENDING_SOUND_REQUESTS];
    CSB_V1_CSBWinDSAPendingDescriptionRequest pending_descriptions[
        CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS];
    CSB_V1_CSBWinDSAPendingSayTextRequest pending_say_text_requests[
        CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS];
    CSB_V1_CSBWinDSAPendingDisplayTextRequest pending_display_text_requests[
        CSB_V1_CSBWIN_DSA_PENDING_DESCRIPTION_REQUESTS];
    CSB_V1_CSBWinDSAPendingGlobalTextStore pending_global_text_stores[
        CSB_V1_CSBWIN_DSA_PENDING_GLOBAL_TEXT_STORES];
    CSB_V1_CSBWinDSAPendingOverlayWrite pending_overlay_writes[
        CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_WRITES];
    CSB_V1_CSBWinDSAPendingOverlayPaletteWrite pending_overlay_palette_writes[
        CSB_V1_CSBWIN_DSA_PENDING_OVERLAY_PALETTE_WRITES];
    CSB_V1_CSBWinDSAPendingSwitchAction pending_switch_actions[
        CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS];
    CSB_V1_CSBWinDSAPendingTeleporterCopy pending_teleporter_copies[
        CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS];
    CSB_V1_CSBWinDSAPendingParameterMessage pending_parameter_messages[
        CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS];
    CSB_V1_CSBWinDSAStackContext context_candidate;
    CSB_V1_CSBWinDSAStackExecution candidate;
    int cursor = 0;
    int depth = 0;
    int pending_skin_write_count = 0;
    int pending_excell_write_count = 0;
    int pending_generator_write_count = 0;
    int pending_monster_write_count = 0;
    int pending_cell_write_count = 0;
    int pending_object_property_write_count = 0;
    int pending_missile_write_count = 0;
    int pending_character_write_count = 0;
    int pending_experience_write_count = 0;
    int pending_character_swap_count = 0;
    int pending_poison_write_count = 0;
    int pending_cloud_request_count = 0;
    int pending_party_teleport_count = 0;
    int pending_monster_group_mutation_count = 0;
    int pending_object_move_count = 0;
    int pending_actuator_copy_count = 0;
    int pending_sound_request_count = 0;
    int discard_text_requested = 0;
    int adjust_skills_parameters_requested = 0;
    int pending_description_count = 0;
    int pending_say_text_request_count = 0;
    int pending_display_text_request_count = 0;
    int pending_global_text_store_count = 0;
    int pending_overlay_write_count = 0;
    int pending_overlay_palette_write_count = 0;
    int pending_switch_action_count = 0;
    int pending_teleporter_copy_count = 0;
    int pending_parameter_message_count = 0;
    int override_requested = 0;
    int dynamic_jump_requested = 0;
    uint32_t pending_adjust_skills_parameters[5] = { 0u, 0u, 0u, 0u, 0u };
    int staged_saves_disabled;
    uint32_t staged_random_state;
    char local_text[CSB_V1_CSBWIN_DSA_TEXT_SLOT_COUNT]
                   [CSB_V1_CSBWIN_DSA_TEXT_BYTES];
    int i;

    if (!state || !context || !out_execution ||
        (context->parameter_count > 0 && !context->parameters) ||
        context->parameter_count < 0 || context->global_variable_count < 0 ||
        context->global_variable_count > CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY ||
        (context->global_variable_count > 0 && !context->global_variables)) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    if (context->timer_type_modifiers_valid &&
        (context->timer_type_modifiers[0] > 3u ||
         context->timer_type_modifiers[1] > 3u ||
         context->timer_type_modifiers[2] > 3u)) {
        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    }
    action = csb_v1_chaos_find_imported_action(state, dsa_id, state_index,
                                                 action_ordinal);
    if (!action || !action->program_words || action->program_word_count < 1) {
        return CSB_V1_CSBWIN_DSA_STACK_NOT_AUTHENTICATED;
    }
    for (i = 0; i < 26 && i < context->parameter_count; ++i) parameters[i] = context->parameters[i];
    for (i = 0; i < context->global_variable_count; ++i) {
        global_variables[i] = context->global_variables[i];
    }
    context_candidate = *context;
    memset(&candidate, 0, sizeof(candidate));
    memset(pending_cell_writes, 0, sizeof(pending_cell_writes));
    memset(local_text, 0, sizeof(local_text));
    candidate.forced_state = -1;
    staged_saves_disabled = context->saves_disabled ? 1 : 0;
    staged_random_state = context->random_state;
    while (cursor < action->program_word_count) {
        uint16_t command = action->program_words[cursor++];
        uint8_t opcode = (uint8_t)(command & 0x3fu);
        int next_state;
        CSB_V1_CSBWinDSAStackResult rc;
        ++candidate.command_count;
        if (opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE ||
            opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE32 ||
            opcode == CSB_V1_CSBWIN_DSACMD_DESSAGE32) {
            uint8_t message_type = (uint8_t)((command >> 6) & 0x03u);
            uint8_t delay_kind = (uint8_t)((command >> 8) & 0x03u);
            uint8_t target_kind = (uint8_t)((command >> 10) & 0x03u);
            uint32_t delay = 0u;
            uint32_t target_location = 0u;
            uint32_t target_low;
            uint32_t action_type;

            /* CSBWin DSA.cpp:635-725 / Data.h DSAmessageCmd. MESSAGE and
             * MESSAGE32 schedule QueueDSASwitchAction with MorD='M';
             * DESSAGE32 uses the same word grammar but forces TT_DESSAGE. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)((command >> 12) & 0x0fu), 4);
            if (next_state == -8) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
            if (delay_kind == 1u) {
                delay = context->parameter_count > 0 ? parameters[0] : 0u;
            } else if (delay_kind == 2u) {
                delay = context->parameter_count > 1 ? parameters[1] : 0u;
            } else if (delay_kind == 3u) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                delay = action->program_words[cursor++];
            }
            if (target_kind == 0u) {
                target_location =
                    context->parameter_count > 0 ? parameters[0] : 0u;
            } else if (target_kind == 1u) {
                target_location =
                    context->parameter_count > 1 ? parameters[1] : 0u;
            } else if (target_kind == 2u) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                target_low = action->program_words[cursor++];
                if (opcode == CSB_V1_CSBWIN_DSACMD_MESSAGE32 ||
                    opcode == CSB_V1_CSBWIN_DSACMD_DESSAGE32) {
                    if (cursor >= action->program_word_count) {
                        return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                    }
                    target_low |= (uint32_t)action->program_words[cursor++] << 16;
                }
                target_location = target_low;
            } else {
                if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth,
                                                 &target_location)) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
            }
            if (message_type == 0u) {
                /* MSG_NULL consumes the same operands but exits before
                 * QueueDSASwitchAction in the source. */
            } else {
                if (!context->queue_switch_action ||
                    pending_switch_action_count >=
                        CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                action_type = message_type - 1u;
                pending_switch_actions[pending_switch_action_count].delay = delay;
                pending_switch_actions[pending_switch_action_count].action =
                    action_type;
                pending_switch_actions[pending_switch_action_count]
                    .target_location = target_location;
                pending_switch_actions[pending_switch_action_count].message_route =
                    opcode == CSB_V1_CSBWIN_DSACMD_DESSAGE32 ? 'D' : 'M';
                ++pending_switch_action_count;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER ||
                   opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32) {
            uint8_t source_kind = (uint8_t)((command >> 6) & 0x03u);
            uint8_t destination_kind = (uint8_t)((command >> 8) & 0x03u);
            uint32_t source_location = 0u;
            uint32_t destination_location = 0u;
            int thirty_two =
                opcode == CSB_V1_CSBWIN_DSACMD_COPYTELEPORTER32;

            /* CSBWin DSA.cpp:731-763 / Data.h DSAcopyTeleporterCmd. The
             * source copies the first DB1 teleporter and its CELLFLAG from
             * source square to an existing destination teleporter square. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)((command >> 10) & 0x3fu), 6);
            if (next_state == -32) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
            if (!csb_v1_csbwin_dsa_decode_target_operand(
                    action, &cursor, source_kind, thirty_two, parameters,
                    context->parameter_count, stack, &depth,
                    &source_location) ||
                !csb_v1_csbwin_dsa_decode_target_operand(
                    action, &cursor, destination_kind, thirty_two, parameters,
                    context->parameter_count, stack, &depth,
                    &destination_location)) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
            if (!context->copy_teleporter || pending_teleporter_copy_count >=
                CSB_V1_CSBWIN_DSA_PENDING_SWITCH_ACTIONS) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            pending_teleporter_copies[pending_teleporter_copy_count]
                .source_location = source_location;
            pending_teleporter_copies[pending_teleporter_copy_count]
                .destination_location = destination_location;
            ++pending_teleporter_copy_count;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_LOAD) {
            uint8_t selector = (uint8_t)((command >> 6) & 0x1fu);
            next_state = csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 11), 5);
            if (next_state == -16) {
                if (cursor >= action->program_word_count) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                next_state = (int)(int16_t)action->program_words[cursor++];
            }
            if (selector == CSB_V1_CSBWIN_DSA_LOAD_ABS32) return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            if (selector == CSB_V1_CSBWIN_DSA_LOAD_INTEGER || selector == CSB_V1_CSBWIN_DSA_LOAD_ABS) {
                if (cursor >= action->program_word_count ||
                    !csb_v1_csbwin_dsa_stack_push(stack, &depth, action->program_words[cursor++])) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            } else if (selector == CSB_V1_CSBWIN_DSA_LOAD_INTEGER32) {
                if (cursor + 1 >= action->program_word_count || !csb_v1_csbwin_dsa_stack_push(stack, &depth,
                    (uint32_t)action->program_words[cursor] | ((uint32_t)action->program_words[cursor + 1] << 16))) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                cursor += 2;
            } else if (selector == CSB_V1_CSBWIN_DSA_LOAD_DOLLAR) {
                if (!csb_v1_csbwin_dsa_stack_push(stack, &depth, context->master_location)) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            } else if (selector <= 25u) {
                if (!csb_v1_csbwin_dsa_stack_push(stack, &depth, parameters[selector])) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            } else return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_NOOP) {
            /* CSBWin DSA.cpp:574-591 / Data.h DSAnoopCmd: NOOP carries
             * only Execute's next-state value. Its MAXSTATE extension is a
             * raw source word, matching the other DSA command extensions. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_FETCH) {
            uint32_t depth_value;
            uint32_t location_value;
            uint32_t position_mask;
            uint32_t object_mask;
            int32_t fetched_thing;

            /* CSBWin DSA.cpp:4986-5040 EX_FETCH pops object mask, position
             * mask, LOCATIONREL and depth, then searches only the loaded
             * square's original Thing chain. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
            if (!context->fetch_object ||
                !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &object_mask) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &position_mask) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &location_value) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &depth_value) ||
                !context->fetch_object(context->dungeon_user, depth_value,
                                        location_value, position_mask,
                                        object_mask, &fetched_thing) ||
                !csb_v1_csbwin_dsa_stack_push(stack, &depth,
                                                (uint32_t)fetched_thing)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_EQUAL) {
            uint32_t first;
            uint32_t second;

            /* DSA.cpp:1491-1515 pops two source stack words, pushes the
             * boolean result, then follows the same DSAequalCmd state form
             * as NOOP. No filter or world surface is involved. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
            if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth, &first) ||
                !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &second) ||
                !csb_v1_csbwin_dsa_stack_push(stack, &depth,
                                                first == second ? 1u : 0u)) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_QUESTION) {
            int condition;
            uint32_t condition_word;
            uint8_t if_command = (uint8_t)((command >> 11) & 0x03u);
            uint8_t else_command = (uint8_t)((command >> 14) & 0x03u);
            uint8_t if_column = (uint8_t)((command >> 10) & 0x01u);
            uint8_t else_column = (uint8_t)((command >> 13) & 0x01u);
            uint8_t selected_command;
            uint32_t if_state = 0u;
            uint32_t else_state = 0u;
            uint32_t if_target_column = 0u;
            uint32_t else_target_column = 0u;
            uint32_t selected_state;
            uint32_t selected_column;

            /* CSBWin DSA.cpp:850-978 / Data.h DSAquestionCmd. QUESTION
             * pops its condition, consumes every declared branch operand in
             * source order, and then either performs no transfer or enters the
             * selected authenticated JUMP/GOSUB action chain. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)((command >> 6) & 0x0fu), 4);
            if (next_state == -2) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                next_state = (int)action->program_words[cursor++];
            }
            if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth,
                                               &condition_word)) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
            condition = condition_word != 0u;
            if (if_command != 0u) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                if_state = action->program_words[cursor++];
            }
            if (if_column) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                if_target_column = action->program_words[cursor++];
            }
            if (else_command != 0u) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                else_state = action->program_words[cursor++];
            }
            if (else_column) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                else_target_column = action->program_words[cursor++];
            }
            selected_command = condition ? if_command : else_command;
            if (selected_command == 1u || selected_command == 2u) {
                const CSB_V1_DSAImportedAction *transfer_action;
                uint8_t required_opcode =
                    selected_command == 1u ? CSB_V1_CSBWIN_DSACMD_JUMP :
                                             CSB_V1_CSBWIN_DSACMD_GOSUB;

                selected_state = condition ? if_state : else_state;
                selected_column = condition ? if_target_column :
                                              else_target_column;
                transfer_action = csb_v1_chaos_find_imported_action_column(
                    state, dsa_id, selected_state, selected_column);
                if (!transfer_action || !transfer_action->program_words ||
                    transfer_action->program_word_count < 1 ||
                    (transfer_action->program_words[0] & 0x3fu) !=
                        required_opcode) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                if (csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
                        state, dsa_id, selected_state, selected_column, 0,
                        &candidate.transfer) !=
                        CSB_V1_CSBWIN_DSA_EXECUTE_OK ||
                    candidate.transfer.final_state < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                candidate.transfer_executed = 1;
                /* EX_QUESTION sets m_nextState before it calls a selected
                 * GOSUB/JUMP. The child Execute return is not the conditional
                 * result; retain the source question continuation. */
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_CASE) {
            uint32_t case_value;
            uint16_t case_count;
            size_t base = 0u;
            size_t remaining;
            int selected = 0;
            uint32_t selected_state = 0u;
            uint32_t selected_column = 0u;

            /* CSBWin DSA.cpp EX_CASE searches ui16_16 pairs: a ui32 key,
             * then a packed state (bits 8..23) and column (bits 0..7).
             * The selected target must be an imported transfer action; every
             * other target form stays outside this bounded interpreter. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 6), 10);
            if (next_state == -512) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                /* EX_CASE assigns its ui16 MAXSTATE extension directly to
                 * i32, unlike EX_LOAD and EX_VARIABLEFETCH. */
                next_state = (int)action->program_words[cursor++];
            }
            if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth, &case_value) ||
                cursor >= action->program_word_count) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
            case_count = action->program_words[cursor++];
            if ((size_t)case_count >
                (size_t)(action->program_word_count - cursor) / 4u) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
            remaining = case_count;
            while (remaining > 0u) {
                size_t middle = remaining / 2u;
                size_t entry_word = (size_t)cursor + (base + middle) * 4u;
                uint32_t key = (uint32_t)action->program_words[entry_word] |
                    ((uint32_t)action->program_words[entry_word + 1u] << 16);

                if (key == case_value) {
                    uint32_t target =
                        (uint32_t)action->program_words[entry_word + 2u] |
                        ((uint32_t)action->program_words[entry_word + 3u] << 16);
                    selected_state = (target >> 8) & 0xffffu;
                    selected_column = target & 0xffu;
                    selected = 1;
                    break;
                }
                if (key > case_value) {
                    remaining = middle;
                } else {
                    base += middle + 1u;
                    remaining -= middle + 1u;
                }
            }
            cursor += (int)case_count * 4;
            if (selected) {
                const CSB_V1_DSAImportedAction *transfer_action =
                    csb_v1_chaos_find_imported_action_column(
                        state, dsa_id, selected_state, selected_column);
                uint8_t target_opcode;

                if (!transfer_action || !transfer_action->program_words ||
                    transfer_action->program_word_count < 1) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                target_opcode = (uint8_t)(transfer_action->program_words[0] &
                                          0x3fu);
                if ((target_opcode != CSB_V1_CSBWIN_DSACMD_JUMP &&
                     target_opcode != CSB_V1_CSBWIN_DSACMD_GOSUB) ||
                    csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
                        state, dsa_id, selected_state, selected_column, 0,
                        &candidate.transfer) != CSB_V1_CSBWIN_DSA_EXECUTE_OK ||
                    candidate.transfer.final_state < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                candidate.transfer_executed = 1;
                /* EX_CASE likewise owns its NextState before a matching
                 * JUMP changes Execute's current state/column. */
            }
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_OVERRIDE) {
            uint8_t what = (uint8_t)((command >> 6) & 0x07u);
            uint8_t value = (uint8_t)((command >> 9) & 0x07u);

            /* CSBWin DSA.cpp EX_OVERRIDE consumes the position extension
             * before MAXSTATE.  It is a ProcessTimers-scoped global write,
             * so an authenticated program still needs its live owner. */
            next_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 12), 4);
            if (what != 1u) return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            if (!context->override_state_valid || !context->set_override_p) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            if (value == 7u) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                context_candidate.override_position = action->program_words[cursor++];
            } else {
                context_candidate.override_position = value;
            }
            if (next_state == -8) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                /* EX_OVERRIDE assigns this ui16 directly to i32. */
                next_state = (int)action->program_words[cursor++];
            }
            context_candidate.override_p = 1;
            override_requested = 1;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_STORE) {
            uint8_t selector = (uint8_t)((command >> 6) & 0x1fu);
            if (selector > 25u) return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            next_state = csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 11), 5);
            if (next_state == -16) {
                if (cursor >= action->program_word_count) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                /* EX_STORE, unlike EX_LOAD, assigns this source word without
                 * an i16 cast (CSBWin DSA.cpp:1371-1380). */
                next_state = (int)action->program_words[cursor++];
            }
            if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth, &parameters[selector])) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_VARIABLEFETCH ||
                   opcode == CSB_V1_CSBWIN_DSACMD_VARIABLESTORE) {
            uint8_t index = (uint8_t)((command >> 6) & 0x7fu);
            int relative_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 13), 3);
            if (index >= CSB_V1_CSBWIN_DSA_VARIABLE_COUNT) {
                return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            }
            if (relative_state == -4) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                /* ReDMCSB: CSBWin DSA.cpp EX_VARIABLEFETCH/STORE reads
                 * the MAXSTATE extension as a signed i16. */
                relative_state = (int)(int16_t)action->program_words[cursor++];
            }
            if (opcode == CSB_V1_CSBWIN_DSACMD_VARIABLEFETCH) {
                /* CSBWin DSADBANK::NoValue writes zero before returning an
                 * undefined local. The definition flag intentionally stays
                 * unset, because a later store is the first definition. */
                if (variable_state[index] != 1u) variables[index] = 0u;
                if (!csb_v1_csbwin_dsa_stack_push(stack, &depth,
                                                    variables[index])) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
            } else {
                if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth,
                                                   &variables[index])) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                variable_state[index] = 1u;
            }
            next_state = relative_state;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_GLOBALFETCH ||
                   opcode == CSB_V1_CSBWIN_DSACMD_GLOBALSTORE) {
            uint8_t index = (uint8_t)((command >> 6) & 0x7fu);
            int relative_state = csb_v1_csbwin_dsa_sign_extend(
                (uint16_t)(command >> 13), 3);

            /* CSBWin Data.h:2268-2324 and DSA.cpp:1244-1312: global
             * commands use the same seven-bit index and MAXSTATE extension
             * contract as local variables, but address numGlobalVariables.
             * The runtime boundary requires a fully owned source-sized bank
             * rather than emulating the source UI warning for a missing one. */
            if (index >= (uint8_t)context->global_variable_count) {
                return CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL;
            }
            if (relative_state == -4) {
                if (cursor >= action->program_word_count) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                relative_state = (int)(int16_t)action->program_words[cursor++];
            }
            if (opcode == CSB_V1_CSBWIN_DSACMD_GLOBALFETCH) {
                if (!csb_v1_csbwin_dsa_stack_push(stack, &depth,
                                                    global_variables[index])) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
            } else if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth,
                                                      &global_variables[index])) {
                return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
            }
            next_state = relative_state;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND ||
                   opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND2) {
            uint16_t subcode = (uint16_t)((command >> 6) & 0x7fu);
            int effective_parameter_count = context->parameter_count;
            next_state = csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 13), 3);
            if (next_state == -4) {
                if (cursor >= action->program_word_count) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                /* EX_AMPERSAND reads this extension as an unsigned source word. */
                next_state = (int)action->program_words[cursor++];
            }
            if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND2) {
                /* CSBWin DSA.cpp:5143-5148 dispatches AMPERSAND2 through
                 * EX_AMPERSAND(exPkt, 128), not a distinct bytecode grammar. */
                subcode = (uint16_t)(subcode + 128u);
            }
            if (subcode == 90u) {
                rc = csb_v1_csbwin_dsa_expand_indirect(
                    stack, &depth, variables, variable_state, parameters,
                    context->parameter_count,
                    &subcode, &effective_parameter_count);
                if (rc != CSB_V1_CSBWIN_DSA_STACK_OK) return rc;
            }
            if (subcode == 98u || subcode == 99u) {
                const CSB_V1_DSAImportedAction *target_action;
                uint8_t target_opcode;
                uint32_t target_state;
                uint32_t target_column;

                /* DSA.cpp:4738-4765 pops state before column. JumpGear
                 * breaks its Execute loop, while GosubGear returns here and
                 * continues this authenticated source program. Only the
                 * existing source-bound JUMP/GOSUB transfer subset is
                 * admitted as a dynamic target. */
                if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth, &target_state) ||
                    !csb_v1_csbwin_dsa_stack_pop(stack, &depth, &target_column)) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                target_action = csb_v1_chaos_find_imported_action_column(
                    state, dsa_id, target_state, target_column);
                if (!target_action || !target_action->program_words ||
                    target_action->program_word_count < 1) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                target_opcode = (uint8_t)(target_action->program_words[0] & 0x3fu);
                if ((target_opcode != CSB_V1_CSBWIN_DSACMD_JUMP &&
                     target_opcode != CSB_V1_CSBWIN_DSACMD_GOSUB) ||
                    csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
                        state, dsa_id, target_state, target_column,
                        subcode == 99u ? 1 : 0,
                        &candidate.transfer) != CSB_V1_CSBWIN_DSA_EXECUTE_OK ||
                    candidate.transfer.final_state < 0) {
                    return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
                }
                candidate.transfer_executed = 1;
                ++candidate.dynamic_transfer_count;
                candidate.last_dynamic_transfer_state = target_state;
                candidate.last_dynamic_transfer_column = target_column;
                candidate.last_dynamic_transfer_gosub = subcode == 99u;
                if (subcode == 98u) {
                    next_state = candidate.transfer.final_state - (int)state_index;
                    dynamic_jump_requested = 1;
                }
            } else {
                rc = csb_v1_csbwin_dsa_execute_stack_subcode(
                    subcode, stack, &depth, &candidate.forced_state, variables,
                    variable_state, parameters,
                    effective_parameter_count, &context_candidate, pending_skin_writes,
                    &pending_skin_write_count, pending_excell_writes,
                    &pending_excell_write_count, pending_generator_writes,
                    &pending_generator_write_count, pending_monster_writes,
                    &pending_monster_write_count, pending_cell_writes,
                    &pending_cell_write_count, pending_object_property_writes,
                    &pending_object_property_write_count, &staged_saves_disabled,
                    &staged_random_state, pending_missile_writes,
                    &pending_missile_write_count, pending_character_writes,
                    &pending_character_write_count, pending_experience_writes,
                    &pending_experience_write_count, pending_character_swaps,
                    &pending_character_swap_count, pending_poison_writes,
                    &pending_poison_write_count, pending_cloud_requests,
                    &pending_cloud_request_count, pending_party_teleports,
                    &pending_party_teleport_count,
                    pending_monster_group_mutations,
                    &pending_monster_group_mutation_count,
                    pending_object_moves, &pending_object_move_count,
                    pending_actuator_copies,
                    &pending_actuator_copy_count, pending_sound_requests,
                    &pending_sound_request_count, &discard_text_requested,
                    &adjust_skills_parameters_requested,
                    pending_adjust_skills_parameters, pending_descriptions,
                    &pending_description_count, pending_say_text_requests,
                    &pending_say_text_request_count, pending_display_text_requests,
                    &pending_display_text_request_count, local_text,
                    pending_global_text_stores,
                    &pending_global_text_store_count,
                    pending_overlay_writes, &pending_overlay_write_count,
                    pending_overlay_palette_writes,
                    &pending_overlay_palette_write_count,
                    pending_parameter_messages,
                    &pending_parameter_message_count);
                if (rc != CSB_V1_CSBWIN_DSA_STACK_OK) return rc;
            }
        } else return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        candidate.next_state = next_state;
        if (dynamic_jump_requested) break;
    }
    /* Commit externally owned EXPOOL state before the local copies become
     * visible. The runtime's skin callback owns a full profile candidate, so
     * a rejected batch leaves every live save surface untouched. */
    for (i = 0; i < pending_skin_write_count; ++i) {
        if (!context->set_skin(context->skin_user,
                               pending_skin_writes[i].location,
                               pending_skin_writes[i].skin)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.skin_store_count;
        candidate.last_skin_store_location = pending_skin_writes[i].location;
        candidate.last_skin_store_before = pending_skin_writes[i].before;
        candidate.last_skin_store_after = pending_skin_writes[i].skin;
    }
    for (i = 0; i < pending_excell_write_count; ++i) {
        uint32_t after_words[8];
        uint32_t flags = pending_excell_writes[i].flags;
        unsigned int word;

        if (!context->set_excell_flags(
                context->excell_user, pending_excell_writes[i].location,
                pending_excell_writes[i].flags)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        for (word = 0u; word < 8u; ++word) {
            const uint32_t mask = 1u <<
                (pending_excell_writes[i].location & 31u);
            after_words[word] = pending_excell_writes[i].before[word] & ~mask;
            if ((flags & 1u) != 0u) after_words[word] |= mask;
            flags >>= 1;
        }
        ++candidate.excell_store_count;
        candidate.last_excell_store_location =
            pending_excell_writes[i].location;
        memcpy(candidate.last_excell_store_before,
               pending_excell_writes[i].before,
               sizeof(candidate.last_excell_store_before));
        memcpy(candidate.last_excell_store_after, after_words,
               sizeof(candidate.last_excell_store_after));
    }
    for (i = 0; i < pending_generator_write_count; ++i) {
        if ((context->commit_generator_delay &&
             !context->commit_generator_delay(
                 context->dungeon_user, pending_generator_writes[i].location,
                 pending_generator_writes[i].expected_delay,
                 pending_generator_writes[i].delay)) ||
            (!context->commit_generator_delay &&
             (!context->set_generator_delay ||
              !context->set_generator_delay(
                  context->dungeon_user, pending_generator_writes[i].location,
                  pending_generator_writes[i].delay)))) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.generator_delay_store_count;
        candidate.last_generator_delay_location =
            pending_generator_writes[i].location;
        candidate.last_generator_delay_before =
            pending_generator_writes[i].expected_delay;
        candidate.last_generator_delay_after =
            pending_generator_writes[i].delay;
        candidate.last_generator_delay_has_generator =
            pending_generator_writes[i].has_generator;
    }
    for (i = 0; i < pending_monster_write_count; ++i) {
        if (!context->set_monster_info(
                context->dungeon_user, pending_monster_writes[i].thing,
                pending_monster_writes[i].values,
                pending_monster_writes[i].write_mask)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.monster_store_count;
        candidate.last_monster_store_thing = pending_monster_writes[i].thing;
        candidate.last_monster_store_write_mask =
            pending_monster_writes[i].write_mask;
        memcpy(candidate.last_monster_store_before,
               pending_monster_writes[i].before,
               sizeof(candidate.last_monster_store_before));
        memcpy(candidate.last_monster_store_after,
               pending_monster_writes[i].values,
               sizeof(candidate.last_monster_store_after));
    }
    for (i = 0; i < pending_cell_write_count; ++i) {
        if (!context->set_cell_info(
                context->dungeon_user, pending_cell_writes[i].location,
                pending_cell_writes[i].values,
                pending_cell_writes[i].write_mask)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.cell_store_count;
        candidate.last_cell_store_location = pending_cell_writes[i].location;
        candidate.last_cell_store_write_mask = pending_cell_writes[i].write_mask;
        memcpy(candidate.last_cell_store_before, pending_cell_writes[i].before,
               sizeof(candidate.last_cell_store_before));
        memcpy(candidate.last_cell_store_after, pending_cell_writes[i].values,
               sizeof(candidate.last_cell_store_after));
        if (pending_cell_writes[i].false_pit) {
            ++candidate.false_pit_count;
            candidate.last_false_pit_location = pending_cell_writes[i].location;
            memcpy(candidate.last_false_pit_before,
                   pending_cell_writes[i].before,
                   sizeof(candidate.last_false_pit_before));
            memcpy(candidate.last_false_pit_after,
                   pending_cell_writes[i].values,
                   sizeof(candidate.last_false_pit_after));
        }
    }
    for (i = 0; i < pending_object_property_write_count; ++i) {
        uint32_t before = 0u;
        uint32_t after = 0u;

        if (!context->get_object_property ||
            context->get_object_property(
                context->dungeon_user, pending_object_property_writes[i].thing,
                pending_object_property_writes[i].property, &before) != 1) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (!context->set_object_property(
                context->dungeon_user,
                pending_object_property_writes[i].thing,
                pending_object_property_writes[i].property,
                pending_object_property_writes[i].value)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (context->get_object_property(
                context->dungeon_user, pending_object_property_writes[i].thing,
                pending_object_property_writes[i].property, &after) != 1) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.object_property_store_count;
        candidate.last_object_property_thing =
            pending_object_property_writes[i].thing;
        candidate.last_object_property_kind =
            (uint8_t)pending_object_property_writes[i].property;
        candidate.last_object_property_before = before;
        candidate.last_object_property_after = after;
    }
    for (i = 0; i < pending_missile_write_count; ++i) {
        if ((context->commit_missile_info &&
             !context->commit_missile_info(
                 context->dungeon_user, pending_missile_writes[i].thing,
                 pending_missile_writes[i].expected_values,
                 pending_missile_writes[i].values)) ||
            (!context->commit_missile_info &&
             (!context->set_missile_info ||
              !context->set_missile_info(
                  context->dungeon_user, pending_missile_writes[i].thing,
                  pending_missile_writes[i].values)))) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.missile_info_store_count;
        candidate.last_missile_info_thing = pending_missile_writes[i].thing;
        memcpy(candidate.last_missile_info_before,
               pending_missile_writes[i].expected_values,
               sizeof(candidate.last_missile_info_before));
        memcpy(candidate.last_missile_info_after,
               pending_missile_writes[i].values,
               sizeof(candidate.last_missile_info_after));
    }
    for (i = 0; i < pending_character_write_count; ++i) {
        if (!context->set_character_info ||
            !context->set_character_info(
                context->dungeon_user,
                pending_character_writes[i].character_selector,
                pending_character_writes[i].values,
                pending_character_writes[i].word_count)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
    }
    for (i = 0; i < pending_experience_write_count; ++i) {
        if (!context->add_experience_plus ||
            !context->add_experience_plus(
                context->dungeon_user,
                pending_experience_writes[i].character_selector,
                pending_experience_writes[i].skill_number,
                pending_experience_writes[i].experience)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.experience_plus_count;
        candidate.last_experience_character_selector =
            pending_experience_writes[i].character_selector;
        candidate.last_experience_skill_number =
            pending_experience_writes[i].skill_number;
        candidate.last_experience_amount =
            pending_experience_writes[i].experience;
    }
    for (i = 0; i < pending_character_swap_count; ++i) {
        if (!context->commit_character_swap ||
            !context->commit_character_swap(
                context->dungeon_user,
                pending_character_swaps[i].party_index,
                pending_character_swaps[i].fingerprint)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
    }
    for (i = 0; i < pending_poison_write_count; ++i) {
        if (!context->commit_cause_poison ||
            !context->commit_cause_poison(
                context->dungeon_user,
                pending_poison_writes[i].character_selector,
                pending_poison_writes[i].poison_value)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.cause_poison_count;
        candidate.last_cause_poison_character_selector =
            pending_poison_writes[i].character_selector;
        candidate.last_cause_poison_attack =
            pending_poison_writes[i].poison_value;
    }
    for (i = 0; i < pending_cloud_request_count; ++i) {
        if (!context->create_cloud || !context->create_cloud(
                context->dungeon_user,
                pending_cloud_requests[i].cloud_type,
                pending_cloud_requests[i].size,
                pending_cloud_requests[i].location)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.create_cloud_count;
        candidate.last_create_cloud_type =
            pending_cloud_requests[i].cloud_type;
        candidate.last_create_cloud_size = pending_cloud_requests[i].size;
        candidate.last_create_cloud_location =
            pending_cloud_requests[i].location;
    }
    for (i = 0; i < pending_party_teleport_count; ++i) {
        if (!context->teleport_party || !context->teleport_party(
                context->dungeon_user,
                pending_party_teleports[i].destination_location)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.teleport_party_count;
        candidate.last_teleport_party_destination =
            pending_party_teleports[i].destination_location;
    }
    for (i = 0; i < pending_monster_group_mutation_count; ++i) {
        if (!context->mutate_monster_group ||
            !context->mutate_monster_group(
                context->dungeon_user,
                pending_monster_group_mutations[i].location,
                pending_monster_group_mutations[i].operand,
                pending_monster_group_mutations[i].insert_monster)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.monster_group_mutation_count;
        candidate.last_monster_group_location =
            pending_monster_group_mutations[i].location;
        candidate.last_monster_group_operand =
            pending_monster_group_mutations[i].operand;
        candidate.last_monster_group_insert =
            pending_monster_group_mutations[i].insert_monster;
    }
    for (i = 0; i < pending_object_move_count; ++i) {
        if (!context->move_object || !context->move_object(
                context->dungeon_user,
                pending_object_moves[i].source_type,
                pending_object_moves[i].source_object_mask,
                pending_object_moves[i].source_position_mask,
                pending_object_moves[i].source_location,
                pending_object_moves[i].source_depth,
                pending_object_moves[i].destination_type,
                pending_object_moves[i].destination_object_mask,
                pending_object_moves[i].destination_position_mask,
                pending_object_moves[i].destination_location,
                pending_object_moves[i].destination_depth)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
    }
    for (i = 0; i < pending_actuator_copy_count; ++i) {
        if ((context->copy_actuator_payload &&
             context->copy_actuator_payload(
                 context->dungeon_user,
                 pending_actuator_copies[i].source_thing,
                 pending_actuator_copies[i].thing,
                 pending_actuator_copies[i].payload) <= 0) ||
            (!context->copy_actuator_payload &&
             (!context->set_actuator_payload ||
              context->set_actuator_payload(
                  context->dungeon_user, pending_actuator_copies[i].thing,
                  pending_actuator_copies[i].payload) <= 0))) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.actuator_copy_count;
        candidate.last_actuator_copy_source_thing =
            pending_actuator_copies[i].source_thing;
        candidate.last_actuator_copy_destination_thing =
            pending_actuator_copies[i].thing;
    }
    for (i = 0; i < pending_sound_request_count; ++i) {
        if (!context->play_sound || !context->play_sound(
                context->dungeon_user, pending_sound_requests[i].sound_number,
                pending_sound_requests[i].volume,
                pending_sound_requests[i].flags)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.sound_notification_count;
        candidate.last_sound_number = pending_sound_requests[i].sound_number;
        candidate.last_sound_volume = pending_sound_requests[i].volume;
        candidate.last_sound_flags = pending_sound_requests[i].flags;
    }
    if (adjust_skills_parameters_requested &&
        (!context->set_adjust_skills_parameters ||
         !context->set_adjust_skills_parameters(
             context->dungeon_user, pending_adjust_skills_parameters))) {
        return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
    }
    for (i = 0; i < pending_description_count; ++i) {
        if (!context->describe || !context->describe(
                context->dungeon_user, pending_descriptions[i].location,
                pending_descriptions[i].index,
                pending_descriptions[i].color)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
    }
    for (i = 0; i < pending_say_text_request_count; ++i) {
        if (!context->say_text || !context->say_text(
                context->text_user, pending_say_text_requests[i].location,
                pending_say_text_requests[i].color)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.say_text_count;
        candidate.last_say_text_location =
            pending_say_text_requests[i].location;
        candidate.last_say_text_color = pending_say_text_requests[i].color;
    }
    for (i = 0; i < pending_display_text_request_count; ++i) {
        if (!context->display_text || !context->display_text(
                context->text_user,
                pending_display_text_requests[i].text,
                pending_display_text_requests[i].color)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.display_text_count;
        candidate.last_display_text_color =
            pending_display_text_requests[i].color;
    }
    for (i = 0; i < pending_global_text_store_count; ++i) {
        if (!context->set_global_text || !context->set_global_text(
                context->text_user,
                pending_global_text_stores[i].global_index,
                pending_global_text_stores[i].text)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.global_text_store_count;
        candidate.last_global_text_store_index =
            pending_global_text_stores[i].global_index;
    }
    for (i = 0; i < pending_overlay_write_count; ++i) {
        CSB_V1_CSBWinDSAPendingOverlayWrite *write = &pending_overlay_writes[i];

        if (!context->set_overlay || !context->set_overlay(
                context->overlay_user, write->overlay_number,
                write->parameters[0], write->parameters[1],
                write->parameters[2], write->parameters[3])) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.overlay_store_count;
        candidate.last_overlay_number = write->overlay_number;
        memcpy(candidate.last_overlay_parameters, write->parameters,
               sizeof(candidate.last_overlay_parameters));
    }
    for (i = 0; i < pending_overlay_palette_write_count; ++i) {
        CSB_V1_CSBWinDSAPendingOverlayPaletteWrite *write =
            &pending_overlay_palette_writes[i];

        if (!context->set_overlay_palette || !context->set_overlay_palette(
                context->overlay_user, write->overlay_number,
                write->parameters[0], write->parameters[1],
                write->parameters[2])) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.overlay_palette_store_count;
        candidate.last_overlay_palette_number = write->overlay_number;
        memcpy(candidate.last_overlay_palette_parameters, write->parameters,
               sizeof(candidate.last_overlay_palette_parameters));
    }
    for (i = 0; i < pending_switch_action_count; ++i) {
        uint8_t event_type = 0u;
        int scheduled;

        if (!context->queue_switch_action) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        scheduled = context->queue_switch_action(
            context->dungeon_user, pending_switch_actions[i].delay,
            pending_switch_actions[i].action,
            pending_switch_actions[i].target_location,
            pending_switch_actions[i].message_route, &event_type);
        if (scheduled < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        if (scheduled > 0) {
            ++candidate.timer_scheduled_count;
            candidate.last_scheduled_event_type = event_type;
            candidate.last_scheduled_target_location =
                pending_switch_actions[i].target_location;
            candidate.last_scheduled_delay = pending_switch_actions[i].delay;
            candidate.last_scheduled_action = pending_switch_actions[i].action;
            ++candidate.message_scheduled_count;
            candidate.last_message_route =
                (uint8_t)pending_switch_actions[i].message_route;
        }
    }
    for (i = 0; i < pending_parameter_message_count; ++i) {
        uint8_t event_type = 0u;
        if (!context->queue_parameter_message ||
            !context->queue_parameter_message(
                context->dungeon_user,
                pending_parameter_messages[i].delay,
                pending_parameter_messages[i].message_type,
                pending_parameter_messages[i].target_location,
                pending_parameter_messages[i].parameters,
                pending_parameter_messages[i].parameter_count, &event_type)) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        ++candidate.timer_scheduled_count;
        ++candidate.parameter_message_count;
        candidate.last_scheduled_event_type = event_type;
        candidate.last_scheduled_delay = pending_parameter_messages[i].delay;
        candidate.last_scheduled_target_location =
            pending_parameter_messages[i].target_location;
        candidate.last_parameter_message_type =
            pending_parameter_messages[i].message_type;
        candidate.last_parameter_message_count =
            pending_parameter_messages[i].parameter_count;
    }
    for (i = 0; i < pending_teleporter_copy_count; ++i) {
        int copied;
        uint32_t source_before[5];
        uint32_t destination_before[5];
        uint32_t destination_after[5];

        if (!context->copy_teleporter) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        if (context->get_cell_info &&
            (!context->get_cell_info(context->dungeon_user,
                                     pending_teleporter_copies[i].source_location,
                                     source_before) ||
             !context->get_cell_info(context->dungeon_user,
                                     pending_teleporter_copies[i].destination_location,
                                     destination_before))) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        }
        copied = context->copy_teleporter(
            context->dungeon_user,
            pending_teleporter_copies[i].source_location,
            pending_teleporter_copies[i].destination_location);
        if (copied < 0) return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        if (copied > 0) {
            if (context->get_cell_info && !context->get_cell_info(
                    context->dungeon_user,
                    pending_teleporter_copies[i].destination_location,
                    destination_after)) {
                return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
            }
            ++candidate.teleporter_copy_count;
            candidate.last_teleporter_copy_source_location =
                pending_teleporter_copies[i].source_location;
            candidate.last_teleporter_copy_destination_location =
                pending_teleporter_copies[i].destination_location;
            memcpy(candidate.last_teleporter_copy_source_before, source_before,
                   sizeof(candidate.last_teleporter_copy_source_before));
            memcpy(candidate.last_teleporter_copy_destination_before,
                   destination_before,
                   sizeof(candidate.last_teleporter_copy_destination_before));
            memcpy(candidate.last_teleporter_copy_destination_after,
                   destination_after,
                   sizeof(candidate.last_teleporter_copy_destination_after));
        }
    }
    if (discard_text_requested && !context->discard_text(context->dungeon_user)) {
        return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
    }
    if (discard_text_requested) ++candidate.text_discard_count;
    if (override_requested && !context->set_override_p(
            context->override_user, context_candidate.override_p,
            context_candidate.override_position)) {
        return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
    }
    for (i = 0; i < 26 && i < context->parameter_count; ++i) context->parameters[i] = parameters[i];
    for (i = 0; i < context->global_variable_count; ++i) {
        context->global_variables[i] = global_variables[i];
    }
    if (context->saves_disabled_valid) {
        context->saves_disabled = staged_saves_disabled;
    }
    if (context->random_state_valid) {
        context->random_state = staged_random_state;
    }
    if (context->timer_type_modifiers_valid) {
        memcpy(context->timer_type_modifiers,
               context_candidate.timer_type_modifiers,
               sizeof(context->timer_type_modifiers));
    }
    if (context->jitter_state_valid) {
        context->x_graphic_jitter = context_candidate.x_graphic_jitter;
        context->y_graphic_jitter = context_candidate.y_graphic_jitter;
        context->x_overlay_jitter = context_candidate.x_overlay_jitter;
        context->y_overlay_jitter = context_candidate.y_overlay_jitter;
        context->jitter_changed = context_candidate.jitter_changed;
    }
    if (context->override_state_valid) {
        context->override_p = context_candidate.override_p;
        context->override_position = context_candidate.override_position;
    }
    memcpy(context->party_champion_talents,
           context_candidate.party_champion_talents,
           sizeof(context->party_champion_talents));
    candidate.wing_talents_store_count =
        context_candidate.wing_talents_store_count;
    candidate.last_wing_talents_fingerprint =
        context_candidate.last_wing_talents_fingerprint;
    candidate.last_wing_talents_before =
        context_candidate.last_wing_talents_before;
    candidate.last_wing_talents_after =
        context_candidate.last_wing_talents_after;
    if (override_requested) {
        candidate.override_p_count = 1;
        candidate.last_override_position = context_candidate.override_position;
    }
    candidate.words_consumed = (uint16_t)cursor;
    candidate.stack_depth = (uint16_t)depth;
    *out_execution = candidate;
    return CSB_V1_CSBWIN_DSA_STACK_OK;
}

int csb_v1_csbwin_dsa_run_authenticated_filter_stack_action(
    const CSB_V1_DSAImportedAction *action, int *parameters,
    int parameter_count, int flgs_inout[2], void *user)
{
    CSB_V1_CSBWinDSAFilterStackRunnerContext *runner = user;
    CSB_V1_CSBWinDSAStackContext context;
    CSB_V1_CSBWinDSAStackExecution execution;
    CSB_V1_CSBWinDSACoreProgramReceipt core_receipt;
    CSB_V1_CSBWinDSAExecuteReceipt transfer;
    const CSB_V1_DSAImportedAction *expected;
    uint32_t parameter_words[26];
    uint16_t opcode;
    int i;

    (void)flgs_inout;
    if (!action || !action->program_words || action->program_word_count < 1 ||
        (parameter_count > 0 && !parameters) || parameter_count < 0 ||
        parameter_count > 26 || !runner || !runner->programs ||
        runner->dsa_id < 0 || runner->action_ordinal < 0 ||
        runner->global_variable_count < 0 ||
        runner->global_variable_count > CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY) {
        return 0;
    }

    /* Do not allow a compatible-looking caller action to borrow the runtime
     * executor. The action pointer must be the exact item selected from the
     * checksum-authenticated CSBWin import, preserving file-order ownership. */
    expected = csb_v1_chaos_find_imported_action(runner->programs,
        runner->dsa_id, runner->state_index, runner->action_ordinal);
    if (expected != action) return 0;

    opcode = (uint16_t)(action->program_words[0] & 0x3fu);
    if (csb_v1_csbwin_dsa_verify_authenticated_core_program(
            runner->programs, runner->dsa_id, runner->state_index,
            runner->action_ordinal, &core_receipt) !=
            CSB_V1_CSBWIN_DSA_CORE_OK || !core_receipt.valid) {
        return 0;
    }
    if (opcode == CSB_V1_CSBWIN_DSACMD_JUMP ||
        opcode == CSB_V1_CSBWIN_DSACMD_GOSUB) {
        if (!core_receipt.transfer_only) return 0;
        /* CSBWin DSA.cpp Execute() owns this transfer as a whole, including
         * nested GOSUB frame handling. It has no parameter/world side effect
         * in the bounded subset, so retain the caller surface exactly while
         * publishing only the completed source final state. */
        if (csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
                runner->programs, runner->dsa_id, runner->state_index,
                action->column, 0, &transfer) !=
                CSB_V1_CSBWIN_DSA_EXECUTE_OK || transfer.final_state < 0) {
            return 0;
        }
        runner->last_transfer = transfer;
        runner->state_index = (uint32_t)transfer.final_state;
        ++runner->execution_count;
        ++runner->transfer_execution_count;
        return 1;
    }
    if (!core_receipt.stack_core) return 0;

    for (i = 0; i < parameter_count; ++i) {
        parameter_words[i] = (uint32_t)parameters[i];
    }
    memset(&context, 0, sizeof(context));
    context.master_location = runner->master_location;
    context.parameters = parameter_words;
    context.parameter_count = parameter_count;
    context.party_location_valid = runner->party_location_valid;
    context.party_level = runner->party_level;
    context.party_x = runner->party_x;
    context.party_y = runner->party_y;
    context.party_direction = runner->party_direction;
    context.movement_filter_active = flgs_inout != NULL;
    context.game_time_valid = runner->game_time_valid;
    context.game_time = runner->game_time;
    context.random_state_valid = runner->random_state_valid;
    context.random_state = runner->random_state;
    context.dsa_slave_thing_valid = runner->dsa_slave_thing_valid;
    context.dsa_slave_thing = runner->dsa_slave_thing;
    context.most_recent_interesting_object_valid =
        runner->most_recent_interesting_object_valid;
    context.most_recent_interesting_object = runner->most_recent_interesting_object;
    context.party_champions_valid = runner->party_champions_valid;
    context.party_champion_count = runner->party_champion_count;
    context.party_leader_index = runner->party_leader_index;
    memcpy(context.party_champion_talents, runner->party_champion_talents,
           sizeof(context.party_champion_talents));
    memcpy(context.party_champion_fingerprints,
           runner->party_champion_fingerprints,
           sizeof(context.party_champion_fingerprints));
    memcpy(context.party_champion_wounds, runner->party_champion_wounds,
           sizeof(context.party_champion_wounds));
    memcpy(context.party_champion_health, runner->party_champion_health,
           sizeof(context.party_champion_health));
    context.saves_disabled_valid = runner->saves_disabled_valid;
    context.saves_disabled = runner->saves_disabled;
    context.timer_type_modifiers_valid = runner->timer_type_modifiers_valid;
    memcpy(context.timer_type_modifiers, runner->timer_type_modifiers,
           sizeof(context.timer_type_modifiers));
    context.jitter_state_valid = runner->jitter_state_valid;
    context.x_graphic_jitter = runner->x_graphic_jitter;
    context.y_graphic_jitter = runner->y_graphic_jitter;
    context.x_overlay_jitter = runner->x_overlay_jitter;
    context.y_overlay_jitter = runner->y_overlay_jitter;
    context.jitter_changed = runner->jitter_changed;
    context.override_state_valid = runner->override_state_valid;
    context.override_p = runner->override_p;
    context.override_position = runner->override_position;
    context.set_override_p = runner->set_override_p;
    context.override_user = runner->override_user;
    context.global_variables = runner->global_variables;
    context.global_variable_count = runner->global_variable_count;
    context.get_skin = runner->get_skin;
    context.set_skin = runner->set_skin;
    context.skin_user = runner->skin_user;
    context.get_wing_talents = runner->get_wing_talents;
    context.has_wing_character = runner->has_wing_character;
    context.set_wing_talents = runner->set_wing_talents;
    context.get_dsa_info = runner->get_dsa_info;
    context.wing_user = runner->wing_user;
    context.get_excell_flags = runner->get_excell_flags;
    context.set_excell_flags = runner->set_excell_flags;
    context.excell_user = runner->excell_user;
    context.get_generator_delay = runner->get_generator_delay;
    context.set_generator_delay = runner->set_generator_delay;
    context.commit_generator_delay = runner->commit_generator_delay;
    context.get_monster_info = runner->get_monster_info;
    context.set_monster_info = runner->set_monster_info;
    context.monster_invisible_enabled = runner->monster_invisible_enabled;
    context.monster_size4_enabled = runner->monster_size4_enabled;
    context.get_cell_info = runner->get_cell_info;
    context.resolve_cell_store = runner->resolve_cell_store;
    context.set_cell_info = runner->set_cell_info;
    context.copy_teleporter = runner->copy_teleporter;
    context.get_object_property = runner->get_object_property;
    context.set_object_property = runner->set_object_property;
    context.normalize_object_property = runner->normalize_object_property;
    context.get_actuator_payload = runner->get_actuator_payload;
    context.set_actuator_payload = runner->set_actuator_payload;
    context.copy_actuator_payload = runner->copy_actuator_payload;
    context.get_champion_possession = runner->get_champion_possession;
    context.get_monster_possession = runner->get_monster_possession;
    context.inspect_cells = runner->inspect_cells;
    context.get_thing_type = runner->get_thing_type;
    context.fetch_object = runner->fetch_object;
    context.is_carried = runner->is_carried;
    context.get_level_multiplier = runner->get_level_multiplier;
    context.get_missile_info = runner->get_missile_info;
    context.set_missile_info = runner->set_missile_info;
    context.commit_missile_info = runner->commit_missile_info;
    context.get_mastery = runner->get_mastery;
    context.get_party_info = runner->get_party_info;
    context.get_character_info = runner->get_character_info;
    context.prepare_character_store = runner->prepare_character_store;
    context.set_character_info = runner->set_character_info;
    context.prepare_experience_plus = runner->prepare_experience_plus;
    context.add_experience_plus = runner->add_experience_plus;
    context.prepare_character_swap = runner->prepare_character_swap;
    context.commit_character_swap = runner->commit_character_swap;
    context.prepare_cause_poison = runner->prepare_cause_poison;
    context.commit_cause_poison = runner->commit_cause_poison;
    context.create_cloud = runner->create_cloud;
    context.teleport_party = runner->teleport_party;
    context.mutate_monster_group = runner->mutate_monster_group;
    context.move_object = runner->move_object;
    context.discard_text = runner->discard_text;
    context.play_sound = runner->play_sound;
    context.set_adjust_skills_parameters = runner->set_adjust_skills_parameters;
    context.describe = runner->describe;
    context.queue_switch_action = runner->queue_switch_action;
    context.queue_parameter_message = runner->queue_parameter_message;
    context.read_text = runner->read_text;
    context.read_character_name = runner->read_character_name;
    context.set_global_text = runner->set_global_text;
    context.say_text = runner->say_text;
    context.display_text = runner->display_text;
    context.text_user = runner->text_user;
    context.set_overlay = runner->set_overlay;
    context.set_overlay_palette = runner->set_overlay_palette;
    context.overlay_user = runner->overlay_user;
    context.dungeon_user = runner->dungeon_user;
    if (csb_v1_csbwin_dsa_execute_authenticated_stack_action(
            runner->programs, runner->dsa_id, runner->state_index,
            runner->action_ordinal, &context, &execution) !=
        CSB_V1_CSBWIN_DSA_STACK_OK) {
        return 0;
    }
    if (runner->random_state_valid) runner->random_state = context.random_state;
    if (runner->timer_type_modifiers_valid) {
        memcpy(runner->timer_type_modifiers, context.timer_type_modifiers,
               sizeof(runner->timer_type_modifiers));
    }
    if (runner->jitter_state_valid) {
        runner->x_graphic_jitter = context.x_graphic_jitter;
        runner->y_graphic_jitter = context.y_graphic_jitter;
        runner->x_overlay_jitter = context.x_overlay_jitter;
        runner->y_overlay_jitter = context.y_overlay_jitter;
        runner->jitter_changed = context.jitter_changed;
    }
    if (runner->override_state_valid) {
        runner->override_p = context.override_p;
        runner->override_position = context.override_position;
    }
    if (runner->monster_move_inhibit_valid) {
        memcpy(runner->monster_move_inhibit, context.monster_move_inhibit,
               sizeof(runner->monster_move_inhibit));
    }
    if (runner->party_champions_valid) {
        memcpy(runner->party_champion_talents,
               context.party_champion_talents,
               sizeof(runner->party_champion_talents));
    }
    for (i = 0; i < parameter_count; ++i) {
        parameters[i] = (int)parameter_words[i];
    }
    runner->saves_disabled = context.saves_disabled;
    runner->last_execution = execution;
    ++runner->execution_count;
    if (execution.transfer_executed) {
        runner->last_transfer = execution.transfer;
        runner->state_index = (uint32_t)execution.transfer.final_state;
        ++runner->transfer_execution_count;
    }
    return 1;
}

int csb_v1_chaos_load_scripts(CSB_V1_ChaosMagicState *state,
    const uint8_t *data, int data_size)
{
    /* CSBWin/DSA.cpp script loading:
     * Script table at start of DSA data block.
     * Each entry: uint16 byte offset, uint16 byte length.  Decode into an
     * owned word stream so a file-loader buffer may be released afterwards.
     * CSBWin/DSA.cpp DSA::Read retains the program rather than borrowing the
     * transient save/dungeon read buffer. */
    CSB_V1_ChaosMagicState candidate;
    int i, offset = 0;
    uint16_t count;
    size_t word_count;
    if (!state || !data || data_size < 2 ||
        data_size > CSB_V1_MAX_DSA_BYTECODE_BYTES || (data_size & 1) != 0) {
        return -1;
    }

    count = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    if (count > CSB_V1_MAX_DSA_SCRIPTS ||
        (size_t)count > ((size_t)data_size - 2u) / 4u) {
        return -1;
    }

    csb_v1_chaos_init(&candidate);
    word_count = (size_t)data_size / 2u;
    candidate.loaded_bytecode = calloc(word_count, sizeof(*candidate.loaded_bytecode));
    if (!candidate.loaded_bytecode) return -1;
    candidate.loaded_bytecode_words = (int)word_count;
    candidate.loaded_bytecode_magic = CSB_V1_DSA_LOADED_BYTECODE_MAGIC;
    for (i = 0; i < (int)word_count; ++i) {
        candidate.loaded_bytecode[i] = (uint16_t)data[i * 2] |
            ((uint16_t)data[i * 2 + 1] << 8);
    }

    candidate.script_count = count;
    offset = 2;

    for (i = 0; i < count; i++) {
        uint16_t soff = (uint16_t)data[offset] | ((uint16_t)data[offset+1] << 8);
        uint16_t slen = (uint16_t)data[offset+2] | ((uint16_t)data[offset+3] << 8);
        if ((soff & 1u) != 0 || (slen & 1u) != 0 ||
            (size_t)soff > (size_t)data_size ||
            (size_t)slen > (size_t)data_size - (size_t)soff) {
            csb_v1_chaos_cleanup(&candidate);
            return -1;
        }
        candidate.scripts[i].bytecode = candidate.loaded_bytecode + soff / 2u;
        candidate.scripts[i].bytecode_len = slen / 2u;
        offset += 4;
    }

    if (state->loaded_bytecode_magic == CSB_V1_DSA_LOADED_BYTECODE_MAGIC) {
        free(state->loaded_bytecode);
    }
    *state = candidate;
    return count;
}

int csb_v1_chaos_trigger(CSB_V1_ChaosMagicState *state, int script_id) {
    if (!state || script_id < 0 || script_id >= state->script_count) return -1;
    state->scripts[script_id].active = 1;
    state->scripts[script_id].pc = 0;
    state->scripts[script_id].sp = 0;
    return 0;
}

int csb_v1_dsa_execute_step(CSB_V1_DSAScript *script,
    CSB_V1_ChaosMagicState *state)
{
    uint16_t op;
    int op_pc;
    if (!script || !state || !script->active || !script->bytecode) return 0;
    if (script->delay_ticks > 0) { script->delay_ticks--; return 1; }
    if (script->pc >= script->bytecode_len) { script->active = 0; return 0; }

    op_pc = script->pc;
    op = script->bytecode[script->pc++];
    switch (op & 0xFF) {
        case CSB_DSA_OP_NOP: break;
        case CSB_DSA_OP_SET:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] = 1;
            }
            break;
        case CSB_DSA_OP_CLEAR:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] = 0;
            }
            break;
        case CSB_DSA_OP_TOGGLE:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] ^= 1;
            }
            break;
        case CSB_DSA_OP_TEST:
            if (!csb_v1_dsa_has_operands(script, 2)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                int target = script->bytecode[script->pc++];
                /* ReDMCSB: DEFS.H lines 1206-1208 constrain remote sensor
                 * target fields, and MOVESENS.C lines 1198-1206 indexes the
                 * target square directly before enqueuing the event.  Reject
                 * malformed DSA target operands at parse/VM time so imported
                 * edge scripts cannot drive an out-of-range target lookup. */
                if (!csb_v1_dsa_flag_is_valid(flag) ||
                    !csb_v1_dsa_target_is_valid(script, target)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                if (state->flags[flag])
                    script->pc = target;
            }
            break;
        case CSB_DSA_OP_DELAY:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            script->delay_ticks = script->bytecode[script->pc++];
            break;
        case CSB_DSA_OP_MESSAGE:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            /* CSBWin/DSA.cpp QueueDSASwitchAction lines 523-531 queues
             * TT_DESSAGE timer records, and ProcessDSATimer6 lines
             * 5415-5441 maps timer position/function to an Execute message
             * column.  The text surface eventually reaches ReDMCSB TEXT.C
             * F0047_TEXT_MESSAGEAREA_PrintMessage lines 1670-1775. */
            csb_v1_dsa_record_dispatch(state,
                CSB_V1_DSA_DISPATCH_MESSAGE,
                CSB_DSA_OP_MESSAGE,
                script->bytecode[script->pc++],
                op_pc);
            break;
        case CSB_DSA_OP_END:
            script->active = 0;
            break;
        default:
            script->pc++; /* skip unknown op + arg */
            break;
    }
    return script->active;
}

int csb_v1_chaos_tick(CSB_V1_ChaosMagicState *state) {
    int i, active = 0;
    if (!state) return 0;
    for (i = 0; i < state->script_count; i++) {
        if (state->scripts[i].active) {
            csb_v1_dsa_execute_step(&state->scripts[i], state);
            if (state->scripts[i].active) active++;
        }
    }
    return active;
}

const char *csb_v1_chaos_source_evidence(void) {
    return
        "CSBWin/Chaos.cpp:584 InitializeE\n"
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9 DSA dispatch\n"
        "CSBWin/DSA.cpp DSA interpreter (5806 lines)\n"
        "CSBWin/DSA.cpp:523-531 QueueDSASwitchAction TT_DESSAGE timer dispatch\n"
        "CSBWin/DSA.cpp:5415-5441 ProcessDSATimer6 message column execution\n"
        "ReDMCSB TEXT.C:1670-1775 F0047_TEXT_MESSAGEAREA_PrintMessage\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "CSB-specific: DSA bytecode VM, 256 global flags\n";
}

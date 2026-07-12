
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
#define CSB_V1_CSBWIN_DSA_VARIABLE_COUNT 100

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

    offset = features.extension_payload_offset;
    for (dsa_ordinal = 0u; dsa_ordinal < features.dsa_count; ++dsa_ordinal) {
        uint32_t dsa_id;
        uint32_t non_empty_states;
        uint32_t state_ordinal;
        if (offset > (size_t)size || (size_t)size - offset < 108u) goto reject;
        dsa_id = csb_v1_read_le32(bytes + offset);
        offset += 4u + 80u + 12u;
        offset += 4u; /* DSAState slot count, already validated by inspector. */
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
     * command. A GOSUB return frame intentionally discards its child result:
     * EX_GOSUB ignores Execute's return value before its caller returns. */
    for (;;) {
        const CSB_V1_DSAImportedAction *action =
            csb_v1_chaos_find_imported_action_column(state, dsa_id,
                                                       current_state,
                                                       current_column);
        uint16_t opcode;

        if (!action) {
            int returned_state = final_state == -1 ? (int)current_state : final_state;
            if (return_depth == 0) {
                candidate.final_state = returned_state;
                *out_receipt = candidate;
                return CSB_V1_CSBWIN_DSA_EXECUTE_OK;
            }
            --return_depth;
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

static CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_execute_stack_subcode(uint8_t subcode, uint32_t *stack,
    int *depth, int *forced_state)
{
    uint32_t v;
    uint32_t w;
    uint32_t count;
    int32_t sv;
    int32_t sw;

    if (!stack || !depth || !forced_state) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
    switch (subcode) {
    case 1u: /* STKOP_Plus */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w + v)) goto underflow;
        break;
    case 2u: /* STKOP_Roll */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &count) ||
            count >= (uint32_t)*depth) goto underflow;
        v = stack[*depth - (int)count - 1];
        memmove(&stack[*depth - (int)count - 1],
                &stack[*depth - (int)count], (size_t)count * sizeof(*stack));
        stack[*depth - 1] = v;
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
    case 27u: /* STKOP_Less */
    case 50u: /* STKOP_ULess */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w)) goto underflow;
        if (!csb_v1_csbwin_dsa_stack_push(stack, depth,
                subcode == 27u ? ((int32_t)w < (int32_t)v) : (w < v))) goto underflow;
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
    case 37u: /* STKOP_Gear */
        if (!csb_v1_csbwin_dsa_stack_pop(stack, depth, &v) ||
            !csb_v1_csbwin_dsa_stack_pop(stack, depth, &w) ||
            !csb_v1_csbwin_dsa_stack_push(stack, depth, w * v)) goto underflow;
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
    uint8_t variable_defined[CSB_V1_CSBWIN_DSA_VARIABLE_COUNT] = { 0u };
    CSB_V1_CSBWinDSAStackExecution candidate;
    int cursor = 0;
    int depth = 0;
    int i;

    if (!state || !context || !out_execution || !context->parameters ||
        context->parameter_count < 0 || context->global_variable_count < 0 ||
        context->global_variable_count > CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY ||
        (context->global_variable_count > 0 && !context->global_variables)) {
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
    memset(&candidate, 0, sizeof(candidate));
    candidate.forced_state = -1;
    while (cursor < action->program_word_count) {
        uint16_t command = action->program_words[cursor++];
        uint8_t opcode = (uint8_t)(command & 0x3fu);
        int next_state;
        CSB_V1_CSBWinDSAStackResult rc;
        ++candidate.command_count;
        if (opcode == CSB_V1_CSBWIN_DSACMD_LOAD) {
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
                if (!variable_defined[index]) variables[index] = 0u;
                if (!csb_v1_csbwin_dsa_stack_push(stack, &depth,
                                                    variables[index])) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
            } else {
                if (!csb_v1_csbwin_dsa_stack_pop(stack, &depth,
                                                   &variables[index])) {
                    return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                }
                variable_defined[index] = 1u;
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
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND) {
            uint8_t subcode = (uint8_t)((command >> 6) & 0x7fu);
            next_state = csb_v1_csbwin_dsa_sign_extend((uint16_t)(command >> 13), 3);
            if (next_state == -4) {
                if (cursor >= action->program_word_count) return CSB_V1_CSBWIN_DSA_STACK_MALFORMED;
                /* EX_AMPERSAND reads this extension as an unsigned source word. */
                next_state = (int)action->program_words[cursor++];
            }
            rc = csb_v1_csbwin_dsa_execute_stack_subcode(subcode, stack, &depth,
                                                           &candidate.forced_state);
            if (rc != CSB_V1_CSBWIN_DSA_STACK_OK) return rc;
        } else if (opcode == CSB_V1_CSBWIN_DSACMD_AMPERSAND2) {
            return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        } else return CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED;
        candidate.next_state = next_state;
    }
    for (i = 0; i < 26 && i < context->parameter_count; ++i) context->parameters[i] = parameters[i];
    for (i = 0; i < context->global_variable_count; ++i) {
        context->global_variables[i] = global_variables[i];
    }
    candidate.words_consumed = (uint16_t)cursor;
    candidate.stack_depth = (uint16_t)depth;
    *out_execution = candidate;
    return CSB_V1_CSBWIN_DSA_STACK_OK;
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
        "CSBWin/DSA.cpp:5053-5293 Execute continuation and return flow\n"
        "CSBWin/DSA.cpp:523-531 QueueDSASwitchAction TT_DESSAGE timer dispatch\n"
        "CSBWin/DSA.cpp:5415-5441 ProcessDSATimer6 message column execution\n"
        "CSBWin/Data.h:1686-1708 DSACOMMAND; 1947-1984 DSAloadCmd\n"
        "CSBWin/DSA.cpp:1074-1189 EX_LOAD; 1317-1385 EX_STORE\n"
        "ReDMCSB TEXT.C:1670-1775 F0047_TEXT_MESSAGEAREA_PrintMessage\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "CSB-specific: DSA bytecode VM, 256 global flags\n";
}

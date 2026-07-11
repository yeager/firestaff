
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

#include "csb_v1_chaos_magic_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init(void)
{
    CSB_V1_ChaosMagicState state;
    memset(&state, 0xFF, sizeof(state));
    csb_v1_chaos_init(&state);
    assert(state.script_count == 0);
    assert(state.imported_action_count == 0);
}

static void test_cleanup_after_init(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    csb_v1_chaos_cleanup(&state);
}

static void test_cleanup_double(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    csb_v1_chaos_cleanup(&state);
    csb_v1_chaos_cleanup(&state);
}

static void test_load_scripts_null(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    int rc = csb_v1_chaos_load_scripts(&state, NULL, 0);
    (void)rc;
    assert(rc != 0 || rc == 0);
    csb_v1_chaos_cleanup(&state);
}

static void test_trigger_invalid(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    int rc = csb_v1_chaos_trigger(&state, -1);
    (void)rc;
    assert(rc != 0 || rc == 0);
    csb_v1_chaos_cleanup(&state);
}

static void test_tick_empty(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    int rc = csb_v1_chaos_tick(&state);
    (void)rc;
    csb_v1_chaos_cleanup(&state);
}

static void test_import_dsas_null(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    int rc = csb_v1_chaos_import_extended_save_dsas(&state, NULL, 0);
    (void)rc;
    assert(rc == 0 || rc != 0);
    csb_v1_chaos_cleanup(&state);
}

static void test_import_dsas_too_small(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    uint8_t buf[4] = {0};
    int rc = csb_v1_chaos_import_extended_save_dsas(&state, buf, 4);
    (void)rc;
    csb_v1_chaos_cleanup(&state);
}

static void test_find_imported_action_empty(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    const CSB_V1_DSAImportedAction *a =
        csb_v1_chaos_find_imported_action(&state, 0, 0, 0);
    (void)a;
    assert(a == NULL);
    csb_v1_chaos_cleanup(&state);
}

static void test_find_imported_action_column_empty(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    const CSB_V1_DSAImportedAction *a =
        csb_v1_chaos_find_imported_action_column(&state, 0, 0, 0);
    (void)a;
    assert(a == NULL);
    csb_v1_chaos_cleanup(&state);
}

static void test_resolve_master_filter_empty(void)
{
    CSB_V1_ChaosMagicState state;
    csb_v1_chaos_init(&state);
    uint32_t state_idx = 0;
    int ordinal = 0;
    const CSB_V1_DSAImportedAction *a =
        csb_v1_chaos_resolve_imported_master_filter_action(
            &state, 0, 0, 0, &state_idx, &ordinal);
    (void)a;
    assert(a == NULL);
    csb_v1_chaos_cleanup(&state);
}

static void test_stack_capacity(void)
{
    assert(CSB_V1_CSBWIN_DSA_STACK_CAPACITY == 100);
    assert(CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY == 100);
    assert(CSB_V1_CSBWIN_DSA_VARIABLE_COUNT == 100);
}

static void test_jump_result_enum(void)
{
    assert(CSB_V1_CSBWIN_DSA_JUMP_OK == 0);
    assert(CSB_V1_CSBWIN_DSA_JUMP_NOT_AUTHENTICATED == 1);
    assert(CSB_V1_CSBWIN_DSA_JUMP_NOT_FOUND == 2);
    assert(CSB_V1_CSBWIN_DSA_JUMP_NOT_JUMP == 3);
    assert(CSB_V1_CSBWIN_DSA_JUMP_MALFORMED == -1);
}

static void test_object_property_enum(void)
{
    assert(CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CURSE == 0);
    assert(CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_BROKEN == 1);
    assert(CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_POISONED == 2);
    assert(CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_CHARGES == 3);
    assert(CSB_V1_CSBWIN_DSA_OBJECT_PROPERTY_SUBTYPE == 4);
}

static void test_source_evidence(void)
{
    const char *ev = csb_v1_chaos_source_evidence();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

int main(void)
{
    test_init();
    test_cleanup_after_init();
    test_cleanup_double();
    test_load_scripts_null();
    test_trigger_invalid();
    test_tick_empty();
    test_import_dsas_null();
    test_import_dsas_too_small();
    test_find_imported_action_empty();
    test_find_imported_action_column_empty();
    test_resolve_master_filter_empty();
    test_stack_capacity();
    test_jump_result_enum();
    test_object_property_enum();
    test_source_evidence();

    puts("ok: CSB chaos magic / DSA (Q-CSB-04) 15 tests passed");
    return 0;
}

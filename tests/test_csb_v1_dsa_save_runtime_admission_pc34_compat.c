#include "csb_v1_boot.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void build_otherwise_ready_import(
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 *receipt)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->csbwin_runtime_load_succeeded = 1;
    receipt->csbwin_dsa_corpus_positive = 1;
    receipt->csbwin_dsa_runtime_handoff_ready = 1;
    receipt->csbwin_dsa_extended_tail_valid = 1;
    receipt->csbwin_dsa_section_valid = 1;
    receipt->csbwin_dsa_has_runtime_actions = 1;
    receipt->csbwin_dsa_gameblock1_valid = 1;
    receipt->runtime_party_loaded_after = 1;
    receipt->runtime_import_source_after = CSB_SAVE_IMPORT_SOURCE;
    receipt->runtime_champion_count_after = 1;
}

int main(void)
{
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 save_import;
    CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 handoff;

    build_otherwise_ready_import(&save_import);
    check(!csb_v1_boot_runtime_dsa_save_handoff_receipt_pc34(
              &save_import, &handoff) && !handoff.valid &&
              !handoff.gameblock1_body_valid &&
              handoff.save_bytes_fnv1a == 0u &&
              handoff.gameblock1_body_fnv1a == 0u,
          "DSA handoff rejects a valid header without verified GAMEBLOCK1 body");

    save_import.csbwin_dsa_gameblock1_body_valid = 1;
    check(!csb_v1_boot_runtime_dsa_save_handoff_receipt_pc34(
              &save_import, &handoff) && !handoff.valid &&
              handoff.gameblock1_body_valid &&
              handoff.save_bytes_fnv1a == 0u &&
              handoff.gameblock1_body_fnv1a == 0u,
          "DSA handoff rejects a verified body without admitted source identities");

    return failures == 0 ? 0 : 1;
}

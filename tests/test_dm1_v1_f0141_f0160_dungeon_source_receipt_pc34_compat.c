#include "dm1_v1_f0141_f0160_dungeon_source_receipt_pc34_compat.h"
#include <stdio.h>
#include <string.h>
static int check(int ok, const char *name) { if (ok) return 1; fprintf(stderr, "FAIL: %s\n", name); return 0; }
int main(void) {
    DM1_V1_F0141F0160RuntimeInputPc34 input;
    DM1_V1_F0141F0160ReceiptPc34 receipt;
    const char *evidence;
    int ok = 1;
    memset(&input, 0, sizeof(input));
    ok &= check(!dm1_v1_f0141_f0160_dungeon_source_receipt_pc34(&input, &receipt), "missing original dungeon sources fail closed");
    ok &= check(receipt.suppressSyntheticRuntime && !receipt.valid, "no synthetic dungeon runtime is admitted");
    evidence = dm1_v1_f0141_f0160_source_evidence_pc34();
    ok &= check(strstr(evidence, "F0141/F0142") && strstr(evidence, "F0143-F0149") && strstr(evidence, "F0150-F0155") && strstr(evidence, "F0156-F0160"), "audit covers the complete disjoint range");
    ok &= check(dm1_v1_f0141_f0160_fnv1a_pc34(0, 0U) == 0U, "empty source cannot authenticate");
    if (!ok) return 1;
    puts("PASS dm1_v1_f0141_f0160_dungeon_source_receipt_pc34_compat"); return 0;
}

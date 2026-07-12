#include "theron_v1_irq2_live_trace_gate.h"

#include <stdio.h>
#include <string.h>

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

int main(void) {
    Theron_V1Irq2LiveTrace absent_trace;
    Theron_V1Irq2LiveBranchReceipt receipt;

    memset(&absent_trace, 0, sizeof(absent_trace));
    check(!theron_v1_irq2_live_branch_from_trace(&absent_trace, &receipt) &&
              !receipt.valid,
          "missing live trace cannot select an IRQ2 branch");
    ++g_skip;
    printf("[SKIP] no authenticated Mednafen IRQ2 trace staged; branch remains unbound\n");
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}

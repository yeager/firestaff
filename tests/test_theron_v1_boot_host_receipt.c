/*
 * test_theron_v1_boot_host_receipt.c — Theron V1 startup host-receipt apply facade
 *
 * Regression coverage for theron_v1_boot_apply_startup_host_receipt().
 * The facade owns the Theron_StartupHostReceipt -> host UI semantics mapping
 * so M11 no longer applies status/inspect/log/input-result directly.
 *
 * Source references:
 *   THQUEST.ASM T400 — startup state handoff and status/inspect flow
 *   include/theron_v1_startup_flow.h — Theron_StartupHostReceipt layout
 */

#include "theron_v1_boot.h"
#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* ── Test counters ─────────────────────────────────────────────────── */

static int g_tests_run    = 0;
static int g_tests_passed = 0;
static int g_failures     = 0;

#define TEST(name) do {                                             \
    printf("  %-55s ", name);                                      \
    fflush(stdout);                                                 \
    g_tests_run++;                                                  \
} while (0)

#define PASS() do {                                                 \
    printf("PASS\n");                                               \
    g_tests_passed++;                                               \
} while (0)

#define FAIL(msg) do {                                              \
    printf("FAIL: %s\n", msg);                                      \
    g_failures++;                                                   \
} while (0)

#define ASSERT(cond, msg) do {                                      \
    if (!(cond)) { FAIL(msg); return 0; }                           \
} while (0)

/* ══════════════════════════════════════════════════════════════════════
 * Mock callbacks
 * ══════════════════════════════════════════════════════════════════════ */

typedef struct {
    int set_status_calls;
    char last_status_scope[64];
    char last_status[64];

    int set_inspect_calls;
    char last_inspect_scope[64];
    char last_inspect_detail[320];

    int log_event_calls;
    unsigned int last_log_color;
    char last_log_line[320];
} MockHostReceiptContext;

static void mock_set_status(void *userdata, const char *scope, const char *status)
{
    MockHostReceiptContext *ctx = (MockHostReceiptContext *)userdata;
    ctx->set_status_calls++;
    snprintf(ctx->last_status_scope, sizeof(ctx->last_status_scope),
             "%s", scope ? scope : "");
    snprintf(ctx->last_status, sizeof(ctx->last_status),
             "%s", status ? status : "");
}

static void mock_set_inspect(void *userdata, const char *scope, const char *detail)
{
    MockHostReceiptContext *ctx = (MockHostReceiptContext *)userdata;
    ctx->set_inspect_calls++;
    snprintf(ctx->last_inspect_scope, sizeof(ctx->last_inspect_scope),
             "%s", scope ? scope : "");
    snprintf(ctx->last_inspect_detail, sizeof(ctx->last_inspect_detail),
             "%s", detail ? detail : "");
}

static void mock_log_event(void *userdata, unsigned int color, const char *line)
{
    MockHostReceiptContext *ctx = (MockHostReceiptContext *)userdata;
    ctx->log_event_calls++;
    ctx->last_log_color = color;
    snprintf(ctx->last_log_line, sizeof(ctx->last_log_line),
             "%s", line ? line : "");
}

static void setup_callbacks(Theron_V1_BootHostReceiptCallbacks *callbacks,
                            MockHostReceiptContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    callbacks->userdata = ctx;
    callbacks->set_status = mock_set_status;
    callbacks->set_inspect = mock_set_inspect;
    callbacks->log_event = mock_log_event;
}

/* ══════════════════════════════════════════════════════════════════════
 * Receipt init / null safety
 * ══════════════════════════════════════════════════════════════════════ */

static int test_null_receipt_returns_zero(void)
{
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result =
        (Theron_V1_BootHostReceiptResult)999;

    TEST("Null receipt returns 0 and defaults result to IGNORED");
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               NULL, NULL, &callbacks, &result) == 0,
           "null receipt should return 0");
    ASSERT(result == THERON_V1_BOOT_HOST_RECEIPT_RESULT_IGNORED,
           "result should default to IGNORED");
    ASSERT(ctx.set_status_calls == 0, "no status call for null receipt");
    PASS();
    return 1;
}

static int test_null_callbacks_returns_zero(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptResult result =
        (Theron_V1_BootHostReceiptResult)999;

    TEST("Null callbacks returns 0 and defaults result to IGNORED");
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = "READY";
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, NULL, &result) == 0,
           "null callbacks should return 0");
    ASSERT(result == THERON_V1_BOOT_HOST_RECEIPT_RESULT_IGNORED,
           "result should default to IGNORED");
    PASS();
    return 1;
}

static int test_null_out_result_ok(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;

    TEST("Null out_result is allowed");
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = "READY";
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, NULL) == 1,
           "null out_result should still consume receipt");
    ASSERT(ctx.set_status_calls == 1, "status should still be set");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Status / inspect / log callbacks
 * ══════════════════════════════════════════════════════════════════════ */

static int test_status_scope_and_status(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("Status scope and status are delivered to callback");
    memset(&receipt, 0, sizeof(receipt));
    receipt.status_scope = "STARTUP";
    receipt.status = "TRACK02 READY";
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.set_status_calls == 1, "one status call");
    ASSERT(strcmp(ctx.last_status_scope, "STARTUP") == 0,
           "scope passed through");
    ASSERT(strcmp(ctx.last_status, "TRACK02 READY") == 0,
           "status passed through");
    PASS();
    return 1;
}

static int test_status_defaults_to_startup_scope(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("Missing status scope defaults to STARTUP");
    memset(&receipt, 0, sizeof(receipt));
    receipt.status = "NO SCOPE";
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(strcmp(ctx.last_status_scope, "STARTUP") == 0,
           "default scope is STARTUP");
    ASSERT(strcmp(ctx.last_status, "NO SCOPE") == 0,
           "status passed through");
    PASS();
    return 1;
}

static int test_inspect_scope_and_detail(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("Inspect scope and detail are delivered to callback");
    memset(&receipt, 0, sizeof(receipt));
    receipt.inspect_scope = "CHAPTER";
    snprintf(receipt.inspect_detail, sizeof(receipt.inspect_detail),
             "level=0 seed=0x0108e938");
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.set_inspect_calls == 1, "one inspect call");
    ASSERT(strcmp(ctx.last_inspect_scope, "CHAPTER") == 0,
           "inspect scope passed through");
    ASSERT(strcmp(ctx.last_inspect_detail, "level=0 seed=0x0108e938") == 0,
           "inspect detail passed through");
    PASS();
    return 1;
}

static int test_inspect_detail_defaults_to_empty(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("Empty inspect detail is passed as empty string");
    memset(&receipt, 0, sizeof(receipt));
    receipt.inspect_scope = "CHAPTER";
    receipt.inspect_detail[0] = '\0';
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.set_inspect_calls == 1, "one inspect call");
    ASSERT(strcmp(ctx.last_inspect_detail, "") == 0,
           "empty detail passed as empty");
    PASS();
    return 1;
}

static int test_log_first_line(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("log_first_line is emitted through log_event callback");
    memset(&receipt, 0, sizeof(receipt));
    receipt.log_first_line = "TQR level load";
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.log_event_calls == 1, "one log call");
    ASSERT(strcmp(ctx.last_log_line, "TQR level load") == 0,
           "log line passed through");
    PASS();
    return 1;
}

static int test_runtime_receipt_logged_when_flag_set(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("runtime_receipt is logged when log_receipt is set");
    memset(&receipt, 0, sizeof(receipt));
    receipt.log_receipt = 1;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, "RUNTIME: Hall of Records",
               &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.log_event_calls == 1, "one log call");
    ASSERT(strcmp(ctx.last_log_line, "RUNTIME: Hall of Records") == 0,
           "runtime receipt passed through");
    PASS();
    return 1;
}

static int test_runtime_receipt_skipped_when_flag_clear(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("runtime_receipt is skipped when log_receipt is clear");
    memset(&receipt, 0, sizeof(receipt));
    receipt.log_receipt = 0;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, "RUNTIME: Hall of Records",
               &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.log_event_calls == 0, "no log call when flag clear");
    PASS();
    return 1;
}

static int test_both_log_lines_in_order(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;
    char first_line[320];

    TEST("log_first_line and runtime_receipt are emitted in order");
    memset(&receipt, 0, sizeof(receipt));
    receipt.log_first_line = "FIRST";
    receipt.log_receipt = 1;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, "SECOND", &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(ctx.log_event_calls == 2, "two log calls");
    snprintf(first_line, sizeof(first_line), "%s", ctx.last_log_line);
    ASSERT(strcmp(first_line, "SECOND") == 0,
           "last logged line should be runtime receipt");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Input result mapping
 * ══════════════════════════════════════════════════════════════════════ */

static int test_input_result_ignored(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("THERON_STARTUP_INPUT_RESULT_IGNORED maps to IGNORED");
    memset(&receipt, 0, sizeof(receipt));
    receipt.input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(result == THERON_V1_BOOT_HOST_RECEIPT_RESULT_IGNORED,
           "result should be IGNORED");
    PASS();
    return 1;
}

static int test_input_result_redraw(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("THERON_STARTUP_INPUT_RESULT_REDRAW maps to REDRAW");
    memset(&receipt, 0, sizeof(receipt));
    receipt.input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(result == THERON_V1_BOOT_HOST_RECEIPT_RESULT_REDRAW,
           "result should be REDRAW");
    PASS();
    return 1;
}

static int test_input_result_return_to_menu(void)
{
    Theron_StartupHostReceipt receipt;
    Theron_V1_BootHostReceiptCallbacks callbacks;
    MockHostReceiptContext ctx;
    Theron_V1_BootHostReceiptResult result;

    TEST("THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER maps to RETURN_TO_MENU");
    memset(&receipt, 0, sizeof(receipt));
    receipt.input_result = THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER;
    setup_callbacks(&callbacks, &ctx);
    ASSERT(theron_v1_boot_apply_startup_host_receipt(
               &receipt, NULL, &callbacks, &result) == 1,
           "should consume receipt");
    ASSERT(result == THERON_V1_BOOT_HOST_RECEIPT_RESULT_RETURN_TO_MENU,
           "result should be RETURN_TO_MENU");
    PASS();
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Main
 * ══════════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("=== Theron V1 Boot Host Receipt Apply Facade Tests ===\n\n");

    test_null_receipt_returns_zero();
    test_null_callbacks_returns_zero();
    test_null_out_result_ok();
    test_status_scope_and_status();
    test_status_defaults_to_startup_scope();
    test_inspect_scope_and_detail();
    test_inspect_detail_defaults_to_empty();
    test_log_first_line();
    test_runtime_receipt_logged_when_flag_set();
    test_runtime_receipt_skipped_when_flag_clear();
    test_both_log_lines_in_order();
    test_input_result_ignored();
    test_input_result_redraw();
    test_input_result_return_to_menu();

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed  (%s)\n",
           g_tests_passed, g_tests_run,
           g_failures == 0 ? "all passed" : "FAILURES");
    printf("=====================================================\n");
    return g_failures == 0 ? 0 : 1;
}

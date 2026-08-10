/* Source gate for ReDMCSB CHAMDRAW.C F0292 status-box ownership. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIRESTAFF_ROOT_PATH
#error "FIRESTAFF_ROOT_PATH required"
#endif

static int contains_between(const char *begin, const char *end,
                            const char *needle)
{
    size_t needle_length;
    const char *cursor;

    if (!begin || !end || end < begin || !needle) return 0;
    needle_length = strlen(needle);
    for (cursor = begin; cursor + needle_length <= end; ++cursor) {
        if (memcmp(cursor, needle, needle_length) == 0) return 1;
    }
    return 0;
}

int main(void)
{
    FILE *f = fopen(FIRESTAFF_ROOT_PATH "/src/engine/m11_game_view.c", "rb");
    char *source;
    char *panel;
    char *panel_end;
    long size;
    int ok;

    if (!f || fseek(f, 0, SEEK_END) || (size = ftell(f)) < 0 ||
        fseek(f, 0, SEEK_SET) ||
        !(source = malloc((size_t)size + 1U)) ||
        fread(source, 1, (size_t)size, f) != (size_t)size) {
        return 1;
    }
    source[size] = '\0';
    fclose(f);

    panel = strstr(source, "static void m11_draw_party_panel(");
    panel_end = panel ? strstr(panel,
        "static void m11_format_front_cell_prompt(") : NULL;
    ok = panel && panel_end && panel_end > panel &&
         /* m11_v1_chrome_mode_enabled widened from a zero-arg query to
          * one that takes state so it can consult V2 vertical-slice mode
          * per source kind, and a third predicate was appended to
          * suppress the fallback while the HoC presented-frame consumer
          * receipt is active. Substring-match only the historically
          * pinned two predicates so future guard extensions don't drift
          * this lock. */
         contains_between(panel, panel_end,
            "if (!drewStatusBox && !m11_v1_chrome_mode_enabled(state) &&\n"
            "                !m11_is_dm1_source_kind(state->sourceKind)") &&
         contains_between(panel, panel_end,
            "/* Procedural fallback.  V1 uses the same source status-box") &&
         contains_between(panel, panel_end,
            "CHAMDRAW.C F0292's C008 dead status box is a") &&
         contains_between(panel, panel_end, "x, y, -1);") &&
         !contains_between(panel, panel_end,
            "if (!drewStatusBox && !m11_v1_chrome_mode_enabled(state)) {");
    free(source);
    return ok ? 0 : 1;
}

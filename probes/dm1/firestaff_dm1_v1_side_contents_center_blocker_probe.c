#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Focused DM1 V1 viewport/walls occlusion source probe.
 *
 * Primary audit source (N2-local):
 *   ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C
 *
 * Source-locked slice:
 * - DUNVIEW.C:8446-8542: F0128 draws each depth row side cells before the
 *   same-depth center cell (D3L,D3R,D3C; D2L,D2R,D2C; D1L,D1R,D1C).
 * - DUNVIEW.C:7833-7872: F0124 D1C wall draw/alcove branch returns after
 *   drawing the nearest center wall.
 * - DUNVIEW.C:7874-7937: F0124 D1C door-front pass draws rear cells, door
 *   frame/door, then front cells.
 * - DUNVIEW.C:4547-4582,4819-4860,5195-5202,5679-5683,5915-5933:
 *   F0115 draws objects, then creatures, then projectiles for each packed
 *   visible cell; explosions are restarted and drawn after all cells.
 *
 * Firestaff parity guard:
 * - m11_draw_dm1_side_contents and m11_draw_dm1_deferred_explosion_pass must
 *   compute m11_dm1_nearest_blocking_center_depth_index(cells) and stop side
 *   item/creature/projectile/explosion drawing at the nearest blocking center
 *   depth so late split passes cannot repaint over a nearer center wall/door.
 * - dm1_v1_viewport_3d_pc34_compat keeps the F0115 content-layer metadata
 *   explicit: object/creature/projectile phases repeat per cell; the explosion
 *   phase runs after all cells.
 */

struct TextFile {
    char *data;
    size_t size;
};

static void free_text(struct TextFile *tf)
{
    free(tf->data);
    tf->data = NULL;
    tf->size = 0;
}

static int read_file(const char *path, struct TextFile *out)
{
    FILE *f = fopen(path, "rb");
    long n;
    size_t got;

    out->data = NULL;
    out->size = 0;
    if (!f) {
        fprintf(stderr, "FAIL open %s\n", path);
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        fprintf(stderr, "FAIL seek %s\n", path);
        return 0;
    }
    n = ftell(f);
    if (n < 0) {
        fclose(f);
        fprintf(stderr, "FAIL tell %s\n", path);
        return 0;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        fprintf(stderr, "FAIL rewind %s\n", path);
        return 0;
    }
    out->data = (char *)malloc((size_t)n + 1);
    if (!out->data) {
        fclose(f);
        fprintf(stderr, "FAIL alloc %s\n", path);
        return 0;
    }
    got = fread(out->data, 1, (size_t)n, f);
    fclose(f);
    if (got != (size_t)n) {
        free_text(out);
        fprintf(stderr, "FAIL read %s\n", path);
        return 0;
    }
    out->data[got] = 0;
    out->size = got;
    return 1;
}

static int line_no(const char *base, const char *p)
{
    int line = 1;
    const char *q;
    for (q = base; q && q < p; ++q) {
        if (*q == '\n') ++line;
    }
    return line;
}

static const char *must_find_after(const char *base, const char *from, const char *needle, const char *label)
{
    const char *p = strstr(from, needle);
    if (!p) {
        fprintf(stderr, "FAIL %s missing %s\n", label, needle);
        return NULL;
    }
    printf("anchor %s line=%d text=%s\n", label, line_no(base, p), needle);
    return p + strlen(needle);
}

static int text_has_order_from(const char *line_base, const char *search_from, const char *label,
    const char *const *needles, size_t count)
{
    const char *p = search_from;
    size_t i;

    for (i = 0; i < count; ++i) {
        const char *hit = strstr(p, needles[i]);
        if (!hit) {
            fprintf(stderr, "FAIL %s missing/in-wrong-order %s\n", label, needles[i]);
            return 0;
        }
        printf("anchor %s line=%d text=%s\n", label, line_no(line_base, hit), needles[i]);
        p = hit + strlen(needles[i]);
    }
    return 1;
}

static int text_has_order(const char *base, const char *label, const char *const *needles, size_t count)
{
    return text_has_order_from(base, base, label, needles, count);
}

static int verify_redmcsb_f0115_content_layers(const char *dunview)
{
    const char *fn = strstr(dunview, "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF");
    const char *const f0115_needles[] = {
        "draw each object found",
        "Draw one creature at the cell being processed",
        "Draw only projectiles at specified cell",
        "Draw only explosions at specified cell",
        "L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals);",
        "/* Draw objects */",
        "C04_THING_TYPE_GROUP",
        "C14_THING_TYPE_PROJECTILE",
        "C15_THING_TYPE_EXPLOSION",
        "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
        "/* Draw creatures */",
        "T0115129_DrawProjectiles:",
        "Restart processing list of objects from the beginning",
        "C14_THING_TYPE_PROJECTILE",
        "} while (L0130_ul_RemainingViewCellOrdinalsToProcess);",
        "/* Draw explosions */",
        "P0141_T_Thing = L0146_T_FirstThingToDraw",
        "C15_THING_TYPE_EXPLOSION"
    };

    if (!fn) {
        fprintf(stderr, "FAIL missing F0115 content-layer function anchor\n");
        return 0;
    }
    printf("audit f0115ContentEntry source=DUNVIEW.C:%d function=F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF\n",
        line_no(dunview, fn));
    return text_has_order_from(dunview, fn, "F0115-visible-cell-content-layers", f0115_needles,
        sizeof(f0115_needles) / sizeof(f0115_needles[0]));
}

static int verify_redmcsb_order(const char *dunview)
{
    const char *p = strstr(dunview, "F0098_DUNGEONVIEW_DrawFloorAndCeiling");
    if (!p) {
        fprintf(stderr, "FAIL missing floor/ceiling anchor DUNVIEW.C:2962\n");
        return 0;
    }
    printf("audit floorCeiling source=DUNVIEW.C:%d function=F0098_DUNGEONVIEW_DrawFloorAndCeiling\n", line_no(dunview, p));

    p = strstr(dunview, "void F0128_DUNGEONVIEW_Draw_CPSF");
    if (!p) {
        fprintf(stderr, "FAIL missing F0128 draw entry\n");
        return 0;
    }
    printf("audit drawEntry source=DUNVIEW.C:%d function=F0128_DUNGEONVIEW_Draw_CPSF\n", line_no(dunview, p));

    p = must_find_after(dunview, p, "F0116_DUNGEONVIEW_DrawSquareD3L", "D3L-before-D3C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0117_DUNGEONVIEW_DrawSquareD3R", "D3R-before-D3C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "D3C-center-after-sides");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0119_DUNGEONVIEW_DrawSquareD2L", "D2L-before-D2C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "D2R-before-D2C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0121_DUNGEONVIEW_DrawSquareD2C", "D2C-center-after-sides");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0122_DUNGEONVIEW_DrawSquareD1L", "D1L-before-D1C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0123_DUNGEONVIEW_DrawSquareD1R", "D1R-before-D1C");
    if (!p) return 0;
    p = must_find_after(dunview, p, "F0124_DUNGEONVIEW_DrawSquareD1C", "D1C-nearest-center-last");
    if (!p) return 0;

    p = strstr(dunview, "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C]");
    if (!p) {
        fprintf(stderr, "FAIL missing D1C center wall draw anchor DUNVIEW.C:7833-7840\n");
        return 0;
    }
    printf("audit d1cCenterWallDraw source=DUNVIEW.C:%d-7840\n", line_no(dunview, p));
    p = must_find_after(dunview, p, "return;", "D1C-wall-return-blocks-later-this-square-content");
    if (!p) return 0;

    p = strstr(p, "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT");
    if (!p) {
        fprintf(stderr, "FAIL missing D1C door rear pass anchor DUNVIEW.C:7874-7875\n");
        return 0;
    }
    printf("audit d1cDoorRearPass source=DUNVIEW.C:%d\n", line_no(dunview, p));
    p = must_find_after(dunview, p, "F0111_DUNGEONVIEW_DrawDoor", "D1C-door-drawn-after-rear-pass");
    if (!p) return 0;
    p = must_find_after(dunview, p, "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT", "D1C-door-front-pass-after-door");
    if (!p) return 0;

    return 1;
}

static const char *find_function_body(const char *text, const char *name, const char **end_out)
{
    char sig[128];
    const char *start;
    const char *brace;
    int depth = 0;
    const char *p;

    snprintf(sig, sizeof(sig), "static void %s(", name);
    start = strstr(text, sig);
    if (!start) return NULL;
    brace = strchr(start, '{');
    if (!brace) return NULL;
    for (p = brace; *p; ++p) {
        if (*p == '{') ++depth;
        else if (*p == '}') {
            --depth;
            if (depth == 0) {
                *end_out = p + 1;
                return start;
            }
        }
    }
    return NULL;
}

static int body_has_order(const char *base, const char *body, const char *end,
    const char *fn, const char *const *needles, size_t count)
{
    const char *p = body;
    size_t i;
    printf("audit firestaffFunction=%s line=%d\n", fn, line_no(base, body));
    for (i = 0; i < count; ++i) {
        const char *hit = strstr(p, needles[i]);
        if (!hit || hit >= end) {
            fprintf(stderr, "FAIL %s missing/in-wrong-order %s\n", fn, needles[i]);
            return 0;
        }
        printf("guard %s line=%d text=%s\n", fn, line_no(base, hit), needles[i]);
        p = hit + strlen(needles[i]);
    }
    return 1;
}

static int verify_firestaff_content_layer_metadata(const char *view3d_c, const char *view3d_h, const char *view3d_test)
{
    const char *const layer_order[] = {
        "DM1_VIEWPORT_THING_LAYER_OBJECTS",
        "DM1_VIEWPORT_THING_LAYER_CREATURES",
        "DM1_VIEWPORT_THING_LAYER_PROJECTILES",
        "DM1_VIEWPORT_THING_LAYER_EXPLOSIONS"
    };
    const char *const source_lines[] = {
        "DUNVIEW.C:4567-4571,4853-4860",
        "DUNVIEW.C:4573,5195-5202",
        "DUNVIEW.C:4575-4577,5681-5883",
        "DUNVIEW.C:4579-4581,5915-5933"
    };
    const char *const h_contract[] = {
        "object/creature/projectile phases for each",
        "then restarts once after all cells for explosions",
        "DUNVIEW.C:4567-4581, 5915-5933"
    };
    const char *const test_contract[] = {
        "F0115.layer.count",
        "i < 3 ? 1 : 0",
        "i == 3 ? 1 : 0"
    };

    if (!text_has_order(view3d_c, "dm1_v1_viewport_3d_pc34_compat.c:s_thing_layers", layer_order,
            sizeof(layer_order) / sizeof(layer_order[0]))) return 0;
    if (!text_has_order(view3d_c, "dm1_v1_viewport_3d_pc34_compat.c:source-lines", source_lines,
            sizeof(source_lines) / sizeof(source_lines[0]))) return 0;
    if (!text_has_order(view3d_h, "dm1_v1_viewport_3d_pc34_compat.h:layer-contract", h_contract,
            sizeof(h_contract) / sizeof(h_contract[0]))) return 0;
    return text_has_order(view3d_test, "test_dm1_v1_viewport_3d_pc34_compat.c:layer-contract", test_contract,
        sizeof(test_contract) / sizeof(test_contract[0]));
}

static int verify_firestaff_guards(const char *m11)
{
    const char *end = NULL;
    const char *body = find_function_body(m11, "m11_draw_dm1_side_contents", &end);
    const char *side_needles[] = {
        "blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "if (blockingCenterDepth >= 0 && depth >= blockingCenterDepth)",
        "break;",
        "m11_draw_item_sprite"
    };
    const char *explosion_needles[] = {
        "blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "if (blockingCenterDepth >= 0 && depth >= blockingCenterDepth)",
        "m11_draw_dm1_deferred_side_explosion"
    };

    if (!body) {
        fprintf(stderr, "FAIL missing m11_draw_dm1_side_contents\n");
        return 0;
    }
    if (!body_has_order(m11, body, end, "m11_draw_dm1_side_contents", side_needles,
            sizeof(side_needles) / sizeof(side_needles[0]))) {
        return 0;
    }

    body = find_function_body(m11, "m11_draw_dm1_deferred_explosion_pass", &end);
    if (!body) {
        fprintf(stderr, "FAIL missing m11_draw_dm1_deferred_explosion_pass\n");
        return 0;
    }
    return body_has_order(m11, body, end, "m11_draw_dm1_deferred_explosion_pass", explosion_needles,
        sizeof(explosion_needles) / sizeof(explosion_needles[0]));
}

int main(int argc, char **argv)
{
    char redmcsb_path[1024];
    char m11_path[1024];
    char view3d_c_path[1024];
    char view3d_h_path[1024];
    char view3d_test_path[1024];
    const char *root = argc > 1 ? argv[1] : ".";
    const char *home = getenv("HOME");
    struct TextFile dunview;
    struct TextFile m11;
    struct TextFile view3d_c;
    struct TextFile view3d_h;
    struct TextFile view3d_test;
    int ok = 1;

    if (!home) home = "/home/trv2";
    snprintf(redmcsb_path, sizeof(redmcsb_path), "%s/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C", home);
    snprintf(m11_path, sizeof(m11_path), "%s/src/engine/m11_game_view.c", root);
    snprintf(view3d_c_path, sizeof(view3d_c_path), "%s/src/dm1/dm1_v1_viewport_3d_pc34_compat.c", root);
    snprintf(view3d_h_path, sizeof(view3d_h_path), "%s/include/dm1_v1_viewport_3d_pc34_compat.h", root);
    snprintf(view3d_test_path, sizeof(view3d_test_path), "%s/tests/test_dm1_v1_viewport_3d_pc34_compat.c", root);

    printf("probe=firestaff_dm1_v1_side_contents_center_blocker_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C\n");
    printf("sourceEvidence=DUNVIEW.C:4547-4582,4819-4860,5195-5202,5679-5683,5915-5933,7833-7937,8446-8542; dm1_v1_viewport_3d_pc34_compat.[ch],test_dm1_v1_viewport_3d_pc34_compat.c; m11_game_view.c:m11_draw_dm1_side_contents,m11_draw_dm1_deferred_explosion_pass\n");

    ok &= read_file(redmcsb_path, &dunview);
    ok &= read_file(m11_path, &m11);
    ok &= read_file(view3d_c_path, &view3d_c);
    ok &= read_file(view3d_h_path, &view3d_h);
    ok &= read_file(view3d_test_path, &view3d_test);
    if (ok) {
        ok &= verify_redmcsb_order(dunview.data);
        ok &= verify_redmcsb_f0115_content_layers(dunview.data);
        ok &= verify_firestaff_content_layer_metadata(view3d_c.data, view3d_h.data, view3d_test.data);
        ok &= verify_firestaff_guards(m11.data);
    }
    free_text(&dunview);
    free_text(&m11);
    free_text(&view3d_c);
    free_text(&view3d_h);
    free_text(&view3d_test);

    if (!ok) {
        fprintf(stderr, "probe failed\n");
        return 1;
    }
    printf("result=pass\n");
    return 0;
}

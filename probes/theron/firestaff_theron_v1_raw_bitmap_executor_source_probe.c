/*
 * Source-level guard for the startup bitmap route boundary.  Stage Select
 * and Soul Room plans may contain legacy text/rectangle commands, but those
 * plans must never reach the fallback executor without their raw Track 02
 * bitmap routes.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *read_file(const char *path)
{
    FILE *file;
    char *contents;
    long length;

    file = fopen(path, "rb");
    if (!file) {
        return NULL;
    }
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) < 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    contents = (char *)malloc((size_t)length + 1u);
    if (!contents) {
        fclose(file);
        return NULL;
    }
    if (fread(contents, 1, (size_t)length, file) != (size_t)length) {
        free(contents);
        fclose(file);
        return NULL;
    }
    fclose(file);
    contents[length] = '\0';
    return contents;
}

static int contains_after(const char *contents, const char *first,
                          const char *second)
{
    const char *first_at = strstr(contents, first);
    const char *second_at = strstr(contents, second);

    return first_at && second_at && first_at < second_at;
}

int main(int argc, char **argv)
{
    char *boot;
    char *flow;
    const char *gate = "else if (plan.required_bitmap_route_mask != 0u)";
    const char *blocked = "RAW TRACK02 BITMAP REQUIRED";
    const char *fallback =
        "theron_v1_boot_startup_execute_graphics_plan(&plan, executor)";
    const char *stage =
        "case THERON_STARTUP_PHASE_STAGE_SELECT:\n"
        "        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;";
    const char *soul =
        "case THERON_STARTUP_PHASE_SOUL_ROOM:\n"
        "        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM |\n"
        "               THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;";

    if (argc != 3) {
        fprintf(stderr, "usage: %s boot.c startup_flow.c\n", argv[0]);
        return 2;
    }
    boot = read_file(argv[1]);
    flow = read_file(argv[2]);
    if (!boot || !flow) {
        fprintf(stderr, "could not read startup sources\n");
        free(boot);
        free(flow);
        return 2;
    }
    if (!contains_after(boot, gate, blocked) ||
        !contains_after(boot, blocked, fallback) || !strstr(flow, stage) ||
        !strstr(flow, soul)) {
        fprintf(stderr, "raw bitmap executor boundary is incomplete\n");
        free(boot);
        free(flow);
        return 1;
    }
    free(boot);
    free(flow);
    return 0;
}

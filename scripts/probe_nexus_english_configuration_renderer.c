#include "nexus_v1_retail_title_menu_renderer.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_int(const char *text, int *out_value)
{
    char *end = NULL;
    long value;
    if (!text || !out_value) {
        return 0;
    }
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 0 || value > 1000000) {
        return 0;
    }
    *out_value = (int)value;
    return 1;
}

int main(int argc, char **argv)
{
    enum { TITLE_BIN_BYTES = 112216 };
    FILE *input = NULL;
    FILE *output = NULL;
    Nexus_TitleScreen title;
    Nexus_V1_RetailTitleMenuState state;
    Nexus_Framebuffer *framebuffer = NULL;
    int row;
    int music;
    int stereo;
    int easy;
    int gauge;
    int animation_frame;
    int result = 1;

    if (argc != 9 ||
        !parse_int(argv[3], &row) ||
        !parse_int(argv[4], &music) ||
        !parse_int(argv[5], &stereo) ||
        !parse_int(argv[6], &easy) ||
        !parse_int(argv[7], &gauge) ||
        !parse_int(argv[8], &animation_frame) ||
        row > NEXUS_V1_RETAIL_CONFIGURATION_EXIT || music > 1 ||
        stereo > 1 || easy > 1 || gauge > 1) {
        fprintf(stderr,
                "usage: %s TITLE.BIN OUTPUT ROW MUSIC STEREO EASY GAUGE FRAME\n",
                argc > 0 ? argv[0] : "probe_nexus_english_configuration_renderer");
        return 2;
    }

    memset(&title, 0, sizeof(title));
    input = fopen(argv[1], "rb");
    if (!input) {
        perror("open TITLE.BIN");
        goto cleanup;
    }
    title.retail_menu_source_bytes = malloc(TITLE_BIN_BYTES);
    if (!title.retail_menu_source_bytes ||
        fread(title.retail_menu_source_bytes, 1, TITLE_BIN_BYTES, input) !=
            TITLE_BIN_BYTES ||
        fgetc(input) != EOF) {
        fprintf(stderr, "TITLE.BIN is missing or has an unexpected size\n");
        goto cleanup;
    }
    fclose(input);
    input = NULL;
    title.retail_menu_source_size = TITLE_BIN_BYTES;
    title.retail_menu_source_region = NEXUS_V1_TITLE_MENU_SOURCE_ENGLISH;

    nexus_v1_retail_title_menu_init(&state);
    state.menu_active = 1;
    state.selection = NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION;
    state.configuration_active = 1;
    state.configuration_row = (Nexus_V1_RetailConfigurationRow)row;
    state.music_enabled = music;
    state.stereo_enabled = stereo;
    state.easy_level_enabled = easy;
    state.monster_gauge_enabled = gauge;
    framebuffer = calloc(1, sizeof(*framebuffer));
    if (!framebuffer ||
        !nexus_v1_retail_title_menu_render(&title, &state, framebuffer,
                                           animation_frame)) {
        fprintf(stderr, "retail Configuration renderer rejected the input\n");
        goto cleanup;
    }

    output = fopen(argv[2], "wb");
    if (!output) {
        perror("open renderer output");
        goto cleanup;
    }
    if (fwrite(framebuffer->color_buffer, 1,
               sizeof(framebuffer->color_buffer), output) !=
            sizeof(framebuffer->color_buffer) ||
        fwrite(framebuffer->palette, sizeof(framebuffer->palette[0]), 256,
               output) != 256) {
        perror("write renderer output");
        goto cleanup;
    }
    result = 0;

cleanup:
    if (output) {
        fclose(output);
    }
    if (input) {
        fclose(input);
    }
    free(framebuffer);
    free(title.retail_menu_source_bytes);
    return result;
}

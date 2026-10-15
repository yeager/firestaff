#include "dm1_v1_amiga_entrance_f0441.h"
#include "firestaff_amiga_adf.h"
#include "firestaff_zip_extract.h"
#include "asset_status_m12.h"
#include "asset_find_by_hash.h"
#include "fs_portable_compat.h"
#include <SDL3/SDL.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *graphics;
    size_t graphics_bytes;
    uint8_t *dungeon_f;
    size_t dungeon_f_bytes;
    uint8_t *dungeon_g;
    size_t dungeon_g_bytes;
    uint8_t *dungeon;
    size_t dungeon_bytes;
    uint8_t *driver;
    size_t driver_bytes;
} SelectedMedia;

static int selected_adf_member(const char *name, const uint8_t *bytes,
                               size_t byte_count, void *user_data) {
    SelectedMedia *media = (SelectedMedia *)user_data;
    const char *expected;
    uint8_t **destination;
    size_t *destination_bytes;
    if (!name || !bytes || !media) return -1;
    expected = "graphics.dat";
    destination = &media->graphics;
    destination_bytes = &media->graphics_bytes;
    if (strlen(name) == strlen("DUNGEONF.DAT") &&
        tolower((unsigned char)name[7]) == 'f') {
        expected = "dungeonf.dat";
        destination = &media->dungeon_f;
        destination_bytes = &media->dungeon_f_bytes;
    } else if (strlen(name) == strlen("DUNGEONG.DAT") &&
               tolower((unsigned char)name[7]) == 'g') {
        expected = "dungeong.dat";
        destination = &media->dungeon_g;
        destination_bytes = &media->dungeon_g_bytes;
    }
    if (strlen(name) == strlen("DUNGEON.DAT")) {
        expected = "dungeon.dat";
        destination = &media->dungeon;
        destination_bytes = &media->dungeon_bytes;
    }
    if (strlen(name) == strlen("VDEO.FTL")) {
        expected = "vdeo.ftl";
        destination = &media->driver;
        destination_bytes = &media->driver_bytes;
    }
    for (size_t index = 0; ; ++index) {
        if (tolower((unsigned char)name[index]) != (unsigned char)expected[index]) return 0;
        if (!expected[index]) break;
    }
    if (*destination) return -1;
    *destination = malloc(byte_count);
    if (!*destination) return -1;
    memcpy(*destination, bytes, byte_count);
    *destination_bytes = byte_count;
    return 0;
}

static int write_original(const char* path, const uint8_t* bytes, size_t size)
{
    FILE* file = fopen(path,"wb");
    int ok;
    if (!file || !bytes || !size) { if (file) fclose(file); return 0; }
    ok = fwrite(bytes,1,size,file) == size;
    return fclose(file) == 0 && ok;
}

static int check_a36_inventory(const char* selected, const char* expectedGraphics,
                               int expectedReady)
{
    M12_AssetStatus* status = calloc(1,sizeof(*status));
    int index = M12_AssetStatus_FindVersionIndex("dm1","amiga36-multi");
    char runtime[M12_ASSET_DATA_DIR_CAPACITY];
    int ok = 0;
    if (!status || index < 0) goto done;
    for (int scoped=0; scoped<2; ++scoped) {
        const M12_AssetVersionStatus* version;
        if (scoped) M12_AssetStatus_ScanGame(status,selected,"dm1");
        else M12_AssetStatus_Scan(status,selected);
        version=M12_AssetStatus_GetVersion(status,"dm1",(size_t)index);
        if (!version || version->matched!=expectedReady ||
            M12_AssetStatus_PrepareDM1RuntimeVersion(status,"amiga36-multi",
                runtime,sizeof(runtime))!=expectedReady ||
            (expectedReady && (strcmp(version->matchedMd5,"7f9458e4a3972d06e649a6fa85a7f34b") ||
                               strcmp(version->matchedPath,expectedGraphics)))) goto done;
    }
    ok=1;
done:
    if (!ok) fprintf(stderr,"FAIL: A36 same-owner inventory selected=%s expected=%d\n",selected,expectedReady);
    free(status);
    return ok;
}

/* Stage byte-for-byte original files only. An incomplete first graphics copy
 * must not hide a later complete install or borrow its required driver. */
static int check_original_package_ownership(const SelectedMedia* media)
{
    const char* scratch=getenv("FIRESTAFF_TEST_SCRATCH");
    char root[FSP_PATH_MAX] = {0}, orphan[FSP_PATH_MAX] = {0}, complete[FSP_PATH_MAX] = {0};
    char neighbor[FSP_PATH_MAX] = {0}, gfx[2][FSP_PATH_MAX] = {{0}}, dun[2][FSP_PATH_MAX] = {{0}};
    char driver[3][FSP_PATH_MAX] = {{0}}, localized[2][FSP_PATH_MAX] = {{0}};
    int ok=0;
    if (!scratch || !scratch[0]) scratch=".codex-scratch";
    if (!media->dungeon || !media->dungeon_f || !media->dungeon_g || !media->driver ||
        snprintf(root,sizeof(root),"%s/a36-package-%llu",scratch,
                 (unsigned long long)SDL_GetTicksNS()) >= (int)sizeof(root) ||
        FSP_DirExists(root) || !FSP_CreateDirectoryRecursive(root) ||
        !FSP_JoinPath(orphan,sizeof(orphan),root,"a-orphan") ||
        !FSP_JoinPath(complete,sizeof(complete),root,"z-complete") ||
        !FSP_JoinPath(neighbor,sizeof(neighbor),root,"driver-only") ||
        !FSP_CreateDirectory(orphan) || !FSP_CreateDirectory(complete) ||
        !FSP_CreateDirectory(neighbor)) return 0;
    for (int i=0;i<2;++i) {
        const char* parent=i ? complete : orphan;
        if (!FSP_JoinPath(gfx[i],sizeof(gfx[i]),parent,"GRAPHICS.DAT") ||
            !FSP_JoinPath(dun[i],sizeof(dun[i]),parent,"DUNGEON.DAT") ||
            !FSP_JoinPath(driver[i],sizeof(driver[i]),parent,"VDEO.FTL") ||
            !write_original(gfx[i],media->graphics,media->graphics_bytes) ||
            !write_original(dun[i],media->dungeon,media->dungeon_bytes)) goto done;
    }
    if (!FSP_JoinPath(driver[2],sizeof(driver[2]),neighbor,"VDEO.FTL") ||
        !write_original(driver[1],media->driver,media->driver_bytes) ||
        !write_original(driver[2],media->driver,media->driver_bytes) ||
        !check_a36_inventory(root,gfx[1],1) ||
        !check_a36_inventory(gfx[0],NULL,0) ||
        !check_a36_inventory(orphan,NULL,0)) goto done;
    if (!write_original(driver[0],media->graphics,media->graphics_bytes) ||
        !check_a36_inventory(gfx[0],NULL,0) ||
        !check_a36_inventory(root,gfx[1],1)) goto done;
    if (!write_original(driver[0],media->driver,media->driver_bytes) ||
        !check_a36_inventory(gfx[0],gfx[0],1)) goto done;
    /* All three original language payloads remain valid in the loader's
     * explicit DUNGEON.DAT role. Localized filenames alone remain unready. */
    if (!write_original(dun[0],media->dungeon_f,media->dungeon_f_bytes) ||
        !check_a36_inventory(gfx[0],gfx[0],1) ||
        !write_original(dun[0],media->dungeon_g,media->dungeon_g_bytes) ||
        !check_a36_inventory(gfx[0],gfx[0],1) ||
        !FSP_JoinPath(localized[0],sizeof(localized[0]),orphan,"DUNGEONF.DAT") ||
        !FSP_JoinPath(localized[1],sizeof(localized[1]),orphan,"DUNGEONG.DAT") ||
        !write_original(localized[0],media->dungeon_f,media->dungeon_f_bytes) ||
        !write_original(localized[1],media->dungeon_g,media->dungeon_g_bytes)) goto done;
    if (remove(dun[0]) || !check_a36_inventory(gfx[0],NULL,0) ||
        !check_a36_inventory(root,gfx[1],1)) goto done;
    /* Every staged original that remains must retain its authentic identity. */
    if (!asset_file_matches_md5(gfx[0],"7f9458e4a3972d06e649a6fa85a7f34b") ||
        !asset_file_matches_md5(gfx[1],"7f9458e4a3972d06e649a6fa85a7f34b") ||
        !asset_file_matches_md5(dun[1],"30028fb6a301ecb20127ef0b3af32b05") ||
        !asset_file_matches_md5(driver[0],"d86f045085481448f6ae3db8d3d640d2") ||
        !asset_file_matches_md5(driver[1],"d86f045085481448f6ae3db8d3d640d2")) goto done;
    puts("PASS: original A36 scoped/unscoped package ownership, orphan-first rescue, missing/wrong/neighbor driver, three original language payloads and missing default beside F/G");
    ok=1;
done:
    for (int i=0;i<2;++i) { remove(gfx[i]); remove(dun[i]); remove(driver[i]); }
    remove(driver[2]);
    remove(localized[0]); remove(localized[1]);
    (void)SDL_RemovePath(orphan); (void)SDL_RemovePath(complete);
    (void)SDL_RemovePath(neighbor); (void)SDL_RemovePath(root);
    return ok;
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA36_ARCHIVE");
    uint8_t *adf = NULL;
    uint8_t *mutated = NULL;
    size_t adf_bytes = 0u;
    SelectedMedia media;
    DM1_V1_AmigaEntranceF0441Receipt receipt;
    FILE *archive_file;
    int result = 1;

    if (!archive || !archive[0]) {
        archive = getenv("HOME");
        if (archive && archive[0]) {
            static char default_archive[1024];
            (void)snprintf(default_archive, sizeof(default_archive),
                           "%s/.firestaff/data/dm1/"
                           "Dungeon-Master_Amiga_36.zip", archive);
            archive = default_archive;
        }
    }
    archive_file = archive && archive[0] ? fopen(archive, "rb") : NULL;
    if (!archive_file) {
        puts("SKIP: authentic DM1 Amiga 3.6 archive is not staged");
        return 77;
    }
    fclose(archive_file);
    memset(&media, 0, sizeof(media));
    memset(&receipt, 0, sizeof(receipt));
    if (firestaff_zip_extract_by_suffix(
            archive, "Dungeon Master Amiga 3.6.adf", &adf,
            &adf_bytes) != 0 ||
        firestaff_amiga_adf_visit_ofs_files(adf, adf_bytes,
                                            selected_adf_member,
                                            &media) < 0 ||
        !media.graphics) {
        fprintf(stderr, "FAIL: could not read authentic A36 GRAPHICS.DAT\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga36_entrance_f0441_receipt(
            media.graphics, media.graphics_bytes, &receipt)) {
        fprintf(stderr, "FAIL: authentic A36 F0441 receipt rejected\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga36_entrance_bind_driver(&receipt,
            media.driver, media.driver_bytes) || !receipt.a36_fade_valid ||
        strcmp(receipt.a36_driver_md5, "d86f045085481448f6ae3db8d3d640d2")) {
        fputs("FAIL: original A36 DRIA palette driver rejected\n", stderr);
        goto cleanup;
    }
    {
        const uint16_t blank[16] = {0};
        uint16_t palette[16];
        for (unsigned int frame = 1; frame <= 15; ++frame) {
            if (!dm1_v1_amiga36_entrance_fade_step(&receipt, blank,
                    receipt.entrance_palette_rgb4, frame, palette)) goto cleanup;
            for (unsigned int color = 0; color < 16; ++color) {
                unsigned int expected = 0;
                for (unsigned int shift = 0; shift <= 8; shift += 4) {
                    unsigned int delta = (receipt.entrance_palette_rgb4[color] >> shift) & 15;
                    unsigned int component = 0;
                    for (unsigned int step = 0; step < frame; ++step)
                        component += receipt.a36_fade_increments[step][delta];
                    expected |= component << shift;
                }
                if (palette[color] != expected) goto cleanup;
            }
        }
        if (memcmp(palette, receipt.entrance_palette_rgb4, sizeof(palette))) goto cleanup;
        puts("ok: original VDEO.FTL DATA supplies all fifteen source fade steps");
    }
    if (strcmp(receipt.graphics_md5,
               "7f9458e4a3972d06e649a6fa85a7f34b") != 0 ||
        receipt.executable_md5[0] != '\0' ||
        receipt.screen_width != 320u || receipt.screen_height != 200u ||
        receipt.left_door_width != 105u ||
        receipt.left_door_height != 161u ||
        receipt.right_door_width != 128u ||
        receipt.right_door_height != 161u ||
        receipt.screen_nonzero_pixels == 0u ||
        receipt.left_door_nonzero_pixels == 0u ||
        receipt.right_door_nonzero_pixels == 0u ||
        receipt.door_frame_count != 2u || receipt.opening_steps != 64u ||
        !receipt.a36_layout_valid || receipt.a36_step_pixels != 2u ||
        receipt.switch_delay_ticks != 20u || receipt.mouse_input_only ||
        !receipt.has_credits_graphic ||
        !receipt.has_entrance_buttons_graphic ||
        receipt.entrance_palette_rgb4[5] != 0x0c0u ||
        receipt.entrance_palette_rgb4[15] != 0xfffu ||
        !receipt.source_evidence ||
        strstr(receipt.source_evidence, "STARTUP2.C") == NULL ||
        strstr(receipt.source_evidence, "COMMAND.C") == NULL) {
        fprintf(stderr, "FAIL: A36 entrance receipt facts mismatch\n");
        goto cleanup;
    }
    {
        M11_AssetLoader loader = {0};
        uint8_t micro[224 * 136], page[320 * 200];
        const M11_AssetSlot *screen, *left, *right, *buttons;
        const DM1_V1_AmigaEntranceRect *rects[] = {
            &receipt.a36_transfer, &receipt.a36_working,
            &receipt.a36_left, &receipt.a36_right, &receipt.a36_viewport,
            &receipt.a36_enter_button, &receipt.a36_resume_button
        };
        /* Independent original A36 C696 records, decoded from this archive:
         * 428 is240x161;430 left105x161;431 type12 offset-8 gives104;
         * 427 transfers at0,30;432 inserts the224x136 viewport at0,3. */
        static const int expected[7][4] = {
            {0,30,240,161}, {0,0,240,161}, {0,0,105,161},
            {104,0,128,161}, {0,3,224,136}, {243,46,14,13}, {243,78,14,13}
        };
        for (int i = 0; i < 7; ++i) {
            if (rects[i]->x != expected[i][0] || rects[i]->y != expected[i][1] ||
                rects[i]->w != expected[i][2] || rects[i]->h != expected[i][3]) {
                fputs("FAIL: original A36 C696 zone mismatch\n", stderr);
                goto cleanup;
            }
        }
        if (!M11_AssetLoader_InitFromBuffer(&loader, media.graphics,
                                            (long)media.graphics_bytes)) goto cleanup;
        screen = M11_AssetLoader_Load(&loader, 4);
        left = M11_AssetLoader_Load(&loader, 2);
        right = M11_AssetLoader_Load(&loader, 3);
        buttons = M11_AssetLoader_Load(&loader, 11);
        if (!screen || !left || !right || !buttons ||
            !dm1_v1_amiga36_entrance_render_c255(&loader, micro, sizeof(micro))) {
            M11_AssetLoader_Shutdown(&loader); goto cleanup;
        }
        {
            const M11_AssetSlot *floor = M11_AssetLoader_Load(&loader, 78);
            const M11_AssetSlot *ceiling = M11_AssetLoader_Load(&loader, 79);
            const M11_AssetSlot *wall_left = M11_AssetLoader_Load(&loader, 96);
            const M11_AssetSlot *wall_right = M11_AssetLoader_Load(&loader, 95);
            if (!floor || !ceiling || !wall_left || !wall_right ||
                floor->width != 224 || floor->height != 97 ||
                ceiling->width != 224 || ceiling->height != 39 ||
                wall_left->width != 60 || wall_left->height != 111 ||
                wall_right->width != 60 || wall_right->height != 111) {
                M11_AssetLoader_Shutdown(&loader); goto cleanup;
            }
            /* Independent F0797 map proof: at South(2,0), both D1 side
             * squares are walls, while the center stays corridor. C713/714
             * place their native 60x111 panels at(0,9)/(164,9). C716/717
             * cover only x0..32/x191..223. The exposed strips below must
             * therefore retain the actual original96/95 pixels, and the
             * unobstructed center must retain native79/78 ceiling/floor.
             * This checks source-visible crops, not emulator frame parity. */
            for (int y = 0; y < 136; ++y) {
                for (int x = 0; x < 224; ++x) {
                    int expected = -1;
                    if (y < 9 && x >= 33 && x < 191)
                        expected = ceiling->pixels[y * 224 + x];
                    if (y >= 120 && x >= 60 && x < 164)
                        expected = floor->pixels[(y - 39) * 224 + x];
                    if (y >= 9 && y < 120 && x >= 33 && x < 60)
                        expected = wall_left->pixels[(y - 9) * 60 + x];
                    if (y >= 9 && y < 120 && x >= 164 && x < 191)
                        expected = wall_right->pixels[(y - 9) * 60 + x - 164];
                    if (expected >= 0 && micro[y * 224 + x] != expected) {
                        fprintf(stderr, "FAIL: source C255 crop at%d,%d got%u expected%d\n",
                            x, y, micro[y * 224 + x], expected);
                        M11_AssetLoader_Shutdown(&loader); goto cleanup;
                    }
                }
            }
            puts("ok: source F0797 visible D1 walls, ceiling and floor match original native pixels");
        }
        for (unsigned int frame = 0; frame <= 64; ++frame) {
            if (!dm1_v1_amiga36_entrance_compose(&loader, &receipt, micro,
                    frame, 0, page, sizeof(page))) {
                M11_AssetLoader_Shutdown(&loader); goto cleanup;
            }
            /* Pixel oracle uses original image arrays and literal authored
             * source coordinates, independently of production rectangle math. */
            for (int y = 0; y < 200; ++y) {
                for (int x = 0; x < 320; ++x) {
                    uint8_t pixel = screen->pixels[y * 320 + x];
                    if (y >= 30 && y < 191 && x < 240) {
                        int ly = y - 30, shift = (int)frame * 2;
                        if (frame && x < 224 && ly >= 3 && ly < 139)
                            pixel = micro[(ly - 3) * 224 + x];
                        if (x < 105 - shift)
                            pixel = left->pixels[ly * 105 + x + shift];
                        if (x >= 104 + shift && x < 232)
                            pixel = right->pixels[ly * 128 + x - 104 - shift];
                    }
                    if (x >= 243 && x < 257 && y >= 46 && y < 59)
                        pixel = buttons->pixels[(y - 46) * buttons->width + x - 243];
                    if (page[y * 320 + x] != pixel) {
                        fprintf(stderr, "FAIL: A36 frame%u pixel%d,%d\n", frame, x, y);
                        M11_AssetLoader_Shutdown(&loader); goto cleanup;
                    }
                }
            }
        }
        M11_AssetLoader_Shutdown(&loader);
        puts("ok: all 65 original A36 closed/opening pages match native pixel crop oracle");
    }
    mutated = (uint8_t *)malloc(media.graphics_bytes);
    if (!mutated) {
        fprintf(stderr, "FAIL: allocate mutation buffer\n");
        goto cleanup;
    }
    memcpy(mutated, media.graphics, media.graphics_bytes);
    mutated[media.graphics_bytes - 1u] ^= 1u;
    if (dm1_v1_amiga36_entrance_f0441_receipt(
            mutated, media.graphics_bytes, &receipt)) {
        fprintf(stderr, "FAIL: accepted modified A36 GRAPHICS.DAT\n");
        goto cleanup;
    }
    if (!check_original_package_ownership(&media)) goto cleanup;
    puts("ok: A36 F0441 receipt authenticates original C002/C003/C004/C005/C011 media and keyboard input");
    result = 0;

cleanup:
    free(mutated);
    free(media.dungeon_f);
    free(media.dungeon_g);
    free(media.dungeon);
    free(media.driver);
    free(media.graphics);
    free(adf);
    return result;
}

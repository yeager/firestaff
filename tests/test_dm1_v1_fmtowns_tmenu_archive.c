#include "dm1_v1_fmtowns_tmenu_input.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"

/* This is an original-media input receipt.  Its archive reads and byte
 * checks must remain live when the test is compiled in Release. */
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    uint8_t *cue = NULL, *bin = NULL, *tmenu = NULL;
    size_t cue_size = 0u, bin_size = 0u, tmenu_size = 0u;
    char image_member[256];
    FmtownsDiscProbeResult probe;
    const FmtownsIsoEntry *entry;
    size_t load = DM1_V1_FMTOWNS_TMENU_LOAD_IMAGE_OFFSET;
    if (!archive || !archive[0]) { puts("SKIP: FM Towns archive is not configured"); return 0; }
    assert(firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) == 0 && cue);
    assert(fmtowns_cue_parse_image_member((const char *)cue, cue_size, image_member, sizeof(image_member)) == 1);
    assert(firestaff_zip_extract_by_suffix(archive, image_member, &bin, &bin_size) == 0 && bin);
    free(cue);
    assert(fmtowns_disc_probe(bin, bin_size, FMTOWNS_SECTOR_2048, &probe) == 0 && probe.valid);
    entry = fmtowns_disc_find(&probe, "TMENU.EXP");
    assert(entry && fmtowns_disc_extract_alloc(bin, bin_size, FMTOWNS_SECTOR_2048, entry, &tmenu, &tmenu_size) == 0);
    free(bin);
    assert(tmenu && tmenu_size > load + DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR + 3u);
    assert(tmenu[0] == 'P' && tmenu[1] == '3' && tmenu[4] == 0x80 && tmenu[5] == 0x01);
    assert(tmenu[load + DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR] == 0xc8 && tmenu[load + DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR + 1u] == 0 && tmenu[load + DM1_V1_FMTOWNS_TMENU_POLL_MAIN_VADDR + 2u] == 0);
    assert(tmenu[load + DM1_V1_FMTOWNS_TMENU_INIT_EIP_VADDR] == 0xe9 && tmenu[load + DM1_V1_FMTOWNS_TMENU_INIT_EIP_VADDR + 1u] == 3);
    assert(tmenu[load + DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR] == 0x66 && tmenu[load + DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR + 1u] == 0x53 && tmenu[load + DM1_V1_FMTOWNS_TMENU_TBIOS_POLL_VADDR + 2u] == 0x1e);
    free(tmenu);
    puts("PASS: retail FM Towns TMENU input entries read from ZIP/BIN in RAM");
    return 0;
}

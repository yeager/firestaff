#include "theron_v1_mednafen_transition_receipt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
#include <unistd.h>
#endif

static const char *fixture =
    "source=authentic-mednafen-transition-receipt\n"
    "mednafen_module=pce\n"
    "track02_mode=MODE1/2352\n"
    "track02_md5=f23601102138f87c33025877767ebf76\n"
    "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
    "input_transactions=1\ncd_irq_callbacks=1\nraw_sector_spans=1\n"
    "scsi_read_commands=1\nscsi_read_sector_bindings=1\n"
    "byte_exact_origin_ram_receipts=1\nauthenticated_cd_ram_receipts=1\n"
    "game_main_ram_e009_dispatches=1\nmain_ram_consumer_reads=1\n"
    "main_ram_target_reads=0\nmain_ram_target_writes=0\n"
    "spawn_consumer_reads=0\nspawn_entry_b0e5_samples=0\n"
    "rng_consumer_samples=0\nvdc_vram_snapshot_bytes=65536\n"
    "vce_palette_snapshot_bytes=1024\ntransition=observed\n";

static const char *iso_fixture =
    "source=authentic-mednafen-transition-receipt\n"
    "mednafen_module=pce\n"
    "track02_mode=MODE1/2048\n"
    "track02_md5=ceb02343868f80cec899e9b239aff2da\n"
    "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
    "input_transactions=1\ncd_irq_callbacks=1\nraw_sector_spans=1\n"
    "scsi_read_commands=1\nscsi_read_sector_bindings=1\n"
    "byte_exact_origin_ram_receipts=1\nauthenticated_cd_ram_receipts=1\n"
    "game_main_ram_e009_dispatches=1\nmain_ram_consumer_reads=1\n"
    "main_ram_target_reads=0\nmain_ram_target_writes=0\n"
    "spawn_consumer_reads=0\nspawn_entry_b0e5_samples=0\n"
    "rng_consumer_samples=0\nvdc_vram_snapshot_bytes=65536\n"
    "vce_palette_snapshot_bytes=1024\ntransition=observed\n";

int main(void) {
#if defined(_WIN32)
    puts("SKIP: POSIX temporary receipt fixture");
    return 77;
#else
    char path[512];
    const char *tmpdir = getenv("TMPDIR");
    Theron_V1MednafenTransitionReceipt receipt;
    if (!tmpdir || !tmpdir[0]) tmpdir = "/tmp";
    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-transition-XXXXXX",
                    tmpdir) > 0);
    int fd = mkstemp(path);
    FILE *file;
    assert(fd >= 0);
    file = fdopen(fd, "wb");
    assert(file);
    assert(fputs(fixture, file) >= 0);
    assert(fclose(file) == 0);
    assert(theron_v1_mednafen_transition_receipt_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_MEDNAFEN_TRANSITION_READY);
    assert(receipt.transport_verified && !receipt.semantic_publication_allowed);
    assert(receipt.authenticated_cd_ram_receipts == 1u);
    assert(receipt.main_ram_consumer_reads == 1u);
    assert(receipt.main_ram_target_reads == 0u);
    unlink(path);

    assert(snprintf(path, sizeof(path), "%s/firestaff-theron-transition-XXXXXX",
                    tmpdir) > 0);
    fd = mkstemp(path);
    assert(fd >= 0);
    file = fdopen(fd, "wb");
    assert(file);
    assert(fputs(iso_fixture, file) >= 0);
    assert(fclose(file) == 0);
    assert(theron_v1_mednafen_transition_receipt_parse_file(path, &receipt));
    assert(receipt.status == THERON_V1_MEDNAFEN_TRANSITION_READY);
    assert(receipt.transport_verified && !receipt.semantic_publication_allowed);
    assert(strcmp(receipt.track02_md5,
                  "ceb02343868f80cec899e9b239aff2da") == 0);
    unlink(path);

    {
        const char *real_path = getenv("THERON_MEDNAFEN_TRANSITION_RECEIPT");
        if (real_path && real_path[0]) {
            assert(theron_v1_mednafen_transition_receipt_parse_file(
                real_path, &receipt));
            assert(receipt.status == THERON_V1_MEDNAFEN_TRANSITION_READY);
            assert(receipt.transport_verified);
            assert(!receipt.semantic_publication_allowed);
            assert(receipt.authenticated_cd_ram_receipts == 2u);
            assert(receipt.main_ram_consumer_reads == 65536u);
            assert(receipt.main_ram_target_reads == 512u);
            assert(receipt.main_ram_target_writes == 3584u);
            assert(receipt.spawn_consumer_reads == 4096u);
            assert(receipt.spawn_entry_b0e5_samples == 0u);
            assert(receipt.rng_consumer_samples == 3584u);
        }
    }
    puts("PASS: authenticated transition transport admitted; semantics blocked");
    return 0;
#endif
}

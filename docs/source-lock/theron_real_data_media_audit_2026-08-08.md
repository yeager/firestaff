# Theron's Quest Real-Media Audit (2026-08-08)

The authenticated Track 02 intake uses the real files in
`~/.firestaff/data/theron/` when available:

- US: `TQUS02.bin`, MD5 `f23601102138f87c33025877767ebf76`
- JP: `TQJP02.bin`, MD5 `b7afb338ad31be1025b53f9aff12d73a`

The audit confirms the following:

- US and JP roster bytes can be read from the original codon stream.
- Regional 4bpp palette candidates decode as real HuC6260 words. US and JP
  offsets are kept separate.
- Track 02 startup bitmap candidates can decompress to real indexed bytes, but
  their VDC destination, palette owner, and semantic startup routes are not
  bound by a runtime capture.
- The static VCE consumer span at `$96a5` is verified, but its dynamic source
  (`$27c4/$27c5`) has not yet been joined to the palette window or a VDC screen.
- The production gate therefore keeps `startup_presentation_allowed` and
  palette promotion disabled. This is deliberate: real bytes must not be
  turned into invented game meaning.
- No authenticated `vram_dungeon.bin`/`vce_dungeon.bin` snapshots exist in the
  shared data or workspace root. The first Theron image therefore cannot yet
  be published in the README.

Verification:

```text
theron_v1_vram_trace_loader                 PASS
theron_v1_track02_palette_window            PASS
theron_v1_startup_media_palette_bind       PASS
```

The next source-bound step is a run with the original System Card,
instrumented Mednafen, and the same Track 02 media. The capture receipt must
bind FIFO/RAM, the VCE/VDC destination, and the screen route before production
may display the image.

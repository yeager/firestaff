# Theron's Quest — executed main-RAM consumer window

The real Mednafen consumer sidecar
`/tmp/tq-vram.trace.main-ram-consumer` contains a contiguous code-fetch window
at HuC6280 `$2c54–$2c69`. Its file identity during capture was
`9d19ad9b993f1853e868f381756eb1d0` (539,613 bytes). Each byte below was read
with `logical_address == reader_pc` and `physical_address == reader_physical_pc`
in the `$1f` game bank; the regression verifies those execution coordinates,
not merely a matching data sequence.

```asm
; 2c54–2c69, bytes captured from game-owned main RAM
        lda     $3008
        clc
        adc     $fff5
        tam     #$04
        lda     $3009
        clc
        adc     $fff5
        tam     #$08
        lda     $300a
        clc
```

The byte sequence is:

```text
ad 08 30 18 6d f5 ff 53 04 ad 09 30 18 6d f5 ff
53 08 ad 0a 30 18
```

This is stronger than the earlier opaque read count: it proves an executed
HuC6280 instruction window after code has been loaded into main RAM. It still
does not identify the window as a dungeon record, object table, tile bank,
palette or HUD route. The source-LBA/FIFO join and the subsequent consumer
entry remain required before any semantic runtime promotion.

; Runtime-fetched System Card 3.0 code from the authentic US capture.
; This is a bounded disassembly excerpt, not a BIOS image.
;
; System Card identity: MD5 ff1a674273fe3540ccef576376407d1d, 262656 bytes.
; The fetched $F423-$F43C bytes map to BIOS file offset $1623 after HuC_LoadCD
; skips its 512-byte header. Their SHA-256 is
; c3a3bfda4d655483489787bc79aacf282d3cfd2957e11a899bdcb28111516820.
; Runtime MPR7=$00 maps logical $F423 to physical $01423.
; Disassembler: MAME unidasm -arch h6280 on runtime-fetch bytes captured on TRV2.
;
; The caller reads a byte through $F3FE, stores it through ($FA), advances the
; 16-bit pointer, and decrements the 16-bit count at $F8/$F9. Runtime trace
; pairs the $180A reads at $F403 with the subsequent $F425 stores byte for byte.
; Mednafen 1.32.1 source identifies $180A as ADPCM.ReadBuffer:
;   src/pce_fast/ioread.inc, CDROM read dispatch, lines 107-124
;   src/pce_fast/pcecd.cpp, PCECD_Read(), lines 385, 461-478
; That establishes the buffer-to-RAM path, not the source LBA or gameplay use.

f3fe: ad 0c 18    lda $180C
f401: 30 00       bmi $F403
f403: ad 0a 18    lda $180A
f406: 60          rts

f423: 44 d9       bsr $F3FE
f425: 92 fa       sta ($FA)
f427: e6 fa       inc $FA
f429: d0 02       bne $F42D
f42b: e6 fb       inc $FB
f42d: a5 f8       lda $F8
f42f: d0 02       bne $F433
f431: c6 f9       dec $F9
f433: c6 f8       dec $F8
f435: a5 f8       lda $F8
f437: 05 f9       ora $F9
f439: d0 e8       bne $F423
f43b: 62          cla
f43c: 60          rts

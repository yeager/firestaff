; Theron's Quest US Track 02 — authenticated regular-spawn helper contract
;
; Raw source: retail US Track 02 BIN, MD5
;   f23601102138f87c33025877767ebf76
; File offset: $9c4e7 (the same 25-byte span occurs in each dungeon-bank copy)
; HuC6280 entry: $4667
; FNV-1a: $b9075b31
;
; This entry is source evidence, not a host-side RNG implementation.  It
; consumes $B3, dispatches the special $B3&7 == 4 path through $5D6A and
; $5D64, and returns.  The $5D6A/$5D64 bodies are RAM-loaded/overlay-owned
; consumers and their runtime state still require a dynamic capture.
;
; Adjacent preconsumer L4644 is locked separately at raw file offset $9c4c4,
; 27 bytes, FNV-1a $a3c3f7eb. It prepares the arguments and calls the
; consumers at $C96B/$CC4C; those bodies remain unresolved.

        .setcpu "huc6280"
        .org    $4667

L4667:  lda     $B3
        and     #$07
        cmp     #$04
        bne     L4680
        jsr     L5D6A
        lda     #$02
        sta     $8A
        lda     #$04
        ldx     $40
        ldy     $41
        jsr     L5D64
        rts

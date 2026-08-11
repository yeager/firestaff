; Theron's Quest US Track 02 — authenticated record-consumer caller window
;
; Source image: TQUS02.bin, MODE1/2352 raw BIN
; Source MD5: f23601102138f87c33025877767ebf76
; Raw file offset: $9c450, 150 bytes
; HuC6280 execution address: $c3a0
; FNV-1a: $666ded61
;
; This is a source-backed caller window discovered on the external disk and
; matched byte-for-byte against the operator's real US BIN. It selects a
; source table through $ab/$ac, consumes $4667 for one branch, calls the
; source routines at $c000, $c96b and $cc4c, and reads/writes the $2998/$299c
; table. The window does not identify that table as creature stats, an object,
; a generator or a T700/T900 record. It is therefore admitted as disassembly
; provenance only; no host-side semantics are inferred from it.

        .setcpu "huc6280"
        .org    $c3a0

LC3A0:  cmp     #$01
        bne     LC3AF
        lda     #$87
        sta     $ab
        lda     #$ff
        sta     $ac
        jmp     LC166

LC3AF:  cmp     #$02
        bne     LC3BE
        lda     #$83
        sta     $ab
        lda     #$ff
        sta     $ac
        jmp     LC166

LC3BE:  lda     #$80
        sta     $ab
        lda     #$ff
        sta     $ac
        jsr     L4667
        and     #$7f
        adc     #$63
        bra     LC3F7
        lda     #$82
        sta     $ab
        lda     #$ff
        sta     $ac
        lda     #$b4
        bra     LC3F7
        lda     #$83
        sta     $ab
        lda     #$ff
        sta     $ac
        lda     #$96
        bra     LC3ED
        lda     #$fa
LC3ED:  pha
        lda     #$80
        sta     $ab
        lda     #$ff
        sta     $ac
        pla
LC3F7:  sta     $a9
        jsr     LC000
        lda     $2998,x
        cmp     $ad
        lda     $299c,x
        sbc     $ae
        bcs     LC416
        lda     $a9
        lsr     a
        cmp     #$02
        bcs     LC411
        lda     #$02
LC411:  lda     $2998,x
        sta     $ad
LC416:  lda     $a9
        sta     $8a
        lda     $ad
        sta     $8b
        lda     $ab
        ldy     $ac
        jsr     LC96B
        bne     LC429
        lsr     $b4
LC429:  ldx     $bb
        jsr     LCC4C
        rts

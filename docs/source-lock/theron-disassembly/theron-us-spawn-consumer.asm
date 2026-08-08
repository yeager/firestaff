; Theron's Quest US Track 02 — authenticated regular-spawn consumer
;
; Raw source: retail US Track 02 BIN supplied by the operator.
; Source image: 2352-byte sectors, raw file offset $9b195.
; HuC6280 execution address: $b0e5.
; This span is repeated in the dungeon-bank images. It is retained as a
; source lock, not as a host-side implementation or a guessed stat table.
;
; Important ownership facts visible in this body:
;   - category dispatch is the value in A (0..3 after the caller's mapping)
;   - category 1 calls the source multiplier helper with #$15
;   - category 2 calls it with #$19 and applies (value + 1) / 2
;   - category 3 divides the 16-bit source value by 32 and applies 1.5x
;   - the common path consumes L4667 twice and clamps HP to $0384
;   - L4644/L4667 are external RNG consumers; this body alone does not prove
;     their algorithm or the later combat/drop owners.

        .setcpu  "huc6280"
        .org     $b0dd

LB0DD:   lsr     $b5
        ror     $b4
        dey
        bne     LB0DD
        rts

LB0E5:   cmp     #$01
        bne     LB118
        lda     #$15
        bsr     LB10A
        asl     $b8
        lda     #$01
        ldy     $b1
        bsr     LB0FE
        lda     #$02
        ldy     $b0
        bsr     LB0FE
        jmp     LD19D

LB0FE:   phy
        jsr     L5B8F
        pla
        adc     $2a10,y
        sta     $2a10,y
        rts

LB10A:   ldx     $b4
        ldy     $b5
        jsr     L5A76
        stx     $b4
        sty     $b5
        ldx     $bb
        rts

LB118:   cmp     #$03
        bne     LB13C
        ldy     #$05
        bsr     LB0DD
        lda     $b8
        lsr     a
        clc
        adc     $b8
        adc     $29a0,x
        sta     $29a0,x
        bcc     LB131
        inc     $29a4,x
LB131:   lda     #$03
        ldy     $b0
        bsr     LB0FE
        bra     LB161
        jmp     LD19D

LB13C:   cmp     #$02
        bne     LB19D
        lda     #$19
        bsr     LB10A
        lda     $b8
        clc
        adc     $29a0,x
        sta     $29a0,x
        bcc     LB152
        inc     $29a4,x
LB152:   lda     $b8
        inc     a
        lsr     a
        clc
        adc     $b8
        sta     $b8
        lda     #$03
        ldy     $b0
        bsr     LB0FE
LB161:   jsr     L4667
        and     #$03
        sta     $8a
        lda     $b6
        dec     a
        cmp     $8a
        bcs     LB171
        lda     $8a
LB171:   clc
        adc     $29a0,x
        sta     $29a0,x
        bcc     LB17D
        inc     $29a4,x
LB17D:   lda     #$84
        cmp     $29a0,x
        lda     #$03
        sbc     $29a4,x
        bcs     LB193
        lda     #$84
        sta     $29a0,x
        lda     #$03
        sta     $29a4,x
LB193:   jsr     L4667
        and     #$03
        ldy     #$05
        jsr     LD0FE
LB19D:   lda     $b8
        jsr     LD23A
        clc
        adc     $2980,x
        sta     $2980,x
        bcc     LB1AE
        inc     $2984,x
LB1AE:   lda     #$e7
        cmp     $2980,x
        lda     #$03
        sbc     $2984,x
        bcs     LB1C4
        lda     #$e7
        sta     $2980,x
        lda     #$03
        sta     $2984,x
LB1C4:   lda     $b4
        jsr     LD23A
        clc
        adc     $2990,x
        sta     $2990,x
        bcc     LB1D5
        inc     $2994,x
LB1D5:   lda     #$0f
        cmp     $2990,x
        lda     #$27
        sbc     $2994,x
        bcs     LB1EB
        lda     #$0f
        sta     $2990,x
        lda     #$27
        sta     $2994,x
LB1EB:   ldx     $bb
        lda     #$02
        jsr     L5BA5

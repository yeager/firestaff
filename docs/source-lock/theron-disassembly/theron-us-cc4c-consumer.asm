; Theron's Quest US Track 02 — source-bound HuC6280 window
;
; Source: authentic TQUS02.bin, MD5 f23601102138f87c33025877767ebf76
; Raw file offset: $A4ACC; length: 200 bytes; FNV-1a: $4AD0801E
; CPU decode base: $CC4C
; Decoder: MAME unidasm -arch h6280; three repeated outputs matched
; SHA-256: 7674bf8f304d44163d39c367765795641bb9243242f55422d15d062eeb5b8696
;
; This is a linear decode of the byte-locked source window, not proof that
; all rows execute in one runtime bank. The window includes an RTS at $CC6A,
; then continues at $CC6B; the latter bytes are retained as a separate decode
; region, not assumed reachable by fall-through. The 200-byte receipt ends at
; $CD13 on PLY, not at an RTS. MAME renders the entry byte $BB at $CC4C as
; `ill`; no runtime opcode semantics or gameplay meaning are assigned here.
;
; Direct targets computed from decoded branch/call bytes are shown by MAME.
; This listing adds no field, RNG, spawn, AI, loot, or T700/T900 semantics.

        .setcpu "huc6280"
        .org    $cc4c

LCC4C:  ill     $BB
        ldy     #$02
        plx
        lda     $2974,x
        and     #$20
        beq     $CC5D
        tya
        clc
        adc     $BB
        sta     $BB
LCC5D:  lda     #$05
        jsr     $5B64
        cmp     #$FA
        bne     $CC68
        dec     $BB
LCC68:  lda     $BB
        rts

LCC6B:  jsr     $4540
        sxy
        phy
        lda     $29F8,y
        stz     $BB
        asl     a
        rol     $BB
        asl     a
        rol     $BB
        asl     a
        rol     $BB
        clc
        adc     #$64
        sta     $BA
        bcc     $CC87
        inc     $BB
LCC87:  tii     $20BA,$208A,$0002
        jsr     $45E3
        dex
        eor     ($73)
        txa
        jsr     $20BA
        sxy
        brk
        lda     $2974,y
        and     #$7F
        beq     $CCC5
        tii     $20BA,$208C,$0002
        lsr     $8D
        ror     $8C
        lsr     $8D
        ror     $8C
        and     #$10
        bne     $CCB8
        lsr     $8D
        ror     $8C
LCCB8:  sec
        lda     $BA
        sbc     $8C
        sta     $BA
        lda     $BB
        sbc     $8D
        sta     $BB
LCCC5:  tya
        tax
        lda     #$05
        jsr     $5B64
        cmp     #$FA
        bne     $CCF4
        tii     $20BA,$208C,$0002
        lsr     $8D
        ror     $8C
        lsr     $8D
        ror     $8C
        lsr     $8D
        ror     $8C
        lsr     $8D
        ror     $8C
        clc
        lda     $BA
        adc     $8C
        sta     $BA
        lda     $BB
        adc     $8D
        sta     $BB
LCCF4:  clc
        lda     $BA
        adc     #$09
        sta     $BA
        bcc     $CCFF
        inc     $BB
LCCFF:  ldx     $BA
        ldy     $BB
        lda     #$0A
        jsr     $5A76
        sec
        lda     $BA
        sbc     $0B
        sta     $BA
        bcs     $CD13
        dec     $BB
LCD13:  ply

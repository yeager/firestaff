; Theron's Quest US Track 02 — exact $C96B source span
;
; Source image: raw 2352-byte Track 02 BIN, MD5
; f23601102138f87c33025877767ebf76.
; Raw offset: $A47EB.  HuC6280 address: $C96B.  Length: 255 bytes.
; FNV-1a: 0xE689C658.  Disassembled with da65 --cpu huc6280.
;
; This is retained as source-lock evidence only.  $3A is a caller-provided
; pointer; this body does not identify the pointed record as a creature,
; object, generator or T700/T900 state.  The writes through ($3A) and the
; $2040->$2045 copy are observed operations, not semantic labels.

        pha
        jsr     L45E3
        inc     $686D
        cmp     $DB
        bne     LC99B
        lda     $2E03
        bne     LC990
        stz     $2E03
        lda     $2DF8
        bne     LC990
        lda     $2DF5
        sta     $36
        lda     $2DF6
        sta     $37
        jsr     L5C75
LC990:  lda     $2E05
        bne     LC998
        stz     $2E05
LC998:  jsr     L5DC9
LC99B:  ldx     $BB
        jsr     LD3CF
        lda     #$0A
        ldx     #$80
        jsr     L45E3
        and     $CE
        beq     LC9DE
        jsr     L5147
        ldy     #$02
        lda     #$85
        sta     ($3A),y
        ldy     #$03
        lda     ($3A),y
        and     #$3F
        sta     $BA
        lda     $BB
        lsr     a
        ror     a
        ror     a
        and     #$C0
        ora     $BA
        sta     ($3A),y
        ldx     $BB
        lda     $2948,x
        sta     $BA
        lda     $2948,x
        jsr     L511F
        tii     $2040,$2045,$0002
        jsr     L5D78
LC9DE:  ldx     $BB
        cla
        sta     $2958,x
        sta     $2CFC,x
        lda     $3F
        sta     $2944,x
        cla
        sta     $2960,x
        lda     $BA
        sec
        sbc     $3F
        and     #$03
        sta     $BA
        lda     $2964,x
        beq     LCA01
        jsr     LD597
LCA01:  ldx     #$FF
        lda     $3F
        ldy     $BA
        jsr     L45E3
        ldx     $08,y
        ldx     $BB
        jsr     L5D52
        clx
LCA12:  cpx     $DA
        bcs     LCA21
        lda     $2978,x
        ora     $297C,x
        bne     LCA2B
        inx
        bra     LCA12
LCA21:  lda     #$01
        sta     $2DFD
        jsr     L45E3
        ora     ($BE)
LCA2B:  lda     $BB
        cmp     $2DF4
        bne     LCA39
        phx
        jsr     L45E3
        sxy
        .byte   $AF
        plx
LCA39:  lda     $BB
        cmp     $2E3A
        bne     LCA47
        jsr     L45E3
        csh
        rmb6    $80
        php
LCA47:  ldx     $2E3A
        jsr     L45E3
        asl     $6067,x
        jsr     L4540
        st0     #$20
        lda     $45
        stx     $BB
        sta     $B9
        sty     $BA
        sty     $8B
        cpx     #$FF
        beq     LCA69
        inx
        cpx     $2DFA
        bne     LCA6A
LCA69:  rts

; The da65 output preserves bytes whose entry/callee bank ownership is not
; resolved by this raw span.  No host-side consumer may assign them meaning.

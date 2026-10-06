; Theron's Quest JP Rev. 1, Track 02 stage-two record-selector path.
; Disassembled from authentic TQJP02.bin with MAME unidasm -arch h6280.
; JP Track 02 SHA-256:
;   d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; US Track 02 SHA-256:
;   f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565
; Stage-two user data: 17 sectors, 34,816 bytes.
; JP payload SHA-256:
;   bc9ff3922dad71f2cd24afeaaf09747c64db15245eb7f9c454dbb76bd4e12d67
; US payload SHA-256:
;   71c52160a515657680dce3b7c09a19189d62c76806b3d627272c90c1f0a6c51f
; JP sectors 1223..1239; US sectors 1224..1240. Raw-sector headers omitted.
; The byte spans below are locked against JP original media by
; test_stage2_jp_record_selector_52xx_flow(). They are not a runtime trace.
;
; Key span SHA-256 values (half-open CPU address ranges):
;   $5800..$5816  6855102db743a30a09bebf074a46e03c3de4b210039ab3ac211a89945e2f75a1
;   $582c..$5895  693f27dc1371ba8df0636bf7c41e5d1233e739c7fb81d67f8642e1af452d46a3
;   $5895..$58dc  a661d83f872690dc844f7a53e3e1e035f08631bb7514507008268916baa8cd9b
;   $4fea..$506c  2e4e1c78d1a8eddde3885a4341582b089314c7d44ef6faa1b025b5805c230743
;   $50f5..$5111  3018374912361e8a6bac67f252b565a21153ad1e2b0e1e0ae1f1384fcf8ef9f1
;   $525e..$52d9  861aef44aaae08750d3004196f66c72a932aae7159d60aa76e600dcaf8ff294c
;   $52d9..$53d8  6a192f3192797e4843ef4cc0e141ed0d96f2901f61eaa4d5ddbe34bd8e15c461
;   $5669..$5670  2ffc33a966b77db91fcca71812ac1fc7548eca3c00b6ecab571cc32c5c2816b7
;
; Static dispatch interpretation: $5800 reads a selector through ($18),
; doubles it, and jumps through $5810,X. Selector value 2 would use the
; pointer at $5814 ($5895). The observed runtime selector value is unknown.
; $52a2 is the high operand byte of the instruction at $52a0, not an
; instruction entry: $52a0 is JSR $567a, followed at $52a3 by JSR $5669.

$5800:  lda  ($18)
        bne  $5805
        rts
        asl  a
        tax
        lda  $18
        pha
        lda  $19
        pha
        jmp  ($5810,x)
$5810:  .byte $00,$00                 ; selector 0 -> null table entry
        .word $58eb                    ; selector 1
        .word $5895                    ; selector 2

; Selector 2 handler; the exact window continues through $58dc.
$5895:  ldy  #$01
        lda  ($18),y
        sta  $4f8b
        iny
        lda  ($18),y
        sta  $4f8c
        iny
        lda  ($18),y
        sta  $4f89
        iny
        lda  ($18),y
        sta  $4f8a
        iny
        lda  ($18),y
        sta  $4f8f
        iny
        lda  ($18),y
        sta  $4f98
        iny
        lda  ($18),y
        sta  $4f99
        iny
        lda  ($18),y
        sta  $4f9a
        iny
        lda  ($18),y
        sta  $4f9b
        stz  $4f96
        jsr  $5529
$58d2:  jsr  $4fea
        lda  #$0a
        sta  $0a
        jmp  $582c

; $4fea sets up the transfer and reaches $50f5 at $505a.
$4fea:  clc
        lda  $4f98
        adc  $4fd9
        sta  $00
        lda  $4f99
        adc  $4fda
        sta  $01
        lda  $4f89
        sta  $10
        lda  $4f8a
        sta  $11
        lda  $4f8b
        sta  $4f93
        lda  $4f8c
        sta  $4f94
        lda  $4f8d
        lsr  a
        sta  $04
        sta  $4f95
        lda  $4f96
        sta  $4f97
        stz  $50f4
        bsr  $4fdf
        jsr  $50b7
        bcc  $5023
        jsr  $5097
        bne  $5030
        rts
$5030:  jsr  $50a2
        bcc  $5023
        jsr  $50cc
        bcc  $5023
        jsr  $50df
        bcc  $5023
        lda  $50f4
        beq  $505a
        stz  $3b33
        jsr  $e063
        lda  $2228
        bne  $505a
        lda  $4fdd
        cmp  $3b33
        bcs  $5047
        jsr  $51e4
$505a:  jsr  $50f5
        bsr  $508b
        inc  $4f8b
        inc  $4f8b
        dec  $04
        beq  $506c
        jmp  $5023

$50f5:  lda  $00
        pha
        lda  $01
        pha
        lda  $04
        pha
        lda  $05
        pha
$5101:  jsr  $525e
        pla
        sta  $05
        pla
        sta  $04
        pla
        sta  $01
        pla
        sta  $00
        rts

; The two conditional arms are separate JP routines. $52a2 is data within
; the operand bytes of JSR $567a at $52a0, not an entry or call target.
$525e:  lda  $4fd8
        cmp  #$7f
        bcc  $526c
        bne  $526c
        lda  $4fd7
        cmp  #$c1
$526c:  bcs  $529a
        lda  $4fd7
        pha
        lda  $4fd8
        pha
        lda  $4f8f
        sta  $ff
        bsr  $52d9
        pla
        sta  $07
        pla
        sta  $06
        bsr  $52c6
        ldy  #$02
        dec  $5a
        lda  $525d
        bne  $5292
$528e:  bsr  $529b
        bra  $5297
$5292:  bsr  $52af
        stz  $525d
        stz  $5a
        clc
        rts
$529b:  jsr  $551a
        ldx  #$02
$52a0:  jsr  $567a
$52a3:  jsr  $5669
        dex
        bne  $52a0
        bsr  $5251
        dey
        bne  $529b
        rts
$52af:  jsr  $551a
        jsr  $567a
        clc
        lda  $06
        adc  #$02
        sta  $06
        bcc  $52c0
        inc  $07
$52c0:  bsr  $5251
        dey
        bne  $52af
        rts
$52c6:  jsr  $5237
        ldx  #$04
$52cb:  lsr  $07
        ror  $06
        dex
        bne  $52cb
        lda  $07
        ora  #$f0
        sta  $07
        rts

; The continuation $52d9..$53d8 (255 bytes) includes a BIOS call at
; $52fc: JSR $e060. Its internal BSR targets $5318, $533e, $5377, and
; $5354; $53c5 BSRs to the separately byte-bound $53d8 span.
$52d9:  lda  $4fd5
        sta  $fa
        lda  $4fd6
        sta  $fb
        lda  $ff
        beq  $52fc
        ldx  #$04
        cly
        cla
        sta  ($fa),y
        iny
        dex
        bne  $52eb
        clc
        lda  $fa
        adc  #$04
        sta  $fa
        bcc  $52fc
        inc  $fb
$52fc:  jsr  $e060
        cmp  #$00
        bne  $5353
        bsr  $5318
        ldx  #$03
        bsr  $533e
        clc
        lda  $4fd7
        adc  #$40
        sta  $4fd7
        bcc  $5317
        inc  $4fd8
        rts
$5318:  lda  $4fd7
        sta  $06
        lda  $4fd8
        sta  $07
        clc
        lda  $4fd5
        adc  #$20
        sta  $04
        cla
        adc  $4fd6
        sta  $05
        lda  #$10
        sta  $15
        ina
        sta  $14
        cla
        sta  $17
        ina
        sta  $16
        rts
$533e:  phx
        lda  $14,x
        clc
        adc  $4fd5
        sta  $00
        cla
        adc  $4fd6
        sta  $01
        bsr  $5377
        plx
        dex
        bpl  $533e
        rts
$5354:  lda  $11
        pha
        ldx  #$03
$5359:  phx
        lda  $14,x
        tay
        lsr  $11
        bcc  $5365
        lda  #$ff
        bra  $5366
$5365:  cla
$5366:  ldx  #$08
$5368:  sta  ($04),y
        iny
        iny
        dex
        bne  $5368
        plx
        dex
        bpl  $5359
        pla
        sta  $11
        rts
$5377:  lda  $10
        pha
$537a:  bsr  $5354
        ldx  #$03
$537e:  phx
        lsr  $10
        bcc  $5389
        lda  #$ff
        sta  $0e
        bra  $538b
$5389:  stz  $0e
$538b:  lda  $14,x
        tax
        cly
        lda  #$08
        pha
$5391:  lda  ($00),y
        iny
        iny
        sxy
        lsr  $0e
        bcc  $53a1
        ora  ($04),y
        sta  ($04),y
        bra  $53a7
$53a1:  eor  #$ff
        and  ($04),y
        sta  ($04),y
$53a7:  sxy
        inx
        inx
        pla
        dea
        bne  $5391
        plx
        dex
        bpl  $537e
        dec  $5a
        st0  #$00
        lda  $06
        sta  $0002
        lda  $07
        sta  $0003
        st0  #$02
        cly
        ldx  #$10
$53c5:  bsr  $53d8
        clc
        lda  $06
        adc  #$10
        sta  $06
        bcc  $53d2
        inc  $07
        stz  $5a
        pla
        sta  $10
        rts

$5669:  inc  $06
        bne  $566f
        inc  $07
$566f:  rts

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
;   $5800..$582c  9ce2ad3086b435b53f0ca75b12bece1c53db395cbcd65694dd34da4b9930b607
;   $582c..$5895  693f27dc1371ba8df0636bf7c41e5d1233e739c7fb81d67f8642e1af452d46a3
;   $5895..$58dc  a661d83f872690dc844f7a53e3e1e035f08631bb7514507008268916baa8cd9b
;   $4fea..$506c  2e4e1c78d1a8eddde3885a4341582b089314c7d44ef6faa1b025b5805c230743
;   $50f5..$5111  3018374912361e8a6bac67f252b565a21153ad1e2b0e1e0ae1f1384fcf8ef9f1
;   $525e..$52d9  861aef44aaae08750d3004196f66c72a932aae7159d60aa76e600dcaf8ff294c
;   $52d9..$53d8  6a192f3192797e4843ef4cc0e141ed0d96f2901f61eaa4d5ddbe34bd8e15c461
;   $5669..$5670  2ffc33a966b77db91fcca71812ac1fc7548eca3c00b6ecab571cc32c5c2816b7
;   $58dc..$5960  64c8cb6720887093e13d5961ee72485234b8785a116f57765b9c63ae2a0ef2d0
;   $5984..$5989  efd1f17c1409a7c73007486060fc81e2ce6fbf9ffe2832f4973b52db9eb87558
;   $5555..$559d  e5e2b548e880d3547db3bb7cf0ba6adc0993eba3107d960c0375cc5f28d43a63
;   $5966..$5984  1e7bf26c601ff09366c1a2e30cad225e075032d95b8ae3081e46cf36d8e06a6d
;   $5989..$599e  b7d0505079dbe30e91175174a9dabfcbc485e141b811a7f237550145cb32f760
;
; Authentic US same-address $5800..$582c SHA-256:
;   e1ada389530e8bab95341aa062b795a0c72631c34147d55408e0edbe4ee2268c
; Authentic US same-address comparison hashes:
;   $58dc..$5960  51850e3599f323999000202fc37916e93a654289d702e3f0d2ded9f63fd7abf9
;   $5984..$5989  84602c58f8c26368f82a98548e7a55969dd916399d0bb4c1add959b835b721da
;   $5555..$559d  c5c4b53b2ff2b89b3e98571d3322f6d0e8717aeab3fd2075ffc2a13d5ef8e105
;   $5966..$5984  a429462b6f9ea2d9e9fe05f7f639b39a2886f0620d72476f48a75e9cd4327754
;   $5989..$599e  511af50fd0b1654546b7aac5e6cdbb7153bf00edccf2fa49c5f4a688c70aeae6
; Static dispatch interpretation: $5800 reads a selector through ($18),
; doubles it, and jumps through $5810,X. The 14 pointer entries cover indices
; 0..13; selector value 2 uses the pointer at $5814 ($5895). The observed
; runtime selector value is unknown.
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
$5810:  .word $0000                    ; selector 0 -> null table entry
        .word $58eb                    ; selector 1
        .word $5895                    ; selector 2
        .word $58f9                    ; selector 3
        .word $5906                    ; selector 4
        .word $58e6                    ; selector 5
        .word $58dc                    ; selector 6
        .word $58dc                    ; selector 7
        .word $590b                    ; selector 8
        .word $594c                    ; selector 9
        .word $5915                    ; selector 10
        .word $5946                    ; selector 11
        .word $583f                    ; selector 12
        .word $586b                    ; selector 13

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

; Remaining selector-entry bodies; external helpers are targets, not included
; in these windows. $58dc is shared by selector indices 6 and 7.
$58dc:  jsr  $553f
        lda  #$01
        sta  $0a
        jmp  $582c
$58e6:  jsr  $5529
        bra  $58df
$58eb:  bsr  $586e
        bsr  $5888
        jsr  $5555
        lda  #$07
        sta  $0a
        jmp  $582c
$58f9:  jsr  $586e
        jsr  $53e8
        lda  #$05
        sta  $0a
        jmp  $582c
$5906:  jsr  $54b3
        bra  $58df
$590b:  jsr  $555d
        lda  #$01
        sta  $0a
        jmp  $582c
$5915:  bsr  $5921
        bsr  $594e
        lda  #$0c
        sta  $0a
        jmp  $582c
$5921:  jsr  $586e
        jsr  $5888
        iny
        lda  ($18),y
        sta  $4f89
        iny
        lda  ($18),y
        sta  $4f8f
        iny
        lda  ($18),y
        sta  $4f96
        iny
        lda  ($18),y
        sta  $4f9a
        iny
        lda  ($18),y
        sta  $4f9b
        rts
$5946:  bsr  $5921
        bsr  $5984
        bra  $5919
$594c:  bra  $58df
$594e:  jsr  $5555
        inc  $4f8b
        inc  $4f8c
        sec
        lda  $4f8d
        sbc  #$02
        sta  $4f8d

; Selector 11's follow-up helper begins with this bounded entry window; its
; branch target at $5966 and the external JSR $5555 remain outside this file's
; current locked callee set.
$5984:  jsr  $5555
        bra  $5966

; First selector callees reached through $5555/$555d. The $559d BSR target
; and $5685 JSR target are outside these locked bytes.
$5555:  jsr  $53e8
        bcs  $555c
        bsr  $5569
        rts
$555d:  dec  $4f9c
        bne  $5565
        jsr  $553f
$5565:  jsr  $54b3
        rts
$5569:  lda  $4fd8
        cmp  #$7f
        bcc  $5577
        bne  $5577
        lda  $4fd7
        cmp  #$31
$5577:  bcs  $559c
        lda  $4f9c
        bne  $5599
        lda  $4fd7
        sta  $4f9d
        lda  $4fd8
        sta  $4f9e
        bsr  $5529
        lda  $4f92
        sta  $10
        lda  $4f8a
        sta  $11
        jsr  $5685
$5599:  bsr  $559d
        clc
$559c:  rts

; $5966 is the continuation reached by $5984; $5989 is its next helper.
; These bounded roots still contain outbound callees requiring closure.
$5966:  jsr  $5208
        jsr  $5989
        jsr  $5529
        jsr  $4ffb
        lda  $4fde
        bne  $597a
        jsr  $5141
$597a:  stz  $4fde
        jsr  $553f
        jsr  $555d
        rts
$5989:  cly
        lda  ($00),y
        cmp  #$81
        bne  $599d
        iny
        lda  ($00),y
        cmp  #$96
        bne  $599d
        sta  $4fde
        jsr  $508b
$599d:  rts

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

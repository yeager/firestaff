; da65 V2.18 - N/A
; Created:    2026-07-12 09:13:48
; Input file: /tmp/theron-disasm/theron-us-stage2.bin
; Page:       1


        .setcpu "huc6280"

L0000           := $0000
L000B           := $000B
L0011           := $0011
L0020           := $0020
L0028           := $0028
L0038           := $0038
L003A           := $003A
L0040           := $0040
L0044           := $0044
L004C           := $004C
L0066           := $0066
L013E           := $013E
L022D           := $022D
L032A           := $032A
L0800           := $0800
L0810           := $0810
L0876           := $0876
L08A9           := $08A9
L0B2A           := $0B2A
L0C01           := $0C01
L0C3C           := $0C3C
L1010           := $1010
L142D           := $142D
L1700           := $1700
L1800           := $1800
L180C           := $180C
L1E2D           := $1E2D
L1EAD           := $1EAD
L1F00           := $1F00
L2000           := $2000
L201C           := $201C
L2020           := $2020
L2040           := $2040
L2828           := $2828
L2838           := $2838
L3114           := $3114
L3172           := $3172
L3177           := $3177
L3181           := $3181
L3221           := $3221
L33A1           := $33A1
L3583           := $3583
L36FC           := $36FC
L37A0           := $37A0
L37D8           := $37D8
L3800           := $3800
L3828           := $3828
L383E           := $383E
L3848           := $3848
L3876           := $3876
L3879           := $3879
L387C           := $387C
L3A2E           := $3A2E
L3A4C           := $3A4C
L3AB7           := $3AB7
L3B16           := $3B16
L3B86           := $3B86
L3C2D           := $3C2D
L3C59           := $3C59
L3C6E           := $3C6E
L3EBA           := $3EBA
L3EBE           := $3EBE
L3ECA           := $3ECA
L3ECE           := $3ECE
LE006           := $E006
LE009           := $E009
LE00C           := $E00C
LE012           := $E012
LE015           := $E015
LE018           := $E018
LE01B           := $E01B
LE01E           := $E01E
LE02D           := $E02D
LE033           := $E033
LE03C           := $E03C
LE03F           := $E03F
LE042           := $E042
LE045           := $E045
LE04B           := $E04B
LE04E           := $E04E
LE051           := $E051
LE054           := $E054
LE05A           := $E05A
LE063           := $E063
LE069           := $E069
LE06C           := $E06C
LE06F           := $E06F
LE078           := $E078
LE07B           := $E07B
LE081           := $E081
LE099           := $E099
LE09C           := $E09C
LE0D8           := $E0D8
LE0DE           := $E0DE
LE26A           := $E26A
LFC00           := $FC00
LFE00           := $FE00
LFF00           := $FF00
L4000:  sei
        ldx     #$FF
        txs
        lda     $FFF5
        inc     a
        tam     #$08
        inc     a
        inc     a
        inc     a
        tam     #$10
        pha
        cli
        jsr     L8000
        pla
        inc     a
        tam     #$20
        inc     a
        tam     #$40
        jsr     L40B7
L401E:  nop
        jsr     LE042
        cla
        jsr     LE02D
        jsr     LE018
        jsr     L40AE
        jsr     LE00C
        sei
        stz     $F5
        ldx     #$FF
        txs
        cli
        lda     #$10
        sta     $FF
        jsr     LE0D8
        lda     #$01
        sta     $FF
        jsr     LE0D8
        jsr     L4B2D
        jsr     L4B73
        lda     #$00
        jsr     LE06C
        lda     #$00
        ldx     #$20
        ldy     #$1E
        jsr     LE06F
        jsr     LE078
        jsr     LE081
        jsr     LE099
        lda     #$10
        jsr     LE09C
        cla
        jsr     LE069
        stz     $220C
        tii     $220C,$220D,$0007
        jsr     LE07B
        tii     $2700,$2000,$0080
L407E:  bsr     L40AE
        lda     #$01
        sta     $F8
        lda     #$01
        sta     $FF
        lda     #$00
        sta     $FA
        lda     #$38
        sta     $FB
        jsr     LE009
        cmp     #$00
        bne     L407E
        lda     $278A
        ldx     $278B
        ldy     $278C
        iny
        stz     $26FF
        tii     $26FF,$2700,$1100
        jmp     L3800

L40AE:  lda     #$02
        sta     $FE
        stz     $FD
        stz     $FC
        rts

L40B7:  stz     $02
        jsr     L4814
        stz     L4EC1
        lda     #$02
        jsr     L4F5E
        lda     #$00
        sta     $1C
        lda     #$60
        sta     $1D
L40CC:  jsr     L4AF7
        jsr     LE063
        lda     $2228
        and     #$0C
        beq     L40DC
L40D9:  nop
L40DA:  nop
L40DB:  nop
L40DC:  cly
        lda     ($1C)
        asl     a
        tax
        jmp     (L410D,x)

L40E4:  clc
        adc     $1C
        sta     $1C
        cla
        adc     $1D
        sta     $1D
        bra     L40CC
        rts

L40F1:  lda     #$01
        bra     L40E4
L40F5:  lda     #$02
        bra     L40E4
L40F9:  lda     #$03
        bra     L40E4
L40FD:  lda     #$04
        bra     L40E4
L4101:  .byte   $A9
L4102:  ora     $80
        .byte   $DF
L4105:  lda     #$07
        bra     L40E4
L4109:  lda     #$09
        bra     L40E4
L410D:  cmp     $41
        .byte   $CB
        eor     ($D8,x)
        eor     ($DE,x)
        eor     ($E6,x)
        eor     ($EC,x)
        eor     ($F0,x)
        eor     ($F4,x)
        eor     ($14,x)
        say
        .byte   $53
L4120:  say
        csl
        say
L4123:  eor     L6342,y
        say
        adc     ($42),y
        bra     L416D
        dey
        say
        sta     ($42),y
        tin     $1942,$F043,$F845
L4136:  eor     $FE
        eor     $15
        lsr     $1D
        lsr     $23
        lsr     $35
        lsr     $29
        lsr     $2F
        lsr     $45
        tma     #$97
        bsr     L417D
        bsr     L41AB
        bsr     L4195
        lsr     $4F
        lsr     $34
        say
        .byte   $FB
        say
        bit     $43,x
        asl     $49,x
        bpl     L41A4
        dex
        eor     $75
        tma     #$DD
        tma     #$09
        bsr     L41B7
        lsr     $74
        lsr     $8F
        lsr     $CA
        lsr     $94
        .byte   $47
L416D:  ldx     $47
        cmp     $47
        .byte   $D3
        rmb4    $9D
        .byte   $46
L4175:  lda     $B844,x
        lsr     $61
        tma     #$0A
        pha
L417D:  tai     $4247,$5F48,$7F48
        bsr     L41E8
        pha
        bbs1    $48,L4136
        pha
        ora     ($49,x)
        .byte   $1B
        eor     #$BE
        say
        .byte   $73
        eor     #$95
        .byte   $49
L4195:  .byte   $EB
        eor     $AB
        .byte   $49
L4199:  ldy     $49,x
        .byte   $BB
        eor     #$5E
        lsr     a
        .byte   $EB
        bsr     L4123
        lsr     a
        dex
L41A4:  lsr     a
        .byte   $D3
        eor     #$E8
        eor     #$3B
        lsr     a
L41AB:  trb     $4A
        .byte   $1B
        lsr     a
        say
        lsr     a
        bvc     L41FD
        .byte   $FB
        eor     #$4E
L41B6:  pha
L41B7:  ldy     #$4A
L41B9:  iny
        lda     ($1C),y
        tax
        iny
        lda     ($1C),y
        sta     $1D
        stx     $1C
        rts

L41C5:  bsr     L41B9
        cla
        jmp     L40E4

        bsr     L41F8
L41CD:  bne     L41D5
L41CF:  bsr     L41B9
        cla
        jmp     L40E4

L41D5:  jmp     L4101

        .byte   $44
L41D9:  .byte   $1E
L41DA:  bne     L41CF
        bra     L41D5
        bsr     L41F8
        bcc     L41D5
        beq     L41D5
        bra     L41CF
        bsr     L41F8
L41E8:  bcs     L41D5
        bra     L41CF
        bsr     L4203
        bra     L41CD
        bsr     L4203
        bra     L41DA
        bsr     L4203
        bra     L41E8
L41F8:  iny
        lda     ($1C),y
        tax
        .byte   $BD
L41FD:  bra     L4226
        iny
        cmp     ($1C),y
        rts

L4203:  iny
        lda     ($1C),y
        tax
        lda     $2780,x
        pha
        iny
        lda     ($1C),y
        tax
        pla
        cmp     $2780,x
        rts

        iny
L4215:  lda     ($1C),y
        bsr     L421C
        jmp     L40F5

L421C:  sta     L4EC1
        sta     L4D7B
        asl     a
        asl     a
        clc
        .byte   $6D
L4226:  php
        bmi     L41B6
        ora     #$30
        lda     #$02
        jsr     L4F5E
        bcs     L4233
        rts

L4233:  brk
        dec     $5B
        jsr     L43D6
        bsr     L4240
        stz     $5B
        jmp     L40F5

L4240:  jsr     L37D8
        lda     #$00
        sta     L0020
        lda     #$68
        sta     $21
L424B:  lda     #$03
        sta     $1E
        jsr     L383E
        rts

        rts

        jsr     L4B4E
        bra     L427D
        jsr     L41F8
        lda     ($1C),y
        sta     $2780,x
        bra     L427D
        jsr     L41F8
        lda     ($1C),y
        clc
        adc     $2780,x
        sta     $2780,x
        bra     L427D
        jsr     L41F8
        lda     ($1C),y
        sec
        sbc     $2780,x
        sta     $2780,x
L427D:  jmp     L40F9

        jsr     L41F8
        inc     $2780,x
        bra     L428E
        jsr     L41F8
        dec     $2780,x
L428E:  jmp     L40F5

        iny
        lda     ($1C),y
        tay
        lda     $1C
        pha
        lda     $1D
        pha
        bsr     L42A6
        pla
        sta     $1D
        pla
        sta     $1C
        jmp     L40F5

L42A6:  lda     #$00
        sta     L0000
        lda     #$68
        sta     $01
        tya
        asl     a
        tay
        lda     (L0000),y
        sta     $1C
        iny
        lda     (L0000),y
        sta     $1D
        jsr     L40CC
        rts

        .byte   $A5
L42BF:  trb     $A548
        ora     $2048,x
        lda     $2041,y
        cpy     $6840
        sta     $1D
        pla
        sta     $1C
        jmp     L40F9

        iny
        lda     ($1C),y
        sta     L4EC2
        iny
        .byte   $B1
L42DB:  trb     $D78D
        bbr4    $C8,$4292
        trb     $D88D
        bbr4    L0020,$432F
        bbr4    $AD,$4363
        eor     $0C85
        lda     L4D7A
        sta     $0D
        jsr     L4BD2
        jsr     L5E27
        jmp     L40FD

        iny
        lda     ($1C),y
        pha
        jsr     L4B00
        jsr     L4F48
        lda     L4D79
        sta     L4FDB
        lda     L4D7A
        sta     L4FDC
        pla
        clx
        jsr     L56AF
        jmp     L40F5

        lda     $1C
        pha
        lda     $1D
        pha
        jsr     L41B9
        lda     #$43
        pha
        lda     #$2A
        pha
        jmp     (L201C)

        pla
        sta     $1D
        pla
        sta     $1C
        jmp     L40F9

        bsr     L4339
        jmp     L40F5

L4339:  iny
        lda     ($1C),y
        sta     L4EC2
        lda     #$0D
        jsr     L4F5E
        rts

        iny
        lda     ($1C),y
        iny
        sta     $0E
        lda     ($1C),y
        iny
        sta     $10
        lda     ($1C),y
        sta     L0011
        iny
        lda     ($1C),y
        .byte   $85
L4358:  ora     ($A9)
        tsb     L0020
        smb3    L003A
        jmp     L4101

        iny
        lda     ($1C),y
        sta     $15
        iny
        lda     ($1C),y
        sta     $14
        lda     #$14
        jsr     L3AB7
        jmp     L40F9

L4373:  brk
L4374:  brk
        jsr     L4403
        dec     $5B
        ldy     #$01
        lda     ($1C),y
        beq     L4393
        bsr     L43D6
        bsr     L43B5
        stz     $F9
        stz     $FA
        lda     #$0E
        sta     $FF
        dec     $F8
        jsr     LE03F
        bra     L43B0
L4393:  bsr     L43D6
L4395:  bsr     L43B5
        stz     $FA
        stz     $FB
        stz     $FF
        jsr     LE033
        .byte   $C9
L43A1:  brk
        bne     L4395
        lda     $37D0
        sta     L4373
        lda     $37D1
        sta     L4374
L43B0:  stz     $5B
        jmp     L40F9

L43B5:  lda     $37D6
        pha
        lda     $37D7
        pha
        lda     #$9F
        sta     $37D6
        lda     #$5E
        sta     $37D7
        jsr     L37D8
        pla
        sta     $37D7
        pla
        sta     $37D6
L43D2:  jsr     L3848
        rts

L43D6:  iny
        lda     ($1C),y
        sta     $37CC
        rts

        jsr     L4403
        stz     $FA
        stz     $FB
        lda     L4373
        sta     $F8
        lda     L4374
        sta     $F9
        lda     #$0E
        sta     $FF
        stz     $FE
        jsr     LE03C
        ldy     #$01
        lda     ($1C),y
        bne     L4400
        jsr     L4403
L4400:  jmp     L40F5

L4403:  jsr     LE045
        bne     L4403
        rts

        cla
        jsr     LE02D
        bsr     L4415
        jsr     LE012
        jmp     L40F5

L4415:  ldy     #$01
L4417:  lda     ($1C),y
L4419:  tax
        lda     L4B3C,x
        sta     $F8
        clc
        sed
        adc     #$01
        sta     $FC
        cld
        lda     #$80
        sta     $FB
        lda     #$83
        sta     $FF
        rts

L442F:  brk
L4430:  brk
L4431:  brk
L4432:  brk
        bsr     L4446
        lda     L442F
        sta     L0000
        lda     L4430
        sta     $01
L443F:  cla
        jsr     L3AB7
        jmp     L40F5

L4446:  iny
        lda     ($1C),y
        sta     L4EC2
        jsr     L4F48
        lda     L4D79
        sta     L442F
        lda     L4D7A
        sta     L4430
        jsr     L4BD2
        rts

        iny
        lda     ($1C),y
        sta     L4EC2
        iny
        lda     ($1C),y
        sta     L4EC3
        sta     L4431
        iny
        lda     ($1C),y
        sta     L4EC4
        sta     L4432
        lda     #$09
        jsr     L4F5E
        jmp     L40FD

        bsr     L4483
        bra     L443F
L4483:  iny
        lda     ($1C),y
        sta     L4EC2
        .byte   $20
        .byte   $C9
L448B:  lsr     $C3AD
        lsr     a:$85
        lda     L4EC4
        sta     $01
        rts

        iny
        lda     ($1C),y
        sta     L0011
        iny
L449D:  lda     ($1C),y
        sta     $10
        iny
L44A2:  lda     ($1C),y
        sta     $13
        iny
        lda     ($1C),y
        sta     $12
        iny
        lda     ($1C),y
        sta     $15
        iny
        lda     ($1C),y
        sta     $14
        lda     #$01
        jsr     L3AB7
        jmp     L4105

        iny
        lda     ($1C),y
        bne     L44C9
        lda     #$10
        jsr     L3AB7
        bra     L44E4
L44C9:  cmp     #$01
        bne     L44D4
        lda     #$11
        jsr     L3AB7
        bra     L44E4
L44D4:  cmp     #$02
        bne     L44DF
        lda     #$12
        jsr     L3AB7
        bra     L44E4
L44DF:  lda     #$16
        jsr     L3AB7
L44E4:  jmp     L40F5

L44E7:  brk
L44E8:  brk
L44E9:  brk
L44EA:  brk
        iny
        lda     ($1C),y
        pha
        beq     L4511
        and     #$C0
        cmp     #$40
        beq     L44FA
        jmp     L459F

L44FA:  iny
        lda     ($1C),y
        sta     L4EC2
        jsr     L4C17
        lda     L4D79
        sta     L442F
        lda     L4D7A
        sta     L4430
        bra     L4514
L4511:  jsr     L4446
L4514:  jsr     L4F31
        sec
        .byte   $A0
L4519:  ora     ($B1,x)
        brk
        sbc     L4EC7
        sta     (L0000),y
        iny
        lda     (L0000),y
        sbc     L4EC8
        sta     (L0000),y
        clc
        dey
        lda     (L0000),y
        adc     #$25
        sta     (L0000),y
        iny
        cla
        adc     (L0000),y
        sta     (L0000),y
        dec     $5B
        lda     L4EC2
        sta     $37CC
        jsr     L3A2E
        ldy     #$04
        lda     #$25
        sta     (L0000),y
        iny
        cla
        sta     (L0000),y
        stz     $5B
        lda     L442F
        .byte   $8D
L4552:  smb6    L0044
        lda     L4430
        sta     L44E8
        sec
        lda     L4EC7
        sbc     #$25
        sta     L4EC7
        lda     L4EC8
        sbc     #$00
        sta     L4EC8
        tma     #$08
        pha
        jsr     L4AF7
        ldy     #$05
        lda     ($1C),y
        sta     L4EC5
        sta     L44E9
        iny
        lda     ($1C),y
        sta     L4EC6
        sta     L44EA
        pla
        tam     #$08
        clc
        lda     L44E7
        adc     #$25
        .byte   $8D
L458E:  tdd     $624E,$E86D,$8D44
        cpy     $4E
        lda     #$0A
        jsr     L4F5E
        jsr     L4AF7
L459F:  ldy     #$03
        lda     ($1C),y
        sta     L0000
        iny
L45A6:  lda     ($1C),y
        sta     $01
        lda     L44E7
        sta     $02
        lda     L44E8
        sta     $03
        lda     L44E9
        sta     $04
        lda     L44EA
        sta     $05
        pla
        lsr     a
        bcs     L45C7
        lda     #$17
        jsr     L3AB7
L45C7:  jmp     L4105

        jsr     L4446
        lda     L442F
        sta     L0000
        lda     L4430
        sta     $01
L45D7:  lda     L4D7B
        asl     a
        asl     a
        clc
        adc     $3008
        sta     $300A
        lda     #$05
        jsr     L3AB7
        jmp     L40F5

        jsr     L4483
        bra     L45D7
        lda     #$06
        jsr     L3AB7
        jmp     L40F1

        bsr     L4604
        lda     #$08
        bra     L460F
        bsr     L4604
        lda     #$0A
        bra     L460F
L4604:  iny
        lda     ($1C),y
        sta     $0E
        iny
        lda     ($1C),y
        sta     $10
        rts

L460F:  jsr     L3AB7
        jmp     L40F9

        lda     #$07
        jsr     L3AB7
        jmp     L40F1

        bsr     L463B
        lda     #$09
        bra     L4641
        bsr     L463B
        lda     #$0B
        bra     L4641
        bsr     L463B
        lda     #$0C
        bra     L4641
        bsr     L463B
        lda     #$0D
        bra     L4641
        bsr     L463B
        lda     #$0E
        bra     L4641
L463B:  iny
        lda     ($1C),y
        sta     $0E
        rts

L4641:  jsr     L3AB7
        jmp     L40F5

        lda     #$02
L4649:  jsr     L3AB7
        jmp     L40F1

        lda     #$03
        bra     L4649
        iny
        lda     ($1C),y
        pha
        iny
        lda     ($1C),y
        tax
        jsr     L4B00
        jsr     L4F48
        lda     L4D79
        sta     L4FDB
        lda     L4D7A
L466A:  .byte   $8D
L466B:  .byte   $DC
        bbr4    $68,$468F
        bbs2    $56,$46BE
        sbc     L2040,y
        tst     #$44,$A0
        sxy
        lda     ($1C),y
        sta     $02
        iny
        lda     ($1C),y
        sta     $03
        iny
        lda     ($1C),y
        sta     $0E
        lda     #$0F
        jsr     L3AB7
        .byte   $4C
L468D:  .byte   $01
L468E:  eor     ($C8,x)
        .byte   $B1
L4691:  trb     $339C
        .byte   $3B
L4695:  .byte   $CD
L4696:  .byte   $33
        .byte   $3B
        bcs     L4695
        jmp     L40F5

        iny
        lda     ($1C),y
        sta     $0402
        iny
        lda     ($1C),y
        sta     $0403
        iny
        lda     ($1C),y
        sta     $0404
        iny
        lda     ($1C),y
        sta     $0405
        jmp     L4101

        iny
        lda     ($1C),y
        bne     L46C4
        lda     #$13
        jsr     L3AB7
        bra     L46C7
L46C4:  jsr     L5E4D
L46C7:  jmp     L40F5

        iny
        lda     ($1C),y
        bne     L474A
        tma     #$08
        pha
        tma     #$10
        pha
        clc
        lda     $FFF5
        adc     #$06
        tam     #$08
        inc     a
        tam     #$10
        dec     $5B
        jsr     L43D6
        jsr     L37D8
        lda     #$00
        sta     L0020
        lda     #$60
        sta     $21
        jsr     L383E
        stz     $5B
        lda     #$00
        sta     $F8
        lda     #$00
        sta     $FF
        jsr     LE0D8
        lda     #$02
        sta     $F8
        lda     #$02
        sta     $FF
        jsr     LE0D8
        clc
        lda     $FFF5
        adc     #$06
        sta     $F8
        inc     a
        sta     $F9
        lda     #$03
        sta     $FF
        jsr     LE0D8
        lda     #$0A
        sta     $F8
        lda     #$80
        sta     $F9
        lda     #$04
        sta     $FF
        jsr     LE0D8
        lda     #$00
        sta     L0000
        lda     #$60
        sta     $01
        bsr     L476A
        lda     #$00
        sta     $F8
        lda     #$14
        sta     $FF
        jsr     LE0D8
        pla
        tam     #$10
        pla
        tam     #$08
        jmp     L40F9

L474A:  lda     #$3F
        sta     $F8
        lda     #$0F
        sta     $FF
        jsr     LE0D8
        lda     #$3F
        sta     $F8
        lda     #$0E
        sta     $FF
        jsr     LE0D8
        lda     #$01
        sta     $FF
        jsr     LE0D8
        jmp     L40F9

L476A:  ldx     #$05
        ldy     #$FF
L476E:  bsr     L4789
        lda     $F8
        bne     L477A
        lda     $F9
        bne     L477A
        bra     L4783
L477A:  phx
L477B:  phy
        stx     $FF
        jsr     LE0D8
        ply
        plx
L4783:  inx
        cpx     #$0A
L4786:  bne     L476E
        rts

L4789:  iny
        lda     (L0000),y
        sta     $F8
        iny
        lda     (L0000),y
        sta     $F9
        rts

        iny
        lda     ($1C),y
        sta     $F8
        lda     #$0B
        sta     $FF
        jsr     LE0D8
        jsr     L4B2D
        jmp     L40F5

        iny
        lda     ($1C),y
        bne     L47B9
        lda     #$3F
        sta     $F8
        lda     #$0E
        sta     $FF
L47B3:  .byte   $20
L47B4:  cld
L47B5:  .byte   $E0
L47B6:  .byte   $4C
L47B7:  .byte   $F5
L47B8:  rti

L47B9:  .byte   $85
L47BA:  sed
L47BB:  .byte   $A9
L47BC:  .byte   $13
L47BD:  .byte   $85
L47BE:  .byte   $FF
L47BF:  jsr     LE0D8
L47C2:  .byte   $4C
L47C3:  .byte   $F5
L47C4:  rti

        lda     #$3F
L47C7:  .byte   $85
L47C8:  sed
L47C9:  .byte   $A9
L47CA:  .byte   $0F
L47CB:  .byte   $85
L47CC:  .byte   $FF
L47CD:  .byte   $20
L47CE:  cld
        cpx     #$4C
L47D1:  .byte   $F1
L47D2:  rti

L47D3:  iny
L47D4:  .byte   $B1
L47D5:  .byte   $1C
L47D6:  .byte   $D0
L47D7:  .byte   $0E
L47D8:  .byte   $A9
L47D9:  .byte   $BF
L47DA:  .byte   $85
L47DB:  sed
L47DC:  lda     #$0E
        sta     $FF
L47E0:  jsr     LE0D8
L47E3:  jmp     L40F5

L47E6:  lda     #$02
        sta     $F8
        lda     #$12
        sta     $FF
        jsr     LE0D8
        bra     L47E3
        iny
        lda     ($1C),y
        sta     $0E
        iny
        lda     ($1C),y
        sta     $10
        iny
        lda     ($1C),y
        sta     $12
        lda     #$15
        jsr     L3AB7
        jmp     L40FD

        iny
        lda     ($1C),y
        sta     $02
        bsr     L4814
        jmp     L40F5

L4814:  lda     #$D3
        sta     L0000
        lda     #$37
        sta     $01
        clc
        ldy     #$01
        .byte   $A5
L4820:  sxy
        adc     (L0000),y
        sta     $24
        iny
        cla
        adc     (L0000),y
        sta     $23
        cla
        adc     (L0000)
        sta     $22
        lda     #$00
        sta     L0020
        lda     #$28
        sta     $21
        lda     #$01
        sta     $1E
        sta     $25
        jsr     L383E
        rts

        iny
        lda     ($1C),y
        sta     L4EC2
        jsr     L4BE7
        jmp     L40F5

        iny
        lda     ($1C),y
        sta     L4EC2
        stz     L4EC1
        lda     #$0C
        jsr     L4F5E
        jmp     L40F5

        jmp     L40F1

L4862:  jsr     LE063
        lda     $2228
        nop
        nop
        bne     L487E
        lda     #$95
        sta     $FA
        lda     #$48
        sta     $FB
        jsr     LE01E
        lda     L4895
        beq     L4862
        bra     L488B
L487E:  lda     #$0C
        jsr     LE02D
        ldx     #$14
L4885:  jsr     L4B2D
        dex
        bne     L4885
L488B:  jsr     LE018
        cla
        jsr     LE02D
L4892:  jmp     L40F1

L4895:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        jsr     LE063
        lda     $2228
        .byte   $F0
L48A6:  .byte   $EB
        sta     $2780
        bra     L4892
        iny
        lda     ($1C),y
        bne     L48BE
        dec     a
        sta     $3B68
        iny
        lda     ($1C),y
        bne     L48DB
        bsr     L48DE
        bra     L48DB
L48BE:  iny
        lda     ($1C),y
        cmp     #$FF
        beq     L48CE
        sta     $3B6E
        iny
        lda     ($1C),y
        sta     $3B6F
L48CE:  lda     #$FF
        sta     $3B69
L48D3:  lda     $3B69
        bne     L48D3
        stz     $3B68
L48DB:  jmp     L40FD

L48DE:  stz     $0402
        stz     $0403
        lda     #$00
        sta     L0000
        lda     #$04
        sta     $01
L48EC:  stz     $0404
        stz     $0405
        lda     L0000
        bne     L48F8
        dec     $01
L48F8:  dec     L0000
        lda     L0000
L48FC:  ora     $01
        bne     L48EC
        rts

        clx
L4902:  iny
        lda     ($1C),y
        sta     $3B70,x
        inx
        cpx     #$08
        bne     L4902
        jmp     L4109

        jsr     L3177
L4913:  jmp     L40F1

        jsr     L4F11
        bra     L4913
        dec     $5B
        .byte   $20
        .byte   $D6
L491F:  tma     #$C8
        lda     ($1C),y
        cmp     #$01
        beq     L4930
        cmp     #$02
        beq     L4968
        jsr     L37D8
        bra     L4933
L4930:  .byte   $20
        .byte   $B5
L4932:  .byte   $43
L4933:  lda     #$01
        sta     $1E
        sta     $25
        jsr     L4F48
        lda     L4D79
        sta     L0020
        lda     L4D7A
        sta     $21
        jsr     L4B00
L4949:  jsr     L43D2
        jsr     LE009
        cmp     #$00
        bne     L4949
L4953:  lda     #$FF
        jsr     LE01B
        cmp     #$00
        beq     L4960
        cmp     #$0E
        bcc     L4953
L4960:  stz     $5B
        jsr     L4AF7
        jmp     L40F9

L4968:  lda     $37CC
        jsr     L4419
        jsr     LE015
        bra     L4960
        iny
        .byte   $B1
L4975:  trb     $16D0
        iny
        lda     ($1C),y
        sta     $27C2
        iny
        lda     ($1C),y
        sta     $27C3
        stz     $27C6
        lda     #$FF
        sta     $27C7
        bra     L4991
L498E:  stz     $27C7
L4991:  jmp     L40FD

        rts

        jsr     L4446
        lda     L442F
        sta     $27C0
        sta     $62
        lda     L4430
        sta     $27C1
        sta     $63
        jmp     L40F5

        bsr     L49E1
        lda     #$08
        jsr     L4F5E
        bra     L49D8
        bsr     L49E1
        jsr     L4C00
        bra     L49D8
        bsr     L49E1
        iny
        lda     ($1C),y
        sta     L4EC3
        iny
        lda     ($1C),y
        sta     L4EC4
        stz     L4EC5
        lda     #$07
        jsr     L4F5E
        bra     L49DB
        bsr     L49E1
        jsr     L4BD2
L49D8:  jmp     L40F5

L49DB:  jmp     L40FD

L49DE:  jmp     L4101

L49E1:  iny
L49E2:  lda     ($1C),y
        sta     L4EC2
        rts

        bsr     L49E1
        iny
        lda     ($1C),y
        sta     L4EC5
        iny
        lda     ($1C),y
        sta     L4EC6
        jsr     L4C30
        .byte   $80
L49FA:  cpx     #$44
        cpx     $C8
        lda     ($1C),y
        sta     L4EC1
        iny
        lda     ($1C),y
        sta     L4EC5
L4A09:  iny
        lda     ($1C),y
        sta     L4EC6
        jsr     L4C3F
        bra     L49DE
        bsr     L49E1
        jsr     L4C17
        bra     L49D8
        bsr     L49E1
        iny
        lda     ($1C),y
        sta     L4EC1
        iny
        lda     ($1C),y
        sta     L4EC3
        iny
        lda     ($1C),y
        sta     L4EC4
        lda     #$01
        sta     L4EC5
        lda     #$07
        jsr     L4F5E
        bra     L49DE
        bsr     L49E1
        jsr     L4CE1
        bra     L49D8
        bsr     L49E1
        iny
        lda     ($1C),y
        sta     L4EC1
        jsr     L4D0E
        jmp     L40F9

        bsr     L49E1
        iny
        lda     ($1C),y
        sta     L4EC1
        jsr     L4D6A
        jmp     L40F9

        jsr     L463B
        iny
        lda     ($1C),y
        tax
        lda     $0E
L4A67:  phx
        pha
        lda     #$0C
        jsr     L3AB7
        pla
        sta     $0E
        pha
        lda     #$0E
        jsr     L3AB7
        pla
        sta     $0E
        plx
        dex
        bne     L4A67
        jmp     L40F9

        jsr     L463B
L4A84:  lda     $0E
        pha
        lda     #$0C
        jsr     L3AB7
        pla
        sta     $0E
        pha
        lda     #$0E
        jsr     L3AB7
        pla
        sta     $0E
        jsr     LE045
        bne     L4A84
        jmp     L40F5

        iny
        lda     ($1C),y
        bne     L4AB8
        iny
        lda     ($1C),y
        sta     L4EC1
        lda     #$04
        sta     L4EC2
        .byte   $A9
L4AB1:  ora     (L0020,x)
        lsr     $4C4F,x
        .byte   $F9
        rti

L4AB8:  lda     #$04
        sta     L4EC1
        lda     #$7F
        sta     L4EC2
        lda     #$00
        jsr     L4F5E
        jmp     L40F9

        iny
        lda     ($1C),y
        stz     L0000
L4ACF:  pha
        bsr     L4AE4
        pla
        dec     a
        bne     L4ACF
        lda     L0000
        bne     L4ADD
        jmp     L41C5

L4ADD:  iny
        iny
        iny
        tya
        jmp     L40E4

L4AE4:  iny
        lda     ($1C),y
        tax
        iny
        lda     ($1C),y
        cmp     #$FF
        beq     L4AF6
        cmp     $2780,x
        beq     L4AF6
        dec     L0000
L4AF6:  rts

L4AF7:  clc
        lda     $FFF5
        adc     #$01
        tam     #$08
        rts

L4B00:  lda     $3009
        tam     #$08
        inc     a
        tam     #$10
        inc     a
        tam     #$20
        inc     a
        tam     #$40
        rts

        lda     #$CF
        sta     $F8
        lda     #$11
        sta     $FF
        jsr     LE0D8
        jsr     L4B2D
        rts

        lda     #$02
        sta     $F8
        lda     #$12
L4B24:  sta     $FF
        jsr     LE0D8
        jsr     L4B2D
        rts

L4B2D:  pha
        phx
        lda     #$FF
L4B31:  ldx     #$FF
L4B33:  dex
        bne     L4B33
        dec     a
        bne     L4B31
        plx
        pla
        rts

L4B3C:  st0     #$04
        ora     $06
        rmb0    $08
        ora     #$10
        ora     ($12),y
        st1     #$14
        ora     $16,x
        rmb1    $18
L4B4C:  brk
L4B4D:  brk
L4B4E:  iny
        lda     ($1C),y
        sta     L4B4C
        iny
        lda     ($1C),y
        sta     L4B4D
        stz     $3B31
        stz     $3B32
L4B60:  lda     $3B32
        cmp     L4B4D
        bcc     L4B70
        bne     L4B70
        lda     $3B31
        cmp     L4B4C
L4B70:  bcc     L4B60
        rts

L4B73:  sei
        st0     #$00
        st1     #$00
        st2     #$08
        st0     #$02
        clx
        ldy     #$78
L4B7F:  st1     #$00
        st2     #$00
        dex
        bne     L4B7F
        dey
        bne     L4B7F
        st0     #$05
        lda     $F3
        and     #$3F
        sta     $F3
        sta     a:$02
        cli
        rts

        jsr     L3181
        lda     #$04
        sta     L4EC1
        lda     #$7F
        sta     L4EC2
        lda     #$00
        jsr     L4F5E
        lda     #$00
        jsr     L4F5E
        stz     L4EC1
L4BB0:  lda     #$02
        jsr     L4F5E
        stz     L4D7B
        lda     $3008
        sta     $3009
        sta     $300A
        rts

        lda     #$FF
        sta     L4EC1
        lda     #$08
        sta     L4EC2
        lda     #$01
        jsr     L4F5E
        rts

L4BD2:  jsr     L4F48
        lda     L4D7B
        sta     L4EC1
        lda     #$03
        jsr     L4F5E
        bcs     L4BE6
L4BE2:  jsr     L4EF4
        rts

L4BE6:  brk
L4BE7:  dec     $5B
        jsr     L4F31
        lda     L0000
        sta     $02
        lda     $01
        sta     $03
        lda     L4EC2
        sta     $37CC
        jsr     L37A0
        stz     $5B
        rts

L4C00:  lda     L4D7B
        .byte   $8D
        .byte   $C1
L4C05:  lsr     $C59C
        lsr     L4820
        .byte   $4F
        .byte   $A9
L4C0D:  .byte   $05
L4C0E:  .byte   $20
L4C0F:  .byte   $5E
L4C10:  .byte   $4F
L4C11:  .byte   $B0
L4C12:  .byte   $03
L4C13:  .byte   $20
L4C14:  set
L4C15:  .byte   $4E
L4C16:  rts

L4C17:  lda     L4D7B
        sta     L4EC1
        lda     #$01
        sta     L4EC5
        jsr     L4F48
        lda     #$05
        jsr     L4F5E
        bcs     L4C2F
        jsr     L4EF4
L4C2F:  rts

L4C30:  jsr     L4EC9
        bcs     L4C3A
        lda     #$0A
        jsr     L4F5E
L4C3A:  rts

L4C3B:  sec
L4C3C:  stz     $5B
        rts

L4C3F:  jsr     L4EC9
        bcs     L4C3B
        dec     $5B
        jsr     L3221
        lda     L4EC1
L4C4C:  asl     a
        asl     a
        tay
        clx
L4C50:  clc
        lda     ($02),y
        sta     $3447,x
        adc     $3008
        sta     $3B7E,x
        iny
        inx
        cpx     #$04
        bne     L4C50
        tma     #$08
        pha
        tma     #$10
        pha
        tma     #$20
        pha
        tma     #$40
L4C6D:  pha
        jsr     L4B00
L4C71:  lda     L4EC3
        sta     $0E
        .byte   $AD
        .byte   $C4
L4C78:  .byte   $4E
L4C79:  sta     $0F
L4C7B:  lda     #$00
        sta     $2E
        .byte   $85
L4C80:  bmi     $4C07
        bpl     $4C2D
L4C84:  rts

        sta     $2F
        sta     $31
L4C89:  sta     L0011
        lda     L4EC7
        sta     $12
        lda     L4EC8
        sta     $13
        lda     #$47
        .byte   $85
L4C98:  brk
        lda     #$34
L4C9B:  sta     $01
        .byte   $20
L4C9E:  lda     ($33,x)
        jsr     L3583
        .byte   $20
        .byte   $59
L4CA5:  bit     $8620,x
        .byte   $3B
        jsr     L3C6E
        lda     $3B7C
        sta     $3006
        lda     $3B7D
        sta     $3007
        lda     L4EC5
        sta     $3004
        lda     L4EC6
        sta     $3005
        lda     #$00
        sta     $3002
        lda     #$60
        sta     $3003
        jsr     L36FC
        pla
        tam     #$40
        pla
        tam     #$20
        pla
        tam     #$10
        pla
        tam     #$08
        clc
        stz     $5B
        rts

L4CE1:  lda     L4D7B
        sta     L4EC1
        jsr     L4EC9
        bcs     L4D0D
        lda     #$06
        jsr     L4F5E
        bcs     L4D0D
        dec     $5B
        lda     L4EC2
        sta     $37CC
        lda     L4EC7
        sta     $37D0
        lda     L4EC8
        sta     $37D1
        jsr     L3876
        stz     $5B
        rts

L4D0D:  brk
L4D0E:  bsr     L4D15
        bcs     L4D67
        bsr     L4D4A
        rts

L4D15:  lda     L4D7B
        pha
        lda     L4EC1
        sta     L4D68
        sta     L4D7B
        jsr     L4F48
        lda     L4EC3
        sta     L4EC5
        lda     L4EC4
        sta     L4EC6
        pla
        sta     L4D7B
        sta     L4EC1
        jsr     L4EC9
        bcs     L4D67
        lda     L4EC2
        sta     L4D69
        lda     L4D68
        sta     L4EC2
        rts

L4D4A:  lda     #$04
        jsr     L4F5E
        bcs     L4D67
        lda     L4D68
        sta     L4D7B
        lda     L4D69
        sta     L4EC2
        jsr     L4EF4
        lda     L4EC1
        sta     L4D7B
        rts

L4D67:  brk
L4D68:  brk
L4D69:  brk
L4D6A:  bsr     L4D15
        bcs     L4D67
        lda     L4EC1
        ora     #$80
        sta     L4EC1
        bsr     L4D4A
        rts

L4D79:  brk
L4D7A:  rts

L4D7B:  brk
        stx     $4D
        cmp     $4D
        tsb     $4E
        tma     #$4E
        clx
        lsr     a:L0000
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4DA2:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4DDA:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4DE3:  brk
L4DE4:  brk
L4DE5:  brk
L4DE6:  brk
L4DE7:  brk
L4DE8:  brk
L4DE9:  brk
L4DEA:  brk
L4DEB:  brk
L4DEC:  brk
L4DED:  brk
L4DEE:  brk
L4DEF:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4EC1:  brk
L4EC2:  brk
L4EC3:  brk
L4EC4:  brk
L4EC5:  brk
L4EC6:  brk
L4EC7:  brk
L4EC8:  brk
L4EC9:  dec     $5B
        lda     L4EC2
        sta     $37CC
        jsr     L4F31
        jsr     L3A2E
        bcs     L4EF1
        lda     $37CE
        sta     L4EC3
        lda     $37CF
        sta     L4EC4
        lda     $37D0
        sta     L4EC7
        lda     $37D1
        .byte   $8D
L4EEF:  iny
L4EF0:  .byte   $4E
L4EF1:  stz     $5B
        rts

L4EF4:  dec     $5B
        lda     L4EC2
        sta     $37CC
        lda     L4EC7
        sta     $37D0
        lda     L4EC8
        sta     $37D1
        jsr     L4F31
        jsr     L3879
        stz     $5B
        rts

L4F11:  jsr     L4F31
        cly
        cla
        sta     (L0000)
        iny
        lda     #$00
        sta     (L0000),y
        iny
        lda     #$60
        sta     (L0000),y
        rts

        ldx     #$04
L4F25:  stx     L4D7B
        bsr     L4F11
        dex
        bpl     L4F25
        stz     L4D7B
        rts

L4F31:  lda     #$7C
        sta     $02
        lda     #$4D
        sta     $03
        lda     L4D7B
        asl     a
        tay
        lda     ($02),y
        sta     L0000
        iny
        lda     ($02),y
        sta     $01
        rts

L4F48:  bsr     L4F31
        ldy     #$01
        lda     (L0000),y
        sta     L4EC3
        .byte   $8D
L4F52:  adc     $C84D,y
        lda     (L0000),y
        sta     L4EC4
        sta     L4D7A
        rts

L4F5E:  ldx     #$C1
        ldy     #$4E
        jsr     L3114
        rts

L4F66:  pha
        phx
        phy
        lda     #$03
L4F6B:  clx
L4F6C:  cly
L4F6D:  dey
        bne     L4F6D
        dex
        bne     L4F6C
        dec     a
        bne     L4F6B
        ply
        plx
        pla
        rts

L4F7A:  phx
        phy
        ldx     L4FD4
L4F7F:  cly
L4F80:  dey
        bne     L4F80
        dex
        bne     L4F7F
        ply
        plx
        rts

L4F89:  brk
L4F8A:  brk
L4F8B:  brk
L4F8C:  brk
L4F8D:  brk
L4F8E:  brk
L4F8F:  brk
        brk
L4F91:  brk
L4F92:  brk
L4F93:  brk
L4F94:  brk
L4F95:  brk
L4F96:  brk
L4F97:  brk
L4F98:  brk
L4F99:  brk
L4F9A:  brk
L4F9B:  brk
L4F9C:  brk
L4F9D:  brk
L4F9E:  brk
L4F9F:  brk
L4FA0:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4FB8:  brk
L4FB9:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4FD1:  brk
L4FD2:  brk
L4FD3:  brk
L4FD4:  .byte   $30
L4FD5:  brk
L4FD6:  rts

L4FD7:  brk
L4FD8:  .byte   $50
L4FD9:  brk
L4FDA:  brk
L4FDB:  brk
L4FDC:  brk
        .byte   $01
L4FDE:  brk
L4FDF:  lda     (L0000)
        sta     $F8
        rts

L4FE4:  clc
        lda     L4F98
        adc     L4FD9
        sta     L0000
        lda     L4F99
        adc     L4FDA
        sta     $01
L4FF5:  lda     L4F89
        sta     $10
        lda     L4F8A
        sta     L0011
        .byte   $AD
L5000:  .byte   $8B
        bbr4    $8D,L4F97
        bbr4    $AD,L4F93
        bbr4    $8D,L4F9E
        bbr4    $AD,L4F9A
        bbr4    $85,$5014
        sta     L4F95
        lda     L4F96
        sta     L4F97
        stz     L50C2
L501C:  bsr     L4FDF
        .byte   $20
        .byte   $97
L5020:  bvc     $4FB2
        sbc     $8520,y
        bvc     $4FF7
        ora     ($60,x)
L5029:  jsr     L5088
        bcc     L501C
        jsr     L50A6
        bcc     L501C
        jsr     L50B3
        bcc     L501C
        lda     L50C2
        beq     L5050
        stz     $3B33
        jsr     LE063
        lda     $2228
        bne     L5050
L5048:  lda     $3B33
        beq     L5048
        jsr     L51AE
L5050:  jsr     L50C3
        bsr     L507E
        inc     L4F8B
        dec     $04
        beq     L505F
        jmp     L501C

L505F:  jsr     L4FDF
        bsr     L5085
        bne     L5067
        rts

L5067:  bsr     L5088
        bcc     L501C
        bsr     L5097
        bcc     L5078
        bsr     L50A6
        bcc     L5078
        jsr     L50B3
        bcc     L5078
L5078:  jsr     L50DF
        jmp     L501C

L507E:  inc     L0000
        bne     L5084
        inc     $01
L5084:  rts

L5085:  lda     $F8
        rts

L5088:  lda     $F8
        cmp     #$01
        bne     L5095
        jsr     L50DF
        bsr     L507E
        clc
        rts

L5095:  sec
        rts

L5097:  lda     $F8
        cmp     #$04
        bne     L50A4
        jsr     L5198
        bsr     L507E
        clc
        rts

L50A4:  sec
        rts

L50A6:  lda     $F8
        cmp     #$02
        bne     L50A4
        stz     L50C2
        bsr     L507E
        clc
        rts

L50B3:  lda     $F8
        cmp     #$03
        bne     L50A4
        lda     #$FF
        sta     L50C2
        bsr     L507E
        clc
        rts

L50C2:  brk
L50C3:  lda     L0000
        pha
        lda     $01
        pha
        lda     $04
        pha
        lda     $05
        pha
        jsr     L5220
        pla
        sta     $05
        pla
        sta     $04
        pla
        sta     $01
        pla
        sta     L0000
        rts

L50DF:  dec     L4F97
        bne     L50E6
        bsr     L50F5
L50E6:  lda     L4F95
        sta     $04
        inc     L4F8C
        lda     L4F93
L50F1:  sta     L4F8B
        rts

L50F5:  .byte   $20
L50F6:  cmp     $54
        jsr     L54AF
        bsr     L510B
        lda     L4F96
L5100:  sta     L4F97
        lda     L4F94
        dec     a
        sta     L4F8C
        rts

L510B:  bsr     L518A
        lda     L4F8D
        pha
L5111:  lda     L4F8E
        pha
        bsr     L5172
        lda     L4FDE
        bne     L5120
        bsr     L517D
        bra     L5128
L5120:  lda     #$1A
L5122:  jsr     L4F66
        dec     a
        bne     L5122
L5128:  pla
        sta     L4F8E
        pla
        sta     L4F8D
        lda     L4F9D
        sta     $06
        lda     L4F9E
        sta     $07
        lda     L4F93
        sta     L4F8B
        lda     L4F94
        sta     L4F8C
        jsr     L526D
        clc
        lda     $06
        adc     #$04
        sta     $06
        bcc     L5154
        inc     $07
L5154:  ldx     L4F8D
        ldy     L4F8E
L515A:  phx
        lda     $0E
        pha
        lda     $0F
        pha
        jsr     L55E0
        pla
        sta     $0F
        pla
        sta     $0E
        jsr     L5213
        plx
        dey
        bne     L515A
        rts

L5172:  lda     #$01
        sta     L5C22
        lda     #$01
        sta     L5C23
        rts

L517D:  jsr     L5BF5
        jsr     L5C8C
        jsr     L5CB0
        jsr     L5C25
        rts

L518A:  lda     L4F8B
        dec     a
        sta     L5C20
        lda     L4F8C
        sta     L5C21
        rts

L5198:  bsr     L518A
        lda     L4F8D
        pha
        lda     L4F8E
        pha
        bsr     L5172
        bsr     L517D
        pla
        sta     L4F8E
        pla
        sta     L4F8D
L51AE:  lda     $F8
        pha
        lda     $F9
        pha
        lda     $FE
        pha
        lda     $FF
        pha
        lda     #$09
        sta     $F8
        .byte   $A9
L51BF:  .byte   $0B
        sta     $FF
        jsr     LE0D8
        pla
        sta     $FF
        pla
        sta     $FE
        pla
        sta     $F9
        pla
        sta     $F8
        rts

L51D2:  clc
        lda     L4F9A
        .byte   $6D
        .byte   $D9
L51D8:  bbr4    $85,$51DB
        lda     L4F9B
        adc     L4FDA
        sta     $01
        ldx     L4F91
        bne     L51E9
        rts

L51E9:  jsr     L507E
        lda     (L0000)
        cmp     #$00
        bne     L51E9
        dex
        bne     L51E9
        jsr     L507E
        rts

L51F9:  stz     $0E
        lda     L4F8C
        lsr     a
        .byte   $66
L5200:  asl     $664A
        asl     $0F85
        clc
        lda     $0E
        adc     L4F8B
        sta     $0E
        bcc     L5212
        inc     $0F
L5212:  rts

L5213:  clc
        lda     $0E
        adc     #$40
        sta     $0E
        bcc     L521E
        inc     $0F
L521E:  rts

        brk
L5220:  lda     L4FD8
        cmp     #$7F
        bcc     L522E
        bne     L522E
        lda     L4FD7
        cmp     #$C1
L522E:  bcs     L524E
        lda     L4FD7
        pha
        lda     L4FD8
        pha
        lda     L4F8F
        sta     $FF
        bsr     L5280
        pla
        sta     $07
        pla
        sta     $06
        bsr     L526D
        dec     $5A
        bsr     L524F
        stz     $5A
        clc
L524E:  rts

L524F:  jsr     L54A0
        jsr     L5600
        rts

L5256:  jsr     L54A0
        jsr     L5600
        clc
        lda     $06
        adc     #$02
        sta     $06
        bcc     L5267
        inc     $07
L5267:  bsr     L5213
        dey
        bne     L5256
        rts

L526D:  jsr     L51F9
        ldx     #$04
L5272:  lsr     $07
        ror     $06
        dex
        bne     L5272
        lda     $07
        ora     #$F0
        sta     $07
        rts

L5280:  lda     L4FD5
        sta     $FA
        lda     L4FD6
        sta     $FB
        jsr     L5DD5
        bsr     L52A2
        ldx     #$03
        bsr     L52C8
        clc
        lda     L4FD7
        adc     #$10
        sta     L4FD7
        bcc     L52A1
        inc     L4FD8
L52A1:  rts

L52A2:  lda     L4FD7
        sta     $06
        lda     L4FD8
        sta     $07
        clc
        lda     L4FD5
        adc     #$20
        sta     $04
        cla
        adc     L4FD6
        sta     $05
        lda     #$10
        sta     $15
        inc     a
        sta     $14
        cla
        sta     $17
        inc     a
        sta     $16
        rts

L52C8:  lda     $14,x
        clc
        adc     L4FD5
        sta     L0000
        cla
        adc     L4FD6
        sta     $01
        jsr     L52FD
        rts

L52DA:  lda     L0011
        pha
        ldx     #$03
L52DF:  phx
        lda     $14,x
        tay
        lsr     L0011
        bcc     L52EB
        lda     #$FF
        bra     L52EC
L52EB:  cla
L52EC:  ldx     #$08
L52EE:  sta     ($04),y
        iny
        iny
        dex
        bne     L52EE
        plx
        dex
        bpl     L52DF
        pla
        sta     L0011
        rts

L52FD:  lda     $10
        pha
        bsr     L52DA
        ldx     #$03
L5304:  phx
        lsr     $10
        bcc     L530F
        lda     #$FF
        sta     $0E
        bra     L5311
L530F:  stz     $0E
L5311:  lda     $14,x
        tax
        cly
        lda     #$08
L5317:  pha
        lda     (L0000),y
        iny
        iny
        sxy
        lsr     $0E
        bcc     L5327
        ora     ($04),y
        sta     ($04),y
        bra     L532D
L5327:  eor     #$FF
        and     ($04),y
        sta     ($04),y
L532D:  sxy
        inx
        inx
        pla
        dec     a
        bne     L5317
        plx
        dex
        bpl     L5304
        dec     $5A
        st0     #$00
        .byte   $A5
L533D:  asl     $8D
        sxy
        brk
        lda     $07
        sta     a:$03
        st0     #$02
        cly
        ldx     #$10
        bsr     L535E
        clc
        lda     $06
        adc     #$10
        sta     $06
        bcc     L5358
        inc     $07
L5358:  stz     $5A
        pla
        sta     $10
L535D:  rts

L535E:  lda     ($04),y
        iny
        sta     a:$02
        lda     ($04),y
        iny
        sta     a:$03
        dex
        bne     L535E
        rts

L536E:  lda     L4FB8
        asl     a
        tax
        lda     L4FD5
L5376:  sta     L4FB9,x
        inx
        lda     L4FD6
        sta     L4FB9,x
        inc     L4FB8
        phx
        stz     $0E
        stz     $0F
        ldx     L4F8E
L538B:  clc
        lda     $0E
        adc     L4F8D
        sta     $0E
        bcc     L5397
        inc     $0F
L5397:  dex
        bne     L538B
        asl     $0E
        rol     $0F
        plx
        clc
        lda     $0E
        adc     L4FD5
        sta     $0E
        lda     $0F
        adc     L4FD6
        sta     $0F
        lda     $0F
        cmp     #$DF
        bcc     L53BA
        bne     L53BA
        lda     $0E
        cmp     #$F0
L53BA:  bcc     L53C4
        stz     L4FB9,x
        dex
        stz     L4FB9,x
        rts

L53C4:  jsr     L51F9
        lda     L4FD5
        sta     $04
        lda     L4FD6
        sta     $05
        ldx     #$04
        cly
L53D4:  lda     L4F8B,y
        sta     ($04),y
        iny
        dex
        bne     L53D4
        lda     $0E
        sta     ($04),y
        iny
        lda     $0F
        sta     ($04),y
        bsr     L542D
        ldy     L4F8E
        ldx     L4F8D
L53EE:  phy
        phx
        bsr     L5403
        plx
        ply
        dey
        bne     L53EE
        lda     $04
        sta     L4FD5
        lda     $05
        sta     L4FD6
        clc
        rts

L5403:  phx
        cly
        dec     $5A
        bsr     L541E
L5409:  lda     a:$02
        sta     ($04),y
        iny
        lda     a:$03
        sta     ($04),y
        iny
        dex
        bne     L5409
        stz     $5A
        pla
        bsr     L5492
        rts

L541E:  st0     #$01
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        rts

L542D:  clc
        lda     $04
        adc     #$06
        sta     $04
        bcc     L5438
        inc     $05
L5438:  rts

L5439:  dec     L4FB8
        lda     L4FB8
        asl     a
        tax
        lda     L4FB9,x
        sta     $04
        inx
        lda     L4FB9,x
        sta     $05
        lda     $04
        bne     L5455
        lda     $05
        bne     L5455
        rts

L5455:  lda     $04
        sta     L4FD5
        lda     $05
        sta     L4FD6
        ldy     #$04
        lda     ($04),y
        sta     $0E
        iny
        lda     ($04),y
        sta     $0F
        ldy     #$02
        lda     ($04),y
        tax
        iny
        lda     ($04),y
        tay
        bsr     L542D
L5475:  phy
        phx
        jsr     L4F7A
        bsr     L5482
        plx
        ply
        dey
        bne     L5475
        rts

L5482:  phx
        cly
        dec     $5A
        jsr     L54A0
        jsr     L535E
        stz     $5A
        pla
        bsr     L5492
        rts

L5492:  asl     a
        clc
        adc     $04
        sta     $04
        bcc     L549C
        inc     $05
L549C:  jsr     L5213
        rts

L54A0:  st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        rts

L54AF:  lda     L4F9F
        asl     a
        tax
        lda     L4FD7
        sta     L4FA0,x
        inx
        lda     L4FD8
        sta     L4FA0,x
        inc     L4F9F
        rts

L54C5:  dec     L4F9F
        lda     L4F9F
        asl     a
        tax
        lda     L4FA0,x
        sta     L4FD7
        inx
        lda     L4FA0,x
        sta     L4FD8
        rts

L54DB:  jsr     L536E
        bcs     L54E2
        bsr     L54EF
L54E2:  rts

L54E3:  dec     L4F9C
        bne     L54EB
        jsr     L54C5
L54EB:  jsr     L5439
        rts

L54EF:  lda     L4FD8
        cmp     #$7F
        bcc     L54FD
        bne     L54FD
        lda     L4FD7
        cmp     #$31
L54FD:  bcs     L5522
        lda     L4F9C
        bne     L551F
        lda     L4FD7
        sta     L4F9D
        lda     L4FD8
        sta     L4F9E
        bsr     L54AF
        lda     L4F92
        sta     $10
        lda     L4F8A
        sta     L0011
        jsr     L560B
L551F:  bsr     L5523
        clc
L5522:  rts

L5523:  inc     L4F9C
        lda     L4F9D
        sta     $06
        lda     L4F9E
        sta     $07
        jsr     L526D
        lda     L4F8D
        sta     L558D
        lda     L4F8E
        sta     L558E
        ldx     #$04
        stx     L4F8D
        stx     L4F8E
L5547:  lda     $0E
        pha
        lda     $0F
        pha
        lda     $06
        pha
        lda     $07
        pha
        jsr     L4F7A
        bsr     L558F
        pla
        sta     $07
        pla
        sta     $06
L555E:  pla
        sta     $0F
        pla
        sta     $0E
        lda     L4F8D
        cmp     L558D
        beq     L556F
        inc     L4F8D
L556F:  lda     L4F8E
        cmp     L558E
        beq     L557A
        inc     L4F8E
L557A:  lda     L4F8D
        cmp     L558D
        bne     L5547
        lda     L4F8E
        cmp     L558E
        bne     L5547
        bsr     L558F
        rts

L558D:  brk
L558E:  brk
L558F:  bsr     L55B6
        jsr     L5213
        bsr     L55AA
        ldx     L4F8E
        dex
        dex
L559B:  phx
        bsr     L55B6
        jsr     L5213
        plx
        dex
        bne     L559B
        bsr     L55AA
        bsr     L55B6
        rts

L55AA:  clc
        lda     $06
        adc     #$03
        sta     $06
        bcc     L55B5
        inc     $07
L55B5:  rts

L55B6:  lda     $06
        pha
        lda     $07
        pha
        lda     $0E
        pha
        lda     $0F
        pha
        bsr     L55F6
        ldx     L4F8D
        dex
        dex
        bsr     L55EF
        bsr     L55E8
        bsr     L55E0
        bsr     L55EF
        bsr     L55F6
        pla
        sta     $0F
        pla
        sta     $0E
        pla
        sta     $07
        pla
        sta     $06
        rts

L55E0:  bsr     L55F6
        bsr     L55E8
        dex
        bne     L55E0
        rts

L55E8:  inc     $0E
        bne     L55EE
        inc     $0F
L55EE:  rts

L55EF:  inc     $06
        bne     L55F5
        .byte   $E6
L55F4:  .byte   $07
L55F5:  rts

L55F6:  dec     $5A
        jsr     L54A0
        bsr     L5600
        stz     $5A
L55FF:  rts

L5600:  lda     $06
        sta     a:$02
        lda     $07
        sta     a:$03
        rts

L560B:  clc
        lda     L4FD5
        adc     #$11
        sta     $02
        cla
        adc     L4FD6
L5617:  sta     $03
        lda     #$67
        sta     L0000
        lda     #$56
        sta     $01
        ldx     #$09
L5623:  phx
        lda     L0000
        pha
        lda     $01
        pha
L562A:  bsr     L5657
        jsr     L52A2
        clx
        jsr     L52C8
        clc
        lda     L4FD7
        adc     #$10
        sta     L4FD7
        .byte   $90
L563D:  st0     #$EE
        cld
        .byte   $4F
L5641:  pla
        sta     $01
        pla
        sta     L0000
        plx
        clc
        lda     L0000
        adc     #$08
        sta     L0000
        bcc     L5653
        inc     $01
L5653:  dex
        bne     L5623
L5656:  rts

L5657:  clx
        cly
L5659:  lda     (L0000),y
        sxy
        sta     ($02),y
        iny
        iny
        inx
        sxy
        cpy     #$08
        bne     L5659
        rts

        bbs7    $FF,L562A
        cpy     #$C0
        cpy     #$C0
        cpy     #$FF
        bbs7    L0000,L5673
L5673:  brk
        brk
        brk
        brk
        bbs7    $FF,L567D
        st0     #$03
        .byte   $03
L567D:  st0     #$03
        cpy     #$C0
        cpy     #$C0
        cpy     #$C0
        cpy     #$C0
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        st0     #$03
        st0     #$03
        st0     #$03
        st0     #$03
        cpy     #$C0
        cpy     #$C0
        cpy     #$C0
        cpy     #$FF
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        bbs7    $03,L56AC
        st0     #$03
        .byte   $03
L56AC:  st0     #$03
        .byte   $FF
L56AF:  stx     L4F91
        asl     a
        tay
        lda     L4FD9
        sta     L0000
        lda     L4FDA
        sta     $01
        clc
        lda     (L0000),y
        .byte   $6D
        .byte   $D9
L56C3:  .byte   $4F
L56C4:  sta     $0C
        iny
        lda     (L0000),y
        adc     L4FDA
        sta     $0D
        jsr     L5E40
L56D1:  jsr     L573F
        beq     L56E7
        lda     $0C
        pha
        lda     $0D
        pha
        bsr     L572C
L56DE:  jsr     L577E
        pla
        sta     $0D
        pla
        sta     $0C
L56E7:  cly
        lda     ($0C),y
        sta     $08
        iny
        lda     ($0C),y
        sta     $09
        clc
        lda     $0C
        adc     #$02
        sta     $0C
        bcc     L56FC
        inc     $0D
L56FC:  lda     $08
        pha
        lda     $09
        pha
        bsr     L573F
        beq     L5719
        lda     $0C
        pha
        .byte   $A5
L570A:  ora     $4448
        asl     $1C20,x
        eor     $68AA,y
        sta     $0D
        pla
        sta     $0C
        txa
L5719:  pha
L571A:  bsr     L573F
        beq     L5723
L571E:  bsr     L572C
        jsr     L577E
L5723:  plx
        pla
        sta     $0D
        pla
        sta     $0C
        txa
L572B:  rts

L572C:  lda     $0A
        ldx     $0C
        sax
        sta     $0A
        stx     $0C
        lda     L000B
        ldx     $0D
        sax
        sta     L000B
        stx     $0D
        rts

L573F:  cly
        lda     ($0C)
        iny
        ora     ($0C),y
        bne     L5748
        rts

L5748:  bsr     L5761
        rts

L574B:  cly
        lda     ($0C),y
        sta     $0A
        iny
        lda     ($0C),y
        sta     L000B
        clc
        lda     $0C
        adc     #$02
        sta     $0C
        bcc     L5760
        inc     $0D
L5760:  rts

L5761:  cly
        clc
        lda     ($0C),y
        adc     L4FD9
        sta     $0A
        iny
        lda     ($0C),y
        adc     L4FDA
        sta     L000B
        clc
        lda     $0C
        adc     #$02
        sta     $0C
        bcc     L577D
        inc     $0D
L577D:  rts

L577E:  lda     $0C
        sta     $18
        lda     $0D
        sta     $19
L5786:  lda     ($18)
        bne     L578B
        rts

L578B:  asl     a
        tax
        lda     $18
        pha
        lda     $19
        pha
        jmp     (L5796,x)

L5796:  brk
        brk
L5798:  adc     ($58),y
        .byte   $1B
        cli
        bbr7    $58,L572B
        cli
        jmp     (L6258)

        cli
        cla
        cli
        sta     ($58),y
        cmp     ($58)
        .byte   $9B
        cli
        cpy     LC558
        rmb5    $F1
        .byte   $57
L57B2:  pla
        sta     $19
        pla
        sta     $18
        clc
        lda     $18
        adc     $0A
        sta     $18
        bcc     L57C3
        .byte   $E6
L57C2:  .byte   $19
L57C3:  bra     L5786
        lda     $18
        pha
        lda     $19
        pha
        ldy     #$01
        lda     ($18),y
        tax
        iny
        lda     ($18),y
        tay
        txa
        clc
        adc     L4FD9
        sta     $18
        tya
        adc     L4FDA
        sta     $19
        jsr     L5786
        pla
        sta     $19
        pla
        sta     $18
        lda     #$03
        sta     $0A
        jmp     L57B2

        pla
        pla
        rts

L57F4:  ldy     #$01
        lda     ($18),y
        sta     L4F8B
        iny
        lda     ($18),y
        sta     L4F8C
        iny
L5802:  .byte   $B1
L5803:  clc
        sta     L4F8D
        iny
        lda     ($18),y
        sta     L4F8E
        rts

L580E:  iny
        lda     ($18),y
        sta     L4F92
        iny
L5815:  .byte   $B1
L5816:  clc
        sta     L4F8A
        rts

        ldy     #$01
        lda     ($18),y
        sta     L4F8B
        iny
L5823:  lda     ($18),y
        sta     L4F8C
        iny
        lda     ($18),y
        sta     L4F89
        iny
        lda     ($18),y
        sta     L4F8A
        iny
        lda     ($18),y
        sta     L4F8F
        iny
        lda     ($18),y
        sta     L4F98
        iny
        lda     ($18),y
        sta     L4F99
        iny
        lda     ($18),y
        sta     L4F9A
        iny
        lda     ($18),y
        sta     L4F9B
        stz     L4F96
        jsr     L54AF
        jsr     L4FE4
        lda     #$0A
        sta     $0A
        jmp     L57B2

        jsr     L54C5
L5865:  lda     #$01
        sta     $0A
        jmp     L57B2

        .byte   $20
        .byte   $AF
L586E:  csl
        bra     L5865
        bsr     L57F4
        bsr     L580E
        jsr     L54DB
        lda     #$07
        sta     $0A
        jmp     L57B2

        jsr     L57F4
        jsr     L536E
        lda     #$05
        sta     $0A
        jmp     L57B2

        jsr     L5439
        bra     L5865
        jsr     L54E3
        lda     #$01
        sta     $0A
        jmp     L57B2

        bsr     L58A7
        bsr     L58D4
L589F:  lda     #$0C
        sta     $0A
        jmp     L57B2

        rts

L58A7:  jsr     L57F4
        jsr     L580E
        iny
        lda     ($18),y
        sta     L4F89
        iny
        lda     ($18),y
        sta     L4F8F
        iny
        lda     ($18),y
        sta     L4F96
        iny
        lda     ($18),y
        sta     L4F9A
L58C5:  iny
        lda     ($18),y
        sta     L4F9B
        rts

        bsr     L58A7
        bsr     L590A
        bra     L589F
        bra     L5865
L58D4:  jsr     L54DB
        inc     L4F8B
        .byte   $EE
L58DB:  .byte   $8C
L58DC:  .byte   $4F
L58DD:  sec
L58DE:  .byte   $AD
L58DF:  .byte   $8D
L58E0:  bbr4    $E9,$58E5
        sta     L4F8D
        dec     L4F8E
        dec     L4F8E
L58EC:  jsr     L51D2
        jsr     L590F
        jsr     L54AF
        jsr     L4FF5
        lda     L4FDE
        bne     L5900
        jsr     L510B
L5900:  stz     L4FDE
        jsr     L54C5
        jsr     L54E3
        rts

L590A:  jsr     L54DB
        bra     L58EC
L590F:  lda     (L0000)
        cmp     #$05
        bne     L591B
        sta     L4FDE
        jsr     L507E
L591B:  rts

L591C:  lda     $08
        pha
        lda     $09
        pha
        jsr     L5CB0
        jsr     L574B
        lda     $0C
        sta     $18
        lda     $0D
        sta     $19
        clc
        lda     $0C
        adc     $0A
        sta     $0C
        lda     $0D
        adc     L000B
        sta     $0D
        sec
        lda     $0C
        sbc     #$02
        sta     $1A
        lda     $0D
        sbc     #$00
        sta     $1B
        pla
        sta     $0D
        pla
        sta     $0C
L5950:  jsr     L5CF8
L5953:  ldy     #$01
        lda     ($18),y
        asl     a
        bcc     L595E
        cla
        jsr     L5CBF
L595E:  jsr     L5BB0
L5961:  jsr     L5C8C
        ldy     #$01
        lda     ($18),y
        and     #$0F
        cmp     #$00
        bne     L5971
        jmp     L5A56

L5971:  cmp     #$01
        bne     L5978
        jmp     L5B2D

L5978:  ldy     #$05
        lda     $08
L597C:  iny
        asl     a
        bcc     L597C
        lda     ($18),y
        sta     $09
        beq     L5961
        cmp     #$FF
        beq     L59E8
        cmp     #$FE
        bne     L5991
        jmp     L5A0A

L5991:  sta     $0A
        lda     $0A
        pha
        lda     L000B
        pha
        ldy     #$01
        lda     ($18),y
        and     #$40
        beq     L59A4
        jsr     L59D1
L59A4:  pla
        stz     L000B
        pla
        sta     $0A
        bbr7    $0A,L59AF
        dec     L000B
L59AF:  clc
        lda     $18
        adc     $0A
        sta     $18
        lda     $19
        adc     L000B
        sta     $19
        lda     $0C
        pha
        lda     $0D
        pha
        jsr     L5C25
        pla
        sta     $0D
        pla
        sta     $0C
        jsr     L4F66
        jmp     L5953

L59D1:  lda     $1A
        pha
        lda     $1B
        pha
        ldy     #$10
        lda     ($18),y
        jsr     L5AFA
        jsr     L5D37
        pla
        sta     $1B
        pla
        sta     $1A
        rts

L59E8:  jsr     L5C2C
        ldy     #$01
        lda     ($18),y
        and     #$0F
        cmp     #$00
        bne     L59FA
        jsr     L5A13
        bra     L5A01
L59FA:  cmp     #$01
        bne     L5A07
        jsr     L5A3D
L5A01:  inc     a
        beq     L5A07
        jmp     L5950

L5A07:  lda     #$FF
        rts

L5A0A:  pha
        jsr     L5C25
        jsr     L5CB0
        pla
        rts

L5A13:  lda     $1A
        pha
        lda     $1B
        pha
        jsr     L5A2C
        jsr     L5D71
        ldy     #$03
        lda     ($1A),y
        ply
        sty     $1B
        ply
        sty     $1A
        bsr     L5A33
        rts

L5A2C:  jsr     L5B04
        jsr     L5AF2
        rts

L5A33:  cmp     #$00
        beq     L5A3B
        jsr     L5CBF
        rts

L5A3B:  dec     a
        rts

L5A3D:  lda     $1A
        pha
        lda     $1B
        pha
        jsr     L5A2C
        jsr     L5D58
        ldy     #$02
        lda     ($1A),y
        ply
        sty     $1B
        ply
        sty     $1A
        bsr     L5A33
        rts

L5A56:  jsr     L5B1B
        cmp     #$FF
        bne     L5A60
        jmp     L5978

L5A60:  lda     $1A
        pha
        lda     $1B
        pha
        jsr     L5AF2
        jsr     L5B04
        jsr     L5D71
        lda     L4FD3
        bne     L5AAE
        ldy     #$0F
        lda     ($18),y
        and     #$7F
        cmp     $09
        beq     L5A8D
        sec
        lda     $1A
        sbc     #$04
        sta     $1A
        lda     $1B
        sbc     #$00
        sta     $1B
        bra     L5AC2
L5A8D:  jsr     L5B04
        lda     $09
        dec     a
        sta     L0000
        stz     $01
        asl     L0000
        rol     $01
        asl     L0000
        rol     $01
        clc
        lda     $1A
        adc     L0000
        sta     $1A
        lda     $1B
        adc     $01
        sta     $1B
        bra     L5AC2
L5AAE:  dec     $09
        beq     L5ABF
        clc
        lda     $1A
        adc     #$04
        sta     $1A
        bcc     L5ABD
        inc     $1B
L5ABD:  bra     L5AC2
L5ABF:  jsr     L5B04
L5AC2:  jsr     L5AD1
        pla
        sta     $1B
        pla
        sta     $1A
        jsr     L4F7A
        jmp     L5961

L5AD1:  ldy     #$0E
        lda     ($18),y
        tay
        lda     ($1A)
        sta     ($0C),y
        jsr     L5C25
        ldy     #$02
        lda     ($18),y
        sta     L5C20
        ldy     #$01
        lda     ($1A),y
        sta     L5C21
        jsr     L5BF5
        jsr     L5C2C
        rts

L5AF2:  ldy     #$0E
        lda     ($18),y
        tay
        lda     ($0C),y
        rts

L5AFA:  tax
        ldy     #$0E
        lda     ($18),y
        tay
        txa
        sta     ($0C),y
        rts

L5B04:  tax
        ldy     #$0F
        lda     ($18),y
        and     #$7F
        sta     $09
        clc
        lda     $18
        adc     #$10
        sta     $1A
        cla
        adc     $19
        sta     $1B
        txa
        rts

L5B1B:  bbs6    $08,L5B24
        bbs4    $08,L5B28
        lda     #$FF
        rts

L5B24:  lda     #$01
        bra     L5B29
L5B28:  cla
L5B29:  sta     L4FD3
        rts

L5B2D:  jsr     L5B1B
        cmp     #$FF
        bne     L5B37
        jmp     L5978

L5B37:  lda     $1A
        pha
        lda     $1B
        pha
        jsr     L5B04
        jsr     L5AF2
        jsr     L5D58
        lda     L4FD3
        bne     L5B84
        ldy     #$0F
        lda     ($18),y
        and     #$7F
        cmp     $09
        bne     L5B75
        dec     a
        sta     L0000
        stz     $01
        asl     L0000
        rol     $01
        clc
        adc     L0000
        sta     L0000
        cla
        adc     $01
        clc
        lda     $1A
        adc     L0000
        sta     $1A
        lda     $1B
        adc     $01
        sta     $1B
        bra     L5B99
L5B75:  sec
        lda     $1A
        sbc     #$03
        sta     $1A
        lda     $1B
        sbc     #$00
        sta     $1B
        bra     L5B99
L5B84:  lda     $09
        dec     a
        bne     L5B8E
        jsr     L5B04
        bra     L5B99
L5B8E:  clc
        lda     $1A
        adc     #$03
        sta     $1A
        bcc     L5B99
        inc     $1B
L5B99:  lda     ($1A)
        jsr     L5AFA
        jsr     L5D7F
        pla
        sta     $1B
        pla
        sta     $1A
        jsr     L5C9F
        jmp     L595E

        jmp     L5961

L5BB0:  pha
        cmp     #$00
        beq     L5BD7
        ldy     #$03
        lda     ($18),y
        sta     L5C21
L5BBC:  ldy     #$02
        lda     ($18),y
        sta     L5C20
        ldy     #$04
        lda     ($18),y
        sta     L5C22
        iny
        lda     ($18),y
        sta     L5C23
        bsr     L5BF5
        jsr     L5C2C
        pla
        rts

L5BD7:  jsr     L5AF2
        ldy     $1A
        phy
        ldy     $1B
        phy
        jsr     L5B04
        jsr     L5D71
        ldy     #$01
        lda     ($1A),y
        sta     L5C21
        pla
        sta     $1B
        pla
        sta     $1A
        bra     L5BBC
L5BF5:  ldy     #$04
        clx
L5BF8:  lda     L5C20,x
        sta     L4F8B,x
        inx
        dey
        bne     L5BF8
        stz     L4FD1
        rts

L5C06:  dec     L4FD1
        beq     L5C0C
        rts

L5C0C:  lda     L4FD2
        eor     #$01
        sta     L4FD2
        beq     L5C1B
        jsr     L5C25
        cla
        rts

L5C1B:  jsr     L5C2C
        cla
        rts

L5C20:  brk
L5C21:  brk
L5C22:  brk
L5C23:  brk
L5C24:  brk
L5C25:  lda     #$F0
        sta     L5C24
        bra     L5C31
L5C2C:  lda     #$EF
        sta     L5C24
L5C31:  jsr     L536E
        lda     L4FB8
        dec     a
        asl     a
        tax
        lda     L4FB9,x
        clc
        adc     #$06
        sta     $04
        inx
        cla
        adc     L4FB9,x
        sta     $05
        ldy     L4F8E
        ldx     L4F8D
L5C4F:  phy
        phx
        bsr     L5C69
        plx
        ply
        dey
        bne     L5C4F
        lda     L4FD4
        pha
        lda     #$01
        sta     L4FD4
        jsr     L5439
        pla
        sta     L4FD4
        rts

L5C69:  lda     L5C24
        cmp     #$EF
        beq     L5C74
        lda     #$0D
        bra     L5C76
L5C74:  lda     #$2D
L5C76:  sta     L5C7E
        phx
        cly
L5C7B:  iny
        lda     ($04),y
L5C7E:  ora     L5C24
        sta     ($04),y
        iny
        dex
        bne     L5C7B
        pla
        jsr     L5492
        rts

L5C8C:  jsr     L5C06
        beq     L5C94
        jsr     L5C9F
L5C94:  jsr     LE063
        lda     $222D
        beq     L5C8C
        sta     $08
        rts

L5C9F:  lda     L4FD4
        pha
        lda     #$07
        sta     L4FD4
        jsr     L4F7A
        pla
        sta     L4FD4
        rts

L5CB0:  pha
        phx
        phy
L5CB3:  jsr     LE063
        lda     $2228
        bne     L5CB3
        ply
        plx
        pla
        rts

L5CBF:  tax
        lda     $0C
        pha
        lda     $0D
        pha
        lda     $1A
        pha
        lda     $1B
        pha
        lda     $18
        pha
        lda     $19
        pha
        txa
        asl     a
        asl     a
        asl     a
        clc
        adc     $1A
        sta     $0C
        cla
        adc     $1B
        sta     $0D
        jsr     L56D1
        tax
L5CE4:  pla
        sta     $19
        pla
        sta     $18
        pla
        sta     $1B
        pla
        sta     $1A
        pla
        sta     $0D
        pla
        sta     $0C
        txa
        rts

L5CF8:  lda     $1A
        pha
        lda     $1B
        pha
        lda     $18
        pha
        lda     $19
        pha
L5D04:  ldy     #$01
        lda     ($18),y
        and     #$0F
        cmp     #$01
        bne     L5D13
        jsr     L5D37
        .byte   $80
L5D12:  .byte   $06
L5D13:  jsr     L5B04
        jsr     L5D47
L5D19:  cly
        lda     ($18),y
        clc
        adc     $18
        sta     $18
        bcc     L5D25
        inc     $19
L5D25:  lda     ($18),y
        inc     a
        bne     L5D04
        pla
        sta     $19
        pla
        sta     $18
        pla
        sta     $1B
        pla
        sta     $1A
        rts

L5D37:  jsr     L5B04
        bsr     L5D47
        jsr     L5AF2
        jsr     L5D58
        jsr     L5D7F
        cla
        rts

L5D47:  ldy     #$0F
        lda     ($18),y
        asl     a
        bcc     L5D57
        ldy     #$0E
        lda     ($18),y
        tay
        lda     ($1A)
        sta     ($0C),y
L5D57:  rts

L5D58:  cly
L5D59:  cmp     ($1A),y
        beq     L5D65
        iny
        iny
        iny
        dec     $09
        bne     L5D59
        brk
L5D65:  tya
        clc
        adc     $1A
        sta     $1A
        cla
        adc     $1B
        sta     $1B
        rts

L5D71:  cly
L5D72:  cmp     ($1A),y
        beq     L5D65
        iny
        iny
        iny
        iny
        dec     $09
        bne     L5D72
        brk
L5D7F:  ldy     #$02
        lda     ($18),y
        sta     L4F8B
        iny
        lda     ($18),y
        sta     L4F8C
        jsr     L51F9
        cly
        dec     $5A
        .byte   $20
L5D93:  asl     $AD54,x
        sxy
        brk
        sta     $200E,y
        iny
        lda     a:$03
        sta     $200E,y
        stz     $5A
        ldx     #$04
L5DA6:  asl     $0E
        rol     $0F
        dex
        bne     L5DA6
        lda     L4FD7
        pha
        lda     L4FD8
        pha
        lda     $0E
        sta     L4FD7
        lda     $0F
        sta     L4FD8
        ldy     #$01
        lda     ($1A),y
        sta     L4F91
        jsr     L51D2
        jsr     L4FF5
        pla
        sta     L4FD8
        pla
        sta     L4FD7
        rts

L5DD5:  phx
        phy
        lda     $F8
        stz     $F8
        stz     $F9
        sec
        sbc     #$20
        asl     a
        rol     $F9
        asl     a
        rol     $F9
        asl     a
        rol     $F9
        clc
        adc     #$C7
        sta     $F8
        lda     $F9
        adc     #$61
        sta     $F9
        .byte   $A0
L5DF5:  rmb0    $43
        php
        pha
        clc
        lda     $FFF5
        adc     #$01
        tam     #$08
L5E01:  lda     ($F8),y
        sta     L5E1F,y
        dey
        bpl     L5E01
        pla
        tam     #$08
        ldy     #$07
        .byte   $A2
L5E0F:  .byte   $0E
L5E10:  lda     L5E1F,y
        sxy
        sta     ($FA),y
L5E16:  sxy
        dex
        dex
        dey
        bpl     L5E10
        ply
        plx
L5E1E:  rts

L5E1F:  brk
        brk
        brk
        brk
        brk
        brk
        brk
L5E26:  brk
L5E27:  .byte   $9C
L5E28:  .byte   $9C
L5E29:  .byte   $4F
L5E2A:  .byte   $73
L5E2B:  stz     L9D4F
        bbr4    $37,$5E31
        bsr     L5E4D
        bsr     L5E40
        lda     $0C
        sta     L4FD9
        lda     $0D
        sta     L4FDA
        rts

L5E40:  lda     L4FDB
        sta     L4FD5
        lda     L4FDC
        sta     L4FD6
        rts

L5E4D:  lda     #$E0
        sta     $0402
        lda     #$00
        sta     $0403
        tia     $5E5F,$0404,$0040
        rts

        bbs0    L0000,L5E8A
        ora     ($B6,x)
        ora     ($80,x)
        ora     ($EB,x)
        brk
        cpx     #$00
L5E6B:  bbs7    L0000,L5E6E
L5E6E:  ora     ($D9,x)
        brk
        rmb7    $01
        cpx     #$00
        .byte   $AB
        ora     ($0A,x)
        brk
        bbs1    L0000,L5EE9
        ora     ($21,x)
        ora     ($8F,x)
L5E80:  brk
L5E81:  sax
        ora     ($B6,x)
        ora     ($01,x)
        brk
        tst     #$00,$22
L5E8A:  brk
        .byte   $DC
        ora     ($02,x)
        brk
        sta     (L0000),y
        bbs7    $01,L5E9C
        brk
        bbs7    $01,L5E6B
        brk
        eor     (L0000,x)
        .byte   $6D
L5E9C:  ora     (L0000,x)
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        .byte   $FF
        .byte   $FF
L5EAD:  bbs7    $FF,$5EAF
L5EB0:  bbs7    $FF,$5EB2
L5EB3:  bbs7    $FF,$5EB5
L5EB6:  bbs7    $FF,$5EB8
L5EB9:  bbs7    $FF,$5EBB
L5EBC:  bbs7    $FF,$5EBE
L5EBF:  bbs7    $FF,$5EC1
L5EC2:  bbs7    $FF,$5EC4
L5EC5:  bbs7    $FF,$5EC7
L5EC8:  bbs7    $FF,$5ECA
L5ECB:  bbs7    $FF,$5ECD
L5ECE:  bbs7    $FF,$5ED0
L5ED1:  bbs7    $FF,$5ED3
L5ED4:  bbs7    $FF,$5ED6
L5ED7:  bbs7    $FF,$5ED9
L5EDA:  bbs7    $FF,$5EDC
L5EDD:  bbs7    $FF,$5EDF
L5EE0:  bbs7    $FF,$5EE2
L5EE3:  bbs7    $FF,$5EE5
L5EE6:  bbs7    $FF,$5EE8
L5EE9:  .byte   $FF
        .byte   $FF
L5EEB:  bbs7    $FF,$5EED
L5EEE:  bbs7    $FF,$5EF0
L5EF1:  bbs7    $FF,$5EF3
L5EF4:  bbs7    $FF,$5EF6
L5EF7:  bbs7    $FF,$5EF9
L5EFA:  bbs7    $FF,$5EFC
L5EFD:  bbs7    $FF,$5EFF
L5F00:  bbs7    $FF,$5F02
L5F03:  bbs7    $FF,$5F05
L5F06:  bbs7    $FF,$5F08
L5F09:  bbs7    $FF,$5F0B
L5F0C:  bbs7    $FF,$5F0E
L5F0F:  bbs7    $FF,$5F11
L5F12:  bbs7    $FF,$5F14
L5F15:  bbs7    $FF,$5F17
L5F18:  bbs7    $FF,$5F1A
L5F1B:  bbs7    $FF,$5F1D
L5F1E:  bbs7    $FF,$5F20
L5F21:  bbs7    $FF,$5F23
L5F24:  bbs7    $FF,$5F26
L5F27:  bbs7    $FF,$5F29
L5F2A:  bbs7    $FF,$5F2C
L5F2D:  bbs7    $FF,$5F2F
L5F30:  bbs7    $FF,$5F32
L5F33:  bbs7    $FF,$5F35
L5F36:  bbs7    $FF,$5F38
L5F39:  bbs7    $FF,$5F3B
L5F3C:  bbs7    $FF,$5F3E
L5F3F:  bbs7    $FF,$5F41
L5F42:  bbs7    $FF,$5F44
L5F45:  bbs7    $FF,$5F47
L5F48:  bbs7    $FF,$5F4A
L5F4B:  bbs7    $FF,$5F4D
L5F4E:  bbs7    $FF,$5F50
L5F51:  bbs7    $FF,$5F53
L5F54:  bbs7    $FF,$5F56
L5F57:  bbs7    $FF,$5F59
L5F5A:  bbs7    $FF,$5F5C
L5F5D:  bbs7    $FF,$5F5F
L5F60:  bbs7    $FF,$5F62
L5F63:  bbs7    $FF,$5F65
L5F66:  bbs7    $FF,$5F68
L5F69:  bbs7    $FF,$5F6B
L5F6C:  bbs7    $FF,$5F6E
L5F6F:  bbs7    $FF,$5F71
L5F72:  bbs7    $FF,$5F74
L5F75:  bbs7    $FF,$5F77
L5F78:  bbs7    $FF,$5F7A
L5F7B:  bbs7    $FF,$5F7D
L5F7E:  bbs7    $FF,$5F80
L5F81:  bbs7    $FF,$5F83
L5F84:  bbs7    $FF,$5F86
L5F87:  bbs7    $FF,$5F89
L5F8A:  bbs7    $FF,$5F8C
L5F8D:  bbs7    $FF,$5F8F
L5F90:  bbs7    $FF,$5F92
L5F93:  bbs7    $FF,$5F95
L5F96:  bbs7    $FF,$5F98
L5F99:  bbs7    $FF,$5F9B
L5F9C:  bbs7    $FF,$5F9E
L5F9F:  bbs7    $FF,$5FA1
L5FA2:  bbs7    $FF,$5FA4
L5FA5:  bbs7    $FF,$5FA7
L5FA8:  bbs7    $FF,$5FAA
L5FAB:  bbs7    $FF,$5FAD
L5FAE:  bbs7    $FF,$5FB0
L5FB1:  bbs7    $FF,$5FB3
L5FB4:  bbs7    $FF,$5FB6
L5FB7:  bbs7    $FF,$5FB9
L5FBA:  bbs7    $FF,$5FBC
L5FBD:  bbs7    $FF,$5FBF
L5FC0:  bbs7    $FF,$5FC2
L5FC3:  bbs7    $FF,$5FC5
L5FC6:  bbs7    $FF,$5FC8
L5FC9:  bbs7    $FF,$5FCB
L5FCC:  bbs7    $FF,$5FCE
L5FCF:  bbs7    $FF,$5FD1
L5FD2:  bbs7    $FF,$5FD4
L5FD5:  bbs7    $FF,$5FD7
L5FD8:  bbs7    $FF,$5FDA
L5FDB:  bbs7    $FF,$5FDD
L5FDE:  bbs7    $FF,$5FE0
L5FE1:  bbs7    $FF,$5FE3
L5FE4:  bbs7    $FF,$5FE6
L5FE7:  bbs7    $FF,$5FE9
L5FEA:  bbs7    $FF,$5FEC
L5FED:  bbs7    $FF,$5FEF
L5FF0:  bbs7    $FF,$5FF2
L5FF3:  bbs7    $FF,$5FF5
L5FF6:  bbs7    $FF,$5FF8
L5FF9:  bbs7    $FF,$5FFB
L5FFC:  bbs7    $FF,$5FFE
L5FFF:  .byte   $FF
L6000:  brk
        rmb2    $60
L6003:  .byte   $0B
        brk
        brk
        .byte   $0B
        ora     ($06,x)
        brk
        bbs5    $60,L600D
L600D:  tdd     $60,$60AF,$9E00
        rts

        brk
        sta     $60,y
        sty     $60,x
        brk
        bbs0    $60,L601F
L601F:  .byte   $3B
        rts

        brk
        sta     $60
        brk
        txa
        rts

        ora     (L0000,x)
        brk
        .byte   $3B
        rts

        ora     (L0000,x)
        php
        bbs2    $60,L6033
        brk
L6033:  ora     #$C3
        rts

        ora     (L0000,x)
        bbr0    $D7,L609B
        ora     (L0038)
        adc     ($10,x)
        brk
        ora     ($09)
        adc     ($08,x)
        brk
        and     $26
        and     ($16,x)
        say
        bbs7    L0000,L604D
L604D:  bpl     L6050
        brk
L6050:  phy
        rts

        ora     ($09)
        adc     ($10,x)
        st0     #$00
        phy
        rts

        ora     ($5B)
        adc     ($01,x)
        asl     a
        brk
        bra     L60C2
        ora     ($0A,x)
        ora     ($85,x)
        rts

        ora     ($0A,x)
        sxy
        txa
        rts

        ora     ($0A,x)
        st0     #$8F
        rts

        .byte   $01
L6072:  asl     a
        tsb     $94
        rts

        ora     ($0A,x)
        ora     $99
        rts

        ora     ($0A,x)
        asl     $9E
        rts

        bpl     L6086
        brk
        tst     #$60,$10,x
L6086:  ora     L0000
        tst     #$60,$10,x
        asl     L0000
        tst     #$60,$10,x
        rmb0    L0000
        tst     #$60,$10,x
        php
        brk
        tst     #$60,$10,x
        .byte   $09
L609B:  brk
        tst     #$60,$10,x
        asl     a
        brk
        tst     #$60,$42,x
        bbs7    L0000,L60A7
L60A7:  asl     $21,x
        ora     ($18)
        cpx     #$12
        cpy     $61
        ora     ($7E)
        adc     ($10,x)
        .byte   $0B
        brk
        eor     ($60)
        say
        bbs7    L0000,L60BB
L60BB:  asl     $21,x
        ora     ($18)
        cpx     #$00
        .byte   $52
L60C2:  rts

        ora     ($A1)
        adc     ($10,x)
        tsb     L5200
        rts

        say
        bbs7    L0000,L60CF
L60CF:  asl     $21,x
        ora     ($18)
        cpx     #$00
        eor     ($60)
        ora     ($09)
        adc     ($10,x)
        tsb     $DF00
        rts

        ora     ($09)
        adc     ($22,x)
        php
        bpl     L60E6
L60E6:  ora     ($C4)
        adc     ($08,x)
        brk
        and     $26
        and     ($16,x)
        say
        bbs7    L0000,L60F3
L60F3:  ora     ($C4)
        adc     ($37,x)
        brk
        and     $34
        brk
        jsr     L0011
        brk
        bmi     L610C
        asl     a
        brk
        .byte   $0B
        .byte   $0B
        tsb     a:$23
        .byte   $09
L6109:  lda     #$EA
        .byte   $8D
L610C:  cmp     L8D40,y
        phx
        rti

        sta     L40DB
        rts

        lda     #$4C
        sta     L40D9
        lda     #$25
        sta     L40DA
        lda     #$61
        sta     L40DB
        rts

        jsr     L6109
        sei
        ldx     #$FF
        txs
        cli
        lda     #$E9
        sta     $1C
        lda     #$60
        sta     $1D
        jmp     L40DC

        lda     #$4C
        sta     L40D9
        lda     #$48
        sta     L40DA
        lda     #$61
        sta     L40DB
        rts

        jsr     L6109
        sei
        ldx     #$FF
        txs
        cli
        lda     #$40
        sta     $1C
        lda     #$60
        sta     $1D
        jmp     L40DC

        lda     #$4C
        .byte   $8D
L615E:  cmp     $A940,y
        .byte   $6B
        sta     L40DA
L6165:  lda     #$61
        sta     L40DB
        rts

        jsr     L6109
        sei
        ldx     #$FF
        txs
        cli
        lda     #$A3
        sta     $1C
        lda     #$60
        sta     $1D
        jmp     L40DC

        lda     #$4C
        sta     L40D9
        lda     #$8E
        sta     L40DA
        lda     #$61
        sta     L40DB
        rts

        jsr     L6109
        sei
        ldx     #$FF
        txs
        cli
        lda     #$B7
        sta     $1C
        lda     #$60
        sta     $1D
        jmp     L40DC

        lda     #$4C
        sta     L40D9
        lda     #$B1
        sta     L40DA
        lda     #$61
        sta     L40DB
        rts

        jsr     L6109
        sei
        ldx     #$FF
        txs
        cli
        lda     #$CB
        sta     $1C
        lda     #$60
        sta     $1D
        jmp     L40DC

        jmp     L401E

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        clc
        clc
        clc
        clc
        clc
        brk
        clc
        brk
        bvc     L6229
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        plp
        jmp     (L2828,x)

        jmp     (L0028,x)

        bpl     L6221
        csl
        sec
        trb     $54
        sec
        bpl     L6232
        ldy     $48
        bpl     L6218
        lsr     a
        sty     L0000
        sec
        jmp     (L7638)

        cpy     L7ACC
        brk
        rts

        jsr     L0040
        brk
        brk
        brk
        brk
        sxy
        tsb     $08
        php
        php
        tsb     $02
        brk
        rti

        jsr     L1010
        bpl     L6235
        rti

        brk
        .byte   $10
L6218:  sta     ($54)
        sec
        csl
        sta     ($10)
        brk
        bpl     L6231
L6221:  bpl     L6221
        bpl     L6235
        bpl     L6227
L6227:  brk
        brk
L6229:  brk
        brk
        brk
        rts

        jsr     L0040
        brk
L6231:  brk
L6232:  inc     a:L0000,x
L6235:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        rts

        rts

        brk
        sxy
        tsb     $08
        bpl     L6264
        rti

        bra     L6247
L6247:  sec
        stz     $C6
        dec     $C6
        jmp     L0038

        clc
        sec
        clc
        clc
        clc
        clc
        bit     L7C00,x
L6258:  dec     $C6
        trb     $E070
        inc     LFE00,x
        tsb     $3C18
        .byte   $06
L6264:  dec     $7C
        brk
        trb     L6C3C
        cpy     $FECC
        tsb     LFE00
        cpy     #$FC
        asl     $06
        dec     $7C
        brk
        jmp     (LC0C6,x)

        .byte   $FC
        dec     $C6
        jmp     (LFE00,x)

        dec     $0C
        php
        clc
        bpl     L62B6
        brk
        jmp     (LC6C6,x)

        jmp     (LC6C6,x)

        jmp     (L7C00,x)

        dec     $C6
        ror     LC606,x
        jmp     (L1800,x)

        clc
        brk
        brk
        brk
        clc
        clc
        brk
        clc
        clc
        brk
        brk
        brk
        clc
        php
        bpl     L62AA
        tsb     $08
L62AA:  bpl     L62B4
        tsb     $02
        brk
        brk
        brk
        jmp     (L7C00,x)

L62B4:  brk
        brk
L62B6:  brk
        rti

        jsr     L0810
        bpl     L62DD
        rti

        brk
        bit     L6666,x
        tsb     a:$18
        clc
        brk
        sec
        bsr     L6264
        tax
        ldx     $3844,y
        brk
        sec
        jmp     (LC644)

        inc     LC6C6,x
        brk
        .byte   $FC
        dec     $C6
        .byte   $FC
        dec     $C6
L62DD:  .byte   $FC
        brk
        bit     LC066,x
        cpy     #$C0
        ror     $3C
        brk
        sed
        cpy     LC6C6
        dec     $CC
        sed
        brk
        inc     LC0C0,x
        .byte   $FC
        cpy     #$C0
        inc     LFE00,x
        cpy     #$C0
        .byte   $FC
        .byte   $C0
L62FC:  cpy     #$C0
        brk
        bit     LC066,x
        dec     L66C6
        bit     LC600,x
        dec     $C6
        inc     LC6C6,x
        dec     L0000
        .byte   $FC
        bmi     L6342
L6312:  bmi     L6344
        bmi     L6312
        brk
        rol     $0C0C,x
        tsb     $CC0C
        sei
        brk
        dec     $CC
        cld
        beq     L62FC
        cpy     a:$C6
        rts

        rts

        rts

        rts

        rts

        rts

        ror     LC600,x
        dec     $EE
        inc     LC6D6,x
        dec     L0000
        dec     $E6
        inc     $DE,x
        dec     LC6C6
        brk
        .byte   $7C
L6340:  dec     $C6
L6342:  dec     $C6
L6344:  dec     $7C
        brk
        .byte   $FC
        dec     $C6
        dec     $FC
        cpy     #$C0
        brk
        jmp     (LC6C6,x)

        dec     $FE
        dec     $7E
        brk
        .byte   $FC
        dec     $C6
        .byte   $FC
        dec     $C6
        dec     L0000
        jmp     (LC0C6,x)

        jmp     (LC606,x)

        jmp     (LFC00,x)

        bmi     L639A
        bmi     L639C
        bmi     L639E
        brk
        dec     $C6
        dec     $C6
        dec     $C6
        jmp     (LC600,x)

        dec     $C6
        jmp     (L387C)

        bpl     L637F
L637F:  dec     $C6
        dec     $D6
        jmp     (L6C6C,x)

        brk
        dec     $EE
        jmp     (L7C38,x)

        inc     a:$C6
        cpy     $CCCC
        sei
        bmi     L63C5
        bmi     L6397
L6397:  inc     $1C0E,x
L639A:  sec
        .byte   $70
L639C:  cpx     #$FE
L639E:  brk
        tsb     $0808
        php
        php
        php
        tsb     LC600
        jmp     (L7C38)

        bpl     L6429
        bpl     L63AF
L63AF:  bmi     L63C1
        bpl     L63C3
        bpl     L63C5
        bmi     L63B7
L63B7:  brk
        brk
        brk
        bpl     L63E4
        bsr     L6340
        brk
        brk
        brk
L63C1:  brk
        brk
L63C3:  brk
        brk
L63C5:  inc     a:L0000,x
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        sec
        jmp     L4C3C

        jmp     L003A

        rts

        rts

        sei
        stz     $64
        stz     $78
        brk
        brk
        sec
        stz     $60
        rts

L63E4:  stz     L0038
        brk
        tsb     $3C0C
        jmp     L4C4C

        dec     a
        brk
        brk
        sec
        stz     $7C
        rts

        stz     L0038
        brk
        clc
        bmi     L642A
        .byte   $FC
        bmi     L642D
        bmi     L63FF
L63FF:  brk
        sec
        jmp     L4C4C

        bit     $380C,x
        rts

        rts

        sei
        stz     $64
        stz     $64
        brk
        brk
        bmi     L6412
L6412:  bmi     L6444
        bmi     L6446
        brk
        brk
        clc
        brk
        clc
        clc
        clc
        cli
        bmi     L6480
        rts

        ror     $6C
        sei
        jmp     (L0066)

        clc
        clc
L6429:  clc
L642A:  clc
        clc
        clc
L642D:  clc
        brk
        brk
        jmp     (L5656)

        lsr     $56,x
        lsr     L0000,x
        brk
        sei
        jmp     L4C4C

        jmp     L004C

        brk
        sei
        cpy     $CCCC
L6444:  .byte   $CC
        sei
L6446:  brk
        brk
        sei
        stz     $64
        stz     $78
        rts

        rts

        brk
        bit     L4C4C,x
        jmp     L0C3C

        tsb     L6000
        jmp     (L6072)

        rts

        rts

        brk
        brk
        bit     $3C62,x
        asl     L0066
        bit     $3000,x
        bmi     L64E6
        bmi     L649C
        bmi     L6486
        brk
        brk
        jmp     L4C4C

        jmp     L3A4C

        brk
        brk
        bsr     L64BE
        jmp     (L3828)

        bpl     L647F
L647F:  brk
L6480:  dec     $C6
        csl
        jmp     (L2838,x)

L6486:  brk
        brk
        bsr     L64F6
        sec
        sec
        jmp     (L0044)

        brk
        say
        ror     $34
        clc
        clc
        bmi     L64F7
        brk
        jmp     (L180C,x)

        .byte   $30
L649C:  rts

        jmp     (LFF00,x)

        .byte   $FF
        .byte   $FF
L64A2:  bbs7    $FF,$64A4
L64A5:  bbs7    $FF,$64A7
L64A8:  bbs7    $FF,$64AA
L64AB:  bbs7    $FF,$64AD
L64AE:  bbs7    $FF,$64B0
L64B1:  bbs7    $FF,$64B3
L64B4:  bbs7    $FF,$64B6
L64B7:  bbs7    $FF,$64B9
L64BA:  bbs7    $FF,$64BC
L64BD:  .byte   $FF
L64BE:  .byte   $FF
        .byte   $FF
L64C0:  bbs7    $FF,$64C2
L64C3:  bbs7    $FF,$64C5
L64C6:  bbs7    $FF,$64C8
L64C9:  bbs7    $FF,$64CB
L64CC:  bbs7    $FF,$64CE
L64CF:  bbs7    $FF,$64D1
L64D2:  bbs7    $FF,$64D4
L64D5:  bbs7    $FF,$64D7
L64D8:  bbs7    $FF,$64DA
L64DB:  bbs7    $FF,$64DD
L64DE:  bbs7    $FF,$64E0
L64E1:  bbs7    $FF,$64E3
L64E4:  .byte   $FF
        .byte   $FF
L64E6:  .byte   $FF
        .byte   $FF
L64E8:  bbs7    $FF,$64EA
L64EB:  bbs7    $FF,$64ED
L64EE:  bbs7    $FF,$64F0
L64F1:  bbs7    $FF,$64F3
L64F4:  .byte   $FF
        .byte   $FF
L64F6:  .byte   $FF
L64F7:  .byte   $FF
        .byte   $FF
L64F9:  bbs7    $FF,$64FB
L64FC:  bbs7    $FF,$64FE
L64FF:  bbs7    $FF,$6501
L6502:  bbs7    $FF,$6504
L6505:  bbs7    $FF,$6507
L6508:  bbs7    $FF,$650A
L650B:  bbs7    $FF,$650D
L650E:  bbs7    $FF,$6510
L6511:  bbs7    $FF,$6513
L6514:  bbs7    $FF,$6516
L6517:  bbs7    $FF,$6519
L651A:  bbs7    $FF,$651C
L651D:  bbs7    $FF,$651F
L6520:  bbs7    $FF,$6522
L6523:  bbs7    $FF,$6525
L6526:  bbs7    $FF,$6528
L6529:  bbs7    $FF,$652B
L652C:  bbs7    $FF,$652E
L652F:  bbs7    $FF,$6531
L6532:  bbs7    $FF,$6534
L6535:  bbs7    $FF,$6537
L6538:  bbs7    $FF,$653A
L653B:  bbs7    $FF,$653D
L653E:  bbs7    $FF,$6540
L6541:  bbs7    $FF,$6543
L6544:  bbs7    $FF,$6546
L6547:  bbs7    $FF,$6549
L654A:  bbs7    $FF,$654C
L654D:  bbs7    $FF,$654F
L6550:  bbs7    $FF,$6552
L6553:  bbs7    $FF,$6555
L6556:  bbs7    $FF,$6558
L6559:  bbs7    $FF,$655B
L655C:  bbs7    $FF,$655E
L655F:  bbs7    $FF,$6561
L6562:  bbs7    $FF,$6564
L6565:  bbs7    $FF,$6567
L6568:  bbs7    $FF,$656A
L656B:  bbs7    $FF,$656D
L656E:  bbs7    $FF,$6570
L6571:  bbs7    $FF,$6573
L6574:  bbs7    $FF,$6576
L6577:  bbs7    $FF,$6579
L657A:  bbs7    $FF,$657C
L657D:  bbs7    $FF,$657F
L6580:  bbs7    $FF,$6582
L6583:  bbs7    $FF,$6585
L6586:  bbs7    $FF,$6588
L6589:  bbs7    $FF,$658B
L658C:  bbs7    $FF,$658E
L658F:  bbs7    $FF,$6591
L6592:  bbs7    $FF,$6594
L6595:  bbs7    $FF,$6597
L6598:  bbs7    $FF,$659A
L659B:  bbs7    $FF,$659D
L659E:  bbs7    $FF,$65A0
L65A1:  bbs7    $FF,$65A3
L65A4:  bbs7    $FF,$65A6
L65A7:  bbs7    $FF,$65A9
L65AA:  bbs7    $FF,$65AC
L65AD:  bbs7    $FF,$65AF
L65B0:  bbs7    $FF,$65B2
L65B3:  bbs7    $FF,$65B5
L65B6:  bbs7    $FF,$65B8
L65B9:  bbs7    $FF,$65BB
L65BC:  bbs7    $FF,$65BE
L65BF:  bbs7    $FF,$65C1
L65C2:  bbs7    $FF,$65C4
L65C5:  bbs7    $FF,$65C7
L65C8:  bbs7    $FF,$65CA
L65CB:  bbs7    $FF,$65CD
L65CE:  bbs7    $FF,$65D0
L65D1:  bbs7    $FF,$65D3
L65D4:  bbs7    $FF,$65D6
L65D7:  bbs7    $FF,$65D9
L65DA:  bbs7    $FF,$65DC
L65DD:  bbs7    $FF,$65DF
L65E0:  bbs7    $FF,$65E2
L65E3:  bbs7    $FF,$65E5
L65E6:  bbs7    $FF,$65E8
L65E9:  bbs7    $FF,$65EB
L65EC:  bbs7    $FF,$65EE
L65EF:  bbs7    $FF,$65F1
L65F2:  bbs7    $FF,$65F4
L65F5:  bbs7    $FF,$65F7
L65F8:  bbs7    $FF,$65FA
L65FB:  bbs7    $FF,$65FD
L65FE:  bbs7    $FF,$6600
L6601:  bbs7    $FF,$6603
L6604:  bbs7    $FF,$6606
L6607:  bbs7    $FF,$6609
L660A:  bbs7    $FF,$660C
L660D:  bbs7    $FF,$660F
L6610:  bbs7    $FF,$6612
L6613:  bbs7    $FF,$6615
L6616:  bbs7    $FF,$6618
L6619:  bbs7    $FF,$661B
L661C:  bbs7    $FF,$661E
L661F:  bbs7    $FF,$6621
L6622:  bbs7    $FF,$6624
L6625:  bbs7    $FF,$6627
L6628:  bbs7    $FF,$662A
L662B:  bbs7    $FF,$662D
L662E:  bbs7    $FF,$6630
L6631:  bbs7    $FF,$6633
L6634:  bbs7    $FF,$6636
L6637:  bbs7    $FF,$6639
L663A:  bbs7    $FF,$663C
L663D:  bbs7    $FF,$663F
L6640:  bbs7    $FF,$6642
L6643:  bbs7    $FF,$6645
L6646:  bbs7    $FF,$6648
L6649:  bbs7    $FF,$664B
L664C:  bbs7    $FF,$664E
L664F:  bbs7    $FF,$6651
L6652:  bbs7    $FF,$6654
L6655:  bbs7    $FF,$6657
L6658:  bbs7    $FF,$665A
L665B:  bbs7    $FF,$665D
L665E:  bbs7    $FF,$6660
L6661:  bbs7    $FF,$6663
L6664:  .byte   $FF
        .byte   $FF
L6666:  .byte   $FF
        .byte   $FF
L6668:  bbs7    $FF,$666A
L666B:  bbs7    $FF,$666D
L666E:  bbs7    $FF,$6670
L6671:  bbs7    $FF,$6673
L6674:  bbs7    $FF,$6676
L6677:  bbs7    $FF,$6679
L667A:  bbs7    $FF,$667C
L667D:  bbs7    $FF,$667F
L6680:  bbs7    $FF,$6682
L6683:  bbs7    $FF,$6685
L6686:  bbs7    $FF,$6688
L6689:  bbs7    $FF,$668B
L668C:  bbs7    $FF,$668E
L668F:  bbs7    $FF,$6691
L6692:  bbs7    $FF,$6694
L6695:  bbs7    $FF,$6697
L6698:  bbs7    $FF,$669A
L669B:  bbs7    $FF,$669D
L669E:  bbs7    $FF,$66A0
L66A1:  bbs7    $FF,$66A3
L66A4:  bbs7    $FF,$66A6
L66A7:  bbs7    $FF,$66A9
L66AA:  bbs7    $FF,$66AC
L66AD:  bbs7    $FF,$66AF
L66B0:  bbs7    $FF,$66B2
L66B3:  bbs7    $FF,$66B5
L66B6:  bbs7    $FF,$66B8
L66B9:  bbs7    $FF,$66BB
L66BC:  bbs7    $FF,$66BE
L66BF:  bbs7    $FF,$66C1
L66C2:  bbs7    $FF,$66C4
L66C5:  .byte   $FF
L66C6:  .byte   $FF
        .byte   $FF
L66C8:  bbs7    $FF,$66CA
L66CB:  bbs7    $FF,$66CD
L66CE:  bbs7    $FF,$66D0
L66D1:  bbs7    $FF,$66D3
L66D4:  bbs7    $FF,$66D6
L66D7:  bbs7    $FF,$66D9
L66DA:  bbs7    $FF,$66DC
L66DD:  bbs7    $FF,$66DF
L66E0:  bbs7    $FF,$66E2
L66E3:  bbs7    $FF,$66E5
L66E6:  bbs7    $FF,$66E8
L66E9:  bbs7    $FF,$66EB
L66EC:  bbs7    $FF,$66EE
L66EF:  bbs7    $FF,$66F1
L66F2:  bbs7    $FF,$66F4
L66F5:  bbs7    $FF,$66F7
L66F8:  bbs7    $FF,$66FA
L66FB:  bbs7    $FF,$66FD
L66FE:  bbs7    $FF,$6700
L6701:  bbs7    $FF,$6703
L6704:  bbs7    $FF,$6706
L6707:  bbs7    $FF,$6709
L670A:  bbs7    $FF,$670C
L670D:  bbs7    $FF,$670F
L6710:  bbs7    $FF,$6712
L6713:  bbs7    $FF,$6715
L6716:  bbs7    $FF,$6718
L6719:  bbs7    $FF,$671B
L671C:  bbs7    $FF,$671E
L671F:  bbs7    $FF,$6721
L6722:  bbs7    $FF,$6724
L6725:  bbs7    $FF,$6727
L6728:  bbs7    $FF,$672A
L672B:  bbs7    $FF,$672D
L672E:  bbs7    $FF,$6730
L6731:  bbs7    $FF,$6733
L6734:  bbs7    $FF,$6736
L6737:  bbs7    $FF,$6739
L673A:  bbs7    $FF,$673C
L673D:  bbs7    $FF,$673F
L6740:  bbs7    $FF,$6742
L6743:  bbs7    $FF,$6745
L6746:  bbs7    $FF,$6748
L6749:  bbs7    $FF,$674B
L674C:  bbs7    $FF,$674E
L674F:  bbs7    $FF,$6751
L6752:  bbs7    $FF,$6754
L6755:  bbs7    $FF,$6757
L6758:  bbs7    $FF,$675A
L675B:  bbs7    $FF,$675D
L675E:  bbs7    $FF,$6760
L6761:  bbs7    $FF,$6763
L6764:  bbs7    $FF,$6766
L6767:  bbs7    $FF,$6769
L676A:  bbs7    $FF,$676C
L676D:  bbs7    $FF,$676F
L6770:  bbs7    $FF,$6772
L6773:  bbs7    $FF,$6775
L6776:  bbs7    $FF,$6778
L6779:  bbs7    $FF,$677B
L677C:  bbs7    $FF,$677E
L677F:  bbs7    $FF,$6781
L6782:  bbs7    $FF,$6784
L6785:  bbs7    $FF,$6787
L6788:  bbs7    $FF,$678A
L678B:  bbs7    $FF,$678D
L678E:  bbs7    $FF,$6790
L6791:  bbs7    $FF,$6793
L6794:  bbs7    $FF,$6796
L6797:  .byte   $FF
        .byte   $FF
L6799:  .byte   $FF
L679A:  bbs7    $FF,$679C
L679D:  bbs7    $FF,$679F
L67A0:  bbs7    $FF,$67A2
L67A3:  bbs7    $FF,$67A5
L67A6:  bbs7    $FF,$67A8
L67A9:  bbs7    $FF,$67AB
L67AC:  bbs7    $FF,$67AE
L67AF:  bbs7    $FF,$67B1
L67B2:  bbs7    $FF,$67B4
L67B5:  bbs7    $FF,$67B7
L67B8:  bbs7    $FF,$67BA
L67BB:  bbs7    $FF,$67BD
L67BE:  bbs7    $FF,$67C0
L67C1:  bbs7    $FF,$67C3
L67C4:  bbs7    $FF,$67C6
L67C7:  bbs7    $FF,$67C9
L67CA:  bbs7    $FF,$67CC
L67CD:  bbs7    $FF,$67CF
L67D0:  bbs7    $FF,$67D2
L67D3:  bbs7    $FF,$67D5
L67D6:  bbs7    $FF,$67D8
L67D9:  bbs7    $FF,$67DB
L67DC:  bbs7    $FF,$67DE
L67DF:  bbs7    $FF,$67E1
L67E2:  bbs7    $FF,$67E4
L67E5:  bbs7    $FF,$67E7
L67E8:  bbs7    $FF,$67EA
L67EB:  bbs7    $FF,$67ED
L67EE:  bbs7    $FF,$67F0
L67F1:  bbs7    $FF,$67F3
L67F4:  bbs7    $FF,$67F6
L67F7:  bbs7    $FF,$67F9
L67FA:  bbs7    $FF,$67FC
L67FD:  .byte   $FF
L67FE:  bbs7    $FF,L6799
        ror     L6C4E
        sta     ($6C)
        plx
        jmp     (L694D)

        lda     ($69,x)
        asl     $6A,x
        jmp     (LE26A,x)

        ror     a
        pha
        .byte   $6B
        lda     $136B,x
        jmp     (L681C)

        brk
        brk
L681C:  php
        brk
        and     $26
        asl     $21,x
        eor     $D3
        eor     $D4
        eor     $D5
        bvc     L67FE
        brk
        brk
        bmi     L687E
        tin     $00,$4F10,$27D5
        dec     L004C,x
        smb5    $3E
        brk
        brk
        brk
        .byte   $3B
        cmp     L000B,x
        brk
        smb5    $12
        nop
        sei
        eor     ($B2,x)
        tii     $4641,$0174,$0001
        adc     $0168
        ora     ($01,x)
        sta     $0168
        ora     ($02,x)
        lda     $0168
        ora     ($03,x)
        cmp     $0168
        ora     ($04,x)
        sbc     $0168
        ora     ($05,x)
        ora     $0169
        ora     ($06,x)
        and     $1A69
        brk
        st1     #$2D
        sxy
        jsr     L013E
        brk
L6876:  rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
L687E:  sxy
        brk
        inc     a
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        ora     #$1A
        ora     ($13,x)
        and     $2002
        rol     a:$01,x
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        ora     ($1A,x)
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        ora     #$1A
        sxy
        st1     #$2D
        sxy
        jsr     L013E
        brk
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        sxy
        inc     a
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        ora     #$1A
        st0     #$13
        and     $2002
        rol     a:$01,x
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        st0     #$1A
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        ora     #$1A
        tsb     $13
        and     $2002
        rol     a:$01,x
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        tsb     $1A
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        .byte   $09
L690D:  inc     a
        ora     $13
        and     $2002
        rol     a:$01,x
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        ora     $1A
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        ora     #$1A
        asl     $13
        and     $2002
        rol     a:$01,x
        rmb1    $2D
        sxy
        ora     ($03),y
        brk
        rts

        .byte   $2B
        sxy
        asl     $1A
        rmb0    $12
        eor     ($75)
        rmb1    $87
        and     $1203
        sbc     ($74)
        .byte   $09
L694D:  php
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $B3
        eor     $B4
        bvc     L690D
        brk
        brk
        bvc     L69AF
        .byte   $B3
        brk
L6961:  brk
        bpl     $69B3
L6964:  ldy     $27,x
        lda     L004C,x
        ldx     $3E,y
        brk
        brk
        brk
        .byte   $3B
        ldy     L000B,x
        brk
        ldx     $12,y
        nop
        sei
        eor     ($B2,x)
        tii     $7041,$1174,$0002
        rts

        st1     #$2D
        sxy
        jsr     L013E
        brk
        ora     ($35),y
        ora     ($2A,x)
        st0     #$14
        .byte   $01
L698D:  ora     ($15,x)
        ora     ($01,x)
        ora     ($52)
        adc     $17,x
        sta     ($2D,x)
        sxy
        .byte   $2B
        sxy
        brk
        bit     $0134,x
        asl     $21,x
        ora     #$08
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $BB
        eor     $BC
        bvc     L6961
L69AF:  brk
        brk
        bvc     L6A03
        .byte   $BB
        brk
        brk
        bpl     L6A07
        ldy     LBD27,x
        jmp     L3EBE

        brk
        brk
        brk
        .byte   $3B
        ldy     a:L000B,x
        ldx     $EA12,y
        sei
        rol     L0000,x
        sxy
        eor     ($64,x)
        stz     L0011,x
        sxy
        brk
        rts

        st1     #$2D
        sxy
L69D6:  jsr     L013E
        brk
        ora     ($35),y
        ora     ($2A,x)
        st0     #$2D
        bit     $011C,x
        rts

        brk
        st0     #$2D
        bit     $1C,x
        bcc     L69EC
L69EC:  st0     #$2D
        bvc     L6A02
        sta     $74,x
        .byte   $3B
        ldy     $201D,x
        rts

        jsr     L0020
        brk
        and     $1202
        smb2    $74
        inc     a
        brk
L6A02:  .byte   $19
L6A03:  brk
        ora     ($52)
        .byte   $75
L6A07:  rmb1    $81
        rmb1    $82
        and     $2B02
        sxy
        ora     ($3C,x)
        bit     $01,x
        asl     $21,x
        ora     #$08
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $B7
        eor     $B8
        bvc     L69D6
        brk
        brk
        bvc     L6A78
        smb3    L0000
        brk
        bpl     L6A7C
        clv
        rmb2    $B9
        jmp     L3EBA

        brk
        brk
        brk
        .byte   $3B
        clv
        .byte   $0B
        brk
        tsx
        .byte   $12
L6A3C:  nop
        sei
        rol     $01,x
        brk
        eor     ($70,x)
        stz     L0011,x
        sxy
        brk
        rts

        st1     #$2D
        sxy
        jsr     L032A
        rol     a:$01,x
        ora     ($35),y
        ora     ($1C,x)
        st0     #$00
        ora     ($03,x)
        and     $1246
        sta     $74,x
        .byte   $3B
        clv
        eor     ($B2,x)
        tii     $A712,$1474,$0101
        ora     $01,x
        ora     ($12,x)
        eor     ($75)
        rmb1    $81
        and     $2B02
        sxy
        sxy
        .byte   $3C
        .byte   $34
L6A78:  ora     ($16,x)
        and     ($09,x)
L6A7C:  php
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $BF
        eor     $C0
        bvc     L6A3C
        brk
        brk
        bvc     L6ADE
        bbs3    L0000,L6A91
L6A91:  bpl     L6AE2
        cpy     #$27
        cmp     (L004C,x)
        cly
        rol     a:L0000,x
        brk
        .byte   $3B
        cpy     #$0B
        brk
        cly
        .byte   $12
L6AA2:  nop
        sei
        rol     L0000,x
        brk
        eor     ($70,x)
        stz     L0011,x
        sxy
        brk
        rts

        st1     #$2D
        sxy
        jsr     L013E
        brk
        ora     ($35),y
        ora     ($2A,x)
        st0     #$1C
        ora     ($80,x)
        brk
        st0     #$2D
        lsr     $12
        sta     $74,x
        .byte   $3B
        cpy     #$41
        nop
        tii     $A712,$1474,$0101
        ora     $01,x
        ora     ($12,x)
        eor     ($75)
        rmb1    $81
        and     $2B02
        sxy
        st0     #$3C
        .byte   $34
L6ADE:  ora     ($16,x)
        and     ($09,x)
L6AE2:  php
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $C3
        eor     $C4
        bvc     L6AA2
        brk
        brk
        bvc     L6B44
        tdd     $00,$4F10,$27C4
        cmp     L004C
        dec     $3E
        brk
        brk
        brk
        .byte   $3B
        cpy     L000B
        brk
        dec     $12
L6B08:  nop
        sei
        rol     L0000,x
        brk
        eor     ($64,x)
        stz     L0011,x
        sxy
        brk
        rts

        st1     #$2D
        sxy
        jsr     L013E
        brk
        ora     ($35),y
        ora     ($2A,x)
        st0     #$1C
        sxy
        rts

        brk
        st0     #$2D
        bvc     L6B3A
        sta     $74,x
        .byte   $3B
        cpy     $41
        nop
        tii     $A712,$1A74,$1900
        brk
        ora     ($52)
        adc     $17,x
L6B3A:  sta     ($17,x)
        clx
        and     $2B02
        sxy
        tsb     $3C
        .byte   $34
L6B44:  ora     ($16,x)
        and     ($09,x)
        php
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $C7
        eor     $C8
        bvc     L6B08
        brk
        brk
        bvc     L6BAA
        smb4    L0000
        brk
        bpl     L6BAE
        iny
        rmb2    $C9
        jmp     L3ECA

        brk
        brk
        brk
        .byte   $3B
        iny
        .byte   $0B
        brk
        dex
        ora     ($EA)
        sei
        rol     L0000,x
        brk
        trb     L0000
        brk
        ora     L0000,x
        brk
        trb     $02
        sxy
        .byte   $15
L6B7D:  sxy
        sxy
        ora     ($02),y
        brk
        rts

        st1     #$2D
        sxy
        jsr     L013E
        brk
        ora     ($35)
        ora     ($2A,x)
        st0     #$2D
        sxy
        inc     a
        brk
        and     $1C01
        ora     ($60,x)
        brk
        st0     #$2D
        lsr     $12
        sta     $74,x
        .byte   $3B
        iny
        eor     ($EA,x)
        tii     $A712,$1474,$0101
L6BAA:  ora     $01,x
        ora     ($12,x)
L6BAE:  eor     ($75)
        rmb1    $81
        and     $2B02
        sxy
        ora     $3C
        bit     $01,x
        asl     $21,x
        ora     #$08
        brk
        and     $26
        asl     $21,x
        eor     $B2
        eor     $CB
        eor     $CC
        bvc     L6B7D
        brk
        brk
        bvc     L6C1F
        .byte   $CB
        brk
        brk
        bpl     L6C23
        cpy     $CD27
        jmp     L3ECE

        brk
        brk
        brk
        .byte   $3B
        cpy     a:L000B
        dec     $EA12
        sei
        eor     ($B2,x)
        tii     $7041,$1174,$0002
L6BEE:  rts

        st1     #$2D
        sxy
        jsr     L013E
        brk
        ora     ($2D),y
        sxy
        and     $01,x
        rol     a
        st0     #$14
        ora     ($01,x)
        ora     $01,x
        ora     ($12,x)
        eor     ($75)
        rmb1    $81
        and     $2B02
        sxy
        asl     $3C
        bit     $01,x
        asl     $21,x
        ora     #$08
        brk
        and     $26
        asl     $21,x
        eor     $CF
        eor     $D0
        bvc     L6BEE
L6C1F:  brk
        brk
        bpl     L6C72
L6C23:  bne     L6C4C
        cmp     (L004C),y
        cmp     ($3E)
        brk
        brk
        brk
        .byte   $3B
        bne     L6C3A
        brk
        cmp     ($12)
        nop
        sei
        eor     ($B2,x)
        .byte   $73
        eor     ($70,x)
        .byte   $74
L6C3A:  st1     #$2D
L6C3C:  sxy
        jsr     L0B2A
        rol     a:$01,x
        ora     ($1A),y
        brk
        ora     ($52)
        adc     $12,x
        bit     #$6D
L6C4C:  asl     $09,x
L6C4E:  and     ($16,x)
        php
        brk
        and     $26
        jmp     L1F00

        tax
        brk
        bpl     L6C99
        brk
        brk
        brk
        jmp     L1EAD

        .byte   $AB
        rmb2    $AC
        rol     a:L0000,x
        brk
        .byte   $0B
        brk
        .byte   $AD
        .byte   $12
L6C6C:  nop
        sei
        eor     ($B2,x)
        .byte   $73
        .byte   $41
L6C72:  ror     a
        stz     $13,x
        jsr     L013E
        brk
        ora     (L000B)
        brk
        brk
        inc     a
        brk
        ora     ($3E)
        ror     L142D
        ora     (L0000,x)
        brk
        bbs0    $6C,L6C95
        ora     #$FF
        brk
        sta     ($6C)
        .byte   $0B
        ora     #$00
        ora     ($F2)
        .byte   $74
L6C95:  say
        bbs7    L0000,L6C99
L6C99:  asl     $08,x
        brk
        and     L0011
        ora     (L0000,x)
        rts

        .byte   $0B
        tsb     $0200
        ora     #$00
        beq     L6D15
        st2     #$00
        ora     ($0C,x)
        brk
        tsx
        jmp     (L0C01)

        ora     ($CC,x)
        jmp     (L0C01)

        sxy
        dec     $126C,x
        rmb3    $7E
        ora     (L0000,x)
        brk
        tsb     $236D
        st0     #$02
        brk
        brk
        lsr     a:$6C
        tsb     $126D
        rmb3    $7E
        ora     (L0000,x)
        brk
        tsb     $236D
        tsb     $02
        brk
        brk
        lsr     a:$6C
        tsb     $126D
        rmb3    $7E
        .byte   $01
L6CE2:  brk
        brk
        tsb     $236D
        ora     $02
        brk
        brk
        .byte   $4E
L6CEC:  jmp     ($0C00)

        adc     $0123
        sxy
        brk
        brk
        lsr     a:$6C
        tsb     $126D
        sbc     ($74)
        ora     ($0D)
        ror     $0D0B,x
        brk
        .byte   $0B
        ora     #$FF
        ora     ($7E)
        jmp     (L1700,x)

        adc     L5D12
        adc     $01,x
        ora     $17FF
        .byte   $6D
        .byte   $12
L6D15:  ror     $127C,x
        sbc     ($74)
L6D1A:  php
        brk
        and     $26
        eor     $AE
        eor     $AF
        eor     $B0
        eor     $B1
        bit     $01,x
        and     ($16,x)
        rmb4    $AE
        brk
        bpl     L6D7E
        bbs2    $46,L6CE2
        lsr     $B1
        rol     a:L0000,x
        brk
        .byte   $3B
        bbs2    L0044,L6CEC
        rol     a:L0000,x
        brk
        .byte   $0B
        brk
        lda     ($12),y
        nop
        sei
        eor     ($B2,x)
        tii     $4C41,$1A74,$1A01
        sxy
        inc     a
        st0     #$1A
        tsb     $1A
        ora     $1A
        asl     $1A
        rmb0    $12
        dec     $75
        st1     #$20
        ora     ($01),y
        brk
        rts

        ora     $2A07,y
        sxy
        rol     a:$01,x
        st1     #$0B
        brk
        brk
        .byte   $0B
        asl     a
        brk
        .byte   $0B
        .byte   $0B
        bbr0    $12,L6D1A
        adc     L8912
        adc     $0134
        and     ($16,x)
L6D7E:  say
        bbs7    L0000,L6D82
L6D82:  ora     ($18)
        cpx     #$12
        tia     $097D,$0CA9,$2D20
        .byte   $E0
L6D8E:  rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
L6D96:  brk
        sxy
        ora     $01
        tsb     $03
        .byte   $06
L6D9D:  ora     ($04,x)
        sxy
        ora     $06
        st0     #$07
        tii     $2781,$6D8F,$0007
        lda     #$03
        sta     $2781
        lda     #$01
        jsr     L6E80
        lda     #$01
        jsr     L6E88
L6DBA:  jsr     LE063
        lda     $2228
        cmp     #$40
        beq     L6DD1
        cmp     #$10
        beq     L6DE4
        cmp     #$01
        beq     L6DF5
        jsr     L7549
        bra     L6DBA
L6DD1:  lda     $2780
        cmp     #$06
        beq     L6DBA
        bsr     L6E0A
        inc     $2780
        lda     $2780
        bsr     L6E23
        bra     L6DBA
L6DE4:  lda     $2780
        beq     L6DBA
        bsr     L6E0A
        dec     $2780
        lda     $2780
        bsr     L6E23
        bra     L6DBA
L6DF5:  lda     $278A
        tax
        lda     L6D9D,x
        tax
        lda     L6D8E,x
        cmp     #$01
        beq     L6E09
        jsr     L7549
        bra     L6DBA
L6E09:  rts

L6E0A:  tax
        lda     L6D96,x
        tax
        lda     L6D9D,x
        tax
        lda     L6D8E,x
        sta     $2780,x
        txa
        pha
        jsr     L6E80
        pla
        jsr     L6E88
        rts

L6E23:  tax
        lda     L6D96,x
        sta     $278A
        tax
        lda     L6D9D,x
        tax
        lda     #$03
        sta     $2780,x
        txa
        pha
        jsr     L6E80
        pla
        jsr     L6E88
        rts

L6E3E:  jsr     L7549
        jsr     LE063
L6E44:  lda     $2228
        cmp     #$40
        beq     L6E55
        cmp     #$10
        beq     L6E64
        cmp     #$01
        beq     L6E73
        bra     L6E3E
L6E55:  lda     #$01
        sta     $2780
        lda     #$02
        bsr     L6E80
        lda     #$02
        bsr     L6E88
        bra     L6E3E
L6E64:  lda     #$00
        sta     $2780
        lda     #$02
        bsr     L6E80
        lda     #$02
        bsr     L6E88
        bra     L6E3E
L6E73:  lda     #$01
        bsr     L6E80
        lda     #$00
        bsr     L6E90
        lda     #$01
        bsr     L6E88
        rts

L6E80:  sta     $0E
        lda     #$0C
        jsr     L3AB7
        rts

L6E88:  sta     $0E
        lda     #$0E
        jsr     L3AB7
        rts

L6E90:  sta     $0E
        lda     #$0D
        jsr     L3AB7
        rts

        php
        brk
        and     $26
        and     ($1F,x)
        cld
        brk
        bpl     L6EE0
        brk
        brk
        brk
        asl     L41D9,x
        lda     ($73)
        jsr     L013E
        bbs7    L0000,L6EB8
        brk
        and     $26
        php
        ora     ($25,x)
        .byte   $4C
        .byte   $6C
L6EB8:  jmp     L4C6D

        ror     L704C
        jmp     L4C71

        tii     $744C,$754C,$0208
        and     L004C
        bbr6    L004C,L6F44
        jmp     L4C78

        ply
        jmp     L4C7B

        adc     L7E4C,x
        jmp     L4C80

        sta     ($08,x)
        st0     #$25
        .byte   $4C
        .byte   $72
L6EE0:  jmp     L0876

        tsb     $25
        jmp     L4C79

        jmp     (L834C,x)

        jmp     L4C84

        sta     L004C
        smb0    L004C
        dey
        jmp     L4C89

        sty     L8D4C
        rol     $45
        bbr7    $45,L6E80
        eor     $86
        eor     $8A
        eor     $8B
        eor     $8E
        eor     $8F
        eor     $90
        eor     $91
        eor     $92
        eor     $93
        php
        brk
        and     L004C
        brk
        bit     $01,x
        bit     L0000,x
        asl     $21,x
        bbr1    $69,L6F1E
L6F1E:  bpl     L6F3E
        ror     a
        rmb2    $6B
        eor     ($B2,x)
        tii     $7041,$2A74,$2000
        asl     $2D,x
        bit     L3C2D,x
        and     $2D3C
        bit     L3C2D,x
        and     $2D3C
        bit     L3C2D,x
        .byte   $2D
L6F3E:  bit     L3C2D,x
        and     $2D3C
L6F44:  bit     L3C2D,x
        and     $2D3C
        bit     L3C2D,x
        and     $2D3C
        bit     L3C2D,x
        and     $2D3C
        bit     L3C2D,x
        ora     ($F2)
        stz     $39,x
        brk
        php
        ora     ($53,x)
        jmp     (L0000)

        rti

        eor     ($6D)
        brk
        eor     ($6E),y
        brk
        php
        brk
        bit     L0000,x
        asl     $21,x
        bit     L0000,x
        .byte   $3B
        adc     L6E44
L6F77:  .byte   $0B
        brk
        ora     ($36),y
        brk
        brk
        eor     ($70,x)
        stz     $2D,x
        bit     $2D13,x
        sxy
        jsr     LB40A
        brk
        trb     LB001
        brk
        asl     $2D
        sei
        ora     ($F2)
        stz     $39,x
        brk
        asl     $21,x
        bit     L0000,x
        php
        sxy
        tam     #$6F
        brk
        brk
        bpl     L6FA9
        ora     ($52,x)
        bvs     L6FA5
L6FA5:  eor     ($71),y
        brk
        php
L6FA9:  brk
        .byte   $3B
        bvs     L6FF1
        adc     ($36),y
        brk
        ora     ($41,x)
        bvs     L7028
        st1     #$2D
        st0     #$20
        and     $1C3C
        brk
        ldy     #$00
        tsb     $12
        sbc     ($74)
        and     L0800,y
        st0     #$53
        adc     (L0000)
        brk
        bvc     L6FD4
        ora     ($52,x)
        .byte   $73
        brk
        eor     ($74),y
        brk
        .byte   $51
L6FD4:  adc     L0000,x
        php
        brk
        and     $340A
        sxy
        asl     $21,x
        php
        st0     #$53
        ror     $01,x
        brk
        bpl     L6FEE
        brk
        bit     L0000,x
        rol     $0100,x
        brk
        .byte   $3B
L6FEE:  .byte   $73
        bsr     L7065
L6FF1:  rol     $01,x
        brk
        eor     ($6A,x)
        stz     $13,x
        jsr     L000B
        adc     $12,x
        ply
        rmb7    $2D
        sei
        inc     a
        ora     ($2D,x)
        ora     ($1C,x)
        st0     #$40
L7008:  brk
        ora     $19
        ora     ($2D,x)
        sei
        ora     ($F2)
        stz     $39,x
        brk
        php
        sxy
        eor     ($77)
        brk
        eor     ($78),y
        brk
        php
        brk
        and     ($16,x)
        and     $3414
        brk
        .byte   $3B
        rmb7    L0044
        sei
        .byte   $36
L7028:  brk
        brk
        and     L4102
        ror     a
        stz     $13,x
        and     $2002
        and     $1A02
        ora     ($2D,x)
        ora     ($1C,x)
        ora     (L0020,x)
        brk
        php
        ora     $2D01,y
        bit     $F212,x
        stz     $39,x
        brk
        and     ($16,x)
        php
        tsb     $53
L704C:  adc     L0000,y
        bvc     L7059
        sxy
        eor     ($7A)
        brk
        eor     ($7B),y
        brk
        php
L7059:  brk
        .byte   $3B
        ply
        bsr     L70D9
        eor     ($B2,x)
        .byte   $73
        eor     ($64,x)
        stz     $13,x
L7065:  and     $2002
        inc     a
        brk
        and     $1A78
        ora     ($19,x)
        ora     ($2D,x)
        bit     $F212,x
        stz     $39,x
        brk
        php
        tsb     $53
        jmp     (L0000,x)

        bpl     L7087
        sxy
        eor     ($7D)
        brk
        eor     ($7E)
        brk
        .byte   $16
L7087:  and     ($2D,x)
        trb     $34
        brk
        php
        brk
        .byte   $3B
        adc     L7E44,x
        eor     ($B2,x)
        tii     $7041,$1374,$2D20
        ldy     $1B,x
        brk
        asl     $21,x
        ora     ($F2)
        stz     $39,x
        brk
        bvc     L7126
        brk
        brk
        rti

        php
        sxy
        eor     ($80)
        brk
        eor     ($81),y
        brk
        php
        brk
        and     ($16,x)
        and     $3414
        brk
        .byte   $3B
        bra     L7101
        sta     ($41,x)
        lda     ($73)
        eor     ($6A,x)
        stz     $13,x
        jsr     L782D
        and     $2DB4
        ldy     $12,x
        sbc     ($74)
        and     L5000,y
        clx
        brk
        brk
        rts

        php
        tsb     $52
L70D9:  tst     #$00,$51
        sty     L0000
        eor     ($85),y
        brk
        php
        brk
        bit     $01,x
        asl     $21,x
        bit     L0000,x
        rol     a:L0000,x
        brk
        .byte   $3B
        tst     #$44,$84
        rol     a:L0000,x
        brk
        .byte   $0B
        brk
        sta     $12
        nop
        sei
        rol     $01,x
        brk
        eor     ($70,x)
        .byte   $74
L7101:  st1     #$2D
        sxy
        jsr     L3C2D
        rol     a:$01,x
        ora     ($2D),y
        bit     $031C,x
        jsr     L0800
        and     $1278
        sbc     ($74)
        and     $2100,y
        asl     $50,x
        stx     L0000
        brk
        bpl     L7129
        tsb     $52
        smb0    L0000
        .byte   $51
L7126:  dey
        brk
        .byte   $51
L7129:  bit     #$00
        php
        brk
        .byte   $3B
        smb0    L0044
        dey
        .byte   $41
L7132:  lda     ($73)
        eor     ($70,x)
        stz     $13,x
        and     $2002
        and     $0B14
        brk
        bit     #$12
        rts

        rmb7    $2D
        trb     $1A
        brk
        ora     $2D00,y
        trb     $16
        say
        brk
        brk
        brk
L7150:  and     $2D78
        bit     $FF42,x
        brk
        brk
        and     $3400,y
        ora     ($21,x)
        asl     $50,x
        txa
        brk
        brk
        bpl     L71B4
        .byte   $8B
        brk
        brk
        bmi     L7171
        tsb     $52
        sty     L5100
        sta     L0800
L7171:  brk
        asl     $21,x
        .byte   $3B
        sty     L8D44
        eor     ($B2,x)
        tii     $7041,$1374,$022D
        jsr     L022D
        inc     a
        brk
        ora     $1200,y
        sbc     ($74)
        and     $2100,y
        asl     $50,x
        stx     a:L0000
        rts

        bbr4    $8F,L71D5
        brk
        brk
        brk
        .byte   $3B
        bbs0    $41,L7150
        tii     $2D20,$3E3C,$FF01
        brk
        and     $3978
        brk
        bvc     L7132
        brk
        brk
        bpl     L71B8
        tsb     $52
        smb0    L0000
L71B4:  php
        brk
        lsr     $90
L71B8:  lsr     $91
        lsr     $92
        bit     $01,x
        and     ($16,x)
        and     $3E14
        brk
        brk
        brk
        .byte   $3B
        smb0    L0044
        bcc     L720C
        lda     ($73)
        eor     ($70,x)
        stz     $13,x
        jsr     L142D
        .byte   $0B
L71D5:  brk
        sta     ($12),y
        nop
        sei
        rol     a:$01,x
        bpl     L720C
        bit     L000B,x
        sta     ($12)
        rts

        rmb7    $42
        brk
        brk
        brk
        and     $2D78
        bit     $FF42,x
        brk
        brk
        inc     a
        brk
        ora     $2D00,y
        ldy     $12,x
        sbc     ($74)
        and     L5000,y
        clx
        brk
        brk
        rts

        php
        brk
        bbr4    $93,L7228
        asl     $34,x
        brk
        .byte   $3B
        .byte   $93
L720C:  eor     ($B2,x)
        tii     $782D,$2D20,$3CB4
        php
        brk
        and     $26
        php
        ora     ($25,x)
        jmp     L4C98

        sta     L9A4C,y
        jmp     L4C9B

        stz     L9D4C
L7228:  jmp     L4C9E

        bbs1    $08,L7230
        and     L004C
L7230:  ldy     #$4C
        lda     (L004C,x)
        ldx     #$4C
        tst     #$4C,$A4,x
        php
        st0     #$25
        jmp     L4CA5

        ldx     L004C
        smb2    L004C
        tay
        jmp     L08A9

        tsb     $25
        php
        brk
        and     L004C
        brk
        php
        brk
        and     L004C
        brk
        bit     $01,x
        asl     $21,x
        bbr1    $94,L725A
L725A:  bpl     L727A
        sta     $27,x
        stx     L004C,y
        smb1    L000B
        brk
        smb1    $12
        rts

        rmb7    $41
        lda     ($73)
        eor     ($58,x)
        stz     $2A,x
        ora     ($2D,x)
        asl     $1320,x
        say
        brk
        brk
        brk
        and     $1A3C
L727A:  brk
        and     $1AB4
        tsb     $19
        tsb     $19
        brk
        and     $193C
        ora     ($2D,x)
        ldy     $2D,x
        bit     $39,x
        say
        bbs7    L0000,L7291
L7291:  and     ($16,x)
        and     $0802
        ora     ($53,x)
        tya
        brk
        brk
        bvc     L72EF
        sta     L5100,y
        txs
        brk
        php
        brk
        .byte   $3B
        sta     L9A44,y
        eor     ($B2,x)
        tii     $7041,$1374,$022D
        jsr     LB42D
        and     $393C
        brk
        bit     $02,x
        and     ($16,x)
        php
        ora     ($53,x)
        .byte   $9B
        brk
        brk
        bpl     L7316
        stz     L5100
        sta     L5100,x
        bbs1    L0000,L731E
        stz     L0800,x
        brk
        rol     $0100,x
        brk
        .byte   $3B
        stz     L9D44
        .byte   $0B
        brk
        stz     LB241,x
        tii     $6A41,$2074,$7A12
        rmb7    $13
        and     $0B02
        brk
        bbs1    $12,L734E
        .byte   $77
L72EF:  say
        brk
        brk
        brk
        and     $1A28
        ora     ($19,x)
        ora     ($39,x)
        brk
        say
        bbs7    L0000,L72FF
L72FF:  and     ($16,x)
        php
        sxy
        tam     #$A0
        brk
        brk
        rti

        eor     ($A1)
        brk
        eor     ($A2),y
        brk
        php
        brk
        .byte   $3B
        lda     (L0044,x)
        ldx     #$36
        brk
L7316:  ora     ($41,x)
        lsr     $1374,x
        and     $2002
L731E:  and     $1C02
        brk
        ldy     #$00
        ora     ($1A,x)
        st0     #$19
        st0     #$2D
        bit     $39,x
        and     ($16,x)
        php
        sxy
        tam     #$A3
        ora     (L0000,x)
        bpl     L7389
        ldy     L0000
        php
        st0     #$51
        lda     L0000
        php
        brk
        .byte   $3B
        ldy     L0044
        lda     $41
        lda     ($73)
        eor     ($6A,x)
        stz     $13,x
        and     $2002
L734E:  and     $1A3C
        brk
        ora     $2D00,y
        phy
        and     L0800,y
        st0     #$53
        ldx     $01
        brk
        rts

        asl     $21,x
        eor     ($A7)
        brk
        eor     ($A8),y
        brk
        php
        brk
        .byte   $3B
        smb2    L0044
        tay
        eor     ($B2,x)
        tii     $6441,$1374,$022D
        jsr     L1E2D
        inc     a
        ora     ($19,x)
        ora     ($2D,x)
        bit     $39,x
        php
        sxy
        tam     #$A3
        ora     (L0000,x)
        bpl     L73AA
L7389:  asl     $52,x
        ldy     L0000
        php
        st0     #$51
        lda     #$00
        php
        brk
        .byte   $3B
        ldy     L0044
        lda     #$41
        lda     ($73)
        eor     ($6A,x)
        stz     $13,x
        and     $2002
        and     $1A78
        ora     ($19,x)
        ora     ($2D,x)
        .byte   $B4
L73AA:  bit     $0234,x
        asl     $21,x
        bit     $01,x
        ora     #$1D
        brk
        brk
        jsr     L0020
        brk
        ora     #$1D
        jsr     L2000
        jsr     L0000
        ora     #$1D
        rti

        brk
        jsr     L0020
        brk
        ora     #$1D
        rts

        brk
        jsr     L0020
        brk
        ora     #$1D
        bra     L73D5
L73D5:  jsr     L0020
        brk
        ora     #$1D
        ldy     #$00
        jsr     L0020
        brk
        ora     #$1D
        cpy     #$00
        jsr     L0020
        brk
        ora     #$1D
        brk
        jsr     L2020
        brk
        brk
        ora     #$1D
        jsr     L2020
        jsr     L0000
        ora     #$1D
        rti

        jsr     L2020
        brk
        brk
        ora     #$1D
        rts

        jsr     L2020
        brk
        brk
        ora     #$1D
        brk
        rti

        jsr     L0020
        brk
        ora     #$1D
        jsr     L2040
        jsr     L0000
        ora     #$1D
        rti

        rti

        jsr     L0020
        brk
        ora     #$1D
        rts

        rti

        jsr     L0020
        brk
        ora     #$0B
        brk
        brk
        .byte   $0B
        ora     (L0000,x)
        .byte   $0B
        sxy
        brk
        .byte   $0B
        st0     #$00
        .byte   $0B
        tsb     L0000
        .byte   $0B
        ora     L0000
        .byte   $0B
        asl     L0000
        .byte   $0B
        rmb0    L0000
        .byte   $0B
        php
        brk
        ora     #$14
        rmb0    $07
        ora     $07,x
        rmb0    $14
        asl     $06
        ora     $06,x
        asl     $14
        ora     $05
        ora     $05,x
        ora     $14
        tsb     $04
        ora     $04,x
        tsb     $14
        st0     #$03
        ora     $03,x
        st0     #$14
        sxy
        sxy
        ora     $02,x
        sxy
        trb     $01
        ora     ($15,x)
        ora     ($01,x)
        trb     L0000
        brk
        ora     L0000,x
        brk
        ora     #$08
        brk
        and     $08
        ora     ($25,x)
        php
        sxy
        and     $08
        st0     #$25
        php
        tsb     $25
        php
        ora     $25
        php
        asl     $25
        ora     #$A5
        tai     $3F29,$F385,$1880
        lda     $F3
        and     #$7F
        sta     $F3
        bra     L74AD
        lda     $F3
        ora     #$80
        and     #$BF
        sta     $F3
        bra     L74AD
        lda     $F3
        ora     #$C0
        sta     $F3
L74AD:  dec     $5A
        st0     #$05
        lda     $F3
        sta     a:$02
        stz     $5A
        rts

        tma     #$04
        pha
        clc
        lda     $FFF5
        adc     #$02
        tam     #$04
        lda     #$08
        sta     L58DD
        lda     #$09
        sta     L5798
        sta     L571E
        stz     $27C7
        lda     #$82
        sta     L58DB
        pla
        tam     #$04
        rts

        tma     #$04
        pha
        clc
        lda     $FFF5
        adc     #$02
        tam     #$04
        stz     L58DB
        stz     L58DD
        pla
        tam     #$04
        rts

        stz     $27D9
        tma     #$04
        pha
        clc
        lda     $FFF5
        adc     #$02
        tam     #$04
        stz     L4DDA
        pla
        tam     #$04
        rts

        lda     $3009
        sta     $300A
        rts

        dec     $5A
        st0     #$00
        st1     #$00
        st2     #$00
        st0     #$01
        st1     #$00
        st2     #$00
        ldx     #$20
L751E:  phx
        ldx     #$40
L7521:  st0     #$02
        ldy     a:$02
        lda     a:$03
        st0     #$02
        sty     a:$02
        clc
        adc     #$02
        ora     #$10
        sta     a:$03
        dex
        bne     L7521
        stz     $3B33
L753C:  lda     $3B33
        cmp     #$03
        bcc     L753C
        plx
        dex
        bne     L751E
        stz     $5A
L7549:  stz     $3B33
L754C:  lda     $3B33
        beq     L754C
        rts

L7552:  bsr     L7549
        jsr     LE063
        lda     $222D
        beq     L7552
        rts

        lda     #$3C
        sta     $278D
L7562:  jsr     L7549
        jsr     LE063
        lda     $222D
        beq     L759A
        cmp     #$01
        bne     L75AC
L7571:  jsr     L7549
        jsr     LE063
        lda     $222D
        beq     L75A0
        cmp     #$02
        bne     L75AC
L7580:  jsr     L7549
        jsr     LE063
        lda     $222D
        beq     L75A6
        cmp     #$04
        bne     L75AC
        lda     #$FF
        sta     $278D
        jsr     L7C7E
        bsr     L75B7
        rts

L759A:  dec     $278D
        bne     L7562
        rts

L75A0:  dec     $278D
        bne     L7571
        rts

L75A6:  dec     $278D
        bne     L7580
        rts

L75AC:  stz     $278D
        rts

L75B0:  pha
        ldx     #$01
        lda     #$00
        bra     L75BC
L75B7:  pha
        ldx     #$01
        lda     #$01
L75BC:  sta     $2780,x
        inx
        cpx     #$08
        bne     L75BC
        pla
        rts

        ldx     #$01
L75C8:  lda     $2780,x
        cmp     #$02
        bne     L75D3
        phx
        bsr     L75D9
        plx
L75D3:  inx
        cpx     #$08
        bne     L75C8
        rts

L75D9:  dex
        txa
        asl     a
        tay
        lda     #$49
        sta     L0000
        lda     #$76
        sta     $01
        lda     (L0000),y
        sta     $02
        iny
        lda     (L0000),y
        sta     $03
        cly
L75EF:  lda     ($02),y
        cmp     #$FF
        beq     L7646
        sta     L4F8B
        iny
        lda     ($02),y
        sta     L4F8C
        iny
        lda     ($02),y
        sta     L7647
        iny
        lda     ($02),y
        sta     L7648
        iny
        jsr     L51F9
        dec     $5A
        st0     #$01
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
L7628:  st0     #$02
        ldx     a:$02
        lda     a:$03
        and     #$0F
        ora     L7648
        st0     #$02
        .byte   $8E
L7638:  sxy
        brk
        .byte   $8D
L763B:  st0     #$00
        dec     L7647
        bne     L7628
        stz     $5A
        bra     L75EF
L7646:  rts

L7647:  brk
L7648:  brk
        rmb5    $76
        sta     ($76),y
        cpy     L6876
        ror     $A2,x
        ror     $B3,x
        ror     $DD,x
        ror     $0E,x
        tsb     $04
        rts

        asl     $0405
        rts

        php
        asl     $10
        rts

        php
        rmb0    $10
        rts

        bbs7    $0E,L7672
        asl     $60
        asl     $0609
        rts

        .byte   $0E
        asl     a
L7672:  asl     $60
        asl     $060B
        rts

        tsb     $080C
        rts

        tsb     $080D
        rts

        tsb     $080E
        rts

        tsb     $080F
        rts

        asl     $0410
        rts

        asl     $0411
        rts

        bbs7    L0000,L769F
        tsb     a:$80
        ora     L800C
        brk
        asl     L800C
        brk
        .byte   $0F
L769F:  tsb     $FF80
        brk
        ora     ($0A)
        bvs     L76A7
L76A7:  st1     #$0A
        bvs     L76AB
L76AB:  trb     $0A
        bvs     L76AF
L76AF:  ora     $0A,x
        .byte   $70
L76B2:  bbs7    $14,$76C3
        tsb     $1480
        bbr0    $0C,L763B
        trb     $10
        tsb     $1480
        ora     ($0C),y
        bra     L76D8
        ora     ($0C)
        bra     L76DC
        st1     #$0C
        .byte   $80
L76CB:  bbs7    $18,$76D4
        php
        bvs     L76E9
        rmb0    $08
        bvs     L76E9
        php
        .byte   $0C
        .byte   $70
L76D8:  trb     $09
        .byte   $0C
        .byte   $70
L76DC:  bbs7    $0C,L76F3
        php
        bvs     L76EE
        ora     $08,x
        bvs     L76F2
        asl     $08,x
        .byte   $70
L76E9:  tsb     $0817
        bvs     L76FA
L76EE:  clc
        php
        bvs     L76FE
L76F2:  .byte   $19
L76F3:  php
        bvs     L7702
        inc     a
        php
        bvs     L7706
L76FA:  .byte   $1B
        php
        bvs     L770A
L76FE:  trb     L7008
        .byte   $0C
L7702:  ora     L7008,x
        .byte   $FF
L7706:  lda     $2781
        .byte   $85
L770A:  brk
        lda     $2782
L770E:  sta     $01
        ldx     $2780
        cly
        dey
L7715:  iny
        lda     (L0000),y
        sta     L0011
L771A:  iny
        lda     (L0000),y
        sta     $10
        lda     #$20
        sta     $13
        sta     $12
        cla
        sta     $15
        sta     $14
        lda     $2783
        beq     L773F
L772F:  pha
        jsr     L4F7A
        stz     $3B33
L7736:  lda     $3B33
        beq     L7736
        pla
        dec     a
        bne     L772F
L773F:  phx
        phy
        lda     L0000
        pha
        lda     $01
        pha
        lda     #$01
        jsr     L3AB7
        pla
        sta     $01
        pla
        sta     L0000
        ply
        plx
        dex
        bne     L7715
        rts

        brk
        jsr     L4000
        brk
        rts

        brk
        bra     L770E
        bra     L778A
        sta     L4EC2
        jsr     L4EC9
        lda     $37CE
        sta     $27C0
        sta     $62
        lda     $37CF
        sta     $27C1
        sta     $63
        rts

        lda     $2780
        sta     L4EC2
        jsr     L4EC9
        lda     $37CE
        sta     L77B0
        .byte   $AD
L778A:  bbs4    $37,L771A
        lda     ($77),y
        jsr     L4F48
        lda     L4D79
        sta     L0000
        lda     L4D7A
        sta     $01
        bsr     L77B5
        bsr     L77CF
        jsr     L781C
        stz     $3B68
        rts

        stz     $0402
        stz     $0403
        .byte   $F3
        tsb     $04
L77B0:  brk
L77B1:  brk
        brk
        tsb     $60
L77B5:  stz     $0402
        stz     $0403
        ldy     #$02
L77BD:  clx
L77BE:  lda     #$FF
        sta     $0404
        lda     #$01
        sta     $0405
        dex
        bne     L77BE
        dey
        bne     L77BD
        rts

L77CF:  clc
        lda     L0000
        adc     #$00
        sta     L0000
        lda     $01
        adc     #$04
        sta     $01
        lda     L0000
        sta     L7807
        sta     L7815
        lda     $01
        sta     L7808
        sta     L7816
        clc
        lda     L0000
        adc     #$02
        sta     L7809
        lda     $01
        bcc     L77F9
        inc     a
L77F9:  sta     L780A
        lda     #$FF
        sta     (L0000)
        lda     #$01
        ldy     #$01
        sta     (L0000),y
        .byte   $F3
L7807:  brk
L7808:  brk
L7809:  brk
L780A:  brk
        inc     L6003,x
L780E:  stz     $0402
        stz     $0403
        .byte   $E3
L7815:  brk
L7816:  brk
        tsb     $04
        brk
        tsb     $60
L781C:  lda     L77B0
        sta     L0000
        lda     L77B1
        sta     $01
        lda     L7815
        sta     $02
        .byte   $AD
        .byte   $16
L782D:  sei
        sta     $03
        ldx     #$08
        bsr     L7835
        rts

L7835:  phx
        lda     L0000
        pha
        lda     $01
        pha
        lda     $02
        pha
        lda     $03
        pha
        ldx     #$20
        bsr     L7866
        rmb1    $F5
        jsr     L4F7A
        jsr     L4F7A
        jsr     LE07B
        bsr     L780E
        smb1    $F5
        pla
        sta     $03
        pla
        sta     $02
        pla
        sta     $01
        pla
        sta     L0000
        plx
        dex
        bne     L7835
        rts

L7866:  phx
        ldx     #$10
        bsr     L7870
        plx
        dex
        bne     L7866
        rts

L7870:  bsr     L7898
        bsr     L78AF
        lda     $0E
        sta     ($02)
        lda     $0F
        ldy     #$01
        sta     ($02),y
        clc
        lda     L0000
        adc     #$02
        sta     L0000
        bcc     L7889
        inc     $01
L7889:  clc
        lda     $02
        adc     #$02
        sta     $02
        bcc     L7894
        inc     $03
L7894:  dex
        bne     L7870
        rts

L7898:  lda     ($02)
        sta     $0E
        ldy     #$01
        lda     ($02),y
        and     #$01
        sta     $0F
        lda     (L0000)
        sta     $10
        lda     (L0000),y
        and     #$01
        .byte   $85
L78AD:  ora     ($60),y
L78AF:  lda     $0E
        sec
        sbc     $10
        and     #$07
        beq     L78BA
        dec     $0E
L78BA:  lda     $0E
        sec
        sbc     $10
        and     #$38
        beq     L78CA
        sec
        lda     $0E
        sbc     #$08
        sta     $0E
L78CA:  lda     $0E
        sec
        sbc     $10
        and     #$C0
        bne     L78DC
        lda     $0F
        sec
        sbc     L0011
        and     #$01
        beq     L78E9
L78DC:  sec
        lda     $0E
        sbc     #$40
        sta     $0E
        lda     $0F
        sbc     #$00
        sta     $0F
L78E9:  rts

        lda     $2780
        sta     L4EC2
        jsr     L4EC9
        lda     $37CE
        sta     $3B6A
        lda     $37CF
        sta     $3B6B
        rts

        stz     $3B68
        rts

L7904:  brk
L7905:  brk
        brk
        brk
        brk
        brk
L790A:  brk
L790B:  brk
L790C:  brk
L790D:  brk
L790E:  brk
L790F:  brk
L7910:  brk
L7911:  brk
        lda     $2780
        sta     L4EC2
        jsr     L4EC9
        lda     $37CE
        sta     L7904
        sta     L0000
        lda     $37CF
        sta     L7905
        sta     $01
        lda     $2781
        cmp     #$01
        bne     L7935
        jmp     L7A0A

L7935:  cmp     #$02
        bne     L793C
        jmp     L7B23

L793C:  stz     $02
        stz     L790E
        lda     #$0F
        sta     $03
        sta     L790F
        stz     L7910
        lda     #$10
        sta     L7911
        jsr     L7C44
        asl     $0E
        rol     $0F
        clc
        lda     $0E
        adc     L0000
        sta     L790A
        lda     $0F
        adc     $01
        sta     L790B
        clc
        lda     L790A
        adc     #$80
        sta     L790C
        cla
        adc     L790B
        sta     L790D
        ldx     #$10
L7978:  jsr     L4F7A
        lda     L790E
        sta     $02
        lda     L790F
        sta     $03
        lda     L790A
        sta     L0000
        lda     L790B
        sta     $01
        bsr     L79D3
        dec     L790F
        sec
        lda     L790A
        sbc     #$80
        sta     L790A
        lda     L790B
        sbc     #$00
        sta     L790B
        lda     L7910
        sta     $02
        lda     L7911
        sta     $03
        lda     L790C
        sta     L0000
        lda     L790D
        sta     $01
        bsr     L79D3
        inc     L7911
        clc
        lda     L790C
        adc     #$80
        sta     L790C
        bcc     L79CC
        inc     L790D
L79CC:  jsr     L4F7A
        dex
        bne     L7978
        rts

L79D3:  jsr     L7C44
        dec     $5A
        st0     #$05
        lda     $F3
        sta     a:$02
        lda     $F4
        and     #$07
        sta     $F4
        sta     a:$03
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        cly
L79F7:  lda     (L0000),y
        sta     a:$02
        iny
        lda     (L0000),y
        sta     a:$03
        iny
        cpy     #$80
        bne     L79F7
        stz     $5A
        rts

L7A0A:  lda     #$0F
        sta     $02
        sta     L790E
        stz     $03
        stz     L790F
        lda     #$10
        sta     L7910
        stz     L7911
        jsr     L7C44
        asl     $0E
        rol     $0F
        clc
        lda     $0E
        adc     L0000
        sta     L790A
        lda     $0F
        adc     $01
        sta     L790B
        clc
        lda     L790A
        adc     #$02
        sta     L790C
        cla
        adc     L790B
        sta     L790D
        ldx     #$10
L7A46:  jsr     L4F7A
        lda     L790E
        sta     $02
        lda     L790F
        sta     $03
        lda     L790A
        sta     L0000
        lda     L790B
        sta     $01
        bsr     L7ACD
        dec     L790E
        sec
        lda     L790A
        sbc     #$02
        sta     L790A
        lda     L790B
        sbc     #$00
        sta     L790B
        lda     L7910
        sta     $02
        lda     L7911
        sta     $03
        lda     L790C
        sta     L0000
        lda     L790D
        sta     $01
        bsr     L7ACD
        inc     L7910
        clc
        lda     L790C
        adc     #$02
        sta     L790C
        bcc     L7A9A
        inc     L790D
L7A9A:  jsr     L4F7A
        dex
        bne     L7A46
        ldx     #$20
L7AA2:  lda     L7910
        sta     $02
        lda     L7911
        sta     $03
        lda     L790C
        sta     L0000
        lda     L790D
        sta     $01
        bsr     L7ACD
        inc     L7910
        clc
        lda     L790C
        adc     #$02
        sta     L790C
        bcc     L7AC9
        inc     L790D
L7AC9:  dex
        bne     L7AA2
L7ACC:  rts

L7ACD:  phx
        jsr     L7C44
        dec     $5A
        st0     #$05
        lda     $F3
        sta     a:$02
        lda     $F4
        and     #$07
        ora     #$10
        sta     $F4
        sta     a:$03
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        ldx     #$20
        ldy     #$01
L7AF7:  lda     (L0000)
        sta     a:$02
        lda     (L0000),y
        sta     a:$03
        clc
        lda     L0000
        adc     #$80
        sta     L0000
        bcc     L7B0C
        inc     $01
L7B0C:  dex
        bne     L7AF7
        st0     #$05
        lda     $F3
        sta     a:$02
        lda     $F4
        and     #$07
        sta     $F4
        sta     a:$03
        stz     $5A
        plx
        rts

L7B23:  stz     $02
        stz     L790E
        stz     $03
        stz     L790F
        lda     #$01
        sta     L7910
        lda     #$00
        sta     L7911
        jsr     L7C44
        asl     $0E
        rol     $0F
        clc
        lda     $0E
        adc     L0000
        sta     L790A
        lda     $0F
        adc     $01
        sta     L790B
        clc
        lda     L790A
        adc     #$02
        sta     L790C
        cla
        adc     L790B
        sta     L790D
        lda     L790E
        sta     $02
        lda     L790F
        sta     $03
        lda     L790A
        sta     L0000
        lda     L790B
        sta     $01
        bsr     L7B74
        rts

L7B74:  jsr     L7C44
        ldx     #$10
        ldy     #$01
L7B7B:  phx
        jsr     L7C09
        clc
        lda     $0E
        adc     #$01
        sta     $0E
        bcc     L7B8A
        inc     $0F
L7B8A:  clc
        lda     L0000
        adc     #$02
        sta     L0000
        bcc     L7B95
        inc     $01
L7B95:  bsr     L7C09
        sec
        lda     $0E
        sbc     #$01
        sta     $0E
        lda     $0F
        sbc     #$00
        sta     $0F
        sec
        lda     L0000
        sbc     #$02
        sta     L0000
        lda     $01
        sbc     #$00
        sta     $01
        plx
        dex
        bne     L7B7B
        lda     L7910
        sta     $02
        lda     L7911
        sta     $03
        lda     L790C
        sta     L0000
        lda     L790D
        sta     $01
        bsr     L7C44
        ldx     #$10
        ldy     #$01
L7BCF:  phx
        bsr     L7C09
        sec
        lda     $0E
        sbc     #$01
        sta     $0E
        lda     $0F
        sbc     #$00
        sta     $0F
        sec
        lda     L0000
        sbc     #$02
        sta     L0000
        lda     $01
        sbc     #$00
        sta     $01
        bsr     L7C09
        clc
        lda     $0E
        adc     #$01
        sta     $0E
        bcc     L7BF9
        inc     $0F
L7BF9:  clc
        lda     L0000
        adc     #$02
        sta     L0000
L7C00:  bcc     L7C04
        inc     $01
L7C04:  plx
        dex
        bne     L7BCF
        rts

L7C09:  jsr     L4F7A
        ldx     #$20
        dec     $5A
L7C10:  st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        lda     (L0000)
        sta     a:$02
        lda     (L0000),y
        sta     a:$03
        clc
        lda     $0E
        adc     #$02
        sta     $0E
        bcc     L7C33
        inc     $0F
L7C33:  clc
        lda     L0000
        adc     #$04
L7C38:  sta     L0000
        bcc     L7C3E
        inc     $01
L7C3E:  dex
        bne     L7C10
        stz     $5A
        rts

L7C44:  stz     $0E
        lda     $03
        lsr     a
        ror     $0E
        lsr     a
        ror     $0E
        sta     $0F
        clc
        lda     $0E
        adc     $02
        sta     $0E
        bcc     L7C5B
        inc     $0F
L7C5B:  rts

        dec     $5A
        st0     #$07
        st1     #$00
        st2     #$00
        stz     $5A
        rts

        dec     $5A
        st0     #$07
        st1     #$00
        st2     #$01
        stz     $5A
        rts

        brk
        .byte   $0B
        bsr     L7CC3
        tam     #$2D
        tam     #$47
        rol     $3030
        .byte   $31
L7C7E:  lda     $2789
        beq     L7CD6
        jsr     L7CE1
        jsr     L75B0
        lda     L7E49
        lsr     a
        bcc     L7CD0
        jsr     L75B7
        stz     $2787
        cly
        ldx     #$02
        stx     $2781
        lsr     a
        bcc     L7CA2
        stx     $2784
        iny
L7CA2:  lsr     a
        bcc     L7CA9
        stx     $2782
        iny
L7CA9:  lsr     a
        bcc     L7CB0
        stx     $2785
        iny
L7CB0:  lsr     a
        bcc     L7CB7
        stx     $2786
        iny
L7CB7:  lsr     a
        bcc     L7CBE
        stx     $2783
        iny
L7CBE:  cpy     #$05
        bne     L7CCF
        .byte   $A2
L7CC3:  ora     ($8E,x)
        smb0    $27
        lsr     a
        bcc     L7CCF
        ldx     #$02
        stx     $2787
L7CCF:  rts

L7CD0:  lda     #$01
        sta     $2781
        rts

L7CD6:  bsr     L7D1F
        jsr     L75B0
        lda     #$01
        sta     $2781
        rts

L7CE1:  lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        lda     #$49
        sta     $FA
        lda     #$7E
        sta     $FB
        lda     #$88
        sta     $FC
        lda     #$00
        sta     $FD
        jsr     L7DBD
        jsr     LE04E
        cmp     #$00
        beq     L7D1D
        lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        jsr     LE054
        stz     L7E49
        tii     $7E49,$7E4A,$0198
        jsr     L7D50
        sec
        rts

L7D1D:  clc
        rts

L7D1F:  lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        lda     #$49
        sta     $FA
        lda     #$7E
        sta     $FB
        lda     #$99
        sta     $FC
        lda     #$01
        sta     $FD
        lda     #$00
        sta     $FE
        lda     #$00
        sta     $FF
        jsr     LE04E
        lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        jsr     LE054
        jsr     L7D74
L7D50:  lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        lda     #$49
        sta     $FA
        lda     #$7E
        sta     $FB
        lda     #$99
        sta     $FC
        lda     #$01
        sta     $FD
        lda     #$00
        sta     $FE
        lda     #$00
        sta     $FF
        jsr     LE051
        rts

L7D74:  lda     #$49
        sta     L0000
        lda     #$7E
        sta     $01
        ldx     $278C
        beq     L7D91
L7D81:  clc
        lda     L0000
        adc     #$88
        sta     L0000
        lda     $01
        adc     #$00
        sta     $01
        dex
        bne     L7D81
L7D91:  lda     L0000
        sta     L7DAD
        sta     L7DAF
        lda     $01
        sta     L7DAE
        sta     L7DB0
        inc     L7DAF
        bne     L7DA9
        inc     L7DB0
L7DA9:  cla
        sta     (L0000)
        .byte   $73
L7DAD:  .byte   $49
L7DAE:  .byte   $7E
L7DAF:  lsr     a
L7DB0:  ror     a:$87,x
        lda     $278D
        beq     L7DBC
        lda     #$80
        sta     (L0000)
L7DBC:  rts

L7DBD:  stz     $FE
        stz     $FF
        ldx     $278C
        beq     L7DD6
L7DC6:  clc
L7DC7:  lda     $FE
        adc     #$88
        sta     $FE
        lda     $FF
        adc     #$00
        sta     $FF
        dex
        bne     L7DC6
L7DD6:  rts

        jsr     LE04B
        lda     $FC
        cmp     #$8A
        lda     $FD
        sbc     #$00
        rts

        lda     $278C
        sta     L7E49
        lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        lda     #$49
        sta     $FA
        lda     #$7E
        sta     $FB
        lda     #$01
        sta     $FC
        lda     #$00
        sta     $FD
        lda     #$98
        sta     $FE
        lda     #$01
        sta     $FF
        jsr     LE051
        rts

        lda     #$72
        sta     $F8
        lda     #$7C
        sta     $F9
        lda     #$49
        sta     $FA
        lda     #$7E
        sta     $FB
        lda     #$01
        sta     $FC
        lda     #$00
        sta     $FD
        lda     #$98
        sta     $FE
        lda     #$01
        sta     $FF
        jsr     LE04E
        lda     L7E49
        sta     $278C
        rts

        stz     $2780
        jsr     L7CE1
        lda     L7E49
        bne     L7E43
        rts

L7E43:  .byte   $A9
L7E44:  bbs7    $8D,L7DC7
        rmb2    $60
L7E49:  brk
        brk
        brk
L7E4C:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        .byte   $FF
        .byte   $FF
L7FE4:  bbs7    $FF,$7FE6
L7FE7:  bbs7    $FF,$7FE9
L7FEA:  bbs7    $FF,$7FEC
L7FED:  bbs7    $FF,$7FEF
L7FF0:  bbs7    $FF,$7FF2
L7FF3:  bbs7    $FF,$7FF5
L7FF6:  bbs7    $FF,$7FF8
L7FF9:  bbs7    $FF,$7FFB
L7FFC:  bbs7    $FF,$7FFE
L7FFF:  .byte   $FF
L8000:  dec     $5A
        stz     $220C
        stz     $220D
        stz     $2210
        .byte   $9C
L800C:  ora     ($22),y
        st0     #$08
        st1     #$00
        st2     #$00
        st0     #$07
        st1     #$00
        st2     #$00
        stz     $5A
        jsr     L45A6
        lda     L0000
        sta     L004C
        lda     $01
        sta     $4D
        lda     #$08
        clc
        .byte   $65
L802B:  brk
        sta     L0000
        bcc     L8032
        inc     $01
L8032:  lda     L0000
        sta     L47CB
        lda     $01
        sta     L47CC
        lda     $01
        clc
        adc     #$10
        sta     $01
        lda     L0000
        sta     L47CD
        lda     $01
        .byte   $8D
L804B:  dec     $A947
        brk
        sta     L47C7
        lda     #$01
L8054:  sta     L47C8
        ldy     #$04
        lda     (L004C),y
        sta     L47BF
        sta     $0E
        stz     $0F
        iny
        lda     (L004C),y
        sta     L47BE
        sta     $10
        jsr     L4696
        lda     $0E
        sta     L47C9
        lda     $0F
        sta     L47CA
        ldy     #$02
        lda     (L004C),y
        sta     L0000
        iny
        lda     (L004C),y
        sta     $01
        lda     L004C
        clc
        adc     L0000
        sta     L0000
        lda     $4D
        adc     $01
        sta     $01
        lda     L0000
        sta     $3B6A
        lda     $01
        sta     $3B6B
        lda     L0000
        sta     L47D1
        lda     $01
        sta     L47D2
        stz     $02
        stz     $03
        ldy     #$06
        lda     (L004C),y
        sta     $3B6F
        asl     a
        asl     a
        asl     a
        asl     a
        tax
        lda     $3B68
        bne     L80BB
        jsr     L48FC
L80BB:  rts

        stz     $0E
        stz     $0F
        jsr     L41AB
        rts

        lda     $52
        lsr     a
        ror     a
        ror     a
        sta     L47D5
        sta     L47D6
        clc
        adc     #$1F
        sta     L47D7
        lda     $55
        lsr     a
        lda     $54
        ror     a
        ror     a
        ror     a
        sta     L47D8
        sta     L47D9
        clc
        adc     #$1E
        sta     L47DA
        lda     $12
        sta     L47C2
        sta     L47C3
        lda     $0E
        tax
        lda     $10
        sta     L47BC
        sta     L47BA
        lda     L0011
        sta     L47BD
        sta     L47BB
        txa
        bne     L8115
        lda     #$FF
        sta     $51
L810B:  stz     $50
        stz     L47BA
        stz     L47BB
        bra     L8148
L8115:  dec     a
        bne     L8126
        stz     $51
        stz     L47BC
        stz     L47BD
        lda     #$01
        sta     $50
        bra     L8148
L8126:  dec     a
        bne     L8137
        lda     #$01
        sta     $51
        stz     $50
        stz     L47BA
        stz     L47BB
        bra     L8148
L8137:  dec     a
        bne     L8148
        stz     $51
        stz     L47BC
        stz     L47BD
        lda     #$FF
        sta     $50
        bra     L8148
L8148:  bsr     L81A2
L814A:  dec     $5A
        lda     L47C3
        and     #$7F
        dec     a
        sta     L47C3
        bne     L8162
        lda     L47C2
        ora     #$80
        sta     L47C3
        jsr     L4175
L8162:  stz     $5A
        bsr     L81A2
        lda     L47BC
        ora     L47BD
        ora     L47BA
        ora     L47BB
        bne     L814A
        rts

        lda     $51
        ora     $50
        beq     L81A1
        lda     $51
        clc
        adc     $2210
        sta     $2210
        ldy     #$69
        lda     $50
        bpl     L818C
        ldy     #$E9
L818C:  sty     L4199
        clc
        adc     $220C
        sta     $220C
        lda     $220D
        adc     #$00
        sta     $220D
        jsr     L4215
L81A1:  rts

L81A2:  lda     #$01
        sta     $59
L81A6:  lda     $59
        bne     L81A6
        rts

        lda     $0E
        sta     L47C4
        lda     #$20
        sta     L47B8
        lda     #$20
        sta     L47B9
        lda     #$1F
        sta     L47D5
        sta     L47D8
        stz     L47D6
        sta     L47D7
        stz     L47D9
        lda     #$1E
        sta     L47DA
        stz     $52
        stz     $54
L81D4:  lda     $14
        sta     $55
        lda     $15
        sta     $53
        dec     $5A
        ldy     #$40
L81E0:  phy
        ldx     #$08
L81E3:  phx
        lda     #$01
        sta     $50
        jsr     L4175
        jsr     L49FA
        plx
        dex
        bne     L81E3
        ply
        dey
        bne     L81E0
        ldy     #$40
L81F8:  phy
        ldx     #$08
L81FB:  phx
        lda     #$FF
        sta     $50
        jsr     L4175
        jsr     L49FA
        plx
        dex
        bne     L81FB
        ply
        dey
        bne     L81F8
L820E:  stz     $51
        stz     $50
        stz     $5A
        rts

        lda     $50
        beq     L8224
        jsr     L44A2
        lda     L47D4
        beq     L8224
        jsr     L42DB
L8224:  lda     $51
        beq     L8233
        jsr     L4417
        lda     L47D3
        beq     L8233
        jsr     L424B
L8233:  lda     L47D8
        sta     $0E
        lda     L47D5
        sta     $0F
        .byte   $20
        .byte   $8E
L823F:  eor     $A5
        sxy
        .byte   $8D
L8243:  .byte   $DB
        rmb4    $A5
        st0     #$8D
        .byte   $DC
        rmb4    $60
        stz     $13
        jsr     L43D6
        lda     L47B8
        sta     L0011
        lda     #$02
L8257:  .byte   $85
L8258:  bpl     $821C
        lda     ($4E),y
        sta     $14
        iny
        lda     ($4E),y
        sta     $15
        iny
        jsr     L43A1
        lda     $58
        and     #$01
        eor     #$03
        dec     a
        tax
        stz     $12
L8271:  bsr     L82A6
        dec     L0011
        dex
        bne     L8271
        jsr     L42BF
        lda     $58
        and     #$02
        sta     $58
L8281:  ldy     $10
        inc     $10
        inc     $10
        lda     ($4E),y
        sta     $14
        iny
        lda     ($4E),y
        sta     $15
        jsr     L43A1
        ldx     #$02
        stz     $12
L8297:  bsr     L82A6
        dec     L0011
        beq     L82A5
        dex
        bne     L8297
        jsr     L42BF
        bra     L8281
L82A5:  rts

L82A6:  phx
        ldy     $12
        ldx     $13
        lda     (L0000),y
        sta     L47E0,x
        iny
        inx
        lda     (L0000),y
        sta     L47E0,x
        iny
        inx
        sty     $12
        stx     $13
        plx
        rts

        inc     $56
L82C1:  ldx     $56
        cpx     #$10
        bne     L82DA
        stz     $56
        stz     $10
        lda     L47C4
        pha
        inc     a
        sta     L47C4
        jsr     L43D6
        pla
        sta     L47C4
L82DA:  rts

        lda     $3B75
        sta     $0F
        lda     $3B74
        lsr     $0F
        ror     a
        lsr     a
        lsr     a
        sta     $3B7A
        lda     $3B76
        sec
        sbc     $3B74
        sta     $0E
        .byte   $AD
        .byte   $77
L82F6:  .byte   $3B
        sbc     $3B75
        sta     $0F
        lda     $0E
        lsr     $0F
        ror     a
        lsr     a
        lsr     a
        sta     $3B7B
        stz     $13
        jsr     L43D6
        lda     L47B9
        sta     L0011
        cly
        lda     ($4E),y
        sta     $14
        iny
        lda     ($4E),y
        sta     $15
        jsr     L43A1
        lda     $58
        lsr     a
        eor     #$03
        tax
        dex
        stz     $12
        stz     $13
L8328:  bsr     L8386
        dec     L0011
        dex
        bne     L8328
        jsr     L4358
        lda     $58
        and     #$01
        sta     $58
L8338:  cly
        lda     ($4E),y
        sta     $14
        iny
        lda     ($4E),y
        sta     $15
        jsr     L43A1
        ldx     #$02
        stz     $12
L8349:  bsr     L8386
        .byte   $C6
L834C:  ora     ($F0),y
        php
        dex
        bne     L8349
        jsr     L4358
        bra     L8338
        rts

        lda     #$20
        clc
        adc     $4E
        sta     $4E
        lda     #$00
        adc     $4F
        sta     $4F
        inc     $57
        ldx     $57
        cpx     #$10
        bne     L8385
        stz     $57
        lda     L47C4
        pha
        ldx     L47BE
        stx     $0E
        clc
        adc     $0E
        sta     L47C4
        jsr     L43D6
        pla
        sta     L47C4
L8385:  rts

L8386:  phx
        ldy     $12
        ldx     $13
        lda     (L0000),y
        sta     L4820,x
        iny
        inx
        lda     (L0000),y
        sta     L4820,x
        iny
        inx
        iny
        iny
        sty     $12
        stx     $13
        plx
        rts

        lda     $14
        sta     $0E
        lda     $15
        sta     $0F
        asl     $0E
        rol     $0F
        asl     $0E
        rol     $0F
        asl     $0E
        rol     $0F
        lda     L47CB
L83B8:  clc
        adc     $0E
        sta     $0E
        lda     L47CC
        adc     $0F
        sta     $0F
        lda     $58
        asl     a
        clc
        adc     $0E
        sta     $0E
        sta     L0000
        cla
        adc     $0F
        sta     $0F
        sta     $01
        rts

        lda     $57
L83D8:  sta     $4E
        stz     $4F
        asl     $4E
        rol     $4F
        asl     $4E
        rol     $4F
        asl     $4E
        rol     $4F
        asl     $4E
        rol     $4F
        lda     $56
        clc
        adc     $4E
        sta     $4E
        cla
        adc     $4F
        sta     $4F
        asl     $4E
        rol     $4F
        lda     L47C4
        beq     L8407
        asl     a
        clc
        adc     $4F
        sta     $4F
L8407:  lda     L47CD
        clc
        adc     $4E
        sta     $4E
        lda     L47CE
        adc     $4F
        sta     $4F
        rts

        lda     $54
        and     #$07
        bne     L848B
        lda     #$01
        sta     L47D3
        lda     $51
        bmi     L845A
        lda     L47DA
        inc     a
        and     #$1F
        sta     L47DA
        sta     L47D8
        lda     L47D9
        inc     a
        and     #$1F
        sta     L47D9
        lda     $54
        pha
        lda     $55
        pha
        lda     #$F8
        clc
        adc     $54
        sta     $54
        lda     #$00
        adc     $55
        sta     $55
        jsr     L4552
        pla
        sta     $55
        pla
        sta     $54
        jmp     L448B

L845A:  lda     L47D9
        dec     a
        and     #$1F
        sta     L47D9
        sta     L47D8
        lda     L47DA
        dec     a
        and     #$1F
        sta     L47DA
        lda     $54
        pha
        lda     $55
        pha
        lda     #$F8
        clc
        adc     $54
        sta     $54
        lda     $55
        sbc     #$00
        sta     $55
        jsr     L4552
        pla
        sta     $55
        pla
        sta     $54
L848B:  ldy     #$69
        lda     $51
        bpl     L8493
        ldy     #$E9
L8493:  sty     L449D
        clc
        adc     $54
        sta     $54
        lda     $55
        adc     #$00
        sta     $55
        rts

        lda     $52
        and     #$07
        beq     L84AB
        jmp     L4519

L84AB:  lda     #$01
        sta     L47D4
        lda     $50
        bmi     L84E2
        lda     L47D7
        inc     a
        and     #$3F
        sta     L47D5
        sta     L47D7
        lda     L47D6
        inc     a
        and     #$3F
        sta     L47D6
        lda     L47D9
        sta     L47D8
        lda     $52
        pha
        lda     $53
        pha
        inc     $53
        jsr     L4552
        pla
        sta     $53
        pla
        sta     $52
        bra     L8519
L84E2:  lda     L47D6
        dec     a
        and     #$3F
        sta     L47D5
        sta     L47D6
        lda     L47D7
        dec     a
        and     #$3F
        sta     L47D7
        lda     L47D9
        sta     L47D8
        lda     $52
        pha
        lda     $53
        pha
        lda     #$F8
        clc
        adc     $52
        sta     $52
        lda     $53
        sbc     #$00
        sta     $53
        jsr     L4552
        pla
        sta     $53
        pla
        sta     $52
L8519:  lda     $50
        bpl     L8536
        lda     $52
        ora     $53
        bne     L852D
        lda     $3B70
        sta     $52
        lda     $3B71
        sta     $53
L852D:  lda     $52
        bne     L8533
        dec     $53
L8533:  dec     $52
        rts

L8536:  inc     $52
        bne     L853C
        inc     $53
L853C:  lda     $52
        sec
        sbc     $3B70
        sta     $0E
        lda     $53
        sbc     $3B71
        ora     $0E
        bne     L8551
        stz     $52
        stz     $53
L8551:  rts

        lda     $52
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     $56
        lda     $54
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        sta     $57
        cla
        ldx     $55
        beq     L8573
        lda     L47BE
        sta     $0E
        cla
L856D:  clc
        adc     $0E
        dex
        bne     L856D
L8573:  clc
        adc     $53
        sta     L47C4
        lda     $54
        and     #$08
        lsr     a
        lsr     a
        sta     $0E
        lda     $52
        and     #$08
        lsr     a
        lsr     a
        lsr     a
        clc
        adc     $0E
        sta     $58
        rts

        stz     $02
        lda     $0E
        lsr     a
        ror     $02
        lsr     a
        ror     $02
        sta     $03
        lda     $0F
        clc
        adc     $02
        sta     $02
        bcc     L85A5
        inc     $03
L85A5:  rts

        stz     L47B8
        tii     $47B8,$47B9,$00A7
        rts

        lda     $12
        pha
        ldy     L0011
        lda     #$08
        sta     $0E
        jsr     L4696
        lda     $0E
        sta     $54
        lda     $0F
        sta     $55
        sty     $10
        lda     #$08
        sta     $0E
        jsr     L4696
        lda     $0E
        sta     $52
        lda     $0F
        sta     $53
        jsr     L4552
        lda     $13
        asl     a
        sta     L47B8
        pla
        asl     a
        sta     L47B9
        dec     $5A
        jsr     L4932
        stz     $5A
        lda     $14
        sta     $0E
        lda     $15
        sta     $0F
        jsr     L458E
        lda     $02
        sta     $06
        lda     $03
        sta     $07
        lda     $56
        sta     L466A
        lda     $57
        sta     $16
        lda     $58
        sta     $17
        lda     L47B9
        lsr     a
        tax
L8610:  phx
        lda     L466A
        sta     $56
        lda     $16
        sta     $57
        lda     $17
        sta     $58
        jsr     L424B
        lda     #$E0
        sta     L0000
        lda     #$47
        sta     $01
        lda     $06
        sta     $02
        lda     $07
        sta     $03
        lda     L47B8
        sta     $0E
        stz     $0F
        jsr     L466B
        lda     #$40
        clc
        adc     $06
        sta     $06
        bcc     L8646
        inc     $07
L8646:  lda     $17
        eor     #$02
        sta     $17
        bne     L8665
        inc     $16
        lda     $16
        cmp     #$10
        bne     L8665
        stz     $16
        lda     L47BE
        clc
        adc     L47C4
        sta     L47C4
        jsr     L43D6
L8665:  plx
        dex
        bne     L8610
        rts

        brk
        dec     $5A
        st0     #$00
        lda     $02
        sta     a:$02
        lda     $03
        sta     a:$03
        st0     #$02
        lda     $0E
        beq     L8693
        sta     L4691
        lda     L0000
        sta     L468D
        lda     $01
        sta     L468E
        tia     $00,$02,$0000
L8693:  stz     $5A
        rts

        stz     $0F
        stz     L0011
        lda     $0E
        sta     $12
        stz     $0E
        ldx     #$01
        bbs7    $12,L86BB
        bbs6    $12,L86BC
        bbs5    $12,L86BD
        bbs4    $12,L86BE
        bbs3    $12,L86BF
        bbs2    $12,L86C0
        bbs1    $12,L86C1
        bbs0    $12,L86C2
        rts

L86BB:  inx
L86BC:  inx
L86BD:  inx
L86BE:  inx
L86BF:  inx
L86C0:  inx
L86C1:  inx
L86C2:  lsr     $12
        bcc     L86D3
        clc
        lda     $10
        adc     $0E
        sta     $0E
        lda     L0011
        adc     $0F
        sta     $0F
L86D3:  asl     $10
        rol     L0011
        dex
        bne     L86C2
        rts

        bsr     L871E
        bsr     L872E
        bsr     L8740
        jsr     L4786
        ldy     L47B6
L86E7:  dec     $5A
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        ldx     L47B5
L86FA:  lda     $04
        sta     a:$02
        lda     $05
        sta     a:$03
        inc     $04
        bne     L870A
        inc     $05
L870A:  dex
        bne     L86FA
        stz     $5A
        clc
        lda     $0E
        adc     #$40
        sta     $0E
        bcc     L871A
        inc     $0F
L871A:  dey
        bne     L86E7
        rts

L871E:  lda     L0000
        cmp     #$FF
        beq     L8728
        sta     L47B3
        rts

L8728:  lda     ($02)
        sta     L47B3
        rts

L872E:  lda     $01
        cmp     #$FF
        beq     L8738
        sta     L47B4
        rts

L8738:  ldy     #$01
        lda     ($02),y
        sta     L47B4
        rts

L8740:  ldy     #$02
        lda     ($02),y
        sta     L47B5
        iny
        lda     ($02),y
        sta     L47B6
        iny
        lda     ($02),y
        asl     a
        asl     a
        asl     a
        asl     a
        sta     L47B7
        clc
        lda     $02
        adc     #$05
        sta     L477A
        cla
        adc     $03
        sta     L477B
        bsr     L8768
        rts

L8768:  php
        sei
        rmb1    $F5
        plp
        jsr     LE07B
        lda     L47B7
        sta     $0402
        stz     $0403
        tia     $00,$0404,$0020
        php
        sei
        smb1    $F5
        plp
        rts

        bsr     L8799
        ldx     #$04
L878A:  lsr     $05
        ror     $04
        dex
        bne     L878A
        lda     $05
        ora     L47B7
        sta     $05
        rts

L8799:  stz     $0E
        lda     L47B4
        lsr     a
        ror     $0E
        lsr     a
        ror     $0E
        sta     $0F
        clc
        lda     $0E
        adc     L47B3
        sta     $0E
        bcc     L87B2
        inc     $0F
L87B2:  rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        dec     $5A
        bsr     L88A6
        st0     #$05
        lda     $F3
        ora     #$88
        ora     $27D9
        sta     $F3
        sta     a:$02
        stz     $5A
        rts

        dec     $5A
        bsr     L88A6
        st0     #$05
        lda     $F3
        and     #$3B
        sta     $F3
        sta     a:$02
        stz     $3B78
        lda     #$FF
        sta     $3B70
        sta     $3B71
        sta     $3B72
        sta     $3B73
        stz     $3B74
        stz     $3B75
        stz     $3B76
        lda     #$01
        sta     $3B77
        stz     $5A
        rts

L88A6:  stz     $3B33
L88A9:  lda     $3B33
        beq     L88A9
        rts

        dec     $5A
        bsr     L88A6
        st0     #$00
        st1     #$00
        st2     #$00
        st0     #$02
        ldx     #$02
L88BD:  cly
L88BE:  st1     #$00
        st2     #$01
        st1     #$00
        st2     #$01
        st1     #$00
        st2     #$01
        st1     #$00
        st2     #$01
        dey
        bne     L88BE
        dex
        bne     L88BD
        stz     $5A
        rts

        dec     $5A
        bsr     L88A6
        st0     #$00
        lda     $27DA
        sta     a:$02
        lda     $27DB
        sta     a:$03
        st0     #$02
        clx
L88EC:  st1     #$00
        st2     #$00
        st1     #$00
        st2     #$00
        dex
        bne     L88EC
        stz     $5A
        rts

        ldx     $0E
        dec     $5A
        lda     $02
        sta     $0402
        lda     $03
        sta     $0403
        cly
L8909:  lda     (L0000),y
        sta     $0404
        iny
        lda     (L0000),y
        .byte   $8D
L8912:  ora     $04
        iny
        bne     L8919
        inc     $01
L8919:  dex
        bne     L8909
        stz     $5A
        rts

        st0     #$05
        lda     $F3
        sta     a:$02
        lda     $F4
        and     #$07
        ora     #$10
        sta     $F4
        sta     a:$03
        rts

        st0     #$05
        lda     $F3
        sta     a:$02
        lda     $F4
        and     #$07
        sta     $F4
        sta     a:$03
        rts

        tma     #$08
        pha
        tma     #$10
        pha
        tma     #$20
        pha
        tma     #$40
        pha
        lda     $300A
        tam     #$08
        inc     a
        tam     #$10
        inc     a
        tam     #$20
        inc     a
        tam     #$40
        lda     a:L0000
        pha
        ldx     $5A
        beq     L8968
        jmp     L49E2

L8968:  bit     #$04
        bne     L896F
        jmp     L4975

L896F:  jsr     L5E2B
        jmp     L49E2

        bit     #$20
        bne     L897C
        jmp     L49E2

L897C:  lda     $3B69
        beq     L8984
        jsr     L5CE4
L8984:  jsr     L4BB0
        ora     #$00
        bne     L8991
        lda     L47C3
        asl     a
        bcc     L89BA
L8991:  st0     #$08
        lda     $2210
        sta     a:$02
        lda     $2211
        sta     a:$03
        lda     $3B78
        bne     L89B4
        st0     #$07
        lda     $220C
        sta     a:$02
        lda     $220D
        sta     a:$03
        bra     L89BA
L89B4:  st0     #$07
        st1     #$00
        st2     #$00
L89BA:  jsr     L56DE
        jsr     L563D
        jsr     L50F1
        lda     L47C3
        asl     a
        bcc     L89CC
        jsr     L49FA
L89CC:  jsr     L5111
        jsr     L570A
        stz     $59
        lda     $3B78
        beq     L89E2
        st0     #$07
        st1     #$00
        st2     #$00
        stz     $3B79
L89E2:  pla
        and     #$20
        beq     L89ED
        inc     $3B33
        inc     $2249
L89ED:  pla
        tam     #$40
        pla
        tam     #$20
        pla
        tam     #$10
        pla
        tam     #$08
        rts

        lda     $51
        beq     L8A01
        jsr     L4A09
L8A01:  lda     $50
        beq     L8A08
        jsr     L4A84
L8A08:  rts

        lda     L47D3
        beq     L8A56
        jsr     L4932
        st0     #$00
        lda     L47DB
        sta     a:$02
        lda     L47DC
        sta     a:$03
        st0     #$02
        ldx     #$20
        cly
L8A24:  lda     L47E0,y
        sta     a:$02
        iny
        lda     L47E0,y
        sta     a:$03
        iny
        dex
        bne     L8A52
        lda     L47D8
        sta     $3B
        lda     L47D5
        sta     L003A
        jsr     L4B24
        ldx     #$20
        st0     #$00
        lda     $3C
        sta     a:$02
        lda     $3D
        sta     a:$03
        st0     #$02
L8A52:  cpy     #$40
        bne     L8A24
L8A56:  lda     L47BC
        sta     L0038
        lda     L47BD
        sta     $39
        lda     L0038
        ora     $39
        beq     L8A76
        lda     L0038
        bne     L8A6C
        dec     $39
L8A6C:  dec     L0038
        bne     L8A76
        lda     $39
        bne     L8A76
        stz     $51
L8A76:  lda     L0038
        sta     L47BC
        lda     $39
        sta     L47BD
        stz     L47D3
        rts

        lda     L47D4
        beq     L8AF6
        jsr     L491F
        st0     #$00
        lda     $3B75
        sta     $39
        lda     $3B74
        asl     a
        rol     $39
        asl     a
        rol     $39
        asl     a
        rol     $39
        clc
        adc     L47DB
        sta     a:$02
        lda     $39
        adc     L47DC
        sta     a:$03
        st0     #$02
        lda     L47D8
        clc
        adc     $3B7A
        sta     L0038
        lda     $3B7A
        asl     a
        tay
        ldx     $3B7B
L8AC1:  lda     L4820,y
        sta     a:$02
        iny
        lda     L4820,y
        sta     a:$03
        iny
        inc     L0038
        lda     #$20
        cmp     L0038
        bne     L8AF3
        lda     $3B7A
        sta     $3B
        lda     L47D5
        sta     L003A
        bsr     L8B24
        stz     L0038
        st0     #$00
        lda     $3C
        sta     a:$02
        lda     $3D
        sta     a:$03
        st0     #$02
L8AF3:  dex
        bne     L8AC1
L8AF6:  lda     L47BA
        sta     L0038
        lda     L47BB
        sta     $39
        lda     L0038
        ora     $39
        beq     L8B16
        lda     L0038
        bne     L8B0C
        dec     $39
L8B0C:  dec     L0038
        bne     L8B16
        lda     $39
        bne     L8B16
        stz     $50
L8B16:  lda     L0038
        sta     L47BA
        lda     $39
        sta     L47BB
        stz     L47D4
        rts

L8B24:  stz     $3C
        lda     $3B
        lsr     a
        ror     $3C
        lsr     a
        ror     $3C
        sta     $3D
        lda     L003A
        clc
        adc     $3C
        sta     $3C
        bcc     L8B3B
        inc     $3D
L8B3B:  rts

        php
        sei
        lda     $220C
        sta     L4C12
        lda     $220D
        sta     L4C13
        lda     $2210
        sta     L4C14
        lda     $2211
        sta     L4C15
        lda     $0E
        bne     L8B81
        stz     L4C0E
        stz     L4C0D
        stz     L4C11
        lda     $10
        beq     L8BA6
        lda     L4C12
        sta     $220C
        lda     L4C13
        sta     $220D
        lda     L4C14
        sta     $2210
        lda     L4C15
        sta     $2211
        bra     L8BA6
L8B81:  ldx     #$01
        stx     L4C11
        dec     a
        beq     L8B9B
        lda     $10
        eor     #$FF
        inc     a
        sta     L4C0E
        stz     L4C0D
        lda     #$01
        sta     L4C11
        bra     L8BA6
L8B9B:  lda     $10
        eor     #$FF
        inc     a
        sta     L4C0D
        stz     L4C0E
L8BA6:  lda     $12
        sta     L4C0F
        sta     L4C10
        plp
        rts

        lda     L4C11
        beq     L8C0C
        lda     L4C10
        dec     a
        sta     L4C10
        bne     L8C0C
        lda     L4C0F
        sta     L4C10
        ldy     #$69
        lda     L4C0D
        beq     L8BE7
        eor     #$FF
        inc     a
        sta     L4C0D
        bpl     L8BD5
        ldy     #$E9
L8BD5:  sty     L4BE2
        clc
        adc     $220C
        sta     $220C
        lda     $220D
        adc     #$00
        sta     $220D
L8BE7:  ldy     #$69
        beq     L8C0A
        lda     L4C0E
        eor     #$FF
        inc     a
        sta     L4C0E
        bpl     L8BF8
        ldy     #$E9
L8BF8:  sty     L4C05
        clc
        adc     $2210
        sta     $2210
        lda     $2211
        adc     #$00
        sta     $2211
L8C0A:  lda     #$01
L8C0C:  rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        stz     L4DDA
        tii     $4DDA,$4DDB,$0316
        lda     L0000
        sta     L4DE3
        lda     $01
        sta     L4DE4
        cly
        lda     (L0000),y
        sta     L4DE5
        iny
        lda     (L0000),y
        sta     L4DE6
        lda     L0000
        clc
        adc     L4DE5
        sta     L4DE5
        lda     $01
        adc     L4DE6
        sta     L4DE6
        iny
        lda     (L0000),y
        sta     L4DE7
        iny
        lda     (L0000),y
        sta     L4DE8
        lda     L0000
        clc
        adc     L4DE7
        sta     L4DE7
        lda     $01
        adc     L4DE8
        sta     L4DE8
        iny
        lda     (L0000),y
        sta     L4DE9
        iny
        lda     (L0000),y
        sta     L4DEA
        lda     L0000
        clc
        adc     L4DE9
        sta     L4DE9
        lda     $01
        adc     L4DEA
        sta     L4DEA
        iny
        lda     (L0000),y
        sta     L4DEB
        iny
        lda     (L0000),y
        sta     L4DEC
        lda     L0000
        clc
        adc     L4DEB
        sta     L4DEB
        lda     $01
        adc     L4DEC
        sta     L4DEC
        lda     L4DEB
        sta     L0000
        lda     L4DEC
        sta     $01
        lda     #$00
        sta     $02
        lda     #$01
        sta     $03
        clx
        jsr     L48FC
        lda     #$F1
        sta     L4EEF
        lda     #$4E
        sta     L4EF0
        rts

        lda     #$40
        sta     $27D9
        sta     L4DDA
        dec     $5A
        st0     #$13
        lda     $27DA
        sta     a:$02
        lda     $27DB
        sta     a:$03
        st0     #$05
        smb6    $F3
        lda     $F3
        sta     a:$02
        stz     $5A
        rts

        dec     $5A
        stz     L4DDA
        jsr     L5111
        jsr     L50F6
        stz     $27D9
        stz     $5A
        stz     L4DEF
        tii     $4DEF,$4DF0,$00FF
        rts

        dec     $5A
        jsr     L4DA2
        lda     $10
        asl     a
        clc
        adc     L4DE5
        sta     $02
        cla
        adc     L4DE6
        sta     $03
        ldy     #$01
        lda     ($02)
        sta     $0E
        lda     ($02),y
        sta     $0F
        lda     L4DE3
        clc
        adc     $0E
        sta     $0E
        lda     L4DE4
        adc     $0F
        sta     $0F
        ldy     #$0C
        lda     $0E
        .byte   $91
L8D30:  brk
        iny
        lda     $0F
        sta     (L0000),y
        lda     (L0000)
        ora     #$02
        sta     (L0000)
        stz     $5A
        rts

        .byte   $A5
L8D40:  asl     $1085
        .byte   $C6
L8D44:  phy
        bsr     L8DA2
        lda     (L0000)
        bbr7    $10,L8D4E
L8D4C:  and     #$7F
L8D4E:  and     #$FD
        sta     (L0000)
        stz     $5A
        rts

        dec     $5A
        bsr     L8DA2
        lda     $10
        asl     a
        clc
        adc     L4DE7
        sta     $02
        cla
        adc     L4DE8
        sta     $03
        ldy     #$01
        lda     ($02)
        sta     $0E
        lda     ($02),y
        sta     $0F
        lda     L4DE3
        clc
        adc     $0E
        sta     $0E
        lda     L4DE4
        adc     $0F
        sta     $0F
        ldy     #$14
        lda     $0E
        sta     (L0000),y
        iny
        lda     $0F
        sta     (L0000),y
        lda     (L0000)
        ora     #$01
        sta     (L0000)
        stz     $5A
        rts

        dec     $5A
        bsr     L8DA2
        lda     (L0000)
        and     #$FE
        sta     (L0000)
        stz     $5A
        rts

L8DA2:  lda     #$EF
        sta     L0000
        lda     #$4D
        sta     $01
        lda     $0E
        asl     a
        asl     a
        asl     a
        asl     a
        asl     a
        clc
        adc     L0000
        sta     L0000
        bcc     L8DBA
        inc     $01
L8DBA:  rts

        bsr     L8DCF
        lda     #$01
        sta     (L0000),y
        rts

        bsr     L8DCF
        cla
        sta     (L0000),y
        rts

        bsr     L8DCF
L8DCA:  lda     (L0000),y
        bne     L8DCA
        rts

L8DCF:  lda     #$DB
        sta     L0000
        lda     #$4D
        sta     $01
        ldy     $0E
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        lda     $27D9
        beq     L9110
        jsr     L4932
        st0     #$00
        lda     $27DA
        sta     a:$02
        lda     $27DB
        sta     a:$03
        st0     #$02
        tia     $4EF1,$02,$0200
L9110:  rts

        stz     L4EF1
        tii     $4EF1,$4EF2,$01FF
        lda     L4DDA
        beq     L9110
        lda     #$F1
        sta     L4EEF
        lda     #$4E
        sta     L4EF0
        lda     #$EF
        sta     $48
        lda     #$4D
        sta     $49
        ldx     #$08
        cly
L9135:  phy
        lda     L4DE9
        sta     $42
        lda     L4DEA
        sta     $43
        tya
        asl     a
        tay
        lda     ($42),y
        sta     L4DED
        iny
        lda     ($42),y
        sta     L4DEE
        lda     L4DE3
        clc
        adc     L4DED
        sta     L4DED
        lda     L4DE4
        adc     L4DEE
        sta     L4DEE
        bsr     L919F
        jsr     L533D
        bsr     L917A
        lda     #$20
        clc
        adc     $48
        sta     $48
        bcc     L9173
        inc     $49
L9173:  ply
        iny
        cpy     #$08
        bne     L9135
        rts

L917A:  lda     ($48)
        and     #$80
        beq     L919E
L9180:  ldy     #$01
        lda     ($48),y
        iny
        sta     L003A
        lda     ($48),y
        iny
        sta     $3B
        lda     ($48),y
        iny
        sta     $3C
        lda     ($48),y
        iny
        sta     $3D
        lda     ($48),y
        iny
        sta     L0038
        jsr     L555E
L919E:  rts

L919F:  lda     ($48)
        and     #$02
        beq     L91E7
        ldy     #$0C
        lda     ($48),y
        sta     $4A
        iny
        lda     ($48),y
        sta     $4B
        ldy     #$08
        lda     ($48),y
        beq     L91BB
        dec     a
        sta     ($48),y
        bne     L91E7
L91BB:  stz     $3E
        stz     $3F
        ldy     $3E
        lda     ($4A),y
        inc     $3E
        asl     a
        tay
        lda     #$E8
        sta     L0038
        lda     #$51
        sta     $39
        iny
        lda     (L0038),y
        pha
        dey
        lda     (L0038),y
        pha
        rts

        ldy     #$0C
        lda     $4A
        clc
        adc     $3E
        sta     ($48),y
        iny
        cla
        adc     $4B
        sta     ($48),y
L91E7:  rts

        ora     $52
        st1     #$52
        eor     $52
        adc     ($52),y
        bbr7    $52,L9180
        eor     ($B4)
        eor     ($D8)
        eor     ($F1)
        eor     (L000B)
        tam     #$15
        tam     #$1E
        tam     #$21
        tam     #$2A
        tam     #$33
        tam     #$A4
        rol     L4AB1,x
        sta     L0038
        jsr     L55F4
        inc     $3E
        jmp     L51BF

        ldy     #$0E
        lda     ($48),y
        sta     $42
        iny
        lda     ($48),y
        sta     $43
        lda     ($42)
        jsr     L55F4
        ldy     #$01
        lda     ($42),y
        ldy     #$08
        dec     a
        sta     ($48),y
        lda     #$02
        clc
        adc     $42
        sta     $42
        bcc     L9238
        inc     $43
L9238:  ldy     #$0E
        lda     $42
        sta     ($48),y
        iny
        lda     $43
        sta     ($48),y
        jmp     L51D8

        ldy     $3E
        lda     ($4A),y
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        lda     L4DE3
        clc
        adc     L0038
        sta     L0038
        lda     L4DE4
        adc     $39
        sta     $39
        ldy     #$0E
        lda     L0038
        sta     ($48),y
        iny
        lda     $39
        sta     ($48),y
        inc     $3E
        inc     $3E
        jmp     L51BF

        ldy     $3E
        lda     ($4A),y
        inc     $3E
        ldy     #$08
        dec     a
        sta     ($48),y
        jmp     L51D8

        jsr     L55FF
        lda     ($42)
        bne     L928B
        dec     $3E
        dec     $3E
L928B:  jmp     L51D8

        ldy     $3E
L9290:  lda     ($4A),y
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        stz     $3E
        lda     L4DE3
        clc
        adc     L0038
        sta     L0038
        lda     L4DE4
        adc     $39
        sta     $39
        lda     L0038
        sta     $4A
        lda     $39
        sta     $4B
        jmp     L51BF

        ldy     $3E
        lda     ($4A),y
        ldy     #$09
        sta     ($48),y
        lda     $3E
        inc     a
        clc
        adc     $4A
        sta     $4A
        bcc     L92C9
        inc     $4B
L92C9:  stz     $3E
        ldy     #$0A
        lda     $4A
        sta     ($48),y
        iny
        lda     $4B
        sta     ($48),y
        jmp     L51BF

        ldy     #$09
        lda     ($48),y
        dec     a
        sta     ($48),y
        beq     L92EF
        ldy     #$0A
        lda     ($48),y
        iny
        sta     $4A
        lda     ($48),y
        sta     $4B
        stz     $3E
L92EF:  jmp     L51BF

        ldy     $3E
        lda     ($4A),y
        tax
        lda     $2780,x
        iny
        cmp     ($4A),y
        bne     L9302
        iny
        bra     L9290
L9302:  clc
        lda     $3E
        adc     #$04
        sta     $3E
        jmp     L51BF

        jsr     L55FF
        lda     #$01
        sta     ($42)
        jmp     L51D8

        jsr     L55FF
        cla
        sta     ($42)
        jmp     L51D8

        jmp     L51D8

        lda     ($48)
        and     #$FE
        sta     ($48)
        jmp     L51D8

        lda     ($48)
        and     #$FD
        sta     ($48)
        jmp     L51D8

        lda     ($48)
        and     #$7F
        sta     ($48)
        jmp     L51D8

        lda     ($48)
        and     #$01
        beq     L9385
        ldy     #$14
        lda     ($48),y
        sta     $4A
        iny
        lda     ($48),y
        sta     $4B
        ldy     #$10
        lda     ($48),y
        beq     L9359
        dec     a
        sta     ($48),y
        bne     L9385
L9359:  stz     $3E
        stz     $3F
        ldy     $3E
        lda     ($4A),y
        inc     $3E
        asl     a
        tay
        lda     #$86
        sta     L0038
        lda     #$53
        sta     $39
        iny
        lda     (L0038),y
        pha
        dey
        lda     (L0038),y
        pha
        rts

        ldy     #$14
        lda     $4A
        clc
        adc     $3E
        sta     ($48),y
        iny
        cla
        adc     $4B
        sta     ($48),y
L9385:  rts

        smb2    $53
        cpy     $2453
        csl
        rmb2    $54
        adc     L9954
        csl
        smb2    $54
        lda     $54,x
        .byte   $DC
        csl
        brk
        eor     $19,x
        eor     $35,x
        eor     $3F,x
        eor     $48,x
        eor     $4B,x
        eor     $54,x
        eor     $F5,x
        tam     #$A4
        rol     L4AB1,x
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        phy
        jsr     L5617
        ply
        iny
        lda     ($4A),y
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        phy
        jsr     L562A
        ply
        iny
        sty     $3E
        jmp     L535D

        ldy     $3E
        lda     ($4A),y
        tax
        iny
        lda     ($4A),y
        phy
        ldy     #$02
        sta     ($48),y
        dey
        txa
        sta     ($48),y
        ply
        iny
        lda     ($4A),y
        tax
        iny
        lda     ($4A),y
        phy
        ldy     #$04
        sta     ($48),y
        dey
        txa
        sta     ($48),y
        ply
        iny
        sty     $3E
        jmp     L535D

        ldy     $3E
        lda     ($4A),y
        tax
        phy
        lda     $2780,x
        ldy     #$01
        sta     ($48),y
        iny
        inx
        lda     $2780,x
        sta     ($48),y
        ply
        iny
        lda     ($4A),y
        tax
        phy
        lda     $2780,x
        ldy     #$03
        sta     ($48),y
        iny
        inx
        lda     $2780,x
        sta     ($48),y
        ply
        iny
        sty     $3E
        jmp     L535D

        jmp     L5376

        ldy     #$16
        lda     ($48),y
        sta     $42
        iny
        lda     ($48),y
        sta     $43
        cly
        lda     ($42)
        iny
        sta     L0038
        lda     ($42),y
        iny
        sta     $39
        jsr     L5617
        lda     ($42),y
        iny
        sta     L0038
        lda     ($42),y
        iny
        sta     $39
        jsr     L562A
        lda     ($42),y
        ldy     #$10
        dec     a
        sta     ($48),y
        lda     #$05
        clc
        adc     $42
        sta     $42
        bcc     L9460
        inc     $43
L9460:  ldy     #$16
        lda     $42
        sta     ($48),y
        iny
        lda     $43
        sta     ($48),y
        jmp     L5376

        ldy     $3E
        lda     ($4A),y
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        lda     L4DE3
        clc
        adc     L0038
        sta     L0038
        lda     L4DE4
        adc     $39
        sta     $39
        ldy     #$16
        lda     L0038
        sta     ($48),y
        iny
        lda     $39
        sta     ($48),y
        inc     $3E
        inc     $3E
        jmp     L535D

        ldy     $3E
        lda     ($4A),y
        ldy     #$10
        dec     a
        sta     ($48),y
        inc     $3E
        jmp     L5376

        jsr     L55FF
        lda     ($42)
        bne     L94B3
        dec     $3E
        dec     $3E
L94B3:  jmp     L5376

        ldy     $3E
L94B8:  lda     ($4A),y
        sta     L0038
        iny
        lda     ($4A),y
        sta     $39
        stz     $3E
        lda     L4DE3
        clc
        adc     L0038
        sta     L0038
        lda     L4DE4
        adc     $39
        sta     $39
        lda     L0038
        sta     $4A
        lda     $39
        sta     $4B
        jmp     L535D

        ldy     $3E
        lda     ($4A),y
        ldy     #$11
        sta     ($48),y
        lda     $3E
        inc     a
        clc
        adc     $4A
        sta     $4A
        bcc     L94F1
        inc     $4B
L94F1:  stz     $3E
        ldy     #$12
        lda     $4A
        sta     ($48),y
        iny
        lda     $4B
        sta     ($48),y
        jmp     L535D

        ldy     #$11
        lda     ($48),y
        dec     a
        sta     ($48),y
        beq     L9517
        ldy     #$12
        lda     ($48),y
        iny
        sta     $4A
        lda     ($48),y
        sta     $4B
        stz     $3E
L9517:  jmp     L535D

        ldy     $3E
        lda     ($4A),y
        phx
        tax
        lda     $2780,x
        plx
        iny
        cmp     ($4A),y
        bne     L952C
        iny
        bra     L94B8
L952C:  clc
        lda     $3E
        adc     #$04
        sta     $3E
        jmp     L535D

        jsr     L55FF
        lda     #$01
        sta     ($42)
        jmp     L5376

        jsr     L55FF
        cla
        sta     ($42)
        jmp     L5376

        jmp     L5376

        lda     ($48)
        and     #$FD
        sta     ($48)
        jmp     L5376

        lda     ($48)
        and     #$FE
        sta     ($48)
        jmp     L5376

        lda     L4EEF
        sta     L0044
        lda     L4EF0
        sta     $45
        lda     L4DED
        sta     $46
        lda     L4DEE
        sta     $47
        lda     L0038
        asl     a
        clc
        adc     $46
        sta     $46
        bcc     L957E
        inc     $47
L957E:  cly
        lda     ($46),y
        sta     $42
        iny
        lda     ($46),y
        sta     $43
        lda     L4DED
        clc
        adc     $42
        sta     $42
        lda     L4DEE
        adc     $43
        sta     $43
L9597:  ldy     #$06
        lda     ($42),y
        cmp     #$FF
        beq     L95E9
        cly
        lda     ($42),y
        clc
        adc     $3C
        sta     (L0044),y
        iny
        lda     ($42),y
        adc     $3D
        sta     (L0044),y
        iny
        lda     ($42),y
        clc
        adc     L003A
        sta     (L0044),y
        iny
        lda     ($42),y
        adc     $3B
        sta     (L0044),y
        iny
        lda     ($42),y
        sta     (L0044),y
        iny
        lda     ($42),y
        sta     (L0044),y
        iny
        lda     ($42),y
        sta     (L0044),y
        iny
        lda     ($42),y
        sta     (L0044),y
        lda     #$08
        clc
        adc     $42
        sta     $42
        bcc     L95DC
        inc     $43
L95DC:  lda     #$08
        clc
        adc     L0044
        sta     L0044
        bcc     L95E7
        inc     $45
L95E7:  bra     L9597
L95E9:  lda     L0044
        sta     L4EEF
        lda     $45
        sta     L4EF0
        rts

        ldy     #$05
        sta     ($48),y
        lda     ($48)
        ora     #$80
        sta     ($48)
        rts

        lda     #$DB
        sta     $42
        lda     #$4D
        sta     $43
        ldy     $3E
        lda     ($4A),y
        inc     $3E
        clc
        adc     $42
        sta     $42
        bcc     L9616
        inc     $43
L9616:  rts

        phy
        ldy     #$01
        lda     ($48),y
        clc
        adc     L0038
        sta     ($48),y
        iny
        lda     ($48),y
        adc     $39
        sta     ($48),y
        ply
        rts

        phy
        ldy     #$03
        lda     ($48),y
        clc
        adc     L0038
        sta     ($48),y
        iny
        lda     ($48),y
        adc     $39
        sta     ($48),y
        ply
        rts

        lda     $27C7
        bne     L9643
        rts

L9643:  lda     $27C6
        bne     L9665
        lda     ($62)
        asl     a
        tax
        lda     #$56
        pha
        lda     #$5B
        pha
        cly
        jmp     (L5656,x)

        lda     ($56,x)
        sta     $56
        ror     $1856
        adc     $62
        sta     $62
        bcc     L9665
        inc     $63
L9665:  lda     $27C6
        beq     L966D
        dec     $27C6
L966D:  rts

        iny
        lda     ($62),y
        sta     $27C4
        iny
        lda     ($62),y
        sta     $27C5
        iny
        lda     ($62),y
        sta     $27C6
        bsr     L96A5
        lda     #$04
        rts

        iny
        lda     ($62),y
        tax
        iny
        lda     ($62),y
        stx     $62
        sta     $63
        clc
        lda     $62
        adc     $27C0
        sta     $62
        lda     $63
        adc     $27C1
        sta     $63
        cla
        rts

        stz     $27C7
        rts

L96A5:  lda     $27C2
        sta     $0402
        lda     $27C3
        sta     $0403
        bsr     L96CA
        lda     $27C4
        sta     L56C3
        lda     $27C5
        sta     L56C4
        bsr     L96C2
        rts

L96C2:  tia     $00,$0404,$0020
        rts

L96CA:  clc
        lda     $27C4
        adc     $27C0
        sta     $27C4
        lda     $27C5
        adc     $27C1
        sta     $27C5
        rts

        lda     L58DB
        beq     L9709
        tax
        and     #$80
        beq     L96EB
        jmp     L571A

L96EB:  txa
        and     #$40
        beq     L9709
        lda     #$00
        sta     $0402
        lda     #$00
        sta     $0403
        tia     $58E0,$0404,$0400
        lda     L58DB
        and     #$BF
        sta     L58DB
L9709:  rts

        lda     L58DB
        beq     L9719
        dec     a
        dec     a
        beq     L9756
        dec     a
        bne     L9719
        jmp     L57C2

L9719:  rts

        jsr     L58C5
        lda     #$09
        sta     L58DC
        lda     #$10
        sta     L58DD
        lda     #$E0
        sta     L58DE
        lda     #$58
        sta     L58DF
        rts

        lda     #$08
        sta     L58DD
        lda     #$09
L9739:  sta     L5798
        sta     L571E
        stz     $27C7
        lda     #$82
        sta     L58DB
L9747:  lda     L58DD
        bne     L9747
        rts

        lda     #$08
        sta     L58DD
        lda     #$05
        bra     L9739
L9756:  stz     $27C7
        lda     L58DC
        dec     a
        sta     L58DC
        beq     L9785
        lda     L58DE
        sta     $42
        lda     L58DF
        sta     $43
        jsr     L5823
        lda     #$80
        clc
        adc     $42
        sta     $42
        bcc     L977A
        inc     $43
L977A:  lda     $42
        sta     L58DE
        lda     $43
        sta     L58DF
        rts

L9785:  lda     #$E0
        sta     L58DE
        lda     #$58
        sta     L58DF
        lda     L58DB
        ora     #$40
        sta     L58DB
        lda     #$09
        sta     L58DC
        dec     L58DD
        bne     L97A4
        stz     L58DB
L97A4:  rts

        dec     $5A
        jsr     L48A6
        stz     $5A
        stz     $27C7
        lda     #$08
        sta     L58DD
        lda     #$83
        sta     L58DB
        dec     $5A
        jsr     L48A6
        stz     $5A
        bra     L9747
        stz     $27C7
        lda     L58DC
        dec     a
        sta     L58DC
        beq     L9785
        lda     L58DE
        sta     $42
        lda     L58DF
        sta     $43
        jsr     L586E
        lda     #$80
        clc
        adc     $42
        sta     $42
        bcc     L97E6
        inc     $43
L97E6:  lda     $42
        sta     L58DE
        lda     $43
        sta     L58DF
        rts

        bra     L9785
        dec     $5A
        lda     L47D1
        sta     L5802
        lda     L47D2
        sta     L5803
        tii     $47D1,$58E0,$0200
        lda     L4DEB
        sta     L5815
        lda     L4DEC
        sta     L5816
        tii     $4DEB,$5AE0,$0200
        lda     #$40
        sta     L58DB
        stz     $5A
        rts

        ldx     #$40
        cly
L9826:  phx
        phy
        stz     L0038
        stz     $39
        lda     ($42),y
        and     #$07
        beq     L9835
        dec     a
        sta     L0038
L9835:  lda     ($42),y
        and     #$38
        beq     L9842
        sec
        sbc     #$08
        ora     L0038
        sta     L0038
L9842:  lda     ($42),y
        sta     $39
        iny
        lda     ($42),y
        lsr     a
        ror     $39
        lda     $39
        stz     $39
        and     #$E0
        beq     L985E
        sec
        sbc     #$20
        asl     a
        rol     $39
        ora     L0038
        sta     L0038
L985E:  ply
        lda     L0038
        sta     ($42),y
        iny
        lda     $39
        sta     ($42),y
        iny
        plx
        dex
        bne     L9826
        rts

        ldx     #$40
        cly
L9871:  phx
        phy
        lda     #$FF
        sta     L0038
        sta     $39
        lda     ($42),y
        inc     a
        and     #$07
        beq     L9882
        sta     L0038
L9882:  lda     ($42),y
        clc
        adc     #$08
        and     #$38
        beq     L988F
        ora     L0038
        sta     L0038
L988F:  lda     ($42),y
        sta     $39
        iny
        lda     ($42),y
        lsr     a
        ror     $39
        lda     $39
        clc
        adc     #$20
        and     #$E0
        beq     L98AB
        asl     a
        rol     $39
        ora     L0038
        sta     L0038
        bra     L98B5
L98AB:  lda     #$01
        sta     $39
        lda     L0038
        ora     #$C0
        sta     L0038
L98B5:  ply
        lda     L0038
        sta     ($42),y
        iny
        lda     $39
        sta     ($42),y
        iny
        plx
        dex
        bne     L9871
        rts

        stz     $0402
        stz     $0403
        tai     $0404,$58E0,$0400
        lda     L58DB
        and     #$7F
        sta     L58DB
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L9954:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L9A44:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L9A4C:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        bit     $3B69
        bvc     L9D1C
        lda     #$E0
        sta     $5C
        lda     #$58
        sta     $5D
        stz     L58E0
        tii     $58E0,$58E1,$03FF
        ldx     #$07
L9CFD:  stz     L5E16,x
        stz     L5E1E,x
        dex
        bpl     L9CFD
        lda     $3B69
        and     #$BF
        sta     $3B69
        lda     #$01
        sta     L5E27
        lda     #$08
        sta     L5E28
        stz     L5E26
        rts

L9D1C:  lda     L5E26
        bne     L9D4F
        dec     L5E27
        bne     L9D4E
        lda     #$01
        sta     L5E27
        lda     #$E0
        sta     $5C
        lda     #$58
        sta     $5D
        lda     $3B6A
        sta     $5E
        lda     $3B6B
        sta     $5F
        stz     L5E29
        lda     $3B6F
        .byte   $8D
L9D44:  rol     a
        lsr     $DB20,x
        eor     L08A9,x
        .byte   $8D
L9D4C:  rol     $5E
L9D4E:  rts

L9D4F:  lda     #$04
L9D51:  pha
        lda     L5E29
        cmp     $3B6E
        bcc     L9D68
        lda     L5E2A
        beq     L9D68
        dec     L5E2A
        jsr     L5D93
        jsr     L5DF5
L9D68:  inc     L5E29
        lda     $5C
        clc
        adc     #$20
        sta     $5C
        bcc     L9D76
        inc     $5D
L9D76:  lda     $5E
        clc
        adc     #$20
        sta     $5E
        bcc     L9D81
        inc     $5F
L9D81:  pla
        dec     a
        bne     L9D51
        dec     L5E26
        bne     L9D92
        dec     L5E28
        bne     L9D92
        stz     $3B69
L9D92:  rts

        cly
        ldx     #$10
L9D96:  phx
        stz     $60
        iny
        lda     ($5E),y
        lsr     a
        dey
        lda     ($5E),y
        pha
        rol     a
        rol     a
        rol     a
        and     #$07
        tax
        lda     L5E1E,x
        beq     L9DAE
        smb6    $60
L9DAE:  pla
        pha
        lsr     a
        lsr     a
        lsr     a
        and     #$07
        tax
        lda     L5E1E,x
        beq     L9DBD
        smb3    $60
L9DBD:  pla
        and     #$07
        tax
        lda     L5E1E,x
        beq     L9DC8
        smb0    $60
L9DC8:  lda     ($5C),y
        clc
        adc     $60
        sta     ($5C),y
        cla
        iny
        adc     ($5C),y
        sta     ($5C),y
        iny
        plx
        dex
        bne     L9D96
        rts

        ldx     #$07
L9DDD:  stz     L5E1E,x
        txa
        clc
        adc     L5E16,x
        bit     #$08
        beq     L9DEE
        and     #$07
        inc     L5E1E,x
L9DEE:  sta     L5E16,x
        dex
        bne     L9DDD
        rts

        lda     $5C
        sta     L5E0F
        lda     $5D
        sta     L5E10
        lda     L5E29
        asl     a
        asl     a
        asl     a
        asl     a
        sta     $0402
        cla
        rol     a
        sta     $0403
        tia     $00,$0404,$0020
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        ora     (L0000,x)
        brk
        jsr     L78AD
        .byte   $3B
        bne     L9E31
        rts

L9E31:  lda     $3B79
        asl     a
        tax
        jmp     (L5E81,x)

        st0     #$06
        lda     $3B76
        bne     L9E41
        inc     a
L9E41:  clc
        adc     #$3F
        sta     a:$02
        lda     $3B77
        adc     #$00
        sta     a:$03
        st0     #$07
        lda     $220C
        sta     a:$02
        lda     $220D
        sta     a:$03
        inc     $3B79
        rts

        st0     #$06
        lda     $3B74
        bne     L9E69
        inc     a
L9E69:  clc
        adc     #$3F
        sta     a:$02
        lda     $3B75
        adc     #$00
        sta     a:$03
        st0     #$07
        st1     #$00
        st2     #$00
        stz     $3B79
        rts

        and     L615E,y
        lsr     L5E80,x
        .byte   $FF
        .byte   $FF
L9E89:  bbs7    $FF,$9E8B
L9E8C:  bbs7    $FF,$9E8E
L9E8F:  bbs7    $FF,$9E91
L9E92:  bbs7    $FF,$9E94
L9E95:  bbs7    $FF,$9E97
L9E98:  bbs7    $FF,$9E9A
L9E9B:  bbs7    $FF,$9E9D
L9E9E:  bbs7    $FF,$9EA0
L9EA1:  bbs7    $FF,$9EA3
L9EA4:  bbs7    $FF,$9EA6
L9EA7:  bbs7    $FF,$9EA9
L9EAA:  bbs7    $FF,$9EAC
L9EAD:  bbs7    $FF,$9EAF
L9EB0:  bbs7    $FF,$9EB2
L9EB3:  bbs7    $FF,$9EB5
L9EB6:  bbs7    $FF,$9EB8
L9EB9:  bbs7    $FF,$9EBB
L9EBC:  bbs7    $FF,$9EBE
L9EBF:  bbs7    $FF,$9EC1
L9EC2:  bbs7    $FF,$9EC4
L9EC5:  bbs7    $FF,$9EC7
L9EC8:  bbs7    $FF,$9ECA
L9ECB:  bbs7    $FF,$9ECD
L9ECE:  bbs7    $FF,$9ED0
L9ED1:  bbs7    $FF,$9ED3
L9ED4:  bbs7    $FF,$9ED6
L9ED7:  bbs7    $FF,$9ED9
L9EDA:  bbs7    $FF,$9EDC
L9EDD:  bbs7    $FF,$9EDF
L9EE0:  bbs7    $FF,$9EE2
L9EE3:  bbs7    $FF,$9EE5
L9EE6:  bbs7    $FF,$9EE8
L9EE9:  bbs7    $FF,$9EEB
L9EEC:  bbs7    $FF,$9EEE
L9EEF:  bbs7    $FF,$9EF1
L9EF2:  bbs7    $FF,$9EF4
L9EF5:  bbs7    $FF,$9EF7
L9EF8:  bbs7    $FF,$9EFA
L9EFB:  bbs7    $FF,$9EFD
L9EFE:  bbs7    $FF,$9F00
L9F01:  bbs7    $FF,$9F03
L9F04:  bbs7    $FF,$9F06
L9F07:  bbs7    $FF,$9F09
L9F0A:  bbs7    $FF,$9F0C
L9F0D:  bbs7    $FF,$9F0F
L9F10:  bbs7    $FF,$9F12
L9F13:  bbs7    $FF,$9F15
L9F16:  bbs7    $FF,$9F18
L9F19:  bbs7    $FF,$9F1B
L9F1C:  bbs7    $FF,$9F1E
L9F1F:  bbs7    $FF,$9F21
L9F22:  bbs7    $FF,$9F24
L9F25:  bbs7    $FF,$9F27
L9F28:  bbs7    $FF,$9F2A
L9F2B:  bbs7    $FF,$9F2D
L9F2E:  bbs7    $FF,$9F30
L9F31:  bbs7    $FF,$9F33
L9F34:  bbs7    $FF,$9F36
L9F37:  bbs7    $FF,$9F39
L9F3A:  bbs7    $FF,$9F3C
L9F3D:  bbs7    $FF,$9F3F
L9F40:  bbs7    $FF,$9F42
L9F43:  bbs7    $FF,$9F45
L9F46:  bbs7    $FF,$9F48
L9F49:  bbs7    $FF,$9F4B
L9F4C:  bbs7    $FF,$9F4E
L9F4F:  bbs7    $FF,$9F51
L9F52:  bbs7    $FF,$9F54
L9F55:  bbs7    $FF,$9F57
L9F58:  bbs7    $FF,$9F5A
L9F5B:  bbs7    $FF,$9F5D
L9F5E:  bbs7    $FF,$9F60
L9F61:  bbs7    $FF,$9F63
L9F64:  bbs7    $FF,$9F66
L9F67:  bbs7    $FF,$9F69
L9F6A:  bbs7    $FF,$9F6C
L9F6D:  bbs7    $FF,$9F6F
L9F70:  bbs7    $FF,$9F72
L9F73:  bbs7    $FF,$9F75
L9F76:  bbs7    $FF,$9F78
L9F79:  bbs7    $FF,$9F7B
L9F7C:  bbs7    $FF,$9F7E
L9F7F:  bbs7    $FF,$9F81
L9F82:  bbs7    $FF,$9F84
L9F85:  bbs7    $FF,$9F87
L9F88:  bbs7    $FF,$9F8A
L9F8B:  bbs7    $FF,$9F8D
L9F8E:  bbs7    $FF,$9F90
L9F91:  bbs7    $FF,$9F93
L9F94:  bbs7    $FF,$9F96
L9F97:  bbs7    $FF,$9F99
L9F9A:  bbs7    $FF,$9F9C
L9F9D:  bbs7    $FF,$9F9F
L9FA0:  bbs7    $FF,$9FA2
L9FA3:  bbs7    $FF,$9FA5
L9FA6:  bbs7    $FF,$9FA8
L9FA9:  bbs7    $FF,$9FAB
L9FAC:  bbs7    $FF,$9FAE
L9FAF:  bbs7    $FF,$9FB1
L9FB2:  bbs7    $FF,$9FB4
L9FB5:  bbs7    $FF,$9FB7
L9FB8:  bbs7    $FF,$9FBA
L9FBB:  bbs7    $FF,$9FBD
L9FBE:  bbs7    $FF,$9FC0
L9FC1:  bbs7    $FF,$9FC3
L9FC4:  bbs7    $FF,$9FC6
L9FC7:  bbs7    $FF,$9FC9
L9FCA:  bbs7    $FF,$9FCC
L9FCD:  bbs7    $FF,$9FCF
L9FD0:  bbs7    $FF,$9FD2
L9FD3:  bbs7    $FF,$9FD5
L9FD6:  bbs7    $FF,$9FD8
L9FD9:  bbs7    $FF,$9FDB
L9FDC:  bbs7    $FF,$9FDE
L9FDF:  bbs7    $FF,$9FE1
L9FE2:  bbs7    $FF,$9FE4
L9FE5:  bbs7    $FF,$9FE7
L9FE8:  bbs7    $FF,$9FEA
L9FEB:  bbs7    $FF,$9FED
L9FEE:  bbs7    $FF,$9FF0
L9FF1:  bbs7    $FF,$9FF3
L9FF4:  bbs7    $FF,$9FF6
L9FF7:  bbs7    $FF,$9FF9
L9FFA:  bbs7    $FF,$9FFC
L9FFD:  bbs7    $FF,$9FFF
LA000:  bbs7    $FF,$A002
LA003:  bbs7    $FF,$A005
LA006:  bbs7    $FF,$A008
LA009:  bbs7    $FF,$A00B
LA00C:  bbs7    $FF,$A00E
LA00F:  bbs7    $FF,$A011
LA012:  bbs7    $FF,$A014
LA015:  bbs7    $FF,$A017
LA018:  bbs7    $FF,$A01A
LA01B:  bbs7    $FF,$A01D
LA01E:  bbs7    $FF,$A020
LA021:  bbs7    $FF,$A023
LA024:  bbs7    $FF,$A026
LA027:  bbs7    $FF,$A029
LA02A:  bbs7    $FF,$A02C
LA02D:  bbs7    $FF,$A02F
LA030:  bbs7    $FF,$A032
LA033:  bbs7    $FF,$A035
LA036:  bbs7    $FF,$A038
LA039:  bbs7    $FF,$A03B
LA03C:  bbs7    $FF,$A03E
LA03F:  bbs7    $FF,$A041
LA042:  bbs7    $FF,$A044
LA045:  bbs7    $FF,$A047
LA048:  bbs7    $FF,$A04A
LA04B:  bbs7    $FF,$A04D
LA04E:  bbs7    $FF,$A050
LA051:  bbs7    $FF,$A053
LA054:  bbs7    $FF,$A056
LA057:  bbs7    $FF,$A059
LA05A:  bbs7    $FF,$A05C
LA05D:  bbs7    $FF,$A05F
LA060:  bbs7    $FF,$A062
LA063:  bbs7    $FF,$A065
LA066:  bbs7    $FF,$A068
LA069:  bbs7    $FF,$A06B
LA06C:  bbs7    $FF,$A06E
LA06F:  bbs7    $FF,$A071
LA072:  bbs7    $FF,$A074
LA075:  bbs7    $FF,$A077
LA078:  bbs7    $FF,$A07A
LA07B:  bbs7    $FF,$A07D
LA07E:  bbs7    $FF,$A080
LA081:  bbs7    $FF,$A083
LA084:  bbs7    $FF,$A086
LA087:  bbs7    $FF,$A089
LA08A:  bbs7    $FF,$A08C
LA08D:  bbs7    $FF,$A08F
LA090:  bbs7    $FF,$A092
LA093:  bbs7    $FF,$A095
LA096:  bbs7    $FF,$A098
LA099:  bbs7    $FF,$A09B
LA09C:  bbs7    $FF,$A09E
LA09F:  bbs7    $FF,$A0A1
LA0A2:  bbs7    $FF,$A0A4
LA0A5:  bbs7    $FF,$A0A7
LA0A8:  bbs7    $FF,$A0AA
LA0AB:  bbs7    $FF,$A0AD
LA0AE:  bbs7    $FF,$A0B0
LA0B1:  bbs7    $FF,$A0B3
LA0B4:  bbs7    $FF,$A0B6
LA0B7:  bbs7    $FF,$A0B9
LA0BA:  bbs7    $FF,$A0BC
LA0BD:  bbs7    $FF,$A0BF
LA0C0:  bbs7    $FF,$A0C2
LA0C3:  bbs7    $FF,$A0C5
LA0C6:  bbs7    $FF,$A0C8
LA0C9:  bbs7    $FF,$A0CB
LA0CC:  bbs7    $FF,$A0CE
LA0CF:  bbs7    $FF,$A0D1
LA0D2:  bbs7    $FF,$A0D4
LA0D5:  bbs7    $FF,$A0D7
LA0D8:  bbs7    $FF,$A0DA
LA0DB:  bbs7    $FF,$A0DD
LA0DE:  bbs7    $FF,$A0E0
LA0E1:  bbs7    $FF,$A0E3
LA0E4:  bbs7    $FF,$A0E6
LA0E7:  bbs7    $FF,$A0E9
LA0EA:  bbs7    $FF,$A0EC
LA0ED:  bbs7    $FF,$A0EF
LA0F0:  bbs7    $FF,$A0F2
LA0F3:  bbs7    $FF,$A0F5
LA0F6:  bbs7    $FF,$A0F8
LA0F9:  bbs7    $FF,$A0FB
LA0FC:  bbs7    $FF,$A0FE
LA0FF:  bbs7    $FF,$A101
LA102:  bbs7    $FF,$A104
LA105:  bbs7    $FF,$A107
LA108:  bbs7    $FF,$A10A
LA10B:  bbs7    $FF,$A10D
LA10E:  bbs7    $FF,$A110
LA111:  bbs7    $FF,$A113
LA114:  bbs7    $FF,$A116
LA117:  bbs7    $FF,$A119
LA11A:  bbs7    $FF,$A11C
LA11D:  bbs7    $FF,$A11F
LA120:  bbs7    $FF,$A122
LA123:  bbs7    $FF,$A125
LA126:  bbs7    $FF,$A128
LA129:  bbs7    $FF,$A12B
LA12C:  bbs7    $FF,$A12E
LA12F:  bbs7    $FF,$A131
LA132:  bbs7    $FF,$A134
LA135:  bbs7    $FF,$A137
LA138:  bbs7    $FF,$A13A
LA13B:  bbs7    $FF,$A13D
LA13E:  bbs7    $FF,$A140
LA141:  bbs7    $FF,$A143
LA144:  bbs7    $FF,$A146
LA147:  bbs7    $FF,$A149
LA14A:  bbs7    $FF,$A14C
LA14D:  bbs7    $FF,$A14F
LA150:  bbs7    $FF,$A152
LA153:  bbs7    $FF,$A155
LA156:  bbs7    $FF,$A158
LA159:  bbs7    $FF,$A15B
LA15C:  bbs7    $FF,$A15E
LA15F:  bbs7    $FF,$A161
LA162:  bbs7    $FF,$A164
LA165:  bbs7    $FF,$A167
LA168:  bbs7    $FF,$A16A
LA16B:  bbs7    $FF,$A16D
LA16E:  bbs7    $FF,$A170
LA171:  bbs7    $FF,$A173
LA174:  bbs7    $FF,$A176
LA177:  bbs7    $FF,$A179
LA17A:  bbs7    $FF,$A17C
LA17D:  bbs7    $FF,$A17F
LA180:  bbs7    $FF,$A182
LA183:  bbs7    $FF,$A185
LA186:  bbs7    $FF,$A188
LA189:  bbs7    $FF,$A18B
LA18C:  bbs7    $FF,$A18E
LA18F:  bbs7    $FF,$A191
LA192:  bbs7    $FF,$A194
LA195:  bbs7    $FF,$A197
LA198:  bbs7    $FF,$A19A
LA19B:  bbs7    $FF,$A19D
LA19E:  bbs7    $FF,$A1A0
LA1A1:  bbs7    $FF,$A1A3
LA1A4:  bbs7    $FF,$A1A6
LA1A7:  bbs7    $FF,$A1A9
LA1AA:  bbs7    $FF,$A1AC
LA1AD:  bbs7    $FF,$A1AF
LA1B0:  bbs7    $FF,$A1B2
LA1B3:  bbs7    $FF,$A1B5
LA1B6:  bbs7    $FF,$A1B8
LA1B9:  bbs7    $FF,$A1BB
LA1BC:  bbs7    $FF,$A1BE
LA1BF:  bbs7    $FF,$A1C1
LA1C2:  bbs7    $FF,$A1C4
LA1C5:  bbs7    $FF,$A1C7
LA1C8:  bbs7    $FF,$A1CA
LA1CB:  bbs7    $FF,$A1CD
LA1CE:  bbs7    $FF,$A1D0
LA1D1:  bbs7    $FF,$A1D3
LA1D4:  bbs7    $FF,$A1D6
LA1D7:  bbs7    $FF,$A1D9
LA1DA:  bbs7    $FF,$A1DC
LA1DD:  bbs7    $FF,$A1DF
LA1E0:  bbs7    $FF,$A1E2
LA1E3:  bbs7    $FF,$A1E5
LA1E6:  bbs7    $FF,$A1E8
LA1E9:  bbs7    $FF,$A1EB
LA1EC:  bbs7    $FF,$A1EE
LA1EF:  bbs7    $FF,$A1F1
LA1F2:  bbs7    $FF,$A1F4
LA1F5:  bbs7    $FF,$A1F7
LA1F8:  bbs7    $FF,$A1FA
LA1FB:  bbs7    $FF,$A1FD
LA1FE:  bbs7    $FF,$A200
LA201:  bbs7    $FF,$A203
LA204:  bbs7    $FF,$A206
LA207:  bbs7    $FF,$A209
LA20A:  bbs7    $FF,$A20C
LA20D:  bbs7    $FF,$A20F
LA210:  bbs7    $FF,$A212
LA213:  bbs7    $FF,$A215
LA216:  bbs7    $FF,$A218
LA219:  bbs7    $FF,$A21B
LA21C:  bbs7    $FF,$A21E
LA21F:  bbs7    $FF,$A221
LA222:  bbs7    $FF,$A224
LA225:  bbs7    $FF,$A227
LA228:  bbs7    $FF,$A22A
LA22B:  bbs7    $FF,$A22D
LA22E:  bbs7    $FF,$A230
LA231:  bbs7    $FF,$A233
LA234:  bbs7    $FF,$A236
LA237:  bbs7    $FF,$A239
LA23A:  bbs7    $FF,$A23C
LA23D:  bbs7    $FF,$A23F
LA240:  bbs7    $FF,$A242
LA243:  bbs7    $FF,$A245
LA246:  bbs7    $FF,$A248
LA249:  bbs7    $FF,$A24B
LA24C:  bbs7    $FF,$A24E
LA24F:  bbs7    $FF,$A251
LA252:  bbs7    $FF,$A254
LA255:  bbs7    $FF,$A257
LA258:  bbs7    $FF,$A25A
LA25B:  bbs7    $FF,$A25D
LA25E:  bbs7    $FF,$A260
LA261:  bbs7    $FF,$A263
LA264:  bbs7    $FF,$A266
LA267:  bbs7    $FF,$A269
LA26A:  bbs7    $FF,$A26C
LA26D:  bbs7    $FF,$A26F
LA270:  bbs7    $FF,$A272
LA273:  bbs7    $FF,$A275
LA276:  bbs7    $FF,$A278
LA279:  bbs7    $FF,$A27B
LA27C:  bbs7    $FF,$A27E
LA27F:  bbs7    $FF,$A281
LA282:  bbs7    $FF,$A284
LA285:  bbs7    $FF,$A287
LA288:  bbs7    $FF,$A28A
LA28B:  bbs7    $FF,$A28D
LA28E:  bbs7    $FF,$A290
LA291:  bbs7    $FF,$A293
LA294:  bbs7    $FF,$A296
LA297:  bbs7    $FF,$A299
LA29A:  bbs7    $FF,$A29C
LA29D:  bbs7    $FF,$A29F
LA2A0:  bbs7    $FF,$A2A2
LA2A3:  bbs7    $FF,$A2A5
LA2A6:  bbs7    $FF,$A2A8
LA2A9:  bbs7    $FF,$A2AB
LA2AC:  bbs7    $FF,$A2AE
LA2AF:  bbs7    $FF,$A2B1
LA2B2:  bbs7    $FF,$A2B4
LA2B5:  bbs7    $FF,$A2B7
LA2B8:  bbs7    $FF,$A2BA
LA2BB:  bbs7    $FF,$A2BD
LA2BE:  bbs7    $FF,$A2C0
LA2C1:  bbs7    $FF,$A2C3
LA2C4:  bbs7    $FF,$A2C6
LA2C7:  bbs7    $FF,$A2C9
LA2CA:  bbs7    $FF,$A2CC
LA2CD:  bbs7    $FF,$A2CF
LA2D0:  bbs7    $FF,$A2D2
LA2D3:  bbs7    $FF,$A2D5
LA2D6:  bbs7    $FF,$A2D8
LA2D9:  bbs7    $FF,$A2DB
LA2DC:  bbs7    $FF,$A2DE
LA2DF:  bbs7    $FF,$A2E1
LA2E2:  bbs7    $FF,$A2E4
LA2E5:  bbs7    $FF,$A2E7
LA2E8:  bbs7    $FF,$A2EA
LA2EB:  bbs7    $FF,$A2ED
LA2EE:  bbs7    $FF,$A2F0
LA2F1:  bbs7    $FF,$A2F3
LA2F4:  bbs7    $FF,$A2F6
LA2F7:  bbs7    $FF,$A2F9
LA2FA:  bbs7    $FF,$A2FC
LA2FD:  bbs7    $FF,$A2FF
LA300:  bbs7    $FF,$A302
LA303:  bbs7    $FF,$A305
LA306:  bbs7    $FF,$A308
LA309:  bbs7    $FF,$A30B
LA30C:  bbs7    $FF,$A30E
LA30F:  bbs7    $FF,$A311
LA312:  bbs7    $FF,$A314
LA315:  bbs7    $FF,$A317
LA318:  bbs7    $FF,$A31A
LA31B:  bbs7    $FF,$A31D
LA31E:  bbs7    $FF,$A320
LA321:  bbs7    $FF,$A323
LA324:  bbs7    $FF,$A326
LA327:  bbs7    $FF,$A329
LA32A:  bbs7    $FF,$A32C
LA32D:  bbs7    $FF,$A32F
LA330:  bbs7    $FF,$A332
LA333:  bbs7    $FF,$A335
LA336:  bbs7    $FF,$A338
LA339:  bbs7    $FF,$A33B
LA33C:  bbs7    $FF,$A33E
LA33F:  bbs7    $FF,$A341
LA342:  bbs7    $FF,$A344
LA345:  bbs7    $FF,$A347
LA348:  bbs7    $FF,$A34A
LA34B:  bbs7    $FF,$A34D
LA34E:  bbs7    $FF,$A350
LA351:  bbs7    $FF,$A353
LA354:  bbs7    $FF,$A356
LA357:  bbs7    $FF,$A359
LA35A:  bbs7    $FF,$A35C
LA35D:  bbs7    $FF,$A35F
LA360:  bbs7    $FF,$A362
LA363:  bbs7    $FF,$A365
LA366:  bbs7    $FF,$A368
LA369:  bbs7    $FF,$A36B
LA36C:  bbs7    $FF,$A36E
LA36F:  bbs7    $FF,$A371
LA372:  bbs7    $FF,$A374
LA375:  bbs7    $FF,$A377
LA378:  bbs7    $FF,$A37A
LA37B:  bbs7    $FF,$A37D
LA37E:  bbs7    $FF,$A380
LA381:  bbs7    $FF,$A383
LA384:  bbs7    $FF,$A386
LA387:  bbs7    $FF,$A389
LA38A:  bbs7    $FF,$A38C
LA38D:  bbs7    $FF,$A38F
LA390:  bbs7    $FF,$A392
LA393:  bbs7    $FF,$A395
LA396:  bbs7    $FF,$A398
LA399:  bbs7    $FF,$A39B
LA39C:  bbs7    $FF,$A39E
LA39F:  bbs7    $FF,$A3A1
LA3A2:  bbs7    $FF,$A3A4
LA3A5:  bbs7    $FF,$A3A7
LA3A8:  bbs7    $FF,$A3AA
LA3AB:  bbs7    $FF,$A3AD
LA3AE:  bbs7    $FF,$A3B0
LA3B1:  bbs7    $FF,$A3B3
LA3B4:  bbs7    $FF,$A3B6
LA3B7:  bbs7    $FF,$A3B9
LA3BA:  bbs7    $FF,$A3BC
LA3BD:  bbs7    $FF,$A3BF
LA3C0:  bbs7    $FF,$A3C2
LA3C3:  bbs7    $FF,$A3C5
LA3C6:  bbs7    $FF,$A3C8
LA3C9:  bbs7    $FF,$A3CB
LA3CC:  bbs7    $FF,$A3CE
LA3CF:  bbs7    $FF,$A3D1
LA3D2:  bbs7    $FF,$A3D4
LA3D5:  bbs7    $FF,$A3D7
LA3D8:  bbs7    $FF,$A3DA
LA3DB:  bbs7    $FF,$A3DD
LA3DE:  bbs7    $FF,$A3E0
LA3E1:  bbs7    $FF,$A3E3
LA3E4:  bbs7    $FF,$A3E6
LA3E7:  bbs7    $FF,$A3E9
LA3EA:  bbs7    $FF,$A3EC
LA3ED:  bbs7    $FF,$A3EF
LA3F0:  bbs7    $FF,$A3F2
LA3F3:  bbs7    $FF,$A3F5
LA3F6:  bbs7    $FF,$A3F8
LA3F9:  bbs7    $FF,$A3FB
LA3FC:  bbs7    $FF,$A3FE
LA3FF:  bbs7    $FF,$A401
LA402:  bbs7    $FF,$A404
LA405:  bbs7    $FF,$A407
LA408:  bbs7    $FF,$A40A
LA40B:  bbs7    $FF,$A40D
LA40E:  bbs7    $FF,$A410
LA411:  bbs7    $FF,$A413
LA414:  bbs7    $FF,$A416
LA417:  bbs7    $FF,$A419
LA41A:  bbs7    $FF,$A41C
LA41D:  bbs7    $FF,$A41F
LA420:  bbs7    $FF,$A422
LA423:  bbs7    $FF,$A425
LA426:  bbs7    $FF,$A428
LA429:  bbs7    $FF,$A42B
LA42C:  bbs7    $FF,$A42E
LA42F:  bbs7    $FF,$A431
LA432:  bbs7    $FF,$A434
LA435:  bbs7    $FF,$A437
LA438:  bbs7    $FF,$A43A
LA43B:  bbs7    $FF,$A43D
LA43E:  bbs7    $FF,$A440
LA441:  bbs7    $FF,$A443
LA444:  bbs7    $FF,$A446
LA447:  bbs7    $FF,$A449
LA44A:  bbs7    $FF,$A44C
LA44D:  bbs7    $FF,$A44F
LA450:  bbs7    $FF,$A452
LA453:  bbs7    $FF,$A455
LA456:  bbs7    $FF,$A458
LA459:  bbs7    $FF,$A45B
LA45C:  bbs7    $FF,$A45E
LA45F:  bbs7    $FF,$A461
LA462:  bbs7    $FF,$A464
LA465:  bbs7    $FF,$A467
LA468:  bbs7    $FF,$A46A
LA46B:  bbs7    $FF,$A46D
LA46E:  bbs7    $FF,$A470
LA471:  bbs7    $FF,$A473
LA474:  bbs7    $FF,$A476
LA477:  bbs7    $FF,$A479
LA47A:  bbs7    $FF,$A47C
LA47D:  bbs7    $FF,$A47F
LA480:  bbs7    $FF,$A482
LA483:  bbs7    $FF,$A485
LA486:  bbs7    $FF,$A488
LA489:  bbs7    $FF,$A48B
LA48C:  bbs7    $FF,$A48E
LA48F:  bbs7    $FF,$A491
LA492:  bbs7    $FF,$A494
LA495:  bbs7    $FF,$A497
LA498:  bbs7    $FF,$A49A
LA49B:  bbs7    $FF,$A49D
LA49E:  bbs7    $FF,$A4A0
LA4A1:  bbs7    $FF,$A4A3
LA4A4:  bbs7    $FF,$A4A6
LA4A7:  bbs7    $FF,$A4A9
LA4AA:  bbs7    $FF,$A4AC
LA4AD:  bbs7    $FF,$A4AF
LA4B0:  bbs7    $FF,$A4B2
LA4B3:  bbs7    $FF,$A4B5
LA4B6:  bbs7    $FF,$A4B8
LA4B9:  bbs7    $FF,$A4BB
LA4BC:  bbs7    $FF,$A4BE
LA4BF:  bbs7    $FF,$A4C1
LA4C2:  bbs7    $FF,$A4C4
LA4C5:  bbs7    $FF,$A4C7
LA4C8:  bbs7    $FF,$A4CA
LA4CB:  bbs7    $FF,$A4CD
LA4CE:  bbs7    $FF,$A4D0
LA4D1:  bbs7    $FF,$A4D3
LA4D4:  bbs7    $FF,$A4D6
LA4D7:  bbs7    $FF,$A4D9
LA4DA:  bbs7    $FF,$A4DC
LA4DD:  bbs7    $FF,$A4DF
LA4E0:  bbs7    $FF,$A4E2
LA4E3:  bbs7    $FF,$A4E5
LA4E6:  bbs7    $FF,$A4E8
LA4E9:  bbs7    $FF,$A4EB
LA4EC:  bbs7    $FF,$A4EE
LA4EF:  bbs7    $FF,$A4F1
LA4F2:  bbs7    $FF,$A4F4
LA4F5:  bbs7    $FF,$A4F7
LA4F8:  bbs7    $FF,$A4FA
LA4FB:  bbs7    $FF,$A4FD
LA4FE:  bbs7    $FF,$A500
LA501:  bbs7    $FF,$A503
LA504:  bbs7    $FF,$A506
LA507:  bbs7    $FF,$A509
LA50A:  bbs7    $FF,$A50C
LA50D:  bbs7    $FF,$A50F
LA510:  bbs7    $FF,$A512
LA513:  bbs7    $FF,$A515
LA516:  bbs7    $FF,$A518
LA519:  bbs7    $FF,$A51B
LA51C:  bbs7    $FF,$A51E
LA51F:  bbs7    $FF,$A521
LA522:  bbs7    $FF,$A524
LA525:  bbs7    $FF,$A527
LA528:  bbs7    $FF,$A52A
LA52B:  bbs7    $FF,$A52D
LA52E:  bbs7    $FF,$A530
LA531:  bbs7    $FF,$A533
LA534:  bbs7    $FF,$A536
LA537:  bbs7    $FF,$A539
LA53A:  bbs7    $FF,$A53C
LA53D:  bbs7    $FF,$A53F
LA540:  bbs7    $FF,$A542
LA543:  bbs7    $FF,$A545
LA546:  bbs7    $FF,$A548
LA549:  bbs7    $FF,$A54B
LA54C:  bbs7    $FF,$A54E
LA54F:  bbs7    $FF,$A551
LA552:  bbs7    $FF,$A554
LA555:  bbs7    $FF,$A557
LA558:  bbs7    $FF,$A55A
LA55B:  bbs7    $FF,$A55D
LA55E:  bbs7    $FF,$A560
LA561:  bbs7    $FF,$A563
LA564:  bbs7    $FF,$A566
LA567:  bbs7    $FF,$A569
LA56A:  bbs7    $FF,$A56C
LA56D:  bbs7    $FF,$A56F
LA570:  bbs7    $FF,$A572
LA573:  bbs7    $FF,$A575
LA576:  bbs7    $FF,$A578
LA579:  bbs7    $FF,$A57B
LA57C:  bbs7    $FF,$A57E
LA57F:  bbs7    $FF,$A581
LA582:  bbs7    $FF,$A584
LA585:  bbs7    $FF,$A587
LA588:  bbs7    $FF,$A58A
LA58B:  bbs7    $FF,$A58D
LA58E:  bbs7    $FF,$A590
LA591:  bbs7    $FF,$A593
LA594:  bbs7    $FF,$A596
LA597:  bbs7    $FF,$A599
LA59A:  bbs7    $FF,$A59C
LA59D:  bbs7    $FF,$A59F
LA5A0:  bbs7    $FF,$A5A2
LA5A3:  bbs7    $FF,$A5A5
LA5A6:  bbs7    $FF,$A5A8
LA5A9:  bbs7    $FF,$A5AB
LA5AC:  bbs7    $FF,$A5AE
LA5AF:  bbs7    $FF,$A5B1
LA5B2:  bbs7    $FF,$A5B4
LA5B5:  bbs7    $FF,$A5B7
LA5B8:  bbs7    $FF,$A5BA
LA5BB:  bbs7    $FF,$A5BD
LA5BE:  bbs7    $FF,$A5C0
LA5C1:  bbs7    $FF,$A5C3
LA5C4:  bbs7    $FF,$A5C6
LA5C7:  bbs7    $FF,$A5C9
LA5CA:  bbs7    $FF,$A5CC
LA5CD:  bbs7    $FF,$A5CF
LA5D0:  bbs7    $FF,$A5D2
LA5D3:  bbs7    $FF,$A5D5
LA5D6:  bbs7    $FF,$A5D8
LA5D9:  bbs7    $FF,$A5DB
LA5DC:  bbs7    $FF,$A5DE
LA5DF:  bbs7    $FF,$A5E1
LA5E2:  bbs7    $FF,$A5E4
LA5E5:  bbs7    $FF,$A5E7
LA5E8:  bbs7    $FF,$A5EA
LA5EB:  bbs7    $FF,$A5ED
LA5EE:  bbs7    $FF,$A5F0
LA5F1:  bbs7    $FF,$A5F3
LA5F4:  bbs7    $FF,$A5F6
LA5F7:  bbs7    $FF,$A5F9
LA5FA:  bbs7    $FF,$A5FC
LA5FD:  bbs7    $FF,$A5FF
LA600:  bbs7    $FF,$A602
LA603:  bbs7    $FF,$A605
LA606:  bbs7    $FF,$A608
LA609:  bbs7    $FF,$A60B
LA60C:  bbs7    $FF,$A60E
LA60F:  bbs7    $FF,$A611
LA612:  bbs7    $FF,$A614
LA615:  bbs7    $FF,$A617
LA618:  bbs7    $FF,$A61A
LA61B:  bbs7    $FF,$A61D
LA61E:  bbs7    $FF,$A620
LA621:  bbs7    $FF,$A623
LA624:  bbs7    $FF,$A626
LA627:  bbs7    $FF,$A629
LA62A:  bbs7    $FF,$A62C
LA62D:  bbs7    $FF,$A62F
LA630:  bbs7    $FF,$A632
LA633:  bbs7    $FF,$A635
LA636:  bbs7    $FF,$A638
LA639:  bbs7    $FF,$A63B
LA63C:  bbs7    $FF,$A63E
LA63F:  bbs7    $FF,$A641
LA642:  bbs7    $FF,$A644
LA645:  bbs7    $FF,$A647
LA648:  bbs7    $FF,$A64A
LA64B:  bbs7    $FF,$A64D
LA64E:  bbs7    $FF,$A650
LA651:  bbs7    $FF,$A653
LA654:  bbs7    $FF,$A656
LA657:  bbs7    $FF,$A659
LA65A:  bbs7    $FF,$A65C
LA65D:  bbs7    $FF,$A65F
LA660:  bbs7    $FF,$A662
LA663:  bbs7    $FF,$A665
LA666:  bbs7    $FF,$A668
LA669:  bbs7    $FF,$A66B
LA66C:  bbs7    $FF,$A66E
LA66F:  bbs7    $FF,$A671
LA672:  bbs7    $FF,$A674
LA675:  bbs7    $FF,$A677
LA678:  bbs7    $FF,$A67A
LA67B:  bbs7    $FF,$A67D
LA67E:  bbs7    $FF,$A680
LA681:  bbs7    $FF,$A683
LA684:  bbs7    $FF,$A686
LA687:  bbs7    $FF,$A689
LA68A:  bbs7    $FF,$A68C
LA68D:  bbs7    $FF,$A68F
LA690:  bbs7    $FF,$A692
LA693:  bbs7    $FF,$A695
LA696:  bbs7    $FF,$A698
LA699:  bbs7    $FF,$A69B
LA69C:  bbs7    $FF,$A69E
LA69F:  bbs7    $FF,$A6A1
LA6A2:  bbs7    $FF,$A6A4
LA6A5:  bbs7    $FF,$A6A7
LA6A8:  bbs7    $FF,$A6AA
LA6AB:  bbs7    $FF,$A6AD
LA6AE:  bbs7    $FF,$A6B0
LA6B1:  bbs7    $FF,$A6B3
LA6B4:  bbs7    $FF,$A6B6
LA6B7:  bbs7    $FF,$A6B9
LA6BA:  bbs7    $FF,$A6BC
LA6BD:  bbs7    $FF,$A6BF
LA6C0:  bbs7    $FF,$A6C2
LA6C3:  bbs7    $FF,$A6C5
LA6C6:  bbs7    $FF,$A6C8
LA6C9:  bbs7    $FF,$A6CB
LA6CC:  bbs7    $FF,$A6CE
LA6CF:  bbs7    $FF,$A6D1
LA6D2:  bbs7    $FF,$A6D4
LA6D5:  bbs7    $FF,$A6D7
LA6D8:  bbs7    $FF,$A6DA
LA6DB:  bbs7    $FF,$A6DD
LA6DE:  bbs7    $FF,$A6E0
LA6E1:  bbs7    $FF,$A6E3
LA6E4:  bbs7    $FF,$A6E6
LA6E7:  bbs7    $FF,$A6E9
LA6EA:  bbs7    $FF,$A6EC
LA6ED:  bbs7    $FF,$A6EF
LA6F0:  bbs7    $FF,$A6F2
LA6F3:  bbs7    $FF,$A6F5
LA6F6:  bbs7    $FF,$A6F8
LA6F9:  bbs7    $FF,$A6FB
LA6FC:  bbs7    $FF,$A6FE
LA6FF:  bbs7    $FF,$A701
LA702:  bbs7    $FF,$A704
LA705:  bbs7    $FF,$A707
LA708:  bbs7    $FF,$A70A
LA70B:  bbs7    $FF,$A70D
LA70E:  bbs7    $FF,$A710
LA711:  bbs7    $FF,$A713
LA714:  bbs7    $FF,$A716
LA717:  bbs7    $FF,$A719
LA71A:  bbs7    $FF,$A71C
LA71D:  bbs7    $FF,$A71F
LA720:  bbs7    $FF,$A722
LA723:  bbs7    $FF,$A725
LA726:  bbs7    $FF,$A728
LA729:  bbs7    $FF,$A72B
LA72C:  bbs7    $FF,$A72E
LA72F:  bbs7    $FF,$A731
LA732:  bbs7    $FF,$A734
LA735:  bbs7    $FF,$A737
LA738:  bbs7    $FF,$A73A
LA73B:  bbs7    $FF,$A73D
LA73E:  bbs7    $FF,$A740
LA741:  bbs7    $FF,$A743
LA744:  bbs7    $FF,$A746
LA747:  bbs7    $FF,$A749
LA74A:  bbs7    $FF,$A74C
LA74D:  bbs7    $FF,$A74F
LA750:  bbs7    $FF,$A752
LA753:  bbs7    $FF,$A755
LA756:  bbs7    $FF,$A758
LA759:  bbs7    $FF,$A75B
LA75C:  bbs7    $FF,$A75E
LA75F:  bbs7    $FF,$A761
LA762:  bbs7    $FF,$A764
LA765:  bbs7    $FF,$A767
LA768:  bbs7    $FF,$A76A
LA76B:  bbs7    $FF,$A76D
LA76E:  bbs7    $FF,$A770
LA771:  bbs7    $FF,$A773
LA774:  bbs7    $FF,$A776
LA777:  bbs7    $FF,$A779
LA77A:  bbs7    $FF,$A77C
LA77D:  bbs7    $FF,$A77F
LA780:  bbs7    $FF,$A782
LA783:  bbs7    $FF,$A785
LA786:  bbs7    $FF,$A788
LA789:  bbs7    $FF,$A78B
LA78C:  bbs7    $FF,$A78E
LA78F:  bbs7    $FF,$A791
LA792:  bbs7    $FF,$A794
LA795:  bbs7    $FF,$A797
LA798:  bbs7    $FF,$A79A
LA79B:  bbs7    $FF,$A79D
LA79E:  bbs7    $FF,$A7A0
LA7A1:  bbs7    $FF,$A7A3
LA7A4:  bbs7    $FF,$A7A6
LA7A7:  bbs7    $FF,$A7A9
LA7AA:  bbs7    $FF,$A7AC
LA7AD:  bbs7    $FF,$A7AF
LA7B0:  bbs7    $FF,$A7B2
LA7B3:  bbs7    $FF,$A7B5
LA7B6:  bbs7    $FF,$A7B8
LA7B9:  bbs7    $FF,$A7BB
LA7BC:  bbs7    $FF,$A7BE
LA7BF:  bbs7    $FF,$A7C1
LA7C2:  bbs7    $FF,$A7C4
LA7C5:  bbs7    $FF,$A7C7
LA7C8:  bbs7    $FF,$A7CA
LA7CB:  bbs7    $FF,$A7CD
LA7CE:  bbs7    $FF,$A7D0
LA7D1:  bbs7    $FF,$A7D3
LA7D4:  bbs7    $FF,$A7D6
LA7D7:  bbs7    $FF,$A7D9
LA7DA:  bbs7    $FF,$A7DC
LA7DD:  bbs7    $FF,$A7DF
LA7E0:  bbs7    $FF,$A7E2
LA7E3:  bbs7    $FF,$A7E5
LA7E6:  bbs7    $FF,$A7E8
LA7E9:  bbs7    $FF,$A7EB
LA7EC:  bbs7    $FF,$A7EE
LA7EF:  bbs7    $FF,$A7F1
LA7F2:  bbs7    $FF,$A7F4
LA7F5:  bbs7    $FF,$A7F7
LA7F8:  bbs7    $FF,$A7FA
LA7FB:  bbs7    $FF,$A7FD
LA7FE:  bbs7    $FF,$A800
LA801:  bbs7    $FF,$A803
LA804:  bbs7    $FF,$A806
LA807:  bbs7    $FF,$A809
LA80A:  bbs7    $FF,$A80C
LA80D:  bbs7    $FF,$A80F
LA810:  bbs7    $FF,$A812
LA813:  bbs7    $FF,$A815
LA816:  bbs7    $FF,$A818
LA819:  bbs7    $FF,$A81B
LA81C:  bbs7    $FF,$A81E
LA81F:  bbs7    $FF,$A821
LA822:  bbs7    $FF,$A824
LA825:  bbs7    $FF,$A827
LA828:  bbs7    $FF,$A82A
LA82B:  bbs7    $FF,$A82D
LA82E:  bbs7    $FF,$A830
LA831:  bbs7    $FF,$A833
LA834:  bbs7    $FF,$A836
LA837:  bbs7    $FF,$A839
LA83A:  bbs7    $FF,$A83C
LA83D:  bbs7    $FF,$A83F
LA840:  bbs7    $FF,$A842
LA843:  bbs7    $FF,$A845
LA846:  bbs7    $FF,$A848
LA849:  bbs7    $FF,$A84B
LA84C:  bbs7    $FF,$A84E
LA84F:  bbs7    $FF,$A851
LA852:  bbs7    $FF,$A854
LA855:  bbs7    $FF,$A857
LA858:  bbs7    $FF,$A85A
LA85B:  bbs7    $FF,$A85D
LA85E:  bbs7    $FF,$A860
LA861:  bbs7    $FF,$A863
LA864:  bbs7    $FF,$A866
LA867:  bbs7    $FF,$A869
LA86A:  bbs7    $FF,$A86C
LA86D:  bbs7    $FF,$A86F
LA870:  bbs7    $FF,$A872
LA873:  bbs7    $FF,$A875
LA876:  bbs7    $FF,$A878
LA879:  bbs7    $FF,$A87B
LA87C:  bbs7    $FF,$A87E
LA87F:  bbs7    $FF,$A881
LA882:  bbs7    $FF,$A884
LA885:  bbs7    $FF,$A887
LA888:  bbs7    $FF,$A88A
LA88B:  bbs7    $FF,$A88D
LA88E:  bbs7    $FF,$A890
LA891:  bbs7    $FF,$A893
LA894:  bbs7    $FF,$A896
LA897:  bbs7    $FF,$A899
LA89A:  bbs7    $FF,$A89C
LA89D:  bbs7    $FF,$A89F
LA8A0:  bbs7    $FF,$A8A2
LA8A3:  bbs7    $FF,$A8A5
LA8A6:  bbs7    $FF,$A8A8
LA8A9:  bbs7    $FF,$A8AB
LA8AC:  bbs7    $FF,$A8AE
LA8AF:  bbs7    $FF,$A8B1
LA8B2:  bbs7    $FF,$A8B4
LA8B5:  bbs7    $FF,$A8B7
LA8B8:  bbs7    $FF,$A8BA
LA8BB:  bbs7    $FF,$A8BD
LA8BE:  bbs7    $FF,$A8C0
LA8C1:  bbs7    $FF,$A8C3
LA8C4:  bbs7    $FF,$A8C6
LA8C7:  bbs7    $FF,$A8C9
LA8CA:  bbs7    $FF,$A8CC
LA8CD:  bbs7    $FF,$A8CF
LA8D0:  bbs7    $FF,$A8D2
LA8D3:  bbs7    $FF,$A8D5
LA8D6:  bbs7    $FF,$A8D8
LA8D9:  bbs7    $FF,$A8DB
LA8DC:  bbs7    $FF,$A8DE
LA8DF:  bbs7    $FF,$A8E1
LA8E2:  bbs7    $FF,$A8E4
LA8E5:  bbs7    $FF,$A8E7
LA8E8:  bbs7    $FF,$A8EA
LA8EB:  bbs7    $FF,$A8ED
LA8EE:  bbs7    $FF,$A8F0
LA8F1:  bbs7    $FF,$A8F3
LA8F4:  bbs7    $FF,$A8F6
LA8F7:  bbs7    $FF,$A8F9
LA8FA:  bbs7    $FF,$A8FC
LA8FD:  bbs7    $FF,$A8FF
LA900:  bbs7    $FF,$A902
LA903:  bbs7    $FF,$A905
LA906:  bbs7    $FF,$A908
LA909:  bbs7    $FF,$A90B
LA90C:  bbs7    $FF,$A90E
LA90F:  bbs7    $FF,$A911
LA912:  bbs7    $FF,$A914
LA915:  bbs7    $FF,$A917
LA918:  bbs7    $FF,$A91A
LA91B:  bbs7    $FF,$A91D
LA91E:  bbs7    $FF,$A920
LA921:  bbs7    $FF,$A923
LA924:  bbs7    $FF,$A926
LA927:  bbs7    $FF,$A929
LA92A:  bbs7    $FF,$A92C
LA92D:  bbs7    $FF,$A92F
LA930:  bbs7    $FF,$A932
LA933:  bbs7    $FF,$A935
LA936:  bbs7    $FF,$A938
LA939:  bbs7    $FF,$A93B
LA93C:  bbs7    $FF,$A93E
LA93F:  bbs7    $FF,$A941
LA942:  bbs7    $FF,$A944
LA945:  bbs7    $FF,$A947
LA948:  bbs7    $FF,$A94A
LA94B:  bbs7    $FF,$A94D
LA94E:  bbs7    $FF,$A950
LA951:  bbs7    $FF,$A953
LA954:  bbs7    $FF,$A956
LA957:  bbs7    $FF,$A959
LA95A:  bbs7    $FF,$A95C
LA95D:  bbs7    $FF,$A95F
LA960:  bbs7    $FF,$A962
LA963:  bbs7    $FF,$A965
LA966:  bbs7    $FF,$A968
LA969:  bbs7    $FF,$A96B
LA96C:  bbs7    $FF,$A96E
LA96F:  bbs7    $FF,$A971
LA972:  bbs7    $FF,$A974
LA975:  bbs7    $FF,$A977
LA978:  bbs7    $FF,$A97A
LA97B:  bbs7    $FF,$A97D
LA97E:  bbs7    $FF,$A980
LA981:  bbs7    $FF,$A983
LA984:  bbs7    $FF,$A986
LA987:  bbs7    $FF,$A989
LA98A:  bbs7    $FF,$A98C
LA98D:  bbs7    $FF,$A98F
LA990:  bbs7    $FF,$A992
LA993:  bbs7    $FF,$A995
LA996:  bbs7    $FF,$A998
LA999:  bbs7    $FF,$A99B
LA99C:  bbs7    $FF,$A99E
LA99F:  bbs7    $FF,$A9A1
LA9A2:  bbs7    $FF,$A9A4
LA9A5:  bbs7    $FF,$A9A7
LA9A8:  bbs7    $FF,$A9AA
LA9AB:  bbs7    $FF,$A9AD
LA9AE:  bbs7    $FF,$A9B0
LA9B1:  bbs7    $FF,$A9B3
LA9B4:  bbs7    $FF,$A9B6
LA9B7:  bbs7    $FF,$A9B9
LA9BA:  bbs7    $FF,$A9BC
LA9BD:  bbs7    $FF,$A9BF
LA9C0:  bbs7    $FF,$A9C2
LA9C3:  bbs7    $FF,$A9C5
LA9C6:  bbs7    $FF,$A9C8
LA9C9:  bbs7    $FF,$A9CB
LA9CC:  bbs7    $FF,$A9CE
LA9CF:  bbs7    $FF,$A9D1
LA9D2:  bbs7    $FF,$A9D4
LA9D5:  bbs7    $FF,$A9D7
LA9D8:  bbs7    $FF,$A9DA
LA9DB:  bbs7    $FF,$A9DD
LA9DE:  bbs7    $FF,$A9E0
LA9E1:  bbs7    $FF,$A9E3
LA9E4:  bbs7    $FF,$A9E6
LA9E7:  bbs7    $FF,$A9E9
LA9EA:  bbs7    $FF,$A9EC
LA9ED:  bbs7    $FF,$A9EF
LA9F0:  bbs7    $FF,$A9F2
LA9F3:  bbs7    $FF,$A9F5
LA9F6:  bbs7    $FF,$A9F8
LA9F9:  bbs7    $FF,$A9FB
LA9FC:  bbs7    $FF,$A9FE
LA9FF:  bbs7    $FF,$AA01
LAA02:  bbs7    $FF,$AA04
LAA05:  bbs7    $FF,$AA07
LAA08:  bbs7    $FF,$AA0A
LAA0B:  bbs7    $FF,$AA0D
LAA0E:  bbs7    $FF,$AA10
LAA11:  bbs7    $FF,$AA13
LAA14:  bbs7    $FF,$AA16
LAA17:  bbs7    $FF,$AA19
LAA1A:  bbs7    $FF,$AA1C
LAA1D:  bbs7    $FF,$AA1F
LAA20:  bbs7    $FF,$AA22
LAA23:  bbs7    $FF,$AA25
LAA26:  bbs7    $FF,$AA28
LAA29:  bbs7    $FF,$AA2B
LAA2C:  bbs7    $FF,$AA2E
LAA2F:  bbs7    $FF,$AA31
LAA32:  bbs7    $FF,$AA34
LAA35:  bbs7    $FF,$AA37
LAA38:  bbs7    $FF,$AA3A
LAA3B:  bbs7    $FF,$AA3D
LAA3E:  bbs7    $FF,$AA40
LAA41:  bbs7    $FF,$AA43
LAA44:  bbs7    $FF,$AA46
LAA47:  bbs7    $FF,$AA49
LAA4A:  bbs7    $FF,$AA4C
LAA4D:  bbs7    $FF,$AA4F
LAA50:  bbs7    $FF,$AA52
LAA53:  bbs7    $FF,$AA55
LAA56:  bbs7    $FF,$AA58
LAA59:  bbs7    $FF,$AA5B
LAA5C:  bbs7    $FF,$AA5E
LAA5F:  bbs7    $FF,$AA61
LAA62:  bbs7    $FF,$AA64
LAA65:  bbs7    $FF,$AA67
LAA68:  bbs7    $FF,$AA6A
LAA6B:  bbs7    $FF,$AA6D
LAA6E:  bbs7    $FF,$AA70
LAA71:  bbs7    $FF,$AA73
LAA74:  bbs7    $FF,$AA76
LAA77:  bbs7    $FF,$AA79
LAA7A:  bbs7    $FF,$AA7C
LAA7D:  bbs7    $FF,$AA7F
LAA80:  bbs7    $FF,$AA82
LAA83:  bbs7    $FF,$AA85
LAA86:  bbs7    $FF,$AA88
LAA89:  bbs7    $FF,$AA8B
LAA8C:  bbs7    $FF,$AA8E
LAA8F:  bbs7    $FF,$AA91
LAA92:  bbs7    $FF,$AA94
LAA95:  bbs7    $FF,$AA97
LAA98:  bbs7    $FF,$AA9A
LAA9B:  bbs7    $FF,$AA9D
LAA9E:  bbs7    $FF,$AAA0
LAAA1:  bbs7    $FF,$AAA3
LAAA4:  bbs7    $FF,$AAA6
LAAA7:  bbs7    $FF,$AAA9
LAAAA:  bbs7    $FF,$AAAC
LAAAD:  bbs7    $FF,$AAAF
LAAB0:  bbs7    $FF,$AAB2
LAAB3:  bbs7    $FF,$AAB5
LAAB6:  bbs7    $FF,$AAB8
LAAB9:  bbs7    $FF,$AABB
LAABC:  bbs7    $FF,$AABE
LAABF:  bbs7    $FF,$AAC1
LAAC2:  bbs7    $FF,$AAC4
LAAC5:  bbs7    $FF,$AAC7
LAAC8:  bbs7    $FF,$AACA
LAACB:  bbs7    $FF,$AACD
LAACE:  bbs7    $FF,$AAD0
LAAD1:  bbs7    $FF,$AAD3
LAAD4:  bbs7    $FF,$AAD6
LAAD7:  bbs7    $FF,$AAD9
LAADA:  bbs7    $FF,$AADC
LAADD:  bbs7    $FF,$AADF
LAAE0:  bbs7    $FF,$AAE2
LAAE3:  bbs7    $FF,$AAE5
LAAE6:  bbs7    $FF,$AAE8
LAAE9:  bbs7    $FF,$AAEB
LAAEC:  bbs7    $FF,$AAEE
LAAEF:  bbs7    $FF,$AAF1
LAAF2:  bbs7    $FF,$AAF4
LAAF5:  bbs7    $FF,$AAF7
LAAF8:  bbs7    $FF,$AAFA
LAAFB:  bbs7    $FF,$AAFD
LAAFE:  bbs7    $FF,$AB00
LAB01:  bbs7    $FF,$AB03
LAB04:  bbs7    $FF,$AB06
LAB07:  bbs7    $FF,$AB09
LAB0A:  bbs7    $FF,$AB0C
LAB0D:  bbs7    $FF,$AB0F
LAB10:  bbs7    $FF,$AB12
LAB13:  bbs7    $FF,$AB15
LAB16:  bbs7    $FF,$AB18
LAB19:  bbs7    $FF,$AB1B
LAB1C:  bbs7    $FF,$AB1E
LAB1F:  bbs7    $FF,$AB21
LAB22:  bbs7    $FF,$AB24
LAB25:  bbs7    $FF,$AB27
LAB28:  bbs7    $FF,$AB2A
LAB2B:  bbs7    $FF,$AB2D
LAB2E:  bbs7    $FF,$AB30
LAB31:  bbs7    $FF,$AB33
LAB34:  bbs7    $FF,$AB36
LAB37:  bbs7    $FF,$AB39
LAB3A:  bbs7    $FF,$AB3C
LAB3D:  bbs7    $FF,$AB3F
LAB40:  bbs7    $FF,$AB42
LAB43:  bbs7    $FF,$AB45
LAB46:  bbs7    $FF,$AB48
LAB49:  bbs7    $FF,$AB4B
LAB4C:  bbs7    $FF,$AB4E
LAB4F:  bbs7    $FF,$AB51
LAB52:  bbs7    $FF,$AB54
LAB55:  bbs7    $FF,$AB57
LAB58:  bbs7    $FF,$AB5A
LAB5B:  bbs7    $FF,$AB5D
LAB5E:  bbs7    $FF,$AB60
LAB61:  bbs7    $FF,$AB63
LAB64:  bbs7    $FF,$AB66
LAB67:  bbs7    $FF,$AB69
LAB6A:  bbs7    $FF,$AB6C
LAB6D:  bbs7    $FF,$AB6F
LAB70:  bbs7    $FF,$AB72
LAB73:  bbs7    $FF,$AB75
LAB76:  bbs7    $FF,$AB78
LAB79:  bbs7    $FF,$AB7B
LAB7C:  bbs7    $FF,$AB7E
LAB7F:  bbs7    $FF,$AB81
LAB82:  bbs7    $FF,$AB84
LAB85:  bbs7    $FF,$AB87
LAB88:  bbs7    $FF,$AB8A
LAB8B:  bbs7    $FF,$AB8D
LAB8E:  bbs7    $FF,$AB90
LAB91:  bbs7    $FF,$AB93
LAB94:  bbs7    $FF,$AB96
LAB97:  bbs7    $FF,$AB99
LAB9A:  bbs7    $FF,$AB9C
LAB9D:  bbs7    $FF,$AB9F
LABA0:  bbs7    $FF,$ABA2
LABA3:  bbs7    $FF,$ABA5
LABA6:  bbs7    $FF,$ABA8
LABA9:  bbs7    $FF,$ABAB
LABAC:  bbs7    $FF,$ABAE
LABAF:  bbs7    $FF,$ABB1
LABB2:  bbs7    $FF,$ABB4
LABB5:  bbs7    $FF,$ABB7
LABB8:  bbs7    $FF,$ABBA
LABBB:  bbs7    $FF,$ABBD
LABBE:  bbs7    $FF,$ABC0
LABC1:  bbs7    $FF,$ABC3
LABC4:  bbs7    $FF,$ABC6
LABC7:  bbs7    $FF,$ABC9
LABCA:  bbs7    $FF,$ABCC
LABCD:  bbs7    $FF,$ABCF
LABD0:  bbs7    $FF,$ABD2
LABD3:  bbs7    $FF,$ABD5
LABD6:  bbs7    $FF,$ABD8
LABD9:  bbs7    $FF,$ABDB
LABDC:  bbs7    $FF,$ABDE
LABDF:  bbs7    $FF,$ABE1
LABE2:  bbs7    $FF,$ABE4
LABE5:  bbs7    $FF,$ABE7
LABE8:  bbs7    $FF,$ABEA
LABEB:  bbs7    $FF,$ABED
LABEE:  bbs7    $FF,$ABF0
LABF1:  bbs7    $FF,$ABF3
LABF4:  bbs7    $FF,$ABF6
LABF7:  bbs7    $FF,$ABF9
LABFA:  bbs7    $FF,$ABFC
LABFD:  bbs7    $FF,$ABFF
LAC00:  bbs7    $FF,$AC02
LAC03:  bbs7    $FF,$AC05
LAC06:  bbs7    $FF,$AC08
LAC09:  bbs7    $FF,$AC0B
LAC0C:  bbs7    $FF,$AC0E
LAC0F:  bbs7    $FF,$AC11
LAC12:  bbs7    $FF,$AC14
LAC15:  bbs7    $FF,$AC17
LAC18:  bbs7    $FF,$AC1A
LAC1B:  bbs7    $FF,$AC1D
LAC1E:  bbs7    $FF,$AC20
LAC21:  bbs7    $FF,$AC23
LAC24:  bbs7    $FF,$AC26
LAC27:  bbs7    $FF,$AC29
LAC2A:  bbs7    $FF,$AC2C
LAC2D:  bbs7    $FF,$AC2F
LAC30:  bbs7    $FF,$AC32
LAC33:  bbs7    $FF,$AC35
LAC36:  bbs7    $FF,$AC38
LAC39:  bbs7    $FF,$AC3B
LAC3C:  bbs7    $FF,$AC3E
LAC3F:  bbs7    $FF,$AC41
LAC42:  bbs7    $FF,$AC44
LAC45:  bbs7    $FF,$AC47
LAC48:  bbs7    $FF,$AC4A
LAC4B:  bbs7    $FF,$AC4D
LAC4E:  bbs7    $FF,$AC50
LAC51:  bbs7    $FF,$AC53
LAC54:  bbs7    $FF,$AC56
LAC57:  bbs7    $FF,$AC59
LAC5A:  bbs7    $FF,$AC5C
LAC5D:  bbs7    $FF,$AC5F
LAC60:  bbs7    $FF,$AC62
LAC63:  bbs7    $FF,$AC65
LAC66:  bbs7    $FF,$AC68
LAC69:  bbs7    $FF,$AC6B
LAC6C:  bbs7    $FF,$AC6E
LAC6F:  bbs7    $FF,$AC71
LAC72:  bbs7    $FF,$AC74
LAC75:  bbs7    $FF,$AC77
LAC78:  bbs7    $FF,$AC7A
LAC7B:  bbs7    $FF,$AC7D
LAC7E:  bbs7    $FF,$AC80
LAC81:  bbs7    $FF,$AC83
LAC84:  bbs7    $FF,$AC86
LAC87:  bbs7    $FF,$AC89
LAC8A:  bbs7    $FF,$AC8C
LAC8D:  bbs7    $FF,$AC8F
LAC90:  bbs7    $FF,$AC92
LAC93:  bbs7    $FF,$AC95
LAC96:  bbs7    $FF,$AC98
LAC99:  bbs7    $FF,$AC9B
LAC9C:  bbs7    $FF,$AC9E
LAC9F:  bbs7    $FF,$ACA1
LACA2:  bbs7    $FF,$ACA4
LACA5:  bbs7    $FF,$ACA7
LACA8:  bbs7    $FF,$ACAA
LACAB:  bbs7    $FF,$ACAD
LACAE:  bbs7    $FF,$ACB0
LACB1:  bbs7    $FF,$ACB3
LACB4:  bbs7    $FF,$ACB6
LACB7:  bbs7    $FF,$ACB9
LACBA:  bbs7    $FF,$ACBC
LACBD:  bbs7    $FF,$ACBF
LACC0:  bbs7    $FF,$ACC2
LACC3:  bbs7    $FF,$ACC5
LACC6:  bbs7    $FF,$ACC8
LACC9:  bbs7    $FF,$ACCB
LACCC:  bbs7    $FF,$ACCE
LACCF:  bbs7    $FF,$ACD1
LACD2:  bbs7    $FF,$ACD4
LACD5:  bbs7    $FF,$ACD7
LACD8:  bbs7    $FF,$ACDA
LACDB:  bbs7    $FF,$ACDD
LACDE:  bbs7    $FF,$ACE0
LACE1:  bbs7    $FF,$ACE3
LACE4:  bbs7    $FF,$ACE6
LACE7:  bbs7    $FF,$ACE9
LACEA:  bbs7    $FF,$ACEC
LACED:  bbs7    $FF,$ACEF
LACF0:  bbs7    $FF,$ACF2
LACF3:  bbs7    $FF,$ACF5
LACF6:  bbs7    $FF,$ACF8
LACF9:  bbs7    $FF,$ACFB
LACFC:  bbs7    $FF,$ACFE
LACFF:  bbs7    $FF,$AD01
LAD02:  bbs7    $FF,$AD04
LAD05:  bbs7    $FF,$AD07
LAD08:  bbs7    $FF,$AD0A
LAD0B:  bbs7    $FF,$AD0D
LAD0E:  bbs7    $FF,$AD10
LAD11:  bbs7    $FF,$AD13
LAD14:  bbs7    $FF,$AD16
LAD17:  bbs7    $FF,$AD19
LAD1A:  bbs7    $FF,$AD1C
LAD1D:  bbs7    $FF,$AD1F
LAD20:  bbs7    $FF,$AD22
LAD23:  bbs7    $FF,$AD25
LAD26:  bbs7    $FF,$AD28
LAD29:  bbs7    $FF,$AD2B
LAD2C:  bbs7    $FF,$AD2E
LAD2F:  bbs7    $FF,$AD31
LAD32:  bbs7    $FF,$AD34
LAD35:  bbs7    $FF,$AD37
LAD38:  bbs7    $FF,$AD3A
LAD3B:  bbs7    $FF,$AD3D
LAD3E:  bbs7    $FF,$AD40
LAD41:  bbs7    $FF,$AD43
LAD44:  bbs7    $FF,$AD46
LAD47:  bbs7    $FF,$AD49
LAD4A:  bbs7    $FF,$AD4C
LAD4D:  bbs7    $FF,$AD4F
LAD50:  bbs7    $FF,$AD52
LAD53:  bbs7    $FF,$AD55
LAD56:  bbs7    $FF,$AD58
LAD59:  bbs7    $FF,$AD5B
LAD5C:  bbs7    $FF,$AD5E
LAD5F:  bbs7    $FF,$AD61
LAD62:  bbs7    $FF,$AD64
LAD65:  bbs7    $FF,$AD67
LAD68:  bbs7    $FF,$AD6A
LAD6B:  bbs7    $FF,$AD6D
LAD6E:  bbs7    $FF,$AD70
LAD71:  bbs7    $FF,$AD73
LAD74:  bbs7    $FF,$AD76
LAD77:  bbs7    $FF,$AD79
LAD7A:  bbs7    $FF,$AD7C
LAD7D:  bbs7    $FF,$AD7F
LAD80:  bbs7    $FF,$AD82
LAD83:  bbs7    $FF,$AD85
LAD86:  bbs7    $FF,$AD88
LAD89:  bbs7    $FF,$AD8B
LAD8C:  bbs7    $FF,$AD8E
LAD8F:  bbs7    $FF,$AD91
LAD92:  bbs7    $FF,$AD94
LAD95:  bbs7    $FF,$AD97
LAD98:  bbs7    $FF,$AD9A
LAD9B:  bbs7    $FF,$AD9D
LAD9E:  bbs7    $FF,$ADA0
LADA1:  bbs7    $FF,$ADA3
LADA4:  bbs7    $FF,$ADA6
LADA7:  bbs7    $FF,$ADA9
LADAA:  bbs7    $FF,$ADAC
LADAD:  bbs7    $FF,$ADAF
LADB0:  bbs7    $FF,$ADB2
LADB3:  bbs7    $FF,$ADB5
LADB6:  bbs7    $FF,$ADB8
LADB9:  bbs7    $FF,$ADBB
LADBC:  bbs7    $FF,$ADBE
LADBF:  bbs7    $FF,$ADC1
LADC2:  bbs7    $FF,$ADC4
LADC5:  bbs7    $FF,$ADC7
LADC8:  bbs7    $FF,$ADCA
LADCB:  bbs7    $FF,$ADCD
LADCE:  bbs7    $FF,$ADD0
LADD1:  bbs7    $FF,$ADD3
LADD4:  bbs7    $FF,$ADD6
LADD7:  bbs7    $FF,$ADD9
LADDA:  bbs7    $FF,$ADDC
LADDD:  bbs7    $FF,$ADDF
LADE0:  bbs7    $FF,$ADE2
LADE3:  bbs7    $FF,$ADE5
LADE6:  bbs7    $FF,$ADE8
LADE9:  bbs7    $FF,$ADEB
LADEC:  bbs7    $FF,$ADEE
LADEF:  bbs7    $FF,$ADF1
LADF2:  bbs7    $FF,$ADF4
LADF5:  bbs7    $FF,$ADF7
LADF8:  bbs7    $FF,$ADFA
LADFB:  bbs7    $FF,$ADFD
LADFE:  bbs7    $FF,$AE00
LAE01:  bbs7    $FF,$AE03
LAE04:  bbs7    $FF,$AE06
LAE07:  bbs7    $FF,$AE09
LAE0A:  bbs7    $FF,$AE0C
LAE0D:  bbs7    $FF,$AE0F
LAE10:  bbs7    $FF,$AE12
LAE13:  bbs7    $FF,$AE15
LAE16:  bbs7    $FF,$AE18
LAE19:  bbs7    $FF,$AE1B
LAE1C:  bbs7    $FF,$AE1E
LAE1F:  bbs7    $FF,$AE21
LAE22:  bbs7    $FF,$AE24
LAE25:  bbs7    $FF,$AE27
LAE28:  bbs7    $FF,$AE2A
LAE2B:  bbs7    $FF,$AE2D
LAE2E:  bbs7    $FF,$AE30
LAE31:  bbs7    $FF,$AE33
LAE34:  bbs7    $FF,$AE36
LAE37:  bbs7    $FF,$AE39
LAE3A:  bbs7    $FF,$AE3C
LAE3D:  bbs7    $FF,$AE3F
LAE40:  bbs7    $FF,$AE42
LAE43:  bbs7    $FF,$AE45
LAE46:  bbs7    $FF,$AE48
LAE49:  bbs7    $FF,$AE4B
LAE4C:  bbs7    $FF,$AE4E
LAE4F:  bbs7    $FF,$AE51
LAE52:  bbs7    $FF,$AE54
LAE55:  bbs7    $FF,$AE57
LAE58:  bbs7    $FF,$AE5A
LAE5B:  bbs7    $FF,$AE5D
LAE5E:  bbs7    $FF,$AE60
LAE61:  bbs7    $FF,$AE63
LAE64:  bbs7    $FF,$AE66
LAE67:  bbs7    $FF,$AE69
LAE6A:  bbs7    $FF,$AE6C
LAE6D:  bbs7    $FF,$AE6F
LAE70:  bbs7    $FF,$AE72
LAE73:  bbs7    $FF,$AE75
LAE76:  bbs7    $FF,$AE78
LAE79:  bbs7    $FF,$AE7B
LAE7C:  bbs7    $FF,$AE7E
LAE7F:  bbs7    $FF,$AE81
LAE82:  bbs7    $FF,$AE84
LAE85:  bbs7    $FF,$AE87
LAE88:  bbs7    $FF,$AE8A
LAE8B:  bbs7    $FF,$AE8D
LAE8E:  bbs7    $FF,$AE90
LAE91:  bbs7    $FF,$AE93
LAE94:  bbs7    $FF,$AE96
LAE97:  bbs7    $FF,$AE99
LAE9A:  bbs7    $FF,$AE9C
LAE9D:  bbs7    $FF,$AE9F
LAEA0:  bbs7    $FF,$AEA2
LAEA3:  bbs7    $FF,$AEA5
LAEA6:  bbs7    $FF,$AEA8
LAEA9:  bbs7    $FF,$AEAB
LAEAC:  bbs7    $FF,$AEAE
LAEAF:  bbs7    $FF,$AEB1
LAEB2:  bbs7    $FF,$AEB4
LAEB5:  bbs7    $FF,$AEB7
LAEB8:  bbs7    $FF,$AEBA
LAEBB:  bbs7    $FF,$AEBD
LAEBE:  bbs7    $FF,$AEC0
LAEC1:  bbs7    $FF,$AEC3
LAEC4:  bbs7    $FF,$AEC6
LAEC7:  bbs7    $FF,$AEC9
LAECA:  bbs7    $FF,$AECC
LAECD:  bbs7    $FF,$AECF
LAED0:  bbs7    $FF,$AED2
LAED3:  bbs7    $FF,$AED5
LAED6:  bbs7    $FF,$AED8
LAED9:  bbs7    $FF,$AEDB
LAEDC:  bbs7    $FF,$AEDE
LAEDF:  bbs7    $FF,$AEE1
LAEE2:  bbs7    $FF,$AEE4
LAEE5:  bbs7    $FF,$AEE7
LAEE8:  bbs7    $FF,$AEEA
LAEEB:  bbs7    $FF,$AEED
LAEEE:  bbs7    $FF,$AEF0
LAEF1:  bbs7    $FF,$AEF3
LAEF4:  bbs7    $FF,$AEF6
LAEF7:  bbs7    $FF,$AEF9
LAEFA:  bbs7    $FF,$AEFC
LAEFD:  bbs7    $FF,$AEFF
LAF00:  bbs7    $FF,$AF02
LAF03:  bbs7    $FF,$AF05
LAF06:  bbs7    $FF,$AF08
LAF09:  bbs7    $FF,$AF0B
LAF0C:  bbs7    $FF,$AF0E
LAF0F:  bbs7    $FF,$AF11
LAF12:  bbs7    $FF,$AF14
LAF15:  bbs7    $FF,$AF17
LAF18:  bbs7    $FF,$AF1A
LAF1B:  bbs7    $FF,$AF1D
LAF1E:  bbs7    $FF,$AF20
LAF21:  bbs7    $FF,$AF23
LAF24:  bbs7    $FF,$AF26
LAF27:  bbs7    $FF,$AF29
LAF2A:  bbs7    $FF,$AF2C
LAF2D:  bbs7    $FF,$AF2F
LAF30:  bbs7    $FF,$AF32
LAF33:  bbs7    $FF,$AF35
LAF36:  bbs7    $FF,$AF38
LAF39:  bbs7    $FF,$AF3B
LAF3C:  bbs7    $FF,$AF3E
LAF3F:  bbs7    $FF,$AF41
LAF42:  bbs7    $FF,$AF44
LAF45:  bbs7    $FF,$AF47
LAF48:  bbs7    $FF,$AF4A
LAF4B:  bbs7    $FF,$AF4D
LAF4E:  bbs7    $FF,$AF50
LAF51:  bbs7    $FF,$AF53
LAF54:  bbs7    $FF,$AF56
LAF57:  bbs7    $FF,$AF59
LAF5A:  bbs7    $FF,$AF5C
LAF5D:  bbs7    $FF,$AF5F
LAF60:  bbs7    $FF,$AF62
LAF63:  bbs7    $FF,$AF65
LAF66:  bbs7    $FF,$AF68
LAF69:  bbs7    $FF,$AF6B
LAF6C:  bbs7    $FF,$AF6E
LAF6F:  bbs7    $FF,$AF71
LAF72:  bbs7    $FF,$AF74
LAF75:  bbs7    $FF,$AF77
LAF78:  bbs7    $FF,$AF7A
LAF7B:  bbs7    $FF,$AF7D
LAF7E:  bbs7    $FF,$AF80
LAF81:  bbs7    $FF,$AF83
LAF84:  bbs7    $FF,$AF86
LAF87:  bbs7    $FF,$AF89
LAF8A:  bbs7    $FF,$AF8C
LAF8D:  bbs7    $FF,$AF8F
LAF90:  bbs7    $FF,$AF92
LAF93:  bbs7    $FF,$AF95
LAF96:  bbs7    $FF,$AF98
LAF99:  bbs7    $FF,$AF9B
LAF9C:  bbs7    $FF,$AF9E
LAF9F:  bbs7    $FF,$AFA1
LAFA2:  bbs7    $FF,$AFA4
LAFA5:  bbs7    $FF,$AFA7
LAFA8:  bbs7    $FF,$AFAA
LAFAB:  bbs7    $FF,$AFAD
LAFAE:  bbs7    $FF,$AFB0
LAFB1:  bbs7    $FF,$AFB3
LAFB4:  bbs7    $FF,$AFB6
LAFB7:  bbs7    $FF,$AFB9
LAFBA:  bbs7    $FF,$AFBC
LAFBD:  bbs7    $FF,$AFBF
LAFC0:  bbs7    $FF,$AFC2
LAFC3:  bbs7    $FF,$AFC5
LAFC6:  bbs7    $FF,$AFC8
LAFC9:  bbs7    $FF,$AFCB
LAFCC:  bbs7    $FF,$AFCE
LAFCF:  bbs7    $FF,$AFD1
LAFD2:  bbs7    $FF,$AFD4
LAFD5:  bbs7    $FF,$AFD7
LAFD8:  bbs7    $FF,$AFDA
LAFDB:  bbs7    $FF,$AFDD
LAFDE:  bbs7    $FF,$AFE0
LAFE1:  bbs7    $FF,$AFE3
LAFE4:  bbs7    $FF,$AFE6
LAFE7:  bbs7    $FF,$AFE9
LAFEA:  bbs7    $FF,$AFEC
LAFED:  bbs7    $FF,$AFEF
LAFF0:  bbs7    $FF,$AFF2
LAFF3:  bbs7    $FF,$AFF5
LAFF6:  bbs7    $FF,$AFF8
LAFF9:  bbs7    $FF,$AFFB
LAFFC:  bbs7    $FF,$AFFE
LAFFF:  .byte   $FF
        .byte   $FF
LB001:  .byte   $FF
        .byte   $FF
LB003:  bbs7    $FF,$B005
LB006:  bbs7    $FF,$B008
LB009:  bbs7    $FF,$B00B
LB00C:  bbs7    $FF,$B00E
LB00F:  bbs7    $FF,$B011
LB012:  bbs7    $FF,$B014
LB015:  bbs7    $FF,$B017
LB018:  bbs7    $FF,$B01A
LB01B:  bbs7    $FF,$B01D
LB01E:  bbs7    $FF,$B020
LB021:  bbs7    $FF,$B023
LB024:  bbs7    $FF,$B026
LB027:  bbs7    $FF,$B029
LB02A:  bbs7    $FF,$B02C
LB02D:  bbs7    $FF,$B02F
LB030:  bbs7    $FF,$B032
LB033:  bbs7    $FF,$B035
LB036:  bbs7    $FF,$B038
LB039:  bbs7    $FF,$B03B
LB03C:  bbs7    $FF,$B03E
LB03F:  bbs7    $FF,$B041
LB042:  bbs7    $FF,$B044
LB045:  bbs7    $FF,$B047
LB048:  bbs7    $FF,$B04A
LB04B:  bbs7    $FF,$B04D
LB04E:  bbs7    $FF,$B050
LB051:  bbs7    $FF,$B053
LB054:  bbs7    $FF,$B056
LB057:  bbs7    $FF,$B059
LB05A:  bbs7    $FF,$B05C
LB05D:  bbs7    $FF,$B05F
LB060:  bbs7    $FF,$B062
LB063:  bbs7    $FF,$B065
LB066:  bbs7    $FF,$B068
LB069:  bbs7    $FF,$B06B
LB06C:  bbs7    $FF,$B06E
LB06F:  bbs7    $FF,$B071
LB072:  bbs7    $FF,$B074
LB075:  bbs7    $FF,$B077
LB078:  bbs7    $FF,$B07A
LB07B:  bbs7    $FF,$B07D
LB07E:  bbs7    $FF,$B080
LB081:  bbs7    $FF,$B083
LB084:  bbs7    $FF,$B086
LB087:  bbs7    $FF,$B089
LB08A:  bbs7    $FF,$B08C
LB08D:  bbs7    $FF,$B08F
LB090:  bbs7    $FF,$B092
LB093:  bbs7    $FF,$B095
LB096:  bbs7    $FF,$B098
LB099:  bbs7    $FF,$B09B
LB09C:  bbs7    $FF,$B09E
LB09F:  bbs7    $FF,$B0A1
LB0A2:  bbs7    $FF,$B0A4
LB0A5:  bbs7    $FF,$B0A7
LB0A8:  bbs7    $FF,$B0AA
LB0AB:  bbs7    $FF,$B0AD
LB0AE:  bbs7    $FF,$B0B0
LB0B1:  bbs7    $FF,$B0B3
LB0B4:  bbs7    $FF,$B0B6
LB0B7:  bbs7    $FF,$B0B9
LB0BA:  bbs7    $FF,$B0BC
LB0BD:  bbs7    $FF,$B0BF
LB0C0:  bbs7    $FF,$B0C2
LB0C3:  bbs7    $FF,$B0C5
LB0C6:  bbs7    $FF,$B0C8
LB0C9:  bbs7    $FF,$B0CB
LB0CC:  bbs7    $FF,$B0CE
LB0CF:  bbs7    $FF,$B0D1
LB0D2:  bbs7    $FF,$B0D4
LB0D5:  bbs7    $FF,$B0D7
LB0D8:  bbs7    $FF,$B0DA
LB0DB:  bbs7    $FF,$B0DD
LB0DE:  bbs7    $FF,$B0E0
LB0E1:  bbs7    $FF,$B0E3
LB0E4:  bbs7    $FF,$B0E6
LB0E7:  bbs7    $FF,$B0E9
LB0EA:  bbs7    $FF,$B0EC
LB0ED:  bbs7    $FF,$B0EF
LB0F0:  bbs7    $FF,$B0F2
LB0F3:  bbs7    $FF,$B0F5
LB0F6:  bbs7    $FF,$B0F8
LB0F9:  bbs7    $FF,$B0FB
LB0FC:  bbs7    $FF,$B0FE
LB0FF:  bbs7    $FF,$B101
LB102:  bbs7    $FF,$B104
LB105:  bbs7    $FF,$B107
LB108:  bbs7    $FF,$B10A
LB10B:  bbs7    $FF,$B10D
LB10E:  bbs7    $FF,$B110
LB111:  bbs7    $FF,$B113
LB114:  bbs7    $FF,$B116
LB117:  bbs7    $FF,$B119
LB11A:  bbs7    $FF,$B11C
LB11D:  bbs7    $FF,$B11F
LB120:  bbs7    $FF,$B122
LB123:  bbs7    $FF,$B125
LB126:  bbs7    $FF,$B128
LB129:  bbs7    $FF,$B12B
LB12C:  bbs7    $FF,$B12E
LB12F:  bbs7    $FF,$B131
LB132:  bbs7    $FF,$B134
LB135:  bbs7    $FF,$B137
LB138:  bbs7    $FF,$B13A
LB13B:  bbs7    $FF,$B13D
LB13E:  bbs7    $FF,$B140
LB141:  bbs7    $FF,$B143
LB144:  bbs7    $FF,$B146
LB147:  bbs7    $FF,$B149
LB14A:  bbs7    $FF,$B14C
LB14D:  bbs7    $FF,$B14F
LB150:  bbs7    $FF,$B152
LB153:  bbs7    $FF,$B155
LB156:  bbs7    $FF,$B158
LB159:  bbs7    $FF,$B15B
LB15C:  bbs7    $FF,$B15E
LB15F:  bbs7    $FF,$B161
LB162:  bbs7    $FF,$B164
LB165:  bbs7    $FF,$B167
LB168:  bbs7    $FF,$B16A
LB16B:  bbs7    $FF,$B16D
LB16E:  bbs7    $FF,$B170
LB171:  bbs7    $FF,$B173
LB174:  bbs7    $FF,$B176
LB177:  bbs7    $FF,$B179
LB17A:  bbs7    $FF,$B17C
LB17D:  bbs7    $FF,$B17F
LB180:  bbs7    $FF,$B182
LB183:  bbs7    $FF,$B185
LB186:  bbs7    $FF,$B188
LB189:  bbs7    $FF,$B18B
LB18C:  bbs7    $FF,$B18E
LB18F:  bbs7    $FF,$B191
LB192:  bbs7    $FF,$B194
LB195:  bbs7    $FF,$B197
LB198:  bbs7    $FF,$B19A
LB19B:  bbs7    $FF,$B19D
LB19E:  bbs7    $FF,$B1A0
LB1A1:  bbs7    $FF,$B1A3
LB1A4:  bbs7    $FF,$B1A6
LB1A7:  bbs7    $FF,$B1A9
LB1AA:  bbs7    $FF,$B1AC
LB1AD:  bbs7    $FF,$B1AF
LB1B0:  bbs7    $FF,$B1B2
LB1B3:  bbs7    $FF,$B1B5
LB1B6:  bbs7    $FF,$B1B8
LB1B9:  bbs7    $FF,$B1BB
LB1BC:  bbs7    $FF,$B1BE
LB1BF:  bbs7    $FF,$B1C1
LB1C2:  bbs7    $FF,$B1C4
LB1C5:  bbs7    $FF,$B1C7
LB1C8:  bbs7    $FF,$B1CA
LB1CB:  bbs7    $FF,$B1CD
LB1CE:  bbs7    $FF,$B1D0
LB1D1:  bbs7    $FF,$B1D3
LB1D4:  bbs7    $FF,$B1D6
LB1D7:  bbs7    $FF,$B1D9
LB1DA:  bbs7    $FF,$B1DC
LB1DD:  bbs7    $FF,$B1DF
LB1E0:  bbs7    $FF,$B1E2
LB1E3:  bbs7    $FF,$B1E5
LB1E6:  bbs7    $FF,$B1E8
LB1E9:  bbs7    $FF,$B1EB
LB1EC:  bbs7    $FF,$B1EE
LB1EF:  bbs7    $FF,$B1F1
LB1F2:  bbs7    $FF,$B1F4
LB1F5:  bbs7    $FF,$B1F7
LB1F8:  bbs7    $FF,$B1FA
LB1FB:  bbs7    $FF,$B1FD
LB1FE:  bbs7    $FF,$B200
LB201:  bbs7    $FF,$B203
LB204:  bbs7    $FF,$B206
LB207:  bbs7    $FF,$B209
LB20A:  bbs7    $FF,$B20C
LB20D:  bbs7    $FF,$B20F
LB210:  bbs7    $FF,$B212
LB213:  bbs7    $FF,$B215
LB216:  bbs7    $FF,$B218
LB219:  bbs7    $FF,$B21B
LB21C:  bbs7    $FF,$B21E
LB21F:  bbs7    $FF,$B221
LB222:  bbs7    $FF,$B224
LB225:  bbs7    $FF,$B227
LB228:  bbs7    $FF,$B22A
LB22B:  bbs7    $FF,$B22D
LB22E:  bbs7    $FF,$B230
LB231:  bbs7    $FF,$B233
LB234:  bbs7    $FF,$B236
LB237:  bbs7    $FF,$B239
LB23A:  bbs7    $FF,$B23C
LB23D:  bbs7    $FF,$B23F
LB240:  .byte   $FF
LB241:  .byte   $FF
        .byte   $FF
LB243:  bbs7    $FF,$B245
LB246:  bbs7    $FF,$B248
LB249:  bbs7    $FF,$B24B
LB24C:  bbs7    $FF,$B24E
LB24F:  bbs7    $FF,$B251
LB252:  bbs7    $FF,$B254
LB255:  bbs7    $FF,$B257
LB258:  bbs7    $FF,$B25A
LB25B:  bbs7    $FF,$B25D
LB25E:  bbs7    $FF,$B260
LB261:  bbs7    $FF,$B263
LB264:  bbs7    $FF,$B266
LB267:  bbs7    $FF,$B269
LB26A:  bbs7    $FF,$B26C
LB26D:  bbs7    $FF,$B26F
LB270:  bbs7    $FF,$B272
LB273:  bbs7    $FF,$B275
LB276:  bbs7    $FF,$B278
LB279:  bbs7    $FF,$B27B
LB27C:  bbs7    $FF,$B27E
LB27F:  bbs7    $FF,$B281
LB282:  bbs7    $FF,$B284
LB285:  bbs7    $FF,$B287
LB288:  bbs7    $FF,$B28A
LB28B:  bbs7    $FF,$B28D
LB28E:  bbs7    $FF,$B290
LB291:  bbs7    $FF,$B293
LB294:  bbs7    $FF,$B296
LB297:  bbs7    $FF,$B299
LB29A:  bbs7    $FF,$B29C
LB29D:  bbs7    $FF,$B29F
LB2A0:  bbs7    $FF,$B2A2
LB2A3:  bbs7    $FF,$B2A5
LB2A6:  bbs7    $FF,$B2A8
LB2A9:  bbs7    $FF,$B2AB
LB2AC:  bbs7    $FF,$B2AE
LB2AF:  bbs7    $FF,$B2B1
LB2B2:  bbs7    $FF,$B2B4
LB2B5:  bbs7    $FF,$B2B7
LB2B8:  bbs7    $FF,$B2BA
LB2BB:  bbs7    $FF,$B2BD
LB2BE:  bbs7    $FF,$B2C0
LB2C1:  bbs7    $FF,$B2C3
LB2C4:  bbs7    $FF,$B2C6
LB2C7:  bbs7    $FF,$B2C9
LB2CA:  bbs7    $FF,$B2CC
LB2CD:  bbs7    $FF,$B2CF
LB2D0:  bbs7    $FF,$B2D2
LB2D3:  bbs7    $FF,$B2D5
LB2D6:  bbs7    $FF,$B2D8
LB2D9:  bbs7    $FF,$B2DB
LB2DC:  bbs7    $FF,$B2DE
LB2DF:  bbs7    $FF,$B2E1
LB2E2:  bbs7    $FF,$B2E4
LB2E5:  bbs7    $FF,$B2E7
LB2E8:  bbs7    $FF,$B2EA
LB2EB:  bbs7    $FF,$B2ED
LB2EE:  bbs7    $FF,$B2F0
LB2F1:  bbs7    $FF,$B2F3
LB2F4:  bbs7    $FF,$B2F6
LB2F7:  bbs7    $FF,$B2F9
LB2FA:  bbs7    $FF,$B2FC
LB2FD:  bbs7    $FF,$B2FF
LB300:  bbs7    $FF,$B302
LB303:  bbs7    $FF,$B305
LB306:  bbs7    $FF,$B308
LB309:  bbs7    $FF,$B30B
LB30C:  bbs7    $FF,$B30E
LB30F:  bbs7    $FF,$B311
LB312:  bbs7    $FF,$B314
LB315:  bbs7    $FF,$B317
LB318:  bbs7    $FF,$B31A
LB31B:  bbs7    $FF,$B31D
LB31E:  bbs7    $FF,$B320
LB321:  bbs7    $FF,$B323
LB324:  bbs7    $FF,$B326
LB327:  bbs7    $FF,$B329
LB32A:  bbs7    $FF,$B32C
LB32D:  bbs7    $FF,$B32F
LB330:  bbs7    $FF,$B332
LB333:  bbs7    $FF,$B335
LB336:  bbs7    $FF,$B338
LB339:  bbs7    $FF,$B33B
LB33C:  bbs7    $FF,$B33E
LB33F:  bbs7    $FF,$B341
LB342:  bbs7    $FF,$B344
LB345:  bbs7    $FF,$B347
LB348:  bbs7    $FF,$B34A
LB34B:  bbs7    $FF,$B34D
LB34E:  bbs7    $FF,$B350
LB351:  bbs7    $FF,$B353
LB354:  bbs7    $FF,$B356
LB357:  bbs7    $FF,$B359
LB35A:  bbs7    $FF,$B35C
LB35D:  bbs7    $FF,$B35F
LB360:  bbs7    $FF,$B362
LB363:  bbs7    $FF,$B365
LB366:  bbs7    $FF,$B368
LB369:  bbs7    $FF,$B36B
LB36C:  bbs7    $FF,$B36E
LB36F:  bbs7    $FF,$B371
LB372:  bbs7    $FF,$B374
LB375:  bbs7    $FF,$B377
LB378:  bbs7    $FF,$B37A
LB37B:  bbs7    $FF,$B37D
LB37E:  bbs7    $FF,$B380
LB381:  bbs7    $FF,$B383
LB384:  bbs7    $FF,$B386
LB387:  bbs7    $FF,$B389
LB38A:  bbs7    $FF,$B38C
LB38D:  bbs7    $FF,$B38F
LB390:  bbs7    $FF,$B392
LB393:  bbs7    $FF,$B395
LB396:  bbs7    $FF,$B398
LB399:  bbs7    $FF,$B39B
LB39C:  bbs7    $FF,$B39E
LB39F:  bbs7    $FF,$B3A1
LB3A2:  bbs7    $FF,$B3A4
LB3A5:  bbs7    $FF,$B3A7
LB3A8:  bbs7    $FF,$B3AA
LB3AB:  bbs7    $FF,$B3AD
LB3AE:  bbs7    $FF,$B3B0
LB3B1:  bbs7    $FF,$B3B3
LB3B4:  bbs7    $FF,$B3B6
LB3B7:  bbs7    $FF,$B3B9
LB3BA:  bbs7    $FF,$B3BC
LB3BD:  bbs7    $FF,$B3BF
LB3C0:  bbs7    $FF,$B3C2
LB3C3:  bbs7    $FF,$B3C5
LB3C6:  bbs7    $FF,$B3C8
LB3C9:  bbs7    $FF,$B3CB
LB3CC:  bbs7    $FF,$B3CE
LB3CF:  bbs7    $FF,$B3D1
LB3D2:  bbs7    $FF,$B3D4
LB3D5:  bbs7    $FF,$B3D7
LB3D8:  bbs7    $FF,$B3DA
LB3DB:  bbs7    $FF,$B3DD
LB3DE:  bbs7    $FF,$B3E0
LB3E1:  bbs7    $FF,$B3E3
LB3E4:  bbs7    $FF,$B3E6
LB3E7:  bbs7    $FF,$B3E9
LB3EA:  bbs7    $FF,$B3EC
LB3ED:  bbs7    $FF,$B3EF
LB3F0:  bbs7    $FF,$B3F2
LB3F3:  bbs7    $FF,$B3F5
LB3F6:  bbs7    $FF,$B3F8
LB3F9:  bbs7    $FF,$B3FB
LB3FC:  bbs7    $FF,$B3FE
LB3FF:  bbs7    $FF,$B401
LB402:  bbs7    $FF,$B404
LB405:  bbs7    $FF,$B407
LB408:  .byte   $FF
        .byte   $FF
LB40A:  .byte   $FF
        .byte   $FF
LB40C:  bbs7    $FF,$B40E
LB40F:  bbs7    $FF,$B411
LB412:  bbs7    $FF,$B414
LB415:  bbs7    $FF,$B417
LB418:  bbs7    $FF,$B41A
LB41B:  bbs7    $FF,$B41D
LB41E:  bbs7    $FF,$B420
LB421:  bbs7    $FF,$B423
LB424:  bbs7    $FF,$B426
LB427:  bbs7    $FF,$B429
LB42A:  bbs7    $FF,$B42C
LB42D:  .byte   $FF
        .byte   $FF
LB42F:  bbs7    $FF,$B431
LB432:  bbs7    $FF,$B434
LB435:  bbs7    $FF,$B437
LB438:  bbs7    $FF,$B43A
LB43B:  bbs7    $FF,$B43D
LB43E:  bbs7    $FF,$B440
LB441:  bbs7    $FF,$B443
LB444:  bbs7    $FF,$B446
LB447:  bbs7    $FF,$B449
LB44A:  bbs7    $FF,$B44C
LB44D:  bbs7    $FF,$B44F
LB450:  bbs7    $FF,$B452
LB453:  bbs7    $FF,$B455
LB456:  bbs7    $FF,$B458
LB459:  bbs7    $FF,$B45B
LB45C:  bbs7    $FF,$B45E
LB45F:  bbs7    $FF,$B461
LB462:  bbs7    $FF,$B464
LB465:  bbs7    $FF,$B467
LB468:  bbs7    $FF,$B46A
LB46B:  bbs7    $FF,$B46D
LB46E:  bbs7    $FF,$B470
LB471:  bbs7    $FF,$B473
LB474:  bbs7    $FF,$B476
LB477:  bbs7    $FF,$B479
LB47A:  bbs7    $FF,$B47C
LB47D:  bbs7    $FF,$B47F
LB480:  bbs7    $FF,$B482
LB483:  bbs7    $FF,$B485
LB486:  bbs7    $FF,$B488
LB489:  bbs7    $FF,$B48B
LB48C:  bbs7    $FF,$B48E
LB48F:  bbs7    $FF,$B491
LB492:  bbs7    $FF,$B494
LB495:  bbs7    $FF,$B497
LB498:  bbs7    $FF,$B49A
LB49B:  bbs7    $FF,$B49D
LB49E:  bbs7    $FF,$B4A0
LB4A1:  bbs7    $FF,$B4A3
LB4A4:  bbs7    $FF,$B4A6
LB4A7:  bbs7    $FF,$B4A9
LB4AA:  bbs7    $FF,$B4AC
LB4AD:  bbs7    $FF,$B4AF
LB4B0:  bbs7    $FF,$B4B2
LB4B3:  bbs7    $FF,$B4B5
LB4B6:  bbs7    $FF,$B4B8
LB4B9:  bbs7    $FF,$B4BB
LB4BC:  bbs7    $FF,$B4BE
LB4BF:  bbs7    $FF,$B4C1
LB4C2:  bbs7    $FF,$B4C4
LB4C5:  bbs7    $FF,$B4C7
LB4C8:  bbs7    $FF,$B4CA
LB4CB:  bbs7    $FF,$B4CD
LB4CE:  bbs7    $FF,$B4D0
LB4D1:  bbs7    $FF,$B4D3
LB4D4:  bbs7    $FF,$B4D6
LB4D7:  bbs7    $FF,$B4D9
LB4DA:  bbs7    $FF,$B4DC
LB4DD:  bbs7    $FF,$B4DF
LB4E0:  bbs7    $FF,$B4E2
LB4E3:  bbs7    $FF,$B4E5
LB4E6:  bbs7    $FF,$B4E8
LB4E9:  bbs7    $FF,$B4EB
LB4EC:  bbs7    $FF,$B4EE
LB4EF:  bbs7    $FF,$B4F1
LB4F2:  bbs7    $FF,$B4F4
LB4F5:  bbs7    $FF,$B4F7
LB4F8:  bbs7    $FF,$B4FA
LB4FB:  bbs7    $FF,$B4FD
LB4FE:  bbs7    $FF,$B500
LB501:  bbs7    $FF,$B503
LB504:  bbs7    $FF,$B506
LB507:  bbs7    $FF,$B509
LB50A:  bbs7    $FF,$B50C
LB50D:  bbs7    $FF,$B50F
LB510:  bbs7    $FF,$B512
LB513:  bbs7    $FF,$B515
LB516:  bbs7    $FF,$B518
LB519:  bbs7    $FF,$B51B
LB51C:  bbs7    $FF,$B51E
LB51F:  bbs7    $FF,$B521
LB522:  bbs7    $FF,$B524
LB525:  bbs7    $FF,$B527
LB528:  bbs7    $FF,$B52A
LB52B:  bbs7    $FF,$B52D
LB52E:  bbs7    $FF,$B530
LB531:  bbs7    $FF,$B533
LB534:  bbs7    $FF,$B536
LB537:  bbs7    $FF,$B539
LB53A:  bbs7    $FF,$B53C
LB53D:  bbs7    $FF,$B53F
LB540:  bbs7    $FF,$B542
LB543:  bbs7    $FF,$B545
LB546:  bbs7    $FF,$B548
LB549:  bbs7    $FF,$B54B
LB54C:  bbs7    $FF,$B54E
LB54F:  bbs7    $FF,$B551
LB552:  bbs7    $FF,$B554
LB555:  bbs7    $FF,$B557
LB558:  bbs7    $FF,$B55A
LB55B:  bbs7    $FF,$B55D
LB55E:  bbs7    $FF,$B560
LB561:  bbs7    $FF,$B563
LB564:  bbs7    $FF,$B566
LB567:  bbs7    $FF,$B569
LB56A:  bbs7    $FF,$B56C
LB56D:  bbs7    $FF,$B56F
LB570:  bbs7    $FF,$B572
LB573:  bbs7    $FF,$B575
LB576:  bbs7    $FF,$B578
LB579:  bbs7    $FF,$B57B
LB57C:  bbs7    $FF,$B57E
LB57F:  bbs7    $FF,$B581
LB582:  bbs7    $FF,$B584
LB585:  bbs7    $FF,$B587
LB588:  bbs7    $FF,$B58A
LB58B:  bbs7    $FF,$B58D
LB58E:  bbs7    $FF,$B590
LB591:  bbs7    $FF,$B593
LB594:  bbs7    $FF,$B596
LB597:  bbs7    $FF,$B599
LB59A:  bbs7    $FF,$B59C
LB59D:  bbs7    $FF,$B59F
LB5A0:  bbs7    $FF,$B5A2
LB5A3:  bbs7    $FF,$B5A5
LB5A6:  bbs7    $FF,$B5A8
LB5A9:  bbs7    $FF,$B5AB
LB5AC:  bbs7    $FF,$B5AE
LB5AF:  bbs7    $FF,$B5B1
LB5B2:  bbs7    $FF,$B5B4
LB5B5:  bbs7    $FF,$B5B7
LB5B8:  bbs7    $FF,$B5BA
LB5BB:  bbs7    $FF,$B5BD
LB5BE:  bbs7    $FF,$B5C0
LB5C1:  bbs7    $FF,$B5C3
LB5C4:  bbs7    $FF,$B5C6
LB5C7:  bbs7    $FF,$B5C9
LB5CA:  bbs7    $FF,$B5CC
LB5CD:  bbs7    $FF,$B5CF
LB5D0:  bbs7    $FF,$B5D2
LB5D3:  bbs7    $FF,$B5D5
LB5D6:  bbs7    $FF,$B5D8
LB5D9:  bbs7    $FF,$B5DB
LB5DC:  bbs7    $FF,$B5DE
LB5DF:  bbs7    $FF,$B5E1
LB5E2:  bbs7    $FF,$B5E4
LB5E5:  bbs7    $FF,$B5E7
LB5E8:  bbs7    $FF,$B5EA
LB5EB:  bbs7    $FF,$B5ED
LB5EE:  bbs7    $FF,$B5F0
LB5F1:  bbs7    $FF,$B5F3
LB5F4:  bbs7    $FF,$B5F6
LB5F7:  bbs7    $FF,$B5F9
LB5FA:  bbs7    $FF,$B5FC
LB5FD:  bbs7    $FF,$B5FF
LB600:  bbs7    $FF,$B602
LB603:  bbs7    $FF,$B605
LB606:  bbs7    $FF,$B608
LB609:  bbs7    $FF,$B60B
LB60C:  bbs7    $FF,$B60E
LB60F:  bbs7    $FF,$B611
LB612:  bbs7    $FF,$B614
LB615:  bbs7    $FF,$B617
LB618:  bbs7    $FF,$B61A
LB61B:  bbs7    $FF,$B61D
LB61E:  bbs7    $FF,$B620
LB621:  bbs7    $FF,$B623
LB624:  bbs7    $FF,$B626
LB627:  bbs7    $FF,$B629
LB62A:  bbs7    $FF,$B62C
LB62D:  bbs7    $FF,$B62F
LB630:  bbs7    $FF,$B632
LB633:  bbs7    $FF,$B635
LB636:  bbs7    $FF,$B638
LB639:  bbs7    $FF,$B63B
LB63C:  bbs7    $FF,$B63E
LB63F:  bbs7    $FF,$B641
LB642:  bbs7    $FF,$B644
LB645:  bbs7    $FF,$B647
LB648:  bbs7    $FF,$B64A
LB64B:  bbs7    $FF,$B64D
LB64E:  bbs7    $FF,$B650
LB651:  bbs7    $FF,$B653
LB654:  bbs7    $FF,$B656
LB657:  bbs7    $FF,$B659
LB65A:  bbs7    $FF,$B65C
LB65D:  bbs7    $FF,$B65F
LB660:  bbs7    $FF,$B662
LB663:  bbs7    $FF,$B665
LB666:  bbs7    $FF,$B668
LB669:  bbs7    $FF,$B66B
LB66C:  bbs7    $FF,$B66E
LB66F:  bbs7    $FF,$B671
LB672:  bbs7    $FF,$B674
LB675:  bbs7    $FF,$B677
LB678:  bbs7    $FF,$B67A
LB67B:  bbs7    $FF,$B67D
LB67E:  bbs7    $FF,$B680
LB681:  bbs7    $FF,$B683
LB684:  bbs7    $FF,$B686
LB687:  bbs7    $FF,$B689
LB68A:  bbs7    $FF,$B68C
LB68D:  bbs7    $FF,$B68F
LB690:  bbs7    $FF,$B692
LB693:  bbs7    $FF,$B695
LB696:  bbs7    $FF,$B698
LB699:  bbs7    $FF,$B69B
LB69C:  bbs7    $FF,$B69E
LB69F:  bbs7    $FF,$B6A1
LB6A2:  bbs7    $FF,$B6A4
LB6A5:  bbs7    $FF,$B6A7
LB6A8:  bbs7    $FF,$B6AA
LB6AB:  bbs7    $FF,$B6AD
LB6AE:  bbs7    $FF,$B6B0
LB6B1:  bbs7    $FF,$B6B3
LB6B4:  bbs7    $FF,$B6B6
LB6B7:  bbs7    $FF,$B6B9
LB6BA:  bbs7    $FF,$B6BC
LB6BD:  bbs7    $FF,$B6BF
LB6C0:  bbs7    $FF,$B6C2
LB6C3:  bbs7    $FF,$B6C5
LB6C6:  bbs7    $FF,$B6C8
LB6C9:  bbs7    $FF,$B6CB
LB6CC:  bbs7    $FF,$B6CE
LB6CF:  bbs7    $FF,$B6D1
LB6D2:  bbs7    $FF,$B6D4
LB6D5:  bbs7    $FF,$B6D7
LB6D8:  bbs7    $FF,$B6DA
LB6DB:  bbs7    $FF,$B6DD
LB6DE:  bbs7    $FF,$B6E0
LB6E1:  bbs7    $FF,$B6E3
LB6E4:  bbs7    $FF,$B6E6
LB6E7:  bbs7    $FF,$B6E9
LB6EA:  bbs7    $FF,$B6EC
LB6ED:  bbs7    $FF,$B6EF
LB6F0:  bbs7    $FF,$B6F2
LB6F3:  bbs7    $FF,$B6F5
LB6F6:  bbs7    $FF,$B6F8
LB6F9:  bbs7    $FF,$B6FB
LB6FC:  bbs7    $FF,$B6FE
LB6FF:  bbs7    $FF,$B701
LB702:  bbs7    $FF,$B704
LB705:  bbs7    $FF,$B707
LB708:  bbs7    $FF,$B70A
LB70B:  bbs7    $FF,$B70D
LB70E:  bbs7    $FF,$B710
LB711:  bbs7    $FF,$B713
LB714:  bbs7    $FF,$B716
LB717:  bbs7    $FF,$B719
LB71A:  bbs7    $FF,$B71C
LB71D:  bbs7    $FF,$B71F
LB720:  bbs7    $FF,$B722
LB723:  bbs7    $FF,$B725
LB726:  bbs7    $FF,$B728
LB729:  bbs7    $FF,$B72B
LB72C:  bbs7    $FF,$B72E
LB72F:  bbs7    $FF,$B731
LB732:  bbs7    $FF,$B734
LB735:  bbs7    $FF,$B737
LB738:  bbs7    $FF,$B73A
LB73B:  bbs7    $FF,$B73D
LB73E:  bbs7    $FF,$B740
LB741:  bbs7    $FF,$B743
LB744:  bbs7    $FF,$B746
LB747:  bbs7    $FF,$B749
LB74A:  bbs7    $FF,$B74C
LB74D:  bbs7    $FF,$B74F
LB750:  bbs7    $FF,$B752
LB753:  bbs7    $FF,$B755
LB756:  bbs7    $FF,$B758
LB759:  bbs7    $FF,$B75B
LB75C:  bbs7    $FF,$B75E
LB75F:  bbs7    $FF,$B761
LB762:  bbs7    $FF,$B764
LB765:  bbs7    $FF,$B767
LB768:  bbs7    $FF,$B76A
LB76B:  bbs7    $FF,$B76D
LB76E:  bbs7    $FF,$B770
LB771:  bbs7    $FF,$B773
LB774:  bbs7    $FF,$B776
LB777:  bbs7    $FF,$B779
LB77A:  bbs7    $FF,$B77C
LB77D:  bbs7    $FF,$B77F
LB780:  bbs7    $FF,$B782
LB783:  bbs7    $FF,$B785
LB786:  bbs7    $FF,$B788
LB789:  bbs7    $FF,$B78B
LB78C:  bbs7    $FF,$B78E
LB78F:  bbs7    $FF,$B791
LB792:  bbs7    $FF,$B794
LB795:  bbs7    $FF,$B797
LB798:  bbs7    $FF,$B79A
LB79B:  bbs7    $FF,$B79D
LB79E:  bbs7    $FF,$B7A0
LB7A1:  bbs7    $FF,$B7A3
LB7A4:  bbs7    $FF,$B7A6
LB7A7:  bbs7    $FF,$B7A9
LB7AA:  bbs7    $FF,$B7AC
LB7AD:  bbs7    $FF,$B7AF
LB7B0:  bbs7    $FF,$B7B2
LB7B3:  bbs7    $FF,$B7B5
LB7B6:  bbs7    $FF,$B7B8
LB7B9:  bbs7    $FF,$B7BB
LB7BC:  bbs7    $FF,$B7BE
LB7BF:  bbs7    $FF,$B7C1
LB7C2:  bbs7    $FF,$B7C4
LB7C5:  bbs7    $FF,$B7C7
LB7C8:  bbs7    $FF,$B7CA
LB7CB:  bbs7    $FF,$B7CD
LB7CE:  bbs7    $FF,$B7D0
LB7D1:  bbs7    $FF,$B7D3
LB7D4:  bbs7    $FF,$B7D6
LB7D7:  bbs7    $FF,$B7D9
LB7DA:  bbs7    $FF,$B7DC
LB7DD:  bbs7    $FF,$B7DF
LB7E0:  bbs7    $FF,$B7E2
LB7E3:  bbs7    $FF,$B7E5
LB7E6:  bbs7    $FF,$B7E8
LB7E9:  bbs7    $FF,$B7EB
LB7EC:  bbs7    $FF,$B7EE
LB7EF:  bbs7    $FF,$B7F1
LB7F2:  bbs7    $FF,$B7F4
LB7F5:  bbs7    $FF,$B7F7
LB7F8:  bbs7    $FF,$B7FA
LB7FB:  bbs7    $FF,$B7FD
LB7FE:  bbs7    $FF,$B800
LB801:  bbs7    $FF,$B803
LB804:  bbs7    $FF,$B806
LB807:  bbs7    $FF,$B809
LB80A:  bbs7    $FF,$B80C
LB80D:  bbs7    $FF,$B80F
LB810:  bbs7    $FF,$B812
LB813:  bbs7    $FF,$B815
LB816:  bbs7    $FF,$B818
LB819:  bbs7    $FF,$B81B
LB81C:  bbs7    $FF,$B81E
LB81F:  bbs7    $FF,$B821
LB822:  bbs7    $FF,$B824
LB825:  bbs7    $FF,$B827
LB828:  bbs7    $FF,$B82A
LB82B:  bbs7    $FF,$B82D
LB82E:  bbs7    $FF,$B830
LB831:  bbs7    $FF,$B833
LB834:  bbs7    $FF,$B836
LB837:  bbs7    $FF,$B839
LB83A:  bbs7    $FF,$B83C
LB83D:  bbs7    $FF,$B83F
LB840:  bbs7    $FF,$B842
LB843:  bbs7    $FF,$B845
LB846:  bbs7    $FF,$B848
LB849:  bbs7    $FF,$B84B
LB84C:  bbs7    $FF,$B84E
LB84F:  bbs7    $FF,$B851
LB852:  bbs7    $FF,$B854
LB855:  bbs7    $FF,$B857
LB858:  bbs7    $FF,$B85A
LB85B:  bbs7    $FF,$B85D
LB85E:  bbs7    $FF,$B860
LB861:  bbs7    $FF,$B863
LB864:  bbs7    $FF,$B866
LB867:  bbs7    $FF,$B869
LB86A:  bbs7    $FF,$B86C
LB86D:  bbs7    $FF,$B86F
LB870:  bbs7    $FF,$B872
LB873:  bbs7    $FF,$B875
LB876:  bbs7    $FF,$B878
LB879:  bbs7    $FF,$B87B
LB87C:  bbs7    $FF,$B87E
LB87F:  bbs7    $FF,$B881
LB882:  bbs7    $FF,$B884
LB885:  bbs7    $FF,$B887
LB888:  bbs7    $FF,$B88A
LB88B:  bbs7    $FF,$B88D
LB88E:  bbs7    $FF,$B890
LB891:  bbs7    $FF,$B893
LB894:  bbs7    $FF,$B896
LB897:  bbs7    $FF,$B899
LB89A:  bbs7    $FF,$B89C
LB89D:  bbs7    $FF,$B89F
LB8A0:  bbs7    $FF,$B8A2
LB8A3:  bbs7    $FF,$B8A5
LB8A6:  bbs7    $FF,$B8A8
LB8A9:  bbs7    $FF,$B8AB
LB8AC:  bbs7    $FF,$B8AE
LB8AF:  bbs7    $FF,$B8B1
LB8B2:  bbs7    $FF,$B8B4
LB8B5:  bbs7    $FF,$B8B7
LB8B8:  bbs7    $FF,$B8BA
LB8BB:  bbs7    $FF,$B8BD
LB8BE:  bbs7    $FF,$B8C0
LB8C1:  bbs7    $FF,$B8C3
LB8C4:  bbs7    $FF,$B8C6
LB8C7:  bbs7    $FF,$B8C9
LB8CA:  bbs7    $FF,$B8CC
LB8CD:  bbs7    $FF,$B8CF
LB8D0:  bbs7    $FF,$B8D2
LB8D3:  bbs7    $FF,$B8D5
LB8D6:  bbs7    $FF,$B8D8
LB8D9:  bbs7    $FF,$B8DB
LB8DC:  bbs7    $FF,$B8DE
LB8DF:  bbs7    $FF,$B8E1
LB8E2:  bbs7    $FF,$B8E4
LB8E5:  bbs7    $FF,$B8E7
LB8E8:  bbs7    $FF,$B8EA
LB8EB:  bbs7    $FF,$B8ED
LB8EE:  bbs7    $FF,$B8F0
LB8F1:  bbs7    $FF,$B8F3
LB8F4:  bbs7    $FF,$B8F6
LB8F7:  bbs7    $FF,$B8F9
LB8FA:  bbs7    $FF,$B8FC
LB8FD:  bbs7    $FF,$B8FF
LB900:  bbs7    $FF,$B902
LB903:  bbs7    $FF,$B905
LB906:  bbs7    $FF,$B908
LB909:  bbs7    $FF,$B90B
LB90C:  bbs7    $FF,$B90E
LB90F:  bbs7    $FF,$B911
LB912:  bbs7    $FF,$B914
LB915:  bbs7    $FF,$B917
LB918:  bbs7    $FF,$B91A
LB91B:  bbs7    $FF,$B91D
LB91E:  bbs7    $FF,$B920
LB921:  bbs7    $FF,$B923
LB924:  bbs7    $FF,$B926
LB927:  bbs7    $FF,$B929
LB92A:  bbs7    $FF,$B92C
LB92D:  bbs7    $FF,$B92F
LB930:  bbs7    $FF,$B932
LB933:  bbs7    $FF,$B935
LB936:  bbs7    $FF,$B938
LB939:  bbs7    $FF,$B93B
LB93C:  bbs7    $FF,$B93E
LB93F:  bbs7    $FF,$B941
LB942:  bbs7    $FF,$B944
LB945:  bbs7    $FF,$B947
LB948:  bbs7    $FF,$B94A
LB94B:  bbs7    $FF,$B94D
LB94E:  bbs7    $FF,$B950
LB951:  bbs7    $FF,$B953
LB954:  bbs7    $FF,$B956
LB957:  bbs7    $FF,$B959
LB95A:  bbs7    $FF,$B95C
LB95D:  bbs7    $FF,$B95F
LB960:  bbs7    $FF,$B962
LB963:  bbs7    $FF,$B965
LB966:  bbs7    $FF,$B968
LB969:  bbs7    $FF,$B96B
LB96C:  bbs7    $FF,$B96E
LB96F:  bbs7    $FF,$B971
LB972:  bbs7    $FF,$B974
LB975:  bbs7    $FF,$B977
LB978:  bbs7    $FF,$B97A
LB97B:  bbs7    $FF,$B97D
LB97E:  bbs7    $FF,$B980
LB981:  bbs7    $FF,$B983
LB984:  bbs7    $FF,$B986
LB987:  bbs7    $FF,$B989
LB98A:  bbs7    $FF,$B98C
LB98D:  bbs7    $FF,$B98F
LB990:  bbs7    $FF,$B992
LB993:  bbs7    $FF,$B995
LB996:  bbs7    $FF,$B998
LB999:  bbs7    $FF,$B99B
LB99C:  bbs7    $FF,$B99E
LB99F:  bbs7    $FF,$B9A1
LB9A2:  bbs7    $FF,$B9A4
LB9A5:  bbs7    $FF,$B9A7
LB9A8:  bbs7    $FF,$B9AA
LB9AB:  bbs7    $FF,$B9AD
LB9AE:  bbs7    $FF,$B9B0
LB9B1:  bbs7    $FF,$B9B3
LB9B4:  bbs7    $FF,$B9B6
LB9B7:  bbs7    $FF,$B9B9
LB9BA:  bbs7    $FF,$B9BC
LB9BD:  bbs7    $FF,$B9BF
LB9C0:  bbs7    $FF,$B9C2
LB9C3:  bbs7    $FF,$B9C5
LB9C6:  bbs7    $FF,$B9C8
LB9C9:  bbs7    $FF,$B9CB
LB9CC:  bbs7    $FF,$B9CE
LB9CF:  bbs7    $FF,$B9D1
LB9D2:  bbs7    $FF,$B9D4
LB9D5:  bbs7    $FF,$B9D7
LB9D8:  bbs7    $FF,$B9DA
LB9DB:  bbs7    $FF,$B9DD
LB9DE:  bbs7    $FF,$B9E0
LB9E1:  bbs7    $FF,$B9E3
LB9E4:  bbs7    $FF,$B9E6
LB9E7:  bbs7    $FF,$B9E9
LB9EA:  bbs7    $FF,$B9EC
LB9ED:  bbs7    $FF,$B9EF
LB9F0:  bbs7    $FF,$B9F2
LB9F3:  bbs7    $FF,$B9F5
LB9F6:  bbs7    $FF,$B9F8
LB9F9:  bbs7    $FF,$B9FB
LB9FC:  bbs7    $FF,$B9FE
LB9FF:  bbs7    $FF,$BA01
LBA02:  bbs7    $FF,$BA04
LBA05:  bbs7    $FF,$BA07
LBA08:  bbs7    $FF,$BA0A
LBA0B:  bbs7    $FF,$BA0D
LBA0E:  bbs7    $FF,$BA10
LBA11:  bbs7    $FF,$BA13
LBA14:  bbs7    $FF,$BA16
LBA17:  bbs7    $FF,$BA19
LBA1A:  bbs7    $FF,$BA1C
LBA1D:  bbs7    $FF,$BA1F
LBA20:  bbs7    $FF,$BA22
LBA23:  bbs7    $FF,$BA25
LBA26:  bbs7    $FF,$BA28
LBA29:  bbs7    $FF,$BA2B
LBA2C:  bbs7    $FF,$BA2E
LBA2F:  bbs7    $FF,$BA31
LBA32:  bbs7    $FF,$BA34
LBA35:  bbs7    $FF,$BA37
LBA38:  bbs7    $FF,$BA3A
LBA3B:  bbs7    $FF,$BA3D
LBA3E:  bbs7    $FF,$BA40
LBA41:  bbs7    $FF,$BA43
LBA44:  bbs7    $FF,$BA46
LBA47:  bbs7    $FF,$BA49
LBA4A:  bbs7    $FF,$BA4C
LBA4D:  bbs7    $FF,$BA4F
LBA50:  bbs7    $FF,$BA52
LBA53:  bbs7    $FF,$BA55
LBA56:  bbs7    $FF,$BA58
LBA59:  bbs7    $FF,$BA5B
LBA5C:  bbs7    $FF,$BA5E
LBA5F:  bbs7    $FF,$BA61
LBA62:  bbs7    $FF,$BA64
LBA65:  bbs7    $FF,$BA67
LBA68:  bbs7    $FF,$BA6A
LBA6B:  bbs7    $FF,$BA6D
LBA6E:  bbs7    $FF,$BA70
LBA71:  bbs7    $FF,$BA73
LBA74:  bbs7    $FF,$BA76
LBA77:  bbs7    $FF,$BA79
LBA7A:  bbs7    $FF,$BA7C
LBA7D:  bbs7    $FF,$BA7F
LBA80:  bbs7    $FF,$BA82
LBA83:  bbs7    $FF,$BA85
LBA86:  bbs7    $FF,$BA88
LBA89:  bbs7    $FF,$BA8B
LBA8C:  bbs7    $FF,$BA8E
LBA8F:  bbs7    $FF,$BA91
LBA92:  bbs7    $FF,$BA94
LBA95:  bbs7    $FF,$BA97
LBA98:  bbs7    $FF,$BA9A
LBA9B:  bbs7    $FF,$BA9D
LBA9E:  bbs7    $FF,$BAA0
LBAA1:  bbs7    $FF,$BAA3
LBAA4:  bbs7    $FF,$BAA6
LBAA7:  bbs7    $FF,$BAA9
LBAAA:  bbs7    $FF,$BAAC
LBAAD:  bbs7    $FF,$BAAF
LBAB0:  bbs7    $FF,$BAB2
LBAB3:  bbs7    $FF,$BAB5
LBAB6:  bbs7    $FF,$BAB8
LBAB9:  bbs7    $FF,$BABB
LBABC:  bbs7    $FF,$BABE
LBABF:  bbs7    $FF,$BAC1
LBAC2:  bbs7    $FF,$BAC4
LBAC5:  bbs7    $FF,$BAC7
LBAC8:  bbs7    $FF,$BACA
LBACB:  bbs7    $FF,$BACD
LBACE:  bbs7    $FF,$BAD0
LBAD1:  bbs7    $FF,$BAD3
LBAD4:  bbs7    $FF,$BAD6
LBAD7:  bbs7    $FF,$BAD9
LBADA:  bbs7    $FF,$BADC
LBADD:  bbs7    $FF,$BADF
LBAE0:  bbs7    $FF,$BAE2
LBAE3:  bbs7    $FF,$BAE5
LBAE6:  bbs7    $FF,$BAE8
LBAE9:  bbs7    $FF,$BAEB
LBAEC:  bbs7    $FF,$BAEE
LBAEF:  bbs7    $FF,$BAF1
LBAF2:  bbs7    $FF,$BAF4
LBAF5:  bbs7    $FF,$BAF7
LBAF8:  bbs7    $FF,$BAFA
LBAFB:  bbs7    $FF,$BAFD
LBAFE:  bbs7    $FF,$BB00
LBB01:  bbs7    $FF,$BB03
LBB04:  bbs7    $FF,$BB06
LBB07:  bbs7    $FF,$BB09
LBB0A:  bbs7    $FF,$BB0C
LBB0D:  bbs7    $FF,$BB0F
LBB10:  bbs7    $FF,$BB12
LBB13:  bbs7    $FF,$BB15
LBB16:  bbs7    $FF,$BB18
LBB19:  bbs7    $FF,$BB1B
LBB1C:  bbs7    $FF,$BB1E
LBB1F:  bbs7    $FF,$BB21
LBB22:  bbs7    $FF,$BB24
LBB25:  bbs7    $FF,$BB27
LBB28:  bbs7    $FF,$BB2A
LBB2B:  bbs7    $FF,$BB2D
LBB2E:  bbs7    $FF,$BB30
LBB31:  bbs7    $FF,$BB33
LBB34:  bbs7    $FF,$BB36
LBB37:  bbs7    $FF,$BB39
LBB3A:  bbs7    $FF,$BB3C
LBB3D:  bbs7    $FF,$BB3F
LBB40:  bbs7    $FF,$BB42
LBB43:  bbs7    $FF,$BB45
LBB46:  bbs7    $FF,$BB48
LBB49:  bbs7    $FF,$BB4B
LBB4C:  bbs7    $FF,$BB4E
LBB4F:  bbs7    $FF,$BB51
LBB52:  bbs7    $FF,$BB54
LBB55:  bbs7    $FF,$BB57
LBB58:  bbs7    $FF,$BB5A
LBB5B:  bbs7    $FF,$BB5D
LBB5E:  bbs7    $FF,$BB60
LBB61:  bbs7    $FF,$BB63
LBB64:  bbs7    $FF,$BB66
LBB67:  bbs7    $FF,$BB69
LBB6A:  bbs7    $FF,$BB6C
LBB6D:  bbs7    $FF,$BB6F
LBB70:  bbs7    $FF,$BB72
LBB73:  bbs7    $FF,$BB75
LBB76:  bbs7    $FF,$BB78
LBB79:  bbs7    $FF,$BB7B
LBB7C:  bbs7    $FF,$BB7E
LBB7F:  bbs7    $FF,$BB81
LBB82:  bbs7    $FF,$BB84
LBB85:  bbs7    $FF,$BB87
LBB88:  bbs7    $FF,$BB8A
LBB8B:  bbs7    $FF,$BB8D
LBB8E:  bbs7    $FF,$BB90
LBB91:  bbs7    $FF,$BB93
LBB94:  bbs7    $FF,$BB96
LBB97:  bbs7    $FF,$BB99
LBB9A:  bbs7    $FF,$BB9C
LBB9D:  bbs7    $FF,$BB9F
LBBA0:  bbs7    $FF,$BBA2
LBBA3:  bbs7    $FF,$BBA5
LBBA6:  bbs7    $FF,$BBA8
LBBA9:  bbs7    $FF,$BBAB
LBBAC:  bbs7    $FF,$BBAE
LBBAF:  bbs7    $FF,$BBB1
LBBB2:  bbs7    $FF,$BBB4
LBBB5:  bbs7    $FF,$BBB7
LBBB8:  bbs7    $FF,$BBBA
LBBBB:  bbs7    $FF,$BBBD
LBBBE:  bbs7    $FF,$BBC0
LBBC1:  bbs7    $FF,$BBC3
LBBC4:  bbs7    $FF,$BBC6
LBBC7:  bbs7    $FF,$BBC9
LBBCA:  bbs7    $FF,$BBCC
LBBCD:  bbs7    $FF,$BBCF
LBBD0:  bbs7    $FF,$BBD2
LBBD3:  bbs7    $FF,$BBD5
LBBD6:  bbs7    $FF,$BBD8
LBBD9:  bbs7    $FF,$BBDB
LBBDC:  bbs7    $FF,$BBDE
LBBDF:  bbs7    $FF,$BBE1
LBBE2:  bbs7    $FF,$BBE4
LBBE5:  bbs7    $FF,$BBE7
LBBE8:  bbs7    $FF,$BBEA
LBBEB:  bbs7    $FF,$BBED
LBBEE:  bbs7    $FF,$BBF0
LBBF1:  bbs7    $FF,$BBF3
LBBF4:  bbs7    $FF,$BBF6
LBBF7:  bbs7    $FF,$BBF9
LBBFA:  bbs7    $FF,$BBFC
LBBFD:  bbs7    $FF,$BBFF
LBC00:  bbs7    $FF,$BC02
LBC03:  bbs7    $FF,$BC05
LBC06:  bbs7    $FF,$BC08
LBC09:  bbs7    $FF,$BC0B
LBC0C:  bbs7    $FF,$BC0E
LBC0F:  bbs7    $FF,$BC11
LBC12:  bbs7    $FF,$BC14
LBC15:  bbs7    $FF,$BC17
LBC18:  bbs7    $FF,$BC1A
LBC1B:  bbs7    $FF,$BC1D
LBC1E:  bbs7    $FF,$BC20
LBC21:  bbs7    $FF,$BC23
LBC24:  bbs7    $FF,$BC26
LBC27:  bbs7    $FF,$BC29
LBC2A:  bbs7    $FF,$BC2C
LBC2D:  bbs7    $FF,$BC2F
LBC30:  bbs7    $FF,$BC32
LBC33:  bbs7    $FF,$BC35
LBC36:  bbs7    $FF,$BC38
LBC39:  bbs7    $FF,$BC3B
LBC3C:  bbs7    $FF,$BC3E
LBC3F:  bbs7    $FF,$BC41
LBC42:  bbs7    $FF,$BC44
LBC45:  bbs7    $FF,$BC47
LBC48:  bbs7    $FF,$BC4A
LBC4B:  bbs7    $FF,$BC4D
LBC4E:  bbs7    $FF,$BC50
LBC51:  bbs7    $FF,$BC53
LBC54:  bbs7    $FF,$BC56
LBC57:  bbs7    $FF,$BC59
LBC5A:  bbs7    $FF,$BC5C
LBC5D:  bbs7    $FF,$BC5F
LBC60:  bbs7    $FF,$BC62
LBC63:  bbs7    $FF,$BC65
LBC66:  bbs7    $FF,$BC68
LBC69:  bbs7    $FF,$BC6B
LBC6C:  bbs7    $FF,$BC6E
LBC6F:  bbs7    $FF,$BC71
LBC72:  bbs7    $FF,$BC74
LBC75:  bbs7    $FF,$BC77
LBC78:  bbs7    $FF,$BC7A
LBC7B:  bbs7    $FF,$BC7D
LBC7E:  bbs7    $FF,$BC80
LBC81:  bbs7    $FF,$BC83
LBC84:  bbs7    $FF,$BC86
LBC87:  bbs7    $FF,$BC89
LBC8A:  bbs7    $FF,$BC8C
LBC8D:  bbs7    $FF,$BC8F
LBC90:  bbs7    $FF,$BC92
LBC93:  bbs7    $FF,$BC95
LBC96:  bbs7    $FF,$BC98
LBC99:  bbs7    $FF,$BC9B
LBC9C:  bbs7    $FF,$BC9E
LBC9F:  bbs7    $FF,$BCA1
LBCA2:  bbs7    $FF,$BCA4
LBCA5:  bbs7    $FF,$BCA7
LBCA8:  bbs7    $FF,$BCAA
LBCAB:  bbs7    $FF,$BCAD
LBCAE:  bbs7    $FF,$BCB0
LBCB1:  bbs7    $FF,$BCB3
LBCB4:  bbs7    $FF,$BCB6
LBCB7:  bbs7    $FF,$BCB9
LBCBA:  bbs7    $FF,$BCBC
LBCBD:  bbs7    $FF,$BCBF
LBCC0:  bbs7    $FF,$BCC2
LBCC3:  bbs7    $FF,$BCC5
LBCC6:  bbs7    $FF,$BCC8
LBCC9:  bbs7    $FF,$BCCB
LBCCC:  bbs7    $FF,$BCCE
LBCCF:  bbs7    $FF,$BCD1
LBCD2:  bbs7    $FF,$BCD4
LBCD5:  bbs7    $FF,$BCD7
LBCD8:  bbs7    $FF,$BCDA
LBCDB:  bbs7    $FF,$BCDD
LBCDE:  bbs7    $FF,$BCE0
LBCE1:  bbs7    $FF,$BCE3
LBCE4:  bbs7    $FF,$BCE6
LBCE7:  bbs7    $FF,$BCE9
LBCEA:  bbs7    $FF,$BCEC
LBCED:  bbs7    $FF,$BCEF
LBCF0:  bbs7    $FF,$BCF2
LBCF3:  bbs7    $FF,$BCF5
LBCF6:  bbs7    $FF,$BCF8
LBCF9:  bbs7    $FF,$BCFB
LBCFC:  bbs7    $FF,$BCFE
LBCFF:  bbs7    $FF,$BD01
LBD02:  bbs7    $FF,$BD04
LBD05:  bbs7    $FF,$BD07
LBD08:  bbs7    $FF,$BD0A
LBD0B:  bbs7    $FF,$BD0D
LBD0E:  bbs7    $FF,$BD10
LBD11:  bbs7    $FF,$BD13
LBD14:  bbs7    $FF,$BD16
LBD17:  bbs7    $FF,$BD19
LBD1A:  bbs7    $FF,$BD1C
LBD1D:  bbs7    $FF,$BD1F
LBD20:  bbs7    $FF,$BD22
LBD23:  bbs7    $FF,$BD25
LBD26:  .byte   $FF
LBD27:  .byte   $FF
        .byte   $FF
LBD29:  bbs7    $FF,$BD2B
LBD2C:  bbs7    $FF,$BD2E
LBD2F:  bbs7    $FF,$BD31
LBD32:  bbs7    $FF,$BD34
LBD35:  bbs7    $FF,$BD37
LBD38:  bbs7    $FF,$BD3A
LBD3B:  bbs7    $FF,$BD3D
LBD3E:  bbs7    $FF,$BD40
LBD41:  bbs7    $FF,$BD43
LBD44:  bbs7    $FF,$BD46
LBD47:  bbs7    $FF,$BD49
LBD4A:  bbs7    $FF,$BD4C
LBD4D:  bbs7    $FF,$BD4F
LBD50:  bbs7    $FF,$BD52
LBD53:  bbs7    $FF,$BD55
LBD56:  bbs7    $FF,$BD58
LBD59:  bbs7    $FF,$BD5B
LBD5C:  bbs7    $FF,$BD5E
LBD5F:  bbs7    $FF,$BD61
LBD62:  bbs7    $FF,$BD64
LBD65:  bbs7    $FF,$BD67
LBD68:  bbs7    $FF,$BD6A
LBD6B:  bbs7    $FF,$BD6D
LBD6E:  bbs7    $FF,$BD70
LBD71:  bbs7    $FF,$BD73
LBD74:  bbs7    $FF,$BD76
LBD77:  bbs7    $FF,$BD79
LBD7A:  bbs7    $FF,$BD7C
LBD7D:  bbs7    $FF,$BD7F
LBD80:  bbs7    $FF,$BD82
LBD83:  bbs7    $FF,$BD85
LBD86:  bbs7    $FF,$BD88
LBD89:  bbs7    $FF,$BD8B
LBD8C:  bbs7    $FF,$BD8E
LBD8F:  bbs7    $FF,$BD91
LBD92:  bbs7    $FF,$BD94
LBD95:  bbs7    $FF,$BD97
LBD98:  bbs7    $FF,$BD9A
LBD9B:  bbs7    $FF,$BD9D
LBD9E:  bbs7    $FF,$BDA0
LBDA1:  bbs7    $FF,$BDA3
LBDA4:  bbs7    $FF,$BDA6
LBDA7:  bbs7    $FF,$BDA9
LBDAA:  bbs7    $FF,$BDAC
LBDAD:  bbs7    $FF,$BDAF
LBDB0:  bbs7    $FF,$BDB2
LBDB3:  bbs7    $FF,$BDB5
LBDB6:  bbs7    $FF,$BDB8
LBDB9:  bbs7    $FF,$BDBB
LBDBC:  bbs7    $FF,$BDBE
LBDBF:  bbs7    $FF,$BDC1
LBDC2:  bbs7    $FF,$BDC4
LBDC5:  bbs7    $FF,$BDC7
LBDC8:  bbs7    $FF,$BDCA
LBDCB:  bbs7    $FF,$BDCD
LBDCE:  bbs7    $FF,$BDD0
LBDD1:  bbs7    $FF,$BDD3
LBDD4:  bbs7    $FF,$BDD6
LBDD7:  bbs7    $FF,$BDD9
LBDDA:  bbs7    $FF,$BDDC
LBDDD:  bbs7    $FF,$BDDF
LBDE0:  bbs7    $FF,$BDE2
LBDE3:  bbs7    $FF,$BDE5
LBDE6:  bbs7    $FF,$BDE8
LBDE9:  bbs7    $FF,$BDEB
LBDEC:  bbs7    $FF,$BDEE
LBDEF:  bbs7    $FF,$BDF1
LBDF2:  bbs7    $FF,$BDF4
LBDF5:  bbs7    $FF,$BDF7
LBDF8:  bbs7    $FF,$BDFA
LBDFB:  bbs7    $FF,$BDFD
LBDFE:  bbs7    $FF,$BE00
LBE01:  bbs7    $FF,$BE03
LBE04:  bbs7    $FF,$BE06
LBE07:  bbs7    $FF,$BE09
LBE0A:  bbs7    $FF,$BE0C
LBE0D:  bbs7    $FF,$BE0F
LBE10:  bbs7    $FF,$BE12
LBE13:  bbs7    $FF,$BE15
LBE16:  bbs7    $FF,$BE18
LBE19:  bbs7    $FF,$BE1B
LBE1C:  bbs7    $FF,$BE1E
LBE1F:  bbs7    $FF,$BE21
LBE22:  bbs7    $FF,$BE24
LBE25:  bbs7    $FF,$BE27
LBE28:  bbs7    $FF,$BE2A
LBE2B:  bbs7    $FF,$BE2D
LBE2E:  bbs7    $FF,$BE30
LBE31:  bbs7    $FF,$BE33
LBE34:  bbs7    $FF,$BE36
LBE37:  bbs7    $FF,$BE39
LBE3A:  bbs7    $FF,$BE3C
LBE3D:  bbs7    $FF,$BE3F
LBE40:  bbs7    $FF,$BE42
LBE43:  bbs7    $FF,$BE45
LBE46:  bbs7    $FF,$BE48
LBE49:  bbs7    $FF,$BE4B
LBE4C:  bbs7    $FF,$BE4E
LBE4F:  bbs7    $FF,$BE51
LBE52:  bbs7    $FF,$BE54
LBE55:  bbs7    $FF,$BE57
LBE58:  bbs7    $FF,$BE5A
LBE5B:  bbs7    $FF,$BE5D
LBE5E:  bbs7    $FF,$BE60
LBE61:  bbs7    $FF,$BE63
LBE64:  bbs7    $FF,$BE66
LBE67:  bbs7    $FF,$BE69
LBE6A:  bbs7    $FF,$BE6C
LBE6D:  bbs7    $FF,$BE6F
LBE70:  bbs7    $FF,$BE72
LBE73:  bbs7    $FF,$BE75
LBE76:  bbs7    $FF,$BE78
LBE79:  bbs7    $FF,$BE7B
LBE7C:  bbs7    $FF,$BE7E
LBE7F:  bbs7    $FF,$BE81
LBE82:  bbs7    $FF,$BE84
LBE85:  bbs7    $FF,$BE87
LBE88:  bbs7    $FF,$BE8A
LBE8B:  bbs7    $FF,$BE8D
LBE8E:  bbs7    $FF,$BE90
LBE91:  bbs7    $FF,$BE93
LBE94:  bbs7    $FF,$BE96
LBE97:  bbs7    $FF,$BE99
LBE9A:  bbs7    $FF,$BE9C
LBE9D:  bbs7    $FF,$BE9F
LBEA0:  bbs7    $FF,$BEA2
LBEA3:  bbs7    $FF,$BEA5
LBEA6:  bbs7    $FF,$BEA8
LBEA9:  bbs7    $FF,$BEAB
LBEAC:  bbs7    $FF,$BEAE
LBEAF:  bbs7    $FF,$BEB1
LBEB2:  bbs7    $FF,$BEB4
LBEB5:  bbs7    $FF,$BEB7
LBEB8:  bbs7    $FF,$BEBA
LBEBB:  bbs7    $FF,$BEBD
LBEBE:  bbs7    $FF,$BEC0
LBEC1:  bbs7    $FF,$BEC3
LBEC4:  bbs7    $FF,$BEC6
LBEC7:  bbs7    $FF,$BEC9
LBECA:  bbs7    $FF,$BECC
LBECD:  bbs7    $FF,$BECF
LBED0:  bbs7    $FF,$BED2
LBED3:  bbs7    $FF,$BED5
LBED6:  bbs7    $FF,$BED8
LBED9:  bbs7    $FF,$BEDB
LBEDC:  bbs7    $FF,$BEDE
LBEDF:  bbs7    $FF,$BEE1
LBEE2:  bbs7    $FF,$BEE4
LBEE5:  bbs7    $FF,$BEE7
LBEE8:  bbs7    $FF,$BEEA
LBEEB:  bbs7    $FF,$BEED
LBEEE:  bbs7    $FF,$BEF0
LBEF1:  bbs7    $FF,$BEF3
LBEF4:  bbs7    $FF,$BEF6
LBEF7:  bbs7    $FF,$BEF9
LBEFA:  bbs7    $FF,$BEFC
LBEFD:  bbs7    $FF,$BEFF
LBF00:  bbs7    $FF,$BF02
LBF03:  bbs7    $FF,$BF05
LBF06:  bbs7    $FF,$BF08
LBF09:  bbs7    $FF,$BF0B
LBF0C:  bbs7    $FF,$BF0E
LBF0F:  bbs7    $FF,$BF11
LBF12:  bbs7    $FF,$BF14
LBF15:  bbs7    $FF,$BF17
LBF18:  bbs7    $FF,$BF1A
LBF1B:  bbs7    $FF,$BF1D
LBF1E:  bbs7    $FF,$BF20
LBF21:  bbs7    $FF,$BF23
LBF24:  bbs7    $FF,$BF26
LBF27:  bbs7    $FF,$BF29
LBF2A:  bbs7    $FF,$BF2C
LBF2D:  bbs7    $FF,$BF2F
LBF30:  bbs7    $FF,$BF32
LBF33:  bbs7    $FF,$BF35
LBF36:  bbs7    $FF,$BF38
LBF39:  bbs7    $FF,$BF3B
LBF3C:  bbs7    $FF,$BF3E
LBF3F:  bbs7    $FF,$BF41
LBF42:  bbs7    $FF,$BF44
LBF45:  bbs7    $FF,$BF47
LBF48:  bbs7    $FF,$BF4A
LBF4B:  bbs7    $FF,$BF4D
LBF4E:  bbs7    $FF,$BF50
LBF51:  bbs7    $FF,$BF53
LBF54:  bbs7    $FF,$BF56
LBF57:  bbs7    $FF,$BF59
LBF5A:  bbs7    $FF,$BF5C
LBF5D:  bbs7    $FF,$BF5F
LBF60:  bbs7    $FF,$BF62
LBF63:  bbs7    $FF,$BF65
LBF66:  bbs7    $FF,$BF68
LBF69:  bbs7    $FF,$BF6B
LBF6C:  bbs7    $FF,$BF6E
LBF6F:  bbs7    $FF,$BF71
LBF72:  bbs7    $FF,$BF74
LBF75:  bbs7    $FF,$BF77
LBF78:  bbs7    $FF,$BF7A
LBF7B:  bbs7    $FF,$BF7D
LBF7E:  bbs7    $FF,$BF80
LBF81:  bbs7    $FF,$BF83
LBF84:  bbs7    $FF,$BF86
LBF87:  bbs7    $FF,$BF89
LBF8A:  bbs7    $FF,$BF8C
LBF8D:  bbs7    $FF,$BF8F
LBF90:  bbs7    $FF,$BF92
LBF93:  bbs7    $FF,$BF95
LBF96:  bbs7    $FF,$BF98
LBF99:  bbs7    $FF,$BF9B
LBF9C:  bbs7    $FF,$BF9E
LBF9F:  bbs7    $FF,$BFA1
LBFA2:  bbs7    $FF,$BFA4
LBFA5:  bbs7    $FF,$BFA7
LBFA8:  bbs7    $FF,$BFAA
LBFAB:  bbs7    $FF,$BFAD
LBFAE:  bbs7    $FF,$BFB0
LBFB1:  bbs7    $FF,$BFB3
LBFB4:  bbs7    $FF,$BFB6
LBFB7:  bbs7    $FF,$BFB9
LBFBA:  bbs7    $FF,$BFBC
LBFBD:  bbs7    $FF,$BFBF
LBFC0:  bbs7    $FF,$BFC2
LBFC3:  bbs7    $FF,$BFC5
LBFC6:  bbs7    $FF,$BFC8
LBFC9:  bbs7    $FF,$BFCB
LBFCC:  bbs7    $FF,$BFCE
LBFCF:  bbs7    $FF,$BFD1
LBFD2:  bbs7    $FF,$BFD4
LBFD5:  bbs7    $FF,$BFD7
LBFD8:  bbs7    $FF,$BFDA
LBFDB:  bbs7    $FF,$BFDD
LBFDE:  bbs7    $FF,$BFE0
LBFE1:  bbs7    $FF,$BFE3
LBFE4:  bbs7    $FF,$BFE6
LBFE7:  bbs7    $FF,$BFE9
LBFEA:  bbs7    $FF,$BFEC
LBFED:  bbs7    $FF,$BFEF
LBFF0:  bbs7    $FF,$BFF2
LBFF3:  bbs7    $FF,$BFF5
LBFF6:  bbs7    $FF,$BFF8
LBFF9:  bbs7    $FF,$BFFB
LBFFC:  bbs7    $FF,$BFFE
LBFFF:  bbs7    L0020,$C04D
        bra     $C024
        bbr3    $82,$C027
        asl     $2082
        .byte   $2B
        bra     $C02D
        tma     #$82
        jsr     L81D4
        jsr     L83B8
        jsr     L8054
        jsr     L3172
        jsr     L810B
        jsr     LE07B
        jsr     L82F6
        jsr     L82C1
        jsr     L3B16
        rts

        lda     #$00
        jsr     LE06C
        lda     #$00
        ldx     #$20
        ldy     #$1E
        jsr     LE06F
        jsr     LE078
        jsr     LE081
        jsr     LE099
        lda     #$10
        jsr     LE09C
        jsr     LE07B
        rts

        sei
        stz     $F5
        rmb7    $F3
        rmb6    $F3
        cli
        rts

        clc
        lda     #$02
        adc     $FFF5
        tam     #$04
        bsr     LC066
        clc
        cla
        adc     $FFF5
        tam     #$04
        rts

LC066:  sei
        stz     $27D9
        lda     #$00
        sta     $27DA
        lda     #$08
        sta     $27DB
        stz     $220C
        stz     $220D
        stz     $2210
        stz     $2211
        bsr     LC084
        cli
        rts

LC084:  st0     #$00
        st1     #$00
        st2     #$00
        st0     #$02
        ldx     #$04
LC08E:  cly
LC08F:  st1     #$00
        st2     #$01
        st1     #$00
        st2     #$01
        dey
        bne     LC08F
        dex
        bne     LC08E
        st0     #$00
        lda     $27DA
        sta     a:$02
        lda     $27DB
        sta     a:$03
        st0     #$02
        clx
LC0AE:  st1     #$00
        st2     #$00
        st1     #$00
        st2     #$00
        dex
        bne     LC0AE
        st0     #$00
        st1     #$00
        st2     #$10
        .byte   $03
LC0C0:  sxy
        .byte   $E3
        .byte   $EB
        bra     LC0C7
        brk
LC0C6:  .byte   $20
LC0C7:  brk
        stz     $0402
        stz     $0403
        ldx     #$20
LC0D0:  tia     $80EB,$0404,$0020
        dex
        bne     LC0D0
        st0     #$05
        lda     #$88
        sta     $F3
        sta     a:$02
        lda     #$00
        sta     $F4
        sta     a:$03
        rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        lda     #$01
        jsr     LE069
        rts

        sei
        st0     #$05
        rmb7    $F3
        lda     $F3
        sta     a:$02
        lda     $F4
        sta     a:$03
        lda     #$00
        sta     $0E
        lda     #$08
        sta     $0F
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        lda     #$00
        sta     L0000
        lda     #$57
        sta     $01
LC13E:  stz     a:$02
        stz     a:$03
        lda     L0000
        bne     LC14A
        dec     $01
LC14A:  dec     L0000
        lda     L0000
        ora     $01
        bne     LC13E
        lda     #$00
        sta     $0E
        lda     #$00
        sta     $0F
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        lda     #$70
        sta     $04
        lda     #$05
        sta     $05
        lda     #$80
        sta     L0000
        lda     #$00
        sta     $01
LC178:  lda     L0000
        sta     a:$02
        lda     $01
        sta     a:$03
        inc     L0000
        bne     LC188
        inc     $01
LC188:  lda     $04
        bne     LC18E
        dec     $05
LC18E:  dec     $04
        lda     $04
        ora     $05
        bne     LC178
        lda     #$80
        sta     L0000
        lda     #$00
        sta     $01
        lda     #$90
        sta     $04
        lda     #$02
        sta     $05
LC1A6:  lda     L0000
        sta     a:$02
        lda     $01
        sta     a:$03
        inc     L0000
        bne     LC1B6
        inc     $01
LC1B6:  lda     $04
        bne     LC1BC
        dec     $05
LC1BC:  dec     $04
        lda     $04
        ora     $05
        bne     LC1A6
        st0     #$05
        smb7    $F3
        lda     $F3
        sta     a:$02
        lda     $F4
        sta     a:$03
        cli
        rts

        lda     #$80
        sta     $FB
        lda     #$19
        sta     $F8
        lda     #$02
        sta     $FC
        jsr     LE006
        rts

        pha
        jsr     LE07B
        lda     #$00
        sta     $0402
        lda     #$01
        sta     $0403
        tia     $8281,$0404,$0020
        tia     $82A1,$0404,$0020
        stz     $0402
        stz     $0403
        stz     $0404
        stz     $0405
        pla
        rts

        ldy     #$64
        clx
LC211:  stz     L0000,x
        inx
        dey
        bne     LC211
        lda     #$80
        sta     L0000
        lda     #$27
        sta     $01
        lda     #$80
        sta     $02
        lda     #$00
        sta     $03
LC227:  cla
        sta     (L0000)
        inc     L0000
        bne     LC230
        inc     $01
LC230:  lda     $02
        bne     LC236
        dec     $03
LC236:  dec     $02
        lda     $02
        ora     $03
        bne     LC227
        rts

        stz     $0801
        rts

        lda     $2700
LC246:  sta     L8257
        lda     $2701
        .byte   $8D
        cli
LC24E:  clx
        .byte   $73
        rmb5    $82
        bra     $C27B
LC254:  .byte   $20
        brk
LC256:  rts

        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC25E:  brk
        brk
        brk
        brk
        brk
        brk
        brk
LC265:  brk
LC266:  brk
        brk
        brk
        brk
        brk
LC26B:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC276:  brk
        brk
        brk
        brk
        brk
        brk
LC27C:  brk
        brk
LC27E:  brk
        brk
        brk
        brk
        brk
        bbs7    $01,LC246
        ora     ($FF,x)
        ora     ($07,x)
        brk
        bbs7    $01,LC24E
LC28E:  ora     ($FF,x)
        ora     (L0000,x)
        brk
        bbs7    $01,LC256
        ora     ($FF,x)
        ora     ($07,x)
        brk
        bbs7    $01,LC25E
        ora     ($FF,x)
        ora     (L0000,x)
        brk
        bbs7    $01,LC266
        ora     ($FF,x)
        ora     ($18,x)
        brk
        bbs7    $01,LC28E
        brk
        bbs7    $01,LC2B2
LC2B2:  brk
        bbs7    $01,LC276
        ora     ($FF,x)
        ora     (L0038,x)
        brk
        bbs7    $01,LC27E
        ora     ($FF,x)
        ora     ($A9,x)
        bbr7    $8D,LC340
        bmi     LC254
        jmp     (L8D30,x)

        adc     L8D30,x
        ror     L8D30,x
        bbr7    $30,LC27C
        bbs7    $8D,LC265
        bmi     LC34B
        bbs0    $30,LC26B
        bmi     LC2F0
        brk
        cla
        clx
LC2E0:  sta     $30A3,x
        inc     a
        inx
        cpx     #$14
        bne     LC2E0
        stz     $307A
        lda     $3008
        .byte   $8D
LC2F0:  ora     #$30
        sta     $300A
        rts

        jsr     LE05A
        cpx     #$03
        bcc     LC302
        jsr     LE0DE
        bcc     LC329
LC302:  lda     #$30
        sta     $0C
        lda     #$83
        sta     $0D
        lda     #$00
        sta     L4FDB
        lda     #$A0
        sta     L4FDC
        lda     #$00
        sta     L4FD7
        lda     #$60
        sta     L4FD8
        jsr     L5E27
LC321:  cla
        clx
        sei
        jsr     L56AF
        bra     LC321
LC329:  jsr     LE0DE
        sta     $3008
        rts

        sxy
        brk
        asl     a
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        asl     a
        ora     ($12,x)
        asl     $010A,x
LC340:  php
        sxy
        ora     ($04,x)
        rmb1    L0000
        brk
        sxy
        jsr     L2020
LC34B:  jsr     L4120
        csl
        csl
        eor     $4E
        csl
        eor     #$4F
        lsr     $0121
        csl
        pla
        adc     #$73
        jsr     L6964
        tii     $2063,$6E6F,$796C
        jsr     L6F77
        adc     ($6B)
        tii     $6F20,$016E,$6874
        adc     L0020
        tam     #$55
        bvc     LC3BD
        eor     (L0020)
        tma     #$44
        and     L4F52
        eor     $2032
        tam     #$59
        tam     #$54
        eor     $4D
        rol     $2001
        ora     (L0020,x)
        jsr     L2020
        jsr     L5020
        jmp     (L6165)

        tii     $2065,$7375,$0165
        stz     $68,x
        adc     L0020
        tam     #$55
        bvc     LC3EA
        eor     (L0020)
        tma     #$44
        and     L4F52
        eor     $2032
        tam     #$59
        tam     #$54
        eor     $4D
        rol     a:L0020
        lda     #$04
        sta     $0400
LC3BD:  clx
LC3BE:  lda     L83D8,x
        sta     a:L0000
        inx
        lda     L83D8,x
        sta     a:$02
        inx
        lda     L83D8,x
        sta     a:$03
        inx
        cpx     #$24
        bcc     LC3BE
        rts

        asl     L0000
        brk
        rmb0    L0000
        brk
        php
        brk
        brk
        ora     #$00
        brk
        asl     a
        sxy
        sxy
        .byte   $0B
        .byte   $1F
        .byte   $04
LC3EA:  tsb     $0F02
        ora     a:$EF
        asl     a:$04
        bbr0    $10,LC3F6
LC3F6:  st1     #$00
        php
        ora     $08
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC558:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC600:  brk
        brk
        brk
        brk
        brk
        brk
LC606:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC644:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC6C6:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
LC6D6:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk

; da65 V2.18 - N/A
; Created:    2026-07-12 09:14:04
; Input file: /tmp/theron-disasm/theron-us-ipl.bin
; Page:       1


        .setcpu "huc6280"

L0018           := $0018
L0020           := $0020
L0038           := $0038
L003A           := $003A
L0040           := $0040
L0044           := $0044
L004C           := $004C
L0066           := $0066
L0202           := $0202
L0810           := $0810
L09FF           := $09FF
L0C3C           := $0C3C
L1010           := $1010
L1800           := $1800
L180C           := $180C
L1838           := $1838
L1F9F           := $1F9F
L2000           := $2000
L2020           := $2020
L2045           := $2045
L204C           := $204C
L21C2           := $21C2
L2828           := $2828
L2838           := $2838
L3418           := $3418
L3700           := $3700
L3828           := $3828
L387C           := $387C
L3A4C           := $3A4C
L6000           := $6000
L6006           := $6006
L6008           := $6008
L6072           := $6072
L6C6C           := $6C6C
L7638           := $7638
L7C00           := $7C00
L7C38           := $7C38
L9FDF           := $9FDF
LC07C           := $C07C
LC0C0           := $C0C0
LC0C2           := $C0C2
LC0C6           := $C0C6
LC27C           := $C27C
LC2C2           := $C2C2
LC2C6           := $C2C6
LC2FC           := $C2FC
LC4F8           := $C4F8
LC600           := $C600
LC606           := $C606
LC644           := $C644
LC6C6           := $C6C6
LE009           := $E009
LE00F           := $E00F
LE02D           := $E02D
LE048           := $E048
LE04B           := $E04B
LE04E           := $E04E
LE051           := $E051
LE054           := $E054
LE057           := $E057
LE05A           := $E05A
LE063           := $E063
LE069           := $E069
LE07B           := $E07B
LE0D8           := $E0D8
LFC00           := $FC00
LFE00           := $FE00
LFEFE           := $FEFE
        stz     $00
        tii     $2000,$2001,$000F
        tii     $2000,$2700,$0080
        cla
        jsr     LE02D
        lda     #$01
        sta     $FF
        jsr     LE0D8
        sei
        stz     $F5
        cli
        sei
        lda     $F3
        and     #$3F
        sta     $F3
        st0     #$05
        sta     a:$02
        lda     $F4
        sta     a:$03
        cli
        clx
        ldy     #$02
        stz     $0402
        stz     $0403
L403A:  stz     $0404
        stz     $0405
L4040:  dex
        bne     L403A
        dey
        bne     L403A
        jsr     LE05A
        cpx     #$03
        bcc     L4050
        jsr     L40E3
L4050:  lda     $FFF5
        tam     #$08
        tii     $4000,$6000,$00E3
        tam     #$04
        inc     a
        tam     #$08
        inc     a
        tam     #$10
        inc     a
        tam     #$20
        inc     a
        tam     #$40
        bsr     L40A9
        bra     L4080
        clc
        lda     $24
        adc     #$01
        sta     $24
        cla
        adc     $23
        sta     $23
        cla
        adc     $22
        sta     $22
        rts

L4080:  clx
        lda     L40D5,x
        sta     $FC
        inx
        lda     L40D5,x
        sta     $FE
        inx
        lda     L40D5,x
        sta     $FD
        inx
        lda     L40D5,x
        sta     $F8
        lda     #$00
        sta     $FA
        lda     #$40
        sta     $FB
        lda     #$01
        sta     $FF
        jsr     LE00F
        bra     L4080
L40A9:  clx
        lda     L40DC,x
        sta     $FC
        inx
        lda     L40DC,x
        sta     $FE
        inx
        lda     L40DC,x
        sta     $FD
        inx
        lda     L40DC,x
        sta     $F8
        lda     #$00
        sta     $FA
        lda     #$30
        sta     $FB
        lda     #$01
        sta     $FF
        jsr     LE009
        cmp     #$00
        bne     L40A9
        rts

L40D5:  brk
        smb6    $03
        ora     ($00),y
        .byte   $FC
        .byte   $83
L40DC:  brk
        .byte   $E3
        st0     #$02
        brk
        .byte   $2B
        .byte   $0D
L40E3:  jsr     L4143
        bcs     L40E9
        rts

L40E9:  lda     #$01
        jsr     LE069
        jsr     L44CD
        jsr     L447C
        jsr     L4438
        jsr     L4B4D
        stz     L517B
        .byte   $9C
        .byte   $5B
L40FF:  rmb4    $A9
        brk
        sta     L476C
        lda     #$30
        sta     L476D
        lda     #$85
        sta     L476A
        lda     #$53
        sta     L476B
        lda     #$00
        sta     L4757
        lda     #$01
        sta     L5179
        jsr     L42D1
        bcs     L413F
        beq     L413F
        jsr     L4512
        bpl     L413F
        jsr     L45C4
        jsr     L42C7
        jsr     L41B9
        jsr     L476F
        jsr     L4B3B
        jsr     L4B2F
        jsr     L44CD
L413F:  jsr     L4179
        rts

L4143:  lda     #$AC
        sta     $F8
L4147:  .byte   $A9
L4148:  eor     ($85,x)
        sbc     $19A9,y
        sta     $FA
        lda     #$53
        sta     $FB
L4153:  lda     #$99
        sta     $FC
        lda     #$01
        sta     $FD
        lda     #$00
        sta     $FE
        lda     #$00
        sta     $FF
        jsr     LE04E
        cmp     #$00
        beq     L4177
        lda     #$AC
        sta     $F8
        lda     #$41
        sta     $F9
        jsr     LE054
        sec
        rts

L4177:  clc
        rts

L4179:  jsr     L4512
        bmi     L41AB
        stz     L5180
        tii     $5180,$5181,$0198
        lda     #$AC
        sta     $F8
        lda     #$41
        sta     $F9
        lda     #$80
        sta     $FA
        lda     #$51
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
L41AB:  rts

        brk
        .byte   $0B
        bsr     L41FD
        tam     #$2D
        tam     #$47
        rol     $3030
        .byte   $31
L41B8:  brk
L41B9:  jsr     L4783
        jsr     L451E
        jsr     LE063
        lda     $2228
        cmp     L41B8
        beq     L41B9
        sta     L41B8
        cmp     #$40
        beq     L4203
        cmp     #$10
        beq     L41DE
        cmp     #$01
        bne     L41DC
        jmp     L4273

L41DC:  bra     L41B9
L41DE:  lda     L5179
        cmp     #$01
        beq     L41B9
        lda     L517C
        beq     L422B
        dec     L5179
        dec     L517C
        jsr     L4539
        sec
        lda     L517D
        sbc     #$02
        sta     L517D
        .byte   $20
L41FD:  sta     $45
        bsr     L426A
        bra     L41B9
L4203:  lda     L5179
        cmp     L5178
        beq     L41B9
        lda     L517C
        cmp     #$08
        beq     L4259
        inc     L5179
        inc     L517C
        jsr     L4539
        clc
        lda     L517D
        adc     #$02
        sta     L517D
        jsr     L4585
        bsr     L426A
        bra     L41B9
L422B:  jsr     L4539
        lda     L5179
        pha
        sec
        sbc     #$09
        sta     L5179
        jsr     L42C7
        pla
        dec     a
        sta     L5179
        ldx     #$08
        stx     L517C
L4245:  clc
        lda     L517D
        adc     #$02
        sta     L517D
        dex
        bne     L4245
        jsr     L4585
        bsr     L426A
        jmp     L41B9

L4259:  jsr     L4539
        inc     L5179
        jsr     L42C7
        jsr     L4585
        bsr     L426A
        jmp     L41B9

L426A:  lda     #$08
        sta     L45C2
        stz     L45C3
        rts

L4273:  jsr     L4585
        jsr     L4674
        lda     L4694
        bne     L4284
        jsr     L45CE
        jmp     L41B9

L4284:  lda     #$19
        sta     $00
        lda     #$53
        sta     $01
        lda     L517C
        jsr     L43FB
        lda     $00
        sta     $F8
        lda     $01
        sta     $F9
        jsr     LE054
        cmp     #$00
        beq     L42A4
        jmp     L41B9

L42A4:  jsr     L42D1
        bcs     L42BC
        lda     #$01
        sta     L5179
        jsr     L42C7
        jsr     L4512
        bpl     L42BC
        jsr     L45CE
        jmp     L41B9

L42BC:  jsr     L45DD
        rts

        jsr     L4539
        jsr     L45FF
        rts

L42C7:  jsr     L4317
        jsr     L4378
        jsr     L426A
        rts

L42D1:  stz     L5178
        lda     #$19
        sta     $FA
        lda     #$53
        sta     $FB
        lda     #$FF
        sta     $F8
        jsr     LE057
        cmp     #$FF
        beq     L42EE
        lda     $F8
        sta     L5178
        clc
        rts

L42EE:  bsr     L42F2
        sec
        rts

L42F2:  lda     #$0C
        sta     $F8
        lda     #$43
        sta     $F9
        jsr     LE048
        jsr     L476F
        lda     #$0C
        sta     $F8
        lda     #$43
        sta     $F9
        jsr     LE048
        rts

        and     ($42,x)
        eor     L4620
        bbr4    $52,L4361
        eor     ($54,x)
        .byte   $21
L4317:  lda     #$20
        sta     L5319
        tii     $5319,$531A,$006B
        lda     L5179
        pha
        stz     L517A
        lda     #$0D
        sta     $00
        lda     #$53
        sta     $01
        cly
        ldx     #$09
L4335:  phx
        phy
        jsr     L436C
        jsr     L435D
        jsr     L4366
        jsr     LE057
        inc     L5179
        ply
        plx
        iny
        dex
        beq     L4350
        cmp     #$00
        beq     L4335
L4350:  cmp     #$00
        beq     L4355
        dey
L4355:  sty     L517A
        pla
        sta     L5179
        rts

L435D:  lda     $00
        sta     $FA
L4361:  lda     $01
        sta     $FB
        rts

L4366:  lda     L5179
        sta     $F8
        rts

L436C:  clc
        lda     $00
        adc     #$0C
        sta     $00
        bcc     L4377
        inc     $01
L4377:  rts

L4378:  lda     #$09
        sta     L517F
        sta     L517D
        stz     L517C
        ldx     #$09
L4385:  bsr     L438B
        stz     L517C
        rts

L438B:  phx
        lda     #$08
        sta     L517E
        jsr     L43A5
        clc
        lda     L517F
        adc     #$02
        sta     L517F
        inc     L517C
        plx
        dex
        bne     L4385
        rts

L43A5:  lda     #$19
        sta     $00
        lda     #$53
        sta     $01
        lda     L517C
        bsr     L43FB
        clc
        lda     $00
        adc     #$02
        sta     $00
        bcc     L43BD
        inc     $01
L43BD:  lda     #$DF
        sta     $02
        lda     #$4B
        sta     $03
        ldx     #$0A
L43C7:  bsr     L43CD
        dex
        bne     L43C7
        rts

L43CD:  bsr     L441E
        lda     ($00)
        sec
        sbc     #$20
        asl     a
        tay
        sei
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        lda     ($02),y
        sta     a:$02
        iny
        lda     ($02),y
        sta     a:$03
        cli
        inc     L517E
        inc     $00
        bne     L43FA
        inc     $01
L43FA:  rts

L43FB:  sta     $02
        stz     $03
        lda     #$0C
        sta     $04
        ldx     #$04
L4405:  lsr     $04
        bcc     L4416
        clc
        lda     $00
        adc     $02
        sta     $00
        lda     $01
        adc     $03
        sta     $01
L4416:  asl     $02
        rol     $03
        dex
        bne     L4405
        rts

L441E:  stz     $0E
        lda     L517F
        lsr     a
        ror     $0E
        lsr     a
        ror     $0E
        sta     $0F
        clc
        lda     $0E
        adc     L517E
        sta     $0E
        bcc     L4437
        inc     $0F
L4437:  rts

L4438:  lda     #$61
        sta     $00
        lda     #$4C
        sta     $01
        sei
        st0     #$00
        st1     #$00
        st2     #$20
        st0     #$02
        ldx     #$29
L444B:  phx
        bsr     L445F
        clc
        lda     $00
        adc     #$08
        sta     $00
        bcc     L4459
        inc     $01
L4459:  plx
        dex
        bne     L444B
        cli
        rts

L445F:  cly
        ldx     #$08
L4462:  lda     ($00),y
        sta     a:$02
        sta     a:$03
        iny
        dex
        bne     L4462
        ldx     #$08
        lda     #$FF
L4472:  sta     a:$02
        sta     a:$03
        dex
        bne     L4472
        rts

L447C:  lda     #$CC
        sta     $0402
        lda     #$00
        sta     $0403
        lda     #$00
        sta     $0404
        lda     #$00
        sta     $0405
        lda     #$CF
        sta     $0402
        lda     #$00
        sta     $0403
        lda     #$FF
        sta     $0404
        lda     #$01
        sta     $0405
        lda     #$DC
        sta     $0402
        lda     #$00
        sta     $0403
        lda     #$07
        sta     $0404
        lda     #$00
        sta     $0405
        lda     #$DF
        sta     $0402
        lda     #$00
        sta     $0403
        lda     #$FF
        sta     $0404
        lda     #$01
        sta     $0405
        rts

L44CD:  sei
        st0     #$05
        lda     $F3
        and     #$3F
        sta     $F3
        sta     a:$02
        st0     #$00
        st1     #$00
        st2     #$00
        st0     #$02
        ldx     #$00
        ldy     #$80
L44E5:  st1     #$00
        st2     #$00
        dex
        bne     L44E5
        dey
        bne     L44E5
        st0     #$00
        st1     #$00
        st2     #$00
        st0     #$02
        ldx     #$00
        ldy     #$08
L44FB:  st1     #$00
        st2     #$01
        dex
        bne     L44FB
        dey
        bne     L44FB
        st0     #$05
        lda     $F3
        ora     #$80
        sta     $F3
        sta     a:$02
        cli
        rts

L4512:  jsr     LE04B
        lda     $FC
        cmp     #$9B
        lda     $FD
        sbc     #$01
        rts

L451E:  dec     L45C2
        beq     L4524
        rts

L4524:  lda     #$08
        sta     L45C2
        lda     L45C3
        eor     #$01
        sta     L45C3
        beq     L4536
        bsr     L4539
        rts

L4536:  bsr     L4585
        rts

L4539:  bsr     L4576
        ldx     #$0A
L453D:  phx
        sei
        st0     #$01
        .byte   $A5
L4542:  asl     $028D
        brk
        lda     $0F
        sta     a:$03
        st0     #$02
        ldx     a:$02
        ldy     a:$03
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        tya
        and     #$CF
        stx     a:$02
        sta     a:$03
        cli
        inc     $0E
        bne     L4571
        inc     $0F
L4571:  plx
        dex
        bne     L453D
        rts

L4576:  lda     #$08
        sta     L517E
        lda     L517D
        sta     L517F
        jsr     L441E
        rts

L4585:  bsr     L4576
        ldx     #$0A
L4589:  phx
        sei
        st0     #$01
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        ldx     a:$02
        ldy     a:$03
        st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        tya
        ora     #$D0
        stx     a:$02
        sta     a:$03
        cli
        inc     $0E
        bne     L45BD
        inc     $0F
L45BD:  plx
        dex
        bne     L4589
        rts

L45C2:  brk
L45C3:  brk
L45C4:  jsr     L4ABC
        cla
        jsr     L4619
        jsr     L465C
L45CE:  jsr     L4ABC
        lda     #$01
        jsr     L4619
        jsr     L465C
        jsr     L4AD2
        rts

L45DD:  jsr     L4ABC
        lda     #$03
        jsr     L4619
        jsr     L465C
        lda     #$04
        jsr     L4619
        jsr     L465C
        lda     #$05
        jsr     L4619
        jsr     L465C
        jsr     L4AD2
        jsr     L4AD2
        rts

L45FF:  jsr     L4ABC
        lda     #$06
        jsr     L4619
        jsr     L465C
        lda     #$07
        jsr     L4619
        jsr     L465C
        jsr     L4AD2
        jsr     L4AD2
        rts

L4619:  asl     a
        asl     a
        tay
        lda     #$3C
        sta     $00
L4620:  lda     #$46
        sta     $01
        lda     ($00),y
        sta     L4768
        iny
        lda     ($00),y
        sta     L4769
        iny
        lda     ($00),y
        sta     L4748
        iny
        lda     ($00),y
        sta     L4749
        rts

        sta     ($50,x)
        ora     ($02,x)
        ldx     $0150,y
        ora     $DD
        bvc     L4648
        .byte   $05
L4648:  .byte   $FC
        bvc     L464C
        sxy
L464C:  .byte   $FC
        bvc     L4650
        .byte   $03
L4650:  .byte   $1B
        eor     ($01),y
        ora     L003A
        eor     ($01),y
        sxy
        eor     $0151,y
        .byte   $05
L465C:  lda     #$1E
        sta     L474C
        lda     #$0F
        sta     L4746
        lda     #$0C
        sta     L4747
        lda     #$02
        sta     L4750
        jsr     L479E
        rts

L4674:  jsr     L4ABC
        lda     #$02
        jsr     L4619
        jsr     L465C
        jsr     L4AD2
        lda     L517E
        pha
        jsr     L46FF
        jsr     L4696
        jsr     L4722
        pla
        sta     L517E
        rts

L4694:  brk
L4695:  brk
L4696:  jsr     L4B3B
        stz     L4694
        stz     L4695
L469F:  jsr     L4783
        jsr     LE063
        lda     $2228
        cmp     L4695
        beq     L469F
        sta     L4695
        cmp     #$80
        beq     L46BF
        cmp     #$20
        beq     L46E0
        cmp     #$01
        beq     L46BE
        bra     L469F
L46BE:  rts

L46BF:  lda     #$19
        sta     L517E
        jsr     L441E
        ldx     #$03
        jsr     L453D
        lda     #$15
        sta     L517E
        jsr     L441E
        ldx     #$03
        jsr     L4589
        lda     #$FF
        sta     L4694
        bra     L469F
L46E0:  lda     #$15
        sta     L517E
        jsr     L441E
        ldx     #$03
        jsr     L453D
        lda     #$19
        sta     L517E
        jsr     L441E
        ldx     #$03
        jsr     L4589
        stz     L4694
        bra     L469F
L46FF:  lda     #$38
        sta     $00
        lda     #$47
        sta     $01
        lda     #$15
        sta     L517E
        ldx     #$07
L470E:  jsr     L43CD
        dex
        bne     L470E
        lda     #$19
        sta     L517E
        jsr     L441E
        ldx     #$03
        jsr     L4589
        rts

L4722:  lda     #$3F
        sta     $00
        lda     #$47
        sta     $01
        lda     #$15
        sta     L517E
        ldx     #$07
L4731:  jsr     L43CD
        dex
        bne     L4731
        rts

        eor     L5345,y
        jsr     L4F4E
        jsr     L2020
        jsr     L2020
        .byte   $20
        .byte   $20
L4746:  brk
L4747:  brk
L4748:  brk
L4749:  brk
L474A:  brk
L474B:  brk
L474C:  brk
L474D:  brk
L474E:  brk
L474F:  brk
L4750:  brk
L4751:  brk
L4752:  brk
L4753:  brk
L4754:  brk
L4755:  brk
        brk
L4757:  brk
        brk
        brk
        brk
L475B:  brk
L475C:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L4768:  brk
L4769:  brk
L476A:  brk
L476B:  brk
L476C:  brk
L476D:  brk
L476E:  brk
L476F:  pha
        phx
        phy
        lda     #$03
L4774:  clx
L4775:  cly
L4776:  dey
        bne     L4776
        dex
        bne     L4775
        dec     a
        bne     L4774
        ply
        plx
        pla
        rts

L4783:  pha
        phx
        phy
        lda     #$01
        ldx     L4757
L478B:  cly
L478C:  dey
        bne     L478C
L478F:  dex
        bne     L478B
        dec     a
        bne     L478B
        ply
        plx
        pla
        rts

L4799:  lda     ($00)
        sta     $F8
        rts

L479E:  lda     L4768
        sta     $00
        lda     L4769
        sta     $01
        lda     L4746
        sta     $10
        lda     L4747
        sta     $11
        lda     L4748
        sta     L474A
        lda     L4749
        sta     L474B
        lda     L474C
        sta     $04
        sta     L474F
        lda     L4750
        sta     L4751
        stz     L4872
L47CF:  bsr     L4799
        jsr     L4847
        bcc     L47CF
        jsr     L4835
        bne     L47DC
        rts

L47DC:  jsr     L4838
        bcc     L47CF
        jsr     L4856
        bcc     L47CF
        jsr     L4863
        bcc     L47CF
        lda     L4872
        beq     L4800
        lda     L4757
        pha
        lda     #$0A
        sta     L4757
        jsr     L4783
        pla
        sta     L4757
L4800:  jsr     L4873
        bsr     L482E
        inc     L4748
        dec     $04
        beq     L480F
        jmp     L47CF

L480F:  jsr     L4799
        bsr     L4835
        bne     L4817
        rts

L4817:  bsr     L4838
        bcc     L47CF
        bsr     L4847
        bcc     L4828
        bsr     L4856
        bcc     L4828
        jsr     L4863
        bcc     L4828
L4828:  jsr     L488F
        jmp     L47CF

L482E:  inc     $00
        bne     L4834
        inc     $01
L4834:  rts

L4835:  lda     $F8
        rts

L4838:  lda     $F8
        cmp     #$01
        bne     L4845
        jsr     L488F
        bsr     L482E
L4843:  clc
        rts

L4845:  sec
        rts

L4847:  lda     $F8
        cmp     #$04
        bne     L4854
        jsr     L4935
        bsr     L482E
        clc
        rts

L4854:  sec
        rts

L4856:  lda     $F8
        cmp     #$02
        bne     L4854
        stz     L4872
        bsr     L482E
        clc
        rts

L4863:  lda     $F8
        cmp     #$03
        bne     L4854
        lda     #$FF
        sta     L4872
        bsr     L482E
        clc
        rts

L4872:  brk
L4873:  lda     $00
        pha
        lda     $01
        pha
        lda     $04
        pha
        lda     $05
        pha
        jsr     L4973
        pla
        sta     $05
        pla
        sta     $04
        pla
        sta     $01
        pla
        sta     $00
        rts

L488F:  dec     L4751
        bne     L4896
        bsr     L48A5
L4896:  lda     L474F
        sta     $04
        inc     L4749
        lda     L474A
        sta     L4748
        rts

L48A5:  jsr     L4AD2
        jsr     L4ABC
        bsr     L48BB
        lda     L4750
        sta     L4751
        lda     L474B
        dec     a
        sta     L4749
        rts

L48BB:  bsr     L4927
        lda     L474C
        pha
        lda     L474D
        pha
        bsr     L4915
        lda     L476E
        bne     L48D0
        bsr     L4920
        bra     L48D8
L48D0:  lda     #$03
L48D2:  jsr     L476F
        dec     a
        bne     L48D2
L48D8:  pla
        sta     L474D
        pla
        sta     L474C
        lda     #$00
        sta     $06
        lda     #$10
        sta     $07
        lda     L474A
        sta     L4748
        lda     L474B
        sta     L4749
        jsr     L49BE
        ldx     L474C
        ldy     L474D
L48FD:  phx
        lda     $0E
        pha
        lda     $0F
        pha
        jsr     L4B19
        pla
        sta     $0F
        pla
        sta     $0E
        jsr     L4966
        plx
        dey
        bne     L48FD
        rts

L4915:  lda     #$01
        sta     L4754
        lda     #$01
        sta     L4755
        rts

L4920:  jsr     L4B2F
        jsr     L4B3B
        rts

L4927:  lda     L4748
        dec     a
        sta     L4752
        lda     L4749
        sta     L4753
        rts

L4935:  bsr     L4927
        lda     L474C
        pha
        lda     L474D
        pha
        bsr     L4915
        bsr     L4920
        pla
        .byte   $8D
        .byte   $4D
L4946:  rmb4    $68
        sta     L474C
        rts

L494C:  stz     $0E
        lda     L4749
        lsr     a
        ror     $0E
        lsr     a
        ror     $0E
        sta     $0F
        clc
        lda     $0E
        adc     L4748
        sta     $0E
        bcc     L4965
        inc     $0F
L4965:  rts

L4966:  clc
        lda     $0E
        adc     #$40
        sta     $0E
        bcc     L4971
        inc     $0F
L4971:  rts

        brk
L4973:  lda     L476D
        cmp     #$7F
        bcc     L4981
        bne     L4981
        lda     L476C
        cmp     #$C1
L4981:  bcs     L499F
        lda     L476C
        pha
        lda     L476D
        pha
        lda     L474E
        sta     $FF
        bsr     L49D1
        pla
        sta     $07
        pla
        sta     $06
        bsr     L49BE
        sei
        bsr     L49A0
        cli
        clc
L499F:  rts

L49A0:  jsr     L4AFB
        jsr     L4AF0
        rts

L49A7:  jsr     L4AFB
        jsr     L4AF0
        clc
        lda     $06
        adc     #$02
        sta     $06
        bcc     L49B8
        inc     $07
L49B8:  bsr     L4966
        dey
        bne     L49A7
        rts

L49BE:  jsr     L494C
        ldx     #$04
L49C3:  lsr     $07
        ror     $06
        dex
        bne     L49C3
        lda     $07
        ora     #$F0
        sta     $07
        rts

L49D1:  lda     L476A
        sta     $FA
        lda     L476B
        sta     $FB
        jsr     L4BAE
        bsr     L49F3
        ldx     #$03
        bsr     L4A19
        clc
        lda     L476C
        adc     #$10
        sta     L476C
        bcc     L49F2
        inc     L476D
L49F2:  rts

L49F3:  lda     L476C
        sta     $06
        lda     L476D
        sta     $07
        clc
        lda     L476A
        adc     #$20
        sta     $04
        cla
        adc     L476B
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

L4A19:  lda     $14,x
        clc
        adc     L476A
        sta     $00
        cla
        adc     L476B
        sta     $01
        bsr     L4A4D
        rts

L4A2A:  lda     $11
        pha
        ldx     #$03
L4A2F:  phx
        lda     $14,x
        tay
        lsr     $11
        bcc     L4A3B
        lda     #$FF
        bra     L4A3C
L4A3B:  cla
L4A3C:  ldx     #$08
L4A3E:  sta     ($04),y
        iny
        iny
        dex
        bne     L4A3E
        plx
        dex
        bpl     L4A2F
        pla
        sta     $11
        rts

L4A4D:  lda     $10
        pha
        bsr     L4A2A
        ldx     #$03
L4A54:  phx
        lsr     $10
        bcc     L4A5F
        lda     #$FF
        sta     $0E
        bra     L4A61
L4A5F:  stz     $0E
L4A61:  lda     $14,x
        tax
        cly
        lda     #$08
L4A67:  pha
        lda     ($00),y
        iny
        iny
        sxy
        lsr     $0E
        bcc     L4A77
        ora     ($04),y
        sta     ($04),y
        bra     L4A7D
L4A77:  eor     #$FF
        and     ($04),y
        sta     ($04),y
L4A7D:  sxy
        inx
        inx
        pla
        dec     a
        bne     L4A67
        plx
        dex
        bpl     L4A54
        sei
        st0     #$00
        lda     $06
        sta     a:$02
        lda     $07
        sta     a:$03
        st0     #$02
        cly
        ldx     #$10
        bsr     L4AAC
        clc
        lda     $06
        adc     #$10
        sta     $06
        bcc     L4AA7
        inc     $07
L4AA7:  cli
        pla
        sta     $10
        rts

L4AAC:  lda     ($04),y
        iny
        sta     a:$02
        lda     ($04),y
        iny
        sta     a:$03
        dex
        bne     L4AAC
        rts

L4ABC:  lda     L475B
        asl     a
        tax
        lda     L476C
        sta     L475C,x
        inx
        lda     L476D
        sta     L475C,x
        inc     L475B
        rts

L4AD2:  dec     L475B
        lda     L475B
        asl     a
        tax
        lda     L475C,x
        sta     L476C
        inx
        lda     L475C,x
        sta     L476D
        rts

L4AE8:  sei
        jsr     L4AFB
        bsr     L4AF0
        cli
        rts

L4AF0:  lda     $06
        sta     a:$02
L4AF5:  lda     $07
        sta     a:$03
        rts

L4AFB:  st0     #$00
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        rts

        st0     #$01
        lda     $0E
        sta     a:$02
        lda     $0F
        sta     a:$03
        st0     #$02
        rts

L4B19:  bsr     L4AE8
        bsr     L4B21
        dex
L4B1E:  bne     L4B19
        rts

L4B21:  inc     $0E
        bne     L4B27
        inc     $0F
L4B27:  rts

        inc     $06
        bne     L4B2E
        inc     $07
L4B2E:  rts

L4B2F:  jsr     L4783
        jsr     LE063
        lda     $2228
        beq     L4B2F
        rts

L4B3B:  pha
        phx
        phy
L4B3E:  jsr     L4783
        jsr     LE063
        lda     $2228
        bne     L4B3E
        ply
        plx
        pla
L4B4C:  rts

L4B4D:  pha
        phx
        phy
        jsr     LE07B
        cly
        lda     L4B6C,y
        sta     $0402
        iny
        lda     L4B6C,y
        sta     $0403
        tia     $4B6E,$0404,$0040
        ply
        plx
        pla
        rts

L4B6C:  cpx     #$00
        inc     $2401
        brk
        bbs7    $01,L4AF5
        ora     ($EB,x)
        brk
        cpx     #$00
        bbs7    $00,L4B7D
L4B7D:  ora     ($1F,x)
        ora     ($77,x)
        ora     ($E0,x)
        brk
        .byte   $AB
        ora     ($07,x)
        brk
        bbs1    $00,L4B4C
        brk
        bbs7    $01,L4B1E
        brk
        ldy     $B701,x
        ora     ($01,x)
        brk
        tst     #$00,$22
        brk
        .byte   $DC
        ora     ($02,x)
        brk
        tst     #$00,$FF
        ora     ($08,x)
        brk
        bbs7    $01,L4BA7
L4BA7:  brk
        lda     $1001,y
        brk
        .byte   $FF
        .byte   $01
L4BAE:  phx
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
        adc     #$A9
        sta     $F8
        lda     $F9
        adc     #$4D
        sta     $F9
        ldy     #$07
        ldx     #$0E
L4BD1:  lda     ($F8),y
        sxy
        sta     ($FA),y
        sxy
        dex
        dex
        dey
        bpl     L4BD1
        ply
        plx
        rts

        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
L4BE9:  brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        ora     ($C2,x)
        sxy
        cly
        st0     #$C2
        tsb     $C2
        ora     $C2
        asl     $C2
        rmb0    $C2
        php
        cly
        ora     #$C2
        asl     a
        cly
        .byte   $0B
        cly
        tsb     $0DC2
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        asl     $0FC2
        cly
        bpl     L4BE9
        ora     ($C2),y
        ora     ($C2)
        st1     #$C2
        trb     $C2
        ora     $C2,x
        asl     $C2,x
        rmb1    $C2
L4C35:  clc
        cly
        ora     $1AC2,y
        cly
        .byte   $1B
L4C3C:  cly
        trb     $1DC2
        cly
        asl     $1FC2,x
        cly
        jsr     L21C2
        cly
        sax
        cly
        .byte   $23
L4C4C:  cly
        bit     $C2
        and     $C2
        rol     $C2
        rmb2    $C2
        brk
        cly
        brk
        cly
        brk
        cly
        brk
        cly
        plp
        cly
        brk
        cly
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        inc     a:$00,x
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        trb     a:$1C
        sxy
        tsb     $08
        bpl     L4C9F
        rti

        bra     L4CFE
        cly
        cly
        cly
        cly
        cly
        cly
        jmp     (L1838,x)

        clc
        clc
        clc
        clc
        clc
        clc
        jmp     (L0202,x)

        jmp     (LC0C0,x)

        cpy     #$7C
        jmp     (L0202,x)

        jmp     (L0202,x)

L4C9F:  sxy
        jmp     (LC2C2,x)

        cly
        jmp     (L0202,x)

        sxy
        sxy
        jmp     (L4040,x)

        jmp     (L0202,x)

        sxy
        jmp     (LC07C,x)

        cpy     #$7C
        cly
        cly
        cly
        jmp     (LC27C,x)

        sxy
        sxy
        sxy
        sxy
        sxy
        sxy
        jmp     (LC2C2,x)

        jmp     (LC2C2,x)

        cly
        jmp     (LC27C,x)

        cly
        jmp     (L0202,x)

        sxy
        jmp     (L3418,x)

        cla
        cly
        inc     LC2C2,x
        cly
        .byte   $FC
        cly
        cly
        .byte   $FC
        cly
        cly
        cly
        inc     LC27C,x
        cpy     #$C0
        cpy     #$C0
        cly
        jmp     (LC4F8,x)

        cly
        cly
        cly
        cly
        cpy     $F8
        inc     LC0C0,x
        .byte   $FC
        cpy     #$C0
        cpy     #$FE
        inc     LC0C0,x
        .byte   $FE
        .byte   $C0
L4CFE:  cpy     #$C0
        cpy     #$7C
        cly
        cpy     #$C0
        dec     LC2C2
        ror     LC2C2,x
        cly
        inc     LC2C2,x
        cly
        cly
        bit     $1818,x
        clc
        clc
L4D16:  clc
        clc
        bit     $0CFE,x
        tsb     $0C0C
        cpy     $78CC
        cly
        cpy     $C8
        bne     L4D16
        cld
        cpy     LC0C6
        cpy     #$C0
        cpy     #$C0
        cpy     #$C0
        inc     $E6C2,x
        inc     $DA
        phx
        cly
        cly
        cly
        cly
        cly
        .byte   $E2
        cmp     ($CA)
        dec     $C2
        cly
        jmp     (LC2C2,x)

        cly
        cly
        cly
        cly
        jmp     (LC2FC,x)

        cly
        cly
        inc     LC0C0,x
        cpy     #$7C
        cly
        cly
        cly
        phx
        inc     $C2
        adc     LC2FC,x
        cly
        cly
        .byte   $FC
        cly
        cly
        cly
        jmp     (LC0C2,x)

        cpy     #$7C
        asl     $C2
        jmp     (LFEFE,x)

        clc
        clc
        clc
        clc
        clc
        clc
        cly
        cly
        cly
        cly
        cly
        cly
        cla
        bit     LC2C2,x
        cly
        cly
        cly
        stz     $34
        clc
        cly
        cly
        phx
        phx
        phx
        phx
        phx
        stz     $C2
        cly
        cly
        stz     L0038
        clc
        jmp     (LC2C6)

        cly
        cly
        stz     L0038
        bmi     L4DC8
L4D98:  bmi     L4D98
        sxy
        tsb     $08
        bpl     L4DBF
        rti

        inc     a:$00,x
        brk
        brk
        brk
        brk
        brk
        inc     a:$00,x
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
        bvc     L4E0B
        brk
        brk
        brk
        brk
L4DBF:  brk
        brk
        brk
        plp
        jmp     (L2828,x)

        .byte   $7C
        plp
L4DC8:  brk
        bpl     L4E03
        csl
        sec
        trb     $54
        sec
        bpl     L4E14
        ldy     $48
        bpl     L4DFA
        lsr     a
        sty     $00
        sec
        jmp     (L7638)

        cpy     $7ACC
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
        bpl     L4E17
        rti

        brk
        .byte   $10
L4DFA:  sta     ($54)
        sec
        csl
        sta     ($10)
        brk
        bpl     L4E13
L4E03:  bpl     L4E03
        bpl     L4E17
        bpl     L4E09
L4E09:  brk
        brk
L4E0B:  brk
        brk
        brk
        rts

        jsr     L0040
        brk
L4E13:  brk
L4E14:  inc     a:$00,x
L4E17:  brk
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
        bpl     L4E46
        rti

        bra     L4E29
L4E29:  sec
L4E2A:  stz     $C6
        dec     $C6
        jmp     L0038

        clc
        sec
        clc
        clc
        clc
        clc
        bit     L7C00,x
        dec     $C6
        trb     $E070
        inc     LFE00,x
        tsb     $3C18
L4E45:  .byte   $06
L4E46:  dec     $7C
        brk
L4E49:  trb     $6C3C
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
        bpl     L4E98
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
        bpl     L4E8C
        tsb     $08
L4E8C:  bpl     L4E96
        tsb     $02
        brk
        brk
        brk
        jmp     (L7C00,x)

L4E96:  brk
        brk
L4E98:  brk
        rti

        jsr     L0810
        bpl     L4EBF
        rti

        brk
        bit     $6666,x
        tsb     a:L0018
        clc
        brk
        sec
        bsr     L4E46
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
L4EBF:  .byte   $FC
        brk
        bit     $C066,x
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
L4EDE:  cpy     #$C0
        brk
        bit     $C066,x
        dec     $66C6
        bit     LC600,x
        dec     $C6
        inc     LC6C6,x
        dec     $00
        .byte   $FC
        bmi     L4F24
L4EF4:  bmi     L4F26
        bmi     L4EF4
        brk
        rol     $0C0C,x
        tsb     $CC0C
        sei
        brk
        dec     $CC
        cld
        beq     L4EDE
        cpy     a:$C6
        rts

        rts

        rts

        rts

        rts

        rts

        ror     LC600,x
        dec     $EE
        inc     $C6D6,x
        dec     $00
        dec     $E6
        inc     $DE,x
        dec     LC6C6
        brk
        .byte   $7C
L4F22:  dec     $C6
L4F24:  dec     $C6
L4F26:  dec     $7C
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
        dec     $00
        jmp     (LC0C6,x)

        jmp     (LC606,x)

        jmp     (LFC00,x)

        bmi     L4F7C
        bmi     L4F7E
L4F4E:  bmi     L4F80
        brk
        dec     $C6
        dec     $C6
        dec     $C6
        .byte   $7C
        brk
L4F59:  dec     $C6
        dec     $6C
        jmp     ($1038,x)

        brk
L4F61:  dec     $C6
        dec     $D6
        jmp     (L6C6C,x)

        brk
        dec     $EE
        jmp     (L7C38,x)

        inc     a:$C6
        cpy     $CCCC
        sei
        bmi     L4FA7
        bmi     L4F79
L4F79:  inc     $1C0E,x
L4F7C:  sec
        .byte   $70
L4F7E:  cpx     #$FE
L4F80:  brk
        tsb     $0808
        php
        php
        php
        tsb     LC600
        jmp     (L7C38)

        bpl     L500B
        bpl     L4F91
L4F91:  bmi     L4FA3
        bpl     L4FA5
        bpl     L4FA7
        bmi     L4F99
L4F99:  brk
        brk
        brk
        bpl     L4FC6
        bsr     L4F22
        brk
        brk
        brk
L4FA3:  brk
        brk
L4FA5:  brk
        brk
L4FA7:  inc     a:$00,x
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

L4FC6:  stz     L0038
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
        bmi     L500C
        .byte   $FC
        bmi     L500F
        bmi     L4FE1
L4FE1:  brk
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
        bmi     L4FF4
L4FF4:  bmi     L5026
        bmi     L5028
        brk
        brk
        clc
        brk
        clc
        clc
        clc
        cli
        bmi     L5062
        rts

        ror     $6C
        sei
        jmp     (L0066)

        clc
        clc
L500B:  clc
L500C:  clc
        clc
        clc
L500F:  clc
        brk
        brk
        jmp     (L5656)

        lsr     $56,x
        lsr     $00,x
        brk
        sei
        jmp     L4C4C

        jmp     L004C

        brk
        sei
        cpy     $CCCC
L5026:  .byte   $CC
        sei
L5028:  brk
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
        bmi     L50C8
        bmi     L507E
        bmi     L5068
        brk
        brk
        .byte   $4C
L5053:  jmp     L4C4C

        jmp     L003A

        brk
        bsr     L50A0
        jmp     (L3828)

        bpl     L5061
L5061:  brk
L5062:  dec     $C6
        csl
        jmp     (L2838,x)

L5068:  brk
        brk
        bsr     L50D8
        sec
        sec
        jmp     (L0044)

        brk
        say
        ror     $34
        clc
        clc
        bmi     L50D9
        brk
        jmp     (L180C,x)

        .byte   $30
L507E:  rts

        jmp     (L2000,x)

        jsr     L4F59
        eor     L0020,x
        bsr     L50D8
        lsr     L5427
        jsr     L4148
        lsr     $45,x
        jsr     L4E45
        bbr4    $55,L50DE
        pha
        jsr     L5053
        eor     ($43,x)
        eor     L0020
        .byte   $20
L50A0:  jsr     L2020
        jsr     L4E49
        jsr     L4854
        eor     L0020
        lsr     $49
        jmp     L2045

        tma     #$41
        say
        eor     #$4E
        eor     $54
        and     (L0020,x)
        jsr     L2020
        jsr     L2000
        jsr     L4843
        bbr4    $4F,L5118
        eor     L0020
        .byte   $41
L50C8:  jsr     L4946
        jmp     L2045

        csl
        bbr4    L0020,L5116
        eor     L004C
        eor     $54
        eor     $2E
L50D8:  .byte   $20
L50D9:  jsr     L2020
        brk
        .byte   $20
L50DE:  jsr     L2020
        jsr     L2020
        jsr     L2020
        tam     #$55
        eor     ($45)
        bbr3    L0020,L510E
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2000
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        .byte   $20
        .byte   $20
L510E:  jsr     L2020
        jsr     L2020
        .byte   $20
        .byte   $20
L5116:  .byte   $20
        .byte   $20
L5118:  jsr     L0020
        jsr     L2020
        jsr     L2020
        jsr     L5420
        pha
        eor     ($4E,x)
        .byte   $4B
        jsr     L4F59
        eor     $2E,x
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        brk
        jsr     L5420
        pha
        eor     #$53
        jsr     L4147
        eor     L2045
        rmb5    $49
        jmp     L204C

        lsr     L544F
        jsr     L4542
        jsr     L4153
        lsr     $45,x
        bsr     L5179
        brk
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        jsr     L2020
        brk
L5178:  brk
L5179:  brk
L517A:  brk
L517B:  brk
L517C:  brk
L517D:  brk
L517E:  brk
L517F:  brk
L5180:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L5319:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L5345:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L5420:  brk
        brk
        brk
        brk
        brk
        brk
        brk
L5427:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L544F:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L5656:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
L57C1:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        ora     $D803
        smb4    L0038
        clv
        and     ($B0),y
        tii     $6370,$731F,$6370
        rts

        brk
        sbc     ($C0,x)
        brk
        smb0    $01
        bbs0    $00,L5828
        brk
        brk
        st0     #$0F
        brk
        bbr1    $00,L57C1
        cpy     #$57
        smb6    $D8
L5825:  st0     #$A8
        .byte   $03
L5828:  lda     $F101
        sbc     $012C,x
        ldy     a:$01
        bbs0    $07,L5874
        .byte   $FB
        rts

L5836:  .byte   $FB
        beq     L5836
        beq     L583C
        .byte   $F0
L583C:  sbc     $FDF0,x
        beq     L5843
        brk
        sxy
L5843:  brk
        sxy
        brk
        sxy
        brk
L5848:  tsb     $00
        tsb     $00
        tsb     $00
        tsb     $00
        asl     $06
        asl     $06
L5854:  asl     $06
        asl     $06
        tsb     $0C0C
        tsb     $0C0C
        tsb     $0E0C
        asl     $0F07,x
        st0     #$07
        ora     ($03,x)
        brk
        ora     ($00,x)
        brk
        brk
        brk
        brk
        brk
        ora     ($03,x)
        brk
        .byte   $01
L5874:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        bra     L5882
L5882:  rti

        bbr3    $A0,L5825
        bne     L5848
        inx
        cpx     #$74
        beq     L58C8
        sei
        ora     $803C,x
        bbs7    $C0,L5854
        rts

        cpx     #$30
        bvs     L58B1
        sec
        tsb     $061C
        bbr0    $03,L58A7
        brk
        brk
        brk
        bra     L58A5
L58A5:  bra     L58A7
L58A7:  brk
        brk
        brk
        brk
        brk
        inc     $FF00,x
        brk
        brk
L58B1:  bra     L58B3
L58B3:  brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        inc     $FF00,x
        pha
L58C1:  bvs     L5912
        rmb7    $A4
L58C5:  .byte   $3B
        ldy     $3B
L58C8:  ldy     L0038
        .byte   $D3
        ora     $1DD2,x
        cmp     ($1D)
L58D0:  bra     $58D9
        smb0    $00
        tdd     $C300,$C000,$E103
L58DB:  brk
        .byte   $E1
L58DD:  brk
        sbc     ($00,x)
        bbr7    $BF,L5922
        bbs5    L0020,L58C5
        jsr     L9FDF
        bbs6    $9F,L58DB
        bcc     L58DD
        pha
        rmb7    $3F
        brk
        bbr1    $00,L5914
        brk
        bbr1    $00,L5908
        brk
        bbr0    $00,L590C
        brk
        smb0    $00
        sbc     $D8E3
        smb4    L0038
        clv
        and     ($B0),y
L5908:  .byte   $73
        bvs     L596E
        .byte   $1F
L590C:  .byte   $73
        .byte   $70
L590E:  .byte   $63
        rts

        cpx     #$01
L5912:  cpy     #$00
L5914:  smb0    $01
        bbs0    $00,L5928
        brk
        brk
        st0     #$0F
        brk
        bbr1    $00,L58D0
        .byte   $CF
L5922:  rmb5    $E7
        cld
        st0     #$A8
        .byte   $03
L5928:  lda     $F101
        sbc     $012C,x
        ldy     $0F01
        bra     L593A
        rti

        .byte   $FB
        rts

L5936:  .byte   $FB
        beq     L5936
        .byte   $F0
L593A:  ora     ($F0,x)
        sbc     $FDF0,x
        beq     L58C1
        brk
        bra     L5944
L5944:  bbs7    $FF,L59C6
        .byte   $5F
L5948:  lsr     a
        eor     L5948,y
        pha
        eor     L5948,y
        bbs7    L0040,$5952
        .byte   $7F
        brk
L5955:  brk
        jsr     L3700
        and     ($37),y
        .byte   $33
        rmb3    $33
        rmb3    $33
        ora     ($00,x)
        ora     ($00,x)
        bbs7    $FF,L5955
        cpy     $99CC
        cpy     $CC99
        .byte   $99
L596E:  cpy     $FF99
        brk
L5972:  bbs7    $FE,L5975
L5975:  brk
        .byte   $33
        ora     ($77),y
        .byte   $33
        rmb7    $33
        rmb7    $33
        rmb7    $33
        brk
L5981:  brk
        brk
        brk
        brk
        brk
        bbr1    $00,L5989
L5989:  .byte   $FF
        .byte   $3F
L598B:  bbs7    $80,L590E
        brk
        bbs7    $00,L5992
L5992:  brk
        brk
        brk
        brk
        brk
        brk
        .byte   $FF
        brk
L599A:  bbs7    $00,$591D
        bbr7    $00,$59A0
        brk
        brk
        brk
L59A3:  brk
        brk
        brk
        sed
        brk
        brk
        .byte   $FF
        .byte   $FC
L59AB:  bbs7    $01,$59AF
        brk
        bbs7    $00,L59B2
L59B2:  brk
        brk
        brk
        brk
        brk
        brk
        .byte   $FF
        brk
L59BA:  bbs7    $00,$59BE
        inc     a:$00,x
        ora     $D8E3
        .byte   $07
L59C4:  sec
        sec
L59C6:  lda     ($B0),y
        tii     $6370,$731F,$6370
        rts

        brk
        sbc     ($00,x)
        cpy     #$07
        sta     ($8F,x)
        brk
        bbr0    $00,L59DB
L59DB:  st0     #$0F
        brk
        bbr1    $00,L5981
        bbs4    $57,L59C4
        cld
        brk
        .byte   $AB
        st0     #$AD
        brk
        sbc     ($FC),y
        ldy     $AD00
        ora     ($00,x)
        bbs0    $00,L5A3B
        sed
L59F5:  .byte   $63
        .byte   $FB
        beq     L59F5
        .byte   $F1
L59FA:  brk
        sbc     ($FC),y
        sbc     ($FD),y
        beq     L5A64
        rts

        adc     ($70),y
        adc     ($1F,x)
        sei
L5A07:  sei
        sec
        sec
        ldy     $18BC,x
        smb4    $0F
        cpx     #$1F
        brk
        bbr0    $00,L5A15
L5A15:  ora     ($07,x)
        brk
        rmb0    $80
        tst     #$00,$C0
L5A1D:  brk
        cpx     #$00
        bit     $2D00
        ora     ($F0,x)
        sbc     $05F4,x
        rts

        brk
        st0     #$03
        cpy     #$F7
        cpx     #$0F
        .byte   $FC
        sbc     ($FD),y
        beq     L5A36
        .byte   $F0
L5A36:  sbc     $F800,y
        st0     #$FB
L5A3B:  brk
        rmb0    $C0
        bbr0    $00,L5A49
        brk
        brk
        bpl     L5A45
L5A45:  jsr     L478F
        asl     a
L5A49:  sxy
        trb     $181C
        clc
        bpl     L5A60
        clc
        clc
        brk
        clc
        php
        bit     $1F,x
        smb6    $1E
        cla
        bit     $181C,x
        sec
        bpl     L5A70
L5A60:  bbs1    $EF,L5AB2
        .byte   $77
L5A64:  pha
        rmb7    $48
        rmb7    $4F
        rmb7    $4F
        rmb7    $A4
        .byte   $3B
        ldy     $3B
L5A70:  bbr0    $00,L59FA
        brk
        smb0    $00
        smb0    $00
        smb0    $00
        smb0    $00
        .byte   $C3
        brk
        .byte   $C3
        brk
        .byte   $FF
        brk
L5A82:  brk
        brk
        bbs7    $FF,L5A07
        bbr7    $FF,L5A8A
L5A8A:  bra     L5A8C
L5A8C:  bbr7    $BF,L5ACF
        .byte   $BF
        brk
L5A91:  bbs7    $00,$5A93
        .byte   $FF
        brk
L5A96:  bbr7    $00,L5A99
L5A99:  bbr7    $00,L5B1B
        bbr3    $00,L5ADE
        brk
L5AA0:  inc     $FC01,x
        st0     #$04
        st0     #$FC
        .byte   $FB
        sbc     $F907,y
        rmb0    $09
        rmb0    $F2
        inc     LFC00
L5AB2:  brk
        sed
        brk
        sed
        sed
        brk
        brk
        beq     L5ABB
L5ABB:  beq     L5ABD
L5ABD:  beq     L5AA0
        brk
        bbs7    $00,L5AC3
L5AC3:  brk
        bbs7    $FF,L5AC8
        .byte   $FE
L5AC8:  bbs7    $00,L5ACC
        brk
L5ACC:  inc     $02FD,x
L5ACF:  sbc     $FF00,x
        brk
        bbs7    $FF,L5AD6
L5AD6:  inc     a:$00,x
        inc     LFE00,x
        .byte   $FC
        brk
L5ADE:  .byte   $FC
        brk
        and     $1C
        sbc     $DC
        bit     $DC
        .byte   $4B
        clv
        lsr     a
        sec
        .byte   $CB
        clv
        .byte   $4B
        clv
        .byte   $4B
        clv
        st0     #$C0
        tdd     $C300,$8700,$0700
        bra     L5A82
        brk
        smb0    $00
        smb0    $00
        sbc     $F2F7,y
        inc     $EE12
        ora     ($EE)
        sbc     ($EE)
        sbc     ($EE)
        and     $DC
        and     $DC
        beq     L5B12
L5B12:  sbc     ($00,x)
        sbc     ($00,x)
        sbc     ($00,x)
        sbc     ($00,x)
        .byte   $E1
L5B1B:  brk
        tdd     $C300,$FC00,$04FB
        .byte   $FB
        .byte   $FC
        st0     #$FC
        st0     #$F9
        .byte   $07
L5B2A:  ora     #$07
L5B2C:  sbc     $09F7,y
        rmb0    $F8
        sed
        sed
        sed
        brk
        sed
        brk
        sed
        beq     L5B2A
        beq     L5B2C
        brk
        beq     L5B3F
L5B3F:  brk
        pha
        eor     L5948,y
        pha
        eor     L5948,y
        bbr7    $5D,L5BCA
        brk
        rti

        brk
        bbr7    $7F,L5B88
        .byte   $33
        rmb3    $33
        rmb3    $33
        rmb3    $33
        sax
        brk
        bbr7    $00,L5BDC
        brk
        brk
        brk
        cpy     $CC99
        sta     $99CC,y
        cpy     $FF99
        cmp     a:$FF,x
        ora     ($00,x)
        bbs7    $FF,L5BE8
        .byte   $33
        rmb7    $33
        rmb7    $33
        rmb7    $33
        sax
        brk
        .byte   $FF
        brk
L5B7C:  bbs7    $00,$5B7F
        brk
        cmp     ($33)
        rts

        bcc     L5BAE
        cpy     #$16
        rts

L5B88:  bbs1    L0020,L5BDA
        ora     #$27
        ora     $1F
        rmb1    $04
        php
        stx     $89
        cly
        cmp     $EE21,x
        jsr     L09FF
        inc     $05,x
        plx
        rmb1    $E8
        bbs3    $7F,L5BEB
        .byte   $AB
        smb3    L0040
        lsr     a
        php
        asl     $9D06,x
        sta     $EFEB,y
L5BAE:  eor     $7F,y
        brk
        brk
        trb     $00
        stz     $F708,x
        asl     $F9
        sta     $EF66,y
        bpl     L5C3E
L5BBF:  bra     L5C24
        rts

        adc     ($70),y
        adc     ($1F,x)
        sei
        sei
        sec
        sec
L5BCA:  bit     L583C,x
        rmb4    $8F
        cpx     #$1F
        brk
        bbr0    $00,L5BD5
L5BD5:  ora     ($07,x)
        brk
        rmb0    $80
L5BDA:  st0     #$80
L5BDC:  rti

        bra     L5BBF
        brk
        .byte   $2D
        brk
L5BE2:  .byte   $2D
        brk
L5BE4:  beq     L5BE2
        sbc     $05,x
L5BE8:  adc     ($00,x)
        .byte   $01
L5BEB:  brk
        cpy     $F4
        tia     $FC0F,$FCF1,$00F1
        sbc     ($F9),y
        brk
        sed
        st0     #$F8
        st0     #$04
        .byte   $C3
        bbr0    $00,$5C00
        .byte   $FF
L5C02:  bbr7    $3F,$5C25
        bbs1    $1F,$5BC8
        php
        rts

        tsb     $30
        sxy
        clc
        ora     ($0C,x)
        bbs7    $FF,L5C53
        cpy     #$20
        rts

        bpl     L5C48
        php
        clc
        tsb     $0C
        sxy
        asl     $01
        st0     #$00
        bbs7    $BF,L5BE4
L5C24:  rti

        cpx     #$BF
        bbr7    $C0,L5C69
        cpy     #$3F
        rti

        bbr3    $3F,L5C30
L5C30:  brk
        bbr7    $00,L5C73
        brk
        bbr1    $00,L5C38
L5C38:  brk
        brk
        brk
        brk
        brk
        brk
L5C3E:  brk
L5C3F:  brk
        bbr7    $7F,L5C02
        bbr3    $00,L5C65
        .byte   $FF
        .byte   $F0
L5C48:  brk
        beq     L5C4B
L5C4B:  .byte   $F0
L5C4C:  brk
L5C4D:  beq     L5C3F
        brk
        bbs7    $7F,L5C93
L5C53:  bra     L5C75
        cpy     #$00
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        ldy     L0038
        smb2    $3B
        .byte   $A4
L5C65:  .byte   $3B
        cmp     ($1D)
        .byte   $D2
L5C69:  .byte   $1C
L5C6A:  tin     $D21D,$D21D,$C01D
        st0     #$C3
L5C73:  brk
        .byte   $C3
L5C75:  brk
        sbc     ($00,x)
        cpx     #$01
        sbc     ($00,x)
        sbc     ($00,x)
        sbc     ($00,x)
        rol     a
        .byte   $4E
        .byte   $35
L5C83:  jmp     L4C35

        rol     a
        lsr     L4E2A
        inc     a
        rol     $1A
        rol     $15
        rmb2    $0D
        bvs     L5C96
L5C93:  rts

        st0     #$60
L5C96:  ora     $0D70
        bvs     L5C9C
        .byte   $30
L5C9C:  ora     ($30,x)
        asl     L0038
        .byte   $AB
        rts

        tax
        rts

        tax
        rts

        tax
        rts

        ldx     $D660
        bmi     L5C83
        bmi     L5D06
        bmi     L5CD0
        brk
        bbr1    $00,L5CD4
        brk
        bbr1    $00,L5CD8
        brk
        bbs0    $00,L5C4C
        brk
        bbs0    $00,L5C6A
        asl     $0EA9
        lda     #$0E
        lda     #$0E
        sbc     #$0E
        ldy     $07,x
        ldy     $07,x
        ldy     $07,x
L5CD0:  beq     L5CD2
L5CD2:  beq     L5CD4
L5CD4:  beq     L5CD6
L5CD6:  beq     L5CD8
L5CD8:  beq     L5CDA
L5CDA:  sed
        brk
        sed
        brk
        sed
        brk
        .byte   $FF
        .byte   $FF
L5CE2:  bbs7    $00,$5CE5
        brk
        .byte   $FF
        .byte   $FF
L5CE8:  bbs7    $FF,$5CEA
        brk
L5CEC:  bra     L5CEE
L5CEE:  bbs7    $7F,L5CF1
L5CF1:  .byte   $FF
        brk
L5CF3:  bbs7    $00,$5CF5
        .byte   $FF
        brk
L5CF8:  brk
        bbs7    $00,L5D7B
        brk
        bbr7    $7F,L5D00
L5D00:  .byte   $FF
        .byte   $FF
L5D02:  bbs7    $00,$5D05
        brk
L5D06:  .byte   $FF
        .byte   $FF
L5D08:  bbs7    $FF,$5D0A
        brk
L5D0C:  ora     ($00,x)
        bbs7    $FE,L5D11
L5D11:  .byte   $FF
        brk
L5D13:  bbs7    $00,$5D15
L5D16:  .byte   $FF
        brk
L5D18:  brk
        .byte   $FF
L5D1A:  brk
        inc     LFE00,x
        inc     $9500,x
        bvs     $5CB8
        bvs     $5CBA
        bvs     $5CBC
        bvs     $5CC0
        bvs     $5D58
        cpx     #$2D
        cpx     #$2D
        cpx     #$0F
        brk
        bbr0    $00,L5D44
        brk
        bbr0    $00,L5D48
        brk
        .byte   $1F
        brk
L5D3C:  bbr1    $00,$5D5E
        brk
L5D40:  cmp     $06,x
L5D42:  eor     $06,x
L5D44:  eor     $06,x
        eor     $06,x
L5D48:  adc     $06,x
        .byte   $6B
        tsb     $0C6B
        nop
        tsb     a:$F8
        sed
        brk
        sed
        brk
        sed
        brk
        sed
        brk
        sbc     ($00),y
L5D5C:  sbc     ($00),y
        sbc     ($00),y
        lsr     $72,x
        ldy     $AC32
        and     ($54)
        adc     ($54)
        adc     ($58)
        stz     $58
        stz     $A8
        cpx     $B0
        asl     $06C0
        cpy     #$06
        bcs     L5D86
        bcs     L5D88
        .byte   $80
L5D7B:  tsb     $0C80
        rts

        trb     $FFFF
        bbr7    $3F,L5DA5
        .byte   $9F
L5D86:  .byte   $1F
        .byte   $C0
L5D88:  clc
        rts

        clc
L5D8B:  jsr     L0018
        brk
        brk
        bbs7    $FF,L5DD3
        cpy     #$20
        rts

        bpl     L5DC8
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        brk
        .byte   $95
L5DA1:  bvs     $5D38
        bvs     $5D3A
L5DA5:  bvs     L5D3C
        bvs     L5D40
        bvs     L5D40
        bvs     L5D42
        bvs     L5D44
        bvs     L5DC0
        brk
        bbr0    $00,L5DC4
        brk
        bbr0    $00,L5DC8
        brk
        bbr0    $00,L5DCC
        brk
        .byte   $0F
        brk
L5DC0:  cmp     $06,x
        eor     $06,x
L5DC4:  eor     $06,x
        eor     $06,x
L5DC8:  adc     $06,x
        adc     $06,x
L5DCC:  eor     $06,x
        cmp     $06,x
        sed
        brk
        sed
L5DD3:  brk
        sed
        brk
        sed
        brk
        sed
        brk
        sed
        brk
        sed
        brk
        sed
        brk
        lsr     $72,x
        ldy     $AC32
        .byte   $32
L5DE6:  csl
        bvs     L5E3F
        adc     ($AC)
        and     ($AC)
        and     ($54)
        bvs     L5DA1
        asl     $06C0
        cpy     #$06
        bcs     L5E04
        bcs     L5E08
        cpy     #$06
        cpy     #$06
        bcs     L5E0C
        bbs7    $FF,L5E03
L5E03:  .byte   $FF
L5E04:  .byte   $FF
        brk
L5E06:  .byte   $FF
        brk
L5E08:  bbs7    $00,L5D8B
        brk
L5E0C:  bbr7    $BF,L5E4F
        .byte   $80
L5E10:  bbs7    $FF,$5E12
L5E13:  bbs7    $00,$5E15
        brk
        bbr7    $7F,$5E99
        bbr7    $7F,$5E1D
        bbr3    $00,$5E20
        bbr3    $DF,L5E43
        bbs5    $3F,L5DE6
        bbr3    $C0,L5DC8
        cpx     #$90
        cpx     #$9F
        bbs6    $90,L5E10
        bbr1    $1F,L5E52
        bbr1    $00,L5E55
        brk
        bbr1    $0F,L5E49
        bbr0    $0F,L5E3D
L5E3D:  .byte   $0F
        brk
L5E3F:  brk
        .byte   $FF
        .byte   $FF
L5E42:  brk
L5E43:  bbs7    $FF,L5E46
L5E46:  .byte   $FF
        brk
L5E48:  .byte   $FF
L5E49:  brk
        ora     ($00,x)
        inc     $02FD,x
L5E4F:  ora     ($FF,x)
        .byte   $FF
L5E52:  bbs7    $FF,L5E55
L5E55:  .byte   $FF
L5E56:  brk
        inc     LFEFE,x
        inc     a:$FE,x
L5E5D:  .byte   $FC
        brk
        brk
        .byte   $FF
        brk
L5E62:  bbs7    $00,$5E65
        brk
        .byte   $FF
        .byte   $7F
L5E68:  bbs7    $00,$5E6A
        brk
        rti

        bra     L5EEE
        .byte   $BF
        brk
L5E71:  bbs7    $00,$5E73
        brk
L5E75:  bbs7    $7F,$5E78
        brk
L5E79:  bbr7    $00,L5EFB
        brk
        bbr3    $3F,L5E80
L5E80:  ora     $27,x
        ora     ($26)
        ora     $0A13
        st1     #$0A
        st1     #$0D
        st1     #$0D
        st1     #$05
        ora     #$06
        sec
        ora     (L0038,x)
        brk
        clc
        .byte   $03
L5E97:  trb     $1C03
L5E9A:  brk
        clc
        brk
        clc
        ora     ($0E,x)
        eor     $30,x
        cmp     $30,x
        adc     #$18
        lda     #$98
        tax
L5EA9:  tya
        ror     a
        clc
        ror     a
L5EAD:  clc
        eor     $CC,x
        bbs0    $00,L5E42
        brk
        smb4    $00
        rmb4    $00
        rmb4    $00
        smb4    $00
        smb4    $00
        tst     #$00,$B4,x
        rmb0    $B4
        rmb0    $BA
        st0     #$BA
        st0     #$7A
        st0     #$5A
        st0     #$5A
        st0     #$D5
        ora     ($F8,x)
        brk
        sed
        brk
        .byte   $FC
        brk
        .byte   $FC
        brk
        .byte   $FC
        brk
        .byte   $FC
        brk
        .byte   $FC
        brk
        inc     $FF00,x
        bbr7    $FF,L5EE4
L5EE4:  rti

        bra     L5F66
        bbs3    $7F,L5EA9
        bbr7    $80,L5F2D
        .byte   $80
L5EEE:  bbr3    $DF,L5EF1
L5EF1:  bbr7    $00,L5F73
        brk
        bbr3    $3F,L5EF8
L5EF8:  brk
        .byte   $3F
        brk
L5EFB:  bbr3    $00,L5F3D
        .byte   $1F
        brk
L5F00:  bbs7    $FE,$5F02
        brk
        sxy
        ora     ($FE,x)
        sbc     $FDFE,x
        inc     $0201,x
        ora     ($FC,x)
        .byte   $FB
        brk
        inc     LFE00,x
        brk
        .byte   $FC
        .byte   $FC
        brk
        brk
        .byte   $FC
        brk
        .byte   $FC
        brk
        .byte   $FC
        sed
        brk
        and     $2DE0
        cpx     #$5D
        cpy     #$5D
        cpy     #$5E
        cpy     #$5A
L5F2B:  cpy     #$5A
L5F2D:  cpy     #$AB
        .byte   $80
L5F30:  .byte   $1F
        brk
L5F32:  .byte   $1F
        brk
L5F34:  .byte   $3F
        brk
L5F36:  bbr3    $00,L5F78
        brk
        bbr3    $00,L5F7C
L5F3D:  brk
        bbr7    $00,L5F2B
        tsb     $0C6B
        lsr     L0018,x
        eor     $19,x
        cmp     $19,x
        dec     L0018,x
        dec     L0018,x
        tax
        .byte   $33
L5F50:  sbc     ($00),y
        sbc     ($00),y
        tia     $E200,$E200,$E300
        brk
        tia     $C500,$A800,$48E4
        stz     $B0
        iny
L5F66:  bvc     L5F30
        bvc     L5F32
        bcs     L5F34
        bcs     L5F36
        ldy     #$90
        rts

        .byte   $1C
        .byte   $80
L5F73:  .byte   $1C
L5F74:  brk
        clc
        cpy     #$38
L5F78:  cpy     #$38
        brk
        clc
L5F7C:  brk
        clc
        bra     L5FF0
        .byte   $FF
        .byte   $FF
L5F82:  bbs7    $00,$5F85
        brk
        .byte   $FF
        .byte   $FE
L5F88:  bbs7    $FE,$5F8A
        brk
        sxy
        ora     ($FE,x)
        sbc     $FF00,x
        brk
        .byte   $FF
        brk
L5F95:  .byte   $FF
L5F96:  inc     a:$00,x
        inc     LFE00,x
        brk
        .byte   $FC
        .byte   $FC
        brk
        sta     $70,x
        sta     $70,x
        sta     $70,x
        and     $2EE0
        cpx     #$2A
        cpx     #$5E
        cpy     #$5B
        cpy     #$0F
        brk
        bbr0    $00,L5FC4
        brk
        .byte   $1F
        brk
L5FB8:  bbr1    $00,$5FDA
        brk
        bbr3    $00,$5FFE
        brk
        cmp     $06,x
        eor     $06,x
L5FC4:  eor     $06,x
        ror     a
        tsb     $0CEA
        .byte   $EB
        tsb     $18D7
        ora     $19,x
        sed
        brk
        sed
        brk
        sed
        brk
        sbc     ($00),y
L5FD8:  sbc     ($00),y
        sbc     ($00),y
        tia     $E200,$5600,$AC72
        and     ($AC)
        and     ($A0)
        cpx     $A8
        cpx     $58
        stz     $50
        pla
        bvc     L5FB8
L5FF0:  bcs     L6000
        cpy     #$06
        cpy     #$06
        rts

        trb     $1C60
        bra     L6008
L5FFC:  bra     L6006
        cpy     #$38

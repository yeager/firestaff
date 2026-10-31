; Authentic System Card 3.0 CD status polling and data dispatch.
; This bounded disassembly excerpt is not a BIOS image.
;
; BIOS MD5: ff1a674273fe3540ccef576376407d1d (262144-byte payload).
; The .pce file is 262656 bytes: HuC_LoadCD strips its 512-byte header,
; then reads the payload into ROMSpace. With MPR7=$00, logical $E95E maps
; to ROMSpace offset $095E and file offset 512+$095E = 2910.
; MAME 0.285 command on TRV2:
;   unidasm syscard3.pce -arch h6280 -skip 2910 -basepc 0xE95E -count 80
; At $E976 in the v21 capture, MPR6=$01; logical $CC17 therefore maps to
; ROMSpace offset $2C17 and file offset 512+$2C17 = 11799.
;   unidasm syscard3.pce -arch h6280 -skip 11799 -basepc 0xCC17 -count 80
;
; $1800 status bits are reported by the reference core. This listing records
; the authentic BIOS control flow; it does not assign higher-level CD meaning.

e95e: ad 00 18  lda  $1800
e961: 29 f8     and  #$F8
e963: 8d 7a 22  sta  $227A
e966: c9 d0     cmp  #$D0
e968: f0 10     beq  $E97A
e96a: 29 b8     and  #$B8
e96c: c9 98     cmp  #$98
e96e: f0 1a     beq  $E98A
e970: c9 88     cmp  #$88
e972: f0 16     beq  $E98A
e974: c9 80     cmp  #$80
e976: f0 12     beq  $E98A
e978: 80 e4     bra  $E95E
e97a: bd 4c 22  lda  $224C,x
e97d: e8        inx
e97e: 8d 01 18  sta  $1801
e981: ea        nop
e982: ea        nop
e983: ea        nop
e984: ea        nop
e985: 20 27 ea  jsr  $EA27
e988: 80 d4     bra  $E95E
e98a: ad a4 22  lda  $22A4
e98d: 0d 48 22  ora  $2248
e990: d0 32     bne  $E9C4
e992: ad 4c 22  lda  $224C
e995: c9 d9     cmp  #$D9
e997: d0 0b     bne  $E9A4
e999: ad 4d 22  lda  $224D
e99c: c9 01     cmp  #$01
e99e: f0 24     beq  $E9C4
e9a0: c9 02     cmp  #$02
e9a2: f0 20     beq  $E9C4
e9a4: ae 41 22  ldx  $2241
e9a7: ad 00 18  lda  $1800
e9aa: 29 f8     and  #$F8
e9ac: 8d 7a 22  sta  $227A

cc17: ad 56 22  lda  $2256
cc1a: 20 00 e6  jsr  $E600
cc1d: 8d 56 22  sta  $2256
cc20: ad 57 22  lda  $2257
cc23: 20 00 e6  jsr  $E600
cc26: 8d 57 22  sta  $2257
cc29: ad 58 22  lda  $2258
cc2c: 20 00 e6  jsr  $E600
cc2f: 8d 58 22  sta  $2258
cc32: 64 f8     stz  $F8
cc34: 64 f9     stz  $F9
cc36: 64 fa     stz  $FA
cc38: ad 56 22  lda  $2256
cc3b: 44 5a     bsr  $CC97
cc3d: 44 3b     bsr  $CC7A
cc3f: 44 40     bsr  $CC81
cc41: 44 3e     bsr  $CC81
cc43: 44 3c     bsr  $CC81
cc45: 44 3a     bsr  $CC81
cc47: ad 57 22  lda  $2257
cc4a: 44 4b     bsr  $CC97
cc4c: 44 35     bsr  $CC83
cc4e: a5 f8     lda  $F8
cc50: 85 ec     sta  $EC
cc52: a5 f9     lda  $F9
cc54: 85 ed     sta  $ED
cc56: a5 fa     lda  $FA
cc58: 85 ee     sta  $EE
cc5a: 44 25     bsr  $CC81
cc5c: 44 1c     bsr  $CC7A
cc5e: 44 21     bsr  $CC81
cc60: 44 18     bsr  $CC7A
cc62: 44 16     bsr  $CC7A
cc64: 44 1b     bsr  $CC81
cc66: ad 58 22  lda  $2258
cc69: 38        sec
cc6a: e9 96     sbc  #$96
cc6c: 85 ee     sta  $EE
cc6e: 62        cla
cc6f: e9 00     sbc  #$00
cc71: 85 ed     sta  $ED
cc73: 62        cla
cc74: e9 00     sbc  #$00
cc76: 85 ec     sta  $EC
cc78: 80 09     bra  $CC83
cc7a: 06 ee     asl  $EE
cc7c: 26 ed     rol  $ED
cc7e: 26 ec     rol  $EC
cc80: 60        rts
cc81: 44 f7     bsr  $CC7A
cc83: a5 fa     lda  $FA
cc85: 18        clc
cc86: 65 ee     adc  $EE
cc88: 85 fa     sta  $FA
cc8a: a5 f9     lda  $F9
cc8c: 65 ed     adc  $ED
cc8e: 85 f9     sta  $F9
cc90: a5 f8     lda  $F8
cc92: 65 ec     adc  $EC
cc94: 85 f8     sta  $F8
cc96: 60        rts
cc97: 85 ee     sta  $EE
cc99: 64 ed     stz  $ED
cc9b: 64 ec     stz  $EC
cc9d: 60        rts

; $E600 converts valid packed-decimal nibbles with high*10+low and returns
; the original byte when either nibble is invalid. $CC17 calls it for
; $2256-$2258, then performs the literal shift/add calculation and returns
; at $CC96. The adjacent helper at $CC9E is a separate routine.
e600: da        phx
e601: aa        tax
e602: 29 f0     and  #$F0
e604: c9 a0     cmp  #$A0
e606: b0 16     bcs  $E61E
e608: 4a        lsr  a
e609: 85 ec     sta  $EC
e60b: 4a        lsr  a
e60c: 4a        lsr  a
e60d: 18        clc
e60e: 65 ec     adc  $EC
e610: 85 ec     sta  $EC
e612: 8a        txa
e613: 29 0f     and  #$0F
e615: c9 0a     cmp  #$0A
e617: b0 05     bcs  $E61E
e619: 18        clc
e61a: 65 ec     adc  $EC
e61c: aa        tax
e61d: 18        clc
e61e: 8a        txa
e61f: fa        plx
e620: 60        rts

; $CC96 returns to $E17C. The BIOS then calls $EC05; its control path calls
; $E900, writes $81 to $1801, and polls $1800 before the table checks below.
e17c: 73 f8 20 74 22 03 00  tii  $20F8 $2274 $0003
e183: 73 f8 20 77 22 03 00  tii  $20F8 $2277 $0003
e18a: 64 fc     stz  $FC
e18c: 64 fd     stz  $FD
e18e: 64 fe     stz  $FE
e190: a9 00     lda  #$00
e192: 85 fa     sta  $FA
e194: a9 28     lda  #$28
e196: 85 fb     sta  $FB
e198: a9 01     lda  #$01
e19a: 85 ff     sta  $FF
e19c: 1a        ina
e19d: 85 f8     sta  $F8
e19f: 64 f9     stz  $F9
e1a1: 20 05 ec  jsr  $EC05
e1a4: a9 1f     lda  #$1F
e1a6: 53 40     tam  #$40
e1a8: a9 00     lda  #$00
e1aa: 85 ec     sta  $EC
e1ac: a9 28     lda  #$28
e1ae: 85 ed     sta  $ED
e1b0: a9 e1     lda  #$E1
e1b2: 85 ee     sta  $EE
e1b4: a9 da     lda  #$DA
e1b6: 85 ef     sta  $EF
e1b8: a9 d5     lda  #$D5
e1ba: 85 fc     sta  $FC
e1bc: a9 04     lda  #$04
e1be: 85 fd     sta  $FD
e1c0: b2 ec     lda  ($EC)
e1c2: d2 ee     cmp  ($EE)
e1c4: d0 43     bne  $E209
e1c6: e6 ec     inc  $EC
e1c8: d0 02     bne  $E1CC
e1ca: e6 ed     inc  $ED
e1cc: e6 ee     inc  $EE
e1ce: d0 02     bne  $E1D2
e1d0: e6 ef     inc  $EF
e1d2: a5 fc     lda  $FC
e1d4: d0 02     bne  $E1D8
e1d6: c6 fd     dec  $FD
e1d8: c6 fc     dec  $FC
e1da: a5 fc     lda  $FC
e1dc: 05 fd     ora  $FD
e1de: d0 e0     bne  $E1C0
e1e0: a9 4a     lda  #$4A
e1e2: 85 fc     sta  $FC
e1e4: a9 00     lda  #$00
e1e6: 85 fd     sta  $FD
e1e8: b2 ec     lda  ($EC)
e1ea: d0 1d     bne  $E209
e1ec: e6 ec     inc  $EC
e1ee: d0 02     bne  $E1F2
e1f0: e6 ed     inc  $ED
e1f2: e6 ee     inc  $EE
e1f4: d0 02     bne  $E1F8
e1f6: e6 ef     inc  $EF
e1f8: a5 fc     lda  $FC
e1fa: d0 02     bne  $E1FE
e1fc: c6 fd     dec  $FD
e1fe: c6 fc     dec  $FC
e200: a5 fc     lda  $FC
e202: 05 fd     ora  $FD
e204: d0 e2     bne  $E1E8
e206: 20 26 2b  jsr  $2B26
e209: 38        sec
e20a: 60        rts

; EC05 programs the request at $224C-$2250 and calls E900. Only selected
; entry/return instructions are shown in this excerpt.
ec05: 9c 73 22  stz  $2273
ec08: 20 09 f3  jsr  $F309
ec0b: 20 ee f0  jsr  $F0EE
ec0e: a9 08     lda  #$08
ec10: 8d 4c 22  sta  $224C
ec13: 20 04 f1  jsr  $F104
ec16: a2 04     ldx  #$04
ec18: a0 01     ldy  #$01
ec1a: 20 27 f3  jsr  $F327
ec1d: a5 ff     lda  $FF
ec1f: c9 02     cmp  #$02
ec21: 90 0e     bcc  $EC31
ec31: a5 ff     lda  $FF
ec33: f0 0e     beq  $EC43
ec35: a5 f8     lda  $F8
ec37: 8d 80 22  sta  $2280
ec3a: 0a        asl  a
ec3b: 0a        asl  a
ec3c: 0a        asl  a
ec3d: 85 f9     sta  $F9
ec3f: 64 f8     stz  $F8
ec41: 80 03     bra  $EC46
ec46: ad 80 22  lda  $2280
ec49: f0 2b     beq  $EC76
ec4b: 8d 50 22  sta  $2250
ec4e: 20 00 e9  jsr  $E900
e900: ad a4 22  lda  $22A4
e903: d0 fb     bne  $E900
e905: 9c 9b 22  stz  $229B
e908: a9 81     lda  #$81
e90a: 8d 01 18  sta  $1801
e90d: 93 80 00 18 tst  #$80 $1800
e911: f0 31     beq  $E944
e944: 8d 00 18  sta  $1800
e947: 82        clx
e948: ad 00 18  lda  $1800
e94b: 29 40     and  #$40
e94d: d0 0b     bne  $E95A
e95a: 9c 7a 22  stz  $227A
e95d: 82        clx
e95e: ad 00 18  lda  $1800

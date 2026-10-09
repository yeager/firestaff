; Theron's Quest JP Rev. 1 Track 02 — static target candidate at $C95D
;
; Source: authentic TQJP02.bin, MD5 b7afb338ad31be1025b53f9aff12d73a
; Raw BIN offset: $0A3EBB; length: 255 bytes; FNV-1a: $063B99E9
; MAME 0.285 unidasm -arch h6280, base PC $C95D. Three hashes matched:
; 76ecb69756ce1329fa8be3bb9afa72569b25509b402d2f9d8596eb4c374f3b20
;
; Static candidate only. The JP $C414 caller has a JSR $C95D operand, and
; this raw offset is one sector ($930 bytes) before the US $C96B source span.
; This does not prove that the raw candidate is mapped at $C95D or executes.
; The window includes a bounded RTS at $CA1B and then continues through other
; bytes; no reachability or gameplay meaning is assigned to the tail.

00c95d: 48                    pha
00c95e: 20 dd 45              jsr  $45DD
00c961: e0 6d                 cpx  #$6D
00c963: 68                    pla
00c964: c5 db                 cmp  $DB
00c966: d0 25                 bne  $C98D
00c968: ad 02 2e              lda  $2E02
00c96b: d0 15                 bne  $C982
00c96d: 9c 02 2e              stz  $2E02
00c970: ad f7 2d              lda  $2DF7
00c973: d0 0d                 bne  $C982
00c975: ad f4 2d              lda  $2DF4
00c978: 85 36                 sta  $36
00c97a: ad f5 2d              lda  $2DF5
00c97d: 85 37                 sta  $37
00c97f: 20 73 5c              jsr  $5C73
00c982: ad 04 2e              lda  $2E04
00c985: d0 03                 bne  $C98A
00c987: 9c 04 2e              stz  $2E04
00c98a: 20 c7 5d              jsr  $5DC7
00c98d: a6 bb                 ldx  $BB
00c98f: 20 cf d3              jsr  $D3CF
00c992: a9 0a                 lda  #$0A
00c994: a2 80                 ldx  #$80
00c996: 20 dd 45              jsr  $45DD
00c999: 25 ce                 and  $CE
00c99b: f0 33                 beq  $C9D0
00c99d: 20 42 51              jsr  $5142
00c9a0: a0 02                 ldy  #$02
00c9a2: a9 85                 lda  #$85
00c9a4: 91 3a                 sta  ($3A),y
00c9a6: a0 03                 ldy  #$03
00c9a8: b1 3a                 lda  ($3A),y
00c9aa: 29 3f                 and  #$3F
00c9ac: 85 ba                 sta  $BA
00c9ae: a5 bb                 lda  $BB
00c9b0: 4a                    lsr  a
00c9b1: 6a                    ror  a
00c9b2: 6a                    ror  a
00c9b3: 29 c0                 and  #$C0
00c9b5: 05 ba                 ora  $BA
00c9b7: 91 3a                 sta  ($3A),y
00c9b9: a6 bb                 ldx  $BB
00c9bb: bd 47 29              lda  $2947,x
00c9be: 85 ba                 sta  $BA
00c9c0: bd 47 29              lda  $2947,x
00c9c3: 20 1a 51              jsr  $511A
00c9c6: 73 40 20 45 20 02 00  tii  $2040 $2045 $0002
00c9cd: 20 76 5d              jsr  $5D76
00c9d0: a6 bb                 ldx  $BB
00c9d2: 62                    cla
00c9d3: 9d 57 29              sta  $2957,x
00c9d6: 9d fb 2c              sta  $2CFB,x
00c9d9: a5 3f                 lda  $3F
00c9db: 9d 43 29              sta  $2943,x
00c9de: 62                    cla
00c9df: 9d 5f 29              sta  $295F,x
00c9e2: a5 ba                 lda  $BA
00c9e4: 38                    sec
00c9e5: e5 3f                 sbc  $3F
00c9e7: 29 03                 and  #$03
00c9e9: 85 ba                 sta  $BA
00c9eb: bd 63 29              lda  $2963,x
00c9ee: f0 03                 beq  $C9F3
00c9f0: 20 97 d5              jsr  $D597
00c9f3: a2 ff                 ldx  #$FF
00c9f5: a5 3f                 lda  $3F
00c9f7: a4 ba                 ldy  $BA
00c9f9: 20 dd 45              jsr  $45DD
00c9fc: b0 08                 bcs  $CA06
00c9fe: a6 bb                 ldx  $BB
00ca00: 20 50 5d              jsr  $5D50
00ca03: 82                    clx
00ca04: e4 da                 cpx  $DA
00ca06: b0 0b                 bcs  $CA13
00ca08: bd 77 29              lda  $2977,x
00ca0b: 1d 7b 29              ora  $297B,x
00ca0e: d0 0d                 bne  $CA1D
00ca10: e8                    inx
00ca11: 80 f1                 bra  $CA04
00ca13: a9 01                 lda  #$01
00ca15: 8d fc 2d              sta  $2DFC
00ca18: 20 dd 45              jsr  $45DD
00ca1b: 60                    rts
00ca1c: bc a5 bb              ldy  $BBA5,x
00ca1f: cd f3 2d              cmp  $2DF3
00ca22: d0 07                 bne  $CA2B
00ca24: da                    phx
00ca25: 20 dd 45              jsr  $45DD
00ca28: 02                    sxy
00ca29: af fa a5              bbs2 $FA $C9D1
00ca2c: bb                    ill  $BB
00ca2d: cd 39 2e              cmp  $2E39
00ca30: d0 07                 bne  $CA39
00ca32: 20 dd 45              jsr  $45DD
00ca35: c6 67                 dec  $67
00ca37: 80 08                 bra  $CA41
00ca39: ae 39 2e              ldx  $2E39
00ca3c: 20 dd 45              jsr  $45DD
00ca3f: 10 67                 bpl  $CAA8
00ca41: 60                    rts
00ca42: 20 3a 45              jsr  $453A
00ca45: 03 20                 st0  #$20
00ca47: 9f 45 86              bbs1 $45 $C9D0
00ca4a: bb                    ill  $BB
00ca4b: 85 b9                 sta  $B9
00ca4d: 84 ba                 sty  $BA
00ca4f: 84 8b                 sty  $8B
00ca51: e0 ff                 cpx  #$FF
00ca53: f0 06                 beq  $CA5B
00ca55: e8                    inx
00ca56: ec f9 2d              cpx  $2DF9
00ca59: d0 01                 bne  $CA5C
00ca5b: 60                    rts

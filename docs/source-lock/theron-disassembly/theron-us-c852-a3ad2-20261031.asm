; Theron's Quest (USA) Track 02 — source-locked HuC6280 window
; Authentic input: raw Track 02 BIN, SHA-256 f0474eae8f7c660b94dba7053b2a8e32b7c41330d7e7d3f255b113489731f565
; Raw file offset: $0A3AD2; window size: 512 bytes; window SHA-256:
; c2396321541223629083719fe542d90c5ba81ad63e85af1ba9353aa339e72351
; Linear disassembly: unidasm -arch h6280 -basepc 0xc852
;
; Runtime provenance: the isolated authentic cold US capture's first five
; 512-byte fetch windows at logical PC $C852 / physical PC $0DC852 match this
; source window exactly. Runtime fetch trace SHA-256:
; af0e870c3405700baf34b984bb26b9ef301d06f6ecb4f1b86d28058c7357b3d4
; The capture recorded 132 $C852 fetches and the signature-gated route reached
; Drator start, left-event, and reentered-event (level $03, position $02,$03).
;
; This source join identifies the code bytes mapped at the observed fetch site.
; It does not imply that all bytes in this 512-byte linear listing executed,
; that this routine consumes the generated row, or that any zero-page byte has
; game-field semantics. The capture wrapper remained incomplete because the
; System Card I/O/RAM trace hit its explicit row cap; see the companion report.

c852: a5 ba                 lda  $BA
c854: 85 a9                 sta  $A9
c856: a5 b8                 lda  $B8
c858: 85 a8                 sta  $A8
c85a: a6 a9                 ldx  $A9
c85c: a4 a8                 ldy  $A8
c85e: 20 39 50              jsr  $5039
c861: a5 36                 lda  $36
c863: c9 fe                 cmp  #$FE
c865: d0 04                 bne  $C86B
c867: a5 37                 lda  $37
c869: c9 ff                 cmp  #$FF
c86b: f0 5f                 beq  $C8CC
c86d: 73 36 20 a6 20 02 00  tii  $2036 $20A6 $0002
c874: 20 11 51              jsr  $5111
c877: c9 0e                 cmp  #$0E
c879: d0 45                 bne  $C8C0
c87b: 20 31 53              jsr  $5331
c87e: a8                    tay
c87f: 20 93 5d              jsr  $5D93
c882: a0 04                 ldy  #$04
c884: b1 bc                 lda  ($BC),y
c886: c9 0e                 cmp  #$0E
c888: f0 36                 beq  $C8C0
c88a: 20 09 51              jsr  $5109
c88d: aa                    tax
c88e: b5 ae                 lda  $AE,x
c890: f0 2e                 beq  $C8C0
c892: 3a                    dea
c893: 85 8a                 sta  $8A
c895: a5 bb                 lda  $BB
c897: 85 8c                 sta  $8C
c899: a5 b9                 lda  $B9
c89b: 85 8d                 sta  $8D
c89d: a5 ad                 lda  $AD
c89f: a6 ba                 ldx  $BA
c8a1: a4 b8                 ldy  $B8
c8a3: 20 e3 45              jsr  $45E3
c8a6: 9a                    txs
c8a7: 82                    clx
c8a8: f0 16                 beq  $C8C0
c8aa: 73 a6 20 36 20 02 00  tii  $20A6 $2036 $0002
c8b1: 20 e3 45              jsr  $45E3
c8b4: fa                    plx
c8b5: 81 ad                 sta  ($AD,x)
c8b7: 7c 28 c9              jmp  ($C928),X
c8ba: 02                    sxy
c8bb: d0 9d                 bne  $C85A
c8bd: a9 01                 lda  #$01
c8bf: 60                    rts
c8c0: 73 a6 20 36 20 02 00  tii  $20A6 $2036 $0002
c8c7: 20 7e 51              jsr  $517E
c8ca: 80 95                 bra  $C861
c8cc: a5 aa                 lda  $AA
c8ce: f0 19                 beq  $C8E9
c8d0: a5 b6                 lda  $B6
c8d2: 85 a9                 sta  $A9
c8d4: 1a                    ina
c8d5: 85 bb                 sta  $BB
c8d7: a5 b7                 lda  $B7
c8d9: 85 a8                 sta  $A8
c8db: 85 b9                 sta  $B9
c8dd: 73 b2 20 ae 20 04 00  tii  $20B2 $20AE $0004
c8e4: 64 aa                 stz  $AA
c8e6: 4c 5a c8              jmp  $C85A
c8e9: a9 00                 lda  #$00
c8eb: 60                    rts
c8ec: 64 38                 stz  $38
c8ee: 64 39                 stz  $39
c8f0: 20 40 45              jsr  $4540
c8f3: 0d a5 36              ora  $36A5
c8f6: 48                    pha
c8f7: a5 37                 lda  $37
c8f9: 48                    pha
c8fa: a5 36                 lda  $36
c8fc: c9 fe                 cmp  #$FE
c8fe: d0 04                 bne  $C904
c900: a5 37                 lda  $37
c902: c9 ff                 cmp  #$FF
c904: d0 03                 bne  $C909
c906: 4c 93 ca              jmp  $CA93
c909: 86 ba                 stx  $BA
c90b: 84 bb                 sty  $BB
c90d: 20 47 51              jsr  $5147
c910: 85 b9                 sta  $B9
c912: 73 3a 20 b5 20 02 00  tii  $203A $20B5 $0002
c919: a0 00                 ldy  #$00
c91b: a9 fe                 lda  #$FE
c91d: 91 b5                 sta  ($B5),y
c91f: c8                    iny
c920: a9 ff                 lda  #$FF
c922: 91 b5                 sta  ($B5),y
c924: a5 ba                 lda  $BA
c926: c9 80                 cmp  #$80
c928: 90 03                 bcc  $C92D
c92a: 4c 58 ca              jmp  $CA58
c92d: a6 ba                 ldx  $BA
c92f: a4 bb                 ldy  $BB
c931: 20 cc 50              jsr  $50CC
c934: 85 b8                 sta  $B8
c936: 73 14 20 b3 20 02 00  tii  $2014 $20B3 $0002
c93d: c2                    cly
c93e: b1 b3                 lda  ($B3),y
c940: 29 10                 and  #$10
c942: f0 0e                 beq  $C952
c944: a6 ba                 ldx  $BA
c946: a4 bb                 ldy  $BB
c948: 20 40 50              jsr  $5040
c94b: 86 38                 stx  $38
c94d: 84 39                 sty  $39
c94f: 4c 58 ca              jmp  $CA58
c952: b1 b3                 lda  ($B3),y
c954: 09 10                 ora  #$10
c956: 91 b3                 sta  ($B3),y
c958: a9 0c                 lda  #$0C
c95a: 20 ed 44              jsr  $44ED
c95d: a5 ba                 lda  $BA
c95f: 1a                    ina
c960: 0a                    asl  a
c961: 18                    clc
c962: 65 34                 adc  $34
c964: 85 3a                 sta  $3A
c966: a5 35                 lda  $35
c968: 69 00                 adc  #$00
c96a: 85 3b                 sta  $3B
c96c: 64 b2                 stz  $B2
c96e: a5 29                 lda  $29
c970: 0a                    asl  a
c971: aa                    tax
c972: bd 46 69              lda  $6946,x
c975: 4a                    lsr  a
c976: 85 b1                 sta  $B1
c978: bd 45 69              lda  $6945,x
c97b: 6a                    ror  a
c97c: 18                    clc
c97d: 65 ba                 adc  $BA
c97f: 85 b1                 sta  $B1
c981: 90 02                 bcc  $C985
c983: e6 b2                 inc  $B2
c985: a9 35                 lda  #$35
c987: 38                    sec
c988: e5 b1                 sbc  $B1
c98a: 85 b1                 sta  $B1
c98c: a9 00                 lda  #$00
c98e: e5 b2                 sbc  $B2
c990: 85 b2                 sta  $B2
c992: c2                    cly
c993: c6 b1                 dec  $B1
c995: a5 b1                 lda  $B1
c997: c9 ff                 cmp  #$FF
c999: d0 02                 bne  $C99D
c99b: c6 b2                 dec  $B2
c99d: a5 b1                 lda  $B1
c99f: 05 b2                 ora  $B2
c9a1: f0 16                 beq  $C9B9
c9a3: b1 3a                 lda  ($3A),y
c9a5: 1a                    ina
c9a6: 91 3a                 sta  ($3A),y
c9a8: c8                    iny
c9a9: 29 ff                 and  #$FF
c9ab: d0 05                 bne  $C9B2
c9ad: b1 3a                 lda  ($3A),y
c9af: 1a                    ina
c9b0: 91 3a                 sta  ($3A),y
c9b2: c8                    iny
c9b3: d0 de                 bne  $C993
c9b5: e6 3b                 inc  $3B
c9b7: 80 da                 bra  $C993
c9b9: 64 b7                 stz  $B7
c9bb: a9 0c                 lda  #$0C
c9bd: 20 ed 44              jsr  $44ED
c9c0: a5 ba                 lda  $BA
c9c2: 0a                    asl  a
c9c3: a8                    tay
c9c4: b1 34                 lda  ($34),y
c9c6: 85 b1                 sta  $B1
c9c8: c8                    iny
c9c9: b1 34                 lda  ($34),y
c9cb: 85 b2                 sta  $B2
c9cd: a5 b7                 lda  $B7
c9cf: c5 bb                 cmp  $BB
c9d1: f0 15                 beq  $C9E8
c9d3: a6 ba                 ldx  $BA
c9d5: a4 b7                 ldy  $B7
c9d7: 20 00 51              jsr  $5100
c9da: 29 10                 and  #$10
c9dc: f0 06                 beq  $C9E4
c9de: e6 b1                 inc  $B1
c9e0: d0 02                 bne  $C9E4
c9e2: e6 b2                 inc  $B2
c9e4: e6 b7                 inc  $B7
c9e6: 80 e5                 bra  $C9CD
c9e8: a6 b1                 ldx  $B1
c9ea: a4 b2                 ldy  $B2
c9ec: 20 50 50              jsr  $5050
c9ef: a5 3a                 lda  $3A
c9f1: 8d 46 ca              sta  $CA46
c9f4: 18                    clc
c9f5: 69 02                 adc  #$02
c9f7: 8d 48 ca              sta  $CA48
c9fa: a5 3b                 lda  $3B
c9fc: 8d 47 ca              sta  $CA47
c9ff: 69 00                 adc  #$00
ca01: 8d 49 ca              sta  $CA49
ca04: a9 6e                 lda  #$6E
ca06: 38                    sec
ca07: e5 b1                 sbc  $B1
ca09: aa                    tax
ca0a: a9 02                 lda  #$02
ca0c: e5 b2                 sbc  $B2
ca0e: a8                    tay
ca0f: ca                    dex
ca10: e0 ff                 cpx  #$FF
ca12: d0 01                 bne  $CA15
ca14: 88                    dey
ca15: 8a                    txa
ca16: 0a                    asl  a
ca17: aa                    tax
ca18: 98                    tya
ca19: 2a                    rol  a
ca1a: a8                    tay
ca1b: 8e 4a ca              stx  $CA4A
ca1e: 8c 4b ca              sty  $CA4B
ca21: ca                    dex
ca22: e0 ff                 cpx  #$FF
ca24: d0 01                 bne  $CA27
ca26: 88                    dey
ca27: 8a                    txa
ca28: 18                    clc
ca29: 6d 46 ca              adc  $CA46
ca2c: 8d 46 ca              sta  $CA46
ca2f: 98                    tya
ca30: 6d 47 ca              adc  $CA47
ca33: 8d 47 ca              sta  $CA47
ca36: 8a                    txa
ca37: 18                    clc
ca38: 6d 48 ca              adc  $CA48
ca3b: 8d 48 ca              sta  $CA48
ca3e: 98                    tya
ca3f: 6d 49 ca              adc  $CA49
ca42: 8d 49 ca              sta  $CA49
ca45: c3 00 00 00 00 01 00  tdd  $0000 $0000 $0001
ca4c: c2                    cly
ca4d: a5 36                 lda  $36
ca4f: 91 3a                 sta  ($3A),y
ca51: c8                    iny

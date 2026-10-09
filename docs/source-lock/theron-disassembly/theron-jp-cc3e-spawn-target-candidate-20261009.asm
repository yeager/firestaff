; Theron's Quest JP Rev. 1 Track 02 — static target candidate at $CC3E
;
; Source: authentic TQJP02.bin, MD5 b7afb338ad31be1025b53f9aff12d73a
; Raw BIN offset: $0A419C; length: 200 bytes; FNV-1a: $13A65EA6
; MAME 0.285 unidasm -arch h6280, base PC $CC3E. Three hashes matched:
; a689255f19cda570f115a45d3f9cd01232ea3358784942bf385d7f8a7ab292d9
;
; Static candidate only. The JP $C414 caller has a JSR $CC3E operand, and
; this raw offset is one sector ($930 bytes) before the US $CC4C source span.
; This does not prove that the raw candidate is mapped at $CC3E or executes.
; The window contains an RTS at $CC5C and continues at $CC5D; no reachability
; or gameplay meaning is assigned to the bytes after that return.

00cc3e: bb                    ill  $BB
00cc3f: a0 02                 ldy  #$02
00cc41: fa                    plx
00cc42: bd 73 29              lda  $2973,x
00cc45: 29 20                 and  #$20
00cc47: f0 06                 beq  $CC4F
00cc49: 98                    tya
00cc4a: 18                    clc
00cc4b: 65 bb                 adc  $BB
00cc4d: 85 bb                 sta  $BB
00cc4f: a9 05                 lda  #$05
00cc51: 20 62 5b              jsr  $5B62
00cc54: c9 fa                 cmp  #$FA
00cc56: d0 02                 bne  $CC5A
00cc58: c6 bb                 dec  $BB
00cc5a: a5 bb                 lda  $BB
00cc5c: 60                    rts
00cc5d: 20 3a 45              jsr  $453A
00cc60: 02                    sxy
00cc61: 5a                    phy
00cc62: b9 f7 29              lda  $29F7,y
00cc65: 64 bb                 stz  $BB
00cc67: 0a                    asl  a
00cc68: 26 bb                 rol  $BB
00cc6a: 0a                    asl  a
00cc6b: 26 bb                 rol  $BB
00cc6d: 0a                    asl  a
00cc6e: 26 bb                 rol  $BB
00cc70: 18                    clc
00cc71: 69 64                 adc  #$64
00cc73: 85 ba                 sta  $BA
00cc75: 90 02                 bcc  $CC79
00cc77: e6 bb                 inc  $BB
00cc79: 73 ba 20 8a 20 02 00  tii  $20BA $208A $0002
00cc80: 20 dd 45              jsr  $45DD
00cc83: b7 52                 smb3 $52
00cc85: 73 8a 20 ba 20 02 00  tii  $208A $20BA $0002
00cc8c: b9 73 29              lda  $2973,y
00cc8f: 29 7f                 and  #$7F
00cc91: f0 24                 beq  $CCB7
00cc93: 73 ba 20 8c 20 02 00  tii  $20BA $208C $0002
00cc9a: 46 8d                 lsr  $8D
00cc9c: 66 8c                 ror  $8C
00cc9e: 46 8d                 lsr  $8D
00cca0: 66 8c                 ror  $8C
00cca2: 29 10                 and  #$10
00cca4: d0 04                 bne  $CCAA
00cca6: 46 8d                 lsr  $8D
00cca8: 66 8c                 ror  $8C
00ccaa: 38                    sec
00ccab: a5 ba                 lda  $BA
00ccad: e5 8c                 sbc  $8C
00ccaf: 85 ba                 sta  $BA
00ccb1: a5 bb                 lda  $BB
00ccb3: e5 8d                 sbc  $8D
00ccb5: 85 bb                 sta  $BB
00ccb7: 98                    tya
00ccb8: aa                    tax
00ccb9: a9 05                 lda  #$05
00ccbb: 20 62 5b              jsr  $5B62
00ccbe: c9 fa                 cmp  #$FA
00ccc0: d0 24                 bne  $CCE6
00ccc2: 73 ba 20 8c 20 02 00  tii  $20BA $208C $0002
00ccc9: 46 8d                 lsr  $8D
00cccb: 66 8c                 ror  $8C
00cccd: 46 8d                 lsr  $8D
00cccf: 66 8c                 ror  $8C
00ccd1: 46 8d                 lsr  $8D
00ccd3: 66 8c                 ror  $8C
00ccd5: 46 8d                 lsr  $8D
00ccd7: 66 8c                 ror  $8C
00ccd9: 18                    clc
00ccda: a5 ba                 lda  $BA
00ccdc: 65 8c                 adc  $8C
00ccde: 85 ba                 sta  $BA
00cce0: a5 bb                 lda  $BB
00cce2: 65 8d                 adc  $8D
00cce4: 85 bb                 sta  $BB
00cce6: 18                    clc
00cce7: a5 ba                 lda  $BA
00cce9: 69 09                 adc  #$09
00cceb: 85 ba                 sta  $BA
00cced: 90 02                 bcc  $CCF1
00ccef: e6 bb                 inc  $BB
00ccf1: a6 ba                 ldx  $BA
00ccf3: a4 bb                 ldy  $BB
00ccf5: a9 0a                 lda  #$0A
00ccf7: 20 74 5a              jsr  $5A74
00ccfa: 38                    sec
00ccfb: a5 ba                 lda  $BA
00ccfd: e5 0b                 sbc  $0B
00ccff: 85 ba                 sta  $BA
00cd01: b0 02                 bcs  $CD05
00cd03: c6 bb                 dec  $BB
00cd05: 7a                    ply

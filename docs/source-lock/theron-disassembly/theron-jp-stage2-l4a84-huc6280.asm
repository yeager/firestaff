; Authentic JP/US Stage-2 user-byte window [0x4a84, 0x4b24).
; MAME 0.285 unidasm -arch h6280 -basepc 0x4a84 -skip 2898324 -count 160
; JP and US listings matched in three independent runs; listing SHA-256:
; 62e510911db6017dccdd1c82ed67ce0675c1eab2f2b4a7cc596dadcec1f8ccba
; This is a linear decode. The bounded branch/call edges are independently
; asserted by test_theron_v1_stage2_disassembly_chain.

004a84: ad d4 47  lda  $47D4
004a87: f0 6d     beq  $4AF6
004a89: 20 1f 49  jsr  $491F
004a8c: 03 00     st0  #$00
004a8e: ad 75 3b  lda  $3B75
004a91: 85 39     sta  $39
004a93: ad 74 3b  lda  $3B74
004a96: 0a        asl  a
004a97: 26 39     rol  $39
004a99: 0a        asl  a
004a9a: 26 39     rol  $39
004a9c: 0a        asl  a
004a9d: 26 39     rol  $39
004a9f: 18        clc
004aa0: 6d db 47  adc  $47DB
004aa3: 8d 02 00  sta  $0002
004aa6: a5 39     lda  $39
004aa8: 6d dc 47  adc  $47DC
004aab: 8d 03 00  sta  $0003
004aae: 03 02     st0  #$02
004ab0: ad d8 47  lda  $47D8
004ab3: 18        clc
004ab4: 6d 7a 3b  adc  $3B7A
004ab7: 85 38     sta  $38
004ab9: ad 7a 3b  lda  $3B7A
004abc: 0a        asl  a
004abd: a8        tay
004abe: ae 7b 3b  ldx  $3B7B
004ac1: b9 20 48  lda  $4820,y
004ac4: 8d 02 00  sta  $0002
004ac7: c8        iny
004ac8: b9 20 48  lda  $4820,y
004acb: 8d 03 00  sta  $0003
004ace: c8        iny
004acf: e6 38     inc  $38
004ad1: a9 20     lda  #$20
004ad3: c5 38     cmp  $38
004ad5: d0 1c     bne  $4AF3
004ad7: ad 7a 3b  lda  $3B7A
004ada: 85 3b     sta  $3B
004adc: ad d5 47  lda  $47D5
004adf: 85 3a     sta  $3A
004ae1: 44 41     bsr  $4B24
004ae3: 64 38     stz  $38
004ae5: 03 00     st0  #$00
004ae7: a5 3c     lda  $3C
004ae9: 8d 02 00  sta  $0002
004aec: a5 3d     lda  $3D
004aee: 8d 03 00  sta  $0003
004af1: 03 02     st0  #$02
004af3: ca        dex
004af4: d0 cb     bne  $4AC1
004af6: ad ba 47  lda  $47BA
004af9: 85 38     sta  $38
004afb: ad bb 47  lda  $47BB
004afe: 85 39     sta  $39
004b00: a5 38     lda  $38
004b02: 05 39     ora  $39
004b04: f0 10     beq  $4B16
004b06: a5 38     lda  $38
004b08: d0 02     bne  $4B0C
004b0a: c6 39     dec  $39
004b0c: c6 38     dec  $38
004b0e: d0 06     bne  $4B16
004b10: a5 39     lda  $39
004b12: d0 02     bne  $4B16
004b14: 64 50     stz  $50
004b16: a5 38     lda  $38
004b18: 8d ba 47  sta  $47BA
004b1b: a5 39     lda  $39
004b1d: 8d bb 47  sta  $47BB
004b20: 9c d4 47  stz  $47D4
004b23: 60        rts

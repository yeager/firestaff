; Authentic JP/US Stage-2 user-byte window [0x4bb0, 0x4c0d).
; MAME 0.285 unidasm -arch h6280 -basepc 0x4bb0 -skip 2898624 -count 93
; JP and US listings matched in three independent runs; listing SHA-256:
; beca5347eeeb875f64293ce010eef58bd1f967b909f7fe436027bc6c9f0b9640
; The existing authentic-media verifier binds this 93-byte callee and its
; scroll-state memory stores; this file is a linear decode, not runtime evidence.

004bb0: ad 11 4c  lda  $4C11
004bb3: f0 57     beq  $4C0C
004bb5: ad 10 4c  lda  $4C10
004bb8: 3a        dea
004bb9: 8d 10 4c  sta  $4C10
004bbc: d0 4e     bne  $4C0C
004bbe: ad 0f 4c  lda  $4C0F
004bc1: 8d 10 4c  sta  $4C10
004bc4: a0 69     ldy  #$69
004bc6: ad 0d 4c  lda  $4C0D
004bc9: f0 1c     beq  $4BE7
004bcb: 49 ff     eor  #$FF
004bcd: 1a        ina
004bce: 8d 0d 4c  sta  $4C0D
004bd1: 10 02     bpl  $4BD5
004bd3: a0 e9     ldy  #$E9
004bd5: 8c e2 4b  sty  $4BE2
004bd8: 18        clc
004bd9: 6d 0c 22  adc  $220C
004bdc: 8d 0c 22  sta  $220C
004bdf: ad 0d 22  lda  $220D
004be2: 69 00     adc  #$00
004be4: 8d 0d 22  sta  $220D
004be7: a0 69     ldy  #$69
004be9: f0 1f     beq  $4C0A
004beb: ad 0e 4c  lda  $4C0E
004bee: 49 ff     eor  #$FF
004bf0: 1a        ina
004bf1: 8d 0e 4c  sta  $4C0E
004bf4: 10 02     bpl  $4BF8
004bf6: a0 e9     ldy  #$E9
004bf8: 8c 05 4c  sty  $4C05
004bfb: 18        clc
004bfc: 6d 10 22  adc  $2210
004bff: 8d 10 22  sta  $2210
004c02: ad 11 22  lda  $2211
004c05: 69 00     adc  #$00
004c07: 8d 11 22  sta  $2211
004c0a: a9 01     lda  #$01
004c0c: 60        rts

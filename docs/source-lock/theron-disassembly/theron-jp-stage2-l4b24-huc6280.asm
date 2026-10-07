; Authentic JP/US Stage-2 user-byte window [0x4b24, 0x4b3c).
; MAME 0.285 unidasm -arch h6280 -basepc 0x4b24 -skip 2898484 -count 24
; JP and US listings matched in three independent runs; listing SHA-256:
; d5c35a93f66068445124a7d493e020618bc4b414cedf383df0dcaa2d8b337f78
; The BSR caller at $4ae1 and this helper's branch/RTS are source-locked in
; tests/test_theron_v1_stage2_disassembly_chain.c.

004b24: 64 3c  stz  $3C
004b26: a5 3b  lda  $3B
004b28: 4a     lsr  a
004b29: 66 3c  ror  $3C
004b2b: 4a     lsr  a
004b2c: 66 3c  ror  $3C
004b2e: 85 3d  sta  $3D
004b30: a5 3a  lda  $3A
004b32: 18     clc
004b33: 65 3c  adc  $3C
004b35: 85 3c  sta  $3C
004b37: 90 02  bcc  $4B3B
004b39: e6 3d  inc  $3D
004b3b: 60     rts

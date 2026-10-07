; Authentic JP/US Stage-2 user-byte window [0x491f, 0x4932).
; MAME 0.285 unidasm -arch h6280 -basepc 0x491f -skip 2897967 -count 19
; JP and US listings matched in three independent runs; listing SHA-256:
; 58516ae71204c206553c168603cf7f2e98d8f4806b3c0c0647bac29f1ff8fff6
; Called from $4a89 in the authentic Stage-2 window; exact bytes are also
; admitted by theron_v1_track02_verify_stage2_enclosing_45xx_callees().

00491f: 03 05     st0  #$05
004921: a5 f3     lda  $F3
004923: 8d 02 00  sta  $0002
004926: a5 f4     lda  $F4
004928: 29 07     and  #$07
00492a: 09 10     ora  #$10
00492c: 85 f4     sta  $F4
00492e: 8d 03 00  sta  $0003
004931: 60        rts

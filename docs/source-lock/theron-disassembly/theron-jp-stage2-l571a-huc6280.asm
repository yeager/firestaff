; Authentic JP/US Stage-2 user-byte window [0x571a, 0x5732).
; Decoded directly from original Track 02 BIN bytes with MAME 0.285 unidasm.
; JP raw offset 2901850; US raw offset 2904202; both 24-byte windows match.
; Span SHA-256 b90beca7963cdbb93612902a015b013e69eedb6c1cc8e77426d2b57ddef7da0a
; Span FNV-1a-64 0c210360aebbf9f7. This is static source evidence only.

00571a: 20 c5 58  jsr  $58C5
00571d: a9 09     lda  #$09
00571f: 8d dc 58  sta  $58DC
005722: a9 10     lda  #$10
005724: 8d dd 58  sta  $58DD
005727: a9 e0     lda  #$E0
005729: 8d de 58  sta  $58DE
00572c: a9 58     lda  #$58
00572e: 8d df 58  sta  $58DF
005731: 60        rts

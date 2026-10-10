; MAME 0.285 unidasm, HuC6280, rooted at logical $4CE1.
; Authentic JP Rev. 1 and US Track 02 produce identical 45-byte windows.
; The window includes the shared BRK destination at $4D0D.
004ce1: ad 7b 4d  lda  $4D7B
004ce4: 8d c1 4e  sta  $4EC1
004ce7: 20 c9 4e  jsr  $4EC9
004cea: b0 21     bcs  $4D0D
004cec: a9 06     lda  #$06
004cee: 20 5e 4f  jsr  $4F5E
004cf1: b0 1a     bcs  $4D0D
004cf3: c6 5b     dec  $5B
004cf5: ad c2 4e  lda  $4EC2
004cf8: 8d cc 37  sta  $37CC
004cfb: ad c7 4e  lda  $4EC7
004cfe: 8d d0 37  sta  $37D0
004d01: ad c8 4e  lda  $4EC8
004d04: 8d d1 37  sta  $37D1
004d07: 20 76 38  jsr  $3876
004d0a: 64 5b     stz  $5B
004d0c: 60        rts
004d0d: 00        brk

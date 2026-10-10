; MAME 0.285 unidasm, HuC6280, rooted at logical $4C71.
; Authentic JP Rev. 1 and US Track 02 produce identical 112-byte windows.
; This is a static candidate listing, not runtime source-sector provenance.
004c71: ad c3 4e  lda  $4EC3
004c74: 85 0e     sta  $0E
004c76: ad c4 4e  lda  $4EC4
004c79: 85 0f     sta  $0F
004c7b: a9 00     lda  #$00
004c7d: 85 2e     sta  $2E
004c7f: 85 30     sta  $30
004c81: 85 10     sta  $10
004c83: a9 60     lda  #$60
004c85: 85 2f     sta  $2F
004c87: 85 31     sta  $31
004c89: 85 11     sta  $11
004c8b: ad c7 4e  lda  $4EC7
004c8e: 85 12     sta  $12
004c90: ad c8 4e  lda  $4EC8
004c93: 85 13     sta  $13
004c95: a9 47     lda  #$47
004c97: 85 00     sta  $00
004c99: a9 34     lda  #$34
004c9b: 85 01     sta  $01
004c9d: 20 a1 33  jsr  $33A1
004ca0: 20 83 35  jsr  $3583
004ca3: 20 59 3c  jsr  $3C59
004ca6: 20 86 3b  jsr  $3B86
004ca9: 20 6e 3c  jsr  $3C6E
004cac: ad 7c 3b  lda  $3B7C
004caf: 8d 06 30  sta  $3006
004cb2: ad 7d 3b  lda  $3B7D
004cb5: 8d 07 30  sta  $3007
004cb8: ad c5 4e  lda  $4EC5
004cbb: 8d 04 30  sta  $3004
004cbe: ad c6 4e  lda  $4EC6
004cc1: 8d 05 30  sta  $3005
004cc4: a9 00     lda  #$00
004cc6: 8d 02 30  sta  $3002
004cc9: a9 60     lda  #$60
004ccb: 8d 03 30  sta  $3003
004cce: 20 fc 36  jsr  $36FC
004cd1: 68        pla
004cd2: 53 40     tam  #$40
004cd4: 68        pla
004cd5: 53 20     tam  #$20
004cd7: 68        pla
004cd8: 53 10     tam  #$10
004cda: 68        pla
004cdb: 53 08     tam  #$08
004cdd: 18        clc
004cde: 64 5b     stz  $5B
004ce0: 60        rts

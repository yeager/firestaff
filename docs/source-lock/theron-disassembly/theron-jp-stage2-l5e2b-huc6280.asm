; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5e2b, 86 bytes, ending before the dispatch table at $5e81.
; MAME: unidasm -arch h6280 -basepc 0x5e2b -count 0x56
; Raw window SHA-256: 02f993418bd7a54dc2679854de4f98bc61f43b4fbc676978305c37bd9f3e4a9e
; Listing SHA-256: ad52bc8b458bbbb570813fa1190ed67ab63def1a340f1e856591d82a5ed299e5

005e2b: ad 78 3b  lda  $3B78
005e2e: d0 01     bne  $5E31
005e30: 60        rts
005e31: ad 79 3b  lda  $3B79
005e34: 0a        asl  a
005e35: aa        tax
005e36: 7c 81 5e  jmp  ($5E81),X
005e39: 03 06     st0  #$06
005e3b: ad 76 3b  lda  $3B76
005e3e: d0 01     bne  $5E41
005e40: 1a        ina
005e41: 18        clc
005e42: 69 3f     adc  #$3F
005e44: 8d 02 00  sta  $0002
005e47: ad 77 3b  lda  $3B77
005e4a: 69 00     adc  #$00
005e4c: 8d 03 00  sta  $0003
005e4f: 03 07     st0  #$07
005e51: ad 0c 22  lda  $220C
005e54: 8d 02 00  sta  $0002
005e57: ad 0d 22  lda  $220D
005e5a: 8d 03 00  sta  $0003
005e5d: ee 79 3b  inc  $3B79
005e60: 60        rts
005e61: 03 06     st0  #$06
005e63: ad 74 3b  lda  $3B74
005e66: d0 01     bne  $5E69
005e68: 1a        ina
005e69: 18        clc
005e6a: 69 3f     adc  #$3F
005e6c: 8d 02 00  sta  $0002
005e6f: ad 75 3b  lda  $3B75
005e72: 69 00     adc  #$00
005e74: 8d 03 00  sta  $0003
005e77: 03 07     st0  #$07
005e79: 13 00     st1  #$00
005e7b: 23 00     st2  #$00
005e7d: 9c 79 3b  stz  $3B79
005e80: 60        rts

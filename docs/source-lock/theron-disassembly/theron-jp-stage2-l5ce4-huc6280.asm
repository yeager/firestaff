; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5ce4, 56 bytes; the BVC target is the $5d1c boundary.
; MAME: unidasm -arch h6280 -basepc 0x5ce4 -count 0x38
; Raw window SHA-256: f9732c087fd6f61fcc5449282154b83b4f7ac75595375f1a0c5df1a6a886a8cf
; Listing SHA-256: b7a903a0b6a52d1a51f7f3637e41c382cf7262ffece0f9204eb55473be63d54f

005ce4: 2c 69 3b              bit  $3B69
005ce7: 50 33                 bvc  $5D1C
005ce9: a9 e0                 lda  #$E0
005ceb: 85 5c                 sta  $5C
005ced: a9 58                 lda  #$58
005cef: 85 5d                 sta  $5D
005cf1: 9c e0 58              stz  $58E0
005cf4: 73 e0 58 e1 58 ff 03  tii  $58E0 $58E1 $03FF
005cfb: a2 07                 ldx  #$07
005cfd: 9e 16 5e              stz  $5E16,x
005d00: 9e 1e 5e              stz  $5E1E,x
005d03: ca                    dex
005d04: 10 f7                 bpl  $5CFD
005d06: ad 69 3b              lda  $3B69
005d09: 29 bf                 and  #$BF
005d0b: 8d 69 3b              sta  $3B69
005d0e: a9 01                 lda  #$01
005d10: 8d 27 5e              sta  $5E27
005d13: a9 08                 lda  #$08
005d15: 8d 28 5e              sta  $5E28
005d18: 9c 26 5e              stz  $5E26
005d1b: 60                    rts

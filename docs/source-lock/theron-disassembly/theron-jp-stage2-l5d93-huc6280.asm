; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5d93, 72 bytes; RTS at $5dda, exclusive end $5ddb.
; MAME: unidasm -arch h6280 -basepc 0x5d93 -count 0x48
; Raw window SHA-256: e6bd85ceb98a37737b5d8e0002aeaff2f5029978ff998ded34678e1240413339
; Listing SHA-256: 65c10648c69067bcb7b20774d9a3923bbda1fecac4754d12ca1868582554288c

005d93: c2                    cly
005d94: a2 10                 ldx  #$10
005d96: da                    phx
005d97: 64 60                 stz  $60
005d99: c8                    iny
005d9a: b1 5e                 lda  ($5E),y
005d9c: 4a                    lsr  a
005d9d: 88                    dey
005d9e: b1 5e                 lda  ($5E),y
005da0: 48                    pha
005da1: 2a                    rol  a
005da2: 2a                    rol  a
005da3: 2a                    rol  a
005da4: 29 07                 and  #$07
005da6: aa                    tax
005da7: bd 1e 5e              lda  $5E1E,x
005daa: f0 02                 beq  $5DAE
005dac: e7 60                 smb6 $60
005dae: 68                    pla
005daf: 48                    pha
005db0: 4a                    lsr  a
005db1: 4a                    lsr  a
005db2: 4a                    lsr  a
005db3: 29 07                 and  #$07
005db5: aa                    tax
005db6: bd 1e 5e              lda  $5E1E,x
005db9: f0 02                 beq  $5DBD
005dbb: b7 60                 smb3 $60
005dbd: 68                    pla
005dbe: 29 07                 and  #$07
005dc0: aa                    tax
005dc1: bd 1e 5e              lda  $5E1E,x
005dc4: f0 02                 beq  $5DC8
005dc6: 87 60                 smb0 $60
005dc8: b1 5c                 lda  ($5C),y
005dca: 18                    clc
005dcb: 65 60                 adc  $60
005dcd: 91 5c                 sta  ($5C),y
005dcf: 62                    cla
005dd0: c8                    iny
005dd1: 71 5c                 adc  ($5C),y
005dd3: 91 5c                 sta  ($5C),y
005dd5: c8                    iny
005dd6: fa                    plx
005dd7: ca                    dex
005dd8: d0 bc                 bne  $5D96
005dda: 60                    rts

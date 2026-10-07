; Authentic Theron's Quest JP Rev. 1 Track 02 Stage-2 source.
; User-data offset $5df5, 33 bytes; RTS at $5e15, exclusive end $5e16.
; MAME: unidasm -arch h6280 -basepc 0x5df5 -count 0x21
; Raw window SHA-256: aa37228d4a9401afabc88c37ee57a7b18dc29a7c0cba3b35a9f44d635ef098dd
; Listing SHA-256: 193482b058801ab505ffd49fc53c1556aba57e962c6ba1f63a5e6832ff9242fa

005df5: a5 5c                 lda  $5C
005df7: 8d 0f 5e              sta  $5E0F
005dfa: a5 5d                 lda  $5D
005dfc: 8d 10 5e              sta  $5E10
005dff: ad 29 5e              lda  $5E29
005e02: 0a                    asl  a
005e03: 0a                    asl  a
005e04: 0a                    asl  a
005e05: 0a                    asl  a
005e06: 8d 02 04              sta  $0402
005e09: 62                    cla
005e0a: 2a                    rol  a
005e0b: 8d 03 04              sta  $0403
005e0e: e3 00 00 04 04 20 00  tia  $0000 $0404 $0020
005e15: 60                    rts

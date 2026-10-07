; Authentic JP Track 02 candidate bytes at the observed $44fb call target.
; Track 02 SHA-256: d076b2dd64476256803e84985f10c1b4460364dd064ba351c2b7bc89d70d09fb
; The six raw candidate offsets for the $489f window are documented in
; theron-3879-runtime-window-candidates-20261007.md.
; This candidate-relative alignment subtracts $3a4 in MODE1 user-data space
; from each candidate window before converting back to raw-sector offsets.
; Resulting raw offsets: 610459, 911515, 1212571, 1513627, 1814683, 2115739.
; Each offset has the same 33 authentic bytes, SHA-256:
; 30c4e29752ff4fc5363876072d67f7b1f8e68f96af208064f0ee0e58fc62463d
; MAME 0.285 unidasm -arch h6280 -basepc 0x44fb -skip 610459 -count 33.
; Listing SHA-256: d05c8c2848469977cbfa23f87de74b916dcda10514d4b9f6d61dae41256eac01
; This static candidate does not prove a loader transfer, CD-RAM contents,
; execution of this body, or any routine/gameplay semantics.
; BEGIN MAME OUTPUT
0044fb: 08     php
0044fc: 48     pha
0044fd: 18     clc
0044fe: 65 00  adc  $00
004500: 53 20  tam  #$20
004502: 68     pla
004503: 28     plp
004504: 60     rts
004505: 08     php
004506: 48     pha
004507: 18     clc
004508: 65 00  adc  $00
00450a: 53 40  tam  #$40
00450c: 68     pla
00450d: 28     plp
00450e: 60     rts
00450f: 08     php
004510: 43 08  tma  #$08
004512: 80 0d  bra  $4521
004514: 08     php
004515: 43 10  tma  #$10
004517: 80 08  bra  $4521
004519: 08     php
00451a: 43 20  tma  #$20
; END MAME OUTPUT

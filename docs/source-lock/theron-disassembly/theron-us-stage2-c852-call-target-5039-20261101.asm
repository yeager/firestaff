; Static stage-two candidate at the direct target of the US Track 02
; $C852 source window's JSR at $C85E.
;
; Authentic US Track 02: MD5 f23601102138f87c33025877767ebf76
;   stage-two user-data starts at raw file offset $2BED90, loaded at $4000
;   logical $5039 -> raw file offset $2BFDC9
; Authentic JP Rev. 1 Track 02: MD5 b7afb338ad31be1025b53f9aff12d73a
;   stage-two user-data starts at raw file offset $2BE460, loaded at $4000
;   logical $5039 -> raw file offset $2BF499
; Both editions contain these exact eight bytes (SHA-256
; 1035acb7b3a19a175bbd61cc1174bf0a6d902cded7e87fd8a01f23ec5eb0e859).
;
; The US caller bytes at $C85E are statically `JSR $5039`, and the caller
; window at raw offset $A3AD2 was matched to an authentic runtime fetch.
; Runtime MPR2 at the call, execution at $5039, the $3879 callee mapping,
; and gameplay meaning are not established by these static source bytes.

        .setcpu "huc6280"

L5039:  and     ($4F),y
        jsr     L3879
        stz     $5B
        rts

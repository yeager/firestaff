; Theron's Quest US Track 02 — authenticated regular-spawn helper contract
;
; Raw source: retail US Track 02 BIN, MD5
;   f23601102138f87c33025877767ebf76
; File offset: $9c4e7 (the same 25-byte span occurs in each dungeon-bank copy)
; HuC6280 entry: $4667
; FNV-1a: $b9075b31
;
; The dynamic overlay is now authenticated separately: a real Cocoa-input
; Mednafen run captured the $45E3 code window at physical PC $000d05e3,
; 256 bytes, FNV-1a $cd08af95, and the $5D64 window at physical PC $000d1d64,
; 256 bytes, FNV-1a $38ef5dd1.  The exact $4667 state transition is:
;   L4667: LDA $28B9 / ASL / LDA $28B9 / ROL / ADC #$4E / EOR #$3A
;          STA $28B9 / EOR $28BA / ADC #$C3 / STA $28BA / RTS
; HuC6280 carry is preserved across LDA/EOR, so the two ADC carry inputs are
; part of the authenticated consumer contract.  The adjacent $4644 and $464A
; consumers return $4667&1 and $4667&3 respectively.  Firestaff mirrors these
; byte-level operations in theron_v1_rng_source.c.
;
; The $5D6A/$5D64 callers still have separate gameplay ownership questions;
; decoding this RNG transition does not by itself authorize spawn, AI, loot,
; T700 or T900 publication.
;
; The same bounded real capture also admitted raw overlay windows for helper
; callers reached from $B0E5: $5A76 (FNV-1a $6869c445), $5B8F
; ($88078c02), $5BA5 ($2c323453), and $D23A ($c63dc0fe), each 256 bytes at
; physical PCs $000d1a76, $000d1b8f, $000d1ba5 and $000d723a.  These windows
; are preserved as disassembly evidence only; their table/state owners are
; not promoted until the same execution binds their return values to a real
; category-0..3 spawn record.
;
; Adjacent preconsumer L4644 is locked separately at raw file offset $9c4c4,
; 27 bytes, FNV-1a $a3c3f7eb. It prepares the arguments and calls the
; consumers at $C96B/$CC4C. Their US-BIN byte spans are now statically
; bounded and hash-verified in theron-us-rng-consumers.asm, but their runtime
; bank/callee state and semantic return contract remain unresolved.

        .setcpu "huc6280"
        .org    $4667

L4667:  lda     $B3
        and     #$07
        cmp     #$04
        bne     L4680
        jsr     L5D6A
        lda     #$02
        sta     $8A
        lda     #$04
        ldx     $40
        ldy     $41
        jsr     L5D64
        rts

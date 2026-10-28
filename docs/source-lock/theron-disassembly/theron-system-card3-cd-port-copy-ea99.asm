; Authentic System Card 3.0 instruction window for the CD-to-RAM byte copy.
; This is a five-byte disassembly excerpt, not a BIOS image.
;
; BIOS identity: MD5 ff1a674273fe3540ccef576376407d1d, 262656 bytes.
; Mednafen source: src/pce_fast/huc.cpp, HuC_LoadCD() lines 283-301.
; HuC_LoadCD skips 512 bytes when the file size has bit 512 set, then loads
; 262144 bytes into ROMSpace. With MPR7=00, physical $0A99 maps logical $EA99,
; so the bytes at file offset $0C99 decode at the observed CPU address.
; Disassembler: MAME unidasm -arch h6280, run on trv2 from a bounded window.
;
; Runtime contract in this window:
;   $EA99 reads the byte at CD data port $1808 into A.
;   $EA9C stores A through the zero-page pointer $FA/$FB plus Y.
; The dynamic destination is checked separately from authentic runtime traces.

ea99: ad 08 18    lda $1808
ea9c: 91 fa       sta ($FA),y

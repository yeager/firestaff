# CSB V1 F0168 final message-area ownership

Status: **source-owned runtime path closed**

ReDMCSB `MOVESENS.C` F0276 decodes a visible C02 TextString through F0168 when
the party enters its square. `TEXT.C` then owns its presentation in the C015
message area. Firestaff already retained those decoded bytes in the selected
CSB runtime receipt and translated them only at the final `csb` gettext
boundary.

The main draw path nevertheless called the generic M11 message renderer after
the CSB HUD pass. That renderer cleared C015 and could replace the authentic
receipt with host telemetry. The final message-area dispatch now recognizes a
CSB session, invokes only the source-owned receipt renderer, and returns before
the generic log path. Missing source bytes or source font still leave C015
black; no replacement text is generated.

The runtime remains restricted to supported Atari ST, Amiga, and FM Towns CSB
media. No PC or DOS CSB route is introduced.

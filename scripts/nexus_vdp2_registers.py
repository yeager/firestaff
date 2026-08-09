"""Decode VDP2 register words from both authenticated capture revisions.

The external Mednafen probes that produced the retained witnesses did not
serialize ``uint16`` register words identically.  The raw payload remains the
authoritative witness; this module only selects the byte order for register
field inspection.  It never changes VRAM/CRAM bytes or grants semantic
admission.
"""

from __future__ import annotations


def read_u16(registers: bytes, offset: int, byte_order: str) -> int:
    if byte_order not in ("big", "little"):
        raise ValueError(f"unsupported VDP2 register byte order: {byte_order}")
    return int.from_bytes(registers[offset : offset + 2], byte_order)


def _score(registers: bytes, byte_order: str) -> int:
    tvmd = read_u16(registers, 0x00, byte_order)
    bgon = read_u16(registers, 0x20, byte_order)
    chctla = read_u16(registers, 0x28, byte_order)
    score = 0

    # Saturn TVMD display-enable and BGON layer bits are the strongest
    # discriminator between the two retained Mednafen serialization forms.
    if tvmd & 0x8000:
        score += 3
    if bgon & 0x001F:
        score += 4
    if (bgon & ~0x1F3F) == 0:
        score += 1
    if bgon & 0x0002:
        score += 2
        if chctla & 0x0200:
            score += 1
    return score


def detect_byte_order(registers: bytes) -> str:
    """Return the more plausible register serialization for one frame."""
    if len(registers) < 0x2A:
        raise ValueError("VDP2 register region is truncated")
    scores = {order: _score(registers, order) for order in ("big", "little")}
    if scores["big"] == scores["little"]:
        # The current producer writes native little-endian words.  This is
        # only a deterministic tie-break; callers still report the choice.
        return "little"
    return max(scores, key=scores.get)


def read_detected_u16(registers: bytes, offset: int) -> tuple[int, str]:
    byte_order = detect_byte_order(registers)
    return read_u16(registers, offset, byte_order), byte_order

"""Small Python compatibility helpers used by standalone Theron tools."""

from typing import Any, Iterator


def zip_strict(*iterables: Any) -> Iterator[tuple[Any, ...]]:
    """Zip sized iterables and reject mismatched lengths on Python 3.9+."""
    lengths = [len(iterable) for iterable in iterables]
    if lengths and any(length != lengths[0] for length in lengths[1:]):
        raise ValueError("zip_strict arguments must have equal lengths")
    return zip(*iterables)

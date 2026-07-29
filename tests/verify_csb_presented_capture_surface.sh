#!/bin/sh
# Shared assertion for real PC3.4 CSB startup captures.  Keep this POSIX-only:
# these end-to-end tests intentionally run on every release host with only
# shell, od, awk and the executable itself available.

verify_csb_presented_capture_surface() {
    capture="$1"
    dimensions="$(od -An -v -tu1 -j 18 -N 8 "$capture" | awk '
        {
            for (i = 1; i <= NF; ++i) bytes[count++] = $i;
        }
        END {
            if (count == 8) {
                width = bytes[0] + 256 * bytes[1] + 65536 * bytes[2] + 16777216 * bytes[3];
                height = bytes[4] + 256 * bytes[5] + 65536 * bytes[6] + 16777216 * bytes[7];
                # Firestaff writes top-down BMPs, whose biHeight is negative.
                if (height >= 2147483648) height = 4294967296 - height;
                print width, height;
            }
        }
    ')"
    set -- $dimensions
    width="${1:-0}"
    height="${2:-0}"
    if [ "$width" -le 0 ] || [ "$height" -le 0 ]; then
        echo "FAIL: CSB startup capture has invalid BMP geometry: $capture" >&2
        return 1
    fi

    row_bytes=$((width * 3))
    padded_row_bytes=$(((row_bytes + 3) / 4 * 4))
    expected_bytes=$((54 + padded_row_bytes * height))
    capture_bytes="$(wc -c < "$capture" | tr -d ' ')"
    if [ "$capture_bytes" -ne "$expected_bytes" ]; then
        echo "FAIL: CSB startup capture has wrong BMP size: $capture ($capture_bytes)" >&2
        return 1
    fi

    # A valid header and four files are insufficient: a host-scaling failure
    # can retain a narrow source strip inside an otherwise valid BMP.  Each
    # startup source page must span at least half of the emitted surface.
    if ! od -An -v -tu1 -j 54 "$capture" | awk -v width="$width" -v height="$height" '
        {
            for (i = 1; i <= NF; ++i) {
                channel = pixel_byte % 3;
                value[channel] = $i;
                ++pixel_byte;
                if (channel != 2) continue;
                if (value[0] != 0 || value[1] != 0 || value[2] != 0) {
                    x = pixel % width;
                    y = int(pixel / width);
                    if (!seen || x < min_x) min_x = x;
                    if (!seen || x > max_x) max_x = x;
                    if (!seen || y < min_y) min_y = y;
                    if (!seen || y > max_y) max_y = y;
                    seen = 1;
                }
                ++pixel;
            }
        }
        END {
            if (!seen || max_x - min_x + 1 < width / 2 || max_y - min_y + 1 < height / 2)
                exit 1;
        }
    '; then
        echo "FAIL: CSB startup capture collapsed to a partial presentation surface: $capture" >&2
        return 1
    fi
}

#!/usr/bin/env bash

# Copy and authenticate the exact backup-RAM file that Mednafen will open.
theron_prepare_authentic_bram() {
    local source_path=$1
    local destination_dir=$2
    local expected_md5=$3
    local destination_path="$destination_dir/firestaff-theron-authentic.sav"
    local destination_size
    local actual_md5

    if ! cp "$source_path" "$destination_path"; then
        printf '%s\n' 'FAIL: could not copy THERON_CAPTURE_AUTHENTIC_BRAM into the private capture profile' >&2
        return 1
    fi
    destination_size=$(wc -c < "$destination_path" | tr -d '[:space:]')
    if [[ "$destination_size" != 2048 ]]; then
        rm -f "$destination_path"
        printf '%s\n' 'FAIL: THERON_CAPTURE_AUTHENTIC_BRAM must name a 2048-byte original Backup RAM image' >&2
        return 1
    fi
    actual_md5=$(md5_file "$destination_path") || {
        rm -f "$destination_path"
        printf '%s\n' 'FAIL: could not hash the private authentic Backup RAM copy' >&2
        return 1
    }
    if [[ "$actual_md5" != "$expected_md5" ]]; then
        rm -f "$destination_path"
        printf '%s\n' 'FAIL: THERON_CAPTURE_AUTHENTIC_BRAM is not the authenticated USA Backup RAM image' >&2
        return 1
    fi

    printf '%s\n' 'firestaff-theron-authentic.sav'
}

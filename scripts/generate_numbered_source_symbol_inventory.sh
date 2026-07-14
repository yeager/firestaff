#!/usr/bin/env bash
# Generate the complete numbered-symbol audit table.  ReDMCSB is external;
# this script intentionally takes its source directory as an argument.
set -euo pipefail

if [ "$#" -ne 1 ] || [ ! -d "$1" ]; then
    echo "usage: $0 /path/to/ReDMCSB/Toolchains/Common/Source" >&2
    exit 2
fi

source_dir=$1
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
output="$repo_root/docs/reference/numbered_source_symbol_inventory.tsv"
tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-numbered-symbols.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT

# The suffix distinguishes numbered ReDMCSB symbols from ordinary constants.
rg -n -o --no-heading '\b[A-Z][0-9]{3,4}_[A-Za-z_][A-Za-z0-9_]*' "$source_dir" \
    | awk -F: '
        {
            sym=$NF; file=$1; sub(/^.*\//, "", file);
            key=sym SUBSEP file;
            if (!seen[key]++) {
                if (files[sym] != "") files[sym]=files[sym] ";" file;
                else files[sym]=file;
            }
        }
        END { for (sym in files) print sym "\t" files[sym] }
    ' | LC_ALL=C sort > "$tmp_dir/source.tsv"

rg --no-filename -o '\b[A-Z][0-9]{3,4}(?:_[A-Za-z_][A-Za-z0-9_]*)?' \
    "$repo_root/src" "$repo_root/include" "$repo_root/tests" 2>/dev/null \
    | awk -F_ '{ print $1 }' | LC_ALL=C sort -u > "$tmp_dir/firestaff-symbols.txt" || true

printf 'family\tid\tsymbol\tredmcsb_files\tstatus\n' > "$output"
awk -F '\t' '
    NR == FNR { referenced[$1]=1; next }
    {
        split($1, parts, "_"); prefix=parts[1]; family=substr(prefix, 1, 1);
        id=substr(prefix, 2);
        if (family == "C" || family == "G" || family == "L" || family == "P")
            status="semantic_mapping_not_audited";
        else if (family == "A" || family == "M")
            status="non_executable_symbol_not_audited";
        else if (referenced[prefix])
            status="referenced_not_verified";
        else
            status="unmapped_or_unverified";
        print family "\t" id "\t" $1 "\t" $2 "\t" status;
    }
' "$tmp_dir/firestaff-symbols.txt" "$tmp_dir/source.tsv" | LC_ALL=C sort -t $'\t' -k1,1 -k2,2n >> "$output"

echo "wrote $output"

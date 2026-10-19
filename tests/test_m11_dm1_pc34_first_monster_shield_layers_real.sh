#!/usr/bin/env bash
set -euo pipefail

probe=${1:?usage: test_m11_dm1_pc34_first_monster_shield_layers_real.sh <probe>}
if [[ ! -x "$probe" ]]; then
    echo 'FAIL: PC34 original-media combat probe is unavailable' >&2
    exit 1
fi

run_probe() {
    local shield=$1 output status=0
    output=$(FIRESTAFF_TEST_PARTY_SHIELD="$shield" "$probe" 2>&1) || status=$?
    if [[ $status -ne 0 ]]; then
        printf '%s\n' "$output" >&2
        return "$status"
    fi
    printf '%s\n' "$output"
}

baseline=$(run_probe 0)
shielded=$(run_probe 100)
printf '%s\n%s\n' "$baseline" "$shielded"
baseline_damage=$(sed -n 's/.*naturalDamage=\([0-9][0-9]*\).*/\1/p' <<<"$baseline" | tail -n 1)
shielded_damage=$(sed -n 's/.*naturalDamage=\([0-9][0-9]*\).*/\1/p' <<<"$shielded" | tail -n 1)
if [[ -z "$baseline_damage" || -z "$shielded_damage" ||
      "$baseline_damage" -le 0 || "$shielded_damage" -ge "$baseline_damage" ]]; then
    printf 'FAIL: original group damage was not reduced by live party shield (base=%s shielded=%s)\n' \
        "${baseline_damage:-missing}" "${shielded_damage:-missing}" >&2
    exit 1
fi
printf 'PASS: original PC34 group attack reduces damage with live party ShieldDefense (%s -> %s HP)\n' \
    "$baseline_damage" "$shielded_damage"

#!/usr/bin/env bash
set -euo pipefail
v(){ local k=$1 x; [[ $(grep -Ec "^$k=" "$plan") -eq 1 ]]||return 1;x=$(grep -E "^$k=" "$plan");printf %s "${x#*=}";}; h(){ od -An -tx1 -j "$2" -N "$3" "$1"|tr -d ' \n';}; q(){ [[ $(h "$cap" "$2" 8) == $(printf '%016s' "$1"|tr ' ' 0|tr '[:upper:]' '[:lower:]') ]];}; n(){ printf '%d' "$((16#$1))";}
while (($#));do case "$1" in --plan|--capture)(($#>=2))||exit 2;k=${1#--};printf -v "$k" %s "$2";shift 2;;*)exit 2;;esac;done
: "${plan:?}" "${capture:?}";[[ -f "$plan" && -f "$capture" && $(wc -c < "$capture") -ge 97 && $(head -c 8 "$capture")==NXSLSC01 ]]||exit 1;cap=$capture
[[ $(h "$cap" 8 4)==00000001 && $(h "$cap" 12 4)==00000060 ]];q "$(v route_epoch)" 16;q "$(v package_fnv1a64)" 24;q "$(v card_fnv1a64)" 32;q "$(v task_trace_fnv1a64)" 40;q "$(v sal_descriptor_fnv1a64)" 48;q "$(v map_table_fnv1a64)" 56;q "$(v sddrvs_fnv1a64)" 64
offset=$(n "$(h "$cap" 72 4)");length=$(n "$(h "$cap" 76 4)");[[ $offset -eq 96 && $length -gt 0 && $((offset + length)) -eq $(wc -c < "$cap") ]]

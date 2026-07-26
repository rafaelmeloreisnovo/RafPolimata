#!/usr/bin/env bash
set -Eeuo pipefail

ELF="${1:-}"
[[ -n "$ELF" ]] || { echo "Uso: $0 caminho/artefato.elf" >&2; exit 64; }
[[ -f "$ELF" ]] || { echo "FALHA: ELF ausente: $ELF" >&2; exit 66; }

READELF="${READELF:-readelf}"
NM="${NM:-nm}"

fail() { echo "strict-elf: FAIL — $*" >&2; exit 1; }

hdr="$($READELF -h "$ELF")"
ph="$($READELF -l "$ELF")"
dyn="$($READELF -d "$ELF" 2>&1 || true)"
sec="$($READELF -S "$ELF")"
notes="$($READELF -n "$ELF" 2>&1 || true)"
rel="$($READELF -r "$ELF" 2>&1 || true)"

if ! grep -Eq 'Type:[[:space:]]+EXEC' <<<"$hdr"; then
  [[ "${RAF_ALLOW_DYN:-0}" == 1 ]] || fail 'tipo ELF não é EXEC'
fi

grep -q 'INTERP' <<<"$ph" && fail 'PT_INTERP presente'
grep -q 'DYNAMIC' <<<"$ph" && fail 'PT_DYNAMIC presente'
grep -q '(NEEDED)' <<<"$dyn" && fail 'DT_NEEDED presente'
grep -q 'Build ID:' <<<"$notes" && fail 'build-id presente'

if "$NM" -u "$ELF" | grep -q '[^[:space:]]'; then
  "$NM" -u "$ELF" >&2 || true
  fail 'símbolo indefinido presente'
fi

if grep -Eq 'R_[A-Za-z0-9_]+' <<<"$rel"; then
  fail 'relocação residual presente'
fi

for forbidden in .eh_frame .gcc_except_table .init_array .fini_array; do
  grep -Fq "$forbidden" <<<"$sec" && fail "seção proibida presente: $forbidden"
done

printf 'strict-elf: PASS — %s\n' "$ELF"

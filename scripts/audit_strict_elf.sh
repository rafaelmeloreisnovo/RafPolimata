#!/usr/bin/env bash
set -Eeuo pipefail

usage() {
  echo "Uso: $0 [--profile exec|android-so] caminho/artefato.elf" >&2
  exit 64
}

PROFILE="exec"
if [[ "${1:-}" == "--profile" ]]; then
  [[ $# -ge 3 ]] || usage
  PROFILE="$2"
  shift 2
fi
ELF="${1:-}"
[[ -n "$ELF" ]] || usage
[[ -f "$ELF" ]] || { echo "strict-elf: FAIL — ELF ausente: $ELF" >&2; exit 66; }
[[ "$PROFILE" == "exec" || "$PROFILE" == "android-so" ]] || usage

READELF_BIN="${READELF:-llvm-readelf}"
command -v "$READELF_BIN" >/dev/null 2>&1 || READELF_BIN="readelf"
NM_BIN="${NM:-llvm-nm}"
command -v "$NM_BIN" >/dev/null 2>&1 || NM_BIN="nm"
command -v "$READELF_BIN" >/dev/null 2>&1 || { echo 'strict-elf: FAIL — readelf ausente' >&2; exit 69; }
command -v "$NM_BIN" >/dev/null 2>&1 || { echo 'strict-elf: FAIL — nm ausente' >&2; exit 69; }

fail() { echo "strict-elf: FAIL — $*" >&2; exit 1; }

hdr="$($READELF_BIN -hW "$ELF")"
ph="$($READELF_BIN -lW "$ELF")"
dyn="$($READELF_BIN -dW "$ELF" 2>&1 || true)"
sec="$($READELF_BIN -SW "$ELF")"
notes="$($READELF_BIN -nW "$ELF" 2>&1 || true)"
rel="$($READELF_BIN -rW "$ELF" 2>&1 || true)"

case "$PROFILE" in
  exec)
    grep -Eq 'Type:[[:space:]]+EXEC' <<<"$hdr" || fail 'perfil exec exige ELF EXEC'
    grep -q 'DYNAMIC' <<<"$ph" && fail 'PT_DYNAMIC presente no executável estrito'
    ;;
  android-so)
    grep -Eq 'Type:[[:space:]]+DYN' <<<"$hdr" || fail 'perfil android-so exige ELF DYN'
    grep -q 'DYNAMIC' <<<"$ph" || fail 'Android .so sem PT_DYNAMIC/loader metadata'
    grep -q '(SONAME)' <<<"$dyn" || fail 'Android .so sem DT_SONAME'
    ;;
esac

grep -q 'INTERP' <<<"$ph" && fail 'PT_INTERP presente'
grep -q '(NEEDED)' <<<"$dyn" && fail 'DT_NEEDED presente'
grep -Eq '\((RPATH|RUNPATH|TEXTREL)\)' <<<"$dyn" && fail 'RPATH/RUNPATH/TEXTREL presente'
grep -q 'Build ID:' <<<"$notes" && fail 'build-id presente'

if awk '$1 == "LOAD" && ($0 ~ /RWE/ || $0 ~ /R W E/ || $0 ~ /RWX/) { bad=1 } END { exit bad ? 0 : 1 }' <<<"$ph"; then
  fail 'segmento LOAD simultaneamente gravável e executável'
fi
if grep 'GNU_STACK' <<<"$ph" | grep -Eq 'RWE|R W E|RWX'; then
  fail 'pilha executável'
fi

undefined="$($NM_BIN -u "$ELF" 2>/dev/null || true)"
if grep -q '[^[:space:]]' <<<"$undefined"; then
  printf '%s\n' "$undefined" >&2
  fail 'símbolo indefinido presente'
fi

mapfile -t relocation_types < <(awk '$3 ~ /^R_[A-Za-z0-9_]+$/ { print $3 }' <<<"$rel" | sort -u)
if [[ "$PROFILE" == "exec" && ${#relocation_types[@]} -ne 0 ]]; then
  printf 'relocações: %s\n' "${relocation_types[*]}" >&2
  fail 'relocação residual presente no executável estrito'
fi
if [[ "$PROFILE" == "android-so" ]]; then
  for relocation in "${relocation_types[@]}"; do
    case "$relocation" in
      R_AARCH64_RELATIVE|R_ARM_RELATIVE|R_X86_64_RELATIVE|R_RISCV_RELATIVE|R_*_NONE) ;;
      *) fail "relocação não autorizada no Android .so: $relocation" ;;
    esac
  done
fi

for forbidden in .eh_frame .gcc_except_table .init_array .fini_array; do
  grep -Fq "$forbidden" <<<"$sec" && fail "seção proibida presente: $forbidden"
done

if [[ -n "${RAF_EXPECT_MACHINE:-}" ]]; then
  grep -Fq "$RAF_EXPECT_MACHINE" <<<"$hdr" || fail "máquina diferente da esperada: $RAF_EXPECT_MACHINE"
fi

printf 'strict-elf: PASS — profile=%s file=%s\n' "$PROFILE" "$ELF"

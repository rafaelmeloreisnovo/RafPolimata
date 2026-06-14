#!/bin/sh
set -eu
# Uso:
#   sh big_test.sh
#   BIG_TEST_OUT=big_test_runs/manual sh big_test.sh
#
# Orquestra um teste estrutural amplo do repositório RafPolimata.
# O objetivo é gerar evidências, logs e um resumo auditável sem transformar
# ausência de ferramenta/device em sucesso falso. TOKEN_VAZIO é preservado.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

RUN_ID=$(date -u '+%Y%m%dT%H%M%SZ')
OUT=${BIG_TEST_OUT:-"$ROOT/big_test_runs/$RUN_ID"}
LOGS="$OUT/logs"
BIN="$OUT/bin"
RESULTS="$OUT/results"
SUMMARY="$OUT/BIG_TEST_SUMMARY.md"
COMMANDS="$OUT/commands.tsv"
TOOLS="$OUT/tooling.txt"
ENVINFO="$OUT/environment.txt"
ARTIFACTS="$OUT/artifacts"

mkdir -p "$LOGS" "$BIN" "$RESULTS" "$ARTIFACTS"
: > "$SUMMARY"
: > "$COMMANDS"
: > "$TOOLS"
: > "$ENVINFO"

COMMIT=$(git rev-parse --short HEAD 2>/dev/null || echo TOKEN_VAZIO)
BRANCH=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo TOKEN_VAZIO)
DATE_UTC=$(date -u '+%Y-%m-%dT%H:%M:%SZ')

cat > "$SUMMARY" <<EOF
# Big Test — RafPolimata

- Data UTC: $DATE_UTC
- Branch: $BRANCH
- Commit: $COMMIT
- Saída: $OUT

## Matriz de gates

| Gate | Tipo | Status | Log |
|---|---|---|---|
EOF

{
  echo "date_utc=$DATE_UTC"
  echo "branch=$BRANCH"
  echo "commit=$COMMIT"
  echo "root=$ROOT"
  echo "out=$OUT"
  echo "uname=$(uname -a 2>/dev/null || echo TOKEN_VAZIO)"
  echo "shell=$SHELL"
  echo "path=$PATH"
} > "$ENVINFO"

for t in sh bash python3 gcc clang cc git rg unzip aapt readelf apksigner keytool adb; do
  if command -v "$t" >/dev/null 2>&1; then
    printf '%s=%s\n' "$t" "$(command -v "$t")" >> "$TOOLS"
  else
    printf '%s=TOKEN_VAZIO\n' "$t" >> "$TOOLS"
  fi
done

FAILS=0
HARD_FAILS=0

append_gate(){
  gate=$1
  kind=$2
  status=$3
  log=$4
  printf '| %s | %s | %s | `%s` |\n' "$gate" "$kind" "$status" "$log" >> "$SUMMARY"
}

record_command(){
  gate=$1
  cmd=$2
  printf '%s\t%s\n' "$gate" "$cmd" >> "$COMMANDS"
}

run_gate(){
  gate=$1
  kind=$2
  log_name=$3
  cmd=$4
  log="$LOGS/$log_name"
  record_command "$gate" "$cmd"
  set +e
  sh -c "$cmd" > "$log" 2>&1
  rc=$?
  set -e
  if [ "$rc" -eq 0 ]; then
    if grep -q 'TOKEN_VAZIO' "$log" 2>/dev/null; then
      append_gate "$gate" "$kind" TOKEN_VAZIO "logs/$log_name"
    elif grep -q '| .* | FAIL |' "$log" 2>/dev/null || grep -q '^status: FAIL' "$log" 2>/dev/null; then
      append_gate "$gate" "$kind" FAIL "logs/$log_name"
      FAILS=$((FAILS + 1))
      [ "$kind" = required ] && HARD_FAILS=$((HARD_FAILS + 1))
    else
      append_gate "$gate" "$kind" PASS "logs/$log_name"
    fi
  else
    append_gate "$gate" "$kind" FAIL "logs/$log_name"
    FAILS=$((FAILS + 1))
    [ "$kind" = required ] && HARD_FAILS=$((HARD_FAILS + 1))
  fi
}

run_if_exists(){
  gate=$1
  kind=$2
  path=$3
  log_name=$4
  cmd=$5
  if [ -e "$path" ]; then
    run_gate "$gate" "$kind" "$log_name" "$cmd"
  else
    echo "TOKEN_VAZIO: $path ausente." > "$LOGS/$log_name"
    append_gate "$gate" "$kind" TOKEN_VAZIO "logs/$log_name"
    record_command "$gate" "SKIP: $path ausente"
  fi
}

# ── Gates estruturais gerais ─────────────────────────────────────────────
run_if_exists G00 required scripts/validate_coherence_protocol.py validate-coherence.log \
  'python3 scripts/validate_coherence_protocol.py'

run_if_exists G01 required RAF_host_syntax_check.sh host-syntax.log \
  'sh RAF_host_syntax_check.sh'

if [ -f raf_main.c ] && [ -f raf_frontend.c ] && [ -f raf_cpu.c ] && [ -f raf_asm_emit.c ] && [ -f raf_precomp.c ]; then
  run_gate G02 required raf-compile-build.log \
    "gcc -std=c11 -Wall -Wextra -Werror raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c -o '$BIN/raf_compile'"
else
  echo 'TOKEN_VAZIO: fontes raf_compile incompletas para build host.' > "$LOGS/raf-compile-build.log"
  append_gate G02 required TOKEN_VAZIO logs/raf-compile-build.log
fi

if [ -x "$BIN/raf_compile" ]; then
  run_gate G03 required raf-compile-smoke.log \
    "'$BIN/raf_compile' --help && '$BIN/raf_compile' raf_main.c '$RESULTS/ci_out'"
else
  echo 'TOKEN_VAZIO: binário raf_compile não foi gerado.' > "$LOGS/raf-compile-smoke.log"
  append_gate G03 required TOKEN_VAZIO logs/raf-compile-smoke.log
fi

run_if_exists G04 required scripts/test_ops_manifest.sh ops-manifest.log \
  'sh scripts/test_ops_manifest.sh'

run_if_exists G05 optional scripts/audit_repository_structure.py repo-structure.log \
  "python3 scripts/audit_repository_structure.py --depth 5 --output '$RESULTS/repository-structure.json'"

run_if_exists G06 required scripts/android_build_matrix.sh android-plan.log \
  'sh scripts/android_build_matrix.sh --plan'

run_if_exists G07 required scripts/ci_freestanding_audit.sh freestanding-audit.log \
  'sh scripts/ci_freestanding_audit.sh'

# ── Gates ApkC ───────────────────────────────────────────────────────────
run_if_exists A00 required scripts/apkc_validate.sh apkc-validate.log \
  'sh scripts/apkc_validate.sh'

run_if_exists A01 optional scripts/apkc_sign_debug.sh apkc-sign-debug.log \
  'sh scripts/apkc_sign_debug.sh'

run_if_exists A02 optional scripts/apkc_install_android.sh apkc-install-android.log \
  'sh scripts/apkc_install_android.sh'

# ── Relatório P(k), se existir ───────────────────────────────────────────
run_if_exists P00 optional scripts/first_test_pk.py pk-report.log \
  "python3 scripts/first_test_pk.py --output '$RESULTS/first_test_report.json'"

# ── Coleta de artefatos gerados ──────────────────────────────────────────
if [ -d Apkc/proofs/out ]; then
  cp -R Apkc/proofs/out "$ARTIFACTS/apkc-proofs-out" 2>/dev/null || true
fi
if [ -d ci/reports ]; then
  cp -R ci/reports "$ARTIFACTS/ci-reports" 2>/dev/null || true
fi
if [ -d results ]; then
  cp -R results "$ARTIFACTS/repo-results" 2>/dev/null || true
fi

cat >> "$SUMMARY" <<EOF

## Arquivos de controle

| Arquivo | Função |
|---|---|
| \`environment.txt\` | snapshot de ambiente |
| \`tooling.txt\` | ferramentas encontradas ou TOKEN_VAZIO |
| \`commands.tsv\` | comandos executados por gate |
| \`logs/\` | saída bruta por gate |
| \`artifacts/\` | cópia de relatórios/artefatos gerados |

## Política de verdade

- PASS exige comando executado com evidência.
- FAIL exige log capturado.
- TOKEN_VAZIO preserva ausência de ferramenta, device, fonte ou pré-condição.
- Gates opcionais não derrubam o Big Test quando dependem de ambiente externo.
- Gates obrigatórios com erro real encerram com exit não-zero.

## Resultado agregado

- Falhas totais: $FAILS
- Falhas obrigatórias: $HARD_FAILS
EOF

printf '\nBig Test summary: %s\n' "$SUMMARY"
printf 'Logs: %s\n' "$LOGS"
printf 'Artifacts: %s\n' "$ARTIFACTS"

if [ "$HARD_FAILS" -ne 0 ]; then
  exit 1
fi
exit 0

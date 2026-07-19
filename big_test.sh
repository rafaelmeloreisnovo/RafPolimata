#!/bin/sh
set -eu
# Uso:
#   sh big_test.sh
#   BIG_TEST_OUT=big_test_runs/manual sh big_test.sh
#
# Orquestra gates amplos do RafPolimata e preserva ambiente, comandos, logs e
# artefatos. Exit 0 exige ausência de falha e de TOKEN_VAZIO em gate required.

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

RUN_ID=$(date -u '+%Y%m%dT%H%M%SZ')
OUT=${BIG_TEST_OUT:-"$ROOT/big_test_runs/$RUN_ID"}
LOGS="$OUT/logs"
EXEC_ROOT=${BIG_TEST_EXEC_ROOT:-"${TMPDIR:-${TMP:-/tmp}}/raf_big_test_$RUN_ID"}
BIN="$EXEC_ROOT/bin"
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
- Execução binária temporária: $EXEC_ROOT

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
  echo "exec_root=$EXEC_ROOT"
  echo "uname=$(uname -a 2>/dev/null || echo TOKEN_VAZIO)"
  echo "shell=${SHELL:-TOKEN_VAZIO}"
  echo "path=$PATH"
} > "$ENVINFO"

for t in sh bash python3 gcc clang cc git rg unzip aapt readelf llvm-readelf apksigner keytool adb file; do
  if command -v "$t" >/dev/null 2>&1; then
    printf '%s=%s\n' "$t" "$(command -v "$t")" >> "$TOOLS"
  else
    printf '%s=TOKEN_VAZIO\n' "$t" >> "$TOOLS"
  fi
done

FAILS=0
HARD_FAILS=0
REQUIRED_GAPS=0

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

mark_token_vazio(){
  kind=$1
  if [ "$kind" = required ]; then
    REQUIRED_GAPS=$((REQUIRED_GAPS + 1))
  fi
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

  if [ "$rc" -ne 0 ]; then
    append_gate "$gate" "$kind" FAIL "logs/$log_name"
    FAILS=$((FAILS + 1))
    if [ "$kind" = required ]; then
      HARD_FAILS=$((HARD_FAILS + 1))
    fi
    return
  fi

  if grep -q 'TOKEN_VAZIO' "$log" 2>/dev/null; then
    append_gate "$gate" "$kind" TOKEN_VAZIO "logs/$log_name"
    mark_token_vazio "$kind"
    return
  fi

  if grep -q '| .* | FAIL |' "$log" 2>/dev/null || \
     grep -q '^status: FAIL' "$log" 2>/dev/null || \
     grep -q '"state": "FAIL"' "$log" 2>/dev/null; then
    append_gate "$gate" "$kind" FAIL "logs/$log_name"
    FAILS=$((FAILS + 1))
    if [ "$kind" = required ]; then
      HARD_FAILS=$((HARD_FAILS + 1))
    fi
    return
  fi

  append_gate "$gate" "$kind" PASS "logs/$log_name"
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
    mark_token_vazio "$kind"
  fi
}

# ── Gates estruturais gerais ─────────────────────────────────────────────
run_if_exists G00 required scripts/validate_coherence_protocol.py validate-coherence.log \
  'python3 scripts/validate_coherence_protocol.py'

run_if_exists G01 required RAF_host_syntax_check.sh host-syntax.log \
  'sh RAF_host_syntax_check.sh'

if [ -f raf_main.c ] && [ -f raf_frontend.c ] && [ -f raf_cpu.c ] && \
   [ -f raf_asm_emit.c ] && [ -f raf_precomp.c ]; then
  run_gate G02 required raf-compile-build.log \
    "gcc -std=c11 -Wall -Wextra -Werror raf_main.c raf_frontend.c raf_cpu.c raf_asm_emit.c raf_precomp.c -o '$BIN/raf_compile' && chmod +x '$BIN/raf_compile'"
else
  echo 'TOKEN_VAZIO: fontes raf_compile incompletas para build host.' > "$LOGS/raf-compile-build.log"
  append_gate G02 required TOKEN_VAZIO logs/raf-compile-build.log
  record_command G02 'SKIP: fontes raf_compile incompletas'
  mark_token_vazio required
fi

if [ -x "$BIN/raf_compile" ]; then
  run_gate G02M optional raf-compile-metadata.log \
    "if command -v file >/dev/null 2>&1; then file '$BIN/raf_compile'; else echo 'TOKEN_VAZIO: file ausente'; fi"
  run_gate G03 required raf-compile-smoke.log \
    "'$BIN/raf_compile' --help && '$BIN/raf_compile' raf_main.c '$RESULTS/ci_out'"
else
  {
    echo 'TOKEN_VAZIO: raf_compile não está executável neste ambiente.'
    echo "exec_path=$BIN/raf_compile"
    echo 'Em storage noexec, use BIG_TEST_EXEC_ROOT em diretório interno do Termux.'
  } > "$LOGS/raf-compile-smoke.log"
  append_gate G03 required TOKEN_VAZIO logs/raf-compile-smoke.log
  record_command G03 'SKIP: binário não executável'
  mark_token_vazio required
fi

run_if_exists G04 required scripts/test_ops_manifest.sh ops-manifest.log \
  'sh scripts/test_ops_manifest.sh'

run_if_exists G05 required scripts/audit_repository_structure.py repo-structure.log \
  'python3 scripts/audit_repository_structure.py --depth 5'

run_if_exists G05A required tests/test_document_governance.py document-governance-tests.log \
  'python3 -m unittest tests.test_document_governance tests.test_audit_repository_structure tests.test_validate_root_file_decisions'

run_if_exists G05B required scripts/document_governance.py document-governance.log \
  'python3 scripts/document_governance.py --write --print-summary && python3 scripts/document_governance.py --check --print-summary'

run_if_exists G05C required scripts/validate_root_file_decisions.py root-file-decisions.log \
  'python3 scripts/validate_root_file_decisions.py --write results/root-file-decisions-validation.json'

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

# ── Relatório P(k) ──────────────────────────────────────────────────────
run_if_exists P00 optional scripts/first_test_pk.py pk-report.log \
  "python3 scripts/first_test_pk.py --output '$RESULTS/first_test_report.json'"

# ── Coleta de artefatos ─────────────────────────────────────────────────
copy_if_exists(){
  source_path=$1
  target_path=$2
  if [ -e "$source_path" ]; then
    cp -R "$source_path" "$target_path"
  fi
}

copy_if_exists Apkc/proofs/out "$ARTIFACTS/apkc-proofs-out"
copy_if_exists ci/reports "$ARTIFACTS/ci-reports"
copy_if_exists results "$ARTIFACTS/repo-results"
if [ -f "$BIN/raf_compile" ]; then
  cp "$BIN/raf_compile" "$ARTIFACTS/raf_compile"
fi

cat >> "$SUMMARY" <<EOF

## Arquivos de controle

| Arquivo | Função |
|---|---|
| \`environment.txt\` | snapshot de ambiente |
| \`tooling.txt\` | ferramentas encontradas ou TOKEN_VAZIO |
| \`commands.tsv\` | comandos executados por gate |
| \`logs/\` | saída bruta por gate |
| \`artifacts/\` | cópia de relatórios e artefatos |

## Política de verdade

- PASS exige comando executado sem falha e sem TOKEN_VAZIO em gate required.
- FAIL exige exit não-zero ou marcador explícito de falha.
- TOKEN_VAZIO preserva ausência, mas em gate required encerra o Big Test com exit 2.
- Gates opcionais podem permanecer TOKEN_VAZIO quando dependem de device/tool externo.
- Nenhuma cadeia obrigatória usa \`|| true\` para mascarar o comando principal.

## Resultado agregado

- Falhas totais: $FAILS
- Falhas obrigatórias: $HARD_FAILS
- Lacunas obrigatórias TOKEN_VAZIO: $REQUIRED_GAPS
EOF

printf '\nBig Test summary: %s\n' "$SUMMARY"
printf 'Logs: %s\n' "$LOGS"
printf 'Artifacts: %s\n' "$ARTIFACTS"

if [ "$HARD_FAILS" -ne 0 ]; then
  exit 1
fi
if [ "$REQUIRED_GAPS" -ne 0 ]; then
  exit 2
fi
exit 0

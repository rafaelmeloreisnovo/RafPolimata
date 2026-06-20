#!/usr/bin/env bash
# B7 — Claim traceability binding
# Varre docs/*.md em busca de claims fortes (PASS, implementado, EXECUTA_PASS, etc.)
# e verifica se têm referência rastreável via padrão (ref: arquivo:linha) ou (ref: arquivo).
# Claims sem ref são marcados AUDIT; links ref inválidos são FAIL.
#
# Uso:
#   bash scripts/validate_claims.sh             # todos os docs/*.md
#   bash scripts/validate_claims.sh docs/foo.md # arquivo específico
#   CLAIMS_STRICT=1 bash scripts/validate_claims.sh  # falha se AUDIT encontrado

set -euo pipefail

STRICT="${CLAIMS_STRICT:-0}"
REPORT="ci/reports/claims_audit.md"
mkdir -p ci/reports

PASS=0; AUDIT=0; FAIL=0

if [ $# -gt 0 ]; then
    FILES=("$@")
else
    FILES=($(ls docs/*.md docs/arch/*.md 2>/dev/null || true))
fi

{
echo "# Claims Traceability Audit — B7"
echo ""
echo "Data: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
echo ""
echo "| Arquivo | Linha | Claim | Status |"
echo "|---------|:-----:|-------|--------|"
} > "$REPORT"

# Padrões que indicam claims fortes
CLAIM_PAT='(PASS|EXECUTA_PASS|COMPILE_OK|implementado|implementada|verified|proved|provado|evidence)'
# Padrão de referência rastreável
REF_PAT='\(ref:[^)]+\)'

for f in "${FILES[@]}"; do
    [ -f "$f" ] || continue
    lineno=0
    while IFS= read -r line; do
        lineno=$((lineno+1))
        # Linha tem claim forte?
        if echo "$line" | grep -qE "$CLAIM_PAT"; then
            # Linha tem ref rastreável?
            if echo "$line" | grep -qE "$REF_PAT"; then
                # Extrair o ref e verificar se o arquivo existe
                ref=$(echo "$line" | grep -oE '\(ref:[^)]+\)' | head -1)
                reffile=$(echo "$ref" | sed 's/(ref: *//;s/:.*//;s/)//')
                if [ -n "$reffile" ] && [ ! -f "$reffile" ]; then
                    echo "| $f | $lineno | \`$(echo "$line" | cut -c1-60)...\` | FAIL (ref não encontrado: $reffile) |" >> "$REPORT"
                    FAIL=$((FAIL+1))
                else
                    PASS=$((PASS+1))
                fi
            else
                # Claim forte sem ref
                echo "| $f | $lineno | \`$(echo "$line" | cut -c1-60)...\` | AUDIT (sem ref rastreável) |" >> "$REPORT"
                AUDIT=$((AUDIT+1))
            fi
        fi
    done < "$f"
done

{
echo ""
echo "## Resumo"
echo ""
echo "| PASS | AUDIT | FAIL |"
echo "|:----:|:-----:|:----:|"
echo "| $PASS | $AUDIT | $FAIL |"
echo ""
if [ "$AUDIT" -gt 0 ]; then
    echo "Claims AUDIT: precisam de \`(ref: arquivo:linha)\` para ser promovidos a PASS."
fi
if [ "$FAIL" -gt 0 ]; then
    echo "Claims FAIL: ref aponta para arquivo inexistente."
fi
} >> "$REPORT"

echo "B7 claims audit: PASS=$PASS  AUDIT=$AUDIT  FAIL=$FAIL → $REPORT"

[ "$FAIL" -eq 0 ] || { echo "B7: FAIL — referências quebradas detectadas"; exit 1; }
[ "$STRICT" -eq 0 ] || [ "$AUDIT" -eq 0 ] || { echo "B7: STRICT mode — $AUDIT claims sem ref"; exit 1; }

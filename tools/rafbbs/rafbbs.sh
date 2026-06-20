#!/bin/sh
# RafBBS shell orchestrator — lightweight entry point for CI and direct invocation.
# Usage:
#   bash tools/rafbbs/rafbbs.sh <pipeline>
#   bash tools/rafbbs/rafbbs.sh list
#
# Pipelines implemented here (shell-native, no compiled binary required):
#   proof_chain   — run apkc_validate.sh, append CHAIN_OF_CUSTODY entry
#   lang_matrix   — inventory languages from Apkc/lang_profile.h
#
# All other pipelines are delegated to the compiled rafbbs binary if available.

set -e

PIPELINE="${1:-}"
REPO_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROOFS_DIR="$REPO_ROOT/Apkc/proofs"
DATE="$(date +%Y-%m-%d)"
TIMESTAMP="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

# ---------------------------------------------------------------------------
# pipeline: proof_chain
# ---------------------------------------------------------------------------
run_proof_chain() {
    echo "proof_chain: running scripts/apkc_validate.sh ..."
    OUTPUT="$("$REPO_ROOT/scripts/apkc_validate.sh" 2>&1)" && RC=0 || RC=$?

    # Append timestamped entry to chain-of-custody file
    CUSTODY_FILE="$PROOFS_DIR/CHAIN_OF_CUSTODY_${DATE}.md"
    mkdir -p "$PROOFS_DIR"

    {
        printf '\n## %s\n\n' "$TIMESTAMP"
        printf '**pipeline**: proof_chain  \n'
        printf '**script**: scripts/apkc_validate.sh  \n'
        if [ "$RC" -eq 0 ]; then
            printf '**result**: PASS  \n'
        else
            printf '**result**: FAIL (rc=%d)  \n' "$RC"
        fi
        printf '\n```\n%s\n```\n' "$OUTPUT"
    } >> "$CUSTODY_FILE"

    if [ "$RC" -eq 0 ]; then
        echo "proof_chain: PASS"
        echo "proof_chain: custody entry appended to $CUSTODY_FILE"
        return 0
    else
        echo "proof_chain: FAIL"
        echo "proof_chain: custody entry appended to $CUSTODY_FILE"
        return 1
    fi
}

# ---------------------------------------------------------------------------
# pipeline: lang_matrix
# ---------------------------------------------------------------------------
run_lang_matrix() {
    LANG_PROFILE="$REPO_ROOT/Apkc/lang_profile.h"
    if [ ! -f "$LANG_PROFILE" ]; then
        echo "lang_matrix: ERROR — $LANG_PROFILE not found" >&2
        return 1
    fi

    echo "lang_matrix: reading language table from Apkc/lang_profile.h ..."

    # Extract language names from lines like:  { "py",  ".py", ...
    # Match the name field (first quoted string after { ) inside _lang_table entries.
    N=0
    while IFS= read -r name; do
        echo "lang=$name status=TOKEN_VAZIO"
        N=$((N + 1))
    done < <(
        # Extract language names from lines that have both [LP_XXX] designator
        # and the { "name",  ".ext", pattern (avoids picking up arg-array entries).
        grep -E '^\s*\[LP_[A-Z]+\]\s*=\s*\{[[:space:]]*"[a-z]+"' "$LANG_PROFILE" \
            | sed -E 's/.*\{[[:space:]]*"([a-z]+)".*/\1/'
    )

    echo "lang_matrix: $N languages inventoried"
    return 0
}

# ---------------------------------------------------------------------------
# list command
# ---------------------------------------------------------------------------
run_list() {
    printf '%-16s %s\n' "proof_chain"  "Run apkc_validate.sh and append chain-of-custody entry"
    printf '%-16s %s\n' "lang_matrix"  "Inventory languages from Apkc/lang_profile.h"
    # Delegate to compiled binary for remaining pipelines if available
    BINARY="$REPO_ROOT/tools/rafbbs/rafbbs"
    if [ -x "$BINARY" ]; then
        "$BINARY" list 2>/dev/null | grep -v -E '^(proof_chain|lang_matrix)\s' || true
    fi
}

# ---------------------------------------------------------------------------
# dispatch
# ---------------------------------------------------------------------------
case "$PIPELINE" in
    proof_chain)
        run_proof_chain
        ;;
    lang_matrix)
        run_lang_matrix
        ;;
    list)
        run_list
        ;;
    ""|--help|-h)
        printf 'RafBBS shell orchestrator\n'
        printf 'usage: bash tools/rafbbs/rafbbs.sh <pipeline|list>\n'
        printf '\npipelines:\n'
        run_list
        ;;
    *)
        # Delegate to compiled binary for other known pipelines
        BINARY="$REPO_ROOT/tools/rafbbs/rafbbs"
        if [ -x "$BINARY" ]; then
            exec "$BINARY" run "$PIPELINE"
        else
            printf 'rafbbs.sh: unknown pipeline "%s" and compiled binary not found\n' "$PIPELINE" >&2
            printf 'Build with: sh tools/rafbbs/rafbbs_build.sh host\n' >&2
            exit 2
        fi
        ;;
esac

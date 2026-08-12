#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
RUN_ROOT="$ROOT/build/apkc-hardening-runs"
STAMP=$(date -u '+%Y%m%dT%H%M%SZ')
RUN_DIR="$RUN_ROOT/run-$STAMP-$$"
RECEIPT="$RUN_DIR/receipt.env"
SUCCESS=0

cleanup() {
    rc=$?
    if [ "$SUCCESS" -ne 1 ]; then
        rm -rf "$RUN_DIR"
    fi
    exit "$rc"
}
trap cleanup EXIT HUP INT TERM

command -v sha256sum >/dev/null 2>&1 || {
    printf '%s\n' 'apkc-provenance: ERROR: sha256sum is mandatory for auditable runs' >&2
    exit 1
}
[ -x "$ROOT/scripts/apkc_termux_hermetic_build.sh" ] || [ -f "$ROOT/scripts/apkc_termux_hermetic_build.sh" ] || {
    printf '%s\n' 'apkc-provenance: ERROR: hermetic builder missing' >&2
    exit 1
}

mkdir -p "$RUN_DIR"
sh "$ROOT/scripts/apkc_termux_hermetic_build.sh" --out "$RUN_DIR" "$@"
[ -s "$RECEIPT" ] || {
    printf '%s\n' 'apkc-provenance: ERROR: canonical receipt missing' >&2
    exit 1
}

if grep -q 'TOKEN_VAZIO' "$RECEIPT"; then
    case "$(grep '^claim_allowed=' "$RECEIPT" || true)" in
        claim_allowed=false) : ;;
        *) printf '%s\n' 'apkc-provenance: ERROR: TOKEN_VAZIO present without claim_allowed=false' >&2; exit 1 ;;
    esac
fi

for key in raw_apkc_sha256 hardened_apkc_sha256 source_sha256 apkc_host_sha256 unsigned_apk_sha256; do
    value=$(sed -n "s/^${key}=//p" "$RECEIPT")
    case "$value" in
        ''|TOKEN_VAZIO*) printf 'apkc-provenance: ERROR: missing verified hash: %s\n' "$key" >&2; exit 1 ;;
        *[!0-9a-fA-F]*) printf 'apkc-provenance: ERROR: malformed SHA-256: %s\n' "$key" >&2; exit 1 ;;
    esac
    [ "${#value}" -eq 64 ] || { printf 'apkc-provenance: ERROR: SHA-256 length mismatch: %s\n' "$key" >&2; exit 1; }
done

UNSIGNED=$(sed -n 's/^unsigned_apk=//p' "$RECEIPT")
EXPECTED=$(sed -n 's/^unsigned_apk_sha256=//p' "$RECEIPT")
[ -s "$UNSIGNED" ] || { printf '%s\n' 'apkc-provenance: ERROR: unsigned APK missing after build' >&2; exit 1; }
ACTUAL=$(sha256sum "$UNSIGNED" | awk '{print $1}')
[ "$ACTUAL" = "$EXPECTED" ] || { printf '%s\n' 'apkc-provenance: ERROR: unsigned APK hash mismatch' >&2; exit 1; }

{
    printf '%s\n' 'provenance_gate=PASS'
    printf 'provenance_gate_sha256=%s\n' "$ACTUAL"
    printf '%s\n' 'fail_closed_temp_cleanup=ENFORCED'
    printf '%s\n' 'claim_allowed=false'
} >> "$RECEIPT"

SUCCESS=1
trap - EXIT HUP INT TERM
printf 'apkc-provenance: PASS run_dir=%s\n' "$RUN_DIR"
printf 'apkc-provenance: receipt=%s\n' "$RECEIPT"

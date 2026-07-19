#!/usr/bin/env sh
set -eu

usage() {
  cat <<'EOF'
Usage:
  sh scripts/raf_https_fetch.sh --tls 1.2|1.3 --url https://host/path --output FILE [--report FILE]

Properties:
  - HTTPS only; redirects remain HTTPS.
  - Certificate and hostname validation stay enabled.
  - TLS version is pinned exactly to 1.2 or 1.3.
  - Body is promoted atomically only after curl succeeds.
  - Report stores host/path hash and response metadata, not credentials.

This is an HTTPS transport adapter. It is not an HTML renderer and not an ASM browser.
EOF
}

TLS_VERSION=
URL=
OUTPUT=
REPORT=

while [ "$#" -gt 0 ]; do
  case "$1" in
    --tls)
      [ "$#" -ge 2 ] || { usage >&2; exit 64; }
      TLS_VERSION=$2
      shift 2
      ;;
    --url)
      [ "$#" -ge 2 ] || { usage >&2; exit 64; }
      URL=$2
      shift 2
      ;;
    --output)
      [ "$#" -ge 2 ] || { usage >&2; exit 64; }
      OUTPUT=$2
      shift 2
      ;;
    --report)
      [ "$#" -ge 2 ] || { usage >&2; exit 64; }
      REPORT=$2
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "raf_https_fetch: argumento desconhecido: $1" >&2
      usage >&2
      exit 64
      ;;
  esac
done

[ -n "$TLS_VERSION" ] && [ -n "$URL" ] && [ -n "$OUTPUT" ] || {
  usage >&2
  exit 64
}

case "$TLS_VERSION" in
  1.2) TLS_FLAG=--tlsv1.2 ;;
  1.3) TLS_FLAG=--tlsv1.3 ;;
  *) echo "raf_https_fetch: --tls deve ser 1.2 ou 1.3" >&2; exit 64 ;;
esac

case "$URL" in
  https://*) ;;
  *) echo "raf_https_fetch: somente URLs https:// são permitidas" >&2; exit 65 ;;
esac

# Reject credentials in the authority component and control characters.
case "$URL" in
  https://*@*) echo "raf_https_fetch: credenciais embutidas na URL são proibidas" >&2; exit 65 ;;
esac
if printf '%s' "$URL" | LC_ALL=C grep -q '[[:cntrl:]]'; then
  echo "raf_https_fetch: URL contém caractere de controle" >&2
  exit 65
fi

command -v curl >/dev/null 2>&1 || {
  echo "raf_https_fetch: curl ausente" >&2
  exit 127
}
command -v python3 >/dev/null 2>&1 || {
  echo "raf_https_fetch: python3 ausente" >&2
  exit 127
}

# --tls-max is required to prove the requested exact protocol boundary.
if ! curl --help all 2>/dev/null | grep -q -- '--tls-max'; then
  echo "raf_https_fetch: curl sem suporte a --tls-max" >&2
  exit 69
fi

OUT_DIR=$(dirname -- "$OUTPUT")
mkdir -p "$OUT_DIR"
if [ -z "$REPORT" ]; then
  REPORT="${OUTPUT}.evidence.json"
fi
REPORT_DIR=$(dirname -- "$REPORT")
mkdir -p "$REPORT_DIR"

TMP_BASE=${TMPDIR:-.}
[ -d "$TMP_BASE" ] && [ -w "$TMP_BASE" ] || TMP_BASE=.
WORK=$(mktemp -d "$TMP_BASE/raf-https-fetch.XXXXXX")
cleanup() {
  rm -rf -- "$WORK"
}
trap cleanup EXIT HUP INT TERM

BODY="$WORK/body.bin"
HEADERS="$WORK/headers.txt"
CURL_META="$WORK/curl-meta.txt"

# No -k/--insecure, no custom Host header, no protocol downgrade.
# --proto-redir keeps redirects inside HTTPS.
set +e
curl \
  --silent \
  --show-error \
  --fail \
  --location \
  --max-redirs 5 \
  --connect-timeout 10 \
  --max-time 60 \
  --proto '=https' \
  --proto-redir '=https' \
  "$TLS_FLAG" \
  --tls-max "$TLS_VERSION" \
  --dump-header "$HEADERS" \
  --output "$BODY" \
  --write-out '%{http_code}\n%{url_effective}\n%{remote_ip}\n%{ssl_verify_result}\n%{size_download}\n%{time_total}\n' \
  "$URL" >"$CURL_META"
RC=$?
set -e

if [ "$RC" -ne 0 ]; then
  echo "raf_https_fetch: curl falhou com exit $RC" >&2
  exit "$RC"
fi

python3 - "$REPORT" "$BODY" "$HEADERS" "$CURL_META" "$URL" "$TLS_VERSION" <<'PY'
from __future__ import annotations

import hashlib
import json
import pathlib
import sys
import urllib.parse

report_path = pathlib.Path(sys.argv[1])
body_path = pathlib.Path(sys.argv[2])
headers_path = pathlib.Path(sys.argv[3])
meta_path = pathlib.Path(sys.argv[4])
url = sys.argv[5]
tls_version = sys.argv[6]

meta = meta_path.read_text(encoding="utf-8", errors="replace").splitlines()
while len(meta) < 6:
    meta.append("")
http_code, effective_url, remote_ip, ssl_verify_result, size_download, time_total = meta[:6]
parsed = urllib.parse.urlsplit(effective_url or url)
redacted_target = f"{parsed.scheme}://{parsed.hostname or ''}{parsed.path or '/'}"
if parsed.query:
    redacted_target += "?SHA256=" + hashlib.sha256(parsed.query.encode()).hexdigest()

body = body_path.read_bytes()
headers = headers_path.read_bytes()
report = {
    "schema": "raf.https-fetch-evidence.v1",
    "state": "PASS",
    "capability": "HTTPS_TRANSPORT_ADAPTER",
    "not_capabilities": ["WEB_BROWSER_TLS", "ASM_WEB_BROWSER_TLS", "HTML_RENDERER"],
    "requested_tls_version": tls_version,
    "http_code": int(http_code) if http_code.isdigit() else None,
    "effective_target_redacted": redacted_target,
    "remote_ip": remote_ip,
    "ssl_verify_result": int(ssl_verify_result) if ssl_verify_result.isdigit() else None,
    "certificate_and_hostname_validation_enabled": True,
    "body_size_bytes": len(body),
    "body_sha256": hashlib.sha256(body).hexdigest(),
    "headers_sha256": hashlib.sha256(headers).hexdigest(),
    "curl_reported_size_download": size_download,
    "time_total_seconds": time_total,
    "claim_allowed": False,
    "claim_limit": "Prova uma transferência HTTPS específica; não certifica navegador, biblioteca TLS ou implementação ASM.",
}
report_path.write_text(json.dumps(report, ensure_ascii=False, sort_keys=True, indent=2) + "\n", encoding="utf-8")
PY

# Promote only after successful transfer and report creation.
TMP_OUTPUT="${OUTPUT}.tmp.$$"
cp "$BODY" "$TMP_OUTPUT"
mv -f "$TMP_OUTPUT" "$OUTPUT"

echo "raf_https_fetch: PASS tls=$TLS_VERSION output=$OUTPUT report=$REPORT"

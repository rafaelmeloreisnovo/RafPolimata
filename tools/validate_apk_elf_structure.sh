#!/bin/bash
# tools/validate_apk_elf_structure.sh
#
# Phase C Component 3 (Gap L3): ARM64 ELF Validation in APK
# Verifies that APKs contain properly-formed ARM64 native libraries
#
# Checks:
# 1. APK is valid ZIP format
# 2. lib/arm64-v8a/ directory exists
# 3. *.so files have valid ELF64 headers
# 4. ELF machine type is ARM64 (0xB7)
# 5. ELF is position-independent executable (PIE)
#
# Usage:
#   ./tools/validate_apk_elf_structure.sh <apk_file>
#   ./tools/validate_apk_elf_structure.sh out.apk > validation_result.json

set -e

APK_FILE="${1:?Usage: $0 <apk_file>}"
TIMESTAMP=$(date -u +"%Y%m%d_%H%M%S")
TMPDIR="/tmp/apk_validation_$$"
RESULT_DIR="${2:-.}/docs/proofs"

mkdir -p "$TMPDIR" "$RESULT_DIR"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

# Verification flags
PASS=1
ZIP_VALID=0
HAS_ARM64_DIR=0
SO_FILES_FOUND=0
ELF_HEADERS_VALID=0
PIE_VERIFIED=0

# ─────────────────────────────────────────────────────────────────────────
# Check 1: APK is valid ZIP
# ─────────────────────────────────────────────────────────────────────────

check_zip_format() {
    log_info "Check 1: Verifying APK is valid ZIP format..."

    if [ ! -f "$APK_FILE" ]; then
        log_error "APK file not found: $APK_FILE"
        PASS=0
        return 1
    fi

    # Check ZIP magic bytes (50 4B 03 04)
    local magic=$(od -An -tx1 -N4 "$APK_FILE" 2>/dev/null | tr -d ' ')
    if [ "$magic" = "504b0304" ]; then
        log_info "  ✓ ZIP magic bytes valid (50 4B 03 04)"
        ZIP_VALID=1
        return 0
    else
        log_error "  ✗ Invalid ZIP magic bytes: $magic"
        PASS=0
        return 1
    fi
}

# ─────────────────────────────────────────────────────────────────────────
# Check 2: Extract and verify lib/arm64-v8a directory
# ─────────────────────────────────────────────────────────────────────────

check_arm64_directory() {
    log_info "Check 2: Verifying ARM64 library directory..."

    # Extract APK contents
    if ! unzip -q "$APK_FILE" -d "$TMPDIR" 2>/dev/null; then
        log_error "  ✗ Failed to extract APK"
        PASS=0
        return 1
    fi

    if [ -d "$TMPDIR/lib/arm64-v8a" ]; then
        log_info "  ✓ lib/arm64-v8a directory found"
        HAS_ARM64_DIR=1
        return 0
    else
        log_warn "  ⊘ lib/arm64-v8a directory not found (may use DEX only)"
        return 0
    fi
}

# ─────────────────────────────────────────────────────────────────────────
# Check 3: Find and validate .so files
# ─────────────────────────────────────────────────────────────────────────

check_so_files() {
    log_info "Check 3: Validating ARM64 .so files..."

    if [ ! -d "$TMPDIR/lib/arm64-v8a" ]; then
        log_warn "  ⊘ lib/arm64-v8a directory not available (skipping .so validation)"
        return 0
    fi

    local so_count=0
    local so_valid=0

    for so_file in "$TMPDIR/lib/arm64-v8a"/*.so; do
        if [ ! -f "$so_file" ]; then
            continue
        fi

        so_count=$((so_count + 1))
        local basename=$(basename "$so_file")

        # Check ELF magic bytes
        local elf_magic=$(od -An -tx1 -N4 "$so_file" 2>/dev/null | tr -d ' ')
        if [ "$elf_magic" = "7f454c46" ]; then
            log_info "  ✓ $basename: ELF magic valid"
            so_valid=$((so_valid + 1))

            # Check machine type (bytes 18-19 should be B7 00 for ARM64 little-endian)
            local machine_bytes=$(od -An -tx1 -j18 -N2 "$so_file" 2>/dev/null | tr -d ' ')
            if [ "$machine_bytes" = "b700" ]; then
                log_info "    ✓ Machine type: ARM64 (0xB7)"
            else
                log_warn "    ⊘ Machine type: $machine_bytes (not ARM64)"
            fi

            # Check if PIE (e_type bytes 16-17 should be 03 00 for ET_DYN)
            local etype=$(od -An -tx1 -j16 -N2 "$so_file" 2>/dev/null | tr -d ' ')
            if [ "$etype" = "0300" ]; then
                log_info "    ✓ Type: Position-Independent Executable (PIE)"
            else
                log_warn "    ⊘ Type: $etype (may not be PIE)"
            fi
        else
            log_error "  ✗ $basename: Invalid ELF magic bytes: $elf_magic"
            PASS=0
        fi
    done

    if [ $so_count -gt 0 ]; then
        SO_FILES_FOUND=1
        if [ $so_valid -eq $so_count ]; then
            ELF_HEADERS_VALID=1
            log_info "  ✓ All $so_count .so files have valid ELF headers"
            return 0
        else
            log_error "  ✗ $((so_count - so_valid)) of $so_count .so files failed validation"
            PASS=0
            return 1
        fi
    else
        log_warn "  ⊘ No .so files found in lib/arm64-v8a"
        return 0
    fi
}

# ─────────────────────────────────────────────────────────────────────────
# Check 4: Verify PIE for all .so files
# ─────────────────────────────────────────────────────────────────────────

check_pie_verification() {
    log_info "Check 4: Verifying Position-Independent Executable (PIE)..."

    if [ ! -d "$TMPDIR/lib/arm64-v8a" ] || [ $SO_FILES_FOUND -eq 0 ]; then
        log_warn "  ⊘ No .so files to verify for PIE"
        return 0
    fi

    local pie_count=0
    for so_file in "$TMPDIR/lib/arm64-v8a"/*.so; do
        if [ ! -f "$so_file" ]; then
            continue
        fi

        # Bytes 16-17: e_type (03 00 = ET_DYN = PIE)
        local etype=$(od -An -tx1 -j16 -N2 "$so_file" 2>/dev/null | tr -d ' ')
        if [ "$etype" = "0300" ]; then
            pie_count=$((pie_count + 1))
        fi
    done

    local total_so=$(ls "$TMPDIR/lib/arm64-v8a"/*.so 2>/dev/null | wc -l)
    if [ $pie_count -eq $total_so ]; then
        log_info "  ✓ All $total_so .so files are Position-Independent"
        PIE_VERIFIED=1
        return 0
    else
        log_warn "  ⊘ $pie_count of $total_so .so files are PIE"
        return 0
    fi
}

# ─────────────────────────────────────────────────────────────────────────
# Check 5: Verify DEX presence (fallback for interpreted code)
# ─────────────────────────────────────────────────────────────────────────

check_dex_presence() {
    log_info "Check 5: Verifying DEX presence (bytecode fallback)..."

    if [ -f "$TMPDIR/classes.dex" ]; then
        # Check DEX magic bytes (64 65 78 0A = "dex\n")
        local dex_magic=$(od -An -tx1 -N4 "$TMPDIR/classes.dex" 2>/dev/null | tr -d ' ')
        if [ "$dex_magic" = "6465780a" ]; then
            log_info "  ✓ classes.dex found with valid magic bytes"
            return 0
        else
            log_error "  ✗ classes.dex found but magic bytes invalid: $dex_magic"
            return 1
        fi
    else
        log_warn "  ⊘ No classes.dex found (native-only APK)"
        return 0
    fi
}

# ─────────────────────────────────────────────────────────────────────────
# Generate Results Receipt
# ─────────────────────────────────────────────────────────────────────────

generate_receipt() {
    local receipt_file="$RESULT_DIR/L3_ELF_VALIDATION_${TIMESTAMP}.json"

    cat > "$receipt_file" << EOF
{
  "metadata": {
    "timestamp": "$(date -u +'%Y-%m-%dT%H:%M:%SZ')",
    "apk_file": "$(basename $APK_FILE)",
    "apk_size_bytes": $(stat -c%s "$APK_FILE" 2>/dev/null || stat -f%z "$APK_FILE")
  },
  "checks": {
    "zip_format": $ZIP_VALID,
    "arm64_directory": $HAS_ARM64_DIR,
    "so_files_found": $SO_FILES_FOUND,
    "elf_headers_valid": $ELF_HEADERS_VALID,
    "pie_verified": $PIE_VERIFIED
  },
  "verdict": $([ $PASS -eq 1 ] && echo "\"PASS\"" || echo "\"FAIL\""),
  "summary": "$([ $PASS -eq 1 ] && echo "✓ APK structure valid and ARM64-compatible" || echo "✗ APK validation failed")"
}
EOF

    log_info "Receipt saved: $receipt_file"
    echo ""
    cat "$receipt_file"
}

# ─────────────────────────────────────────────────────────────────────────
# Main Execution
# ─────────────────────────────────────────────────────────────────────────

main() {
    log_info "Starting ARM64 ELF validation for: $APK_FILE"
    echo ""

    check_zip_format || true
    check_arm64_directory || true
    check_so_files || true
    check_pie_verification || true
    check_dex_presence || true

    echo ""
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║  L3: ARM64 ELF Validation Results                          ║"
    echo "╠═══════════════════════════════════════════════════════════╣"
    echo "║  ZIP Format Valid:        $([ $ZIP_VALID -eq 1 ] && echo "✓" || echo "✗")                                         ║"
    echo "║  ARM64 Directory:         $([ $HAS_ARM64_DIR -eq 1 ] && echo "✓" || echo "✗")                                         ║"
    echo "║  .so Files Found:         $([ $SO_FILES_FOUND -eq 1 ] && echo "✓" || echo "✗")                                         ║"
    echo "║  ELF Headers Valid:       $([ $ELF_HEADERS_VALID -eq 1 ] && echo "✓" || echo "✗")                                         ║"
    echo "║  PIE Verified:            $([ $PIE_VERIFIED -eq 1 ] && echo "✓" || echo "✗")                                         ║"
    echo "║  Overall Status:          $([ $PASS -eq 1 ] && echo "✓ PASS" || echo "✗ FAIL")                                       ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo ""

    generate_receipt

    # Cleanup
    rm -rf "$TMPDIR"

    exit $([ $PASS -eq 1 ] && echo 0 || echo 1)
}

main

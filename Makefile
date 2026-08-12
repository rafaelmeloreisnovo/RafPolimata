# RafPolimata — canonical release/compiler entrypoints
SHELL    := /bin/bash
VERBOVIVO := verbovivo_ci
APKC_HARDENED_SRC := build/generated/Apkc/apkc.source-cap-hardened.c
SYNTAX_CC := clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc -ffreestanding -I Apkc $(APKC_HARDENED_SRC)
AUDIT_OUT ?= ci/reports/library-assimilation.json
RAF_LANG ?= c
RAF_ARCH ?= arm64
SRC ?= tests/fixtures/strict_kernel.c
OUT ?= build/strict/libmain.so

.PHONY: help syntax apkc-hardened-source verbovivo verbovivo-demo encoders proof audit language-contract compile compile-plan compiler-contract compiler-selftest hotfix-audit library-audit strict-elf report clean

help:
	@echo 'RafPolimata — make targets:'
	@echo '  help              this list (default)'
	@echo '  apkc-hardened-source  generate + falsify source-cap hardened ApkC TU'
	@echo '  syntax            freestanding aarch64 syntax check (hardened ApkC TU)'
	@echo '  verbovivo         build $(VERBOVIVO) (T^7 toroid pipeline)'
	@echo '  verbovivo-demo    build + smoke run, asserts verbovivo: ... phi='
	@echo '  encoders          ARM32 + ARM64 encoder golden tests'
	@echo '  proof             one clean reproducible proof run (tools/raf_clean_proof_run.sh)'
	@echo '  audit             freestanding invariant audit (scripts/ci_freestanding_audit.sh)'
	@echo '  language-contract M063: policies, compiler station and strict ELF gates'
	@echo '  compile           execute strict compiler: RAF_LANG/RAF_ARCH/SRC/OUT'
	@echo '  compile-plan      emit deterministic JSON plan without executing'
	@echo '  compiler-contract validate the complete machine-readable station inventory'
	@echo '  compiler-selftest run transactional, custody and adversarial gates'
	@echo '  hotfix-audit      contract + station + M063 blocking audit'
	@echo '  library-audit     use LIB/RAF_LANG; writes AUDIT_OUT=$(AUDIT_OUT)'
	@echo '  strict-elf        audit ELF=$(ELF) [ELF_PROFILE=exec|android-so]'
	@echo '  report            show curated proof and latest run state'
	@echo '  clean             remove generated compiler/demo artifacts'

apkc-hardened-source:
	@command -v python3 >/dev/null 2>&1 || { echo 'apkc-hardened-source: FAIL — python3 required' >&2; exit 127; }
	@test -f scripts/patch_apkc_source_cap.py || { echo 'apkc-hardened-source: FAIL — transformer missing' >&2; exit 1; }
	@test -f tests/test_apkc_source_cap_patch.py || { echo 'apkc-hardened-source: FAIL — falsifier missing' >&2; exit 1; }
	@mkdir -p "$(dir $(APKC_HARDENED_SRC))"
	python3 tests/test_apkc_source_cap_patch.py
	python3 scripts/patch_apkc_source_cap.py Apkc/apkc.c "$(APKC_HARDENED_SRC)"
	@grep -q 'source exceeds SRC_CAP' "$(APKC_HARDENED_SRC)" || { echo 'apkc-hardened-source: FAIL — overflow guard absent' >&2; exit 1; }
	@! grep -Fq 'if (n<=0) break;' "$(APKC_HARDENED_SRC)" || { echo 'apkc-hardened-source: FAIL — vulnerable anchor survived' >&2; exit 1; }
	@printf 'apkc-hardened-source: PASS sha256='; sha256sum "$(APKC_HARDENED_SRC)" | cut -d' ' -f1

syntax: apkc-hardened-source
	$(SYNTAX_CC)
	@echo 'syntax: PASS hardened-source'

verbovivo:
	gcc -std=c11 -O2 -I. -IBenchmark -DVERBOVIVO_MAIN rafaelia/verbovivo.c rafaelia/fiber_relmat.c -lm -o $(VERBOVIVO)

verbovivo-demo: verbovivo
	@echo 'RAFAELIA demo' | ./$(VERBOVIVO) /dev/stdin /tmp/engram.svg 2>&1 | grep -E 'verbovivo:.*phi='
	@echo 'verbovivo-demo: PASS'

encoders:
	python3 tests/test_arm32_encoders.py
	python3 tests/test_arm64_encoders.py

proof:
	bash tools/raf_clean_proof_run.sh

audit:
	@if [ -f scripts/ci_freestanding_audit.sh ]; then \
		bash scripts/ci_freestanding_audit.sh; \
	else \
		echo 'audit: TOKEN_VAZIO — scripts/ci_freestanding_audit.sh ausente'; \
	fi

language-contract:
	bash scripts/audit_language_freestanding_contract.sh

compile:
	@test -n "$(RAF_LANG)" && test -n "$(RAF_ARCH)" && test -n "$(SRC)" && test -n "$(OUT)" || { echo 'RAF_LANG, RAF_ARCH, SRC e OUT são obrigatórios' >&2; exit 64; }
	@mkdir -p "$(dir $(OUT))"
	bash scripts/apkc_strict_native_build.sh "$(RAF_LANG)" "$(RAF_ARCH)" "$(OUT)" "$(SRC)"

compile-plan:
	@test -n "$(RAF_LANG)" || { echo 'Uso: make compile-plan RAF_LANG=c RAF_ARCH=arm32 SRC=kernel.c OUT=kernel.o' >&2; exit 64; }
	@test -n "$(RAF_ARCH)" && test -n "$(SRC)" && test -n "$(OUT)" || { echo 'RAF_ARCH, RAF_LANG, SRC e OUT são obrigatórios' >&2; exit 64; }
	python3 scripts/raf_strict_compile_plan.py --language "$(RAF_LANG)" --arch "$(RAF_ARCH)" --source "$(SRC)" --output "$(OUT)"

compiler-contract:
	python3 scripts/validate_compiler_station_contract.py

compiler-selftest:
	bash scripts/test_compiler_station.sh

hotfix-audit: compiler-contract compiler-selftest language-contract
	@echo 'RAFAELIA compiler HOTFIX audit: PASS'

library-audit:
	@test -n "$(LIB)" && test -n "$(RAF_LANG)" || { echo 'Uso: make library-audit LIB=vendor/lib RAF_LANG=c [AUDIT_OUT=...]' >&2; exit 64; }
	python3 scripts/raf_library_assimilation_audit.py "$(LIB)" --language "$(RAF_LANG)" --output "$(AUDIT_OUT)"
	@echo "library-audit: $(AUDIT_OUT)"

strict-elf:
	@test -n "$(ELF)" || { echo 'Uso: make strict-elf ELF=out/programa.elf [ELF_PROFILE=exec|android-so]' >&2; exit 64; }
	bash scripts/audit_strict_elf.sh --profile "$(or $(ELF_PROFILE),exec)" "$(ELF)"

report:
	@echo '== RafPolimata proof report =='
	@echo 'Curated proofs:  Apkc/proofs/out/'
	@ls -1 Apkc/proofs/out 2>/dev/null | sed 's/^/  out\//' || echo '  (vazio)'
	@latest=$$(ls -1dt Apkc/proofs/runs/*/ 2>/dev/null | head -1); \
	if [ -n "$$latest" ] && [ -f "$$latest/summary.txt" ]; then \
		echo "Latest run:      $$latest"; \
		echo '--- summary.txt ---'; \
		cat "$$latest/summary.txt"; \
		echo '--- gates (PASS / TOKEN_VAZIO) ---'; \
		grep -E 'PASS|TOKEN_VAZIO|FAIL' "$$latest/gates.txt" 2>/dev/null || echo '  (no gates.txt)'; \
	else \
		echo 'Latest run:      (none — run "make proof" first)'; \
	fi

clean:
	rm -f $(VERBOVIVO) /tmp/engram.svg *.o raf_compile apkc_host
	rm -rf build/strict build_host_check/ops_manifest build/generated/Apkc
	find . -maxdepth 4 -type f \( -name '*.tmp.*' -o -name '*.so.receipt.json' \) -delete
	@echo 'clean: done'

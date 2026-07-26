# RafPolimata — navegable release entrypoint (L19) + clean proof run (L6)
# All targets REUSE existing scripts/commands. No logic is duplicated here.
# Recipes use real tabs. See CLAUDE.md for the canonical commands.

SHELL    := /bin/sh
VERBOVIVO := verbovivo_ci
SYNTAX_CC := clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc -ffreestanding -I Apkc Apkc/apkc.c
AUDIT_OUT ?= ci/reports/library-assimilation.json

.PHONY: help syntax verbovivo verbovivo-demo encoders proof audit language-contract compile-plan library-audit strict-elf report clean

help:
	@echo 'RafPolimata — make targets:'
	@echo '  help              this list (default)'
	@echo '  syntax            freestanding aarch64 syntax check (Apkc/apkc.c)'
	@echo '  verbovivo         build $(VERBOVIVO) (T^7 toroid pipeline)'
	@echo '  verbovivo-demo    build + smoke run, asserts verbovivo: ... phi='
	@echo '  encoders          ARM32 + ARM64 encoder golden tests'
	@echo '  proof             one clean reproducible proof run (tools/raf_clean_proof_run.sh)'
	@echo '  audit             freestanding invariant audit (scripts/ci_freestanding_audit.sh)'
	@echo '  language-contract M063: 23-language lowering/freestanding/bit-patch gate'
	@echo '  compile-plan      use RAF_LANG/RAF_ARCH/SRC/OUT to emit a strict JSON plan'
	@echo '  library-audit     use LIB/RAF_LANG; writes AUDIT_OUT=$(AUDIT_OUT)'
	@echo '  strict-elf        audit ELF=$(ELF) for zero interpreter/dynamic/runtime residue'
	@echo '  report            point at Apkc/proofs/out + latest run dir (PASS/TOKEN_VAZIO)'
	@echo '  clean             remove $(VERBOVIVO), /tmp/engram.svg, *.o'

syntax:
	$(SYNTAX_CC)
	@echo 'syntax: PASS'

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

compile-plan:
	@test -n "$(RAF_LANG)" || { echo 'Uso: make compile-plan RAF_LANG=c RAF_ARCH=arm32 SRC=kernel.c OUT=kernel.o' >&2; exit 64; }
	@test -n "$(RAF_ARCH)" && test -n "$(SRC)" && test -n "$(OUT)" || { echo 'RAF_ARCH, SRC e OUT são obrigatórios' >&2; exit 64; }
	python3 scripts/raf_strict_compile_plan.py --language "$(RAF_LANG)" --arch "$(RAF_ARCH)" --source "$(SRC)" --output "$(OUT)"

library-audit:
	@test -n "$(LIB)" && test -n "$(RAF_LANG)" || { echo 'Uso: make library-audit LIB=vendor/lib RAF_LANG=c [AUDIT_OUT=...]' >&2; exit 64; }
	python3 scripts/raf_library_assimilation_audit.py "$(LIB)" --language "$(RAF_LANG)" --output "$(AUDIT_OUT)"
	@echo "library-audit: $(AUDIT_OUT)"

strict-elf:
	@test -n "$(ELF)" || { echo 'Uso: make strict-elf ELF=out/programa.elf' >&2; exit 64; }
	bash scripts/audit_strict_elf.sh "$(ELF)"

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
	rm -f $(VERBOVIVO) /tmp/engram.svg *.o
	@echo 'clean: done'

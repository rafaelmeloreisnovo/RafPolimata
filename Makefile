# RafPolimata — navegable release entrypoint (L19) + clean proof run (L6)
# All targets REUSE existing scripts/commands. No logic is duplicated here.
# Recipes use real tabs. See CLAUDE.md for the canonical commands.

SHELL    := /bin/sh
VERBOVIVO := verbovivo_ci
SYNTAX_CC := clang -target aarch64-linux-gnu -fsyntax-only -nostdlib -nostdinc -ffreestanding -I Apkc Apkc/apkc.c

.PHONY: help syntax verbovivo verbovivo-demo encoders proof audit report clean

help:
	@echo 'RafPolimata — make targets:'
	@echo '  help            this list (default)'
	@echo '  syntax          freestanding aarch64 syntax check (Apkc/apkc.c)'
	@echo '  verbovivo       build $(VERBOVIVO) (T^7 toroid pipeline)'
	@echo '  verbovivo-demo  build + smoke run, asserts verbovivo: ... phi='
	@echo '  encoders        ARM32 + ARM64 encoder golden tests'
	@echo '  proof           one clean reproducible proof run (tools/raf_clean_proof_run.sh)'
	@echo '  audit           freestanding invariant audit (scripts/ci_freestanding_audit.sh)'
	@echo '  report          point at Apkc/proofs/out + latest run dir (PASS/TOKEN_VAZIO)'
	@echo '  clean           remove $(VERBOVIVO), /tmp/engram.svg, *.o'

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

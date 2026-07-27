# Coupling-ID: APKC-RMR-RESEARCH-CORE-V1-20260726
# Contract-Role: BUILD_ENTRYPOINT
# License-Role: RESEARCH_NONCOMMERCIAL_ONLY
# Normative comment: verification must succeed before compilation.

PYTHON ?= python3
CC ?= cc
BUILD_DIR ?= build/coupled

.PHONY: verify generate coupled-build test

verify:
	$(PYTHON) tools/coupled_build.py --verify-only

generate:
	$(PYTHON) tools/coupled_build.py --generate

coupled-build:
	$(PYTHON) tools/coupled_build.py --compile --cc "$(CC)" --build-dir "$(BUILD_DIR)"

test:
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py'

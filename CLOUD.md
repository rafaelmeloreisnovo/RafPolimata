# CLOUD.md — cloud coding-agent adapter for RafPolimata

This adapter exists for cloud-hosted coding agents that do not have a stronger repository-specific convention. Claude Code should also read `CLAUDE.md`; GPT/ChatGPT should also read `GPT.md`.

Canonical authority remains:

1. `AGENTS.md`;
2. `docs/AGENTES.md`;
3. nearest scoped `AGENTS.md`;
4. source/tests/current receipts.

For low-level work:

- `freestanding/**` is OS-agnostic L0 and must preserve its zero-runtime invariants;
- `syscall/**` is an optional OS ABI layer and must never leak into L0;
- stubs are structural/fail-closed only;
- comments are part of the engineering contract;
- compile evidence is not runtime/device evidence;
- unavailable runtime/device evidence remains `TOKEN_VAZIO (CLOSURE_L12)`.

A cloud agent must not merge without explicit human authorization and must not weaken a gate to manufacture a green result.

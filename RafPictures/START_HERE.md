# RafPictures V1 — START HERE / current state

- **Scope:** lossless `.rafp` ZIP image bundle with MIME-first entry, SHA-256, typed layers, multi-axis metadata, format-signature routing and original-byte export.
- **Producer:** this isolated `RafPictures/` subtree of `rafaelmeloreisnovo/RafPolimata` (candidate for standalone repo).
- **GitHub PR:** https://github.com/rafaelmeloreisnovo/RafPolimata/pull/450 (DRAFT, no merge).
- **Source gate:** GitHub Actions https://github.com/rafaelmeloreisnovo/RafPolimata/actions/runs/41703439540 — 10/10 Python contract tests PASS + hosted C11 detector compile/smoke PASS, against source head `40dfa2145061b739b3e457dcd2bfe6d58f0bebe3`.
- **Drive receipt:** https://docs.google.com/document/d/1Rm1zPB-mbvHQtIQmzK3VcQHlKIqwBPtlAwG6yMLCyV0/edit, verified under NOVOexport/001_PONTEIRO.
- **Trust boundaries:** SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM; C freestanding source ≠ physical ARM/Android runtime; ZIP storage compatibility ≠ Drive API sync.
- **Gaps:** standalone repository not provisioned (GitHub connector lacks endpoint); Drive upload/download and preview rendering not physically tested; Android/Termux not physically tested; no performance benchmark; standalone licensing pending (`TOKEN_VAZIO`).
- **Promotion:** `claim_allowed=false` for independent repo, Drive integration, Android performance and 3D reconstruction claims.
- **Rollback:** close PR and delete isolated branch; `main`, original photos and Drive corpus stay unchanged.

## Routes

- Human entry: README.md → SPEC.md → rafpictures/ → tests/ → .github/workflows/rafpictures-v1.yml.
- CLI: `cd RafPictures && python3 -m rafpictures --help`.
- Next minimum action: provision a dedicated RafPictures GitHub repo, migrate this subtree by verified source SHA, then run Drive byte-parity and Android multi-ABI tests.

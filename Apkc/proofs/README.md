# ApkC proofs

**Estado:** `EVIDENCE`  
**Proprietário lógico:** `apkc-maintainer`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · apk-android`](../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)

Esta pasta contém provas reproduzíveis do **ApkC**. O objetivo é separar evidência técnica, plano de build, assinatura, instalação, documentação e lacunas sem afirmar sucesso que ainda não foi medido.

## Estados padronizados

| Estado | Significado |
|---|---|
| PASS | Evidência executada e validada. |
| FAIL | Evidência executada e falhou. |
| SKIP | Etapa pulada por pré-condição documentada. |
| TOKEN_VAZIO | Ferramenta, device ou entrada ausente; não é sucesso inventado. |
| NOT_RUN | Etapa ainda não executada. |

## Estrutura esperada

```text
Apkc/proofs/
├── README.md
├── hello-apk-validation.md
├── GAPS.md
├── ARTIFACTS.md
├── VALIDATION_MATRIX.md
└── out/
    ├── unzip.txt
    ├── aapt-xmltree.txt
    ├── readelf-arm64.txt
    ├── readelf-arm32.txt
    ├── dex-sha1.txt
    └── validation-summary.md
```

## Como reproduzir

```sh
bash scripts/apkc_validate.sh
bash scripts/apkc_sign_debug.sh
bash scripts/apkc_install_android.sh
```

Cada script deve registrar comando, data UTC, commit, ferramenta e saída bruta quando possível.

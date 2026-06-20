# ApkC — Política de assinatura (L18)

> **Cadeia de custódia documental — 2026-06-17**
> Fecha a lacuna L18 de `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md`
> ("Assinatura de release, não só debug").
> Toda afirmação sobre estado do repositório está ancorada num arquivo lido e
> citado por caminho. Este documento não é avaliação financeira; é política de
> chaves e definição honesta do que hoje é `PASS`, o que é `PENDING` e o que é
> `TOKEN_VAZIO`.

---

## Regra inegociável: nenhum material de chave no repositório

**Nenhum keystore nem material de chave (chave privada, senha, alias secreto)
é jamais commitado neste repositório.** A assinatura de release acontece
exclusivamente via **CI secrets** ou **HSM**.

O script de debug `scripts/apkc_sign_debug.sh` **gera** o keystore localmente
em tempo de execução (`Apkc/proofs/debug.keystore`,
`scripts/apkc_sign_debug.sh:7,23-25`) e nunca o versiona; ele existe só dentro
do run de CI/lab. A chave de release não passa nem por esse caminho local: vem
de secret/HSM no momento da assinatura e não toca o disco do repo.

---

## Tabela de política de chaves

| Tipo de chave | Uso | Onde vive | Commit no repo? | Estado atual |
|---|---|---|---|---|
| **Debug keystore** (`CN=ApkC Debug, O=Rafael, C=BR`, RSA 2048) | Laboratório / CI — prova reproduzível de que o pipeline de assinatura funciona | Gerada em runtime por `scripts/apkc_sign_debug.sh` (`debug.keystore`); some com o runner | **NUNCA** | `PASS` para v1/v2/v3 (ver evidência abaixo) |
| **Release keystore** | Distribuição / publicação a terceiros | CI secrets (GitHub Actions secrets) ou HSM; nunca em disco do repo | **NUNCA** | `PENDING` — sem wiring de secret; `TOKEN_VAZIO` até existir um APK release-signed real |
| **SourceStamp** | Proveniência / cadeia de custódia comercial | Certificado de stamp gerenciado fora do repo (CI secret / HSM) | **NUNCA** | `false` hoje (`apksigner-verify.txt:14`) → `TOKEN_VAZIO` |

---

## Estado de fato da assinatura (ancorado em prova)

Fonte: `Apkc/proofs/out/apksigner-verify.txt` (resumo de
`apksigner verify --verbose`, capturado de `APK_PROOF_INSTALL_OPEN.txt`,
data `2026-06-14T10:56:26Z`).

| Esquema de assinatura | Estado verificado | Fonte |
|---|---|---|
| v1 JAR signing | **true** | `apksigner-verify.txt:9` |
| v2 APK Signature Scheme | **true** | `apksigner-verify.txt:10` |
| v3 APK Signature Scheme | **true** | `apksigner-verify.txt:11` |
| v3.1 | **false** | `apksigner-verify.txt:12` |
| v4 | **false** | `apksigner-verify.txt:13` |
| SourceStamp | **false** | `apksigner-verify.txt:14` |
| nº de signatários | 1 | `apksigner-verify.txt:15` |
| DN do certificado | `CN=ApkC Debug, O=Rafael, C=BR` | `apksigner-verify.txt:16` |
| algoritmo / tamanho de chave | RSA / 2048 | `apksigner-verify.txt:18-19` |

Leitura honesta: o que está `PASS` (v1/v2/v3) é assinatura **debug**, não de
release. v3.1, v4 e SourceStamp estão `false` — portanto cadeia de proveniência
comercial e os esquemas mais novos ainda não existem.

---

## Fluxo de assinatura debug (existente)

`scripts/apkc_sign_debug.sh` é o caminho debug auditável. Comportamento real,
lido do script:

1. Define `OUT=Apkc/proofs/out`, `KS=Apkc/proofs/debug.keystore`, e os alvos
   `hello.apk` → `hello-signed.apk` (`apkc_sign_debug.sh:6-11`).
2. Escreve `apksigner-sign.txt` com `date_utc`, `repo_commit` (short HEAD ou
   `TOKEN_VAZIO`), `input_apk` e SHA-256 do APK (`apkc_sign_debug.sh:13-19`).
3. **Degradação honesta:** se `apksigner` ausente → grava `TOKEN_VAZIO` e
   `exit 0` (`apkc_sign_debug.sh:20`); se `keytool` ausente → `TOKEN_VAZIO` +
   `exit 0` (`apkc_sign_debug.sh:21`); se `hello.apk` ausente → `TOKEN_VAZIO`
   + `exit 0` (`apkc_sign_debug.sh:22`).
4. Gera o keystore debug (`keytool -genkeypair ... -dname 'CN=ApkC Debug,...'`)
   só se ainda não existir (`apkc_sign_debug.sh:23-25`).
5. Assina (`apksigner sign --ks ...`) e verifica
   (`apksigner verify --verbose`), gravando o resultado em
   `apksigner-verify.txt` (`apkc_sign_debug.sh:26-32`).

No CI, esse script é invocado pelo step
**"Try ApkC debug signing proof (TOKEN_VAZIO allowed)"**
(`.github/workflows/ci.yml:100-101`), e os artefatos de `Apkc/proofs/out/` são
publicados pelo step **"Upload ApkC proof artifacts"**
(`.github/workflows/ci.yml:103-109`). O nome do step já declara que
`TOKEN_VAZIO` é resultado aceitável — não falha o CI quando o toolchain de
assinatura está ausente.

---

## Release signing: PENDING

Estado: **PENDING — precisa de wiring de CI secret / HSM.**
Marca-se **TOKEN_VAZIO até existir um APK release-signed real** verificado.

Requisitos concretos para sair de PENDING (sem violar a regra inegociável):

1. Provisionar a release key em **GitHub Actions secrets** (keystore
   base64 + senhas) ou em **HSM** acessível ao runner — nunca em arquivo
   versionado.
2. Adicionar um step de release-signing separado do debug, que **não** use
   `CN=ApkC Debug` e que injete a chave a partir do secret/HSM em runtime.
3. Capturar `apksigner verify --verbose` do APK release-signed em
   `Apkc/proofs/out/` (novo artefato, p.ex. `apksigner-verify-release.txt`),
   com v1/v2/v3 (e idealmente v3.1/v4/SourceStamp) verificados e DN de release.
4. Só então este documento e `apksigner-verify.txt` deixam de marcar release
   como `TOKEN_VAZIO`.

Enquanto (1)–(3) não existirem como artefato verificável, release-signing
permanece `PENDING`/`TOKEN_VAZIO` — não se converte em `PASS` por omissão
(invariante de `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md:506`).

---

## Referências de arquivo (ancoragem da cadeia de custódia)

| Afirmação | Arquivo : linha |
|---|---|
| Script de assinatura debug | `scripts/apkc_sign_debug.sh:1-32` |
| Keystore gerado em runtime, não versionado | `scripts/apkc_sign_debug.sh:7,23-25` |
| DN debug `CN=ApkC Debug,O=Rafael,C=BR` | `scripts/apkc_sign_debug.sh:24` |
| Degradação honesta (TOKEN_VAZIO + exit 0) | `scripts/apkc_sign_debug.sh:20-22` |
| Step CI de debug signing | `.github/workflows/ci.yml:100-101` |
| Step CI de upload de proofs | `.github/workflows/ci.yml:103-109` |
| v1/v2/v3 true; v3.1/v4/SourceStamp false | `Apkc/proofs/out/apksigner-verify.txt:9-14` |
| Lacuna original deste documento | `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L18 |

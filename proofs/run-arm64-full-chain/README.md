# Prova Android Full Chain — ARM64 / RafPolimata

**Estado:** `EVIDENCE / TOKEN_VAZIO` (gates G3–G5 pendentes de run ARM64)  
**Proprietário lógico:** `apkc-maintainer`  
**Âncora no mapa:** [`docs/MAPA_ESTRUTURAL_REPOSITORIO.md §4 · apk-android`](../../docs/MAPA_ESTRUTURAL_REPOSITORIO.md)

Status: `PROOF_CAPTURE_PROTOCOL`  
Data: 2026-06-15  
Escopo: fechar os gaps de execução que impedem promover o RafPolimata de MVP técnico para demonstração Android rastreável.

---

## 1. O que este diretório resolve

Este diretório é a rota canônica para capturar, numa única sessão, a cadeia:

```text
source
→ compile apkc
→ generate APK
→ sha256
→ unzip/readelf ARM32+ARM64
→ sign/verify quando possível
→ install
→ launch
→ logcat
→ manifest.json
```

A regra é simples:

```text
arquivo existente ≠ PASS automático
log limpo + comando + commit + ambiente + hash = evidência promovível
sem evidência = TOKEN_VAZIO
```

---

## 2. Como rodar

No repositório clonado:

```sh
bash scripts/capture_android_proof_chain.sh
```

Saída padrão:

```text
proofs/run-arm64-full-chain/out/
```

Rodada ARM64 apenas:

```sh
APKC_ARCH=-64 bash scripts/capture_android_proof_chain.sh
```

Rodada ARM32 apenas:

```sh
APKC_ARCH=-32 bash scripts/capture_android_proof_chain.sh
```

Rodada sem instalar no Android:

```sh
DO_INSTALL=0 bash scripts/capture_android_proof_chain.sh
```

Rodada com keystore explícito para assinatura:

```sh
APKSIGNER_KEYSTORE=/caminho/keystore.jks DO_SIGN=1 bash scripts/capture_android_proof_chain.sh
```

---

## 3. Artefatos esperados

| Arquivo | Significado | Status possível |
|---|---|---|
| `00_env.txt` | ambiente, commit, branch, toolchain, getprop | `AUDIT` |
| `01_compile_apkc.txt` | transcript de compilação do binário `apkc` | `PASS/FAIL/TOKEN_VAZIO` |
| `01_apkc.sha256` | hash do binário `apkc` compilado | `PASS/TOKEN_VAZIO` |
| `02_generate_apk.txt` | transcript de geração do APK | `PASS/FAIL/TOKEN_VAZIO` |
| `02_apk.sha256` | hash do APK gerado | `PASS/TOKEN_VAZIO` |
| `03_unzip_list.txt` | lista de entradas do APK | `PASS/TOKEN_VAZIO` |
| `04_readelf_arm32.txt` | cabeçalho ELF ARM32 | `PASS/TOKEN_VAZIO` |
| `05_readelf_arm64.txt` | cabeçalho ELF ARM64 | `PASS/TOKEN_VAZIO` |
| `06_apksigner*.txt` | assinatura/verificação | `PASS/AUDIT/TOKEN_VAZIO` |
| `08_*install*.txt` | stdout de instalação | `PASS/FAIL/TOKEN_VAZIO` |
| `09_launch.txt` | tentativa de launch por monkey | `PASS/FAIL/TOKEN_VAZIO` |
| `10_logcat_nativeactivity.txt` | filtro logcat NativeActivity/crash/dlopen | `PASS/FAIL/TOKEN_VAZIO` |
| `status.tsv` | resumo dos gates | `AUDIT` |
| `manifest.json` | amarração commit→artefatos | `PASS` quando emitido |

---

## 4. Critérios de promoção

### G1 — `source → apkc`

Promover para `PASS` somente se:

```text
01_compile_apkc.txt contém comando completo
01_apkc.sha256 existe
00_env.txt contém commit e toolchain
```

### G2 — `apkc → APK`

Promover para `PASS` somente se:

```text
02_generate_apk.txt não contém erro bloqueante
02_apk.sha256 existe
03_unzip_list.txt lista AndroidManifest.xml e classes.dex
```

### G3 — ARM64 real

Promover para `PASS` somente se:

```text
03_unzip_list.txt lista lib/arm64-v8a/lib*.so
05_readelf_arm64.txt contém Class: ELF64
05_readelf_arm64.txt contém Machine: AArch64
```

### G4 — Runtime NativeActivity

Promover para `PASS` somente se:

```text
install retorna Success ou instalação confirmada
launch executa sem erro fatal
10_logcat_nativeactivity.txt não contém FATAL EXCEPTION
10_logcat_nativeactivity.txt não contém crash/dlopen failed fatal
```

### G5 — Cadeia única

Promover para `PASS` somente se todos os arquivos acima vierem do mesmo:

```text
commit
branch
out_dir
data UTC
manifest.json
```

---

## 5. Estados que devem permanecer honestos

| Caso | Estado correto |
|---|---|
| `readelf` ausente | `TOKEN_VAZIO`, não FAIL do código |
| `adb` ausente | `TOKEN_VAZIO`, não prova runtime |
| APK gerado só ARM32 | ARM32 `PASS`, ARM64 `TOKEN_VAZIO` |
| logcat vazio | `AUDIT` ou `TOKEN_VAZIO`, não PASS automático |
| apksigner sem keystore | `AUDIT`/`TOKEN_VAZIO` conforme log |
| CI x86_64 verde | `PASS sintático`, não Android runtime |

---

## 6. Valor operacional

Quando esta pasta tiver um run completo e consistente, o repositório passa a sustentar a frase:

```text
RafPolimata gerou um APK Android mínimo a partir do source versionado,
produziu artefatos ARM32/ARM64 verificáveis, instalou e lançou no Android,
com logcat e hashes preservados no mesmo run.
```

Antes disso, a frase correta é:

```text
RafPolimata possui código e ferramentas para gerar APKs mínimos,
com prova parcial e lacunas runtime ainda marcadas como TOKEN_VAZIO.
```

---

## 7. Próximo passo depois do run

Depois de executar o script no dispositivo ARM64 real, commitar somente os artefatos pequenos:

```text
00_env.txt
01_compile_apkc.txt
01_apkc.sha256
02_generate_apk.txt
02_apk.sha256
03_unzip_list.txt
04_readelf_arm32.txt
05_readelf_arm64.txt
06_apksigner_verify.txt
08_adb_install.txt ou 08_pm_install.txt
09_launch.txt
10_logcat_nativeactivity.txt
status.tsv
manifest.json
```

Não commitar APK/binários grandes sem necessidade. Se precisar preservar binário, usar release artifact ou anexar checksum + instrução de reprodução.

ΣΩΔΦBITRAF · cadeia de custódia antes de claim.

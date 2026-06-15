# ApkC — cadeia de custódia do APK assinado

Data da prova original: `2026-06-14T10:56:26Z`.
Data desta consolidação documental: `2026-06-15`.
Escopo: artefatos enviados pelo autor para registrar geração, parsing, assinatura e presença instalada do pacote `com.rafael.teste`.

## Regra de interpretação

Este arquivo registra somente evidências com origem material. Não converte lacunas em sucesso. Quando falta comando, ambiente, commit, stdout bruto ou logcat, o estado permanece `TOKEN_VAZIO`.

## Artefatos recebidos e política de versionamento

| Artefato | SHA-256 | Versionar conteúdo bruto? | Motivo |
|---|---|---:|---|
| `hello.apk` | `a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024` | Não | binário gerado; hash basta para cadeia |
| `hello-signed.apk` | `063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28` | Não | binário assinado; hash basta para cadeia |
| `hello-signed.apk.idsig` | `decf3567516958ddfa0db03db1e27d85d9e7347ce7eb05b1141edf6cd7293e7a` | Não | artefato binário v4/idsig local |
| `libhello.so` | `a8f07dae58baf40d8f0f8571e5c344301d75ccfffa6a40d14861d740ba85b2d9` | Não | binário ELF; saída `readelf` é versionada |
| `apkc` | `3d2ac064c26b8ca5be02a74238f877c3e0f9fb1d16dd4bc7da7f0e758042723c` | Não | executável local; falta transcript de compilação |
| `debug.keystore` | `540a192d1be0a62c824cfe07e2b17e486e3d2b4906f4a2b77b22adc6498d9439` | **Não** | chave local; nunca deve ser commitada |
| `generate.log` | `6f957db799abe812ac9e4eec54f54a5cddfcf58941e0aceae15221584e92bf3a` | Sim, como texto | transcript de geração |
| `APK_PROOF_INSTALL_OPEN.txt` | `9f3e239fa40a2ed6ba40f33c3648288e9d79502597a1ca40bdd00115cca46669` | Sim, como texto derivado | assinatura e package visibility |
| `aapt.txt` | `f599c99008c34d12456ffeb57719a0e57f675ef5ee2b3592029b708a9b6cc9a8` | Sim | manifest AXML parseado |
| `readelf_apk.txt` | `f7fbebbd43825011a92f0ccfbd051d31b63f23f9930ae9ed9aa3f70900e0277f` | Sim, como controle negativo | prova de comando inválido sobre APK container |

## Matriz de estado aferida

| Gate | Estado | Evidência versionada | Limite honesto |
|---|---|---|---|
| F0 source exists | PASS | `Apkc/hello.s.txt` existe no repositório | Não prova que o binário local veio exatamente deste commit |
| F1 compile apkc | TOKEN_VAZIO | `Apkc/proofs/out/apkc-compile.txt` | Há binário `apkc` enviado, mas falta transcript source→binary |
| F2 generate hello.apk | PASS | `Apkc/proofs/out/apkc-generate.txt` + SHA-256 | Transcript contém 39 avisos `unknown ARM32 mnemonic` |
| F3 unzip parses | PASS | `Apkc/proofs/out/unzip.txt` | Valida container ZIP/APK, não runtime |
| F4 AXML parses | PASS | `Apkc/proofs/out/aapt-xmltree.txt` | Manifest parseado; sem validação semântica ampla |
| F5 DEX SHA-1 matches | PASS | `Apkc/proofs/out/dex-sha1.txt` | DEX mínimo de 140 bytes; não prova lógica Java útil |
| F6 ELF readelf arm32 | PASS | `Apkc/proofs/out/readelf-arm32.txt` | Somente `armeabi-v7a`; não há arm64 no APK enviado |
| F6 ELF readelf arm64 | SKIP | `Apkc/proofs/out/readelf-arm64.txt` | APK não contém `lib/arm64-v8a/*.so` |
| F6 negative control | FAIL esperado | `Apkc/proofs/out/readelf-apk-invalid.txt` | `readelf` no APK inteiro é alvo incorreto |
| F7 APK signing | PASS | `Apkc/proofs/out/apksigner-verify.txt` | Debug/self-signed; sem SourceStamp; v4 false |
| F8 installed package visible | PASS | `Apkc/proofs/out/adb-install.txt` | stdout exato de `adb install -r` não veio no artefato |
| F9 NativeActivity runtime | TOKEN_VAZIO | `Apkc/proofs/out/logcat-nativeactivity.txt` | Falta `logcat`/lançamento sem crash |

## Leitura técnica sem salto indevido

O que a cadeia permite dizer: o material enviado contém um APK mínimo com `AndroidManifest.xml`, `classes.dex` e `lib/armeabi-v7a/libhello.so`; o APK assinado tem assinatura v1/v2/v3 positiva no transcript de `apksigner`; o manifesto declara `android.app.NativeActivity`, `android:hasCode=false`, pacote `com.rafael.teste`, label `RafaelTeste` e biblioteca nativa `hello`; o pacote aparece instalado como `package:com.rafael.teste`.

O que ainda não deve ser afirmado: runtime NativeActivity efetivo, ausência de crash no lançamento, reprodutibilidade source→binary do executável `apkc`, compatibilidade ARM64, compatibilidade multi-API, ou origem plena bit-a-bit do APK a partir do commit atual.

## FNext — próximo comando necessário

```sh
bash scripts/apkc_validate.sh
bash scripts/apkc_sign_debug.sh
bash scripts/apkc_install_android.sh
adb shell monkey -p com.rafael.teste -c android.intent.category.LAUNCHER 1
adb logcat -d | grep -i -E 'NativeActivity|AndroidRuntime|dlopen|apkc|fatal|crash'
```

A prova só fecha o ciclo runtime quando `Apkc/proofs/out/logcat-nativeactivity.txt` trouxer saída real sem crash relevante, com data UTC, device, commit e hash do APK.

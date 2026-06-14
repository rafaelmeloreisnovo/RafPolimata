# ApkC — prova mínima em Termux/Android ARM32

Este documento registra a prova mínima observada em 2026-06-14: o ApkC foi compilado e usado no próprio Termux/Android ARM32 para gerar, assinar, instalar e abrir um APK Android mínimo.

## Ambiente observado

- Diretório: `$HOME/RafPolimata-main/Apkc`
- Plataforma: Termux em Android ARM32/ARMv7l
- Compilador usado: `cc`
- Assinador usado: `apksigner` instalado via Termux
- Pacote gerado: `com.rafael.teste`
- Biblioteca nativa: `lib/armeabi-v7a/libhello.so`

## Comando mínimo que funcionou

O ponto crítico foi usar `-nostartfiles`, não `-nostdlib`.

```sh
cd "$HOME/RafPolimata-main/Apkc"
mkdir -p out

cc -std=c11 -Oz -Wno-unused-function \
  -nostartfiles -Wl,-e,_start \
  apkc.c -o out/apkc

chmod +x out/apkc

out/apkc hello.s.txt \
  -o out/hello.apk \
  -p com.rafael.teste \
  -l RafaelTeste \
  -n hello \
  -32

unzip -l out/hello.apk
```

## Saída ZIP/APK observada

```text
Archive:  out/hello.apk
  Length      Date    Time    Name
---------  ---------- -----   ----
      660  1980-00-00 00:00   lib/armeabi-v7a/libhello.so
     1848  1980-00-00 00:00   AndroidManifest.xml
      140  1980-00-00 00:00   classes.dex
---------                     -------
     2648                     3 files
```

## Manifest AXML validado por `aapt`

```sh
aapt dump xmltree out/hello.apk AndroidManifest.xml
```

Campos observados:

- `package="com.rafael.teste"`
- `android.app.NativeActivity`
- `android.app.lib_name="hello"`
- `android.intent.action.MAIN`
- `android.intent.category.LAUNCHER`

## ELF ARM32 validado por `readelf`

```sh
unzip -p out/hello.apk lib/armeabi-v7a/libhello.so > out/libhello.so
readelf -h out/libhello.so
readelf -s out/libhello.so | head -80
```

Achados observados:

```text
Class: ELF32
Type: DYN (Shared object file)
Machine: ARM
```

Símbolos exportados observados:

```text
ANativeActivity_onCreate
android_main
```

## DEX SHA-1 validado

```sh
python3 - <<'PY'
import zipfile, hashlib
apk="out/hello.apk"
dex=zipfile.ZipFile(apk).read("classes.dex")
header=dex[12:32]
calc=hashlib.sha1(dex[32:]).digest()
print("header:", header.hex())
print("calc:  ", calc.hex())
print("PASS" if header == calc else "FAIL")
PY
```

Resultado observado:

```text
header: 9ea7c00884bffbdbbab055ddc3ad6565050fc4e4
calc:   9ea7c00884bffbdbbab055ddc3ad6565050fc4e4
PASS
```

## Assinatura APK validada

```sh
keytool -genkeypair \
  -keystore out/debug.keystore \
  -storepass android \
  -keypass android \
  -alias apkc-debug \
  -keyalg RSA \
  -keysize 2048 \
  -validity 10000 \
  -dname "CN=ApkC Debug,O=Rafael,C=BR"

apksigner sign \
  --ks out/debug.keystore \
  --ks-key-alias apkc-debug \
  --ks-pass pass:android \
  --key-pass pass:android \
  --min-sdk-version 21 \
  --out out/hello-signed.apk \
  out/hello.apk

apksigner verify --verbose --print-certs out/hello-signed.apk
```

Resultado observado:

```text
Verifies
Verified using v1 scheme (JAR signing): true
Verified using v2 scheme (APK Signature Scheme v2): true
Verified using v3 scheme (APK Signature Scheme v3): true
Number of signers: 1
Signer #1 certificate DN: CN=ApkC Debug, O=Rafael, C=BR
Signer #1 key algorithm: RSA
Signer #1 key size (bits): 2048
```

Observação: `v3.1`, `v4` e `SourceStamp` apareceram como `false`, o que é aceitável para prova local/debug.

## Instalação confirmada

Arquivo de prova produzido:

```sh
cmd package list packages | grep com.rafael.teste
```

Resultado observado:

```text
package:com.rafael.teste
```

O app também foi aberto manualmente pelo usuário.

## SHA-256 dos artefatos observados

```text
a331d0248d01d8e7030291e93905c2e2f046cf7cb5ba4ecaf02609cec273c024  out/hello.apk
063c1b61c35e45f3cf253d42c99bfcd58910162c46ba6c5160846b56651dcc28  out/hello-signed.apk
```

## Claim permitido

```text
ApkC gera no Termux/Android ARM32 um APK mínimo assinado, instalável e abrível no Android, contendo Manifest AXML válido, classes.dex com SHA-1 correto e libhello.so ARM32 válida com símbolos NativeActivity.
```

## Claim ainda não coberto por esta prova

- Execução funcional interna da NativeActivity via logcat.
- Teste em ARM64.
- Compatibilidade ampla de Android versions/devices.
- Alinhamento/signing de produção.
- Suporte completo a instruções ARM/AArch64.

# Branch protection policy — main

Esta política registra a proteção desejada para a branch `main` após a release funcional `v0.1.0-apkc-termux-arm32-proof`.

## Objetivo

Preservar o estado funcional já provado do ApkC:

- gera APK mínimo no Termux/Android ARM32;
- Manifest AXML válido via `aapt`;
- DEX SHA-1 válido;
- ELF ARM32 válido via `readelf`;
- APK assinado via `apksigner` v1/v2/v3;
- instalação confirmada por `cmd package list packages`;
- app aberto manualmente.

## Regras desejadas no GitHub

Branch name pattern:

```text
main
```

Regras:

- Require a pull request before merging.
- Require at least 1 approval.
- Dismiss stale approvals when new commits are pushed.
- Require conversation resolution before merging.
- Require status checks when CI is stable.
- Require branches to be up to date when status checks are enabled.
- Do not allow force pushes.
- Do not allow branch deletion.

## Política de release

- `main` deve ficar como base funcional estável.
- PRs grandes devem entrar como draft até terem prova mínima reproduzível.
- PR #19 é laboratório de expansão, não requisito da release atual.
- Mudanças em `Apkc/apkc.c`, `Apkc/arch_arm32.h`, `Apkc/arch_arm64.h` e `Apkc/fmt_*` exigem teste simples ou evidência equivalente.

## Gate mínimo para mudanças no ApkC

Antes de mergear mudanças no núcleo ApkC, rodar no mínimo:

```sh
cd Apkc
cc -std=c11 -Oz -Wno-unused-function -nostartfiles -Wl,-e,_start apkc.c -o out/apkc
out/apkc hello.s.txt -o out/hello.apk -p com.rafael.teste -l RafaelTeste -n hello -32
unzip -l out/hello.apk
aapt dump xmltree out/hello.apk AndroidManifest.xml
```

DEX SHA-1:

```sh
python3 - <<'PY'
import zipfile, hashlib
apk='out/hello.apk'
dex=zipfile.ZipFile(apk).read('classes.dex')
assert dex[12:32] == hashlib.sha1(dex[32:]).digest()
print('DEX_SHA1_PASS')
PY
```

Se assinatura ou instalação forem afetadas:

```sh
apksigner verify --verbose --print-certs out/hello-signed.apk
cmd package list packages | grep com.rafael.teste
```

## TOKEN_VAZIO

Se alguma ferramenta estiver ausente, registrar `TOKEN_VAZIO` com o nome da ferramenta ausente. Não transformar ausência de ferramenta em PASS.

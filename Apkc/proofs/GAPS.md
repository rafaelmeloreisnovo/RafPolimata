# Gaps ApkC

| Gap | Status | Proxima acao |
|---|---|---|
| Build documentado de apkc.c | TOKEN_VAZIO | gerar apkc-compile.txt com comando, commit e toolchain |
| Geracao de hello.apk | PASS | manter apkc-generate.txt e hash SHA-256 |
| Avisos unknown ARM32 mnemonic | GAP ABERTO | auditar cobertura do assembler ARM32 |
| Parser ZIP | PASS | manter unzip.txt |
| Parser AXML | PASS | manter aapt-xmltree.txt |
| ELF ARM32 | PASS | manter readelf-arm32.txt |
| ELF ARM64 | SKIP | gerar artefato arm64 ou declarar escopo ARM32 |
| DEX SHA-1 | PASS | manter dex-sha1.txt |
| Assinatura APK | PASS | manter apksigner-verify.txt |
| Package instalado/visivel | PASS limitado | arquivar saida completa de instalacao |
| Runtime NativeActivity | TOKEN_VAZIO | arquivar logcat real do lancamento |
| Reprodutibilidade completa | TOKEN_VAZIO | registrar no mesmo run: commit, comando, ambiente, binario e APK |

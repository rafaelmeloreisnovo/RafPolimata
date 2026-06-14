# Artefatos esperados

| Artefato | Tipo | Versionar? | Origem |
|---|---|---:|---|
| `Apkc/proofs/out/hello.apk` | APK gerado | Não | `scripts/apkc_validate.sh` |
| `Apkc/proofs/out/hello-signed.apk` | APK assinado debug local | Não | `scripts/apkc_sign_debug.sh` |
| `Apkc/proofs/out/unzip.txt` | saída bruta | Sim | `unzip -l` |
| `Apkc/proofs/out/aapt-xmltree.txt` | saída bruta | Sim | `aapt dump xmltree` ou `TOKEN_VAZIO` |
| `Apkc/proofs/out/readelf-arm64.txt` | saída bruta | Sim | `readelf -h/-s` |
| `Apkc/proofs/out/readelf-arm32.txt` | saída bruta | Sim | `readelf -h/-s` |
| `Apkc/proofs/out/dex-sha1.txt` | relatório | Sim | validador Python embutido |
| `Apkc/proofs/out/apksigner-verify.txt` | saída bruta | Sim | `apksigner verify --verbose` ou `TOKEN_VAZIO` |
| `Apkc/proofs/out/adb-install.txt` | saída bruta | Sim | `adb devices/install` ou `TOKEN_VAZIO` |
| `Apkc/proofs/out/logcat-nativeactivity.txt` | saída bruta | Sim | `adb logcat -d` filtrado ou `TOKEN_VAZIO` |
| `Apkc/proofs/debug.keystore` | chave debug local | Não | `keytool` |

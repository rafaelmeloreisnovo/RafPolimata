# Matriz de validação ApkC

| Ferramenta | Fase | Comando | Status inicial | Saída |
|---|---|---|---|---|
| `unzip` | ZIP parse | `unzip -l hello.apk` | NOT_RUN | `Apkc/proofs/out/unzip.txt` |
| `aapt` | AXML parse | `aapt dump xmltree hello.apk AndroidManifest.xml` | TOKEN_VAZIO | `Apkc/proofs/out/aapt-xmltree.txt` |
| `readelf` | ELF header/symbols arm64 | `readelf -h/-s lib/arm64-v8a/*.so` | NOT_RUN | `Apkc/proofs/out/readelf-arm64.txt` |
| `readelf` | ELF header/symbols arm32 | `readelf -h/-s lib/armeabi-v7a/*.so` | NOT_RUN | `Apkc/proofs/out/readelf-arm32.txt` |
| `apksigner` | APK signing/verify | `apksigner verify --verbose hello-signed.apk` | TOKEN_VAZIO | `Apkc/proofs/out/apksigner-verify.txt` |
| `adb` | install | `adb install -r hello-signed.apk` | TOKEN_VAZIO | `Apkc/proofs/out/adb-install.txt` |
| `logcat` | runtime evidence | `adb logcat -d | grep -i -E ...` | TOKEN_VAZIO | `Apkc/proofs/out/logcat-nativeactivity.txt` |

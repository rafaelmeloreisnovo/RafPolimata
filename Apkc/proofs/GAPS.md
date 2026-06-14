# Gaps ApkC

| Gap | Status | Próximo comando |
|---|---|---|
| Compilação host de `apkc.c` | NOT_RUN | `bash scripts/apkc_validate.sh` |
| Geração de `hello.apk` | NOT_RUN | `bash scripts/apkc_validate.sh` |
| Parser ZIP por `unzip` | NOT_RUN | `bash scripts/apkc_validate.sh` |
| Parser AXML por `aapt` | TOKEN_VAZIO | `command -v aapt || echo TOKEN_VAZIO` |
| Verificação ELF por `readelf` | NOT_RUN | `bash scripts/apkc_validate.sh` |
| SHA-1 interno de `classes.dex` | NOT_RUN | `bash scripts/apkc_validate.sh` |
| Assinatura APK por `apksigner` | TOKEN_VAZIO | `bash scripts/apkc_sign_debug.sh` |
| Instalação Android por `adb` | TOKEN_VAZIO | `bash scripts/apkc_install_android.sh` |
| Runtime NativeActivity/logcat | TOKEN_VAZIO | `bash scripts/apkc_install_android.sh` |

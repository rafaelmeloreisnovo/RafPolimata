# ApkC Android Runtime Proof Plan

Caminho mínimo: source → build → APK → sign → install → launch → logcat → verdict → artifact.

Este plano é `DEVICE_REQUIRED`. Sem `adb`, device/emulator, NDK, Android SDK ou `apksigner`, registrar `SKIPPED`/`TOKEN_VAZIO`, nunca PASS falso.

## Artefatos esperados

| Artefato | Estado sem device/ferramenta | Conteúdo esperado |
|---|---|---|
| `Apkc/proofs/out/device-info.txt` | `TOKEN_VAZIO` | `adb shell getprop`, ABI, SDK, fabricante/modelo |
| `Apkc/proofs/out/apkc-compile.txt` | `SKIPPED` ou `PASS_LIMITED` | comando, compilador, flags, stdout/stderr |
| `Apkc/proofs/out/apk-generate.txt` | `SKIPPED` ou `PASS_LIMITED` | geração do APK e hash |
| `Apkc/proofs/out/apksigner-verify.txt` | `SKIPPED` ou `PASS_LIMITED` | verificação v1/v2/v3/v4 quando ferramenta existir |
| `Apkc/proofs/out/adb-install-full.txt` | `DEVICE_REQUIRED` | saída completa de `adb install -r` |
| `Apkc/proofs/out/adb-launch.txt` | `DEVICE_REQUIRED` | comando de launch e resultado |
| `Apkc/proofs/out/logcat-nativeactivity.txt` | `DEVICE_REQUIRED` | logcat filtrado com NativeActivity/crash |
| `Apkc/proofs/out/runtime-verdict.json` | `TOKEN_VAZIO` até completar | verdict, hashes, device, timestamps, logs |

## Comandos mínimos sugeridos

```bash
mkdir -p Apkc/proofs/out
adb devices -l | tee Apkc/proofs/out/device-info.txt
bash scripts/apkc_validate.sh | tee Apkc/proofs/out/apkc-compile.txt
bash scripts/apkc_sign_debug.sh | tee Apkc/proofs/out/apksigner-verify.txt
adb install -r Apkc/hello.apk | tee Apkc/proofs/out/adb-install-full.txt
adb shell monkey -p com.rafael.teste 1 | tee Apkc/proofs/out/adb-launch.txt
adb logcat -d | grep -E 'NativeActivity|rafael|crash|FATAL' | tee Apkc/proofs/out/logcat-nativeactivity.txt
```

## Verdict

`runtime-verdict.json` só pode conter `PASS` quando houver install completo, launch registrado e logcat compatível sem crash para o pacote testado. Caso contrário, usar `TOKEN_VAZIO`, `SKIPPED`, `PASS_LIMITED` ou `DEVICE_REQUIRED`.

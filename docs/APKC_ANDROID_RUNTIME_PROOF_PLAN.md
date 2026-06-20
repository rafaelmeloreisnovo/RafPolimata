# ApkC Android Runtime Proof Plan

Estado: `DEVICE_REQUIRED`

Caminho mínimo de prova:

```text
source -> build -> APK -> sign/verify -> install -> launch -> logcat -> verdict -> artifact
```

| Gate | Artefato esperado | Estado sem ferramenta/device | Evidência para promoção |
|---|---|---|---|
| device info | `Apkc/proofs/out/device-info.txt` | `DEVICE_REQUIRED` | modelo, ABI, SDK, kernel |
| compile apkc | `Apkc/proofs/out/apkc-compile.txt` | `TOKEN_VAZIO` | comando + rc=0 |
| generate apk | `Apkc/proofs/out/apk-generate.txt` | `TOKEN_VAZIO` | APK existe + hash |
| sign/verify | `Apkc/proofs/out/apksigner-verify.txt` | `SKIPPED`/`TOKEN_VAZIO` | verificação registrada |
| install | `Apkc/proofs/out/adb-install-full.txt` | `DEVICE_REQUIRED` | adb/pm install rc=0 |
| launch | `Apkc/proofs/out/adb-launch.txt` | `DEVICE_REQUIRED` | intent/monkey rc=0 |
| logcat | `Apkc/proofs/out/logcat-nativeactivity.txt` | `DEVICE_REQUIRED` | sem fatal/crash relevante |
| verdict | `Apkc/proofs/out/runtime-verdict.json` | `TOKEN_VAZIO` | JSON com status e hashes |

## Regra de veredito

- `PASS`: todos os gates obrigatórios executaram com evidência no mesmo run.
- `PASS_LIMITED`: build/artefato parcial passou, mas runtime/device ausente.
- `FAIL`: qualquer gate obrigatório falhou.
- `TOKEN_VAZIO`: ferramenta, device, dataset, log ou prova ausente.

Sem `adb`, device, NDK, apksigner ou Android SDK: registrar `TOKEN_VAZIO`, `SKIPPED` ou `DEVICE_REQUIRED`; nunca `PASS`.

# ApkC hermético no Termux

`scripts/apkc_termux_hermetic_build.sh` cria um APK NativeActivity mínimo no
próprio Android/Termux. O fluxo usa somente o compilador C local para iniciar
o ApkC e o montador interno do ApkC para gerar AXML, DEX mínimo, ELF e ZIP/APK.

Ele não baixa nem resolve dependências, e não usa Gradle, Android Studio,
Android SDK, JDK, Maven, `aapt`, `d8` ou `zipalign`.

## Uso

```sh
cd RafPolimata
sh scripts/apkc_termux_hermetic_build.sh --abi both
```

Para `armeabi-v7a`:

```sh
sh scripts/apkc_termux_hermetic_build.sh --abi armeabi-v7a
```

Para `arm64-v8a`:

```sh
sh scripts/apkc_termux_hermetic_build.sh --abi arm64-v8a
```

O artefato unsigned fica em `build/apkc-hermetic/`. A assinatura é opcional e
não cria nem armazena chaves. Quando uma chave local já existe, use
`apksigner`:

```sh
APKSIGNER_KEYSTORE=/caminho/chave.jks \
APKSIGNER_ALIAS=meu_alias \
APKSIGNER_KS_PASS_FILE=/caminho/senha-keystore.txt \
APKSIGNER_KEY_PASS_FILE=/caminho/senha-chave.txt \
sh scripts/apkc_termux_hermetic_build.sh --abi both --sign
```

`apktool` não assina APKs; a ferramenta de assinatura deste contrato é
`apksigner`.

## Contrato técnico

| Campo | Estado |
|---|---|
| Rede durante o build | não usada |
| Gradle / Maven / SDK / JDK | não usados |
| Entrada | assembly no subconjunto interno do ApkC |
| ABIs | `armeabi-v7a`, `arm64-v8a` ou ambas |
| Assinatura | opcional, somente com `apksigner` e chave local existente |
| Evidência | `build/apkc-hermetic/receipt.env` e `build.log` |

O bootstrap atual do ApkC usa `-nostartfiles`, pois é a rota comprovada no
Termux ARM32. Isso não promove a alegação de que o executável bootstrap já é
`-nostdlib`; essa lacuna continua explícita na documentação do ApkC.

## Limite deliberado

Este caminho cria uma NativeActivity mínima. Ele não compila C/Kotlin/Compose
arbitrário e não substitui um build completo de aplicativo Android. A execução
no aparelho, a validação estrutural independente e a adequação para páginas de
16 KiB continuam `TOKEN_VAZIO` até uma rodada de prova com os artefatos do
mesmo commit.

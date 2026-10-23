# RAFAELIA Ω — RafPolimata ApkC como laboratório de compilação federado (C04)

**Estágio:** SOURCE_IMPLEMENTED / EXACT_HEAD_CI_PENDING / DRAFT / NÃO PROMOVER.

## Objetivo mensurável
Criar um **candidato** a APK Android dual-ABI por GitHub Actions, reutilizando os próprios falsificadores e o `scripts/apkc_verified_build.py` que já existem no RafPolimata, sem recriar ou substituir o compilador ApkC. Este pacote só adiciona workflow/contrato de interoperação; não muda o código das rotas de compilação, ABI, assinaturas, licenças ou artefatos originais.

O RafGitTools faz a composição e o controle de estado; o Mapa possui autoridade federada de roteamento; o RafPolimata continua sendo autor e proprietário de seus artefatos e seus testes. Nenhuma alteração em um repo presume sucesso de outro.

## Fluxo de prova por etapa
1. `SOURCE_COMMIT` = SHA exato do HEAD do PR, checkout sem credencial persistente.
2. Host (GitHub Actions) verifica toolchain; **Clang/QEMU/Ubuntu/Python são dependências do ambiente de compilação**, não do core freestanding. Se indisponíveis, a execução falha — nunca `SKIPPED=PASS`.
3. Executar testes existentes `tests.test_apkc_verified_build`, `tests.test_validate_apkc_formats` e `tests.test_apkc_structural_closure_receipt`.
4. Reproduzir `tools/raf_source_to_binary_proof.sh`, cuja auditoria constrói ARM32 e ARM64 e rejeita saída parcial.
5. Executar `scripts/apkc_verified_build.py` com `--require-both`, preservando o validador preexistente de ZIP/APK/DEX/ELF. O aplicativo C04 demonstra empacotamento estrutural: **não** é uma interface de navegação, um orquestrador de IA, uma estação operacional no aparelho nem um navegador funcional.
6. Confirmar SHA do APK igual ao relatório; gravar `candidate-receipt.json`, com `claim_allowed=false`, físico `NOT_RUN`, assinatura `TOKEN_VAZIO`.
7. Enviar apenas candidato/recibos como artifact temporário; **sem GitHub Release, sem deploy, sem instalação, sem credencial/PAT, sem executar código privado**.

## Falsificação e rollback
Falhar imediatamente para: SHA de origem divergente, binário ausente, compilação parcial ARM32/ARM64, QEMU ausente, teste anterior falho, `validation.state != PASS`, relatório sem identidade da saída, APK vazio ou divergência SHA. Uma falha desse escopo não é licença para relaxar o falsificador.

Rollback: reverter os arquivos novos deste PR Draft; `main` continua intacta.

## Próximas fronteiras
- **P0**: executar workflow em PR HEAD exato, provar artefato com hash, revisar pela equipe humana e preservar erro real em vez de inventar sucesso.
- **P0**: build independente/reprodutível e revisão de proveniência/licenças.
- **P0 físico**: APK assinado para teste controlado, instalação, launch/dlopen no ARM32 e ARM64, registros com hash de pacote idêntico e privacidade/proteção infantil.
- **P1**: futura UI do laboratório em campo, se autorizada e implementada em uma ponte Android separada do core L0; não usar o APK C04 como prova desse produto.

**Governança:** referências orientativas de DMAIC/Six Sigma, ISO 8000/9001, família ISO/IEC 27000, NIST, legislação aplicável de proteção de dados e direitos de crianças/adolescentes. Nenhuma certificação, conformidade jurídica abrangente ou autorização comercial decorre deste PR.

`SOURCE != ARTIFACT != EXECUTION != EVIDENCE != CLAIM`
`TOKEN_VAZIO != 0`
`HOST_PASS != DEVICE_PASS`
`claim_allowed=false, no_merge=true, no_release=true`

## V2 — Corretivo de composição multifuncional (2026-10-22)

A análise dos próprios repositórios foi feita antes da implementação:
- BLAKE3 `rmr/android`: Gradle/NDK/JNI, três bibliotecas de provedores por ABI, licenças anexadas e assinatura de debug. Seus provedores não são três algoritmos distintos; todos fornecem BLAKE3.
- EstudioAudio `app/src/main/cpp`: núcleo DSP/ZIP32/WAV, Java/Android de aplicação e player; o guia de teleprompter documenta síntese de **16 kHz**, e não uma alegação generalizada de 48 kHz para toda rota de voz.
- RLL `core/lowlevel_runtime` e `app`: objetos C freestanding e provas matemáticas distintas de um APK Android Gradle.
- Nenhuma medida exata de tamanho atual desses três APKs foi homologada por esta execução. Não inferir um valor a partir de tamanho de fonte/zip histórico.

**Falha do V1:** o `Apkc/hello.dualabi.structural.s.txt` contém duas declarações de símbolo e `nop`, e o APK original de 4.454 bytes tinha só 4 entradas: `classes.dex`, `AndroidManifest.xml`, `lib/arm64-v8a/librafpolimata_c04.so`, `lib/armeabi-v7a/librafpolimata_c04.so`. Estrutura ZIP mínima NÃO constitui estação de laboratório em campo.

**V2 aditivo, sem substituir o compilador autoral nem transferir implementações de outros repos:**
- `field_lab/l0/raf_field_lab_core.c`: SHA3-256 (FIPS 202; distinguir de BLAKE3), PCM16 mono 48.000 amostras/s em buffer do chamador, cabeçalho WAV 44B, DFT de 16 amostras/8 bins (analisador de **janela curta**, não espectrometria calibrada), ZIP32 com um arquivo STORED e validação de nome/capacidade.
- `tests/test_field_lab_l0.py`: roda a biblioteca freestanding real no host e cruza SHA3 com `hashlib.sha3_256`, WAV com `wave`, ZIP com `zipfile`, além de controles negativos, PCM e DFT. Não há chamada a essas bibliotecas Python no código L0 nem em tempo de execução Android.
- `scripts/field_lab_repack.py` e `tests/test_field_lab_repack.py`: conservam os 4 itens do C04 original bit a bit; adicionam as bibliotecas ELF reais `lib/armeabi-v7a/libraffieldlab.so` e `lib/arm64-v8a/libraffieldlab.so`. Rejeita nomes duplicados, dados inesperados, ELF de arquitetura errada, entrada no mesmo caminho, limite de 256 KiB.
- **Gate CI rigoroso:** `clang --target=armv7a-linux-androideabi24` e `--target=aarch64-linux-android24` com `-nostdinc -nostdlib -ffreestanding -fno-builtin -shared -Wl,--no-undefined`; audit `DT_NEEDED` e símbolos indefinidos, cabeçalho ELF, ambas ABIs, SHA e validação do APK final por validador já existente.
- O ELF ativo é funcional **como código embarcado**; mas não existe, neste V2, um chamador Android/JNI/NativeActivity que encaminhe os controles da UI a esse L0. Logo, **biblioteca compilada ≠ módulo carregado ≠ funcionalidade visível**.
- `Apkc/hello.dualabi.structural.s.txt` e as 4 entradas originais preservados; V1 original e todos os relatórios negativos preservados, V2 sucessor com receipt próprio. Limite 256 KiB é *teto de orçamento*, não razão para inserir bytes artificiais.

**Hotfix não regressivo observado durante integração:** no ARM32, a inicialização C `a[25]={0}` do estado SHA3 foi transformada por Clang em `__aeabi_memclr8`, símbolo externo que o gate corretamente rejeitou. Corrigido com gravações escalares voláteis e novo exato SHA; o gate `--no-undefined` continua estrito e qualquer regressão reprova.

**Ausências tipadas:** `ANDROID_UI=TOKEN_VAZIO_NOT_CONNECTED`; `LOADER=TOKEN_VAZIO`; `APK_INSTALLABLE=TOKEN_VAZIO_NOT_TESTED`; `SIGNING=TOKEN_VAZIO`; `DEVICE_ARM32/ARM64=NOT_RUN`; `PRIVACY_LEGAL_COMPLETE=TOKEN_VAZIO`; `claim_allowed=false`.

**F_next:** depois de CI exact-head e recibo V2 válido, construir ponte de interface/loader e testar instalação e observação física no aparelho com SHA idêntico. Não confundir esse estágio com aprovação ou publicação.

## V3 — Vínculos de lacunas e sucessor Android (2026-10-23)

A seção V2 acima descreve o estágio histórico sem launcher. O código da PR #441 agora contém `FieldLabActivity`, manifesto compilável e uma ponte JNI nativa com build de APK debug de teste; isto é *fonte candidata*, não execução física. Nenhum estado da V2 é apagado.

- `CLOSURE_L2` (`docs/closures/CLOSURE_L2_RUNTIME_EVIDENCE.md`): governa os `TOKEN_VAZIO` de instalação, launch, loader e execução física, sem fechá-los. Evidência de fechamento: identidade SHA-256 de APK/ELF no aparelho, instalação, execução do JNI/autoteste, captura e readback.
- `CLOSURE_L10` (`docs/closures/CLOSURE_L10_APK_SECURITY.md`): distingue verificação `apksigner` da assinatura de debug efêmera e da assinatura autoral/release ainda não observada.
- `CLOSURE_L5` (`docs/closures/CLOSURE_L5_FFI_VALIDATION.md`): contrato de fronteira FFI; presença do símbolo JNI no ELF não prova chamada executada no aparelho.

Gate CI afetado: `tools/validate_token_vazio_gates.py --changed-since <BASE_SHA> --strict`; CLOSURE é vínculo de governança, nunca conversão `TOKEN_VAZIO → PASS`. Reexecutar no HEAD deste hotfix, preservar relatórios de falha anteriores e manter `claim_allowed=false`, `no_release=true`.

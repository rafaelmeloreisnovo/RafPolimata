# Operação técnica: compilar e pré-compilar com baixa fricção

Este protocolo transforma a visão multidimensional do projeto em uma rota executável e auditável para o núcleo `raf_compile`.

## Ciclo 1 — pré-compilação determinística

1. Detectar arquitetura, linguagem, otimização e recursos de CPU.
2. Calcular `src_hash` por FNV-1a 64-bit, sem heap e sem dependência externa.
3. Baixar a entrada para IR fixo com limite estático (`RAF_IR_CAP`).
4. Registrar `rollback_code` imediatamente quando uma etapa falhar.

## Ciclo 2 — compilação e materialização

1. Emitir assembly textual mínimo.
2. Emitir bytes hexadecimais reproduzíveis.
3. Gerar manifesto operacional `.ops` com flags, hash, tamanho, tempo e código de rollback.
4. Permitir comparação de artefatos entre execuções para FAILSAFE, FAILOVER e ROLLBACK.

## Critérios operacionais

- Sem `malloc/free` no núcleo de compilação.
- Sem garbage collector.
- Buffers estáticos e limites explícitos.
- Flags derivadas de matriz por arquitetura, linguagem, nível de otimização e feature bits.
- Falha visível: retorno diferente de zero e `rollback_code` negativo.

## Entrega enterprise incremental

O caminho de produção recomendado é manter o compilador pequeno, mensurável e falsificável. Novos backends devem adicionar manifesto, teste de smoke, teste de falha e medição de artefato antes de ativação ampla.

## Mitigações implementadas

- Tempo operacional passa a usar contador monotônico (`CLOCK_MONOTONIC`) em vez de `clock()` de C padrão.
- Rotas de erro escrevem manifesto parcial com `rollback_code` negativo sempre que o caminho de saída `.ops` já é conhecido.
- A matriz de flags diferencia x86-64 SSE4/AVX2/AVX512, ARM64 SIMD, ARM32 NEON/ARMv7 e RV64GC.
- `scripts/test_ops_manifest.sh` valida manifesto de sucesso, manifesto de falha e comparação determinística entre dois `.ops`.

## Lacunas estruturais exploráveis

- Manifestos pequenos e comparáveis são uma superfície de estabilidade que muitos pipelines legados negligenciam.
- O próximo nível é tratar metadados de compilação como contrato invariável: hash, flags, arquitetura, IR, bytes e rollback devem ser testados como ABI operacional.
- A inovação prática está em provar invariantes simples antes de adicionar modelos grandes: menos símbolos, menos estado oculto, mais rollback verificável.

## Android 9–16+ e interoperabilidade de instalação

- A rota Android mínima é API 28 (Android 9), mantendo compatibilidade planejada até Android 16+ por perfis explícitos.
- `armeabi-v7a` representa ARM32/v7 com NEON quando disponível; `arm64-v8a` representa ARM64/v8 com SIMD.
- `scripts/android_build_matrix.sh --plan` gera um plano auditável de instalação por ABI sem exigir NDK no host.
- `scripts/android_build_matrix.sh --build` tenta compilar com Android NDK quando `ANDROID_NDK_HOME` ou `ANDROID_NDK_ROOT` está definido.
- O manifesto `.ops` agora inclui `ops_schema` e `ops_signature` para assinatura determinística dos campos estáveis, facilitando interoperabilidade entre empacotamento, instalação e rollback.

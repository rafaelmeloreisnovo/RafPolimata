# RafPolimata — ponte para a Invariante Evolutiva Absoluta V1

**Autoridade canônica:** `rafaelmeloreisnovo/Mapa`  
**Documento:** `governanca/invariantes/INVARIANTE_EVOLUTIVA_ABSOLUTA_V1.md`  
**Estado local:** `RESPONSIBILITY_POINTER`  
**Claim:** `claim_allowed=false`

## Responsabilidade deste repositório

Dentro da IEA, o RafPolimata preserva a cadeia de formalização e produção de evidência:

```text
conceito
→ contrato
→ implementação
→ compilação
→ execução delimitada
→ artefato
→ receipt
→ limite
```

Mudanças de linguagem, codec, arquitetura, toolchain ou representação são permitidas quando mantêm identidade de fonte, invariantes testáveis, comandos de reprodução, hashes, failure mode e rollback.

## Evidência delimitada do espelho auditado

```text
ZIP: RafPolimata-main (4).zip
SHA-256: b88f88d43c2dc441ee0e65038f1479fa9bff2032e66219cc49422ddc62a542ae
arquivos: 853
estado: MIRROR_HEAD_HIGH_CONFIDENCE_CORE
BITRAF: 4/4 PASS
ABI segment.v1: 3/3 PASS
bounded reader C estrito: PASS
```

## Fronteiras preservadas

```text
conceito ≠ implementação
implementação ≠ execução
execução local ≠ produção
receipt próprio ≠ replicação independente
formato gerado ≠ instalação Android validada
```

## Próximo fechamento local

1. ligar receipts a commits e ambientes exatos;
2. repetir BITRAF e segment.v1 em ARM32/ARM64;
3. reproduzir em segundo ambiente independente;
4. promover claims apenas depois do gate correspondente.

A formalização pode ganhar novas camadas; o vínculo entre definição, bytes executados e prova não pode desaparecer.

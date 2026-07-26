# C02 — Runtime Truth Receipt

**Base de implementação:** `b230f79b4519f398d6a0f32c2235527966f14a36`  
**Estado inicial:** `EXECUTION_PENDING`  
**Claim:** `claim_allowed=false`

## Finalidade

O gate existente `scripts/validate_runtime_truth_local.sh` executa nove etapas de validação, mas usa um diretório temporário removido ao final. Este ciclo adiciona uma camada de cadeia de custódia que preserva logs, relatório do Build Doctor, toolchain e hashes sem alterar a semântica do compilador.

## Execução local

```sh
bash scripts/run_runtime_truth_receipt.sh artifacts/runtime-truth
```

O primeiro argumento é o diretório de saída. O script não instala pacotes, não baixa dependências, não modifica Git e não acessa corpus privado.

## Artefatos

```text
artifacts/runtime-truth/
├── runtime-truth.stdout.log
├── runtime-truth.stderr.log
├── build-doctor.stdout.log
├── build-doctor.stderr.log
├── build-doctor.json
├── build-doctor.md
├── toolchain_manifest.json
└── runtime_truth_receipt.json
```

## Estados

- `PASS`: gate integral e Build Doctor retornaram zero e os artefatos obrigatórios existem;
- `FAIL`: o gate integral ou o Build Doctor retornou código diferente de zero;
- `INCOMPLETE`: comandos retornaram zero, mas algum artefato obrigatório não foi preservado.

## Hashes

SHA-256 é calculado internamente em Python e é obrigatório para os arquivos existentes. BLAKE3 é calculado por `b3sum` quando disponível. A ausência da ferramenta é registrada como `TOKEN_VAZIO_B3SUM_ABSENT`, nunca substituída silenciosamente por outro algoritmo.

## Fronteira epistemológica

Uma execução positiva pode promover somente:

```yaml
repository_local_truth: VERIFIED_BY_EXECUTION
static_analysis: VERIFIED_BY_EXECUTION
host_build_execution: VERIFIED_BY_EXECUTION
```

Ela preserva:

```yaml
android_build: TOKEN_VAZIO
apk_install: TOKEN_VAZIO
physical_runtime: TOKEN_VAZIO
performance_claim: FORBIDDEN_OUT_OF_SCOPE
claim_allowed: false
```

## GitHub Actions

O workflow `.github/workflows/runtime-truth-receipt.yml` executa o wrapper em pull requests que alterem o núcleo relevante ou por `workflow_dispatch`. O pacote é enviado por `actions/upload-artifact@v4` mesmo quando o gate falha, permitindo auditar stdout, stderr e receipts parciais.

Workflow não iniciado, indisponível por infraestrutura ou sem crédito não significa PASS nem FAIL do código. Nesses casos, a execução permanece `TOKEN_VAZIO_EXECUTION_NOT_OBSERVED`.

## Falsificadores

- exit code não zero;
- commit ausente ou divergente;
- logs obrigatórios ausentes;
- SHA-256 ausente;
- relatório do Build Doctor ausente;
- promoção indevida de host PASS para Android/device;
- alteração semântica do compilador misturada à instrumentação.

## Relação com os próximos ciclos

Este ciclo fornece a verdade local necessária para o indexador (`C03`) e para a produção de artefatos ApkC (`C04`). Não substitui os gates de NDK, APK, assinatura, instalação, Termux, QEMU ou guest boot.

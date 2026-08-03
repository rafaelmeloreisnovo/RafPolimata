# RAFAELIA B7 — Runtime Federado de Memória, SIMD e Evidência

Estado global neste corte: `IMPLEMENTED / HOST_SELFTEST_LOCAL / DEVICE_TOKEN_VAZIO`.

## Invariante operacional

```text
região fornecida pelo chamador
  ├─ banco A: INGEST
  ├─ banco B: COMPUTE
  ├─ banco C: EGRESS
  └─ cache: CRC32C das 16 lanes + scratch controlado
```

A cada época os papéis giram. Nenhum endereço é mascarado ou reconstruído a partir dos bits baixos: a base permanece íntegra e cada banco conserva endereço lógico, capacidade, uso, estado e CRC32C.

## Composição de forças

| Corpo | Papel do B7 |
|---|---|
| Vectras VM Android | fonte canônica RMR, NEON/SIMD e integração de engine |
| Termux RAFCODEΦ | build nomalloc ARMv7/ARM64 e execução física futura |
| GAIA_phi | núcleo C formal e `ctest` estrutural |
| Rafaelia_Private | adaptador isolado para runtimes privados |
| RafGitTools | transporte nativo de blocos e receipts, sem tocar credenciais |
| RafPolimata | gate de equivalência, estado epistêmico e auditoria federada |

## Backends

1. `SCALAR`: sempre disponível.
2. `NEON`: selecionado em compilação ARM quando suportado.
3. `CRC32C_HW`: selecionado somente com `__ARM_FEATURE_CRC32`.
4. `VULKAN` / `OPENCL`: só entram na capacidade quando o adaptador fornece `available`, `dispatch` e, quando necessário, `wait`.

O núcleo não afirma que GPU existe por encontrar uma biblioteca ou um nó do sistema. Probe, dispatch, sincronização e CRC de saída devem gerar receipt.

## Fronteira de claim

`claim_allowed` começa em zero. O host self-test comprova apenas:

- vetor CRC32C `123456789 = E3069283`;
- partição completa em 16 lanes;
- rotação dos três bancos;
- equivalência determinística do resultado;
- ring de receipts;
- ausência de promoção automática de claim.

Continuam `TOKEN_VAZIO` até execução correspondente:

- equivalência scalar ↔ NEON em ARMv7 e ARM64;
- uso real da instrução CRC32C;
- Vulkan/OpenCL físico;
- throughput, energia, p50/p95/p99 e pressão térmica;
- persistência real em disco Android;
- integração JNI/UI completa.

## Gate local

```sh
make -C runtime/b7 clean test
```

Resultado de referência produzido na construção inicial host:

```text
PASS bytes=8192 crc32c=caf1abc5 caps=00000009 receipts=4
```

Esse valor é vetor de regressão do fallback host; não é benchmark de Android.

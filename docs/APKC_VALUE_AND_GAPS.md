# ApkC — valor e gaps

> **Entrada canônica:** `docs/AGENTES.md` §5 (pipeline operacional VOID → VALIDATED)
> e §3 (estados canônicos — PENDING, TOKEN_VAZIO). Este documento avalia valor conservador
> e lacunas técnicas do ApkC para priorização técnica/acadêmica.

> Estas faixas são estimativas heurísticas para priorização técnica/acadêmica. Não são avaliação financeira formal, recomendação de investimento ou garantia de mercado.

## Valor conservador

| Estado | Faixa heurística |
|---|---:|
| Conservador atual | US$ 25k–75k |
| Com APK validado | US$ 100k–300k+ |
| Com CI verde + artifacts | US$ 300k–750k |
| Com adoção externa | US$ 1M+ |

## Valor potencial

- Micro-toolchain Android freestanding em C, com escrita direta de ZIP/APK, AXML, DEX e ELF.
- Superfície pequena para auditoria técnica e acadêmica.
- Caminho didático para estudar formatos Android sem depender de toolchains completos.

## Gaps técnicos

| Gap | Evidência necessária | Status |
|---|---|---|
| Assinatura APK | `apksigner verify --verbose` | PASS (v1/v2/v3 true; debug key; ver `Apkc/proofs/out/apksigner-verify.txt`) |
| Instalação real | `adb install -r` em device autorizado | PASS_LIMITED (package:com.rafael.teste visível; stdout completo de `adb install -r` ausente) |
| Runtime NativeActivity | `logcat` filtrado sem crash | TOKEN_VAZIO |
| Compatibilidade Android ampla | matriz API/ABI | NOT_RUN |
| Testes regressivos de formatos | corpus de APKs e validadores | NOT_RUN |

## Gaps acadêmicos

- Especificar invariantes formais por formato: ZIP, AXML, DEX, ELF.
- Comparar contra toolchains Android oficiais em reprodutibilidade e tamanho.
- Documentar limites, ameaças à validade e casos negativos.

## Gates que aumentam valuation

- CI com artifacts de `Apkc/proofs/out/`.
- APK assinado e verificado em ambiente reproduzível.
- Instalação e execução comprovadas em device físico/emulador.
- Relatório acadêmico com métodos, resultados e falsificabilidade.

## Valuation amarrada a gates verificáveis

> As faixas continuam **heurísticas** — não são avaliação financeira formal,
> recomendação de investimento ou garantia de mercado (mesmo disclaimer do topo).
> O objetivo desta seção é trocar narrativa por métrica: cada faixa só se
> "destrava" quando os gates abaixo estão **PASS** com artefato verificável.
> Fecha a lacuna L20 de `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md`.
> Regra de honestidade: nenhum gate é marcado PASS sem artefato verificado;
> caso contrário fica PENDING / TOKEN_VAZIO (invariante de
> `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md:506`). Gates citados pelo nome real do
> step em `.github/workflows/ci.yml` e pelos scripts em `scripts/`.

| Faixa heurística | Gates/artefatos que devem estar PASS | Estado hoje |
|---|---|---|
| **US$ 25k–75k** (atual) | Arquitetura + auditoria freestanding (`scripts/ci_freestanding_audit.sh`, step "Audit ApkC freestanding invariants", `.github/workflows/ci.yml:45-46`) + ARM32 encoder golden (`tests/test_arm64_encoders.py`, step "ARM64 encoder golden tests", `.github/workflows/ci.yml:48-49`) + prova ARM32 ELF (`Apkc/proofs/out/readelf-arm32.txt:11` `Status: PASS`) | **Sustentada** — esses três têm artefato; ARM32 ELF `PASS` em `readelf-arm32.txt:11` |
| **US$ 100k–300k** | Tudo acima **+** runtime ARM64 logcat sem crash (L2) **+** ARM64 ELF no APK (L4) **+** transcript source→binary (L1) | **PENDING** — L1/L2/L4 são `TOKEN_VAZIO` (`docs/LACUNAS...:52,80-81,114`); `Apkc/proofs/out/readelf-arm64.txt:1` = `TOKEN_VAZIO` |
| **US$ 300k–750k** | Tudo acima **+** CI artifact verde (run completo com upload, step "Upload ApkC proof artifacts", `.github/workflows/ci.yml:103-109`) **+** 3+ linguagens **provadas em runtime** (não só framework) via `scripts/apkc_lang_coverage.sh` (step `.github/workflows/ci.yml:57-58`) | **PENDING** — coverage roda 6/12 mas runtime por linguagem é `TOKEN_VAZIO` (`docs/LACUNAS...` L9); ARM64 ausente bloqueia a célula |
| **US$ 1M+** | Tudo acima **+** adoção externa **+** corpus regressivo de formatos (`tests/fixtures/`, L17) **+** release navegável (L19) | **PENDING/TOKEN_VAZIO** — corpus regressivo aberto (`docs/LACUNAS...` L17), release navegável inexistente (`docs/LACUNAS...` L19) |

### Gates auxiliares já verdes (suportam a faixa atual)

Estes não destravam faixa sozinhos, mas são pré-condições que já têm artefato:

- **P(k) falsifiability gate** — bloqueante, veredicto `PASS`
  (`scripts/first_test_pk.py`, step `.github/workflows/ci.yml:111-123`;
  `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md:445` registra `rrmse=0.119,
  coverage=1.0`).
- **verbovivo build + smoke** — `PASS` (step
  `.github/workflows/ci.yml:63-72`; reconferido em
  `docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L15).
- **API/ABI matrix** — minSdkVersion provado nos bytes do AXML
  (`scripts/apkc_api_abi_matrix.sh`, step `.github/workflows/ci.yml:60-61`);
  células sem toolchain ARM ficam `TOKEN_VAZIO` honestamente
  (`docs/LACUNAS_PROFUNDAS_MVP_PRODUTO.md` L17, sub-gap fechado).

### Gate de assinatura (limita as faixas superiores)

Assinatura hoje é **debug** (v1/v2/v3 `true`, v3.1/v4/SourceStamp `false` —
`Apkc/proofs/out/apksigner-verify.txt:9-14`). Release-signing é `PENDING`
(ver `docs/APKC_SIGNING_POLICY.md`). Sem release-signing verificado, as faixas
de US$ 300k+ não fecham o critério de "distribuição a terceiros".

> Cada avanço técnico (sair de TOKEN_VAZIO num gate nomeado acima) move a tese
> de mercado de forma mensurável, não narrativa. A faixa só sobe quando o
> artefato existe e é auditável.

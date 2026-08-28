# Canari / Maltego — Integração Controlada no RafPolimata

**Estado estrutural:** `IMPLEMENTED_GATED`  
**Estado de runtime:** `TOKEN_VAZIO`  
**Claim permitido:** `false`  
**Manifesto:** `configs/canari-integration.v1.json`  
**Validador:** `scripts/validate_canari_integration.py`

## 1. Objetivo

Adicionar o Canari ao mapa operacional do RafPolimata como framework externo para desenvolvimento e execução de transforms do Maltego, sem misturar automaticamente sua cadeia de dependências com o núcleo do repositório.

A integração nasce em modo **external-isolated**:

```text
RafPolimata
  -> contrato/manifesto Canari
  -> ambiente isolado e pinado
  -> Canari
  -> transform Maltego
  -> receipt de execução
  -> promoção somente por evidência
```

O estado `PASS` produzido pelo validador significa apenas que essa fronteira está coerente. Não significa que Canari, Maltego, Plume, um transform remoto ou um dispositivo físico tenham sido executados.

## 2. Upstream observado

Fonte primária selecionada para avaliação:

- repositório: `malleum-inc/canari3`;
- finalidade declarada: framework Canari v3 para transforms Maltego locais e remotos;
- licença declarada pelo repositório: `GPL-3.0`;
- branch padrão observada: `master`;
- último push observado no metadata consultado em 2026-08-28: `2021-04-21T16:04:45Z`;
- `setup.py` declara classificadores Python `2.7`, `3.6` e `3.7`;
- o pacote PyPI `canari` possui distribuição legada `3.3.10`, publicada em 2019, ainda classificada como Python 2.7.

Referências externas:

- <https://github.com/malleum-inc/canari3>
- <https://pypi.org/project/canari/>
- <https://www.canariproject.com/en/latest/>

Esses dados são observações de upstream, não certificação de compatibilidade atual com Python moderno, Android/Termux ou versões atuais do Maltego.

## 3. Por que não foi vendorado

O RafPolimata mantém `docs/LICENSE_DECISION_RECORD.md` em `TOKEN_VAZIO_OWNER_DECISION`. O upstream Canari selecionado declara GPL-3.0. Enquanto a política de licença do RafPolimata e a compatibilidade com terceiros não estiverem fechadas, copiar o código do Canari para este repositório criaria uma decisão jurídica que o agente não está autorizado a tomar.

Portanto:

```text
referenciar upstream = permitido nesta etapa
copiar/vendorizar código = bloqueado
importar Canari no núcleo = bloqueado
promover claim de runtime = bloqueado
```

O validador aplica essas invariantes e falha se detectar diretórios candidatos de vendoring ou imports diretos de `canari` no código ativo do núcleo.

## 4. Contrato de evidência

O manifesto separa explicitamente:

| Campo | Estado inicial | Significado |
|---|---|---|
| `upstream_metadata_observed` | `true` | metadata externo foi identificado |
| `source_pin` | `TOKEN_VAZIO` | commit imutável ainda não promovido |
| `compatibility_review` | `TOKEN_VAZIO` | revisão jurídica/técnica pendente |
| `runtime_verified` | `TOKEN_VAZIO` | execução Canari não comprovada |
| `maltego_profile_verified` | `TOKEN_VAZIO` | perfil `.mtz` não comprovado |
| `termux_verified` | `TOKEN_VAZIO` | execução em Termux não comprovada |
| `claim_allowed` | `false` | nenhuma afirmação operacional é promovida |

`TOKEN_VAZIO` não equivale a falha nem a sucesso: registra ausência de evidência suficiente.

## 5. Rota de ativação futura

A ativação deve ocorrer fora do núcleo, com cadeia reproduzível:

1. escolher e registrar um **commit SHA imutável** do upstream;
2. concluir inventário e compatibilidade de licença de terceiro;
3. criar ambiente separado, sem alterar dependências globais do RafPolimata;
4. instalar o upstream pinado e registrar versão de Python/dependências;
5. executar uma prova mínima (`canari --help` ou equivalente) e guardar stdout/stderr/exit code;
6. criar um transform local inofensivo e um perfil mínimo Maltego;
7. verificar o perfil em ambiente autorizado;
8. produzir receipt com hashes, ambiente e resultados;
9. só então revisar `claim_allowed`.

Nenhum transform de rede é ativado por padrão. Credenciais, tokens e chaves não devem entrar no repositório.

## 6. Validação atual

Execução estrutural local/CI:

```sh
python3 -m unittest tests.test_canari_integration
python3 scripts/validate_canari_integration.py \
  --output results/canari/integration-receipt.json
```

O recibo usa o schema:

```text
raf.canari-integration-receipt.v1
```

Ele contém checks, estado de runtime, `claim_allowed`, falhas e a interpretação explícita de que um `PASS` estrutural não é prova de runtime.

## 7. Invariantes de não-regressão

```text
VISÃO != ARTEFATO != EXECUÇÃO != EVIDÊNCIA != CLAIM
TOKEN_VAZIO != PASS
metadata upstream != runtime local
referência externa != vendoring
perfil gerado != transform validado
CI estrutural != dispositivo físico
```

A integração Canari somente evolui por evidência adicional e decisões autorizadas; não por inferência.

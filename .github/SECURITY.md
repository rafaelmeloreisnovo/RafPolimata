# Security Policy — RafPolimata

**Estado:** `ACTIVE_POLICY / PRIVATE_CHANNEL_TOKEN_VAZIO`  
**Responsável lógico:** `security-license`  
**Contrato relacionado:** `configs/operational-gap-topology.v1.json`

## Escopo

Esta política cobre vulnerabilidades relacionadas ao código, scripts, workflows, formatos, artefatos e mecanismos de proveniência mantidos neste repositório.

Ela **não** declara que o RafPolimata é um produto certificado, hardened para produção ou livre de vulnerabilidades. Um gate verde demonstra apenas o escopo daquele gate.

## Como relatar uma vulnerabilidade

1. **Não publique** segredo, chave, credencial, exploit funcional, dado pessoal ou detalhe que permita exploração imediata em issue, discussion, PR ou comentário público.
2. Se a interface do GitHub deste repositório oferecer **Report a vulnerability / Relatar uma vulnerabilidade**, use esse canal privado.
3. A disponibilidade administrativa de Private Vulnerability Reporting não é legível pela integração usada para esta auditoria; portanto seu estado permanece `TOKEN_VAZIO` até verificação explícita.
4. Se o canal privado não estiver disponível, abra apenas um issue público de **security contact request**, sem detalhes técnicos sensíveis, solicitando ao mantenedor um canal privado antes de compartilhar o conteúdo da vulnerabilidade.

## Conteúdo mínimo do relato privado

Quando aplicável, informar:

- commit/tag afetado;
- arquivo/componente;
- ambiente/arquitetura;
- pré-condições;
- passos mínimos para reprodução;
- impacto esperado e impacto observado separados;
- logs sem segredos;
- hashes de artefatos quando relevantes;
- mitigação temporária, se conhecida.

Nunca inclua material secreto somente para “provar” que existe. O receipt deve registrar o detector, estado e identidade necessária, não reproduzir o segredo.

## Estados de triagem

```text
RECEIVED
-> TRIAGE
-> REPRODUCED | NOT_REPRODUCED | TOKEN_VAZIO
-> FIX_PENDING | MITIGATION_PENDING
-> FIXED
-> DISCLOSURE_READY
```

`TOKEN_VAZIO` significa evidência insuficiente, ambiente indisponível ou condição não observada. Não significa que a vulnerabilidade existe nem que não existe.

## Severidade e prioridade

A prioridade considera conjuntamente:

- possibilidade de execução arbitrária;
- exposição de segredo ou material criptográfico;
- corrupção de cadeia de custódia/proveniência;
- bypass de gate fail-closed;
- adulteração silenciosa de APK/DEX/ELF ou artefato;
- impacto sobre dados, disponibilidade ou integridade;
- pré-condições e alcance real do atacante.

Nenhuma classificação interna equivale automaticamente a CVSS, certificação ou parecer externo.

## Versões suportadas

Não existe, neste corte, um SLA comercial ou uma matriz de versões com suporte contratual aprovada pelo proprietário.

- `main`: recebe triagem técnica corrente, sem garantia de prazo.
- releases/prereleases históricos: avaliados por caso e por identidade do artefato.
- política comercial de manutenção/SLA: `TOKEN_VAZIO_OWNER_DECISION` em `GAP-COM-SUPPORT-LIFECYCLE`.

## Divulgação coordenada

A publicação de detalhes deve ocorrer somente depois de:

1. escopo confirmado;
2. risco de exposição de terceiros avaliado;
3. correção ou mitigação disponível quando razoável;
4. evidência preservada;
5. decisão de divulgação registrada.

Não há promessa de bounty, recompensa financeira ou prazo de resposta salvo anúncio explícito e versionado posterior.

## Cadeia de evidência

Uma correção de segurança deve, quando aplicável, ligar:

```text
finding
-> commit afetado
-> teste/falsificador
-> patch
-> teste negativo/positivo
-> artefato
-> verificação
-> receipt
```

Mudança que apenas silencia scanner, pula teste ou transforma falha em sucesso não fecha vulnerabilidade.

## Referências de alinhamento

Esta política usa como referências de arquitetura, sem alegar certificação ou conformidade formal:

- NIST Cybersecurity Framework 2.0;
- NIST Secure Software Development Framework (SSDF) 1.1 como versão final vigente; SSDF 1.2 permanece draft monitorado;
- SLSA 1.2 para proveniência/supply-chain;
- ISO/IEC 27001:2022 como referência de sistema de gestão de segurança.

O mapeamento operacional está em `docs/OPERATIONAL_GAP_TOPOLOGY_V1.md`.

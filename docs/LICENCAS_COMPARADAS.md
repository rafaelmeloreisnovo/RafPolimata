# Licenças e termos: comparação estratégica

> **Nota**: este documento é informativo e não constitui aconselhamento jurídico.

## 1) Ponto de partida: referência BLAKE3
O ecossistema BLAKE3 é normalmente associado a termos permissivos de software livre (uso amplo, com exigências de preservação de aviso de copyright e termos).

### Lições práticas
- Licenças permissivas aceleram adoção técnica.
- Mesmo permissivas, exigem compliance de atribuição e manutenção de avisos.

## 2) Grandes fornecedores (Microsoft, Oracle, Google)
Em contextos corporativos, muitas ofertas combinam:
- software open source em partes,
- termos proprietários de nuvem/SaaS em outras,
- políticas específicas de uso de API, dados e responsabilidade.

### Padrão recorrente
1. **Camada OSS**: permissões de uso/modificação/distribuição sob licença específica.
2. **Camada contratual**: termos de serviço, SLA, limites de responsabilidade.
3. **Camada regulatória**: obrigações locais (privacidade, consumidor, setor regulado).

## 3) Matriz comparativa de decisão

| Critério | Permissiva (ex.: estilo MIT/Apache) | Copyleft | Proprietária/SaaS |
|---|---|---|---|
| Reuso comercial | Alto | Médio/alto com condições | Alto sob contrato |
| Obrigação de abertura derivada | Baixa | Alta (dependendo da licença) | N/A (controle do fornecedor) |
| Dependência contratual | Baixa | Baixa/média | Alta |
| Auditoria de compliance | Média | Alta | Alta |
| Risco de lock-in | Médio | Médio | Alto |

## 4) Protocolo recomendado de compliance
1. Inventário de componentes (SBOM).
2. Classificação de licenças por obrigação.
3. Verificação de compatibilidade entre licenças.
4. Revisão contratual de termos cloud/API.
5. Registro de decisões e exceções.
6. Revalidação periódica (mudança de versão/termos).

## 5) Enfoque supralegal em Estados democráticos de direito
- Licença não neutraliza direitos fundamentais.
- Cláusula contratual não pode legitimar violação de direitos indisponíveis.
- Em alto impacto, governança técnica deve ser testável por auditoria independente.

## 6) Cláusula de prudência para projetos de IA/semântica
Recomenda-se adicionar, além da licença principal:
- política de uso responsável,
- política de dados e retenção,
- canal de contestação/revisão humana,
- política de incidentes e reparação.

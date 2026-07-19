# DeepRafa2 — protocolo de evidência multidомínio

> **Entrada canônica:** docs/AGENTES.md §3 (estados canônicos e epistêmicos) e §5 (pipeline operacional VOID→VALIDATED). Protocolo multidomínio de validação de claims — gates G0-G9 e ciclo R3⟨F_ok, F_gap, F_next⟩.

**Função:** orquestrar projetos científicos, técnicos, estatísticos, econômicos e de propriedade intelectual sem transformar hipótese em prova.

**Estado:** protocolo operacional v1.0  
**Data:** 2026-07-14

---

## 1. Princípio central

\[
\boxed{
claim\ válido = origem + escopo + método + evidência + incerteza + limite
}
\]

DeepRafa2 não é um modelo de deep learning. É uma arquitetura de validação e roteamento que pode usar modelos estatísticos ou neurais quando o problema e os dados justificarem.

A cadeia canônica é:

```text
INTENT -> CLAIM -> SOURCE -> MODEL -> TEST -> EVIDENCE
       -> LIMIT -> DECISION -> REPOSITORY -> FEEDBACK
```

---

## 2. Estados epistêmicos

| Estado | Definição | Pode sustentar claim público? |
|---|---|---:|
| `VOID` | campo ainda não examinado | não |
| `TOKEN_VAZIO` | evidência ou ferramenta necessária ausente | não |
| `REFERENCE` | conteúdo sustentado por fonte identificada | com atribuição |
| `HYPOTHESIS` | proposição falsificável sem validação | apenas como hipótese |
| `DERIVED` | consequência matemática de premissas declaradas | apenas no domínio das premissas |
| `SIMULATED` | resultado reprodutível de simulação | como resultado simulado |
| `MEASURED` | medição com instrumento e incerteza | sim, com protocolo |
| `REPLICATED` | resultado repetido de forma independente | sim |
| `IP_CANDIDATE` | possível solução técnica; anterioridade pendente | não como patente |
| `PATENT_FILED` | pedido identificado por número e data | sim, como pedido |
| `PATENT_GRANTED` | concessão comprovada | sim, nos limites das reivindicações |
| `VALUED` | valor derivado por método documentado | sim, com premissas |

Promóções exigem evidência. O sistema não aceita promoção por repetição textual.

---

## 3. Unidade mínima de conhecimento

Cada bloco deve ser serializável no seguinte contrato:

```yaml
claim_id: string
project_id: string
domain: string
statement: string
state: VOID|TOKEN_VAZIO|REFERENCE|HYPOTHESIS|DERIVED|SIMULATED|MEASURED|REPLICATED|IP_CANDIDATE|PATENT_FILED|PATENT_GRANTED|VALUED
source:
  type: paper|standard|law|dataset|experiment|repository|conversation
  locator: string
  access_date: YYYY-MM-DD
premises: []
variables:
  - name: string
    unit: string
method: string
evidence:
  artifacts: []
  hashes: []
  run_ids: []
uncertainty:
  method: string
  value: null
limitations: []
falsifier: string
next_action: string
repository_target: string
```

Campos ausentes recebem `TOKEN_VAZIO`; não recebem preenchimento inventado.

---

## 4. Gates de excelência operacional

### G0 — identidade e domínio

- o objeto está nomeado sem ambiguidade;
- homônimos técnicos estão separados;
- o repositório de destino pertence ao domínio.

Exemplo: propulsor Hall de plasma não é motor BLDC com sensor Hall.

### G1 — fonte e atribuição

- fontes primárias ou institucionais quando disponíveis;
- data e localizador persistente;
- nenhuma afirmação importada sem atribuição.

### G2 — consistência matemática

- símbolos definidos;
- hipóteses e condições de contorno declaradas;
- unidades conservadas;
- derivadas não promovidas além das premissas.

### G3 — desenho experimental

- variável-alvo e controles definidos;
- instrumentos e calibração registrados;
- critérios de exclusão predefinidos;
- riscos e intertravamentos documentados.

### G4 — estatística

- baseline simples;
- amostragem e poder estatístico justificados;
- treino, validação e teste separados;
- intervalos de confiança ou incerteza;
- múltiplas comparações controladas quando aplicável.

### G5 — aprendizado de máquina

- necessidade do modelo demonstrada;
- leakage testado;
- calibração, drift e OOD avaliados;
- fallback determinístico;
- proveniência de dados e modelo.

### G6 — reproducibilidade

- código, commit, ambiente e seed;
- dados brutos ou regra explícita de acesso;
- hashes de artefatos;
- execução independente possível.

### G7 — propriedade intelectual

- problema técnico e efeito técnico identificados;
- estado da técnica pesquisado;
- fórmula/algoritmo em si não tratado automaticamente como patente;
- número de depósito obrigatório para `PATENT_FILED`.

### G8 — economia

- separar custo, preço, valor presente e valor de opção;
- probabilidades e taxa de desconto declaradas;
- cenário não confundido com valuation.

### G9 — publicação

- resumo compatível com resultados;
- limitações visíveis;
- claims reproduzíveis;
- artefatos ligados ao manuscrito.

---

## 5. Mapeamento NIST AI RMF

Para componentes de IA, DeepRafa2 mapeia suas funções ao NIST AI RMF:

| NIST | DeepRafa2 | Saída mínima |
|---|---|---|
| `GOVERN` | responsabilidades, estados e política de claims | owner + regras + auditoria |
| `MAP` | contexto, atores, danos e limites | mapa de risco |
| `MEASURE` | métricas, testes e incerteza | relatório reproduzível |
| `MANAGE` | decisão, mitigação, rollback e monitoramento | ação rastereável |

A explicabilidade não substitui acurácia, segurança ou justiça; é uma dimensão adicional de controle.

---

## 6. Invariante relacional entre projetos

Projetos distintos podem compartilhar o mesmo ciclo sem compartilhar as mesmas provas:

\[
\mathcal C =
input\rightarrow sensing\rightarrow state\rightarrow model
\rightarrow decision\rightarrow action\rightarrow feedback
\]

| Projeto | Estado físico | Dado | Modelo | Prova principal |
|---|---|---|---|---|
| propulsão Hall | plasma/campos | telemetria de bancada | física + residual | ensaio em vácuo |
| logística reversa | estoque/fluxo | eventos e massa | otimização | balanço material/econômico |
| técnicos em campo | posição/fila | GPS/ordens | roteamento | tempo/custo/qualidade |
| coleta de chuva | reservatório/clima | precipitação/vazão | balanço hídrico | volume e qualidade |
| compilador/runtime | estado computacional | logs/artefatos | semântica/execução | build, teste e hash |

O ciclo é invariante; a evidência não é intercambiável.

---

## 7. Roteamento por repositório

### Regras

1. teoria física e protocolo experimental -> repositório de Física;
2. método transversal e governança -> RafPolimata;
3. catálogo de projetos e estado de evidência -> RafaelCiencias;
4. código de runtime -> repositório que compila e executa esse runtime;
5. papers só recebem material após estabilização metodológica;
6. dados sensíveis e rascunhos de IP permanecem privados;
7. cosmologia não recebe engenharia de motores sem relação demonstrada.

### Anti-padrões

- copiar o mesmo texto para muitos repositórios;
- declarar "concluído" sem artefato;
- usar IA como justificativa genérica;
- misturar valor de mercado com custo de desenvolvimento;
- publicar reivindicação de patente antes de análise de anterioridade;
- transformar metáfora em variável física.

---

## 8. Função de progresso por evidência

\[
G=
0.10D+0.15M+0.15S+0.20P+0.20T+0.10IP+0.10A
\]

onde:

- \(D\): documentação;
- \(M\): modelo matemático;
- \(S\): simulação;
- \(P\): protótipo;
- \(T\): teste;
- \(IP\): anterioridade/proteção;
- \(A\): adoção.

Cada componente varia de 0 a 1 e exige artefato identificável. Essa função é uma convenção de governança, não uma lei científica.

---

## 9. Valuation defensável

Valor de projeto:

\[
V=\sum_{t=0}^{N}\frac{p_tR_t-C_t}{(1+r)^t}-C_{risk}
\]

Valor de IP, como cenário:

\[
V_{IP}=P(grant)P(adoption)PV(royalties)-C_{legal}-C_{prior\ art}
\]

Sem dados de custo, probabilidade, adoção e fluxo de caixa, o estado é `TOKEN_VAZIO`.

---

## 10. Caso de referência: sessão Hall

Diagnóstico:

- arquitetura DeepRafa2: defensável como método organizacional;
- invariantes físicos básicos: defensáveis com referência;
- percentuais de eficiência e vida útil: não demonstrados;
- "Patent A–E": identificadores exemplificativos;
- valor financeiro consolidado: cenário narrativo, não valuation;
- aplicação inicial: bancada em vácuo, não mobilidade terrestre;
- IA: futura camada residual/preditiva, condicionada a dataset.

Resultado do gate:

```text
G0 PASS
G1 PARTIAL
G2 PARTIAL
G3 SPECIFIED
G4 TOKEN_VAZIO
G5 TOKEN_VAZIO
G6 TOKEN_VAZIO
G7 IP_CANDIDATE
G8 TOKEN_VAZIO
G9 NOT_READY
```

---

## 11. Ciclo operacional

### Entrada

- selecionar um claim;
- localizar o repositório dono;
- congelar estado inicial.

### Execução

- aplicar gates G0–G9;
- produzir o menor artefato verificável;
- registrar hashes e lacunas.

### Saída

\[
R_3=\langle F_{ok},F_{gap},F_{next}\rangle
\]

- `F_ok`: o que passou com evidência;
- `F_gap`: o que permanece ausente;
- `F_next`: uma próxima ação executável.

Não existe processamento implícito ou em segundo plano. Cada ciclo deve terminar com estado persistido.

---

## 12. Referências

1. NIST. *Artificial Intelligence Risk Management Framework (AI RMF 1.0)*. NIST AI 100-1, 2023. https://doi.org/10.6028/NIST.AI.100-1
2. Goebel, D. M.; Katz, I. *Fundamentals of Electric Propulsion: Ion and Hall Thrusters*. JPL, 2008. https://descanso.jpl.nasa.gov/SciTechBook/series1/Goebel__cmprsd_opt.pdf
3. Brasil. Lei nº 9.279/1996 — Lei da Propriedade Industrial. https://www.planalto.gov.br/ccivil_03/leis/l9279.htm

---

## 13. Invariante final

\[
\boxed{
excelência\ operacional =
clareza + prova + limite + rastreabilidade + correção
}
\]

O sistema considera o erro medido um insumo de engenharia e a lacuna declarada uma forma de integridade.

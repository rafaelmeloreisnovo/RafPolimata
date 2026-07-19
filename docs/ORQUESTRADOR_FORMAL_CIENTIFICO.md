# Orquestrador Formal Científico RAFAELIA — Física, Matemática, Estatística e Evidência

> **Entrada canônica:** docs/AGENTES.md §5 (pipeline operacional — TOKEN_VAZIO como estado válido para ausência de evidência, disciplina de claim) e §6 (escalação e conflito — quando escalar para humano, como registrar via docs/AGENTES_DECISAO_LOG.md). Este documento é o orquestrador de formalidade científica para física, matemática, estatística e estados epistêmicos.

**Status:** `METHOD_DEFINED` — arquitetura formal e protocolo de validação; não constitui, por si só, confirmação experimental.  
**Escopo:** RafPolimata como núcleo de formalidade; RLL como adaptador físico/observacional; ChipQuantum como runtime/benchmark.  
**Regra central:** nenhuma metáfora, semelhança semântica ou coincidência numérica é promovida a alegação física sem domínio, unidade, incerteza, dado, execução e falsificador.

---

## 1. Resultado da auditoria do material anterior

O conjunto histórico contém quatro classes distintas que precisam permanecer separadas:

1. **Equações estabelecidas:** conservação de energia, dinâmica longitudinal, modelos eletromagnéticos `dq`, lei de Faraday, Navier–Stokes, estatística inferencial e sincronização NTP.
2. **Modelos de engenharia:** comutação estrela–triângulo, controle preditivo, mistura hidrogênio–combustível, gestão térmica, mapas de eficiência e recuperação de energia.
3. **Hipóteses testáveis:** ganho líquido de combustível, redução de emissões, faixa ótima de temperatura, melhoria de retomada e controle por IA.
4. **Afirmações sem evidência suficiente:** raridade extrema, valores monetários específicos, patentes “em registro”, percentuais fixos de economia e equivalência entre formalismo semântico e física quântica.

Os itens da classe 4 passam ao estado `TOKEN_VAZIO` ou `DECLARED_BY_AUTHOR`, até que existam documentos, resultados, pedidos de patente, laudos, datasets ou contratos verificáveis.

### 1.1 Correções técnicas indispensáveis

- Hidrogênio é **combustível**, não comburente. O comburente usual é o oxigênio do ar ou oxigênio suplementar controlado.
- Produzir H₂ a bordo a partir da energia do próprio motor não cria energia. O balanço só pode ser positivo quando a melhora de combustão e a energia recuperada superam as perdas do alternador, eletrólise, compressão, controle e conversão.
- A expressão `T = V I / R` não é equação de torque: dimensionalmente produz corrente ao quadrado, não N·m.
- Em sistema trifásico equilibrado, estrela e triângulo não significam universalmente “mais torque em baixa” e “mais torque em alta”. O resultado depende de tensão de linha, limites de corrente, constante de FEM, impedância, frequência elétrica e geometria da máquina.
- Para a mesma tensão de linha, `V_fase,Y = V_linha/√3` e `V_fase,Δ = V_linha`; por isso a conexão Y reduz a tensão de fase. Em máquinas de indução, o torque de partida em Y é aproximadamente um terço do torque em Δ, sob hipóteses usuais. Em PMSM/BLDC reconfigurável, o ganho deve ser obtido do mapa eletromagnético e dos limites do inversor.
- A temperatura do combustível não possui ganho monotônico. Ela altera densidade, viscosidade, volatilidade, cavitação, atomização, atraso de ignição e risco de detonação; a faixa ótima é específica do combustível e do sistema de injeção.
- Um “motor Hall” precisa ser desambiguado: **propulsor Hall espacial**, **sensor Hall de comutação** ou **motor BLDC com sensores Hall** são objetos físicos diferentes.
- Formalismo quântico só é alegação de mecânica quântica quando há espaço de Hilbert, Hamiltoniano, observáveis, regra de Born e adaptador experimental. Caso contrário, é analogia matemática ou simulação clássica.

---

## 2. Arquitetura do orquestrador

O orquestrador opera como grafo acíclico de evidência:

```text
SEMANTIC_FRAGMENT
      |
      v
FORMAL_DEFINITION --units--> DIMENSION_GATE
      |                           |
      v                           v
MODEL_EQUATION -----------> NUMERICAL_GATE
      |                           |
      v                           v
EXPERIMENT_DESIGN -------> DATASET + UNCERTAINTY
      |                           |
      v                           v
INFERENCE -------------> CLAIM_STATE
      |                           |
      v                           v
ARTIFACT/HASH ---------> REPRODUCIBILITY_GATE
```

### 2.1 Estados epistêmicos

| Estado | Significado operacional |
|---|---|
| `METAPHOR` | linguagem explicativa sem poder probatório |
| `FORMALIZED` | símbolos, domínios, unidades e hipóteses definidos |
| `METHOD_DEFINED` | método de cálculo/experimento e falsificador definidos |
| `SIMULATED` | execução numérica reprodutível, ainda sem validação física |
| `EVIDENCE_LINKED` | dado real, incerteza e cadeia de custódia ligados ao claim |
| `REPLICATED` | resultado reproduzido por execução independente |
| `CLAIM_ALLOWED` | alegação limitada ao domínio demonstrado |
| `TOKEN_VAZIO` | evidência, ferramenta ou dado ainda ausente |
| `CONTRADICTION` | evidência contradiz a hipótese |

### 2.2 Invariante de promoção

Uma alegação `c` só pode avançar quando:

```math
G(c)=G_{dim}\,G_{num}\,G_{data}\,G_{unc}\,G_{prov}\,G_{fals}\,G_{rep}=1,
```

com cada gate binário. Se qualquer fator for zero, o estado não pode ser `CLAIM_ALLOWED`.

---

## 3. Núcleo físico clássico e eletromecânico

### 3.1 Dinâmica longitudinal do veículo

```math
m\dot v = F_{trac}-\frac12\rho_a C_d A_f v^2-C_{rr}mg\cos\theta-mg\sin\theta-F_{aux}.
```

```math
P_{rodas}=F_{trac}v,\qquad P_{eixo}=\tau_m\omega_m,\qquad \omega_m=G\frac{v}{r_w}.
```

Essa equação separa arranque, manutenção de velocidade, subida e retomada. Em velocidade constante e pista plana, `m·dv/dt=0`; logo a potência é dominada pelo arrasto aerodinâmico, que cresce aproximadamente com `v³`.

### 3.2 Modelo PMSM/BLDC em eixos `dq`

Para uma máquina síncrona de ímãs permanentes:

```math
v_d=R_s i_d+L_d\dot i_d-\omega_e L_q i_q,
```

```math
v_q=R_s i_q+L_q\dot i_q+\omega_e(L_d i_d+\psi_f),
```

```math
\tau_e=\frac32p\left[\psi_f i_q+(L_d-L_q)i_d i_q\right].
```

Para rotor superficial, `L_d≈L_q`, então:

```math
\tau_e\approx\frac32p\psi_f i_q.
```

A potência eletromecânica é:

```math
P_{em}=\tau_e\omega_m,
```

não `VI/R`. A potência elétrica trifásica instantânea pode ser modelada por:

```math
P_{el}=\frac32(v_d i_d+v_q i_q).
```

### 3.3 Relações estrela–triângulo

Para sistema equilibrado:

```math
V_{fase,Y}=\frac{V_L}{\sqrt3},\quad I_{L,Y}=I_{fase,Y},
```

```math
V_{fase,\Delta}=V_L,\quad I_{L,\Delta}=\sqrt3 I_{fase,\Delta}.
```

O chaveamento deve ocorrer apenas quando forem satisfeitas simultaneamente:

```math
V_{req}(\omega,\tau)\le V_{inv,max},
```

```math
I_{fase}(\omega,\tau)\le I_{fase,max},
```

```math
T_j\le T_{j,max},\qquad T_{w}\le T_{w,max},
```

```math
\Delta\tau_{switch}\le\tau_{ripple,max}.
```

**Resultado científico correto:** não existe percentual universal de ganho. O ganho é a diferença entre integrais de energia no mesmo ciclo:

```math
G_E=1-\frac{\int_0^T P_{in,orq}(t)\,dt}{\int_0^T P_{in,base}(t)\,dt}.
```

### 3.4 Controlador IA com barreira de segurança

A IA não deve comandar diretamente chaves de potência sem um supervisor determinístico. O custo de controle pode ser:

```math
J=\sum_{k=0}^{N-1}\left(
 w_E E_k+w_\tau e_{\tau,k}^2+w_v e_{v,k}^2+w_T\Phi_T(k)+w_s N_{switch,k}+w_{em}E_{em,k}
\right).
```

Sujeito a limites elétricos, térmicos, mecânicos, emissivos e de estabilidade. A ação final é:

```math
u_k=\Pi_{\mathcal S}\left(u_k^{AI}\right),
```

onde `Π_S` projeta a decisão da IA no conjunto seguro `S`.

---

## 4. Combustão, eletrólise e balanço energético

### 4.1 Lei de Faraday para produção de hidrogênio

```math
n_{H_2}=\eta_F\frac{I t}{2F},
```

```math
m_{H_2}=M_{H_2}n_{H_2},
```

onde `F=96485.33212 C·mol⁻¹`, `η_F` é a eficiência faradáica e `M_H2` é a massa molar.

A vazão energética química pelo poder calorífico inferior é:

```math
P_{H_2,LHV}=\dot m_{H_2}\,LHV_{H_2}.
```

### 4.2 Eficiência do eletrólisador

```math
\eta_{el}=\frac{\dot m_{H_2}LHV_{H_2}}{P_{stack}+P_{BOP}},
```

onde `P_BOP` inclui bomba, refrigeração, controle, secagem e compressão.

### 4.3 Condição de ganho líquido

O sistema só é energeticamente vantajoso quando:

```math
\Delta P_{net}=\Delta P_{ICE}+P_{regen}+P_{waste\ heat}
-P_{electrolyzer}-P_{compression}-P_{control}-P_{conversion}>0.
```

Se a eletrólise for alimentada pelo alternador movido pelo próprio motor, a cadeia possui eficiência:

```math
\eta_{loop}=\eta_{ICE}\eta_{alt}\eta_{el}\eta_{H_2\to ICE}<1.
```

Logo, não há criação líquida de energia. Pode haver benefício local de combustão ou emissões em condições específicas, mas precisa superar a carga parasita e ser medido por balanço completo tanque–roda.

### 4.4 Mistura, ar e equivalência

```math
\lambda=\frac{(A/F)_{real}}{(A/F)_{esteq}},\qquad \phi=\lambda^{-1}.
```

A adição de H₂ deve ser expressa por fração energética, não apenas volumétrica:

```math
x_{H_2,E}=\frac{\dot m_{H_2}LHV_{H_2}}
{\dot m_f LHV_f+\dot m_{H_2}LHV_{H_2}}.
```

Os resultados mínimos são: eficiência térmica indicada e ao freio, BSFC, IMEP/BMEP, pressão máxima, taxa de elevação de pressão, NOx, CO, HC, CO₂ e particulados.

---

## 5. Temperatura do combustível

### 5.1 Densidade e vazão mássica

Em intervalo limitado:

```math
\rho_f(T)\approx\frac{\rho_{ref}}{1+\beta(T-T_{ref})},
```

```math
\dot m_f=\rho_f(T)\,\dot V_f.
```

Se a ECU controla massa via pressão, tempo de injeção e compensação térmica, parte do efeito volumétrico é corrigida. A análise deve usar massa real, não apenas litros.

### 5.2 Atomização e evaporação

O diâmetro médio de Sauter `D_32` depende de viscosidade, tensão superficial, densidade, pressão de injeção e geometria. O estudo deve medir ou modelar:

```math
D_{32}=f(\mu_f(T),\sigma_f(T),\rho_f(T),\Delta p,d_n,Re,We).
```

### 5.3 Critério de otimização térmica

```math
T_f^*=\arg\min_T\left[
BSFC(T)+\alpha_{NOx}NOx(T)+\alpha_{HC}HC(T)+\alpha_{knock}K(T)+\alpha_{vap}V(T)
\right].
```

A faixa ótima é um resultado experimental. “Combustível mais quente é sempre melhor” e “combustível mais frio é sempre melhor” são ambos falsos como regras gerais.

---

## 6. Propulsor Hall espacial

O propulsor Hall não é motor automotivo. Seu modelo mínimo é:

```math
F\approx\dot m v_e,
```

```math
P_{jet}=\frac12\dot m v_e^2,
```

```math
\eta_T=\frac{P_{jet}}{P_{in}}=\frac{F^2}{2\dot m P_{in}},
```

```math
I_{sp}=\frac{F}{\dot m g_0}.
```

A validação exige descarga de plasma, potencial, campo magnético, corrente anódica, vazão, divergência do feixe, erosão e balanço térmico. Fórmulas automotivas não devem ser reutilizadas nesse domínio.

---

## 7. Mecânica quântica e fronteira de alegação

Uma camada é quântica somente se os objetos forem definidos por:

```math
i\hbar\frac{d}{dt}|\psi(t)\rangle=\hat H|\psi(t)\rangle,
```

```math
\langle A\rangle=\langle\psi|\hat A|\psi\rangle,
```

```math
P(a)=|\langle a|\psi\rangle|^2.
```

Requisitos:

- `H` auto-adjunto e domínio declarado;
- observáveis e espectro definidos;
- condições iniciais e de contorno;
- unidade e escala de cada parâmetro;
- adaptador experimental;
- comparação com modelo clássico e hipótese nula.

Um toro `T⁷`, um grafo ou um hash pode parametrizar uma simulação clássica de trajetórias ou estados, mas isso não demonstra hardware quântico, coerência quântica ou vantagem quântica.

---

## 8. Sistemas dinâmicos, Poincaré e estabilidade

Para fluxo `\dot x=f(x,u,p)`, define-se uma seção `Σ` e o mapa de retorno:

```math
P:\Sigma\to\Sigma,\qquad x_{n+1}=P(x_n).
```

Um ponto periódico satisfaz:

```math
P^k(x^*)=x^*.
```

A estabilidade local é determinada pelos autovalores de `DP^k(x*)`; o maior expoente de Lyapunov é:

```math
\lambda_{max}=\lim_{n\to\infty}\frac1{n\Delta t}
\log\frac{\|\delta x_n\|}{\|\delta x_0\|}.
```

Os “42 atratores” são um limite arquitetural até que uma análise de bifurcação prove atratores físicos do sistema modelado.

---

## 9. Fluidos, térmica e números adimensionais

Para fluido Newtoniano incompatível:

```math
\rho\left(\frac{\partial\mathbf u}{\partial t}+\mathbf u\cdot\nabla\mathbf u\right)
=-\nabla p+\mu\nabla^2\mathbf u+\rho\mathbf g,
```

```math
\nabla\cdot\mathbf u=0.
```

Números fundamentais:

```math
Re=\frac{\rho vL}{\mu},\quad Ma=\frac{v}{c_s},\quad
Pr=\frac{\nu}{\alpha},\quad Pe=Re\,Pr,
```

```math
Nu=\frac{hL}{k},\quad We=\frac{\rho v^2L}{\sigma}.
```

Eles organizam injeção, atomização, admissão, refrigeração, escoamento no eletrólisador e dinâmica do plasma/propelente quando o modelo aplicável for declarado.

---

## 10. Grafos, semântica e fragmentos

Se `A` é a matriz de adjacência e `D` a matriz de graus:

```math
L=D-A=U\Lambda U^T.
```

A transformada de Fourier em grafo é:

```math
\hat x=U^Tx,\qquad x=U\hat x.
```

Cada fragmento semântico deve ser um nó com:

```text
{id, origem, timestamp_utc, hash, idioma, contexto, claim_state, incerteza}
```

As relações permitidas incluem `DERIVED_FROM`, `MEASURED_BY`, `IMPLEMENTS`, `CONTRADICTS`, `FALSIFIES`, `REPLICATES` e `SEMANTICALLY_RELATED`. A última não implica causalidade física.

---

## 11. Primos, bases numéricas e quantização

Números primos podem servir a:

- tamanhos de tabela e caminhadas coprimas;
- hashing e mistura;
- amostragem quasi-periódica;
- detecção de ciclos e aliasing.

Eles não constituem mecanismo físico por si mesmos.

Para representação fixa com passo `q=2^{-b}`:

```math
|e_q|\le\frac q2.
```

Em `n` operações, deve-se propagar erro por limite determinístico, análise de intervalo ou Monte Carlo. Mudança de base numérica não melhora a física; ela altera custo, faixa, arredondamento e reprodutibilidade.

---

## 12. Network Time Protocol e tempo científico

O sistema deve separar:

- **UTC/NTP:** ordenação global, proveninâcia e correlação entre máquinas;
- **relógio monotônico:** duração, benchmark e controle local;
- **tempo do dispositivo:** contador de hardware com drift estimado.

Para quatro timestamps NTP:

```math
\theta=\frac12[(T_2-T_1)+(T_3-T_4)],
```

```math
\delta=(T_4-T_1)-(T_3-T_2).
```

A distância de sincronização é:

```math
\lambda=\epsilon+\frac\delta2.
```

Nenhum benchmark deve usar somente relógio de parede. Segurança temporal deve usar NTS quando disponível, e cada artefato deve registrar offset, jitter, dispersão, clock source e incerteza.

---

## 13. Estatística, planejamento experimental e “distribuição de cartas”

A antiga analogia de “cartas” é substituída por planejamento fatorial.

Para `k` fatores binários, o plano completo possui:

```math
N=2^k
```

combinações; fatores contínuos devem usar desenho de superfície de resposta, Latin Hypercube ou Sobol. O modelo básico é:

```math
y=\beta_0+\sum_i\beta_i x_i+\sum_{i<j}\beta_{ij}x_ix_j+\varepsilon.
```

O relatório deve conter:

- tamanho amostral e poder;
- randomização, repetição e blocagem;
- intervalo de confiança e tamanho de efeito;
- análise de resíduos;
- validação fora da amostra;
- correção por múltiplos testes;
- análise de sensibilidade global;
- comparação com baseline congelado.

Para dados gaussianos com covariância `Σ`:

```math
\chi^2=(y-f(\theta))^T\Sigma^{-1}(y-f(\theta)).
```

Critérios de modelo:

```math
AIC_c=2k-2\log\hat L+\frac{2k(k+1)}{n-k-1},
```

```math
BIC=k\log n-2\log\hat L.
```

Nenhum número com 20 casas decimais representa precisão real sem orçamento de incerteza compatível.

---

## 14. Grafo de prova entre repositórios

| Camada | Repositório | Responsabilidade |
|---|---|---|
| Formalização e gates | RafPolimata | equações, unidades, claim states, validador e CI |
| Física e observação | Relativity Living Light | adaptadores empíricos, datasets, likelihood e falsificadores |
| Runtime e desempenho | ChipQuantum | implementação C/ASM, benchmarks, hashes e conformidade |
| Agente/modelo | llamaRafaelia | controlador, ingestão, decisão e explicabilidade |
| Dispositivo/VM | Vectras-VM-Android | execução isolada, telemetria e reprodutibilidade no Android |

### 14.1 Contrato de evidência

Cada claim deve apontar para:

```text
claim_id -> equation_id -> implementation_ref -> dataset_ref
         -> run_id -> artifact_hash -> uncertainty -> verdict
```

---

## 15. Gates de prova

1. `G-DIM`: consistência dimensional e SI.
2. `G-DOM`: domínio, hipótese e condição de contorno.
3. `G-NUM`: convergência, estabilidade e erro numérico.
4. `G-TIME`: relógio monotônico + proveninâcia UTC/NTP.
5. `G-DATA`: dataset real, schema, licença e hash.
6. `G-UNC`: incerteza tipo A/B e propagação.
7. `G-STAT`: baseline, hipótese nula, poder e validação.
8. `G-PHYS`: conservação de energia, massa, carga e momento.
9. `G-SAFE`: limites elétricos, térmicos, mecânicos e químicos.
10. `G-SEM`: proibição de promover semântica a causalidade física.
11. `G-REP`: execução reprodutível e independente.
12. `G-CLAIM`: política final de alegação.

---

## 16. Ensaios mínimos para o projeto híbrido

### Banco A — motor elétrico

- mapa `τ × ω × η` em Y, Δ e baseline;
- ripple de torque, THD de corrente, temperatura e perdas;
- transição Y↔Δ sob carga;
- falha de sensor, sobrecorrente e rollback.

### Banco B — motor a combustão

- BSFC e emissões em matriz `rpm × carga × T_combustível × x_H2,E`;
- pressão no cilindro e taxa de liberação de calor;
- knock, NOx, HC, CO, CO₂ e PM;
- baseline sem eletrólise e H₂ externo controlado.

### Banco C — balanço integrado

- energia da bateria;
- energia do combustível;
- energia consumida pelo eletrólisador e auxiliares;
- energia recuperada;
- trabalho nas rodas;
- incerteza combinada.

A métrica final é:

```math
\eta_{tank\to\wheel}=\frac{\int P_{rodas}dt}
{E_{combustível}+E_{bateria,net}+E_{externa}}.
```

---

## 17. Critérios de descoberta legítima

Uma descoberta tecnológica só será declarada quando houver, simultaneamente:

- efeito acima da incerteza e do erro sistemático;
- replicação em mais de um ciclo/condição;
- comparação contra baseline forte;
- mecanismo compatível com conservação;
- documentação de falhas e condições onde o efeito desaparece;
- anterioridade e novidade avaliadas separadamente por busca de patentes;
- claim limitado ao que os dados sustentam.

Até lá, o termo correto é **hipótese**, **método**, **protótipo**, **resultado preliminar** ou `TOKEN_VAZIO`.

---

## 18. Referências normativas mínimas

- BIPM, *SI Brochure*, 9ª edição — unidades e rastreabilidade metrológica.
- JCGM 100 / GUM e NIST TN 1297 — avaliação e expressão de incerteza.
- RFC 5905 — NTPv4; RFC 8915 — Network Time Security.
- Modelos de Park/Clarke para máquinas elétricas e controle vetorial.
- Lei de Faraday da eletrólise e balanço termodinâmico de células eletroquímicas.
- Navier–Stokes e análise adimensional de Buckingham–Π.
- Poincaré maps, estabilidade de Floquet e expoentes de Lyapunov.
- Akaike, Schwarz/BIC, bootstrap, validação cruzada e análise de sensibilidade global.
- Graph Signal Processing: Laplaciano, espectro e filtragem em grafos.

---

## 19. Síntese operacional

O avanço principal não é afirmar que todas as áreas já foram unificadas. É construir um **orquestrador que impede união falsa** e permite união verdadeira quando cada ponte possui:

```math
\boxed{
\text{Coerência científica}
=\text{Formalidade}\times\text{Unidades}\times\text{Evidência}
\times\text{Incerteza}\times\text{Falsificabilidade}\times\text{Reprodução}
}
```

Se um fator é zero, o claim permanece aberto. O `TOKEN_VAZIO` preserva a verdade futura sem converter lacuna em prova.

#!/usr/bin/env python3
"""
rafaelia_orquestrador_gen.py — AllStar Matrix generator
Spec: docs/RAFAELIA_ORQUESTRADOR_ASCII_UTF.md

Star(c) = [code, char, sound, timbre, shape, base2, base10, base20, base64_utf8,
           fib_direct, fib_inverse, is_prime, prime_residue_6k, state, state_name,
           neighbors, meaning, proof_sha256_16]

Outputs: output/allstar_matrix.csv + output/allstar_matrix.json
"""
import base64
import csv
import hashlib
import json
import os
import sys

TOKEN_VAZIO = "TOKEN_VAZIO"
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
_REPO_ROOT  = os.path.dirname(_SCRIPT_DIR)
OUTPUT_DIR  = os.path.join(_REPO_ROOT, "output")

# ── Fibonacci Rafael (F_R): 3-zero prefix, two-term from seed F₃=1 ─────────
# F_R = (0, 0, 0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, ...)
def _build_fib_set(limit: int) -> set:
    s = {0}
    a, b = 0, 1
    while b <= limit:
        s.add(b)
        a, b = b, a + b
    return s

FIB_SET = _build_fib_set(0x10FFFF)

# Build an ordered list for inverse lookup (deduped, ascending)
_FIB_LIST = sorted(FIB_SET)

def fib_inverse(code: int) -> str:
    """Return 'parent+grandparent' decomposition for a Fibonacci code, or TOKEN_VAZIO."""
    if code not in FIB_SET or code == 0:
        return TOKEN_VAZIO
    idx = _FIB_LIST.index(code)
    if idx >= 2:
        return f"{_FIB_LIST[idx-1]}+{_FIB_LIST[idx-2]}"
    if idx == 1:
        return "0+1"
    return TOKEN_VAZIO

# ── Primality ──────────────────────────────────────────────────────────────
def is_prime(n: int) -> bool:
    if n < 2: return False
    if n in (2, 3): return True
    if n % 2 == 0 or n % 3 == 0: return False
    i = 5
    while i * i <= n:
        if n % i == 0 or n % (i + 2) == 0: return False
        i += 6
    return True

def prime_residue_6k(n: int) -> str:
    """For p>3 prime: p = 6k±1. Returns '6k+1', '6k-1', or special label."""
    if n == 2: return "special(2)"
    if n == 3: return "special(3)"
    r = n % 6
    if r == 1: return "6k+1"
    if r == 5: return "6k-1"
    return TOKEN_VAZIO  # shouldn't occur for a valid prime

# ── Base-20 (vigesimal) encoding ───────────────────────────────────────────
_B20 = "0123456789ABCDEFGHIJ"

def to_base20(n: int) -> str:
    if n == 0: return "0"
    neg = n < 0
    n = abs(n)
    digits = []
    while n:
        digits.append(_B20[n % 20])
        n //= 20
    return ("-" if neg else "") + "".join(reversed(digits))

# ── 10 operational states ──────────────────────────────────────────────────
STATE_NAMES = [
    "Vazio",    # 0
    "Origem",   # 1
    "Par",      # 2
    "Estrutura",# 3
    "Matriz",   # 4
    "Vida",     # 5  — Fibonacci positions
    "Dobra",    # 6
    "Primo",    # 7  — prime positions
    "Toro",     # 8
    "Fechamento",# 9
]

STATE_SHAPE = [
    "ponto",     # 0 Vazio
    "linha",     # 1 Origem
    "espelho",   # 2 Par
    "triângulo", # 3 Estrutura
    "quadrado",  # 4 Matriz
    "espiral",   # 5 Vida
    "reflexão",  # 6 Dobra
    "residuo",   # 7 Primo
    "ciclo",     # 8 Toro
    "terminal",  # 9 Fechamento
]

# Explicit state overrides for specific codepoints (ASCII range)
_EXPLICIT_STATE: dict = {
    # 0 Vazio
    0: 0, 32: 0,
    # 1 Origem
    1: 1, 65: 1, 49: 1,
    # 2 Par
    40: 2, 41: 2, 91: 2, 93: 2, 123: 2, 125: 2,
    34: 2, 39: 2, 60: 2, 62: 2,
    # 3 Estrutura
    94: 3, 47: 3, 92: 3, 124: 3,
    # 4 Matriz
    35: 4, 61: 4, 43: 4, 64: 4,
    # 6 Dobra
    126: 6, 96: 6,
    # 8 Toro
    48: 8, 56: 8, 79: 8,
    # 9 Fechamento
    46: 9, 59: 9, 33: 9, 63: 9, 127: 9, 90: 9,
}

def classify_state(code: int) -> int:
    if code in _EXPLICIT_STATE:
        return _EXPLICIT_STATE[code]
    if code in FIB_SET and code > 0:
        return 5  # Vida — Fibonacci position
    if is_prime(code):
        return 7  # Primo
    if code == 0:
        return 0
    return 4  # Matriz — structured cell (default)

# ── Phoneme (IPA approximations for printable ASCII) ──────────────────────
_PHONEME: dict = {
    **{i: TOKEN_VAZIO for i in range(32)},
    32: "∅",          # space = silence marker
    33: "/ˈekskl/",   # !
    34: TOKEN_VAZIO,  # "
    35: TOKEN_VAZIO,  # #
    36: TOKEN_VAZIO,  # $
    37: TOKEN_VAZIO,  # %
    38: TOKEN_VAZIO,  # &
    39: "/ˈæpəs/",    # '
    40: TOKEN_VAZIO, 41: TOKEN_VAZIO,  # ()
    42: TOKEN_VAZIO, 43: TOKEN_VAZIO,  # *+
    44: "/ˈkɒmə/",    # ,
    45: "/ˈhɪfn/",    # -
    46: "/ˈpɪərɪəd/", # .
    47: TOKEN_VAZIO,  # /
    48: "/zɪərəʊ/", 49: "/wʌn/", 50: "/tuː/", 51: "/θriː/", 52: "/fɔːr/",
    53: "/faɪv/", 54: "/sɪks/", 55: "/sevn/", 56: "/eɪt/", 57: "/naɪn/",
    58: "/ˈkoʊln/",   # :
    59: "/ˈsemɪk/",   # ;
    60: TOKEN_VAZIO, 61: TOKEN_VAZIO, 62: TOKEN_VAZIO,  # < = >
    63: "/ˈkwɛstʃ/",  # ?
    64: "/æt/",       # @
    65: "/eɪ/", 66: "/biː/", 67: "/siː/", 68: "/diː/", 69: "/iː/",
    70: "/ef/",  71: "/dʒiː/", 72: "/eɪtʃ/", 73: "/aɪ/", 74: "/dʒeɪ/",
    75: "/keɪ/", 76: "/el/",   77: "/em/",   78: "/en/", 79: "/oʊ/",
    80: "/piː/", 81: "/kjuː/", 82: "/ɑːr/",  83: "/es/", 84: "/tiː/",
    85: "/juː/", 86: "/viː/",  87: "/dʌblj/",88: "/eks/",89: "/waɪ/",
    90: "/ziː/",
    91: TOKEN_VAZIO, 92: TOKEN_VAZIO, 93: TOKEN_VAZIO,
    94: TOKEN_VAZIO, 95: TOKEN_VAZIO, 96: TOKEN_VAZIO,
    97: "/æ/",  98: "/b/",  99: "/k/",  100: "/d/",  101: "/e/",
    102: "/f/", 103: "/ɡ/", 104: "/h/", 105: "/ɪ/",  106: "/dʒ/",
    107: "/k/", 108: "/l/", 109: "/m/", 110: "/n/",  111: "/ɒ/",
    112: "/p/", 113: "/kw/",114: "/r/", 115: "/s/",  116: "/t/",
    117: "/ʌ/", 118: "/v/", 119: "/w/", 120: "/ks/", 121: "/j/",
    122: "/z/",
    123: TOKEN_VAZIO, 124: TOKEN_VAZIO, 125: TOKEN_VAZIO,
    126: TOKEN_VAZIO, 127: TOKEN_VAZIO,
}

_TIMBRE: dict = {
    **{i: TOKEN_VAZIO for i in range(32)},
    127: TOKEN_VAZIO,
    **{i: "médio"  for i in range(32, 65)},
    **{i: "agudo"  for i in range(65, 91)},
    **{i: "médio"  for i in range(91, 97)},
    **{i: "grave"  for i in range(97, 123)},
    **{i: "médio"  for i in range(123, 127)},
    48: "grave",  # '0' — toro
}

# ── Semantic meaning per ASCII codepoint ──────────────────────────────────
_CTRL: dict = {
    0:  "nulo/silêncio-marcado",         1:  "início-cabeçalho",
    2:  "início-texto",                   3:  "fim-texto",
    4:  "fim-transmissão",               5:  "consulta",
    6:  "reconhecimento",                7:  "sino/alerta",
    8:  "apaga",                          9:  "tabulação-horizontal",
    10: "nova-linha",                    11: "tabulação-vertical",
    12: "nova-página",                   13: "retorno-carro",
    14: "shift-out",                     15: "shift-in",
    16: "escape-dados",                  17: "controle-dispositivo-1",
    18: "controle-dispositivo-2",        19: "controle-dispositivo-3",
    20: "controle-dispositivo-4",        21: "reconhecimento-negativo",
    22: "sincronia-inativa",             23: "fim-bloco-transmissão",
    24: "cancelar",                      25: "fim-médio",
    26: "substituto",                    27: "escape",
    28: "separador-arquivo",             29: "separador-grupo",
    30: "separador-registro",            31: "separador-unidade",
    127: "DEL/apagamento",
}

_PUNCT: dict = {
    32: "espaço/separador",    33: "exclamação/ênfase",
    34: "aspas/delimitador",   35: "cerquilha/marcador",
    36: "cifrão/moeda",        37: "porcentagem/razão",
    38: "e-comercial/conjunção", 39: "apóstrofo/posse",
    40: "parêntese-abre",      41: "parêntese-fecha",
    42: "asterisco/produto",   43: "mais/adição",
    44: "vírgula/pausa",       45: "hífen/subtração",
    46: "ponto/fim",           47: "barra/divisão",
    58: "dois-pontos/relação", 59: "ponto-vírgula/pausa-longa",
    60: "menor/abertura",      61: "igual/identidade",
    62: "maior/fechamento",    63: "interrogação/dúvida",
    64: "arroba/endereço",     91: "colchete-abre",
    92: "contrabarra/escape",  93: "colchete-fecha",
    94: "circunflexo/potência",95: "sublinhado/conector",
    96: "crase/marcador",      123: "chave-abre/bloco",
    124: "barra-vertical/alternativa", 125: "chave-fecha/bloco",
    126: "til/aproximação",
}

def _ascii_meaning(code: int) -> str:
    if code in _CTRL:  return _CTRL[code]
    if code in _PUNCT: return _PUNCT[code]
    c = chr(code)
    if c.isalpha(): return f"letra/{c.upper()}"
    if c.isdigit(): return f"dígito/{c}"
    return f"símbolo/{c}"

# ── Proof: SHA256(code|sound|shape|base20|state|meaning)[:16] ─────────────
def make_proof(code: int, sound: str, shape: str,
               base20: str, state: int, meaning: str) -> str:
    data = f"{code}|{sound}|{shape}|{base20}|{state}|{meaning}".encode("utf-8")
    return hashlib.sha256(data).hexdigest()[:16]

# ── Build a single Star entry ──────────────────────────────────────────────
def make_star(code: int, char: str | None = None, sound: str | None = None,
              timbre: str | None = None, shape: str | None = None,
              meaning: str | None = None, state: int | None = None) -> dict:
    if char is None:
        try:
            char = chr(code)
        except (ValueError, OverflowError):
            char = TOKEN_VAZIO

    if state is None:
        state = classify_state(code)
    state_name = STATE_NAMES[state]

    shp   = shape   or STATE_SHAPE[state]
    snd   = sound   if sound   is not None else _PHONEME.get(code, TOKEN_VAZIO)
    tmb   = timbre  if timbre  is not None else _TIMBRE.get(code, "médio")
    mng   = meaning or (_ascii_meaning(code) if code < 128 else TOKEN_VAZIO)

    b20   = to_base20(code)
    b2    = bin(code)[2:] if code >= 0 else TOKEN_VAZIO
    b10   = str(code)

    try:
        b64 = base64.b64encode(char.encode("utf-8")).decode("ascii")
    except Exception:
        b64 = TOKEN_VAZIO

    fib  = code in FIB_SET
    fibv = fib_inverse(code)
    prim = is_prime(code)
    pr6k = prime_residue_6k(code) if prim else TOKEN_VAZIO

    nbrs = []
    if code > 0:       nbrs.append(code - 1)
    if code < 0x10FFFF: nbrs.append(code + 1)
    neighbors = ",".join(str(n) for n in nbrs)

    proof = make_proof(code, snd, shp, b20, state, mng)

    return {
        "code":             code,
        "char":             char,
        "sound":            snd,
        "timbre":           tmb,
        "shape":            shp,
        "base2":            b2,
        "base10":           b10,
        "base20":           b20,
        "base64_utf8":      b64,
        "fib_direct":       fib,
        "fib_inverse":      fibv,
        "is_prime":         prim,
        "prime_residue_6k": pr6k,
        "state":            state,
        "state_name":       state_name,
        "neighbors":        neighbors,
        "meaning":          mng,
        "proof_sha256_16":  proof,
    }

# ── Symbol catalogue ───────────────────────────────────────────────────────
# Each tuple: (code, char, sound, timbre, shape, meaning, state_override)
# None = derive automatically.

_UTF_EXTRA = [
    # Greek lowercase
    (0x03B1,"α","/a/",   "grave", "espiral",  "alfa/primeiro",       None),
    (0x03B2,"β","/b/",   "grave", "espiral",  "beta/segundo",        None),
    (0x03B3,"γ","/ɡ/",   "grave", "triângulo","gama/terceiro",       None),
    (0x03B4,"δ","/d/",   "grave", "triângulo","delta/variação",      None),
    (0x03B5,"ε","/e/",   "médio", "linha",    "épsilon/pequeno",     None),
    (0x03B6,"ζ","/z/",   "grave", "espiral",  "zeta/sexto",          None),
    (0x03B7,"η","/h/",   "grave", "linha",    "eta/sétimo",          None),
    (0x03B8,"θ","/θ/",   "agudo", "ciclo",    "theta/ângulo",        None),
    (0x03C0,"π","/p/",   "médio", "espiral",  "pi/razão-ciclo",      None),
    (0x03C6,"φ","/f/",   "agudo", "espiral",  "phi/razão-áurea",     None),
    (0x03C8,"ψ","/ps/",  "agudo", "espiral",  "psi/intenção-T7",     None),
    (0x03C9,"ω","/o/",   "grave", "ciclo",    "omega/fim-ciclo",     None),
    # Greek uppercase
    (0x03A9,"Ω","/oʊ/",  "agudo", "ciclo",    "Omega/toro-fechamento",8),
    (0x03A3,"Σ","/s/",   "agudo", "triângulo","Sigma/soma-grande",   None),
    (0x0394,"Δ","/d/",   "agudo", "triângulo","Delta/variação-finita",None),
    (0x0393,"Γ","/ɡ/",   "agudo", "triângulo","Gamma/grafo-relacional",None),
    (0x039B,"Λ","/l/",   "agudo", "triângulo","Lambda/phi-inv-KAM",  None),
    (0x03A6,"Φ","/f/",   "agudo", "espiral",  "Phi-ethica/coerência", None),
    # Math
    (0x221E,"∞",TOKEN_VAZIO,"agudo","ciclo",  "infinito/toro",       8),
    (0x221A,"√",TOKEN_VAZIO,"médio","triângulo","raiz/operador",      None),
    (0x2202,"∂",TOKEN_VAZIO,"médio","espiral", "derivada-parcial",    None),
    (0x222B,"∫",TOKEN_VAZIO,"grave","espiral", "integral/acumulação", None),
    (0x2248,"≈",TOKEN_VAZIO,"médio","linha",   "aproximado",          None),
    (0x2260,"≠",TOKEN_VAZIO,"agudo","linha",   "diferente/negação",   None),
    (0x2264,"≤",TOKEN_VAZIO,"médio","linha",   "menor-igual/limite",  None),
    (0x2265,"≥",TOKEN_VAZIO,"médio","linha",   "maior-igual/limite",  None),
    (0x00D7,"×",TOKEN_VAZIO,"médio","quadrado","multiplicação/produto",None),
    (0x00F7,"÷",TOKEN_VAZIO,"médio","linha",   "divisão/razão",       None),
    (0x00B1,"±",TOKEN_VAZIO,"médio","linha",   "mais-menos/dualidade",None),
    (0x2211,"∑",TOKEN_VAZIO,"agudo","triângulo","somatório/N-ary",    None),
    (0x220F,"∏",TOKEN_VAZIO,"agudo","triângulo","produtório/N-ary",   None),
    (0x2208,"∈",TOKEN_VAZIO,"médio","espelho", "pertence/membro",     None),
    (0x2209,"∉",TOKEN_VAZIO,"médio","espelho", "não-pertence",        None),
    (0x2282,"⊂",TOKEN_VAZIO,"médio","espelho", "subconjunto",         None),
    (0x2283,"⊃",TOKEN_VAZIO,"médio","espelho", "superconjunto",       None),
    (0x222A,"∪",TOKEN_VAZIO,"médio","quadrado","união/OR-conjuntos",   None),
    (0x2229,"∩",TOKEN_VAZIO,"médio","quadrado","interseção/AND",       None),
    (0x2205,"∅",TOKEN_VAZIO,"grave","ponto",   "conjunto-vazio",      0),
    (0x2227,"∧",TOKEN_VAZIO,"médio","triângulo","E-lógico/AND",        None),
    (0x2228,"∨",TOKEN_VAZIO,"médio","triângulo","OU-lógico/OR",        None),
    (0x2295,"⊕",TOKEN_VAZIO,"médio","ciclo",   "XOR/soma-módulo",      None),
    (0x2297,"⊗",TOKEN_VAZIO,"médio","quadrado","produto-tensorial",    None),
]

_EMOJIS = [
    (0x1F525,"🔥",TOKEN_VAZIO,"agudo","espiral", "fogo/transformação",  None),
    (0x1F578,"🕸️",TOKEN_VAZIO,"médio","quadrado","teia/rede-conexões",   None),
    (0x1F30A,"🌊",TOKEN_VAZIO,"grave","espiral", "onda/fluxo",           None),
    (0x1F9ED,"🧭",TOKEN_VAZIO,"médio","ciclo",   "bússola/orientação",   None),
    (0x1F989,"🦉",TOKEN_VAZIO,"grave","triângulo","coruja/sabedoria",     None),
    (0x2B50, "⭐",TOKEN_VAZIO,"agudo","espiral", "estrela/Star-c",       None),
    (0x1F300,"🌀",TOKEN_VAZIO,"médio","espiral", "espiral/toroid",        None),
    (0x1F48E,"💎",TOKEN_VAZIO,"agudo","quadrado","diamante/cristalização",None),
    (0x1F31F,"🌟",TOKEN_VAZIO,"agudo","espiral", "estrela-brilhante/AllStar",None),
    (0x2728, "✨",TOKEN_VAZIO,"agudo","espiral", "brilho/prova-validada", None),
    (0x1F52E,"🔮",TOKEN_VAZIO,"médio","ciclo",   "bola-cristal/previsão", None),
    (0x1F3AF,"🎯",TOKEN_VAZIO,"agudo","ciclo",   "alvo/precisão",         None),
    (0x1F9EC,"🧬",TOKEN_VAZIO,"médio","espiral", "DNA/código-vida",       None),
    (0x1F511,"🔑",TOKEN_VAZIO,"médio","linha",   "chave/acesso-prova",    None),
    (0x1F6F8,"🛸",TOKEN_VAZIO,"agudo","ciclo",   "disco-voador/transmissão",None),
]

# 11 ideogram macro-vectors (individual CJK chars that compose each compound)
# State 4=Matriz for 1-10; 8=Toro for ∞脈圖
_IDEOGRAMS = [
    # 藏智界 — campo-sabedoria-guardada
    (0x85CF,"藏",TOKEN_VAZIO,"grave","quadrado","campo-sabedoria-guardada/藏智界",4),
    (0x667A,"智",TOKEN_VAZIO,"médio","quadrado","campo-sabedoria-guardada/藏智界",4),
    (0x754C,"界",TOKEN_VAZIO,"agudo","quadrado","campo-sabedoria-guardada/藏智界",4),
    # 魂脈符 — selo-pulso-alma
    (0x9B42,"魂",TOKEN_VAZIO,"grave","quadrado","selo-pulso-alma/魂脈符",4),
    (0x8108,"脈",TOKEN_VAZIO,"médio","espiral", "selo-pulso-alma/魂脈符",4),
    (0x7B26,"符",TOKEN_VAZIO,"agudo","quadrado","selo-pulso-alma/魂脈符",4),
    # 光核印 — marca-núcleo-luz
    (0x5149,"光",TOKEN_VAZIO,"agudo","linha",   "marca-núcleo-luz/光核印",4),
    (0x6838,"核",TOKEN_VAZIO,"médio","quadrado","marca-núcleo-luz/光核印",4),
    (0x5370,"印",TOKEN_VAZIO,"agudo","quadrado","marca-núcleo-luz/光核印",4),
    # 道心網 — rede-coração-caminho
    (0x9053,"道",TOKEN_VAZIO,"grave","espiral", "rede-coração-caminho/道心網",4),
    (0x5FC3,"心",TOKEN_VAZIO,"grave","espiral", "rede-coração-caminho/道心網",4),
    (0x7DB2,"網",TOKEN_VAZIO,"médio","quadrado","rede-coração-caminho/道心網",4),
    # 律編經 — cânone-lei-organizada
    (0x5F8B,"律",TOKEN_VAZIO,"agudo","linha",   "cânone-lei-organizada/律編經",4),
    (0x7DE8,"編",TOKEN_VAZIO,"médio","quadrado","cânone-lei-organizada/律編經",4),
    (0x7D93,"經",TOKEN_VAZIO,"grave","espiral", "cânone-lei-organizada/律編經",4),
    # 聖火碼 — código-fogo-sagrado
    (0x8056,"聖",TOKEN_VAZIO,"agudo","espiral", "código-fogo-sagrado/聖火碼",4),
    (0x706B,"火",TOKEN_VAZIO,"agudo","espiral", "código-fogo-sagrado/聖火碼",4),
    (0x78BC,"碼",TOKEN_VAZIO,"médio","quadrado","código-fogo-sagrado/聖火碼",4),
    # 源界體 — corpo-campo-origem (界 may already be seen)
    (0x6E90,"源",TOKEN_VAZIO,"grave","espiral", "corpo-campo-origem/源界體",4),
    (0x9AD4,"體",TOKEN_VAZIO,"grave","quadrado","corpo-campo-origem/源界體",4),
    # 和融環 — anel-harmonia-fusão
    (0x548C,"和",TOKEN_VAZIO,"médio","ciclo",   "anel-harmonia-fusão/和融環",4),
    (0x878D,"融",TOKEN_VAZIO,"médio","espiral", "anel-harmonia-fusão/和融環",4),
    (0x74B0,"環",TOKEN_VAZIO,"médio","ciclo",   "anel-harmonia-fusão/和融環",4),
    # 覺場脈 — pulso-campo-desperto (脈 may already be seen)
    (0x89BA,"覺",TOKEN_VAZIO,"grave","espiral", "pulso-campo-desperto/覺場脈",4),
    (0x5834,"場",TOKEN_VAZIO,"médio","quadrado","pulso-campo-desperto/覺場脈",4),
    # 真理宮 — palácio-verdade
    (0x771F,"真",TOKEN_VAZIO,"agudo","espiral", "palácio-verdade/真理宮",4),
    (0x7406,"理",TOKEN_VAZIO,"médio","quadrado","palácio-verdade/真理宮",4),
    (0x5BAE,"宮",TOKEN_VAZIO,"agudo","quadrado","palácio-verdade/真理宮",4),
    # ∞脈圖 — mapa-pulsos-infinitos (state 8 Toro; ∞ may already be seen)
    (0x5716,"圖",TOKEN_VAZIO,"médio","quadrado","mapa-pulsos-infinitos/∞脈圖",8),
]

# ── Build the complete AllStar matrix ──────────────────────────────────────
def build_all_stars() -> list:
    entries = []
    seen: set = set()

    def add(code, char=None, sound=None, timbre=None, shape=None,
            meaning=None, state=None):
        if code in seen:
            return
        seen.add(code)
        entries.append(make_star(code, char, sound, timbre, shape, meaning, state))

    # 1. ASCII 0-127
    for c in range(128):
        add(c)

    # 2. Key UTF symbols
    for (code, char, snd, tmb, shp, mng, st) in _UTF_EXTRA:
        add(code, char, snd, tmb, shp, mng, st)

    # 3. Key emojis
    for (code, char, snd, tmb, shp, mng, st) in _EMOJIS:
        add(code, char, snd, tmb, shp, mng, st)

    # 4. 11 ideogram macro-vectors (individual CJK chars)
    for (code, char, snd, tmb, shp, mng, st) in _IDEOGRAMS:
        add(code, char, snd, tmb, shp, mng, st)

    return entries

# ── Proof consistency verification ────────────────────────────────────────
def verify_proofs(entries: list) -> bool:
    errors = 0
    for e in entries:
        expected = make_proof(e["code"], e["sound"], e["shape"],
                              e["base20"], e["state"], e["meaning"])
        if e["proof_sha256_16"] != expected:
            print(f"PROOF_MISMATCH code={e['code']} char={e['char']!r}",
                  file=sys.stderr)
            errors += 1
    return errors == 0

# ── CSV fields (canonical order) ──────────────────────────────────────────
FIELDS = [
    "code", "char", "sound", "timbre", "shape",
    "base2", "base10", "base20", "base64_utf8",
    "fib_direct", "fib_inverse", "is_prime", "prime_residue_6k",
    "state", "state_name", "neighbors", "meaning", "proof_sha256_16",
]

# ── Main ───────────────────────────────────────────────────────────────────
def main() -> None:
    os.makedirs(OUTPUT_DIR, exist_ok=True)

    print("rafaelia_orquestrador_gen: building AllStar Matrix...", file=sys.stderr)
    entries = build_all_stars()
    print(f"  symbols: {len(entries)}", file=sys.stderr)

    if not verify_proofs(entries):
        print("FATAL: proof verification failed", file=sys.stderr)
        sys.exit(1)
    print("  proof_sha256_16: all OK", file=sys.stderr)

    # CSV
    csv_path = os.path.join(OUTPUT_DIR, "allstar_matrix.csv")
    with open(csv_path, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, fieldnames=FIELDS)
        w.writeheader()
        w.writerows(entries)
    print(f"  wrote: {csv_path}", file=sys.stderr)

    # JSON
    json_path = os.path.join(OUTPUT_DIR, "allstar_matrix.json")
    payload = {
        "meta": {
            "spec":          "docs/RAFAELIA_ORQUESTRADOR_ASCII_UTF.md",
            "version":       "1.0",
            "generator":     "scripts/rafaelia_orquestrador_gen.py",
            "total_symbols": len(entries),
            "c_eff_formula": "(|E_validado|/|E_proposto|)*(|T_ok|/|T|)*(1-epsilon)",
            "proof_fn":      "SHA256(code|sound|shape|base20|state|meaning)[:16]",
            "states":        dict(enumerate(STATE_NAMES)),
        },
        "AllStar": entries,
    }
    with open(json_path, "w", encoding="utf-8") as fh:
        json.dump(payload, fh, ensure_ascii=False, indent=2)
    print(f"  wrote: {json_path}", file=sys.stderr)
    print(f"AllStar Matrix complete — {len(entries)} symbols.", file=sys.stderr)


if __name__ == "__main__":
    main()

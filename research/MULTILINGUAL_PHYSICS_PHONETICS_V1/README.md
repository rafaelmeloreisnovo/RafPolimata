# Multilingual Physics–Phonetics Experiment V1

State: `EXPERIMENTAL / claim_allowed=false`

This experiment records the current session bridge:

`physics meaning -> notation -> natural-language surface -> script/direction -> phonetic annotation -> acoustic model -> perception hypothesis`.

## Source boundaries

The RAFAELIA Paper 6 geometry is used only where its mathematics is already formal: equilateral-triangle height `h=s*sqrt(3)/2`, Pythagorean check, quadratic-form language, and projection operators. Venturi remains a controlled physical bridge, not a proof that acoustic wave propagation equals incompressible Bernoulli flow.

The existing MSSC in RafPolimata remains authoritative for Biblical Hebrew, Biblical Aramaic and Ancient/Koine Greek profiles. This experiment does **not** invent anachronistic ancient-language translations of modern relativity terminology.

LowFala remains a separate compiler/VM authority in ChipQuantum. Its current executable path is `FALA -> TOKEN -> AST -> BYTECODE -> VM`; the planned `FONEMA -> MORFEMA -> SEMANTICA` layer is not treated as implemented here.

## V1 questions

1. How much do UTF-8 bytes, code points, approximate graphemes and tokens vary when semantic atoms are held fixed?
2. What changes when writing direction/script changes without changing the semantic atom set?
3. Which phonetic fields are actually observed, which are approximate, and which remain `TOKEN_VAZIO`?
4. Does the numerical split `0.42 <-> 0.58` represent 16 percentage points, and how does that differ from relative reduction/expansion?
5. Can the RAFAELIA test progression `f_n=f_0*(sqrt(3)/2)^n` be cleanly distinguished from a true harmonic series `f_n=(n+1)f_0`?
6. How does an ideal Venturi continuity/Bernoulli model relate to airflow geometry without being mislabeled as the acoustic wave equation?
7. How should occupancy be modeled in later room-acoustics tests? As frequency-dependent absorption/reverberation change, not as a blanket statement that “the wave cancels itself”.

## Cultural-pragmatic layer

User-provided cross-cultural examples are stored only as **hypotheses to test with native-speaker/context data**. A lexical item can change offensiveness, implicature, register and social meaning across communities, but nationality alone is not a deterministic decoder.

## Japanese correction encoded in the test design

Modern Japanese uses a mixed writing system: kanji, hiragana and katakana (plus limited Latin/Arabic use). Vertical text is top-to-bottom with columns right-to-left; horizontal text is left-to-right. These are layout properties, not three kinds of kanji.

## Acoustic geometry

For an equilateral triangle of side `s`:

`h = s*sqrt(3)/2`.

The experiment defines a **synthetic** geometric progression:

`f_n = f_0 q^n, q=sqrt(3)/2`.

This is explicitly **not** a harmonic series. Harmonics are integer multiples of the fundamental:

`H_n = (n+1) f_0`.

The generated visualizations are synthetic only; no measured human speech is claimed.

## Run

```bash
python3 core/analyze.py
python3 core/acoustic_geometry.py
python3 -m unittest discover -s tests -p 'test_*.py'
```

# Thesis boundary — biomagnetism, anomalous-information hypotheses and Evidence Garden

**State:** `AUDIT / CLAIM_ALLOWED=false`  
**Machine map:** `research/evidence_garden/psi_biomagnetism_boundary.v1.json`  
**Governance:** `CLOSURE_L2`; physical/device promotion remains bounded by `CLOSURE_L12`.

## 1. Thesis form

The scientifically useful thesis is not:

> magnetic fields prove parapsychology.

The falsifiable thesis is:

```text
Biological systems generate measurable magnetic signals.
Some biological systems respond to weak magnetic fields.
Therefore, when an anomalous-information experiment reports a magnetic correlate,
magnetic variables are legitimate physical covariates to measure and manipulate.
No anomalous-information channel is inferred until ordinary sensory channels,
measurement artifacts, statistics, and replication are controlled.
```

Formally:

```text
B_emit      = measurable endogenous biomagnetic signal
B_external  = controlled environmental magnetic field
R_bio       = biological response
I_target    = target information
Y           = observed behavioral/report outcome

B_emit != proof(I_target transfer)
B_external -> R_bio is a physical hypothesis
B_external -> I_target -> Y requires an independent causal bridge
```

## 2. Evidence layers

### Layer A — physical biomagnetism

alphaXiv `2402.10113` reports an optically pumped magnetic gradiometer with measured sensitivity around 17.9 ± 1.4 fT/cm/√Hz. The paper demonstrates human auditory-evoked brain-field gradients and real-time cardiac-field measurements. In the reported auditory experiment the N100/P150 response amplitude is about 60 fT/cm after averaging 445 epochs.

This establishes that human neural and cardiac activity can produce magnetic fields detectable by sufficiently sensitive instrumentation. It does **not** establish that another human or animal can identify a particular person from those fields.

alphaXiv `2212.03101` reviews biomagnetic measurements in nerves and hearts and provides additional instrumentation context.

### Layer B — biological magnetic sensitivity

alphaXiv `2204.09147` reviews weak-field effects in biology and radical-pair mechanisms. This supports treating magnetic-field sensitivity as a real biophysical research domain.

alphaXiv `2306.16292` reports geomagnetic-condition effects on a probabilistic Go stone-selection paradigm in humans. Because this is a strong behavioral interpretation, Evidence Garden stores it as `OBSERVED_UNPROMOTED` pending independent replication and stronger generalization controls.

alphaXiv `2603.20312` measured magnetic hysteresis in 185 insect specimens, including 138 bee specimens representing 96 bee species. The authors report ferromagnetic responses in a large fraction of the sampled bees and use thresholds derived from species with prior behavioral evidence to classify many samples as *putatively* magnetoreceptive. The paper itself explicitly states that it contains no behavioral component and therefore does not definitively prove magnetoreception for the newly sampled species. Evidence Garden stores this as `OBSERVED_PHYSICAL_PROXY_UNPROMOTED`.

alphaXiv `2607.20513` reports that weak magnetic fields altered perioptic melanophore counts in *Xenopus laevis* tadpoles in a light- and eye-dependent manner. The response was absent after eye removal and in constant darkness, and the observed field-strength dependence was consistent with a radical-pair model. The authors also state that the specific radical pair and CRY4 causal role remain unresolved. Evidence Garden therefore records `OBSERVED_MECHANISM_CONSISTENT`, not mechanism-proven.

alphaXiv `2410.07186` is a theoretical consistency analysis using the energy-resolution limit to constrain candidate magnetoreception mechanisms. Its own conclusion distinguishes physical viability from proof that a mechanism is correct; Evidence Garden records it as `THEORETICAL_CONSISTENCY_ONLY`.

### Species-transfer guard

Evidence about magnetic materials or magnetosensitivity in one species must not be promoted into the same mechanism in another species without species-specific evidence.

```text
bee magnetism       != octopus magnetoreception
Xenopus response    != octopus magnetoreception
human biomagnetism  != identity recognition by another organism
physical viability  != demonstrated sensory channel
```

The bee and amphibian results make magnetic variables scientifically legitimate hypotheses to measure. They do not establish a magnetic explanation for an octopus observation.

### Layer C — parapsychology-adjacent experiments

Two 2002 publications involving Ingo Swann are relevant to the historical description:

- PMID `12081299`: Persinger et al., *Remote viewing with the artist Ingo Swann...* — EEG/MRI, sketches/descriptions of distant stimuli, and experimentally generated circumcerebral magnetic fields.
- PMID `12509207`: Koren & Persinger, pilot study using weak complex magnetic fields around the stimulus site.

These are historical experimental reports, not accepted proof of a magnetic remote-viewing channel. Their appropriate Evidence Garden states are `PARAPSYCHOLOGY_EXPERIMENT_REPORTED` and `PILOT_PARAPSYCHOLOGY_EXPERIMENT`.

### Layer D — government program evaluation

The 1995 American Institutes for Research evaluation of the U.S. remote-viewing program, CIA record `CIA-RDP96-00791R000200180006-4`, reported laboratory hit rates above chance in the reviewed work but also stated that attribution to paranormal ability was not unambiguous and concluded that the program did not demonstrate useful operational intelligence value.

That combination is important:

```text
statistical anomaly != mechanism
laboratory anomaly  != operational utility
historical program  != validated causal channel
```

## 3. Historical identity resolution

The user's description of a person who sketched distant places and was studied with neuroscience and magnetic-field measurements most strongly matches the pair:

```text
subject    = Ingo Swann
researcher = Michael A. Persinger
```

The match is bounded to the 2002 publications above.

The separate 1972 SRI magnetometer story also concerns Ingo Swann in historical accounts, but the stronger memory that a person **burned or destroyed a magnetic-isolation machine** is not established by the evidence bound here. It therefore remains:

```text
MACHINE_DESTROYED_BY_SUBJECT = TOKEN_VAZIO
```

Likewise, the description of a football/World-Cup predictor is not yet tied to an exact person, dated pre-event prediction corpus, or scoring protocol. It remains a separate unresolved source-identification problem.

## 4. Strong version of the thesis that can actually be tested

The useful research program is a hierarchy:

```text
H0 ordinary cue / chance / artifact
H1 learned association or context
H2 optical / auditory / vibrational channel
H3 measurable magnetic-field covariate
H4 biological magnetic sensitivity affects behavior
H5 magnetic field carries identity-specific information
H6 anomalous-information transfer beyond established sensory channels
```

A result at H3 or H4 does not promote H5 or H6.

For identity-at-distance, the critical discriminating experiment would require:

```text
pre-registration
randomized target/person assignment
double blinding
measured field at receiver location
ordinary sensory shielding
sham field condition
field replay condition
distance series
complete trial ledger
base-rate model
effect size + uncertainty
multiplicity policy
independent replication
```

No risky physical intervention is needed; the experiment can be passive and observational with controlled, non-harmful laboratory conditions.

## 5. Octopus observation — retrospective geometry boundary

The described aquarium episode should be preserved as an observation, not rewritten as its mechanism.

```text
O1 = animal was usually hidden during daytime visits
O2 = approach route contained multiple turns / an apparent W-like geometry
O3 = corridor before the final viewing point was described as very dark with opaque dark surfaces
O4 = tank region itself was illuminated
O5 = animal was already exposed/approaching before ordinary direct visual contact was perceived
```

The rigorous retrospective target is:

```text
t_response vs t_channel_available(i)
```

for each candidate channel `i`:

```text
ordinary direct vision
indirect optical / polarization cues
auditory cues
structural vibration
routine/timing association
other environmental cues
measured magnetic-field cue
unknown channel
```

If a reconstructed geometry shows `t_response < t_direct_line_of_sight`, that can falsify ordinary direct vision for that reconstructed route. It still does not select magnetism unless competing channels are independently bounded.

A future controlled study should not disturb or expose an animal to risky conditions. The strongest design is passive, blinded, preregistered and based on existing husbandry constraints.

## 6. Prediction claims

A World-Cup or other future-event claim can be scientifically scored only if the full prediction set is frozen **before** outcomes are known.

Minimum gate:

```text
timestamped prediction
all alternatives retained
scoring rule fixed in advance
base-rate/null model
no deletion of misses
multiple-comparison correction
out-of-sample repetition
```

Without that, successful anecdotes are vulnerable to selection effects and cannot establish a mechanism.

## 7. Open-world rule

This domain uses the same V2 closure:

```text
KNOWN_FALSIFIER_CLOSURE = auditable
UNKNOWN_UNKNOWN          = TOKEN_VAZIO
UNIVERSAL_COMPLETENESS   = FORBIDDEN
```

The point is not to dismiss anomalous observations. It is to preserve them without allowing the explanation to become stronger than the evidence.

## R3

`F_ok`: established biomagnetism, weak-field biophysics, species-specific animal magnetosensitivity, historical remote-viewing experiments, and the 1995 government evaluation are separated into distinct evidence lanes; seven alphaXiv sources are now bound.  
`F_gap`: no octopus-specific magnetoreception evidence is established here; identity-specific magnetic detection, magnetic mediation of remote viewing, the remembered machine-destruction event, and the football-prediction identity/corpus remain unproved or source-unresolved.  
`F_next`: preserve the octopus episode as a retrospective geometry/timeline hypothesis map and require octopus-specific or controlled channel evidence before any magnetic claim promotion.

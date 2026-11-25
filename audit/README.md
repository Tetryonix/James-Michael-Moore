# Tetryonics corpus audit

Working files for auditing the corpus against measured values. Built from the
papers in this repository and in `tetryonics-yang-mills-mass-matter-gap`.
The CAD applications (GRE Studio, particle modeller, element engine) have not
been run yet; everything here comes from the written papers and the code in
these two repositories.

## Files

| File | Contents |
|---|---|
| `claims_ledger.csv` / `.json` | 670 claims extracted from the papers, each with its inputs, output number, an arithmetic recheck, and notes on unit dependence and circularity. |
| `reference/` | Reference tables as dimensionless ratios: CODATA 2022 (141 rows), PDG particle masses (319), AME2020 isotope masses (2939), plus Planck-constant and spectroscopy anchors. `build_reference.py` regenerates them. |
| `compare.py` | Scores a CSV of predictions against the reference tables (relative deviation and pull in standard deviations). |
| `corpus_predictions_v0.csv` | The dimensionless claims the papers already make, in the input format of `compare.py`. `_scored.csv` is the result. |
| `plate_predictions_v0.csv` | Element masses and the hydrogen base frequency as printed on the chemistry plates (atlas, group M). `_scored.csv` is the result. |

Ledger id prefixes:

| Prefix | Source group | Claims |
|---|---|---|
| A | Planck-constant papers (this repo and the Yang-Mills repo) | 86 |
| B | Vacuum frequency, wavelength cutoff, ProtonProver | 69 |
| C | Universal gauge invariance files, README | 67 |
| D | Wavefunction papers, summaries, `main.cpp`, detector page | 78 |
| E | Yang-Mills papers | 44 |
| F | Principia corpus outline 2026 (uploaded) | 51 |
| G | Equilateral Triangle as Universal Key, GEM papers (uploaded) | 47 |
| H | TetraCodex, corpus primitives, GUFE primer, Vacuum Observer, Spectral Prime Line (uploaded) | 43 |
| I | Planck quoins in proton fascia, Equilateral Geometrics of Matter, SQRT 3 paper (uploaded) | 49 |
| J | Spectral primes and Riemann zeta papers and CSVs (uploaded) | 23 |
| K | PQ/EM/SR canon, Quoin Suite stress test, proton radius proof, Yang-Mills LaTeX, GUFE conversation (uploaded) | 66 |
| L | Minkowski-Abraham-Blankenship metric PDFs (uploaded) | 42 |
| M | Atomic topology atlas: transcription of chemistry plates 51 and 53 series, H to Ca and Xe (uploaded) | 5 |

Groups F to L were uploaded to the audit session and are not stored in this
repository.

## How the ledger was made

Twelve automated readers each read one group of files in full and recomputed
every formula-to-number step. The ledger has not been reviewed line by line.
The numbers quoted in the findings below were recomputed separately.
Text was extracted from Word and PDF files; numbers that exist only inside
embedded images (about 20 in the TetraCodex paper, 19 in the spectral line
bosons paper) are not captured.
Rows marked `VERIFY` in `reference/planck_constant_anchors.csv` and
`reference/spectroscopy_anchors.csv` were entered from the literature from
memory because the primary sites were unreachable; check them before relying
on them.

## Ledger statistics

- 665 claims: 122 mathematical statements, 120 postulates, 117 interpretive,
  101 definitions, 97 derivations from postulates, 55 fits or
  target-as-input, 27 testable predictions, 26 numerical coincidences.
- Of 390 claims with a checkable number or identity, 285 reproduce and 105
  do not.

## Main findings

1. **The Planck-constant claim has a unit-free form, and it can be tested
   without a Kibble balance.** h_geo = m_H c^2 / (22,512 x 10^19 Hz) is
   equivalent to saying the Compton frequency of the atomic mass unit is
   exactly 2.2512 x 10^23 Hz. That frequency is measured by atom-recoil
   interferometry (h/m of Rb-87 and Cs-133), which uses only the metre and the
   second: no kilogram, no Kibble balance, no silicon sphere. The measured
   value is 2.25234272 x 10^23 Hz. The corpus value is 507 ppm lower. So the
   0.0507 % offset cannot be a geometric correction missing from Kibble-balance
   methodology (ledger A-007, A-009, E-010, E-013, E-015).

2. **The factor 10^19 and the 1 Hz zero-point frequency are tied to the SI
   second.** 22,512 is a geometric count, but 10^19 per second is not derived
   anywhere in the papers (A-014, E-010). The quantum mass m_z equals
   h_geo x (1 Hz) / c^2 exactly (C-003, D-009, E-019). In any other time unit
   these numbers change, so these steps are not unit-independent as written.

3. **The gauge length of exactly 10^-15 m is tied to the SI metre and base
   10.** The papers reach it by rounding to the nearest power of ten
   (B-001, B-002, C-014) and concede the digit match with c holds only in SI
   metres (B-006 to B-009). Measured proton charge radius: 0.84075(64) fm.

4. **h and m_z define each other.** m_z is computed as h_geo/c^2 and h is then
   recovered as m_z c^2 (A-038, A-049, B-011, D-001). The many-digit agreement
   is that round trip.

5. **Dimensionless claims the corpus already makes, scored against data**
   (`corpus_predictions_v0_scored.csv`):

   | Quantity | Corpus | Measured | Difference |
   |---|---|---|---|
   | proton/electron mass ratio (22,500/12) | 1875 | 1836.152673 | +2.1 % |
   | neutron/proton mass ratio | 1 | 1.00137842 | -0.14 % |
   | deuterium/hydrogen atomic mass | 1.999467 | 1.998464 | +0.050 % |
   | helium-4/hydrogen atomic mass | 3.998934 | 3.971526 | +0.69 % |
   | carbon-12/hydrogen atomic mass | 11.996802 | 11.906829 | +0.76 % |
   | oxygen-16/hydrogen atomic mass | 15.995736 | 15.870726 | +0.79 % |
   | iron-56/hydrogen atomic mass | 51.986141 | 55.500641 | -6.3 % |
   | electron mass in atomic mass units (12/22,512) | 5.3305e-4 | 5.4858e-4 | -2.8 % |
   | proton radius / reduced Compton wavelength (1 fm) | 4.752 | 3.9977(30) | +19 % |
   | same, with the proof's 1.1547 fm | 5.487 | 3.9977(30) | +37 % |

   The isotope ratios are close to the mass numbers, so the counts as written
   carry no nuclear binding energy.

6. **The CMB cosine result is a fit.** h_geo x cos(1.824958 deg) reproduces
   6.626070111 x 10^-34, but the angle depends on a solar-motion speed chosen
   for closest agreement (A-064); the papers' own definition gives 13.258 km/s
   and a 34 ppb miss (A-071), and the stated input uncertainties move the
   result by about 275 ppb (A-070).

7. **Yang-Mills papers.** Four different mass-gap expressions appear across
   versions (E-019 to E-029). The only inequality given is for a finite volume
   and tends to zero as the volume grows; two versions say the infinite-volume
   limit is not proved while the CMI versions claim it is (E-031, E-032). No
   version constructs a quantum field theory on R^4 or checks the Wightman
   axioms (E-043).

8. **What holds up.** The equilateral geometry is correct standard
   mathematics: the 60-degree Gram matrix, the A2 Cartan matrix, the oblique
   Laplacian (4/3)(d_xx + d_yy - d_x d_y), the triangle relations and the
   gnomon counting (C-009 to C-011, C-058, C-059, D-041, D-045).

9. **One unit-free relation worth testing properly.** The measured proton
   radius divided by the proton's reduced Compton wavelength is 3.9977(30),
   consistent with exactly 4. This is a known coincidence in the literature and
   is not in the corpus, but it is the kind of pure-number statement a
   geometric framework could try to derive.

## Findings from the uploaded papers (groups F to L)

10. **The open questions are not answered in any of the 29 additional
    files.** Each reader looked for: a unit-independence argument; the origin
    of the 10^19 factor, the 1 Hz quantum and the 10^-15 m gauge; a derivation
    of 22,500 and 12; the proton-neutron difference; a binding-energy term;
    and the reason for the h offset. In every file these are stated as
    premises or not mentioned (ledger entries with topic `open_q:`).
    - Unit independence is argued once, in the corpus outline, Vol I Ch 8
      (F-016): SI units "are human conventions" and the canonical metric
      "arises from equilateral geometry". No number is shown to survive a
      change of units. The outline contains no numerical values at all
      (F-001).
    - The 10^19 factor is attributed to "my calculations based on observable
      rest-mass for electrons and protons" (H-002), i.e. it is calibrated to
      measured masses in SI units.
    - The counts are calibrated so that 22,512 x 10^19 quanta equal one
      atomic mass unit (I-009, I-010, I-019).

11. **Element masses have no binding-energy term.** The rule is
    N = Z x 45,012 quanta (I-027, I-028), which makes every nucleus have
    equal proton and neutron numbers. Iron comes out with 52 nucleons;
    iron-56 is 7 % heavier than the rule gives. One paper proposes storing the
    excess in electron field energy E = E_0 n^2 with E_0 = 4.088 MeV, but the
    value is fitted and marked unconfirmed (I-012, I-013).

12. **The engine constants are internally derived, not measured.** From the
    Quoin Suite stress-test report (K-027 to K-030), reproduced bit for bit:
    - quoin charge q_q = (1.2 x 10^-3)/c^2, so 12 q_q is 25 ppm above the
      elementary charge;
    - electron mass = 12 x 10^19 h/c^2 = 8.8515 x 10^-31 kg, 2.8 % below the
      measured value;
    - Bohr magneton = 9.549 x 10^-24 J/T, 3.0 % above the measured value.
    The 1,527 passing tests check internal consistency (finite values,
    Newton's third law, the r^-3 dipole law, energy conservation in an ideal
    circuit). None compares an output with a measured quantity (K-038 to
    K-041).

13. **Proton radius proof.** The derived radius is (2/sqrt3) x 10^-15 m =
    1.1547 fm, 37 % above the muonic-hydrogen value the paper itself cites
    (K-010). The paper says the measured values fall inside its bounds
    [0.866, 1.1547] fm; 0.8409 fm does not (K-012).

14. **Spectral primes and zeta zeros.**
    - No proof of the Riemann Hypothesis is claimed; the source says so
      (J-008). The operator is defined to have the zeros as its spectrum.
    - The stated regression t_n = 0.497 sqrt(dM^2) + 0.826 with error 0.66
      does not reproduce; it gives 1.69 for the first zero, which is 14.13
      (J-006).
    - The level-difference formula n^2 - m^2 = (n-m)(n+m) is prime only for
      adjacent levels, so the prime lists assigned to spectral series are
      inserted, not derived (J-003, J-009).
    - `zeta_zeros_v36_alpha.csv`: the `gpu_zero` column has mean error 0.227,
      against 0.468 for the standard smooth zero-counting estimate with no
      theory input and about 1.0 for random guesses. That is better than
      either baseline on these 13 rows, but no algorithm is documented, so
      it cannot be attributed to the geometry: a direct scan of the zeta
      function would give the same (J-019). This needs the generating code.
    - `spectral_prime_scaling_results_v14_gpu.csv`: both converging columns
      are the zero-frequency bin of the transform, i.e. the sum of the input
      series, not a resonance (J-021, J-022).

15. **Minkowski-Abraham-Blankenship metric.** The equilateral metric
    [[1, 1/2], [1/2, 1]] is the standard Gram matrix of the hexagonal lattice.
    With a time component it is related to the Minkowski metric by a linear
    change of basis, so it is the same flat geometry in oblique coordinates,
    not a different one (L-017, L-018). A constant metric has zero curvature,
    so the curvature claims do not follow (L-033). The file named "AUDIT" is a
    revised edition of the same paper by the same author; it names no auditor
    and reports no test results (L-001). Neither file contains a number that
    can be compared with measurement (L-040).

16. **Several files are assistant transcripts.** Much of the arithmetic and
    the evaluative wording ("canonical", "valid", "strong move") in groups H,
    I, J and K is an AI assistant's, given without derivation; the ledger
    notes flag these. In the GUFE conversation the result M = I_em is obtained
    by defining M to be the right-hand side (K-057).

## Findings from the atomic topology atlas (group M)

This is the first source in the audit taken from the plates themselves. It
changes two earlier findings, which were drawn from the papers.

17. **The plates carry a different element mass rule from the papers.** The
    printed quanta counts for deuterium to calcium and for xenon are all
    reproduced exactly by one rule (M-001): each deuterium unit in shell
    K, L, M, N, O carries 72 x n^2 quanta with n = 25, 26, 27, 28, 29, plus
    12 for its electron. The Equilateral Geometrics paper instead uses
    Z x 45,012 (I-028). The two disagree from lithium onward (carbon 284,760
    against 270,072).

18. **Scored against measured atomic masses** (most abundant isotope,
    relative to hydrogen-1; `plate_predictions_v0_scored.csv`): 4 of 21 are
    within 1 % (D +0.05 %, He +0.7 %, Ar +0.8 %, Xe -0.7 %); the mean
    deviation is 6.3 %; the range is Li -11.5 % to Ca +13.4 %. Two structural
    reasons: every element is given 2Z nucleons, so neutron excess is absent
    (lithium-7, argon-40 with 22 neutrons), and outer-shell units are made
    heavier, where measured binding makes bound nucleons lighter. The origin
    of the starting index 25 is not given.

19. **The hydrogen plate does use the 1/n^2 pattern**, with base frequency
    3.268969693e15 Hz and ionisation energy 13.525 eV (M-003). Measured:
    3.288051e15 Hz and 13.598 eV. The plate is 0.58 % low. This corrects the
    earlier remark that the level counts go only as n^2: the plates take the
    Rydberg 1/n^2 form as given. What is not shown is how the base frequency
    is obtained from the geometry.

## Internal inconsistencies to resolve before the CAD run

- Quantum mass: 7.376e-51 kg, 7.376e-32 kg and 7.380e-32 kg in three papers
  (I-006, I-020, I-038). The count 22,512 is hydrogen (proton + electron) in
  one paper and the proton alone in another (I-009, I-017).
- Tetryon: three faces or four (F-004, F-005, H-007, H-032); one quantum or
  four (I-045).
- Planck constant: 6.629432672e-34 in most files, 6.62934259e-34 in the
  SQRT 3 paper (I-033).

- Avogadro constant: 6.02214076e23 in one paper, 6.02214157908737e26 in
  another; only the second reproduces h_geo to all digits (A-003, A-009, A-011).
- Proton quanta: 22,500, 22,500 x 10^19, 22,500 x 29,979.2458, and 36 faces
  (D-031, D-070, C-045).
- Geometric proton radius: 1.0, 1.088, 1.1547 and 1.157 fm (B-041, B-046,
  B-047). The stated envelope [0.866, 1.155] fm excludes the measured 0.841 fm
  (B-044, C-043, D-052).
- Reason for the h offset: isotopes and binding energy, a CMB cosine, or a
  2/sqrt(3) Kibble factor (A-010, A-056, E-015). 2/sqrt(3) = 1.1547 is not the
  observed ratio 1.000507.
- Photon energy versus quanta: linear in frequency in some files, quadratic in
  others (B-018, B-021).
- `ProtonProver`: its sampler converges to about 0.8357 fm, outside its own
  CODATA band, and stops when the running mean first crosses the band
  (B-067 to B-069).
- `main.cpp`: rest energy computed as 4 m_n c^4 (D-074).

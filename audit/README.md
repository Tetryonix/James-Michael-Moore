# Tetryonics corpus audit

Working files for auditing the corpus against measured values. Built from the
papers in this repository and in `tetryonics-yang-mills-mass-matter-gap`.
The CAD applications (GRE Studio, particle modeller, element engine) have not
been run yet; everything here comes from the written papers and the code in
these two repositories.

## Files

| File | Contents |
|---|---|
| `claims_ledger.csv` / `.json` | 344 claims extracted from the papers, each with its inputs, output number, an arithmetic recheck, and notes on unit dependence and circularity. Ids: A = Planck constant, B = vacuum frequency and wavelength cutoff, C = gauge invariance, D = wavefunction, summaries and code, E = Yang-Mills papers. |
| `reference/` | Reference tables as dimensionless ratios: CODATA 2022 (138 rows), PDG particle masses (319), AME2020 isotope masses (2939), plus Planck-constant and spectroscopy anchors. `build_reference.py` regenerates them. |
| `compare.py` | Scores a CSV of predictions against the reference tables (relative deviation and pull in standard deviations). |
| `corpus_predictions_v0.csv` | The dimensionless claims the papers already make, in the input format of `compare.py`. `_scored.csv` is the result. |

## How the ledger was made

Five automated readers each read one group of files in full and recomputed
every formula-to-number step. The ledger has not been reviewed line by line.
The numbers quoted in the findings below were recomputed separately.
Rows marked `VERIFY` in `reference/planck_constant_anchors.csv` and
`reference/spectroscopy_anchors.csv` were entered from the literature from
memory because the primary sites were unreachable; check them before relying
on them.

## Ledger statistics

- 344 claims: 75 derivations from postulates, 61 postulates, 58 mathematical
  statements, 52 interpretive, 37 definitions, 34 fits or target-as-input,
  18 testable predictions, 9 numerical coincidences.
- Of 215 claims with a checkable number, 159 reproduce and 56 do not.
- 98 claims are dimensionless.

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

## Internal inconsistencies to resolve before the CAD run

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

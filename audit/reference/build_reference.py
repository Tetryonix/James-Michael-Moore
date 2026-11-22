#!/usr/bin/env python3
"""Build the reference tables used to audit Tetryonics model output.

Everything is expressed as dimensionless ratios wherever possible, so that a
comparison does not depend on the unit system or on the numerical value
assigned to Planck's constant.

Data sources (all bundled in PyPI packages, no network access needed):
  scipy.constants  -> CODATA 2022 recommended values (needs scipy >= 1.15)
  particle         -> Particle Data Group mass table
  periodictable    -> AME2020 atomic masses, CIAAW abundances

Run:  pip install scipy particle periodictable && python3 build_reference.py
"""
import csv, math
import scipy, scipy.constants as sc
from particle import Particle
import particle as _particle
import periodictable as pt

PC = sc.physical_constants


def val(k):
    return PC[k][0]


def unc(k):
    return PC[k][2]


def write(name, header, rows):
    with open(name, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(header)
        w.writerows(rows)
    print(f"{name}: {len(rows)} rows")


# ---------------------------------------------------------------- 1. CODATA
rows = []
for k in sorted(PC):
    v, u, s = PC[k]
    if u == "" and v != 0:
        rows.append([k, repr(v), repr(s), f"{abs(s / v):.2e}" if s else "exact",
                     "CODATA 2022 (scipy %s)" % scipy.__version__])


def derived(key, v, rel, how):
    rows.append([key, repr(v), repr(abs(v) * rel), f"{rel:.2e}", "derived: " + how])


rel = lambda k: unc(k) / abs(val(k))
quad = lambda *ks: math.sqrt(sum(rel(k) ** 2 for k in ks))
rp, a0 = "proton rms charge radius", "Bohr radius"
lcp, lce = "proton Compton wavelength", "Compton wavelength"
derived("proton rms charge radius / proton Compton wavelength",
        val(rp) / val(lcp), quad(rp, lcp), "r_p / (h/(m_p c))")
derived("proton rms charge radius / reduced proton Compton wavelength",
        val(rp) / (val(lcp) / (2 * math.pi)), quad(rp, lcp), "r_p / (hbar/(m_p c))")
derived("proton rms charge radius / Bohr radius",
        val(rp) / val(a0), quad(rp, a0), "r_p / a_0")
derived("proton rms charge radius / classical electron radius",
        val(rp) / val("classical electron radius"),
        quad(rp, "classical electron radius"), "r_p / r_e")
derived("deuteron rms charge radius / proton rms charge radius",
        val("deuteron rms charge radius") / val(rp),
        quad("deuteron rms charge radius", rp), "r_d / r_p")
derived("Bohr radius / Compton wavelength", val(a0) / val(lce),
        rel("fine-structure constant"), "a_0/lambda_C = 1/(2 pi alpha)")
derived("Rydberg constant x Compton wavelength",
        val("Rydberg constant") * val(lce), 2 * rel("fine-structure constant"),
        "R_inf * lambda_C = alpha^2 / 2")
derived("neutron-proton mass difference / electron mass",
        (val("neutron mass") - val("proton mass")) / val("electron mass"),
        unc("neutron-proton mass difference in u") / val("neutron-proton mass difference in u"),
        "(m_n - m_p)/m_e")
derived("gravitational coupling of the proton G m_p^2/(hbar c)",
        val("Newtonian constant of gravitation") * val("proton mass") ** 2
        / (sc.hbar * sc.c), rel("Newtonian constant of gravitation"),
        "G m_p^2/(hbar c)")
write("codata2022_dimensionless.csv",
      ["quantity", "value", "std_uncertainty", "rel_uncertainty", "source"], rows)

# ------------------------------------------- 2. Planck-constant test anchors
# Quantities that pin down h (or combinations of it) by routes that do not
# share systematics.  Rows marked VERIFY were entered from the published
# literature from memory because the primary sites were not reachable from the
# build environment; check them against the cited paper before relying on them.
planck = [
    ["h (SI definition since 20 May 2019)", "6.62607015e-34", "0", "J s",
     "exact by definition; the kilogram is derived from it", "SI Brochure 9th ed.", "ok"],
    ["N_A (SI definition since 20 May 2019)", "6.02214076e23", "0", "1/mol",
     "exact by definition", "SI Brochure 9th ed.", "ok"],
    ["h, CODATA 2017 special adjustment (old SI, kg = IPK)", "6.626070150e-34", "6.9e-42", "J s",
     "least-squares of Kibble balance + Si-sphere results", "Mohr et al., Metrologia 55, 125 (2018)", "VERIFY"],
    ["h, NRC Kibble balance 2017", "6.626070133e-34", "6.0e-42", "J s",
     "electrical vs mechanical power; mass traceable to IPK", "Wood et al., Metrologia 54, 399 (2017)", "VERIFY"],
    ["h, NIST-4 Kibble balance 2017", "6.626069934e-34", "8.9e-42", "J s",
     "electrical vs mechanical power; mass traceable to IPK", "Haddad et al., Metrologia 54, 633 (2017)", "VERIFY"],
    ["N_A, IAC 28Si sphere (XRCD) 2017", "6.022140526e23", "7.0e15", "1/mol",
     "atom counting in a silicon crystal; gives h via N_A h", "Bartl et al., Metrologia 54, 693 (2017)", "VERIFY"],
    ["N_A h (molar Planck constant), CODATA 2014", "3.9903127110e-10", "1.8e-19", "J s/mol",
     "from c alpha^2 A_r(e) M_u / (2 R_inf); independent of any mass artefact", "CODATA 2014", "VERIFY"],
    ["h/m(87Rb) atom recoil", "4.59135925890e-9", "6.5e-19", "m^2/s",
     "atom interferometry; needs only the metre and the second", "Morel et al., Nature 588, 61 (2020)", "VERIFY"],
    ["1/alpha from h/m(87Rb)", "137.035999206", "1.1e-8", "",
     "dimensionless", "Morel et al., Nature 588, 61 (2020)", "VERIFY"],
    ["1/alpha from h/m(133Cs)", "137.035999046", "2.7e-8", "",
     "dimensionless", "Parker et al., Science 360, 191 (2018)", "VERIFY"],
    ["1/alpha CODATA 2022", repr(val("inverse fine-structure constant")),
     repr(unc("inverse fine-structure constant")), "", "dimensionless", "CODATA 2022", "ok"],
    ["Tetryonics claimed h", "6.629432672e-34", "", "J s",
     "relative offset from SI value: %.6e" % (6.629432672e-34 / 6.62607015e-34 - 1),
     "README of this repository", "claim"],
]
write("planck_constant_anchors.csv",
      ["quantity", "value", "std_uncertainty", "unit", "note", "reference", "status"], planck)

# ------------------------------------------------------------------- 3. PDG
me = Particle.from_pdgid(11).mass
mp = Particle.from_pdgid(2212).mass
rows = []
for p in sorted(Particle.all(), key=lambda p: (p.mass or 0)):
    if p.mass is None or int(p.pdgid) <= 0 or int(p.pdgid) >= 1000000000:
        continue
    up, lo = p.mass_upper or 0.0, p.mass_lower or 0.0
    sig = max(up, lo)
    rows.append([p.name, int(p.pdgid), p.three_charge / 3 if p.three_charge is not None else "",
                 repr(p.mass), repr(sig), repr(p.mass / me), repr(p.mass / mp),
                 f"{sig / p.mass:.2e}" if p.mass else ""])
write("pdg_particle_masses.csv",
      ["name", "pdgid", "charge_e", "mass_MeV", "mass_unc_MeV", "mass_over_electron",
       "mass_over_proton", "rel_uncertainty"], rows)
# High-precision mass ratios should be taken from codata2022_dimensionless.csv;
# the PDG ratios above are limited by the MeV uncertainties of both particles.

# --------------------------------------------------------------- 4. AME2020
mH = pt.H[1].mass
mn = val("neutron mass in u")
u_MeV = val("atomic mass constant energy equivalent in MeV")
rows = []
for el in pt.elements:
    if el.number == 0:
        continue
    for A in el.isotopes:
        iso = el[A]
        M = iso.mass
        if not M:
            continue
        Z, N = el.number, A - el.number
        defect = Z * mH + N * mn - M          # in u, atomic-mass convention
        rows.append([Z, el.symbol, A, N, repr(M), repr(getattr(iso, "_mass_unc", "")),
                     repr(iso.abundance), repr(M / A), repr(M / mH),
                     repr(defect / (Z * mH + N * mn)), repr(defect * u_MeV / A)])
write("ame2020_isotope_masses.csv",
      ["Z", "symbol", "A", "N", "atomic_mass_u", "mass_unc_u", "natural_abundance_pct",
       "mass_per_nucleon_u", "mass_over_1H", "binding_fraction", "binding_energy_per_nucleon_MeV"],
      rows)

# ---------------------------------------------------------- 5. Spectroscopy
# Entered from the literature from memory (primary sites unreachable from the
# build environment).  VERIFY against the reference before relying on a row.
spec = [
    ["H 1S-2S (hyperfine centroid)", "2466061413187035", "10", "Hz",
     "Parthey et al., PRL 107, 203001 (2011)", "VERIFY"],
    ["H-D 1S-2S isotope shift", "670994334606", "15", "Hz",
     "Parthey et al., PRL 104, 233001 (2010)", "VERIFY"],
    ["H ground-state hyperfine (21 cm)", "1420405751.768", "0.001", "Hz",
     "Hellwig et al. 1970; Essen et al. 1971", "VERIFY"],
    ["H Lamb shift 2S1/2-2P1/2", "1057.8298e6", "3.2e3", "Hz",
     "Bezginov et al., Science 365, 1007 (2019)", "VERIFY"],
    ["muonic-H Lamb shift 2S(F=1)-2P(F=2)", "49881.35e9", "0.65e9", "Hz",
     "Antognini et al., Science 339, 417 (2013)", "VERIFY"],
    ["Rydberg frequency c R_inf", repr(val("Rydberg constant times c in Hz")),
     repr(unc("Rydberg constant times c in Hz")), "Hz", "CODATA 2022", "ok"],
    ["electron magnetic moment anomaly a_e", repr(val("electron mag. mom. anomaly")),
     repr(unc("electron mag. mom. anomaly")), "", "CODATA 2022", "ok"],
    ["muon magnetic moment anomaly a_mu", repr(val("muon mag. mom. anomaly")),
     repr(unc("muon mag. mom. anomaly")), "", "CODATA 2022", "ok"],
]
cR = val("Rydberg constant times c in Hz")
out = []
for name, v, s, unit, ref, st in spec:
    ratio = repr(float(v) / cR) if unit == "Hz" else ""
    out.append([name, v, s, unit, ratio, ref, st])
write("spectroscopy_anchors.csv",
      ["quantity", "value", "std_uncertainty", "unit", "ratio_to_Rydberg_frequency",
       "reference", "status"], out)
print("PDG table from particle", _particle.__version__, "| periodictable", pt.__version__)

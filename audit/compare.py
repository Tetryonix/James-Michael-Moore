#!/usr/bin/env python3
"""Score a table of model predictions against the reference tables.

Input: a CSV with header  table,key,column,predicted[,pred_uncertainty][,note]
  table   one of: codata | pdg | isotope | planck | spectroscopy
  key     codata/planck/spectroscopy: the exact 'quantity' string
          pdg: particle name (e.g. p, n, mu-) or PDG id
          isotope: symbol-A (e.g. Fe-56)
  column  which reference column to compare with; leave blank for the default
          (codata: value, pdg: mass_over_electron, isotope: mass_over_1H)
  predicted         the model's number, in the same form as the reference column
  pred_uncertainty  optional; leave blank if the model claims an exact value

Output: one row per prediction with the relative deviation and the pull
(deviation / combined standard uncertainty), plus summary statistics.

Usage:  python3 compare.py predictions.csv [results.csv]
"""
import csv, math, os, sys

REF = os.path.join(os.path.dirname(os.path.abspath(__file__)), "reference")
SPEC = {  # table -> (file, key function, default column, uncertainty getter)
    "codata": ("codata2022_dimensionless.csv", lambda r: r["quantity"], "value",
               lambda r, c: float(r["std_uncertainty"] or 0)),
    "planck": ("planck_constant_anchors.csv", lambda r: r["quantity"], "value",
               lambda r, c: float(r["std_uncertainty"] or 0)),
    "spectroscopy": ("spectroscopy_anchors.csv", lambda r: r["quantity"], "value",
                     lambda r, c: float(r["std_uncertainty"] or 0)),
    "pdg": ("pdg_particle_masses.csv", None, "mass_over_electron",
            lambda r, c: float(r[c]) * float(r["rel_uncertainty"] or 0)),
    "isotope": ("ame2020_isotope_masses.csv", lambda r: f'{r["symbol"]}-{r["A"]}', "mass_over_1H",
                lambda r, c: float(r[c]) * float(r["mass_unc_u"] or 0) / float(r["atomic_mass_u"])),
}


def load(table):
    fname, keyf, _, _ = SPEC[table]
    out = {}
    with open(os.path.join(REF, fname), newline="") as f:
        for r in csv.DictReader(f):
            if table == "pdg":
                out[r["name"]] = r
                out[r["pdgid"]] = r
            else:
                out[keyf(r)] = r
    return out


def main(path, outpath=None):
    cache, results = {}, []
    with open(path, newline="") as f:
        for p in csv.DictReader(f):
            t = p["table"].strip()
            if t not in SPEC:
                results.append({**p, "status": "unknown table"})
                continue
            ref = cache.setdefault(t, load(t)).get(p["key"].strip())
            if ref is None:
                results.append({**p, "status": "key not found"})
                continue
            col = (p.get("column") or "").strip() or SPEC[t][2]
            rv, pv = float(ref[col]), float(p["predicted"])
            ru = SPEC[t][3](ref, col)
            pu = float(p.get("pred_uncertainty") or 0)
            sig = math.hypot(ru, pu)
            results.append({**p, "column": col, "reference": rv, "ref_uncertainty": ru,
                            "rel_deviation": (pv - rv) / rv if rv else "",
                            "pull_sigma": (pv - rv) / sig if sig else "",
                            "status": "ok"})
    fields = ["table", "key", "column", "predicted", "pred_uncertainty", "reference",
              "ref_uncertainty", "rel_deviation", "pull_sigma", "status", "note"]
    out = open(outpath, "w", newline="") if outpath else sys.stdout
    w = csv.DictWriter(out, fieldnames=fields, extrasaction="ignore")
    w.writeheader()
    w.writerows(results)
    ok = [r for r in results if r["status"] == "ok" and r["rel_deviation"] != ""]
    if ok:
        devs = sorted(abs(r["rel_deviation"]) for r in ok)
        pulls = [abs(r["pull_sigma"]) for r in ok if r["pull_sigma"] != ""]
        print(f"\n{len(ok)} scored, {len(results) - len(ok)} unmatched", file=sys.stderr)
        print(f"median |relative deviation| = {devs[len(devs) // 2]:.3e}, max = {devs[-1]:.3e}",
              file=sys.stderr)
        if pulls:
            print(f"within 2 sigma: {sum(x <= 2 for x in pulls)}/{len(pulls)}; "
                  f"beyond 5 sigma: {sum(x > 5 for x in pulls)}/{len(pulls)}", file=sys.stderr)


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    main(*sys.argv[1:3])

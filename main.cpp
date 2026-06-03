/*
================================================================================
  TETRYONIC GAME OF LIFE — Triangle Simplex Cellular Automaton
  Based on Tetryonic Theory by Kelvin C. Abraham
  Framework: James M. Moore, Richard A. Blankenship

  Grid:      A2 simplex lattice (Eisenstein integers Z[ω], ω = e^(2πi/3))
  Metric:    g_μν = [[1, 1/2], [1/2, 1]]  (60° equilateral basis)
  2D cells:  ZPF quanta — equilateral triangles, height = l_q = 1e-15 m
  3D matter: Tetryons — regular tetrahedra (4 ZPF faces), Lorentz invariant
  KEM:       Planar EM field gliders (Lorentz variant)
  Matter:    Tetrahedral 3D gliders (Lorentz invariant, same size/shape always)

  Build:     Visual Studio 2019/2022, SDL2 linked
  Controls:  See README section at bottom of file
================================================================================
*/

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <cmath>
#include <vector>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <random>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
// TETRYONIC PHYSICAL CONSTANTS
// ─────────────────────────────────────────────────────────────────────────────
namespace Tetryonics {
    constexpr double h    = 6.629432672e-34;   // J·s  Planck (Tetryonic)
    constexpr double c    = 2.99792458e8;       // m/s
    constexpr double c2   = c * c;
    constexpr double c4   = c2 * c2;
    constexpr double l_q  = 1.0e-15;           // m  gauge length (1 fm)
    constexpr double L    = (2.0 / 1.7320508075688772) * l_q; // (2/√3)·l_q
    constexpr double A_q  = l_q * l_q / 1.7320508075688772;   // l_q²/√3
    constexpr double nu_vac = 2.99792458e23;   // Hz
    constexpr double m_z  = h / c2;            // 7.376e-51 kg  zero-point mass
    constexpr double m_n  = m_z * 1.0e19;      // 7.376e-32 kg  particle scale
    constexpr double SQRT3 = 1.7320508075688772;
    constexpr double INV_SQRT3 = 0.5773502691896258;
    // Matter formula: M₀ = 4m/c²  → 1 Tetryon = 4 ZPF quanta closed into tetra
    constexpr double M0_tetryon = 4.0 * m_n; // kg  (4 × m_n)
    // Geometry
    // Gram matrix (normalized): g = [[1, 0.5],[0.5, 1]]
    // det(g) = 3/4
    // Jacobian: J = [[1, 0.5],[0, √3/2]]
}

// ─────────────────────────────────────────────────────────────────────────────
// DISPLAY CONSTANTS
// ─────────────────────────────────────────────────────────────────────────────
constexpr int SCREEN_W   = 1440;
constexpr int SCREEN_H   = 900;
constexpr int PANEL_W    = 280;     // right info panel width
constexpr int SIM_W      = SCREEN_W - PANEL_W;
constexpr int SIM_H      = SCREEN_H;

// Triangle rendering size (pixels). l_q maps to TRI_H pixels.
constexpr float TRI_H    = 28.0f;   // pixel height of one equilateral triangle
constexpr float TRI_L    = TRI_H * 2.0f / float(Tetryonics::SQRT3); // side length
constexpr float TRI_HALF = TRI_L * 0.5f;

// ─────────────────────────────────────────────────────────────────────────────
// EISENSTEIN / A2 LATTICE COORDINATES
// An equilateral triangle lattice cell is identified by (q, r, up)
//   q, r  = Eisenstein integer axes (60° angle between them)
//   up    = 0 → upward-pointing triangle, 1 → downward-pointing triangle
// Each (q,r) rhombus contains exactly 2 triangles: up=0 and up=1
// ─────────────────────────────────────────────────────────────────────────────
struct TriCoord {
    int q, r;
    int up; // 0 = △ (points up), 1 = ▽ (points down)

    bool operator==(const TriCoord& o) const {
        return q == o.q && r == o.r && up == o.up;
    }
};

struct TriCoordHash {
    size_t operator()(const TriCoord& c) const {
        size_t h = std::hash<int>{}(c.q);
        h ^= std::hash<int>{}(c.r) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<int>{}(c.up) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }
};

// Convert Eisenstein (q,r,up) to screen pixel center
// Eisenstein basis: e1 = (1,0), e2 = (0.5, √3/2)  scaled to TRI_H
SDL_FPoint triToScreen(int q, int r, int up, float offsetX, float offsetY) {
    // Pixel-space basis vectors for the rhombus grid
    float bx1 = TRI_L;
    float by1 = 0.0f;
    float bx2 = TRI_HALF;
    float by2 = TRI_H;

    // Rhombus origin
    float ox = offsetX + q * bx1 + r * bx2;
    float oy = offsetY + q * by1 + r * by2;

    if (up == 0) {
        // Upward triangle: centroid at (TRI_HALF, TRI_H/3) within rhombus
        return { ox + TRI_HALF, oy + TRI_H / 3.0f };
    } else {
        // Downward triangle: centroid at (TRI_HALF, 2*TRI_H/3)
        return { ox + TRI_HALF, oy + 2.0f * TRI_H / 3.0f };
    }
}

// Get the 3 vertex positions for a triangle cell
void triVertices(int q, int r, int up, float offsetX, float offsetY,
                 SDL_FPoint verts[3]) {
    float bx1 = TRI_L;
    float by1 = 0.0f;
    float bx2 = TRI_HALF;
    float by2 = TRI_H;

    float ox = offsetX + q * bx1 + r * bx2;
    float oy = offsetY + q * by1 + r * by2;

    if (up == 0) {
        // △ upward: bottom-left, bottom-right, top
        verts[0] = { ox,              oy + TRI_H };
        verts[1] = { ox + TRI_L,      oy + TRI_H };
        verts[2] = { ox + TRI_HALF,   oy          };
    } else {
        // ▽ downward: top-left, top-right, bottom
        verts[0] = { ox,              oy          };
        verts[1] = { ox + TRI_L,      oy          };
        verts[2] = { ox + TRI_HALF,   oy + TRI_H  };
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CHARGE STATE  (from training file Section 5.1)
// Positive: clockwise EM flux   → +1
// Negative: counter-clockwise   → -1
// Neutral:  balanced            →  0
// ─────────────────────────────────────────────────────────────────────────────
enum class Charge { NEG = -1, NEUTRAL = 0, POS = 1 };

// ─────────────────────────────────────────────────────────────────────────────
// ZPF CELL — single equilateral triangle quantum
// This is the Game of Life unit on the 2D A2 lattice
// ─────────────────────────────────────────────────────────────────────────────
struct ZPFCell {
    bool    alive    = false;
    Charge  charge   = Charge::NEUTRAL;
    int     quanta   = 0;   // n, energy level
    float   age      = 0.0f;  // for visual fade
    bool    nextAlive = false;
    Charge  nextCharge = Charge::NEUTRAL;
};

// ─────────────────────────────────────────────────────────────────────────────
// TETRYON — 3D Matter particle
// Lorentz invariant: same geometry regardless of velocity
// Made of 4 ZPF faces. Position tracked in Eisenstein coordinates (float)
// Velocity in A2-lattice units per tick
// ─────────────────────────────────────────────────────────────────────────────
struct Tetryon {
    float   fq, fr;         // continuous position in A2 lattice
    float   vq, vr;         // velocity (A2 lattice units / tick)
    Charge  charge;         // net charge of 4-face assembly
    int     quanta;         // always 4 for a minimal Tetryon
    bool    active;
    float   spinPhase;      // visual rotation phase (radians)
    float   spinRate;       // rad/tick
    int     id;

    // KEM field energy = M*v²  (Lorentz variant)
    double kemEnergy() const {
        double speed2 = (double)(vq*vq + vr*vr); // in lattice units
        return Tetryonics::M0_tetryon * speed2;
    }

    // Rest matter energy M₀c⁴  (Lorentz invariant)
    double restEnergy() const {
        return Tetryonics::M0_tetryon * Tetryonics::c4;
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// GAME STATE
// ─────────────────────────────────────────────────────────────────────────────
struct GameState {
    // Grid dimensions
    int gridW = 50;
    int gridH = 35;

    // ZPF lattice (2D triangular Game of Life)
    std::unordered_map<TriCoord, ZPFCell, TriCoordHash> cells;

    // 3D Tetryon particles
    std::vector<Tetryon> tetryons;
    int nextTetryonId = 0;

    // Viewport
    float viewOffX = 80.0f;
    float viewOffY = 60.0f;

    // Sim state
    bool  running   = false;
    bool  showGrid  = true;
    bool  showKEM   = true;
    bool  showMatter = true;
    int   tickCount = 0;
    int   speed     = 6;  // ticks per second
    float tickTimer = 0.0f;

    // Edit mode
    enum class Mode { DRAW_ZPF_POS, DRAW_ZPF_NEG, DRAW_ZPF_NEG2,
                      PLACE_TETRYON, ERASE, SELECT };
    Mode editMode = Mode::DRAW_ZPF_POS;

    // Stats
    int aliveCount    = 0;
    int tetryonCount  = 0;
    double totalKEM   = 0.0;

    // Drag pan
    bool  dragging = false;
    int   dragStartX = 0, dragStartY = 0;
    float dragOffStartX = 0, dragOffStartY = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// TETRYONIC GAME OF LIFE RULES
// Derived from Section 3 and 5 of the training file:
//
// A2 lattice neighbors: Each triangle has 3 edge-neighbors (face sharing = strong
//   force) and 6 vertex-neighbors (edge sharing = weak force interaction).
//   But for GoL we use the 3 direct edge-sharing neighbors (face contact = Strong).
//
// Birth/Survival derived from Tetryonic geometry:
//   - A ZPF quantum SURVIVES if it has 2 same-charge face-neighbors
//     (stable pairing like photon = 2 ZPF)
//   - A ZPF quantum DIES if it has 0 or 3 face-neighbors (isolated or overcrowded)
//   - BIRTH occurs in empty cell with exactly 3 alive face-neighbors of matching
//     charge → forms new ZPF from vacuum
//   - 4 aligned ZPF quanta (2 up + 2 down in rhombus) → Tetryon formation event
//     (Matter assembly: M₀ = 4m/c²)
// ─────────────────────────────────────────────────────────────────────────────

// Returns the 3 face-sharing (edge-adjacent) neighbors of a triangle
// (these share a full edge = strong force range)
std::array<TriCoord, 3> faceNeighbors(TriCoord c) {
    if (c.up == 0) {
        // Upward △: shares edges with:
        // bottom edge → (q, r-1, up=1)  ... rhombus below
        // left edge   → (q-1, r, up=1)
        // right edge  → (q, r, up=1)    ... same rhombus, downward partner
        return {{
            { c.q,   c.r - 1, 1 },
            { c.q-1, c.r,     1 },
            { c.q,   c.r,     1 }
        }};
    } else {
        // Downward ▽: shares edges with:
        // top edge    → (q, r, up=0)    ... same rhombus partner
        // right edge  → (q+1, r, up=0)
        // bottom edge → (q, r+1, up=0)
        return {{
            { c.q,   c.r,     0 },
            { c.q+1, c.r,     0 },
            { c.q,   c.r + 1, 0 }
        }};
    }
}

// Returns 6 vertex-sharing (corner-adjacent) neighbors
// (weak force range — edge sharing)
std::array<TriCoord, 6> vertexNeighbors(TriCoord c) {
    if (c.up == 0) {
        return {{
            { c.q+1, c.r-1, 1 },
            { c.q-1, c.r+1, 1 },
            { c.q+1, c.r,   1 },
            { c.q-1, c.r,   0 },
            { c.q,   c.r+1, 0 },  // this might overlap, but fine for GoL
            { c.q,   c.r-2, 1 }
        }};
    } else {
        return {{
            { c.q-1, c.r+1, 0 },
            { c.q+1, c.r-1, 0 },
            { c.q-1, c.r,   1 },
            { c.q+1, c.r,   0 },
            { c.q,   c.r-1, 1 },
            { c.q,   c.r+2, 0 }
        }};
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Apply one Game of Life tick
// ─────────────────────────────────────────────────────────────────────────────
void stepZPF(GameState& gs) {
    // Collect all cells to consider (alive + their neighbors)
    std::unordered_map<TriCoord, bool, TriCoordHash> toCheck;
    for (auto& [coord, cell] : gs.cells) {
        if (cell.alive) {
            toCheck[coord] = true;
            for (auto& nb : faceNeighbors(coord)) toCheck[nb] = true;
        }
    }

    // Compute next state
    std::unordered_map<TriCoord, ZPFCell, TriCoordHash> newCells;

    // Tetryon formation candidates: rhombus quads with 4 alive matching cells
    std::vector<TriCoord> tetFormationCandidates;

    for (auto& [coord, _] : toCheck) {
        auto& cell = gs.cells[coord];
        auto neighbors = faceNeighbors(coord);

        int aliveNeighbors    = 0;
        int posNeighbors      = 0;
        int negNeighbors      = 0;

        for (auto& nc : neighbors) {
            auto it = gs.cells.find(nc);
            if (it != gs.cells.end() && it->second.alive) {
                aliveNeighbors++;
                if (it->second.charge == Charge::POS) posNeighbors++;
                if (it->second.charge == Charge::NEG) negNeighbors++;
            }
        }

        ZPFCell next;
        next.charge = cell.charge;
        next.quanta = cell.quanta;
        next.age    = cell.age;

        if (cell.alive) {
            // SURVIVAL: 2 alive face-neighbors = stable
            if (aliveNeighbors == 2) {
                next.alive = true;
                next.age  += 1.0f;
            }
            // SURVIVAL at 1 neighbor: weak hold
            else if (aliveNeighbors == 1) {
                next.alive = true;
                next.age  = std::max(0.0f, next.age - 0.5f);
            }
            else {
                // DEATH: isolated (0) or overcrowded (3)
                next.alive = false;
                next.age   = 0.0f;
            }
        } else {
            // BIRTH: empty cell with exactly 3 alive face-neighbors
            if (aliveNeighbors == 3) {
                next.alive = true;
                next.age   = 0.0f;
                // Charge from majority of neighbors
                if (posNeighbors > negNeighbors) next.charge = Charge::POS;
                else if (negNeighbors > posNeighbors) next.charge = Charge::NEG;
                else next.charge = Charge::NEUTRAL;
                next.quanta = 1;
            }
            // BIRTH from photon: exactly 2 neighbors, one + one - → neutral photon pair
            else if (aliveNeighbors == 2 && posNeighbors == 1 && negNeighbors == 1) {
                next.alive  = true;
                next.charge = Charge::NEUTRAL;
                next.quanta = 2;
                next.age    = 0.0f;
            }
        }

        if (next.alive) {
            newCells[coord] = next;
            // Check for Tetryon formation: upward △ in same rhombus as its downward ▽ partner
            // and both have 3 more alive face-partners: 4-quantum tetrahedron
            if (coord.up == 0 && next.quanta >= 1) {
                TriCoord partner = { coord.q, coord.r, 1 };
                auto it = newCells.find(partner);
                if (it == newCells.end()) {
                    auto it2 = gs.cells.find(partner);
                    if (it2 != gs.cells.end() && it2->second.alive) {
                        tetFormationCandidates.push_back(coord);
                    }
                }
            }
        }
    }

    // Tetryon formation: if 4 ZPF quanta cluster (2 rhombus pairs), form Tetryon
    // M₀ = 4m/c² — consume 4 alive ZPF cells, spawn 1 Tetryon
    for (auto& base : tetFormationCandidates) {
        TriCoord p1 = { base.q,   base.r,   0 };
        TriCoord p2 = { base.q,   base.r,   1 };
        TriCoord p3 = { base.q+1, base.r,   0 };
        TriCoord p4 = { base.q+1, base.r,   1 };

        bool all4 = true;
        Charge totalCharge = Charge::NEUTRAL;
        int chargeSum = 0;
        for (auto& tc : {p1, p2, p3, p4}) {
            auto it = newCells.find(tc);
            if (it == newCells.end() || !it->second.alive) { all4 = false; break; }
            chargeSum += (int)it->second.charge;
        }

        if (all4 && (gs.tickCount % 3 == 0)) { // throttle formation rate
            // Remove the 4 ZPF quanta
            for (auto& tc : {p1, p2, p3, p4}) newCells.erase(tc);

            // Create Tetryon
            Tetryon t;
            t.fq = (float)base.q + 0.5f;
            t.fr = (float)base.r + 0.5f;
            // Small random velocity (KEM field — Lorentz variant)
            static std::mt19937 rng(42);
            std::uniform_real_distribution<float> vdist(-0.05f, 0.05f);
            t.vq = vdist(rng);
            t.vr = vdist(rng);
            if (chargeSum > 0) t.charge = Charge::POS;
            else if (chargeSum < 0) t.charge = Charge::NEG;
            else t.charge = Charge::NEUTRAL;
            t.quanta   = 4;
            t.active   = true;
            t.spinPhase = 0.0f;
            t.spinRate  = 0.03f + std::abs(t.vq) * 0.1f;
            t.id = gs.nextTetryonId++;
            gs.tetryons.push_back(t);
        }
    }

    gs.cells = std::move(newCells);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tetryon physics step
// Matter is Lorentz invariant — topology unchanged
// KEM field = M*v² changes with velocity (Lorentz variant)
// ─────────────────────────────────────────────────────────────────────────────
void stepTetryons(GameState& gs) {
    for (auto& t : gs.tetryons) {
        if (!t.active) continue;

        // Move (simple Euler; topology invariant)
        t.fq += t.vq;
        t.fr += t.vr;

        // Wrap at grid boundaries
        if (t.fq < 0)      { t.fq += gs.gridW; }
        if (t.fq >= gs.gridW) { t.fq -= gs.gridW; }
        if (t.fr < 0)      { t.fr += gs.gridH; }
        if (t.fr >= gs.gridH) { t.fr -= gs.gridH; }

        // Spin (visual representation of QAM)
        t.spinPhase += t.spinRate;
        if (t.spinPhase > 2.0f * float(M_PI)) t.spinPhase -= 2.0f * float(M_PI);

        // Tetryon-Tetryon interaction: face-to-face strong force
        // (attractive for opposite charge, repulsive for same)
        for (auto& other : gs.tetryons) {
            if (&other == &t || !other.active) continue;
            float dq = other.fq - t.fq;
            float dr = other.fr - t.fr;
            // Use Gram metric distance: ||v||² = g_μν v^μ v^ν = q²+qr+r²
            float dist2 = dq*dq + dq*dr + dr*dr;
            if (dist2 < 0.01f) dist2 = 0.01f;
            float dist = std::sqrt(dist2);

            // Only interact within L range (strong force cutoff from Section 5.2)
            if (dist < 2.5f) {
                float forceMag = 0.002f / dist2;
                float sign = ((int)t.charge * (int)other.charge < 0) ? +1.0f : -1.0f;
                // Force direction in A2 space
                t.vq += sign * forceMag * dq / dist;
                t.vr += sign * forceMag * dr / dist;
            }
        }

        // Velocity damping (energy loss into ZPF vacuum)
        t.vq *= 0.998f;
        t.vr *= 0.998f;

        // Tetryon → ZPF decay: if barely moving, deposit back as ZPF
        float speed = std::sqrt(t.vq*t.vq + t.vr*t.vr);
        if (speed < 0.001f && (gs.tickCount % 20 == 0)) {
            // Deposit 4 ZPF quanta back
            int qi = (int)std::round(t.fq);
            int ri = (int)std::round(t.fr);
            for (int du = 0; du <= 1; ++du) {
                for (int dv = 0; dv <= 1; ++dv) {
                    TriCoord tc{ qi + du, ri + dv, du==dv ? 0 : 1 };
                    ZPFCell cell;
                    cell.alive  = true;
                    cell.charge = t.charge;
                    cell.quanta = 1;
                    cell.age    = 0.0f;
                    gs.cells[tc] = cell;
                }
            }
            t.active = false;
        }
    }

    // Remove dead tetryons
    gs.tetryons.erase(
        std::remove_if(gs.tetryons.begin(), gs.tetryons.end(),
                       [](const Tetryon& t){ return !t.active; }),
        gs.tetryons.end());
}

// ─────────────────────────────────────────────────────────────────────────────
// PRESETS — famous Game of Life patterns adapted to A2 lattice
// ─────────────────────────────────────────────────────────────────────────────
void clearGrid(GameState& gs) {
    gs.cells.clear();
    gs.tetryons.clear();
    gs.tickCount = 0;
}

void placeGlider(GameState& gs, int q0, int r0, Charge ch) {
    // KEM field glider: minimal moving pattern on A2 triangular lattice
    // 5-cell glider adapted for equilateral triangle topology
    std::vector<std::pair<int,int>> upTris = {
        {q0, r0}, {q0+1, r0}, {q0, r0+1}
    };
    std::vector<std::pair<int,int>> downTris = {
        {q0, r0}, {q0, r0+1}
    };
    for (auto [q,r] : upTris) {
        ZPFCell c; c.alive=true; c.charge=ch; c.quanta=1;
        gs.cells[{q,r,0}] = c;
    }
    for (auto [q,r] : downTris) {
        ZPFCell c; c.alive=true; c.charge=ch; c.quanta=1;
        gs.cells[{q,r,1}] = c;
    }
}

void placePhoton(GameState& gs, int q0, int r0) {
    // Photon = 2 ZPF quanta, + and -, rhombus geometry (diamond)
    ZPFCell pos; pos.alive=true; pos.charge=Charge::POS; pos.quanta=2;
    ZPFCell neg; neg.alive=true; neg.charge=Charge::NEG; neg.quanta=2;
    gs.cells[{q0, r0, 0}] = pos;
    gs.cells[{q0, r0, 1}] = neg;
}

void placeTetryon(GameState& gs, float fq, float fr, Charge ch) {
    Tetryon t;
    t.fq = fq; t.fr = fr;
    static std::mt19937 rng(12345);
    std::uniform_real_distribution<float> vd(-0.04f, 0.04f);
    t.vq = vd(rng); t.vr = vd(rng);
    t.charge = ch;
    t.quanta = 4;
    t.active = true;
    t.spinPhase = 0.0f;
    t.spinRate  = 0.025f;
    t.id = gs.nextTetryonId++;
    gs.tetryons.push_back(t);
}

void loadPreset(GameState& gs, int preset) {
    clearGrid(gs);
    switch(preset) {
    case 0: // ZPF sea — random ZPF vacuum noise
        {
            std::mt19937 rng(42);
            std::bernoulli_distribution alive(0.28);
            std::uniform_int_distribution<int> cdist(0,2);
            for (int q = 2; q < gs.gridW-2; ++q) {
                for (int r = 2; r < gs.gridH-2; ++r) {
                    for (int up = 0; up <= 1; ++up) {
                        if (alive(rng)) {
                            ZPFCell c;
                            c.alive  = true;
                            int cv = cdist(rng);
                            c.charge = cv==0 ? Charge::POS : cv==1 ? Charge::NEG : Charge::NEUTRAL;
                            c.quanta = 1;
                            gs.cells[{q,r,up}] = c;
                        }
                    }
                }
            }
        }
        break;
    case 1: // Glider pair — KEM field gliders
        placeGlider(gs, 5,  5,  Charge::POS);
        placeGlider(gs, 15, 10, Charge::NEG);
        placeGlider(gs, 25, 5,  Charge::POS);
        break;
    case 2: // Photon lattice — photons in diamond formation
        for (int i = 0; i < 6; ++i)
            for (int j = 0; j < 4; ++j)
                placePhoton(gs, 5 + i*6, 5 + j*5);
        break;
    case 3: // Tetryon matter — 3D particles (Lorentz invariant)
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 3; ++j) {
                Charge ch = ((i+j)%2==0) ? Charge::POS : Charge::NEG;
                placeTetryon(gs, 8.0f+i*8.0f, 8.0f+j*6.0f, ch);
            }
        break;
    case 4: // Mixed — ZPF + Tetryons interacting
        placeGlider(gs, 5, 5, Charge::POS);
        placeGlider(gs, 20, 5, Charge::NEG);
        placeTetryon(gs, 12.0f, 12.0f, Charge::NEUTRAL);
        placeTetryon(gs, 16.0f, 8.0f,  Charge::POS);
        for (int i = 3; i < 30; i += 4)
            placePhoton(gs, i, 20);
        break;
    case 5: // Stable ring — 6 ZPF quanta around a hexagonal vertex (6×60°=360°)
        {
            int q0=15, r0=12;
            // 6 triangles meeting at vertex — vacuum closure proof
            std::vector<TriCoord> ring = {
                {q0,   r0,   0}, {q0,   r0,   1},
                {q0+1, r0,   0}, {q0-1, r0+1, 1},
                {q0,   r0+1, 0}, {q0,   r0-1, 1}
            };
            Charge charges[] = { Charge::POS, Charge::NEG, Charge::POS,
                                  Charge::NEG, Charge::POS, Charge::NEG };
            for (int i = 0; i < 6; ++i) {
                ZPFCell c; c.alive=true; c.charge=charges[i]; c.quanta=1;
                gs.cells[ring[i]] = c;
            }
        }
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RENDERING
// ─────────────────────────────────────────────────────────────────────────────

// SDL doesn't have filled triangle — implement with scanline or just 3 lines + fill
void drawFilledTriangle(SDL_Renderer* ren, SDL_FPoint v[3], SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    // Sort vertices by y
    SDL_FPoint p[3] = {v[0], v[1], v[2]};
    if (p[0].y > p[1].y) std::swap(p[0], p[1]);
    if (p[1].y > p[2].y) std::swap(p[1], p[2]);
    if (p[0].y > p[1].y) std::swap(p[0], p[1]);

    auto lerp = [](float a, float b, float t){ return a + (b-a)*t; };

    float totalH = p[2].y - p[0].y;
    for (float y = p[0].y; y <= p[2].y; y += 1.0f) {
        float tFull = (totalH > 0) ? (y - p[0].y) / totalH : 0.0f;
        float xA = lerp(p[0].x, p[2].x, tFull);
        float xB;
        if (y < p[1].y) {
            float segH = p[1].y - p[0].y;
            float t = (segH > 0) ? (y - p[0].y) / segH : 0.0f;
            xB = lerp(p[0].x, p[1].x, t);
        } else {
            float segH = p[2].y - p[1].y;
            float t = (segH > 0) ? (y - p[1].y) / segH : 0.0f;
            xB = lerp(p[1].x, p[2].x, t);
        }
        if (xA > xB) std::swap(xA, xB);
        SDL_RenderDrawLineF(ren, xA, y, xB, y);
    }
}

void drawTriangleOutline(SDL_Renderer* ren, SDL_FPoint v[3], SDL_Color col) {
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
    SDL_RenderDrawLineF(ren, v[0].x, v[0].y, v[1].x, v[1].y);
    SDL_RenderDrawLineF(ren, v[1].x, v[1].y, v[2].x, v[2].y);
    SDL_RenderDrawLineF(ren, v[2].x, v[2].y, v[0].x, v[0].y);
}

// Draw a tetrahedron projected onto screen (3 visible faces of a regular tetra)
// Lorentz invariant: same size always (l_q height → TRI_H pixels)
void drawTetryon(SDL_Renderer* ren, float screenX, float screenY,
                 float spinPhase, Charge charge, float scale=1.0f) {
    float r = TRI_H * 0.9f * scale;

    // 3 vertices of projected tetrahedron (Schlegel-style projection)
    // Regular tetrahedron 3 bottom vertices + 1 apex
    // For 2D projection: show 3 outer triangles around center
    SDL_FPoint cx = {screenX, screenY};

    float angles[3] = {
        spinPhase,
        spinPhase + 2.0f * float(M_PI) / 3.0f,
        spinPhase + 4.0f * float(M_PI) / 3.0f
    };

    SDL_FPoint outer[3];
    for (int i = 0; i < 3; ++i) {
        outer[i] = {
            cx.x + r * std::cos(angles[i]),
            cx.y + r * std::sin(angles[i])
        };
    }

    // Colors from charge state
    SDL_Color faceColors[3];
    if (charge == Charge::POS) {
        faceColors[0] = {255, 120, 60,  200};
        faceColors[1] = {200, 80,  40,  180};
        faceColors[2] = {150, 50,  30,  160};
    } else if (charge == Charge::NEG) {
        faceColors[0] = {60,  120, 255, 200};
        faceColors[1] = {40,  80,  200, 180};
        faceColors[2] = {30,  50,  150, 160};
    } else {
        faceColors[0] = {100, 220, 100, 200};
        faceColors[1] = {60,  160, 60,  180};
        faceColors[2] = {40,  120, 40,  160};
    }

    // Draw 3 projected faces
    for (int i = 0; i < 3; ++i) {
        SDL_FPoint face[3] = {
            cx,
            outer[i],
            outer[(i+1)%3]
        };
        drawFilledTriangle(ren, face, faceColors[i]);
    }

    // Draw edges
    SDL_SetRenderDrawColor(ren, 255, 255, 200, 220);
    for (int i = 0; i < 3; ++i) {
        SDL_RenderDrawLineF(ren, cx.x, cx.y, outer[i].x, outer[i].y);
        SDL_RenderDrawLineF(ren, outer[i].x, outer[i].y,
                            outer[(i+1)%3].x, outer[(i+1)%3].y);
    }

    // Apex dot
    SDL_Rect dot = {(int)(cx.x-2), (int)(cx.y-2), 5, 5};
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderFillRect(ren, &dot);
}

void renderGrid(SDL_Renderer* ren, const GameState& gs) {
    // Draw grid lines first (A2 lattice)
    if (gs.showGrid) {
        SDL_SetRenderDrawColor(ren, 20, 28, 40, 255);
        for (int q = 0; q < gs.gridW; ++q) {
            for (int r = 0; r < gs.gridH; ++r) {
                for (int up = 0; up <= 1; ++up) {
                    SDL_FPoint verts[3];
                    triVertices(q, r, up, gs.viewOffX, gs.viewOffY, verts);

                    // Clip to sim area
                    bool visible = false;
                    for (auto& v : verts)
                        if (v.x >= 0 && v.x < SIM_W && v.y >= 0 && v.y < SIM_H)
                            visible = true;
                    if (!visible) continue;

                    SDL_SetRenderDrawColor(ren, 18, 28, 45, 255);
                    SDL_RenderDrawLineF(ren, verts[0].x, verts[0].y, verts[1].x, verts[1].y);
                    SDL_RenderDrawLineF(ren, verts[1].x, verts[1].y, verts[2].x, verts[2].y);
                    SDL_RenderDrawLineF(ren, verts[2].x, verts[2].y, verts[0].x, verts[0].y);
                }
            }
        }
    }

    // Draw alive ZPF cells
    for (auto& [coord, cell] : gs.cells) {
        if (!cell.alive) continue;

        SDL_FPoint verts[3];
        triVertices(coord.q, coord.r, coord.up, gs.viewOffX, gs.viewOffY, verts);

        // Clip
        bool visible = false;
        for (auto& v : verts)
            if (v.x >= -TRI_L && v.x < SIM_W+TRI_L && v.y >= -TRI_H && v.y < SIM_H+TRI_H)
                visible = true;
        if (!visible) continue;

        // Age-based brightness
        float brightness = std::min(1.0f, 0.4f + cell.age * 0.04f);

        SDL_Color fillColor;
        if (cell.charge == Charge::POS) {
            fillColor = {
                Uint8(std::min(255.0f, 255 * brightness)),
                Uint8(std::min(255.0f, 80  * brightness)),
                Uint8(std::min(255.0f, 30  * brightness)),
                200
            };
        } else if (cell.charge == Charge::NEG) {
            fillColor = {
                Uint8(std::min(255.0f, 30  * brightness)),
                Uint8(std::min(255.0f, 100 * brightness)),
                Uint8(std::min(255.0f, 255 * brightness)),
                200
            };
        } else {
            // Neutral: photon (2 quanta) = cyan
            fillColor = {
                Uint8(std::min(255.0f, 40  * brightness)),
                Uint8(std::min(255.0f, 220 * brightness)),
                Uint8(std::min(255.0f, 200 * brightness)),
                200
            };
        }

        drawFilledTriangle(ren, verts, fillColor);

        // Outline
        SDL_Color outline = {
            Uint8(std::min(255, (int)fillColor.r + 40)),
            Uint8(std::min(255, (int)fillColor.g + 40)),
            Uint8(std::min(255, (int)fillColor.b + 40)),
            255
        };
        drawTriangleOutline(ren, verts, outline);

        // Quanta indicator (dot for n>1)
        if (cell.quanta > 1) {
            SDL_FPoint center = triToScreen(coord.q, coord.r, coord.up,
                                            gs.viewOffX, gs.viewOffY);
            SDL_Rect dot = {(int)(center.x-2), (int)(center.y-2), 4, 4};
            SDL_SetRenderDrawColor(ren, 255, 255, 100, 255);
            SDL_RenderFillRect(ren, &dot);
        }
    }

    // Draw Tetryons (3D matter — always same shape/size = Lorentz invariant)
    if (gs.showMatter) {
        for (auto& t : gs.tetryons) {
            if (!t.active) continue;

            // Convert A2 position to screen
            float bx1 = TRI_L,       by1 = 0.0f;
            float bx2 = TRI_HALF,    by2 = TRI_H;
            float sx = gs.viewOffX + t.fq * bx1 + t.fr * bx2;
            float sy = gs.viewOffY + t.fq * by1 + t.fr * by2;

            if (sx < -TRI_H*2 || sx > SIM_W+TRI_H*2 ||
                sy < -TRI_H*2 || sy > SIM_H+TRI_H*2) continue;

            drawTetryon(ren, sx, sy, t.spinPhase, t.charge);

            // KEM field indicator: circle showing kinetic energy (Lorentz variant)
            if (gs.showKEM) {
                float speed = std::sqrt(t.vq*t.vq + t.vr*t.vr);
                int kemRad = (int)(speed * 500.0f);
                if (kemRad > 2) {
                    kemRad = std::min(kemRad, (int)(TRI_H * 2));
                    SDL_SetRenderDrawColor(ren, 255, 200, 50, 80);
                    // Draw KEM circle approximation (octagon)
                    for (int i = 0; i < 8; ++i) {
                        float a1 = i * float(M_PI) / 4.0f;
                        float a2 = (i+1) * float(M_PI) / 4.0f;
                        SDL_RenderDrawLineF(ren,
                            sx + kemRad * std::cos(a1),
                            sy + kemRad * std::sin(a1),
                            sx + kemRad * std::cos(a2),
                            sy + kemRad * std::sin(a2));
                    }
                }
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TEXT RENDERING (minimal, using SDL_ttf if available, else skip)
// ─────────────────────────────────────────────────────────────────────────────
void renderText(SDL_Renderer* ren, TTF_Font* font, const std::string& text,
                int x, int y, SDL_Color col) {
    if (!font) return;
    SDL_Surface* surf = TTF_RenderText_Blended(font, text.c_str(), col);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(ren, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void renderPanel(SDL_Renderer* ren, TTF_Font* font, TTF_Font* fontSmall,
                 const GameState& gs) {
    // Panel background
    SDL_Rect panel = {SIM_W, 0, PANEL_W, SCREEN_H};
    SDL_SetRenderDrawColor(ren, 8, 10, 18, 255);
    SDL_RenderFillRect(ren, &panel);
    SDL_SetRenderDrawColor(ren, 25, 35, 55, 255);
    SDL_RenderDrawLine(ren, SIM_W, 0, SIM_W, SCREEN_H);

    if (!font) return;

    SDL_Color white  = {220, 220, 240, 255};
    SDL_Color cyan   = {80,  200, 220, 255};
    SDL_Color gold   = {255, 200, 80,  255};
    SDL_Color red    = {255, 80,  80,  255};
    SDL_Color blue   = {80,  120, 255, 255};
    SDL_Color green  = {80,  220, 100, 255};
    SDL_Color muted  = {80,  90,  120, 255};

    int px = SIM_W + 12;
    int py = 12;
    int lineH = 18;

    renderText(ren, font, "TETRYONICS", px, py, cyan);
    py += lineH + 4;
    renderText(ren, fontSmall, "Triangle Game of Life", px, py, muted);
    py += lineH;
    renderText(ren, fontSmall, "A2 Eisenstein Lattice", px, py, muted);
    py += lineH + 8;

    // Divider
    SDL_SetRenderDrawColor(ren, 30, 45, 70, 255);
    SDL_RenderDrawLine(ren, px, py, SIM_W+PANEL_W-12, py);
    py += 6;

    // Stats
    renderText(ren, fontSmall, "SIMULATION", px, py, gold);
    py += lineH;

    auto statLine = [&](const std::string& label, const std::string& val,
                        SDL_Color vc = white) {
        renderText(ren, fontSmall, label, px, py, muted);
        renderText(ren, fontSmall, val, px + 120, py, vc);
        py += lineH;
    };

    statLine("Tick:", std::to_string(gs.tickCount));
    statLine("ZPF alive:", std::to_string(gs.aliveCount), green);
    statLine("Tetryons:", std::to_string(gs.tetryonCount), gold);

    std::ostringstream kem;
    kem << std::scientific << std::setprecision(2) << gs.totalKEM;
    statLine("KEM:", kem.str(), {255,200,50,255});
    statLine("Speed:", std::to_string(gs.speed) + " t/s");
    statLine("State:", gs.running ? "RUNNING" : "PAUSED",
             gs.running ? green : red);

    py += 6;
    SDL_SetRenderDrawColor(ren, 30, 45, 70, 255);
    SDL_RenderDrawLine(ren, px, py, SIM_W+PANEL_W-12, py);
    py += 6;

    // Physics constants
    renderText(ren, fontSmall, "CONSTANTS", px, py, gold);
    py += lineH;

    auto constLine = [&](const std::string& label, const std::string& val) {
        renderText(ren, fontSmall, label, px, py, muted);
        renderText(ren, fontSmall, val, px + 80, py, cyan);
        py += lineH;
    };

    constLine("h =", "6.629e-34 J·s");
    constLine("l_q =", "1e-15 m (1 fm)");
    constLine("L =", "1.1547e-15 m");
    constLine("M0 =", "4m/c² (Tetryon)");
    constLine("m_z =", "7.376e-51 kg");
    constLine("c =", "2.998e8 m/s");

    py += 4;
    SDL_SetRenderDrawColor(ren, 30, 45, 70, 255);
    SDL_RenderDrawLine(ren, px, py, SIM_W+PANEL_W-12, py);
    py += 6;

    // Edit mode
    renderText(ren, fontSmall, "DRAW MODE", px, py, gold);
    py += lineH;

    struct ModeLabel { GameState::Mode m; const char* label; SDL_Color col; };
    ModeLabel modes[] = {
        { GameState::Mode::DRAW_ZPF_POS,  "[1] ZPF +",     {255,100,60,255}  },
        { GameState::Mode::DRAW_ZPF_NEG,  "[2] ZPF -",     {60,100,255,255}  },
        { GameState::Mode::DRAW_ZPF_NEG2, "[3] ZPF 0",     {60,220,200,255}  },
        { GameState::Mode::PLACE_TETRYON, "[4] Tetryon",   {255,200,50,255}  },
        { GameState::Mode::ERASE,         "[5] Erase",     {180,60,60,255}   },
    };
    for (auto& ml : modes) {
        SDL_Color col = (gs.editMode == ml.m) ? white : muted;
        if (gs.editMode == ml.m) {
            SDL_Rect hl = {px-2, py-1, 130, lineH};
            SDL_SetRenderDrawColor(ren, 25, 40, 70, 255);
            SDL_RenderFillRect(ren, &hl);
            col = ml.col;
        }
        renderText(ren, fontSmall, ml.label, px, py, col);
        py += lineH;
    }

    py += 4;
    SDL_SetRenderDrawColor(ren, 30, 45, 70, 255);
    SDL_RenderDrawLine(ren, px, py, SIM_W+PANEL_W-12, py);
    py += 6;

    // Controls
    renderText(ren, fontSmall, "CONTROLS", px, py, gold);
    py += lineH;

    std::vector<std::pair<std::string,std::string>> controls = {
        {"SPACE",  "pause/run"},
        {"C",      "clear"},
        {"G",      "toggle grid"},
        {"K",      "toggle KEM"},
        {"M",      "toggle matter"},
        {"+/-",    "speed"},
        {"P0-P5",  "presets"},
        {"LMB",    "draw/place"},
        {"RMB+drag","pan view"},
        {"R",      "reset view"},
    };
    for (auto& [key,desc] : controls) {
        renderText(ren, fontSmall, key, px, py, cyan);
        renderText(ren, fontSmall, desc, px+50, py, muted);
        py += lineH - 2;
    }

    py += 4;
    SDL_SetRenderDrawColor(ren, 30, 45, 70, 255);
    SDL_RenderDrawLine(ren, px, py, SIM_W+PANEL_W-12, py);
    py += 6;

    // Legend
    renderText(ren, fontSmall, "LEGEND", px, py, gold);
    py += lineH;

    // ZPF + dot
    SDL_Rect zd = {px, py+4, 10, 10};
    SDL_SetRenderDrawColor(ren, 255, 80, 30, 220);
    SDL_RenderFillRect(ren, &zd);
    renderText(ren, fontSmall, "  ZPF+ (pos charge)", px+2, py, {200,100,80,255});
    py += lineH;

    SDL_SetRenderDrawColor(ren, 30, 80, 255, 220);
    SDL_RenderFillRect(ren, &zd);
    renderText(ren, fontSmall, "  ZPF- (neg charge)", px+2, py, {80,100,200,255});
    py += lineH;

    SDL_SetRenderDrawColor(ren, 40, 200, 180, 220);
    SDL_RenderFillRect(ren, &zd);
    renderText(ren, fontSmall, "  ZPF0 (neutral/photon)", px+2, py, {80,200,180,255});
    py += lineH;

    // Tetryon mini icon
    SDL_SetRenderDrawColor(ren, 255,180,50,220);
    SDL_RenderFillRect(ren, &zd);
    renderText(ren, fontSmall, "  Tetryon (3D Matter)", px+2, py, {255,200,80,255});
    py += lineH;

    // KEM circle
    SDL_SetRenderDrawColor(ren, 255,200,50,120);
    for (int i = 0; i < 8; ++i) {
        float a1 = i*float(M_PI)/4.0f, a2=(i+1)*float(M_PI)/4.0f;
        SDL_RenderDrawLineF(ren, px+5+6*std::cos(a1), py+8+6*std::sin(a1),
                                 px+5+6*std::cos(a2), py+8+6*std::sin(a2));
    }
    renderText(ren, fontSmall, "  KEM field (Lorentz var)", px+2, py, {200,160,60,255});
    py += lineH + 4;

    // Physics footnote
    renderText(ren, fontSmall, "M=4m/c² · KEM=Mv²", px, py, muted);
    py += lineH;
    renderText(ren, fontSmall, "E=Mc4+(γ-1)mc²", px, py, muted);
}

// ─────────────────────────────────────────────────────────────────────────────
// HIT TEST — which triangle is under the mouse?
// ─────────────────────────────────────────────────────────────────────────────
TriCoord screenToTri(int mx, int my, float offX, float offY) {
    // Invert A2 transform:
    // screen = off + q*(TRI_L,0) + r*(TRI_HALF, TRI_H)
    // → subtract offset, divide by basis
    float fx = mx - offX;
    float fy = my - offY;

    // Integer rhombus
    // r ≈ fy / TRI_H
    // q ≈ (fx - r*TRI_HALF) / TRI_L
    float fr = fy / TRI_H;
    float fq = (fx - fr * TRI_HALF) / TRI_L;

    int q = (int)std::floor(fq);
    int r = (int)std::floor(fr);

    // Determine up/down within rhombus
    float localX = fx - q * TRI_L - r * TRI_HALF;
    float localY = fy - r * TRI_H;

    // In the rhombus, the diagonal goes from (0,TRI_H) to (TRI_L,0)
    // Points above the diagonal → up=0 (upward △)
    // Points below → up=1 (downward ▽)
    // Diagonal line: y = TRI_H - (TRI_H/TRI_L)*x
    float diagY = TRI_H - (TRI_H / TRI_L) * localX;
    int up = (localY < diagY) ? 0 : 1;

    return {q, r, up};
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow(
        "Tetryonic Game of Life — A2 Simplex Triangle Automaton",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);

    SDL_Renderer* ren = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    // Font loading — try common system paths
    TTF_Font* font      = nullptr;
    TTF_Font* fontSmall = nullptr;

    const char* fontPaths[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/System/Library/Fonts/Menlo.ttc",
        nullptr
    };
    for (int i = 0; fontPaths[i] && !font; ++i) {
        font      = TTF_OpenFont(fontPaths[i], 14);
        fontSmall = TTF_OpenFont(fontPaths[i], 11);
    }

    GameState gs;
    gs.gridW = 52;
    gs.gridH = 36;
    loadPreset(gs, 0); // Start with random ZPF sea

    Uint32 lastTick = SDL_GetTicks();
    float  tickAccum = 0.0f;

    bool quit = false;
    SDL_Event ev;

    while (!quit) {
        Uint32 now = SDL_GetTicks();
        float dt = (now - lastTick) / 1000.0f;
        lastTick = now;
        dt = std::min(dt, 0.05f);

        // ── Events ─────────────────────────────────────────────────────────
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { quit = true; break; }

            if (ev.type == SDL_KEYDOWN) {
                switch (ev.key.keysym.sym) {
                case SDLK_SPACE: gs.running = !gs.running; break;
                case SDLK_c:     clearGrid(gs); break;
                case SDLK_g:     gs.showGrid    = !gs.showGrid;   break;
                case SDLK_k:     gs.showKEM     = !gs.showKEM;    break;
                case SDLK_m:     gs.showMatter  = !gs.showMatter; break;
                case SDLK_r:     gs.viewOffX = 80.0f; gs.viewOffY = 60.0f; break;
                case SDLK_1:     gs.editMode = GameState::Mode::DRAW_ZPF_POS;  break;
                case SDLK_2:     gs.editMode = GameState::Mode::DRAW_ZPF_NEG;  break;
                case SDLK_3:     gs.editMode = GameState::Mode::DRAW_ZPF_NEG2; break;
                case SDLK_4:     gs.editMode = GameState::Mode::PLACE_TETRYON; break;
                case SDLK_5:     gs.editMode = GameState::Mode::ERASE;         break;
                case SDLK_EQUALS: case SDLK_PLUS:
                    gs.speed = std::min(gs.speed + 1, 30); break;
                case SDLK_MINUS:
                    gs.speed = std::max(gs.speed - 1, 1); break;
                case SDLK_p:
                    {
                        // Check for 0-5 following (use scan code)
                        int preset = -1;
                        switch(ev.key.keysym.sym) { default: break; }
                    }
                    break;
                // Preset keys: F1-F6
                case SDLK_F1: loadPreset(gs, 0); break;
                case SDLK_F2: loadPreset(gs, 1); break;
                case SDLK_F3: loadPreset(gs, 2); break;
                case SDLK_F4: loadPreset(gs, 3); break;
                case SDLK_F5: loadPreset(gs, 4); break;
                case SDLK_F6: loadPreset(gs, 5); break;
                case SDLK_ESCAPE: quit = true;   break;
                }
            }

            // Mouse draw
            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                int mx = ev.button.x, my = ev.button.y;
                if (ev.button.button == SDL_BUTTON_RIGHT) {
                    gs.dragging = true;
                    gs.dragStartX = mx; gs.dragStartY = my;
                    gs.dragOffStartX = gs.viewOffX;
                    gs.dragOffStartY = gs.viewOffY;
                }
                if (ev.button.button == SDL_BUTTON_LEFT && mx < SIM_W) {
                    TriCoord tc = screenToTri(mx, my, gs.viewOffX, gs.viewOffY);
                    switch (gs.editMode) {
                    case GameState::Mode::DRAW_ZPF_POS: {
                        ZPFCell c; c.alive=true; c.charge=Charge::POS; c.quanta=1;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::DRAW_ZPF_NEG: {
                        ZPFCell c; c.alive=true; c.charge=Charge::NEG; c.quanta=1;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::DRAW_ZPF_NEG2: {
                        ZPFCell c; c.alive=true; c.charge=Charge::NEUTRAL; c.quanta=2;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::PLACE_TETRYON: {
                        SDL_FPoint sc = triToScreen(tc.q, tc.r, tc.up,
                                                    gs.viewOffX, gs.viewOffY);
                        // Convert screen back to A2 float
                        float fq = (float)tc.q + 0.5f;
                        float fr = (float)tc.r + 0.5f;
                        placeTetryon(gs, fq, fr, Charge::NEUTRAL);
                        break; }
                    case GameState::Mode::ERASE:
                        gs.cells.erase(tc); break;
                    default: break;
                    }
                }
            }
            if (ev.type == SDL_MOUSEBUTTONUP) {
                if (ev.button.button == SDL_BUTTON_RIGHT) gs.dragging = false;
            }
            if (ev.type == SDL_MOUSEMOTION) {
                if (gs.dragging) {
                    gs.viewOffX = gs.dragOffStartX + (ev.motion.x - gs.dragStartX);
                    gs.viewOffY = gs.dragOffStartY + (ev.motion.y - gs.dragStartY);
                }
                // Drag draw
                if ((ev.motion.state & SDL_BUTTON_LMASK) && ev.motion.x < SIM_W) {
                    TriCoord tc = screenToTri(ev.motion.x, ev.motion.y,
                                             gs.viewOffX, gs.viewOffY);
                    switch (gs.editMode) {
                    case GameState::Mode::DRAW_ZPF_POS: {
                        ZPFCell c; c.alive=true; c.charge=Charge::POS; c.quanta=1;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::DRAW_ZPF_NEG: {
                        ZPFCell c; c.alive=true; c.charge=Charge::NEG; c.quanta=1;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::DRAW_ZPF_NEG2: {
                        ZPFCell c; c.alive=true; c.charge=Charge::NEUTRAL; c.quanta=2;
                        gs.cells[tc] = c; break; }
                    case GameState::Mode::ERASE:
                        gs.cells.erase(tc); break;
                    default: break;
                    }
                }
            }
        }

        // ── Simulation step ────────────────────────────────────────────────
        if (gs.running) {
            tickAccum += dt;
            float tickInterval = 1.0f / gs.speed;
            while (tickAccum >= tickInterval) {
                stepZPF(gs);
                stepTetryons(gs);
                gs.tickCount++;
                tickAccum -= tickInterval;
            }
        }

        // Update stats
        gs.aliveCount   = (int)gs.cells.size();
        gs.tetryonCount = (int)gs.tetryons.size();
        gs.totalKEM     = 0.0;
        for (auto& t : gs.tetryons) gs.totalKEM += t.kemEnergy();

        // ── Render ─────────────────────────────────────────────────────────
        SDL_SetRenderDrawColor(ren, 6, 8, 14, 255);
        SDL_RenderClear(ren);

        // Clip to sim area
        SDL_Rect simClip = {0, 0, SIM_W, SIM_H};
        SDL_RenderSetClipRect(ren, &simClip);
        renderGrid(ren, gs);
        SDL_RenderSetClipRect(ren, nullptr);

        // Right panel
        renderPanel(ren, font, fontSmall, gs);

        SDL_RenderPresent(ren);
    }

    if (font)      TTF_CloseFont(font);
    if (fontSmall) TTF_CloseFont(fontSmall);
    TTF_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

/*
================================================================================
TETRYONIC PHYSICS IMPLEMENTATION NOTES
================================================================================

COORDINATE SYSTEM:
  A2 simplex lattice = Eisenstein integers Z[ω], ω = e^(2πi/3)
  Basis: e1 = (1,0), e2 = (1/2, √3/2)
  Gram matrix: g_μν = [[1, 1/2],[1/2, 1]], det(g) = 3/4
  Each (q,r) cell contains 2 triangles: up=0 (△) and up=1 (▽)

TRIANGLE GEOMETRY:
  Height:    l_q = 1×10⁻¹⁵ m (1 femtometre)
  Side:      L = (2/√3)×l_q = 1.1547×10⁻¹⁵ m
  Area:      A_q = l_q²/√3 = 5.7735×10⁻³¹ m²
  In pixels: TRI_H = 28px per triangle height

ZPF GAME OF LIFE (2D — KEM FIELD LAYER):
  Cells are equilateral triangles on the A2 lattice
  Neighbors: 3 face-sharing (strong force range) + 6 vertex-sharing (weak)
  Rules derived from Section 3 & 5:
    Survive: 2 face-neighbors (stable pairing)
    Survive weakly: 1 face-neighbor
    Die: 0 or 3 face-neighbors
    Birth: 3 alive face-neighbors → new ZPF from vacuum
    Photon birth: 2 neighbors (+/-) → neutral pair (2 quanta)
  These are Lorentz VARIANT — KEM field gliders

TETRYON FORMATION (2D→3D transition):
  When 4 aligned ZPF quanta form a 2-rhombus cluster → Tetryon spawns
  Implements M₀ = 4m/c² (Section 3.4, 2.2)
  Consumed ZPF cells removed from 2D grid
  Tetryon is Lorentz INVARIANT — same geometry at all velocities

TETRYON PHYSICS:
  Position: continuous float in A2 coordinates
  Velocity: (vq, vr) — KEM contribution = M*v² (Lorentz variant)
  Rest energy: M₀c⁴ (Lorentz invariant)
  Total: E = M₀c⁴ + (γ-1)m_M c² [Section 2.11]
  Interaction: strong force (face contact) within L range
               repulsive same charge, attractive opposite
  Decay: slow Tetryon deposits 4 ZPF quanta back into vacuum

RENDERING:
  Tetrahedra: projected using 3-fold Schlegel projection, spin = QAM
  KEM field:  circle around Tetryon showing kinetic energy (Lorentz variant)
  ZPF+:       red/orange fill (clockwise EM flux)
  ZPF-:       blue fill (counter-clockwise EM flux)
  ZPF0:       cyan (neutral / photon pair)

CONTROLS:
  SPACE     Pause/resume simulation
  F1-F6     Load preset (ZPF sea, gliders, photons, tetryons, mixed, hex ring)
  1-5       Draw mode (ZPF+, ZPF-, ZPF0, Tetryon, Erase)
  +/-       Increase/decrease sim speed
  G         Toggle grid lines
  K         Toggle KEM field indicators
  M         Toggle Tetryon rendering
  C         Clear all
  R         Reset viewport
  RMB+drag  Pan view
  LMB       Draw / place (drag to paint)
  ESC       Quit

BUILD (Visual Studio):
  1. Install SDL2 and SDL2_ttf via NuGet or manually
     - SDL2:     https://github.com/libsdl-org/SDL/releases
     - SDL2_ttf: https://github.com/libsdl-org/SDL_ttf/releases
  2. Create new Empty C++ project, add this file as main.cpp
  3. Properties → C++ → Language → C++17 standard
  4. Properties → Linker → Input → Additional Dependencies:
     SDL2.lib;SDL2main.lib;SDL2_ttf.lib
  5. Copy SDL2.dll, SDL2_ttf.dll, libfreetype-6.dll to project output folder
  6. Build and run

SDL2 NUGET (easiest):
  Tools → NuGet Package Manager → search "SDL2" and "sdl2.ttf"
  Install SDL2 (by avio) and SDL2_ttf
================================================================================
*/

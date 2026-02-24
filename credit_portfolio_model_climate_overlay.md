# Credit Portfolio Model With Climate Overlay

A factor-based framework with deterministic climate/scenario drivers and systematic credit factors calibrated by a structured "hybrid" approach.

---

## 1. Purpose and scope

### What we are trying to achieve

We want a credit portfolio model that can produce a distribution of portfolio losses (or other credit outcomes such as PD changes or migrations) and that can be conditioned on climate scenarios (e.g., NGFS pathways). The model should:

1. Behave like a standard credit factor model (i.e., familiar to risk practitioners and governance).
2. Translate climate scenario variables into systematic credit stress in a transparent, auditable way.
3. Support scenario analysis and stress testing rather than claim to "predict" climate-driven defaults from first principles.
4. Be implementable even when we lack clean historical observables to statistically estimate climate-credit parameters.

### Key simplification we adopt

We keep one main factor object that drives correlated credit outcomes:
- **F**: a sector–region factor matrix, representing systematic "credit conditions" for each sector and region cell.

Climate enters through:
- **C**: a deterministic vector of climate/scenario variables (e.g., carbon price path, energy prices, hazard intensities), which drives F via a mapping F = g(C).

So the model is:
- Credit outcomes depend on F
- F depends on C
- Idiosyncratic noise provides name-level randomness.

---

## 2. Conceptual overview

### Three layers of the system

1. **Scenario/Climate state (deterministic)**:
   C contains the scenario inputs. For an NGFS scenario, C is fixed and known per year/horizon.

2. **Systematic credit environment (factor matrix)**:
   F summarises how "good vs bad" credit conditions are for each sector and region cell. It is the channel through which correlation and clustering of losses are produced.

3. **Idiosyncratic noise**:
   Each obligor still has a firm-specific component independent of the systematic environment.

### Why a sector–region matrix?

A sector–region grid is a practical compromise:
- Captures the biggest drivers of correlated credit risk in diversified portfolios.
- Is interpretable and easy to explain.
- Provides a natural scaffold to attach climate sensitivities:
  - Transition risk is often sector-driven (and region-modulated).
  - Physical risk is often region-driven (and sector-modulated).

---

## 3. Underlying theory: credit portfolio factor models (in plain terms)

### The classic structure

Most portfolio credit models start by constructing a latent creditworthiness variable for each obligor i:

```
Z_i = systematic part + idiosyncratic part
```

The systematic part is shared across obligors (causing correlated outcomes). The idiosyncratic part is independent.

A common, governance-friendly form is:

```
Z_i = β_i^T f + ε_i
```

- **f** are systematic factors (e.g., macro, sector, region).
- **β_i** are loadings/exposures for obligor i.
- **ε_i** is idiosyncratic noise (often standard normal).

A default event is triggered by a threshold:

```
Default if Z_i < τ_i
```

The threshold τ_i is set to reproduce the obligor's unconditional probability of default (PD), typically via:

```
τ_i = Φ^(-1)(PD_i)
```

where Φ is the standard normal CDF.

### Why this works

- The model reproduces marginal PDs by construction.
- It generates default correlation through shared factors.
- It can be simulated efficiently (Monte Carlo) and decomposed by risk drivers.

---

## 4. Our model specification

### 4.1 Factor definition: a single sector–region matrix F

Let sectors be s = 1,...,S and regions be r = 1,...,R.

Define one systematic factor per cell:

```
F_{s,r}
```

**Interpretation**: how favourable or stressed credit conditions are for that sector–region segment over the horizon being modelled (e.g., one-year shock, or multi-year change).

Stack the matrix into a vector:

```
vec(F) ∈ ℝ^(SR)
```

### 4.2 Obligor equation

Each obligor i has exposures β_i ∈ ℝ^(SR).

In the simplest "one bucket per obligor" assignment:
- if obligor i belongs to sector s(i) and region r(i), then β_i is 1 on that cell and 0 elsewhere.

More realistic variants allow mixtures (e.g., multi-region revenue weighting).

The latent credit index is:

```
Z_i = β_i^T vec(F) + ε_i
```

Default rule:

```
Default if Z_i < τ_i,  where τ_i = Φ^(-1)(PD_i)
```

Loss is then `LGD_i × EAD_i` if default occurs (or a more detailed loss model if desired).

---

## 5. Climate overlay: deterministic climate variables C driving F

### 5.1 What C is

C is a vector of scenario variables:

```
C ∈ ℝ^k
```

It is deterministic for a chosen scenario and horizon. It can include both transition and physical variables, such as:

**Transition examples**: carbon price, energy price indices, GDP deviation, policy stringency.

**Physical examples**: region-level hazard severity indices (flood, storm, heat, drought), or derived damage proxies.

**Crucially**: C is not directly used in the obligor default equation. Instead, it determines the systematic credit environment F.

### 5.2 Mapping climate state to credit factors

We define a mapping:

```
F = g(C)
```

A practical, interpretable form is cell-wise linear mapping on standardised features:

```
F_{s,r} = α_{s,r} + w_{s,r}^T φ(C)
```

- **φ(C)** is a feature transform (see below).
- **w_{s,r}** are sensitivities ("weights") that we must calibrate.
- **α_{s,r}** can be set to zero if factors represent deviations from baseline.

### Why we standardise the drivers

We define features in unitless "sigma" terms:

```
φ_k(C) = ΔC_k / σ_k
```

- **ΔC_k** is the scenario change (over horizon).
- **σ_k** is a scale representing a "typical" move for driver k (chosen by design; see calibration).

This makes w interpretable as:

> "How many standard deviations does the sector–region credit factor move for a 1σ move in driver k?"

---

## 6. Key design choices we made (and why)

### Choice 1: Keep a single factor object F (sector–region matrix)

We explicitly simplified the factor universe:
- We do not keep separate F^phys, F^tran factor blocks.
- Instead, we represent the net effect of all systematic stress through a single matrix F_{s,r}.

This is governance-friendly: one factor layer, one place credit correlation lives.

### Choice 2: Treat C as deterministic scenario inputs

We take climate scenarios as exogenous and deterministic:
- selecting "NGFS scenario X" fixes C(t).

This matches typical stress testing usage: scenarios are narratives, not stochastic forecasts.

### Choice 3: Climate affects credit only through sector–region systematic conditions

No direct climate term appears in Z_i. That implies:
- Climate does not create new within-cell differentiation unless obligors have different β_i mixtures.
- Cross-sectional differentiation comes from sector and region assignment (or weights).

This is a deliberate simplification appropriate for a first build; it keeps the model coherent and auditable.

### Choice 4: Calibration by "hybrid" approach rather than pure estimation

Because we lack observables for fitting w, we calibrate by:
1. Constructing relative sensitivities from economic/physical logic, and then
2. Setting overall magnitudes via a small set of scaling parameters chosen to meet reasonableness targets.

This sacrifices statistical identification but gains transparency.

---

## 7. The calibration challenge: what is w, and why it's hard

### What w_{s,r} represents

Each w_{s,r} is a vector of sensitivities linking scenario features φ(C) to systematic credit stress for that sector–region cell.

It encodes:
- **Transition sensitivity**: "how do credit conditions for sector s in region r respond to policy/energy/macro changes?"
- **Physical sensitivity**: "how do they respond to hazard severity changes?"

### Why we can't just "estimate it"

To estimate w statistically you need:
- time series of credit outcomes by sector/region (spreads, EDFs, defaults), and
- matching time series of climate/transition drivers that are meaningful at credit horizons.

Often:
- defaults are sparse and noisy,
- spreads contain many confounders,
- climate variables are not observed historically in the way scenario models require.

Hence the hybrid approach.

---

## 8. Hybrid calibration approach (in detail)

The hybrid approach deliberately separates shape and scale.

### 8.1 Step A: choose a small set of drivers in C

We keep the driver set compact to avoid overfitting and to keep interpretation clear.

A typical v1 might use 4–6 drivers:

**Transition drivers**
1. Carbon policy / carbon price shock
2. Energy price shock (or fossil price index)
3. Macro demand/output shock (GDP deviation proxy)

**Physical drivers**
4. Flood severity index (by region/country)
5. Storm severity index (by region/country)
6. Heat/drought severity index (by region/country)

These can be constructed from NGFS scenario outputs plus physical risk datasets (or NGFS physical indicators where available).

### 8.2 Step B: define standardisation scales σ_k

We need σ_k for each driver. Because we aren't fitting statistically, σ_k is a modelling choice.

Common defensible options:
- **Cross-scenario dispersion**: use SD of ΔC_k across the scenario set and models.
- **Chosen stress unit**: define "1σ carbon shock" as a specified log-change, etc.

The aim is interpretability and stability.

### 8.3 Step C: build a sensitivity library (relative weights)

Define sector scores and region scores:
- **S_{s,k} ∈ [0,1]**: how exposed sector s is to driver k
- **R_{r,k} ∈ [0,1]**: how exposed region r is to driver k

Then define relative cell sensitivities:

```
w̃_{s,r,k} = S_{s,k} · R_{r,k}
```

This produces the correct qualitative behaviour:
- high-carbon sectors respond more to carbon policy shocks,
- hazard-prone regions respond more to physical indices,
- some sectors are naturally more vulnerable to physical disruption than others.

### What goes into the scores

These are not arbitrary; they are grounded in observable proxies and expert judgement:

**Transition:**
- emissions intensity / abatement difficulty,
- energy intensity,
- pricing power / pass-through,
- stranded asset risk.

**Physical:**
- asset heaviness and fixed-site dependence,
- supply chain fragility,
- sensitivity to outages and business interruption,
- adaptation/insurance assumptions (if included).

**Region scores:**
- hazard propensity (for physical),
- policy/regulatory stance and energy mix (for transition),
- adaptation capacity (optional).

### 8.4 Step D: introduce a small number of scale parameters λ_k

We then define:

```
w_{s,r,k} = λ_k · w̃_{s,r,k}
```

Now calibration reduces to choosing 4–6 numbers λ_k, rather than S×R×k parameters.

**Interpretation:**

λ_k controls how strongly the entire credit system responds to driver k, while w̃ controls which sectors/regions are most exposed.

### 8.5 Step E: set λ_k using anchor constraints ("reasonableness targets")

With no observables, we choose λ_k so that outputs meet a small set of qualitative and quantitative expectations:

**Anchor type 1: factor-magnitude plausibility**

We impose conventions such as:
- Under a "strong transition shock" year, the most exposed cells should move by ~2σ (severe but not absurd).
- Under moderate scenario years, the shift might be ~0.5–1σ.

This is a disciplined way to avoid unrealistic overreaction.

**Anchor type 2: portfolio-level uplift bounds**

We pick a small set of portfolio metrics to match under benchmark scenarios:
- EL uplift under severe transition scenario is of plausible order (you choose bounds).
- Tail loss uplift under severe scenario is plausible and not dominated by a single cell unless expected.
- Sector contributions line up with intuition (brown sectors contribute most under transition).

This is calibration-by-target, but with minimal knobs and explicit governance.

### 8.6 Step F: scenario sanity testing and iterative tightening

We iterate:
- Run baseline and selected NGFS scenarios,
- inspect factor heatmaps F_{s,r},
- inspect loss decomposition by sector/region,
- adjust λ_k (and only if necessary, adjust score tables).

Document each change with a rationale (this matters for audit).

---

## 9. What this model can and cannot claim

### What it can do well

- Produce coherent loss distributions under climate scenarios.
- Attribute incremental risk to sector and region segments in a controlled way.
- Provide a transparent, explainable linkage from scenario variables to credit stress.
- Support stress testing, risk appetite discussion, and portfolio steering.

### What it should not claim (without further development)

- Precise prediction of obligor defaults driven by climate.
- Fine-grained within-sector differentiation unless β_i includes richer exposure splits.
- Causal inference from historical climate to credit outcomes (we are not estimating causally).

---

## 10. Implementation blueprint (high level)

1. **Portfolio data preparation**
   - Assign each obligor to sector and region (or weights across them).
   - Set PD, LGD, EAD and horizon.

2. **Define scenario driver vector C**
   - Choose drivers and create deterministic scenario paths.
   - Compute φ(C) for the chosen horizon.

3. **Construct the mapping F = g(C)**
   - Build sector and region score tables.
   - Compute w̃ and apply λ scales.
   - Compute F for the scenario.

4. **Simulate portfolio outcomes**
   - For each simulation draw:
     - sample idiosyncratic ε_i (and optionally, stochasticity in F if later added),
     - compute Z_i, apply default thresholds,
     - compute losses.

5. **Report**
   - Baseline vs scenario EL/UL/tail quantiles.
   - Sector/region contribution analysis.
   - Factor heatmaps and scenario deltas.

---

## 11. Optional extensions (kept out of v1, but natural later steps)

### Extension A: add systematic residual noise η

If deterministic C makes F too rigid, introduce:

```
F = g(C) + η
```

with parsimonious covariance (e.g., sector + region random effects). This creates "sector-level random behaviour" beyond pure idiosyncratics while keeping the scenario mean anchored to C.

### Extension B: time dynamics

Move from a single-horizon shock to a multi-year path:
- F(t) = g(C(t))
- default can be simulated annually with updating thresholds or via migration modelling.

### Extension C: richer obligor exposures

Instead of single-cell assignment:
- allow β_i to be a weighted mix (revenue split across regions, supply chain geography, etc.).

---

## 12. Summary of the final simplified model

### Objects

- **C**: deterministic climate/scenario variable vector (NGFS-like inputs)
- **F**: sector–region matrix of systematic credit conditions

### Equations

**Mapping:**
```
F_{s,r} = α_{s,r} + Σ_k w_{s,r,k} φ_k(C)
```

**Obligor index:**
```
Z_i = β_i^T vec(F) + ε_i
```

**Default threshold:**
```
Default if Z_i < Φ^(-1)(PD_i)
```

**Calibration:**
```
w_{s,r,k} = λ_k · (S_{s,k} R_{r,k})
```

with λ_k chosen via factor-plausibility and portfolio-target anchors.

---

## Notation Reference

| Symbol | Description |
|--------|-------------|
| C | Deterministic climate/scenario variable vector |
| F | Sector–region factor matrix (systematic credit conditions) |
| Z_i | Latent creditworthiness index for obligor i |
| β_i | Factor loadings/exposures for obligor i |
| ε_i | Idiosyncratic noise for obligor i |
| τ_i | Default threshold for obligor i |
| Φ | Standard normal cumulative distribution function |
| PD_i | Probability of default for obligor i |
| LGD_i | Loss given default for obligor i |
| EAD_i | Exposure at default for obligor i |
| w_{s,r} | Sensitivity weights for sector s, region r |
| φ(C) | Standardized feature transformation of climate variables |
| S_{s,k} | Sector exposure score for sector s to driver k |
| R_{r,k} | Region exposure score for region r to driver k |
| λ_k | Scale parameter for driver k |
| σ_k | Standardization scale for driver k |

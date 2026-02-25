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

### Four layers of the system

1. **Scenario/Climate drivers (deterministic)**:
   X contains the scenario inputs. For an NGFS scenario, X is fixed and known per year/horizon. Includes both transition drivers (carbon price, energy prices, GDP) and physical drivers (heat, flood, drought).

2. **Systematic credit factor surface (random, conditional on X)**:
   - F_{s,r}: Sector-region cell factors (s = 1,...,S; r = 1,...,R)
   - Each F_{s,r} represents systematic credit conditions for sector s in region r
   - Decomposed as: F_{s,r} = m_{s,r}(X) + u_{s,r}
     - m_{s,r}(X): deterministic climate-driven mean
     - u_{s,r}: stochastic residual with structured variance decomposition

3. **Obligor creditworthiness**:
   Each obligor loads on sector-region cells: Z_i = β_i^T · vec(F) + ε_i

4. **Idiosyncratic noise**:
   Each obligor has a firm-specific component ε_i independent of systematic factors.

### Why a sector-region cell structure with residual decomposition?

The cell-level factor matrix F_{s,r} provides:
- **Natural interpretation**: F_{coal,europe} directly represents "coal sector in Europe"
- **Flexible exposures**: Obligors can load on multiple cells (diversified portfolios)
- **Climate overlay**: Deterministic scenario X drives systematic mean m_{s,r}(X)

The residual decomposition u_{s,r} = a_s + b_r + ξ_{s,r} provides:
- **Parsimonious correlation**: Only 3 parameters (σ_a, σ_b, σ_ξ) generate full cell correlation structure
- **Interpretable co-movement**:
  - a_s captures sector-wide shocks (brown sectors co-move)
  - b_r captures region-wide shocks (regional business cycles)
  - ξ_{s,r} captures cell-specific variation
- **Easy simulation**: S + R + (S×R) independent normal draws, no matrix decomposition needed

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

### 4.1 Factor definition: Sector-region cell matrix

Let sectors be s = 1,...,S (e.g., S=15) and regions be r = 1,...,R (e.g., R=7).

Define the systematic factor matrix:

```
F ∈ ℝ^(S×R)
```

Where F_{s,r} represents the systematic credit condition for sector s in region r.

Total number of cells: M = S × R (e.g., M = 105 for 15 sectors × 7 regions).

**Interpretation**:
- F_{coal,europe} represents systematic credit stress for coal companies in Europe
- F_{agriculture,asia} represents systematic credit stress for agriculture in Asia
- These are **latent random variables** conditional on climate scenario

### 4.2 Factor decomposition: Climate mean + residual

Each cell factor is decomposed as:

```
F_{s,r} = m_{s,r}(X) + u_{s,r}
```

Where:
- **m_{s,r}(X)**: Deterministic scenario-driven mean (climate overlay)
- **u_{s,r}**: Stochastic residual (non-climate systematic uncertainty)

### 4.3 Obligor equation

Each obligor i has a loading vector β_i ∈ ℝ^M.

The latent credit index is:

```
Z_i = β_i^T · vec(F) + ε_i
```

Where vec(F) stacks the S×R matrix column-wise into a vector.

**Simple assignment** (single cell):
- If obligor i is in sector s(i) and region r(i), then β_i has a 1 in position (s(i),r(i)) and 0 elsewhere

**Mixed assignment** (diversified exposure):
- If obligor i has 60% revenue from coal in Europe, 40% from oil in North America:
  - β_{coal,europe} = 0.6, β_{oil,NA} = 0.4, all others = 0
- Loadings sum to 1: Σ_{s,r} β_{i,(s,r)} = 1

Default rule:

```
Default if Z_i < τ_i,  where τ_i = Φ^(-1)(PD_i)
```

Loss is then `LGD_i × EAD_i` if default occurs.

---

## 5. Climate overlay: deterministic scenario drivers X

### 5.1 Scenario driver vector X

X is a vector of scenario variables:

```
X ∈ ℝ^K    (K = 8 in our implementation)
```

It is **deterministic** for a chosen scenario and horizon. For NGFS "Net Zero 2050" year 2050, X is fixed.

**Transition drivers (5)**:
1. CarbonPrice - Carbon tax/price (US$2010/t CO2)
2. CoalPrice - Coal price index (US$2010/GJ)
3. OilPrice - Oil price index (US$2010/GJ)
4. GasPrice - Natural gas price (US$2010/GJ)
5. GDP - GDP growth/deviation (billion US$2010/yr, PPP)

**Physical drivers (3)**:
6. HeatIndex - Days with heat index >35°C
7. FloodRisk - Maximum 1-day precipitation (mm)
8. DroughtRisk - Consecutive dry days

**Crucially**: X does not directly appear in the obligor equation. It determines the **mean** of systematic factors: m_{s,r}(X).

### 5.2 Standardization of drivers φ(X)

We standardize drivers to unitless "sigma" terms:

```
φ_k(X) = ΔX_k / σ_k
```

Where:
- **ΔX_k**: Scenario change for driver k relative to baseline (e.g., Current Policies)
- **σ_k**: Scale parameter ("typical move" for driver k)

In our implementation:
- **σ_k = cross-scenario standard deviation** (computed from Net Zero, Delayed, Current Policies)
- **φ_k ranges from -2σ to +2σ** across scenarios

**Interpretation**: φ_k = 1 means "a 1-sigma move in driver k relative to baseline"

### 5.3 Climate-to-credit mapping m_{s,r}(X)

The deterministic climate mean is a linear function of standardized drivers:

```
m_{s,r}(X) = α_{s,r} + Σ_k w_{s,r,k} · φ_k(X)
```

Where:
- **α_{s,r}**: Baseline intercept (typically 0 if F represents deviations)
- **w_{s,r,k}**: Sensitivity of cell (s,r) to driver k

**Interpretation of w_{coal,europe,carbon} = -2.0**:
- When carbon price increases by 1σ, coal-Europe systematic factor decreases by 2.0σ (severe stress)

### 5.4 Hybrid calibration: Structured weights

Rather than calibrate M×K = 840 weights independently, we use structured parameterization:

**Step 1**: Define relative exposure scores
- **S_{s,k} ∈ [0,1]**: How exposed sector s is to driver k
- **R_{r,k} ∈ [0,1]**: How exposed region r is to driver k

**Step 2**: Combine via product
```
w̃_{s,r,k} = S_{s,k} · R_{r,k}
```

**Step 3**: Apply global scale parameters
```
w_{s,r,k} = λ_k · w̃_{s,r,k} = λ_k · S_{s,k} · R_{r,k}
```

**Interpretation**:
- **S and R** encode relative vulnerabilities (which sectors/regions are most exposed)
- **λ_k** controls overall magnitude (how strongly driver k translates to credit stress)

This reduces calibration from 840 parameters to:
- S matrix: 15 sectors × 8 drivers = 120 scores
- R matrix: 7 regions × 8 drivers = 56 scores
- λ vector: 8 scale parameters
- **Total: 184 parameters** (vs 840), with clear economic interpretation

### 5.5 Residual variance decomposition u_{s,r}

The stochastic residual captures non-climate systematic uncertainty with a **random effects structure**:

```
u_{s,r} = a_s + b_r + ξ_{s,r}
```

Where:
- **a_s ~ N(0, σ_a²)**: Sector-wide residual shock (independent across sectors)
- **b_r ~ N(0, σ_b²)**: Region-wide residual shock (independent across regions)
- **ξ_{s,r} ~ N(0, σ_ξ²)**: Cell-specific residual shock (independent across cells)
- All components mutually independent

**Induced correlation structure**:

```
Cov(u_{s,r}, u_{s',r'}) = {
  σ_a² + σ_b² + σ_ξ²    if s = s', r = r'   (same cell, = total variance)
  σ_a²                  if s = s', r ≠ r'   (same sector, different region)
  σ_b²                  if s ≠ s', r = r'   (different sector, same region)
  0                     if s ≠ s', r ≠ r'   (different sector AND region)
}
```

**Correlations**:
```
Corr(u_{s,r}, u_{s,r'}) = σ_a² / (σ_a² + σ_b² + σ_ξ²)    [within-sector correlation]

Corr(u_{s,r}, u_{s',r}) = σ_b² / (σ_a² + σ_b² + σ_ξ²)    [within-region correlation]
```

**Proposed calibration** (based on equity market empirics):
- **σ_a = 0.50**: Implies within-sector correlation ≈ 0.40
- **σ_b = 0.40**: Implies within-region correlation ≈ 0.25
- **σ_ξ = 0.47**: Cell-specific residual
- **Total variance**: σ_a² + σ_b² + σ_ξ² = 0.25 + 0.16 + 0.22 = 0.63 (σ_total ≈ 0.79)

**Key advantages**:
- **Only 3 parameters** (vs 5,460 for full 105×105 covariance matrix)
- **Interpretable**: Sector shocks, region shocks, cell shocks
- **Easy simulation**: Draw S + R + (S×R) independent normals, no matrix decomposition
- **Economically motivated**: Brown sectors co-move, regional business cycles co-move

---

## 6. Key design choices we made (and why)

### Choice 1: Cell-level factors F_{s,r} (not global or separate sector/region factors)

We use a **sector-region cell matrix** F ∈ ℝ^(S×R):
- **Natural interpretation**: F_{coal,europe} directly represents "coal in Europe"
- **Flexible loadings**: Obligors can have multi-cell exposures (e.g., diversified revenue)
- **Direct climate mapping**: Each cell responds to climate drivers independently

This preserves maximum granularity while keeping the factor structure interpretable.

### Choice 2: Decomposition into climate mean + residual

**Climate mean m_{s,r}(X)**: Deterministic, scenario-driven
- Selecting "NGFS Net Zero 2050" fixes m(X) completely
- Linear mapping: m_{s,r}(X) = Σ_k λ_k · S_{s,k} · R_{r,k} · φ_k(X)
- Matches stress testing convention: scenarios are narratives, not forecasts

**Stochastic residual u_{s,r}**: Random, captures non-climate systematic risk
- u_{s,r} = a_s + b_r + ξ_{s,r}
- Allows sector co-movement, region co-movement, cell-specific shocks
- Only 3 parameters (σ_a, σ_b, σ_ξ) instead of 5,460 covariance parameters

This separation makes uncertainty quantification clean: scenario risk (deterministic X) vs. conditional risk (random u).

### Choice 3: Random effects residual structure u_{s,r} = a_s + b_r + ξ_{s,r}

**Key innovation**: Rather than specifying a full M×M covariance matrix, we use additive components:
- **a_s**: Sector-wide shocks (brown sectors co-move)
- **b_r**: Region-wide shocks (regional business cycles)
- **ξ_{s,r}**: Cell-specific shocks

**Benefits**:
- **Parsimony**: 3 parameters generate full correlation structure
- **Interpretability**: Each component has clear economic meaning
- **Easy simulation**: S + R + (S×R) independent draws, no Cholesky needed
- **Empirically grounded**: Calibrated to match equity sector/region correlations (~0.40 and ~0.25)

This is a standard random effects model from econometrics, adapted to credit portfolios.

### Choice 4: Hybrid calibration for climate sensitivity

Rather than calibrate 840 independent weights w_{s,r,k}, we use structured approach:
```
w_{s,r,k} = λ_k · S_{s,k} · R_{r,k}
```

**184 parameters instead of 840**:
- S matrix (15×8 = 120): Sector vulnerabilities
- R matrix (7×8 = 56): Region exposures
- λ vector (8): Global scale parameters

**Rationale**:
- Lack of long climate-credit time series
- Economic logic guides S and R (emissions intensity, hazard exposure)
- λ calibrated to "reasonableness targets" (max cell move ~2σ under severe scenario)

This sacrifices statistical identification but gains transparency and implementability.

### Choice 5: Climate affects credit only through systematic factors

No direct climate term in Z_i. All climate impact flows through F:
```
Z_i = β_i^T · vec(F) + ε_i,    where F_{s,r} = m_{s,r}(X) + u_{s,r}
```

**Implications**:
- Climate does not create within-cell differentiation (unless β differs)
- Cross-sectional variation comes from sector-region assignments
- All climate-driven correlation flows through systematic factors

This is appropriate for portfolio-level stress testing where obligor-specific climate data is sparse.

---

## 7. The calibration challenge: what needs to be specified

We need to calibrate five objects:

### 7.1 Sector exposure scores S (15 × 8)

Each S_{s,k} ∈ [0,1] represents: "How exposed is sector s to driver k (relative to other sectors)?"

**Example**:
- S_{coal, carbon} = 1.0: Coal is maximally exposed to carbon price
- S_{renewables, carbon} = 0.2: Renewables benefit from carbon price (reverse exposure)
- S_{agriculture, drought} = 0.9: Agriculture very sensitive to drought
- S_{tech, carbon} = 0.1: Technology sector has minimal carbon exposure

**Data sources**: Emissions intensity, energy intensity, physical asset exposure, expert judgment

### 7.2 Region exposure scores R (7 × 8)

Each R_{r,k} ∈ [0,1] represents: "How exposed is region r to driver k (relative to other regions)?"

**Example**:
- R_{asia_em, heat} = 1.0: Asia-Pacific emerging maximally exposed to heat stress
- R_{europe, carbon} = 0.8: Europe high regulatory/policy exposure to carbon price
- R_{MEA, drought} = 0.9: Middle East & Africa very exposed to drought

**Data sources**: Climate hazard exposure (World Bank CCKP), policy stringency, GDP structure

### 7.3 Global scale parameters λ (8 scalars)

Each λ_k controls: "How strongly does a 1σ move in driver k translate to credit stress?"

**Example**:
- λ_{carbon} = 0.8: Moderate translation of carbon price to credit factors
- λ_{drought} = 0.7: Moderate translation of drought to credit factors

**Calibration approach**: "Reasonableness targets"
- Max exposed cell should move ~2σ under severe scenario
- Portfolio EL uplift should be plausible (2x-5x baseline)

### 7.4 Residual variance parameters (3 scalars)

- **σ_a**: Sector-wide residual standard deviation
- **σ_b**: Region-wide residual standard deviation
- **σ_ξ**: Cell-specific residual standard deviation

These control correlation structure:
- Within-sector correlation = σ_a² / (σ_a² + σ_b² + σ_ξ²)
- Within-region correlation = σ_b² / (σ_a² + σ_b² + σ_ξ²)

**Calibration approach**: Target empirical correlations from equity markets (~0.40 sector, ~0.25 region)

### 7.5 Obligor loadings β (N × M matrix)

Each β_i ∈ ℝ^M specifies obligor i's exposure across sector-region cells.

**Pure play**: β_i has single 1 entry for obligor's primary sector-region
**Diversified**: β_i weighted by revenue/asset breakdowns

**Data source**: Financial reports, segment disclosures

---

## 8. Calibration approach (in detail)

### 8.1 Step A: Define climate drivers C (COMPLETE ✅)

We use **8 drivers** (5 transition + 3 physical):

**Transition drivers**:
1. CarbonPrice - Carbon tax (US$2010/t CO2)
2. CoalPrice - Coal price index (US$2010/GJ)
3. OilPrice - Oil price index (US$2010/GJ)
4. GasPrice - Natural gas price (US$2010/GJ)
5. GDP - GDP growth/deviation (billion US$2010/yr, PPP)

**Physical drivers**:
6. HeatIndex - Days >35°C heat index
7. FloodRisk - Max 1-day precipitation (mm)
8. DroughtRisk - Consecutive dry days

**Data sources**:
- Transition: NGFS Phase V (IIASA), 420 data points
- Physical: World Bank CCKP (CMIP6), 126 data points
- Total: 546 data points (3 scenarios × 7 regions × varying years)

### 8.2 Step B: Standardize drivers φ(C) (COMPLETE ✅)

We standardize using cross-scenario dispersion:

```
φ_k = (C_k - C_baseline,k) / σ_k
```

Where:
- **C_baseline**: Current Policies scenario (high warming baseline)
- **σ_k**: Standard deviation across 3 scenarios (Net Zero, Delayed, Current)

**Result**: φ values range from -2σ to +2σ across scenarios, interpretable as "mild" to "severe" deviations from baseline.

### 8.3 Step C: Calibrate factor loading matrix A (22 × 8) - PHASE 3

The A matrix encodes how each of 22 factors responds to each of 8 drivers.

**Calibration approaches**:

1. **Expert judgment** (Phase 3 start):
   - Sector sensitivities: Brown sectors (coal, oil) stressed by carbon price; renewables benefit
   - Region sensitivities: Physical-exposed regions (Asia emerging, Africa) stressed by heat/flood
   - Sign and rough magnitude based on economic logic
   - Document rationale for each entry

2. **Historical regression** (Phase 3 optional enhancement):
   - Regress sector CDS spreads / equity indices on macro proxies
   - Use energy price shocks, GDP growth as training data
   - Extrapolate to climate drivers (carbon price ~ energy price shock)
   - Limited by data availability

3. **Anchor calibration** (Phase 3 validation):
   - Check that severe scenarios produce plausible factor moves (~2σ for most exposed)
   - Check that portfolio-level EL uplift is reasonable (e.g., 2x-5x baseline under Net Zero)
   - Iterate on magnitudes

**Example calibration**:
```
A_{coal, carbon} = -2.0      (coal sector crashes under high carbon price)
A_{coal, coal_price} = -1.5  (coal sector also hurt by low coal prices)
A_{coal, GDP} = +0.8         (coal somewhat pro-cyclical)
A_{renewables, carbon} = +1.0 (renewables benefit from high carbon price)
A_{europe, heat} = -0.6      (European economy moderately stressed by heat)
A_{agriculture, drought} = -1.2 (agriculture very stressed by drought)
```

### 8.4 Step D: Specify residual factor covariance Σ_F|C (22 × 22) - PHASE 3

Captures factor correlation beyond climate drivers.

**Calibration approaches**:

1. **Diagonal (simple)**: Σ_F|C = σ² · I
   - Factors independent conditional on climate
   - Single volatility parameter σ (e.g., σ = 1.0 or 0.5)
   - Easy to implement, minimal parameters

2. **Block-diagonal (moderate)**:
   - Within-sector block: Corr(F_coal, F_oil | C) = 0.5
   - Within-region block: Corr(F_europe, F_asia | C) = 0.3
   - Captures brown sector co-movement, regional contagion
   - ~10-20 parameters

3. **Full covariance (realistic)**:
   - Estimate from historical factor analysis (sector/region indices)
   - Use equity sector indices, regional GDP correlations
   - ~200+ parameters but most realistic

**Recommended start**: Diagonal with σ = 0.8 (allows some residual volatility but keeps factors mostly climate-driven)

### 8.5 Step E: Assign obligor loadings β - PHASE 1 COMPLETE ✅

For each obligor i, specify β_i ∈ ℝ^22:

**Pure play** (single sector, single region):
- β_{coal,i} = 1, β_{europe,i} = 1, all others = 0
- Most common for focused corporates

**Diversified** (multi-sector or multi-region):
- β_{coal,i} = 0.6, β_{oil,i} = 0.4 (revenue-weighted sectors)
- β_{NA,i} = 0.5, β_{europe,i} = 0.5 (revenue-weighted regions)
- Typical for large multinationals

**Data source**: Financial reports (segment revenue breakdowns, geographic revenue)

**Current status**: Synthetic portfolio created with pure-play assignments. Real portfolio can load from CSV templates.

### 8.6 Step F: Validation and iteration - PHASE 6

After calibration:
1. Run all 3 scenarios (Net Zero, Delayed, Current Policies)
2. Check factor realizations: Do they make sense? (e.g., F_coal very negative under Net Zero?)
3. Check portfolio losses: Plausible magnitudes? (e.g., EL 2x-5x baseline?)
4. Check decomposition: Brown sectors dominate transition scenarios? Physical-exposed sectors dominate Current Policies?
5. Sensitivity analysis: Vary A entries by ±30%, check impact on results
6. Document all assumptions and rationale

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

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
   C contains the scenario inputs. For an NGFS scenario, C is fixed and known per year/horizon. Includes both transition drivers (carbon price, energy prices, GDP) and physical drivers (heat, flood, drought).

2. **Systematic latent factors (random, conditional on C)**:
   - F_s: Sector factors (s = 1,...,S) representing systematic credit conditions for each sector
   - F_r: Region factors (r = 1,...,R) representing systematic credit conditions for each region
   - These are multivariate normal, conditional on climate drivers: F | C ~ MVN(μ(C), Σ_F|C)

3. **Obligor creditworthiness**:
   Each obligor loads on sector and region factors: Z_i = β_{s,i} · F_s + β_{r,i} · F_r + ε_i

4. **Idiosyncratic noise**:
   Each obligor still has a firm-specific component ε_i independent of systematic factors.

### Why separate sector and region factors?

Sector and region factors are explicit latent variables (not derived constructs):
- **Interpretability**: F_coal directly represents systematic stress in the coal sector
- **Flexibility**: Sectors and regions can have residual correlation beyond climate drivers
- **Clean separation**: Climate drivers C are deterministic (scenario-defined), while factors F are random (business cycle variation)
- **Realistic correlation**: Brown sectors co-move, physical-exposed regions co-move, naturally

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

### 4.1 Factor definition: Separate sector and region factors

Let sectors be s = 1,...,S (e.g., S=15) and regions be r = 1,...,R (e.g., R=7).

Define systematic factors:

```
F = [F_1, ..., F_S, F_{R+1}, ..., F_{R+R}]  ∈ ℝ^(S+R)
```

Where:
- **F_s** (s = 1,...,S): Sector factors representing systematic credit stress for each sector
- **F_r** (r = 1,...,R): Region factors representing systematic credit stress for each region

**Interpretation**:
- F_coal represents how stressed the coal sector is (globally, or as a sector-wide systematic shock)
- F_europe represents how stressed the European economy is (across all sectors)
- These are **latent random variables**, not deterministic functions of climate

### 4.2 Obligor equation

Each obligor i loads on both sector and region factors:

```
Z_i = Σ_s β_{s,i} · F_s + Σ_r β_{r,i} · F_r + ε_i
```

In the simplest "single sector, single region" assignment:
- If obligor i is in sector s(i) and region r(i), then β_{s(i),i} = 1, β_{r(i),i} = 1, all other β = 0

More realistic variants allow mixtures:
- Revenue-weighted sectors: β_{coal,i} = 0.7, β_{oil,i} = 0.3
- Revenue-weighted regions: β_{NA,i} = 0.6, β_{EU,i} = 0.4

Default rule:

```
Default if Z_i < τ_i,  where τ_i = Φ^(-1)(PD_i)
```

Loss is then `LGD_i × EAD_i` if default occurs (or a more detailed loss model if desired).

### 4.3 Factor count: S+R rather than S×R

**Key advantage**: We use S+R = 22 factors (15 sectors + 7 regions) instead of S×R = 105 sector-region cell factors.

This is:
- More parsimonious (22 vs 105 dimensions)
- More interpretable (factors have clear meaning)
- Statistically better behaved (fewer parameters to calibrate)
- Flexible enough (captures sector-wide and region-wide correlation)

---

## 5. Climate overlay: deterministic climate variables C driving F

### 5.1 What C is

C is a vector of scenario variables:

```
C ∈ ℝ^K    (K = 8 in our implementation)
```

It is deterministic for a chosen scenario and horizon. It includes:

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

**Crucially**: C is not directly used in the obligor default equation. Instead, it determines the **conditional distribution** of systematic factors F.

### 5.2 Factors are random, conditional on climate drivers

The key innovation: factors F are **random variables** that depend on climate scenario C:

```
F | C ~ MVN(μ(C), Σ_F|C)
```

Where:
- **μ(C) = A · φ(C)**: Expected factors given climate scenario (S+R × 1 vector)
- **A**: Factor loading matrix (S+R × K), how each factor responds to each driver
- **φ(C)**: Standardized climate drivers (K × 1 vector)
- **Σ_F|C**: Residual factor covariance (S+R × S+R), capturing non-climate systematic risk

### 5.3 Factor loading matrix A

The A matrix (22 × 8) encodes how each sector/region factor responds to each climate driver:

```
         CarbonPrice  CoalPrice  OilPrice  GasPrice  GDP  HeatIndex  FloodRisk  DroughtRisk
F_coal        -2.0      -1.5      -0.3      -0.2    0.8     -0.2        0.0        0.0
F_oil         -1.5      -0.2      -1.8      -0.5    0.8     -0.1        0.0        0.0
F_renew       +1.0      +0.5      +0.3      +0.2    0.5     -0.3       -0.2       -0.2
F_agri        -0.3       0.0       0.0       0.0    1.2     -1.0       -0.8       -1.2
...
F_europe      -0.5      -0.2      -0.3      -0.2    1.5     -0.6       -0.4       -0.3
F_asia_em     -0.4      -0.1      -0.2      -0.1    1.8     -1.2       -0.8       -0.5
...
```

**Interpretation of A_{coal, carbon} = -2.0**:
- When carbon price increases by 1σ (standardized), coal sector factor decreases by 2.0σ (severe stress)

**Interpretation of A_{agri, heat} = -1.0**:
- When heat index increases by 1σ, agriculture sector factor decreases by 1.0σ (credit stress)

### 5.4 Standardization of climate drivers

We define standardized features in unitless "sigma" terms:

```
φ_k(C) = (C_k - C_baseline,k) / σ_k
```

Where:
- **C_k**: Raw driver value in scenario
- **C_baseline,k**: Baseline value (we use Current Policies scenario)
- **σ_k**: Standard deviation across scenarios (cross-scenario dispersion)

This makes A_{f,k} interpretable as:

> "How many σ does factor f move for a 1σ move in driver k?"

**Example**: Under Net Zero 2050 in year 2050:
- CarbonPrice: φ ~ +2.0σ (high carbon price vs. baseline)
- HeatIndex: φ ~ -2.0σ (low heat stress vs. baseline)
- Combined effect on F_coal: μ_coal = -2.0×(+2.0) + (-0.2)×(-2.0) = -3.6σ (severe stress)

### 5.5 Residual factor covariance Σ_F|C

The Σ_F|C matrix (22 × 22) captures factor correlations **beyond climate drivers**:

```
Σ_F|C = [Corr(F_coal, F_oil | C)      ...  Corr(F_coal, F_europe | C)     ]
        [        ...                   ...           ...                    ]
        [Corr(F_europe, F_coal | C)    ...  Corr(F_europe, F_asia_em | C) ]
```

**Sources of residual correlation**:
- **Sectoral co-movement**: Business cycles, supply chains, credit spreads (e.g., brown sectors beyond carbon price)
- **Regional co-movement**: Macro shocks, trade linkages, contagion (e.g., Europe-Asia beyond climate)
- **Factor structure**: Can be diagonal (independent residuals) or structured (sectoral/regional blocks)

**Calibration approach**:
- **Option 1 (simple)**: Diagonal Σ_F|C (factors independent conditional on climate)
- **Option 2 (moderate)**: Block-diagonal (within-sector correlation, within-region correlation)
- **Option 3 (realistic)**: Full matrix estimated from historical factor analysis or expert judgment

---

## 6. Key design choices we made (and why)

### Choice 1: Separate sector and region factors (not sector×region cells)

We use **S+R = 22 factors** (15 sectors + 7 regions) rather than S×R = 105 sector-region cell factors:
- **Parsimony**: Far fewer dimensions (22 vs 105)
- **Interpretability**: F_coal, F_europe have clear meaning
- **Flexibility**: Sectors and regions can have independent residual correlations
- **Realistic**: A coal company in Europe loads on both F_coal (sector stress) and F_europe (regional macro)

This additive structure captures the key correlation patterns while remaining tractable.

### Choice 2: Factors are random, drivers are deterministic

**Climate drivers C**: Deterministic, scenario-defined
- Selecting "NGFS Net Zero 2050" fixes C(t) completely
- No uncertainty in carbon price, heat index conditional on scenario
- Matches stress testing convention: scenarios are narratives, not forecasts

**Systematic factors F**: Random, conditional on C
- F | C ~ MVN(μ(C), Σ_F|C)
- Captures business cycle variation, credit market volatility, non-climate shocks
- Allows proper correlation structure and tail risk

This separation makes uncertainty quantification clean: scenario risk (deterministic C) vs. conditional risk (random F).

### Choice 3: Linear factor loading (A matrix) with residual covariance

The conditional mean is linear:
```
μ(C) = A · φ(C)
```

This is transparent and auditable:
- Each A_{f,k} has clear interpretation
- Can be calibrated via expert judgment or historical regression
- Easy to explain to governance

The residual covariance Σ_F|C captures everything beyond climate:
- Can start simple (diagonal) and enhance later (structured)
- Allows sectoral/regional correlation beyond climate drivers

### Choice 4: Climate affects credit only through systematic factors

No direct climate term appears in Z_i. That implies:
- Climate does not create within-sector/region differentiation unless obligors have different β mixtures
- Cross-sectional differentiation comes from sector and region loadings
- All climate-driven default correlation flows through F

This is a deliberate simplification appropriate for a first build; it keeps the model coherent and auditable.

### Choice 5: Calibration by structured approach

Because we lack long time series of climate-credit outcomes, we calibrate:
1. **A matrix**: Expert judgment on factor-driver sensitivities (how does F_coal respond to carbon price?)
2. **Σ_F|C**: Historical factor analysis or simplified structure (diagonal, block-diagonal)
3. **β loadings**: Portfolio data on sector/region exposures (revenue breakdowns)

This sacrifices statistical identification but gains transparency and implementability.

---

## 7. The calibration challenge: what needs to be specified

We need to calibrate three objects:

### 7.1 Factor loading matrix A (22 × 8)

Each A_{f,k} represents: "How many σ does factor f move for a 1σ move in driver k?"

**Example rows**:
- A_{coal, carbon} = -2.0: Coal sector stressed by carbon price
- A_{renewables, carbon} = +1.0: Renewables benefit from carbon price
- A_{europe, heat} = -0.6: European economy stressed by heat
- A_{agriculture, drought} = -1.2: Agriculture very stressed by drought

**Challenge**: We don't have long time series of climate drivers and sector factor realizations.

### 7.2 Residual factor covariance Σ_F|C (22 × 22)

Captures factor correlation beyond climate drivers:
- Corr(F_coal, F_oil | C): How much do coal and oil co-move beyond energy prices?
- Corr(F_europe, F_asia | C): How much do regional factors co-move beyond climate?

**Challenge**: Need to estimate systematic correlation not explained by climate.

### 7.3 Obligor loadings β (N × 22, where N = number of obligors)

Each obligor's exposure to sector and region factors:
- Pure play: β_i has one sector entry = 1, one region entry = 1, rest = 0
- Diversified: β_i has multiple nonzero entries (revenue-weighted)

**Challenge**: Need revenue/asset breakdown by sector and region (often available from financial reports).

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

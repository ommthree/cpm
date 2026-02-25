# Midas: Credit Portfolio Model With Climate Overlay

**Midas** is a factor-based framework with deterministic climate/scenario drivers and systematic credit factors with market-calibrated correlation structure.

**About Midas**: The name stands for **M**odel for **I**ntegrated **D**efault **A**nalysis with **S**cenarios. Midas transforms climate scenario data into actionable credit risk insights through a transparent, governance-friendly factor model that preserves familiar portfolio modeling behavior while incorporating climate drivers.

**Related Documentation**:
- **Implementation Plan**: `/docs/implementation_plan.md`
- **Data Sources**: `/docs/data_sources.md` (all external data - scenarios, correlations, calibration)
- **Calibration Parameters**: `/data/calibration/README.md`
- **Quick Reference**: `/docs/correlation_calibration_quick_reference.md`

---

## 1. Purpose and scope

### What we are trying to achieve

We want a credit portfolio model that produces a distribution of defaults and losses under a chosen deterministic climate scenario, while preserving familiar factor-model behaviour (correlated outcomes, portfolio tails) using a structured, calibratable correlation layer.

The model should:

1. Behave like a standard credit factor model (familiar to risk practitioners and governance).
2. Translate climate scenario variables into systematic credit stress in a transparent, auditable way.
3. Support scenario analysis and stress testing rather than claim to "predict" climate-driven defaults from first principles.
4. Be implementable even when we lack clean historical observables to statistically estimate climate-credit parameters.

### Key simplification we adopt

We keep one main factor object that drives correlated credit outcomes:
- **F**: a sector–region factor matrix, representing systematic "credit conditions" for each sector and region cell.

Climate enters through:
- **X**: a deterministic vector of climate/scenario variables (e.g., carbon price path, energy prices, hazard intensities), which drives F via a mapping F = m(X) + u.

So the model is:
- Credit outcomes depend on F
- F = m(X) + u where m(X) is deterministic scenario mean, u is stochastic residual
- Idiosyncratic noise provides name-level randomness.

---

## 2. Conceptual overview

### Three layers of the system

1. **Scenario layer (deterministic)**:
   A climate scenario gives a set of driver values X (e.g., carbon price change, energy price shock, physical hazard indices). These are **not random** in the model.

2. **Systematic credit layer (stochastic)**:
   The scenario shifts the mean systematic credit conditions for each sector × region cell. Around that mean, there is a stochastic residual credit environment that creates correlation and tail risk.

3. **Obligor layer (stochastic)**:
   Each obligor loads on the sector × region systematic credit conditions via weights β_i and has an idiosyncratic shock. A single "asset-correlation" knob ρ controls how strongly obligors co-move with systematic conditions overall.

### Why a sector-region cell structure?

The cell-level factor matrix F_{s,r} provides:
- **Natural interpretation**: F_{coal,europe} directly represents "coal sector in Europe"
- **Flexible exposures**: Obligors can load on multiple cells (diversified portfolios)
- **Climate overlay**: Deterministic scenario X drives systematic mean m_{s,r}(X)
- **Residual correlation**: Stochastic u captures non-climate systematic uncertainty via market-calibrated covariance Σ_u

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
Z_i = √ρ · (systematic score) + √(1-ρ) · ε_i
```

- **ρ ∈ (0,1)**: fraction of variance that is systematic ("asset correlation")
- **ε_i**: idiosyncratic noise (standard normal)

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
- It generates default correlation through shared systematic component.
- It can be simulated efficiently (Monte Carlo) and decomposed by risk drivers.
- The single parameter ρ controls tail thickness and diversification benefits.

---

## 4. Our model specification

### 4.1 Index sets and dimensions

**Sectors**: s = 1,...,S (e.g., S=15)
**Regions**: r = 1,...,R (e.g., R=7)
**Obligors**: i = 1,...,N
**Climate drivers**: k = 1,...,K (K=8 in our implementation)

**Number of sector–region cells**:
```
M := S × R    (e.g., M = 105 for 15 sectors × 7 regions)
```

**Indexing convention**: Sector-major (regions vary fastest within sector)
```
j = idx(s,r) := (s-1)R + r
```

So j=1 is (s=1, r=1), j=2 is (s=1, r=2), ..., j=R is (s=1, r=R), j=R+1 is (s=2, r=1), etc.

### 4.2 Factor surface definition

Define the systematic factor matrix:

```
F ∈ ℝ^(S×R)
```

Where F_{s,r} represents the systematic credit condition for sector s in region r.

**Vector form**:
```
f := vec(F) ∈ ℝ^M
```

**Interpretation**:
- F_{coal,europe} represents systematic credit stress for coal companies in Europe
- F_{agriculture,asia} represents systematic credit stress for agriculture in Asia
- These are **latent random variables** conditional on climate scenario

### 4.3 Factor decomposition: Scenario mean + residual

Each cell factor is decomposed as:

```
f = m(X) + u
```

Where:
- **m(X) ∈ ℝ^M**: Deterministic scenario-driven mean (climate overlay)
- **u ∈ ℝ^M**: Stochastic residual ~ N(0, Σ_u) (non-climate systematic uncertainty)

**Key insight**: Climate fixes the mean; Σ_u controls correlated randomness around that mean.

### 4.4 Obligor exposure weights (must sum to 1)

Each obligor i has exposure weights:

```
β_i ∈ ℝ^M
```

With the constraint:

```
β_{i,j} ≥ 0  for all j,    Σ_j β_{i,j} = 1
```

So β_i is a **convex combination** across sector–region cells.

**Single-cell assignment** (special case):
- If obligor i belongs purely to cell (s(i), r(i)), then β_i has a 1 in position idx(s(i),r(i)) and 0 elsewhere.

**Multi-cell assignment** (diversified exposure):
- If obligor i has 60% revenue from coal in Europe, 40% from oil in North America:
  - β_{idx(coal,europe)} = 0.6, β_{idx(oil,NA)} = 0.4, all others = 0

**Data source**: Financial reports, segment disclosures, asset locations

### 4.5 Obligor latent variable with asset correlation ρ

We define the obligor's systematic score:

```
S_i := β_i^T · f
```

Since different β_i imply different systematic variance, we **standardize**:

```
Var(S_i) = β_i^T Σ_u β_i

S̃_i := S_i / √(β_i^T Σ_u β_i)
```

Now S̃_i has unit variance.

Then we introduce a single global systematic share **ρ ∈ (0,1)**:

```
Z_i = √ρ · S̃_i + √(1-ρ) · ε_i,    ε_i ~ N(0,1)
```

**Interpretation of ρ**:
- ρ is the fraction of variance in Z_i that is systematic for every obligor.
- Higher ρ ⇒ stronger default clustering and fatter portfolio tails.
- Typical range: ρ ∈ [0.15, 0.25] calibrated to match portfolio tail behavior.

### 4.6 Default and loss

Default threshold matches baseline PD:

```
Default_i = 1{Z_i < Φ^(-1)(PD_i)}
```

Loss:

```
L_i = Default_i · EAD_i · LGD_i

L = Σ_i L_i
```

---

## 5. Climate overlay: deterministic scenario drivers X

### 5.1 Scenario driver vector X

X is a vector of scenario variables:

```
X ∈ ℝ^K    (K = 8 in our implementation)
```

It is **deterministic** for a chosen scenario and horizon. For NGFS "Net Zero 2050" year 2050, X is fixed.

**Deterministic scenario assumption**:
```
X = μ^(scenario)
```

No randomness is introduced at the climate-driver level.

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

**Crucially**: X does not directly appear in the obligor equation. It determines the **mean** of systematic factors: m(X).

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

### 5.3 Climate-to-credit mapping m(X)

The deterministic climate mean is a linear function of standardized drivers:

**Cellwise**:
```
m_{s,r}(X) = α_{s,r} + Σ_k w_{s,r,k} · φ_k(X)
```

**Vector form**:
```
m(X) = α + W · φ(X)
```

Where:
- **α := vec([α_{s,r}]) ∈ ℝ^M**: Baseline intercept (typically 0 if F represents deviations)
- **W ∈ ℝ^(M×K)**: Sensitivity matrix ("how driver k affects cell j")

**Interpretation**: W φ(X) is the scenario-implied shift in credit conditions across sector×region cells.

### 5.4 Optional "hybrid" structured parameterization for W

Rather than calibrate M×K = 840 weights independently, you can use structured parameterization:

**Step 1**: Define relative exposure scores
- **S_{s,k} ∈ [0,1]**: How exposed sector s is to driver k
- **R_{r,k} ∈ [0,1]**: How exposed region r is to driver k

**Step 2**: Combine via product
```
w_{(s,r),k} = λ_k · S_{s,k} · R_{r,k}
```

Where:
- **λ_k**: Overall scale for driver k

**Interpretation**:
- **S and R** encode relative vulnerabilities (which sectors/regions are most exposed)
- **λ_k** controls overall magnitude (how strongly driver k translates to credit stress)

This reduces calibration from 840 parameters to:
- S matrix: 15 sectors × 8 drivers = 120 scores
- R matrix: 7 regions × 8 drivers = 56 scores
- λ vector: 8 scale parameters
- **Total: 184 parameters** (vs 840), with clear economic interpretation

**Note**: This is **optional but governance-friendly**. The fundamental object is W; the hybrid structure is a convenient parameterization.

---

## 6. Residual systematic uncertainty: Σ_u and correlation structure

### 6.1 The residual covariance matrix

The stochastic residual captures non-climate systematic uncertainty:

```
u ~ N(0, Σ_u)
```

Where **Σ_u ∈ ℝ^(M×M)** is the residual covariance matrix that encodes:
- Inter-sector co-movement
- Inter-region co-movement
- Portfolio tail thickness (together with ρ)
- Diversification benefits

**Key design principle**: We **cannot estimate climate-credit correlations robustly** from historical data. Instead, we:
- Keep climate deterministic (no correlation parameters needed)
- Put stochastic correlations into Σ_u
- **Calibrate Σ_u from observable market data** (equity sector/region indices)

### 6.2 Practical construction of Σ_u from market data

Instead of a full free M×M matrix (5,460 parameters), build it from sector and region correlation blocks:

**Step 1**: Estimate sector and region correlations from market data

- **Corr_S**: Sector correlation matrix (S×S) estimated from sector index returns (e.g., MSCI sector indices)
- **Corr_R**: Region correlation matrix (R×R) estimated from regional index returns (e.g., MSCI regional indices)

**Step 2**: Convert to covariances with chosen volatility scales

```
Σ_S = D_S · Corr_S · D_S
Σ_R = D_R · Corr_R · D_R
```

Where D_S and D_R are diagonal matrices with sector/region volatilities (often start with D_S = D_R = I for simplicity).

**Step 3**: Embed into cell space using membership matrices

Define **membership matrices**:
- **A ∈ ℝ^(M×S)**: Maps each cell to its sector
  - A_{j,s} = 1 if cell j belongs to sector s, else 0
- **B ∈ ℝ^(M×R)**: Maps each cell to its region
  - B_{j,r} = 1 if cell j belongs to region r, else 0

**Step 4**: Mix sector and region components with ω

Define **sector-vs-region mix parameter ω ∈ [0,1]**:
- ω controls whether residual co-movement is primarily sector-cycle or region-cycle driven
- High ω (→ 1): Sector cycles dominate (e.g., energy sector co-moves globally)
- Low ω (→ 0): Regional cycles dominate (e.g., geographic macro conditions matter more)
- Typical starting point: ω = 0.5 (equal weight)

Construct the structured covariance component:
```
Σ_structured = ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)
```

**Step 5**: Mix structured vs cell-specific with η

Define **structured-vs-cell share parameter η ∈ [0,1]**:
- η controls how much of residual variance is structured (via Σ_structured) vs cell-specific
- High η (→ 1): Strong systematic clustering, less diversification benefit
- Low η (→ 0): More cell-specific noise, greater diversification benefit
- Typical starting point: η = 0.7 (moderately systemic)

Construct final residual covariance:
```
Σ_u = η · Σ_structured + (1-η) · I_M
```

Expanding:
```
Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I_M
```

**Knobs**:
- **Corr_S, Corr_R**: Estimated from data (MSCI sector/regional indices, 5-year rolling correlations)
- **D_S, D_R**: Volatility scales (typically set to I for unit volatility)
- **ω**: Sector-vs-region mix (0 = all region, 1 = all sector, 0.5 = equal)
- **η**: Structured-vs-cell share (0 = all cell-specific, 1 = all structured)

### 6.3 What these components do

**Corr_S** (sector correlation):
- Captures how different sectors co-move (e.g., coal and oil&gas move together due to energy policy)
- Typical values: 0.3-0.5 for related sectors, 0.1-0.2 for unrelated sectors
- Estimated from MSCI Global Sector Indices or credit sector sub-indices

**Corr_R** (region correlation):
- Captures how different regions co-move (e.g., Europe and Asia linked by trade)
- Typical values: 0.2-0.4 for developed regions, 0.3-0.5 for emerging regions
- Estimated from MSCI Regional Indices or regional GDP growth correlations

**D_S, D_R** (volatility scales):
- Controls overall strength of sector/region components
- Typically set to I (unit volatility) for simplicity
- Alternative: Use empirical std devs if sector/region-specific volatilities are desired

**ω** (sector-vs-region mix):
- Controls whether residual co-movement is driven by sector cycles or regional cycles
- ω = 1: Pure sector co-movement (coal in Europe and coal in Asia move together)
- ω = 0: Pure regional co-movement (coal in Europe and oil in Europe move together)
- ω = 0.5: Balanced mix of both
- **Calibration**: Start with 0.5, adjust based on portfolio composition and risk decomposition
- **Sensitivity**: Test ω ∈ {0.3, 0.5, 0.7} to understand sector vs region dominance

**η** (structured-vs-cell share):
- Controls how much residual variance is systematic vs cell-specific
- η = 1: All residual variance is structured (maximum clustering, minimum diversification)
- η = 0: All residual variance is cell-specific (maximum diversification, no clustering)
- η = 0.7: 70% structured, 30% cell-specific (moderately systemic)
- **Calibration**: Start with 0.7, adjust to match observed tail thickness and diversification
- **Sensitivity**: Test η ∈ {0.5, 0.7, 0.9} to understand impact on VaR and ES
- **Economic interpretation**: η is like an "intra-cell correlation" - higher η means cells are more correlated beyond sector/region patterns

### 6.4 Induced correlation structure

The construction Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I induces:

**Within-sector correlation** (same sector, different regions):
```
Corr(u_{s,r}, u_{s,r'}) = (η · ω · Sector variance component) / (Total variance)
```
- Controlled by η (strength of structure) and ω (sector vs region weight)
- Higher η and ω → stronger within-sector correlation

**Within-region correlation** (different sectors, same region):
```
Corr(u_{s,r}, u_{s',r}) = (η · (1-ω) · Region variance component) / (Total variance)
```
- Controlled by η and (1-ω)
- Higher η and lower ω → stronger within-region correlation

**Cross-sector-region correlation** (different sector AND region):
```
Corr(u_{s,r}, u_{s',r'}) ≈ 0  (by construction, no direct linkage)
```

**Diagonal variance** (variance of each cell):
```
Var(u_{s,r}) = η · [ω · (A Σ_S A^T)_{s,r} + (1-ω) · (B Σ_R B^T)_{s,r}] + (1-η)
```
- With D_S = D_R = I, the diagonal is approximately η + (1-η) = 1 (unit variance cells)

**Key advantages**:
- **Observable data**: Corr_S and Corr_R estimated from equity/credit market indices
- **Parsimonious**: Only 4 key knobs (ω, η, plus Corr_S and Corr_R structures)
- **Interpretable**: ω controls sector vs region, η controls clustering strength
- **Separable**: Can tune sector-region balance (ω) independently from overall clustering (η)
- **Stable simulation**: Well-conditioned by construction, positive definite

---

## 7. The correlation levers (intuitive summary)

There are five distinct "correlation levers" and they do different jobs:

**Lever 1: Corr_S and Corr_R (correlation patterns)**
- "Which sectors co-move? Which regions co-move?"
- Estimated from market data (MSCI sector/regional indices)
- Provides the underlying correlation structure

**Lever 2: ω (sector-vs-region mix)**
- "Is residual co-movement driven by sector cycles or regional cycles?"
- ω = 1: Sector dominates (e.g., global energy sector cycle)
- ω = 0: Region dominates (e.g., regional macro conditions)
- Typical starting point: ω = 0.5 (balanced)

**Lever 3: η (structured-vs-cell share)**
- "How much residual variance is systematic vs cell-specific?"
- Higher η ⇒ stronger clustering, less diversification, fatter tails
- Lower η ⇒ more cell-specific noise, more diversification
- Typical starting point: η = 0.7 (moderately systemic)

**Lever 4: ρ (asset correlation at obligor level)**
- "How strongly do obligors respond to systematic conditions vs idiosyncratic noise?"
- Higher ρ ⇒ stronger default clustering, fatter tails
- Lower ρ ⇒ more idiosyncratic behavior, thinner tails
- Typical range: ρ ∈ [0.15, 0.25]

**Lever 5: β_i (who is exposed to which cells)**
- "Which obligors are linked to which sector×region conditions?"
- From portfolio data (sector/region assignments, revenue splits)

**Climate scenarios do not appear as correlations**; they appear as mean shifts m(X).

That keeps the model auditable:
- Climate scenario assumptions are fully visible in X, σ_k, and W
- Correlation assumptions are fully visible in Corr_S, Corr_R, ω, η, and ρ

---

## 8. The calibration "knobs": what you must choose/estimate

Think of knobs in four groups.

### A) Scenario knobs (deterministic)

**(A1) Scenario selection**
- **Knob**: Choice of scenario (NGFS pathway, internal pathway)
- **Effect**: Sets X = μ^(scenario) and therefore the mean stress surface m(X)

**(A2) Driver definitions (X_k)**
- **Knob**: Which drivers you include (carbon price, energy price, flood index, etc.)
- **Effect**: Determines what kinds of climate effects you can express
- **Design note**: Can start "global" and later add more drivers without changing engine

**(A3) Feature scaling σ_k**
- **Knob**: "Typical move" for each driver
- **Effect**: Defines how big φ_k(X) is; scales scenario impact for a given W
- **Practical role**: Interpretability ("1σ carbon shock") and stability across scenarios

### B) Climate→credit transmission knobs (mean mapping)

**(B1) Sensitivity matrix W** (or structured scores)
- **Knob**: W_{(s,r),k} or the score libraries S_{s,k}, R_{r,k}
- **Effect**: Controls where climate stress lands (which sectors/regions) and with what sign

**(B2) Driver strength scalars λ_k** (if using structured W)
- **Knob**: λ_k
- **Effect**: Overall magnitude of impact of driver k on credit factors
- **Calibration style**: "Reasonableness targets" (most exposed cells move ~1–2σ under severe scenario)

**(B3) Intercept α**
- **Knob**: α_{s,r} (usually set to 0)
- **Effect**: Baseline offset; cleaner to set α=0 and let baseline be zero-stress state

### C) Residual credit correlation knobs

**(C1) Sector correlation Corr_S**
- **Knob**: (S×S) correlation matrix
- **Effect**: Controls how different sectors co-move
- **Calibration**: Estimated from MSCI sector indices (e.g., 5-year rolling correlations)

**(C2) Region correlation Corr_R**
- **Knob**: (R×R) correlation matrix
- **Effect**: Controls how different regions co-move
- **Calibration**: Estimated from MSCI regional indices

**(C3) Volatility scales D_S, D_R**
- **Knob**: Diagonal matrices
- **Effect**: Overall strength of sector/region components in residual systematic risk
- **Start**: D_S = D_R = I (unit volatility)

**(C4) Sector-vs-region mix ω**
- **Knob**: ω ∈ [0,1]
- **Effect**: Controls whether residual co-movement is sector-driven or region-driven
- **Interpretation**:
  - ω = 1: Pure sector co-movement
  - ω = 0: Pure regional co-movement
  - ω = 0.5: Balanced
- **Typical range**: ω ∈ [0.3, 0.7]
- **Calibration**: Start with 0.5, adjust based on portfolio sector vs geographic concentration

**(C5) Structured-vs-cell share η**
- **Knob**: η ∈ [0,1]
- **Effect**: Controls how much residual variance is structured vs cell-specific
- **Interpretation**:
  - η = 1: All variance is structured (maximum clustering)
  - η = 0: All variance is cell-specific (maximum diversification)
  - η = 0.7: Moderately systemic (70% structured, 30% cell-specific)
- **Typical range**: η ∈ [0.5, 0.9]
- **Calibration**: Start with 0.7, adjust to match portfolio tail thickness and diversification behavior

### D) Obligor-level "systematic strength" knob

**(D1) Global ρ**
- **Knob**: ρ ∈ (0,1)
- **Effect**: Determines how much of each obligor's latent credit index is systematic vs idiosyncratic
  - Higher ρ ⇒ more clustering, fatter tails
  - Lower ρ ⇒ more independence, thinner tails
- **Typical range**: ρ ∈ [0.15, 0.25]
- **Extension**: Later, can be sector-specific ρ_s if needed

### E) Credit inputs (portfolio primitives)

These are standard:
- **PD_i**: Baseline unconditional PD (horizon-specific)
- **LGD_i**: Loss given default
- **EAD_i**: Exposure at default
- **β_i**: Exposure weights across sector–region cells (sum to 1)

### F) Simulation and reporting knobs

- **# simulations**: N_sim (accuracy vs runtime)
- **Confidence levels**: VaR/ES percentile (e.g., 99%, 99.9%)
- **Decomposition granularity**: Sector-only vs sector×region contributions

---

## 9. Key design choices we made (and why)

### Choice 1: Cell-level factors F_{s,r}

We use a **sector-region cell matrix** F ∈ ℝ^(S×R):
- **Natural interpretation**: F_{coal,europe} directly represents "coal in Europe"
- **Flexible loadings**: Obligors can have multi-cell exposures (diversified revenue)
- **Direct climate mapping**: Each cell responds to climate drivers

### Choice 2: Decomposition into climate mean + residual

**Climate mean m(X)**: Deterministic, scenario-driven
- Selecting "NGFS Net Zero 2050" fixes m(X) completely
- Linear mapping: m(X) = α + W φ(X)
- Matches stress testing convention: scenarios are narratives, not forecasts

**Stochastic residual u ~ N(0, Σ_u)**: Random, captures non-climate systematic risk
- Constructed from market-observable sector/region correlations
- Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I
- Grounded in equity market data, not climate-credit speculation
- Two intuitive knobs: ω (sector vs region) and η (structured vs cell-specific)

This separation makes uncertainty quantification clean: scenario risk (deterministic X) vs. conditional risk (random u).

### Choice 3: Market-calibrated Σ_u with intuitive mixing parameters

**Key innovation**: We **cannot estimate climate-credit correlations** from historical data.

Instead:
- Estimate Corr_S and Corr_R from **observable equity/credit market data** (MSCI indices)
- Mix sector and region components with ω (sector-vs-region balance)
- Mix structured vs cell-specific with η (clustering strength)
- Only 4 key knobs: Corr_S, Corr_R, ω, η (instead of 5,460 covariance parameters)

**Benefits**:
- **Empirically grounded**: Uses 20+ years of equity market data
- **Intuitive knobs**: ω and η have clear economic interpretations
- **Separable**: Can tune sector/region balance independently from clustering strength
- **Stable**: Well-conditioned by construction, positive definite
- **Governance-friendly**: Easy to explain and validate

### Choice 4: Single ρ parameter (asset correlation)

Rather than let each obligor have different systematic loading strength, we:
- Standardize systematic scores: S̃_i = S_i / √(β_i^T Σ_u β_i)
- Apply single global ρ: Z_i = √ρ S̃_i + √(1-ρ) ε_i

**Benefits**:
- **Simplicity**: One global knob for tail thickness
- **Familiarity**: ρ is standard in Basel/internal models
- **Sensitivity**: Easy to report results for ρ ∈ [0.15, 0.25] range

### Choice 5: Climate affects credit only through systematic factors

No direct climate term in Z_i. All climate impact flows through f:
```
Z_i = √ρ · (β_i^T f / √(β_i^T Σ_u β_i)) + √(1-ρ) · ε_i
where f = m(X) + u
```

**Implications**:
- Climate does not create within-cell differentiation (unless β differs)
- Cross-sectional variation comes from sector-region assignments
- All climate-driven correlation flows through systematic factors

This is appropriate for portfolio-level stress testing where obligor-specific climate data is sparse.

---

## 10. Calibration workflow (practical "start building" plan)

### Step 1: Define taxonomy
- Choose S sectors and R macro-regions
- Define mapping from obligors to β_i weights (summing to 1)

### Step 2: Choose climate driver set X and scaling σ_k
- Start with a small global driver set (3–6)
- Define σ_k for each driver (cross-scenario standard deviation)

### Step 3: Specify climate-to-credit mapping
- Either define W directly or use structured S_{s,k}, R_{r,k} and calibrate λ_k
- Set α = 0 (baseline is zero-stress state)

### Step 4: Build residual covariance Σ_u
- **Download MSCI sector indices** (15 sectors, 5-year rolling returns)
- **Compute Corr_S** (15×15 sector correlation matrix)
- **Download MSCI regional indices** (7 regions, 5-year rolling returns)
- **Compute Corr_R** (7×7 region correlation matrix)
- Apply shrinkage if needed for stability (e.g., Ledoit-Wolf)
- Set D_S = D_R = I (unit volatility)
- **Choose ω** (sector-vs-region mix, start with 0.5)
- **Choose η** (structured-vs-cell share, start with 0.7)
- Construct membership matrices A (M×S) and B (M×R)
- Construct Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I

### Step 5: Choose ρ
- Pick a single global ρ (start with 0.20)
- Run sensitivity bands: ρ ∈ [0.15, 0.25]

### Step 6: Run Monte Carlo and report
- Baseline and scenario loss distributions
- Contributions by sector and region
- Sensitivities to ρ, ω, η, and λ_k

---

## 11. Simulation algorithm (single horizon)

Given scenario X = μ^(scenario):

1. Compute features φ(X)
2. Compute deterministic mean m(X) = α + W φ(X)
3. Draw residual systematic factor u ~ N(0, Σ_u)
4. Form f = m(X) + u
5. For each obligor i:
   - Compute systematic score S_i = β_i^T f
   - Standardize S̃_i = S_i / √(β_i^T Σ_u β_i)
   - Draw ε_i ~ N(0,1)
   - Compute Z_i = √ρ S̃_i + √(1-ρ) ε_i
   - Set Default_i = 1{Z_i < Φ^(-1)(PD_i)}
   - Compute L_i = Default_i · EAD_i · LGD_i
6. Aggregate L = Σ_i L_i

Repeat (3)–(6) to obtain the loss distribution under the scenario.

---

## 12. What this model can and cannot claim

### What it can do well

- Produce coherent loss distributions under climate scenarios.
- Attribute incremental risk to sector and region segments in a controlled way.
- Provide transparent, explainable linkage from scenario variables to credit stress.
- Support stress testing, risk appetite discussion, and portfolio steering.
- Use market-observable data (sector/region correlations) rather than speculative climate-credit correlations.

### What it should not claim (without further development)

- Precise prediction of obligor defaults driven by climate.
- Fine-grained within-sector differentiation unless β_i includes richer exposure splits.
- Causal inference from historical climate to credit outcomes.

---

## 13. One-paragraph "model story" (for stakeholders)

Midas takes climate scenarios as deterministic narratives expressed through a small set of global driver variables X. These drivers shift the mean systematic credit conditions for each sector–region cell via a transparent mapping m(X). Around that scenario-implied mean, we add a stochastic residual systematic credit environment u whose correlations across sectors and regions are calibrated using observable market co-movement (sector and macro-region indices). Obligors load on the sector–region surface via exposure weights β_i, and a single parameter ρ controls how strongly obligors co-move with systematic conditions versus idiosyncratic noise. This produces coherent loss distributions under scenarios without claiming to estimate causal climate–default relationships from limited historical data.

---

## 14. Notation Reference

| Symbol | Description |
|--------|-------------|
| X | Deterministic climate/scenario variable vector |
| F | Sector–region factor matrix (systematic credit conditions) |
| m(X) | Deterministic scenario-driven mean of factor surface |
| u | Stochastic residual ~ N(0, Σ_u) |
| Σ_u | Residual covariance matrix (M×M) |
| Corr_S | Sector correlation matrix (S×S), estimated from MSCI sector indices |
| Corr_R | Region correlation matrix (R×R), estimated from MSCI regional indices |
| A | Membership matrix (M×S), maps cells to sectors |
| B | Membership matrix (M×R), maps cells to regions |
| ω | Sector-vs-region mix parameter ∈ [0,1] (ω=1: all sector, ω=0: all region) |
| η | Structured-vs-cell share parameter ∈ [0,1] (η=1: all structured, η=0: all cell-specific) |
| ρ | Asset correlation (fraction of variance that is systematic) |
| Z_i | Latent creditworthiness index for obligor i |
| S_i | Systematic score for obligor i: β_i^T f |
| S̃_i | Standardized systematic score: S_i / √(β_i^T Σ_u β_i) |
| β_i | Factor loadings/exposures for obligor i (sum to 1) |
| ε_i | Idiosyncratic noise for obligor i ~ N(0,1) |
| τ_i | Default threshold for obligor i: Φ^(-1)(PD_i) |
| Φ | Standard normal cumulative distribution function |
| PD_i | Probability of default for obligor i |
| LGD_i | Loss given default for obligor i |
| EAD_i | Exposure at default for obligor i |
| W | Sensitivity matrix (M×K) |
| φ(X) | Standardized feature transformation of climate variables |
| S_{s,k} | Sector exposure score for sector s to driver k (optional hybrid structure) |
| R_{r,k} | Region exposure score for region r to driver k (optional hybrid structure) |
| λ_k | Scale parameter for driver k (optional hybrid structure) |
| σ_k | Standardization scale for driver k |

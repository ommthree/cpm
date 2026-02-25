# Credit Portfolio Model - Implementation Plan

## Overview

This plan breaks the implementation into 5 technical phases, with each phase producing concrete deliverables. The plan follows a "crawl-walk-run" approach: start with a minimal viable model, then add sophistication incrementally.

**Timeline estimate:** 16-18 weeks for v1.0
**Focus:** Pure technical implementation (C++ engine + React dashboard)

### Phase Summary

| Phase | Timeline | Key Deliverables |
|-------|----------|------------------|
| **1. Portfolio Data Preparation** | Weeks 1-3 | Clean portfolio data, sector-region mappings, exposure heatmap, CSV templates |
| **2. Scenario Data Preparation** | Weeks 2-4 | NGFS scenarios processed, standardized driver database φ(C) |
| **3. Sensitivity Calibration** | Weeks 5-8 | Sector/region scores (S, R), calibrated scale parameters (λ) |
| **4. C++ Engine Implementation** | Weeks 7-12 | Working simulation engine with REST API, return/risk analytics, unit tests |
| **5. React Frontend Development** | Weeks 10-15 | Dashboard with visualizations, scenario runner, results explorer |
| **6. Validation & Analysis** | Weeks 13-18 | Validation report, full scenario analysis, uncertainty quantification |

*Note: Phases overlap where prerequisites are met*

---

## Phase 1: Portfolio Data Preparation (Weeks 1-3)

### Objectives
Clean, standardize, and enrich portfolio data with sector/region mappings and climate-relevant attributes.

### Prerequisites
- Access to portfolio data (obligor-level EAD, PD, LGD)
- Development environment set up (Python 3.9+, Jupyter, Git)

### Tasks

#### 1.1 Portfolio Data Ingestion
- [x] Load obligor-level exposure data (EAD, PD, LGD)
- [x] Validate data quality (completeness, consistency)
- [x] Handle missing values and outliers
- [x] Create unique obligor identifiers

#### 1.2 Sector Classification
- [x] Define sector taxonomy (start with 10-15 sectors, expandable)
- [x] Map each obligor to primary sector
- [x] Validate sector assignments (sample audit)
- [x] Document sector definitions and mapping rules

**Sector taxonomy (initial):**
1. Energy - Oil & Gas
2. Energy - Coal
3. Energy - Renewables
4. Utilities
5. Transportation
6. Manufacturing - Heavy Industry
7. Manufacturing - Light Industry
8. Agriculture
9. Real Estate & Construction
10. Financial Services
11. Technology & Services
12. Consumer Goods
13. Healthcare
14. Mining & Metals
15. Other

#### 1.3 Region Classification
- [x] Define region taxonomy (start with 6-8 regions)
- [x] Map each obligor to primary region (by HQ, revenue, or assets)
- [x] Consider multi-region weighting for large corporates
- [x] Validate region assignments

**Region taxonomy (initial):**
1. North America
2. Europe
3. Asia-Pacific (developed)
4. Asia-Pacific (emerging)
5. Latin America
6. Middle East & Africa
7. Global (for multinational obligors)

#### 1.4 Exposure Mapping
- [x] Create sector-region grid (S × R = 15 × 7 = 105 cells)
- [x] Compute total EAD by cell
- [x] Compute weighted average PD by cell
- [x] Identify concentration risk (cells with >5% of EAD)

#### 1.5 Data Enrichment (Optional but Recommended)
- [x] Collect emissions data (Scope 1, 2, 3 if available) - **Used synthetic data**
- [x] Collect revenue geography breakdowns (for multi-region β) - **Used synthetic data**
- [ ] Collect physical asset locations - **Deferred to real portfolio**
- [x] Create obligor-level climate attributes database - **Synthetic portfolio created**

#### 1.6 CSV Template Creation
- [x] Create portfolio CSV template (columns: ObligorID, Name, Sector, Region, PD, LGD, EAD, ...)
- [x] Create scenario CSV template (columns: ScenarioName, Year, Driver, Region, Value)
- [x] Create calibration CSV templates (sector scores, region scores, λ parameters)
- [x] Document CSV formats and validation rules

### Deliverables
✅ Clean portfolio dataset with obligor-level PD, LGD, EAD
✅ Sector and region taxonomies with mapping rules documented
✅ Sector-region exposure heatmap (EAD by cell)
✅ Portfolio concentration report
✅ Data quality report (coverage, completeness)
✅ CSV templates for all data inputs

### Success Criteria
- >95% of EAD mapped to sector and region ✅
- No critical data quality issues identified ✅

### Status: ✅ COMPLETE (Week 1)
**Actual completion:** February 25, 2026
**Notes:** Generated synthetic portfolio (500 obligors, $5.8B EAD) for initial development. Real portfolio data can be loaded via CSV templates when available.

---

## Phase 2: Scenario Data Preparation (Weeks 2-4)

### Objectives
Extract, transform, and standardize NGFS climate scenarios into model-ready driver variables C.

### Prerequisites
- Access to NGFS scenario data (download from NGFS portal or IIASA database)
- Access to physical risk data sources (World Bank, EM-DAT, or commercial providers)

### Tasks

#### 2.1 NGFS Scenario Selection
- [x] Review NGFS scenario suite (current version: Phase V)
- [x] Select 4-6 scenarios for implementation:
  - Baseline: Current Policies ✅
  - Orderly transition: Net Zero 2050 ✅
  - Disorderly transition: Delayed Transition ✅
  - Physical risk: Current Policies (high warming) ✅
  - Optional: Divergent Net Zero, Low demand - **Deferred**
- [x] Document scenario narratives

#### 2.2 Transition Driver Extraction
- [x] Extract carbon price paths by region - **NGFS Phase V IAM data**
- [x] Extract energy price indices (fossil fuels, electricity) - **Coal, Oil, Gas prices extracted**
- [x] Extract GDP/output growth paths - **GDP PPP extracted**
- [x] Extract policy stringency indicators (if available) - **Captured via carbon price**

#### 2.3 Physical Driver Construction
- [x] Extract temperature anomaly paths by region - **World Bank CCKP hi35 (heat index)**
- [x] Extract precipitation anomaly paths by region - **World Bank CCKP rx1day (max precip)**
- [x] Construct or source physical hazard indices:
  - [x] Flood severity index by region - **rx1day indicator**
  - [ ] Storm/cyclone severity index by region - **Deferred (data not readily available)**
  - [x] Heat stress index by region - **hi35 indicator**
  - [x] Drought index by region - **cdd (consecutive dry days) indicator**
- [x] Align all data to common time grid (annual, 2025-2050) - **Mapped to 2030/2050**

#### 2.4 Driver Consolidation
- [x] Define final driver set C (recommend 5-7 variables):
  1. Carbon price shock (transition) ✅
  2. Fossil energy price shock (transition) - **Split into Coal/Oil/Gas ✅**
  3. GDP deviation from baseline (transition/macro) ✅
  4. Flood severity index (physical) ✅
  5. Storm severity index (physical) - **Deferred**
  6. Heat/drought severity index (physical) - **Split into HeatIndex/DroughtRisk ✅**
  7. Optional: Policy stringency score - **Captured via carbon price**
- [x] Create driver database with scenario × year × driver × region structure
- **Final: 8 drivers (5 transition + 3 physical), 546 data points**

#### 2.5 Driver Standardization (φ transformation)
- [x] Compute baseline values C_baseline for each driver - **Used Current Policies scenario**
- [x] Compute scenario deviations ΔC = C_scenario - C_baseline
- [x] Define standardization scales σ_k using cross-scenario dispersion
- [x] Compute standardized features φ_k = ΔC_k / σ_k
- [x] Document standardization methodology

### Deliverables
✅ Scenario narrative summary (1-2 pages per scenario)
✅ Raw NGFS data extracts (transition variables)
✅ Physical hazard index database
✅ Consolidated driver database C (scenario × year × driver × region)
✅ Standardized driver database φ(C) with σ_k documentation
✅ Driver visualization notebook (time series plots, scenario comparisons)

### Success Criteria
- All scenarios have complete driver coverage (no missing values) ✅
- Standardization produces φ values in interpretable range (-3 to +3 σ) ✅

### Status: ✅ COMPLETE (Week 1)
**Actual completion:** February 25, 2026
**Data sources:**
- Transition: NGFS Phase V (November 2024) via IIASA - 420 rows
- Physical: World Bank CCKP CMIP6 (ACCESS-CM2 model) - 126 rows
**Total:** 546 data points (8 drivers × 7 regions × 3 scenarios)

**Key files created:**
- `data/scenarios/net_zero_2050.csv` (182 rows)
- `data/scenarios/delayed_transition.csv` (182 rows)
- `data/scenarios/current_policies.csv` (182 rows)
- `data/scenarios/physical_risk_full.csv` (126 rows)
- `data/scenarios/process_physical_risk.py` (NetCDF processing script)
- `data/scenarios/README.md` (comprehensive documentation)

---

## Phase 3: Sensitivity Calibration (Weeks 5-8)

### Objectives
Calibrate the climate-to-credit transmission parameters (S, R, λ) and residual covariance structure (Σ_u) from market data for the cell-level factor model with climate overlay.

### Prerequisites
- Completed Phase 1 (portfolio data with sector/region mappings)
- Completed Phase 2 (standardized climate drivers φ(X))
- Updated model documentation (cell-level factors with market-calibrated Σ_u)
- Access to MSCI sector/regional indices or equivalent equity market data
- **Data Sources**: See `/docs/data_sources.md` for comprehensive documentation of all external data (NGFS, World Bank CCKP, Yahoo Finance)

### Tasks

#### 3.1 Sector Exposure Scores S (15 × 8)
- [ ] For each sector s (15 sectors) and driver k (8 drivers), assign S_{s,k} ∈ [0,1]:

  **Transition drivers** (CarbonPrice, CoalPrice, OilPrice, GasPrice, GDP):
  - Coal: S_{coal,carbon} = 1.0 (max carbon exposure), S_{coal,GDP} = 0.7 (cyclical)
  - Oil & Gas: S_{oil,carbon} = 0.9, S_{oil,oil} = 1.0
  - Renewables: S_{renewables,carbon} = 0.2 (benefit from carbon price, inverted)
  - Heavy Manufacturing: S_{heavy,carbon} = 0.8, S_{heavy,GDP} = 0.9
  - Agriculture: S_{agri,carbon} = 0.3, S_{agri,GDP} = 0.6

  **Physical drivers** (HeatIndex, FloodRisk, DroughtRisk):
  - Agriculture: S_{agri,drought} = 0.9, S_{agri,flood} = 0.7, S_{agri,heat} = 0.8
  - Real Estate: S_{real_estate,flood} = 0.8, S_{real_estate,heat} = 0.5
  - Transportation: S_{transport,flood} = 0.6, S_{transport,heat} = 0.4
  - Tech/Services: S_{tech,physical} ≈ 0.1-0.2 (low physical exposure)

- [ ] Document rationale for each score (emissions intensity, asset type, supply chain)
- [ ] Create sector-driver heatmap visualization
- [ ] Validate against available data (CDP disclosures, sector reports)

**Data sources:**
- Transition: Emissions intensity, energy intensity, abatement cost curves
- Physical: Asset heaviness, fixed-site dependence, business interruption sensitivity

#### 3.2 Region Exposure Scores R (7 × 8)
- [ ] For each region r (7 regions) and driver k (8 drivers), assign R_{r,k} ∈ [0,1]:

  **Transition drivers**:
  - Europe: R_{europe,carbon} = 0.9 (high policy stringency), R_{europe,GDP} = 0.8
  - North America: R_{NA,carbon} = 0.7, R_{NA,GDP} = 0.8
  - Asia-Pacific (emerging): R_{asia_em,GDP} = 1.0 (most cyclical)
  - Middle East & Africa: R_{MEA,oil} = 1.0 (oil-dependent economies)

  **Physical drivers** (use World Bank CCKP data for validation):
  - Asia-Pacific (emerging): R_{asia_em,heat} = 1.0, R_{asia_em,flood} = 0.9
  - Middle East & Africa: R_{MEA,heat} = 0.95, R_{MEA,drought} = 1.0
  - Europe: R_{europe,heat} = 0.6, R_{europe,flood} = 0.5
  - North America: R_{NA,flood} = 0.7 (hurricanes, flooding)

- [ ] Document rationale (climate zones, historical hazards, policy stance)
- [ ] Validate against World Bank physical risk data (actual φ values from Phase 2)
- [ ] Create region-driver heatmap visualization

**Data sources:**
- Physical: World Bank CCKP, EM-DAT historical disasters, coastal exposure
- Transition: OECD policy tracker, energy mix, GDP structure

#### 3.3 Global Scale Parameters λ (8 values)
- [ ] Initialize all λ_k = 1.0 as starting point
- [ ] Define anchor targets for calibration:

  **Factor magnitude targets**:
  - Max exposed cell under severe scenario: |m_{s,r}| ≈ 2σ
  - Median cell under moderate scenario: |m_{s,r}| ≈ 0.5-1σ

  **Portfolio-level targets** (to be defined based on expert judgment):
  - EL uplift under Net Zero 2050: 2x-5x baseline EL
  - VaR₉₉ uplift: <20% of portfolio EAD
  - Coal + Oil&Gas contribution to Net Zero loss: >50%

- [ ] Iterative calibration process:
  1. Compute m_{s,r}(X) = Σ_k λ_k · S_{s,k} · R_{r,k} · φ_k(X) for each scenario
  2. Check max |m_{s,r}| against targets
  3. If max too high/low: scale λ_k proportionally
  4. Check which cells contribute most (should match intuition)
  5. Adjust individual λ_k if needed

- [ ] Document final λ values with rationale
- [ ] Perform sensitivity analysis: vary each λ_k by ±30%, measure impact on:
  - Max/median factor values
  - Portfolio EL/VaR
  - Sector contributions

**Expected ranges:** λ_k ∈ [0.5, 1.2] (most drivers), with adjustments based on validation

#### 3.4 Residual Covariance Σ_u from Market Data

**Reference**: See `/docs/data_sources.md` (Part 2: Market Correlation Data) for comprehensive mapping between our 15-sector, 7-region taxonomy and available market indices. That section provides:
- Detailed sector-to-ETF mappings (downloaded from Yahoo Finance)
- Regional index mappings (7 regions, complete data)
- Data status and quality notes
- Processing requirements for correlation matrices

**Step 1**: Download and process equity sector index data
- [ ] Download MSCI Global Sector Indices (or equivalent):
  - Energy, Materials, Industrials, Consumer Discretionary, Consumer Staples,
    Health Care, Financials, Information Technology, Communication Services,
    Utilities, Real Estate
  - 5-10 year history, monthly returns
- [ ] Map MSCI sectors to our 15-sector taxonomy:
  - Create mapping table (e.g., MSCI Energy → our Coal, Oil&Gas, Renewables)
  - Document any aggregation/splitting decisions
- [ ] Compute sector correlation matrix Corr_S (15×15):
  - Use 5-year rolling window for robustness
  - Consider residualizing against market factor (optional)
  - Apply shrinkage if needed for stability (Ledoit-Wolf)
- [ ] Set sector volatilities D_S (15×1 diagonal):
  - Start with D_S = I (unit volatility)
  - Alternative: Use empirical std devs if sector-specific volatilities desired
- [ ] Compute sector covariance: Σ_S = D_S · Corr_S · D_S

**Step 2**: Download and process equity regional index data
- [ ] Download MSCI Regional Indices (or equivalent):
  - North America (USA, Canada)
  - Europe (Developed)
  - Pacific (Japan, Australia, Korea)
  - Emerging Asia (China, India, ASEAN)
  - Latin America
  - Middle East & Africa
  - World/Global
- [ ] Map to our 7-region taxonomy
- [ ] Compute region correlation matrix Corr_R (7×7):
  - 5-year rolling window
  - Consider residualizing against global market
  - Apply shrinkage if needed
- [ ] Set region volatilities D_R (7×1 diagonal):
  - Start with D_R = I (unit volatility)
- [ ] Compute region covariance: Σ_R = D_R · Corr_R · D_R

**Step 3**: Define membership matrices
- [ ] Construct A ∈ ℝ^(M×S) where M = S×R = 105:
  - A_{j,s} = 1 if cell j belongs to sector s, else 0
  - Using sector-major indexing: j = (s-1)R + r
- [ ] Construct B ∈ ℝ^(M×R):
  - B_{j,r} = 1 if cell j belongs to region r, else 0

**Step 4**: Choose sector-vs-region mix ω
- [ ] Define ω ∈ [0,1]: controls whether residual co-movement is sector-driven or region-driven
  - ω = 1: Pure sector co-movement (e.g., global energy sector cycle)
  - ω = 0: Pure regional co-movement (e.g., regional macro conditions)
  - ω = 0.5: Balanced (typical starting point)
- [ ] **Start with ω = 0.5** (equal weight to sector and region)
- [ ] Rationale: Most portfolios have both sector-specific and region-specific concentrations
- [ ] Consider portfolio composition:
  - If concentrated in few sectors: higher ω (e.g., 0.6-0.7)
  - If geographically concentrated: lower ω (e.g., 0.3-0.4)
- [ ] Perform sensitivity analysis: ω ∈ {0.3, 0.5, 0.7}

**Step 5**: Choose structured-vs-cell share η
- [ ] Define η ∈ [0,1]: controls how much residual variance is structured vs cell-specific
  - η = 1: All variance is structured (maximum clustering, minimum diversification)
  - η = 0: All variance is cell-specific (maximum diversification, no clustering)
  - η = 0.7: Moderately systemic (typical starting point)
- [ ] **Start with η = 0.7** (70% structured, 30% cell-specific)
- [ ] Rationale: Credit markets exhibit moderate clustering (not perfectly diversifiable, not perfectly clustered)
- [ ] Economic interpretation: η is like "intra-cell correlation" - higher η means less diversification benefit
- [ ] Perform sensitivity analysis: η ∈ {0.5, 0.7, 0.9}
  - Check impact on VaR, ES (tail risk increases with η)
  - Check diversification benefit (decreases with η)

**Step 6**: Construct full residual covariance
- [ ] Compute structured component:
  - Σ_structured = ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)
- [ ] Compute full covariance:
  - Σ_u = η · Σ_structured + (1-η) · I_M
- [ ] Verify Σ_u is positive definite (should be by construction)
- [ ] Compute induced correlations:
  - Within-sector correlation: η · ω · (component from Corr_S)
  - Within-region correlation: η · (1-ω) · (component from Corr_R)
  - Check these are reasonable given ω and η choices
- [ ] Check total variance: Tr(Σ_u)/M should be close to 1 (unit variance cells)

**Step 7**: Validate correlation structure
- [ ] Compare induced correlations to empirical targets:
  - Within-sector correlations should reflect Corr_S structure scaled by η · ω
  - Within-region correlations should reflect Corr_R structure scaled by η · (1-ω)
- [ ] Perform eigenvalue analysis: All eigenvalues > 0, no excessive concentration
- [ ] Document chosen ω and η with rationale
- [ ] Document sensitivity analysis results

**Data sources:**
- MSCI indices (preferred): https://www.msci.com/end-of-day-data-search
- Alternative: S&P sector indices, FTSE sector indices
- Alternative: Credit sector sub-indices (iTraxx, CDX) if equity data unavailable

#### 3.8 Asset Correlation Parameter ρ
- [ ] Choose global asset correlation ρ ∈ (0,1):
  - **Start with ρ = 0.20** (20% of variance is systematic)
  - Typical range in credit models: ρ ∈ [0.15, 0.25]
  - Higher ρ → fatter tails, more default clustering
  - Lower ρ → thinner tails, more diversification
- [ ] Perform sensitivity analysis: ρ ∈ {0.15, 0.18, 0.20, 0.23, 0.25}
  - Check impact on VaR, ES, loss distribution quantiles
  - Document sensitivity ranges for reporting
- [ ] (Optional) Consider sector-specific ρ_s in future enhancement
- [ ] Document rationale for chosen ρ and sensitivity ranges

### Deliverables
✅ Sector exposure score matrix S (15 × 8) with documented rationale
✅ Region exposure score matrix R (7 × 8) with documented rationale
✅ Global scale parameters λ (8 values) with calibration report
- [ ] Market data calibration report:
  - MSCI sector index data (15 sectors, 5-10 years)
  - MSCI regional index data (7 regions, 5-10 years)
  - Sector correlation matrix Corr_S (15×15)
  - Region correlation matrix Corr_R (7×7)
  - Membership matrices A (M×S) and B (M×R)
  - Sector-vs-region mix ω (chosen value and rationale)
  - Structured-vs-cell share η (chosen value and rationale)
  - Full residual covariance matrix Σ_u (M×M)
- [ ] Asset correlation parameter ρ with sensitivity analysis
✅ Calibration CSV templates:
  - `sector_scores.csv` (15 sectors × 8 drivers)
  - `region_scores.csv` (7 regions × 8 drivers)
  - `lambda_parameters.csv` (8 scale parameters)
- [ ] `residual_covariance.csv` (Corr_S, Corr_R, D_S, D_R, ω, η)
- [ ] `asset_correlation.csv` (ρ parameter with sensitivity ranges)
- [ ] Anchor target specification document
- [ ] Validation report (factor magnitudes, portfolio losses, sector decompositions)
- [ ] Sensitivity analysis report (impact of λ, ω, η, ρ uncertainty)
- [ ] Correlation validation report (within-sector, within-region co-movement with ω and η)

### Success Criteria
- All S and R scores have documented economic/physical rationale
- λ calibration meets anchor targets (max cell ~2σ, portfolio EL 2x-5x baseline)
- Corr_S and Corr_R successfully estimated from MSCI data (or equivalent)
- ω and η chosen with clear rationale based on portfolio characteristics
- Induced correlations are reasonable given ω and η choices
- Σ_u is positive definite with no numerical issues
- Sensitivity analysis shows:
  - ω impact on sector vs region concentration
  - η impact on tail risk (VaR, ES) and diversification benefit
  - ρ impact on default clustering
- Factor magnitudes make intuitive sense across scenarios

---

## Phase 4: C++ Engine Implementation (Weeks 7-12)

### Objectives
Build the core C++ simulation engine with REST API, computing both risk metrics (VaR, CVaR) and return metrics (expected return, Sharpe ratio, efficient frontier).

### Prerequisites
- Completed Phase 1 (portfolio data with CSV templates)
- Completed Phase 3 (calibrated w and λ parameters)
- C++ development environment set up (CMake, SQLite3, Eigen3)

### Tasks

#### 4.1 Project Structure Setup
- [ ] Create CMake project following ScenarioAnalysis2 pattern:
  - `engine/` directory with `include/` and `src/` subdirectories
  - `external/` for dependencies (eigen, crow, nlohmann_json, spdlog)
  - `tests/` for unit tests
- [ ] Set up git submodules for dependencies
- [ ] Configure CMakeLists.txt with C++20, optimization flags, SQLite3
- [ ] Create initial header structure (core/, database/, utils/)

#### 4.2 Database Layer
- [ ] Design SQLite schema:
  - `portfolios` table (id, name, description)
  - `obligors` table (id, portfolio_id, name, sector, region, pd, lgd, ead)
  - `sectors` table (id, name, description)
  - `regions` table (id, name, description)
  - `scenarios` table (id, name, narrative, year)
  - `scenario_drivers` table (scenario_id, driver_name, region, value, standardized_value)
  - `calibration_sector_scores` table (sector_id, driver_name, score)
  - `calibration_region_scores` table (region_id, driver_name, score)
  - `calibration_lambda` table (driver_name, lambda_value)
  - `simulation_results` table (id, scenario_id, portfolio_id, timestamp, metrics_json)
- [ ] Implement DatabaseManager class with CRUD operations
- [ ] CSV import functions (portfolio, scenarios, calibration)
- [ ] CSV export functions (results, factor matrices)

#### 4.3 Core Data Structures
- [ ] Define `Portfolio` class:
  - Obligor list with sector/region assignments
  - β matrix (N × M): obligor loadings on factors (M = S×R = 105 cells)
  - PD, LGD, EAD vectors
  - Expected returns (for return analytics)
- [ ] Define `Scenario` class:
  - Raw drivers X (K × 1 vector, K=8)
  - Standardized φ (K × 1 vector)
  - Scenario metadata (name, year, narrative)
- [ ] Define `CalibrationParams` class:
  - W matrix (M × K): sensitivity matrix (or S, R, λ for hybrid structure)
  - Σ_u (M × M): residual covariance matrix
  - ρ: asset correlation parameter
  - Corr_S, Corr_R, D_S, D_R (for Σ_u construction)
  - ω: sector-vs-region mix parameter
  - η: structured-vs-cell share parameter
  - Membership matrices A (M × S) and B (M × R)
- [ ] Define `FactorRealization` class:
  - Factor vector f (M × 1)
  - Conditional mean m(X) = α + W · φ(X)
  - Covariance Σ_u
- [ ] Use Eigen for all matrix/vector operations

#### 4.4 Factor Generation Module
- [ ] Implement `FactorGenerator` class:
  - `computeConditionalMean(Scenario, CalibrationParams) -> Eigen::VectorXd`:
    * m(X) = α + W · φ(X)  (M × 1 vector)
  - `sampleFactors(m, Σ_u, rng) -> Eigen::VectorXd`:
    * u ~ MVN(0, Σ_u)
    * f = m + u
    * Use Cholesky decomposition for sampling
  - `sampleFactorsBatch(m, Σ_u, n_simulations, rng) -> Eigen::MatrixXd`:
    * Generate n_simulations draws (M × n_simulations matrix)
- [ ] Vectorized operations using Eigen
- [ ] Factor diagnostics (min, max, mean, covariance across simulations)
- [ ] Unit tests for MVN sampling
- [ ] Precompute Cholesky decomposition of Σ_u for efficiency

#### 4.5 Monte Carlo Simulation Engine
- [ ] Implement `SimulationEngine` class with parallel execution:
  - `simulate(Portfolio, FactorMatrix, CalibrationParams, N_sims) -> SimulationResults`
  - For each simulation (parallelized with OpenMP):
    - Compute systematic scores: S_i = β_i^T · f for each obligor i
    - Standardize: S̃_i = S_i / √(β_i^T Σ_u β_i)
    - Draw ε_i ~ N(0,1) using thread-safe RNG
    - Compute Z_i = √ρ · S̃_i + √(1-ρ) · ε_i
    - Determine defaults: D_i = (Z_i < τ_i), where τ_i = Φ^(-1)(PD_i)
    - Compute loss: L = Σ_i D_i · LGD_i · EAD_i
  - Collect loss distribution across simulations
- [ ] Precompute β_i^T Σ_u β_i for all obligors (variance of systematic score)
- [ ] Implement efficient normal CDF/inverse (use boost or Eigen or hand-rolled)
- [ ] Multi-threading with configurable thread count
- [ ] Progress callback for long simulations

#### 4.6 Return Analytics Module (NEW)
- [ ] Implement `ReturnAnalytics` class:
  - For each simulation, compute portfolio return:
    - R = Σ_i (1 - D_i · LGD_i) · weight_i · expected_return_i - Σ_i D_i · LGD_i · weight_i
    - Or simpler: R = baseline_return - loss_rate
  - Collect return distribution across simulations
- [ ] Compute expected return E[R]
- [ ] Compute return volatility σ[R]
- [ ] Compute Sharpe ratio: (E[R] - risk_free_rate) / σ[R]
- [ ] Compute return-risk pairs for multiple scenarios (for efficient frontier)

#### 4.7 Risk Metrics Module
- [ ] Implement `RiskMetrics` class:
  - Expected Loss (EL): mean of loss distribution
  - Unexpected Loss (UL): std dev of loss distribution
  - VaR at 95%, 99%, 99.9% percentiles
  - CVaR (Expected Shortfall): mean loss beyond VaR threshold
  - Loss distribution histogram (binned)
- [ ] Attribution analysis:
  - Losses by sector, region, sector-region cell
  - Marginal contribution of each driver to EL (via factor sensitivity)

#### 4.8 Efficient Frontier Calculator (NEW)
- [ ] Implement `EfficientFrontier` class:
  - Run simulations for multiple scenarios (varying climate stress levels)
  - For each scenario, compute (σ[R], E[R]) pair
  - Sort by σ[R] to create frontier curve
  - Identify Sharpe-optimal portfolio position
- [ ] Export frontier data for visualization

#### 4.9 REST API Layer
- [ ] Implement Crow-based HTTP server:
  - `POST /api/portfolio/import` - upload portfolio CSV
  - `POST /api/scenarios/import` - upload scenario CSV
  - `POST /api/calibration/import` - upload calibration CSV
  - `POST /api/simulate` - run simulation for scenario(s)
    - Body: {portfolio_id, scenario_ids, n_sims, compute_frontier}
    - Returns: {results_id, status}
  - `GET /api/results/{id}` - fetch simulation results
    - Returns: {EL, UL, VaR, CVaR, Sharpe, return_dist, loss_dist, factor_matrix, attributions}
  - `GET /api/results/{id}/export` - download results as CSV
  - `GET /api/frontier/{results_id}` - fetch efficient frontier data
- [ ] JSON serialization/deserialization with nlohmann/json
- [ ] Error handling and validation
- [ ] CORS headers for local development

#### 4.10 Integration and Testing
- [ ] Unit tests for each module (Google Test or Catch2)
- [ ] Integration test: load sample portfolio → run simulation → verify metrics
- [ ] Performance benchmarking:
  - 10,000 obligors × 10,000 simulations should run in <5 minutes (with parallelization)
- [ ] Memory profiling (check for leaks)
- [ ] CSV round-trip tests (import → export → import should be idempotent)

### Deliverables
✅ C++ engine with modular architecture (database, core, simulation, API)
✅ Working REST API on localhost:8080
✅ CSV import/export functionality
✅ Risk metrics (EL, UL, VaR, CVaR) computation
✅ Return metrics (E[R], σ[R], Sharpe ratio) computation
✅ Efficient frontier calculation
✅ Unit test suite with >80% coverage
✅ Performance benchmarking report

### Success Criteria
- All unit tests pass
- API responds to requests with valid JSON
- 10K obligor × 10K simulation test completes in <5 minutes
- CSV import/export works correctly
- Sharpe ratio and efficient frontier calculations are mathematically correct

---

## Phase 5: React Frontend Development (Weeks 10-15)

### Objectives
Build a simple but functional React dashboard for portfolio management, scenario running, and results visualization.

### Prerequisites
- Completed Phase 4.9 (C++ REST API running)
- Node.js and npm installed

### Tasks

#### 5.1 Project Setup
- [ ] Initialize Vite + React + TypeScript project
- [ ] Install dependencies:
  - Recharts, Plotly.js for charts
  - D3.js for custom visualizations
  - Radix UI for components
  - Tailwind CSS for styling
  - Zustand for state management
  - React Router for navigation
  - Axios for API calls
- [ ] Configure Vite proxy to C++ backend (localhost:8080)
- [ ] Set up project structure (pages/, components/, hooks/, services/, types/)

#### 5.2 API Client Layer
- [ ] Create API service with Axios:
  - `uploadPortfolio(csvFile)`
  - `uploadScenarios(csvFile)`
  - `uploadCalibration(csvFile)`
  - `runSimulation(portfolioId, scenarioIds, nSims, computeFrontier)`
  - `getResults(resultsId)`
  - `getFrontier(resultsId)`
  - `exportResults(resultsId)`
- [ ] TypeScript interfaces for all API responses
- [ ] Error handling and loading states

#### 5.3 State Management
- [ ] Create Zustand stores:
  - `portfolioStore` (uploaded portfolios, current selection)
  - `scenarioStore` (available scenarios, selections)
  - `simulationStore` (running simulations, results)
  - `calibrationStore` (current calibration parameters)
- [ ] Persist state to localStorage where appropriate

#### 5.4 Core Pages

**5.4.1 Portfolio Manager Page**
- [ ] CSV upload component with drag-and-drop
- [ ] Portfolio summary table (total EAD, # obligors, sector breakdown)
- [ ] Sector-region heatmap (EAD by cell) using Recharts
- [ ] Concentration analysis chart

**5.4.2 Scenario Manager Page**
- [ ] CSV upload for scenarios
- [ ] Scenario list with descriptions (NGFS narratives)
- [ ] Driver time series visualization (Plotly line charts)
- [ ] Scenario comparison view (multiple driver paths on one chart)

**5.4.3 Calibration Page**
- [ ] CSV upload for sector/region scores and λ parameters
- [ ] Editable tables for S_{s,k}, R_{r,k}, λ_k
- [ ] Heatmaps showing relative weights w̃
- [ ] "Save calibration" button to update backend

**5.4.4 Simulation Runner Page**
- [ ] Portfolio selector (dropdown)
- [ ] Scenario multi-select (checkboxes)
- [ ] Simulation settings:
  - Number of simulations (slider: 1K-100K)
  - Compute efficient frontier (checkbox)
  - Risk-free rate input (for Sharpe ratio)
- [ ] "Run Simulation" button
- [ ] Progress indicator during simulation
- [ ] Results link when complete

**5.4.5 Results Explorer Page**
- [ ] Summary metrics cards:
  - Expected Loss (EL), Unexpected Loss (UL)
  - VaR 95/99/99.9
  - CVaR 99.9
  - Expected Return, Return Volatility
  - Sharpe Ratio
- [ ] Loss distribution histogram (Recharts bar chart)
- [ ] Return distribution histogram
- [ ] Factor matrix heatmap (F_{s,r} values)
- [ ] Loss attribution charts:
  - Pie chart by sector
  - Pie chart by region
  - Waterfall chart by driver
- [ ] Scenario comparison table (if multiple scenarios run)

**5.4.6 Efficient Frontier Page** (NEW)
- [ ] Scatter plot of (σ[R], E[R]) for all scenarios
- [ ] Highlight Sharpe-optimal point
- [ ] Capital Allocation Line (CAL) from risk-free rate
- [ ] Tooltips showing scenario name on hover
- [ ] Export frontier data as CSV

#### 5.5 Visualization Components

**5.5.1 Risk-Return Scatter (D3 + Plotly)**
- [ ] Interactive scatter plot with zoom/pan
- [ ] Color-code by scenario type (baseline, transition, physical)
- [ ] Annotations for key scenarios

**5.5.2 Factor Heatmap (Recharts or D3)**
- [ ] Sector × Region grid
- [ ] Color scale for F values (-3 to +3 σ)
- [ ] Tooltip showing exact F_{s,r} value and contribution to loss

**5.5.3 Distribution Charts**
- [ ] Histogram with KDE overlay (loss and return distributions)
- [ ] VaR/CVaR threshold lines
- [ ] Interactive bins (clickable to drill down)

#### 5.6 UI Polish
- [ ] Responsive layout (works on laptop screens)
- [ ] Dark mode support (optional)
- [ ] Loading spinners and error messages
- [ ] Navigation bar with logo and page links
- [ ] Export buttons (download charts as PNG, data as CSV)

#### 5.7 Testing
- [ ] Component tests for key pages (React Testing Library)
- [ ] API integration tests (mock API responses)
- [ ] End-to-end test: upload portfolio → run simulation → view results

### Deliverables
✅ Working React dashboard on localhost:5173
✅ Portfolio upload and management UI
✅ Scenario upload and visualization
✅ Calibration parameter editor
✅ Simulation runner with progress tracking
✅ Results explorer with all key visualizations
✅ Efficient frontier visualization
✅ CSV export functionality

### Success Criteria
- Dashboard connects to C++ backend API successfully
- CSV upload works for portfolio, scenarios, calibration
- Simulation can be triggered and results displayed
- Charts render correctly with real data
- Efficient frontier plot is mathematically accurate
- UI is intuitive (no user manual needed for basic operations)

---

## Phase 6: Validation & Scenario Analysis (Weeks 13-18)

### Objectives
Validate model behavior, run full scenario suite, and produce initial risk reports.

### Prerequisites
- Completed Phase 4 (working C++ engine)
- Completed Phase 5 (working React dashboard)
- Completed Phase 2 (scenario data for all selected NGFS pathways)

### Tasks

#### 6.1 Model Validation Tests

**6.1.1 Reproduce Marginal PDs**
- [ ] Run model with F = 0 (no scenario stress)
- [ ] Verify that realized default frequencies ≈ input PDs
- [ ] Document any deviations

**5.1.2 Correlation Validation**
- [ ] Compute pairwise default correlation from simulations
- [ ] Compare to analytical expectations
- [ ] Verify that same-sector, same-region pairs have higher correlation

**5.1.3 Sensitivity to F**
- [ ] Manually shock F for specific cells (e.g., F_coal,Asia = +2σ)
- [ ] Verify that coal/Asia exposures drive loss increases
- [ ] Check linearity: does 2× shock produce 2× loss increase?

**5.1.4 Parameter Sensitivity**
- [ ] Re-run scenarios with λ_k varied ±30%
- [ ] Document range of EL and VaR outcomes
- [ ] Identify most influential parameters

**5.1.5 Scenario Coherence**
- [ ] Check that scenario rankings make sense:
  - Net Zero 2050 < Delayed Transition (for transition-heavy portfolios)
  - Current Policies (high warming) shows high physical risk
- [ ] Verify factor heatmaps align with scenario narratives

**6.1.6 Benchmarking (if possible)**
- [ ] Compare outputs to peer models (if available)
- [ ] Compare to published stress test results (ECB, BoE, NGFS)
- [ ] Document differences and hypotheses

#### 6.2 Full Scenario Suite Analysis
- [ ] Run 4-6 NGFS scenarios on full portfolio
- [ ] Run for multiple time horizons (2030, 2040, 2050)
- [ ] Generate outputs for each scenario:
  - Expected Loss (EL)
  - Unexpected Loss (UL = SD of loss)
  - VaR 95, 99, 99.9
  - CVaR (expected shortfall)
  - Loss distribution histogram
  - Loss attribution (by sector, region, driver)
- [ ] Create comparative summary table across scenarios

#### 6.3 Deep-Dive Analysis

**6.3.1 Transition Risk Deep-Dive**
- [ ] Isolate transition drivers (set physical drivers to 0)
- [ ] Identify most vulnerable sectors/regions
- [ ] Analyze heterogeneity within sectors (if data allows)

**6.3.2 Physical Risk Deep-Dive**
- [ ] Isolate physical drivers (set transition drivers to 0)
- [ ] Map physical risk by geography
- [ ] Identify exposed sectors (real estate, agriculture, infrastructure)

**6.3.3 Concentration Risk**
- [ ] Identify single-name or single-cell concentrations
- [ ] Stress test: what if largest exposure defaults?
- [ ] Recommend portfolio rebalancing if needed

#### 6.4 Uncertainty Quantification
- [ ] Produce "confidence intervals" by varying λ_k
- [ ] Report EL and VaR as ranges, not point estimates
- [ ] Create tornado diagram showing parameter sensitivities

### Deliverables
✅ Model validation report
  - PD calibration check
  - Correlation validation
  - Sensitivity analysis results
  - Benchmarking (if available)
✅ Scenario analysis report
  - Full scenario results (all metrics)
  - Comparative scenario summary
  - Transition vs physical risk decomposition
  - Concentration risk analysis
✅ Uncertainty quantification analysis
  - Parameter sensitivity analysis
  - EL/VaR ranges under parameter uncertainty
✅ Summary of key findings and portfolio climate risk profile

### Success Criteria
- Validation tests show model behaves as expected (no red flags)
- Scenario results are interpretable and align with narratives
- Uncertainty analysis shows reasonable stability

---

## Ongoing: Model Monitoring & Enhancement (Post-Implementation)

### Routine Maintenance
- Quarterly: Update portfolio data and re-run scenarios
- Annually: Update NGFS scenarios when new versions released
- Annually: Recalibrate λ_k if portfolio composition changes significantly
- Ongoing: Track model performance metrics (if validation data emerges)
- Ongoing: Monitor for climate events that could inform calibration

### Enhancement Roadmap (Post-v1.0)

**v1.1 Enhancements (3-6 months post-deployment):**
- Add multi-region β weighting for multinational obligors
- Refine sector taxonomy (15 → 30+ sectors)
- Add interaction terms to g(C) (non-linearities)

**v1.2 Enhancements (6-12 months):**
- Separate F into F^tran and F^phys (address key critique)
- Implement multi-period dynamics F(t)
- Add systematic residual noise η to F

**v2.0 Major Upgrade (12-18 months):**
- Incorporate firm-level climate attributes (emissions, asset locations)
- Estimate parameters from spreads/EDFs (if data available)
- Add feedback loops (credit stress → macro → C)
- Develop proprietary physical risk indices

---

## Technical Requirements

### Technology Stack

**Architecture:** Following ScenarioAnalysis2 pattern with C++ backend and React frontend

**Backend (C++ Engine):**
- C++20 standard with CMake build system
- SQLite3 for data storage and on-the-fly manipulation
- Eigen3 for linear algebra (matrix operations, factor models)
- Crow web framework for REST API
- nlohmann/json for JSON serialization
- spdlog for logging
- OpenMP or std::execution for parallel Monte Carlo simulations

**Frontend (React Dashboard):**
- React 19 + TypeScript + Vite
- Recharts and Plotly.js for visualization
- D3.js for custom visualizations (heatmaps, efficient frontier)
- Radix UI components for UI elements
- Tailwind CSS for styling
- Zustand for state management
- React Router for navigation

**Data Layer:**
- CSV files for input/output (portfolio data, scenario drivers, results)
- SQLite database for:
  - Portfolio storage (obligors, PD, LGD, EAD, sector/region mappings)
  - Calibration parameters (S, R, w, λ)
  - Scenario data (C, φ)
  - Simulation results (loss distributions, VaR, return metrics)

**Version Control:**
- Git for source control
- GitHub/GitLab for collaboration

**Build & Deployment:**
- CMake for C++ compilation
- npm/vite for React build
- Docker for containerized deployment (optional)

### Key Data Sources

| Item | Purpose | Source Options |
|------|---------|----------------|
| NGFS scenario data | Climate scenarios | NGFS Data Portal / IIASA NGFS Scenario Explorer (free) |
| Physical risk data | Hazard indices | World Bank Climate Portal, EM-DAT (free) or commercial providers |
| Emissions data | Sector calibration | CDP, company disclosures, Our World in Data (free/low cost) or Refinitiv, Bloomberg (commercial) |
| Sector mappings | Industry classification | NACE, NAICS, GICS codes |

---

## Implementation Milestones

### Week 3
- Portfolio data cleaned and mapped to sector-region grid
- CSV templates created
- Exposure heatmap generated

### Week 4
- NGFS scenarios downloaded and processed
- Driver variables C constructed for 4-6 scenarios

### Week 8
- Sector and region sensitivity scores (S, R) complete
- Relative weights w̃ constructed
- λ scale parameters calibrated

### Week 12
- C++ engine complete with REST API
- SQLite database operational
- CSV import/export working
- Return and risk metrics computation tested

### Week 15
- React dashboard complete
- All visualizations working (including efficient frontier)
- End-to-end workflow: upload CSV → run simulation → view results

### Week 18
- Full validation suite complete
- Scenario analysis for all NGFS pathways generated
- Key findings documented and visualized

---

## Key Model Files Structure

```
cpm/
├── CMakeLists.txt                  # Top-level CMake configuration
├── README.md                       # Project overview and setup guide
│
├── engine/                         # C++ backend
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── core/
│   │   │   ├── Portfolio.hpp
│   │   │   ├── Scenario.hpp
│   │   │   ├── CalibrationParams.hpp
│   │   │   └── FactorMatrix.hpp
│   │   ├── database/
│   │   │   └── DatabaseManager.hpp
│   │   ├── simulation/
│   │   │   ├── SimulationEngine.hpp
│   │   │   ├── RiskMetrics.hpp
│   │   │   └── ReturnAnalytics.hpp
│   │   ├── analysis/
│   │   │   ├── FactorMapper.hpp
│   │   │   └── EfficientFrontier.hpp
│   │   └── api/
│   │       └── Server.hpp
│   ├── src/
│   │   ├── core/              # Implementation files
│   │   ├── database/
│   │   ├── simulation/
│   │   ├── analysis/
│   │   ├── api/
│   │   └── main.cpp           # Server entry point
│   └── tests/
│       ├── test_factor_mapper.cpp
│       ├── test_simulation.cpp
│       └── test_integration.cpp
│
├── dashboard/                      # React frontend
│   ├── package.json
│   ├── vite.config.ts
│   ├── tsconfig.json
│   ├── public/
│   ├── src/
│   │   ├── pages/
│   │   │   ├── PortfolioManager.tsx
│   │   │   ├── ScenarioManager.tsx
│   │   │   ├── CalibrationPage.tsx
│   │   │   ├── SimulationRunner.tsx
│   │   │   ├── ResultsExplorer.tsx
│   │   │   └── EfficientFrontier.tsx
│   │   ├── components/
│   │   │   ├── charts/
│   │   │   │   ├── RiskReturnScatter.tsx
│   │   │   │   ├── FactorHeatmap.tsx
│   │   │   │   └── DistributionChart.tsx
│   │   │   └── ui/              # Radix UI components
│   │   ├── services/
│   │   │   └── api.ts           # Axios API client
│   │   ├── stores/
│   │   │   ├── portfolioStore.ts
│   │   │   ├── scenarioStore.ts
│   │   │   └── simulationStore.ts
│   │   ├── types/
│   │   │   └── index.ts         # TypeScript interfaces
│   │   └── App.tsx
│   └── index.html
│
├── data/                           # Data files
│   ├── portfolios/
│   │   └── sample_portfolio.csv
│   ├── scenarios/
│   │   ├── ngfs_net_zero_2050.csv
│   │   ├── ngfs_delayed_transition.csv
│   │   └── ngfs_current_policies.csv
│   ├── calibration/
│   │   ├── sector_scores.csv
│   │   ├── region_scores.csv
│   │   └── lambda_parameters.csv
│   └── templates/
│       ├── portfolio_template.csv
│       ├── scenario_template.csv
│       └── calibration_template.csv
│
├── external/                       # External C++ libraries (git submodules)
│   ├── eigen/
│   ├── crow/
│   ├── nlohmann_json/
│   └── spdlog/
│
├── docs/
│   ├── methodology.md              # Model documentation
│   ├── calibration_log.md          # Calibration decisions
│   ├── validation.md               # Validation results
│   ├── api_reference.md            # REST API documentation
│   └── user_guide.md               # How to use the dashboard
│
└── scripts/
    ├── init_database.sh            # Initialize SQLite schema
    ├── import_sample_data.sh       # Load sample data
    └── run_server.sh               # Start C++ backend
```

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
- [ ] Load obligor-level exposure data (EAD, PD, LGD)
- [ ] Validate data quality (completeness, consistency)
- [ ] Handle missing values and outliers
- [ ] Create unique obligor identifiers

#### 1.2 Sector Classification
- [ ] Define sector taxonomy (start with 10-15 sectors, expandable)
- [ ] Map each obligor to primary sector
- [ ] Validate sector assignments (sample audit)
- [ ] Document sector definitions and mapping rules

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
- [ ] Define region taxonomy (start with 6-8 regions)
- [ ] Map each obligor to primary region (by HQ, revenue, or assets)
- [ ] Consider multi-region weighting for large corporates
- [ ] Validate region assignments

**Region taxonomy (initial):**
1. North America
2. Europe
3. Asia-Pacific (developed)
4. Asia-Pacific (emerging)
5. Latin America
6. Middle East & Africa
7. Global (for multinational obligors)

#### 1.4 Exposure Mapping
- [ ] Create sector-region grid (S × R = 15 × 7 = 105 cells)
- [ ] Compute total EAD by cell
- [ ] Compute weighted average PD by cell
- [ ] Identify concentration risk (cells with >5% of EAD)

#### 1.5 Data Enrichment (Optional but Recommended)
- [ ] Collect emissions data (Scope 1, 2, 3 if available)
- [ ] Collect revenue geography breakdowns (for multi-region β)
- [ ] Collect physical asset locations
- [ ] Create obligor-level climate attributes database

#### 1.6 CSV Template Creation
- [ ] Create portfolio CSV template (columns: ObligorID, Name, Sector, Region, PD, LGD, EAD, ...)
- [ ] Create scenario CSV template (columns: ScenarioName, Year, Driver, Region, Value)
- [ ] Create calibration CSV templates (sector scores, region scores, λ parameters)
- [ ] Document CSV formats and validation rules

### Deliverables
✅ Clean portfolio dataset with obligor-level PD, LGD, EAD
✅ Sector and region taxonomies with mapping rules documented
✅ Sector-region exposure heatmap (EAD by cell)
✅ Portfolio concentration report
✅ Data quality report (coverage, completeness)
✅ CSV templates for all data inputs

### Success Criteria
- >95% of EAD mapped to sector and region
- No critical data quality issues identified

---

## Phase 2: Scenario Data Preparation (Weeks 2-4)

### Objectives
Extract, transform, and standardize NGFS climate scenarios into model-ready driver variables C.

### Prerequisites
- Access to NGFS scenario data (download from NGFS portal or IIASA database)
- Access to physical risk data sources (World Bank, EM-DAT, or commercial providers)

### Tasks

#### 2.1 NGFS Scenario Selection
- [ ] Review NGFS scenario suite (current version: Phase IV)
- [ ] Select 4-6 scenarios for implementation:
  - Baseline: Current Policies
  - Orderly transition: Net Zero 2050
  - Disorderly transition: Delayed Transition
  - Physical risk: Current Policies (high warming)
  - Optional: Divergent Net Zero, Low demand
- [ ] Document scenario narratives

#### 2.2 Transition Driver Extraction
- [ ] Extract carbon price paths by region
- [ ] Extract energy price indices (fossil fuels, electricity)
- [ ] Extract GDP/output growth paths
- [ ] Extract policy stringency indicators (if available)

#### 2.3 Physical Driver Construction
- [ ] Extract temperature anomaly paths by region
- [ ] Extract precipitation anomaly paths by region
- [ ] Construct or source physical hazard indices:
  - Flood severity index by region
  - Storm/cyclone severity index by region
  - Heat stress index by region
  - Drought index by region
- [ ] Align all data to common time grid (annual, 2025-2050)

#### 2.4 Driver Consolidation
- [ ] Define final driver set C (recommend 5-7 variables):
  1. Carbon price shock (transition)
  2. Fossil energy price shock (transition)
  3. GDP deviation from baseline (transition/macro)
  4. Flood severity index (physical)
  5. Storm severity index (physical)
  6. Heat/drought severity index (physical)
  7. Optional: Policy stringency score
- [ ] Create driver database with scenario × year × driver × region structure

#### 2.5 Driver Standardization (φ transformation)
- [ ] Compute baseline values C_baseline for each driver
- [ ] Compute scenario deviations ΔC = C_scenario - C_baseline
- [ ] Define standardization scales σ_k using cross-scenario dispersion
- [ ] Compute standardized features φ_k = ΔC_k / σ_k
- [ ] Document standardization methodology

### Deliverables
✅ Scenario narrative summary (1-2 pages per scenario)
✅ Raw NGFS data extracts (transition variables)
✅ Physical hazard index database
✅ Consolidated driver database C (scenario × year × driver × region)
✅ Standardized driver database φ(C) with σ_k documentation
✅ Driver visualization notebook (time series plots, scenario comparisons)

### Success Criteria
- All scenarios have complete driver coverage (no missing values)
- Standardization produces φ values in interpretable range (-3 to +3 σ)

---

## Phase 3: Sensitivity Calibration (Weeks 5-8)

### Objectives
Build sector and region sensitivity scores, construct relative weights w̃, and calibrate scale parameters λ.

### Prerequisites
- Completed Phase 1 (portfolio data with sector/region mappings)
- Completed Phase 2 (standardized climate drivers φ(C))

### Tasks

#### 3.1 Sector Sensitivity Scores (Transition)
- [ ] For each sector s and transition driver k, assign S_{s,k} ∈ [0,1]:
  - Carbon price sensitivity (based on emissions intensity, abatement cost)
  - Energy price sensitivity (based on energy intensity)
  - GDP sensitivity (based on cyclicality)
- [ ] Document scoring methodology and data sources
- [ ] Create sector sensitivity heatmap

**Data sources for sector scores:**
- Emissions intensity: CDP, company disclosures, sector averages
- Energy intensity: National accounts, industry reports
- Abatement cost curves: McKinsey, IEA
- Expert judgment: Internal sector specialists

#### 3.2 Sector Sensitivity Scores (Physical)
- [ ] For each sector s and physical driver k, assign S_{s,k} ∈ [0,1]:
  - Flood sensitivity (asset-heavy, location-dependent sectors)
  - Storm sensitivity (infrastructure, supply chain fragility)
  - Heat/drought sensitivity (agriculture, water-dependent sectors)
- [ ] Document scoring methodology

#### 3.3 Region Sensitivity Scores (Transition)
- [ ] For each region r and transition driver k, assign R_{r,k} ∈ [0,1]:
  - Carbon policy likelihood/stringency
  - Energy mix (fossil dependence)
  - Regulatory environment
- [ ] Document methodology

#### 3.4 Region Sensitivity Scores (Physical)
- [ ] For each region r and physical driver k, assign R_{r,k} ∈ [0,1]:
  - Flood propensity (coastal exposure, flood plain share)
  - Storm propensity (cyclone tracks, historical frequency)
  - Heat/drought propensity (climate zone, water stress)
- [ ] Use historical hazard data and climate projections
- [ ] Document sources and assumptions

**Data sources for region scores:**
- Hazard exposure: World Bank Climate Change Knowledge Portal, EM-DAT
- Coastal exposure: UNEP, national statistics
- Policy stance: OECD, IMF climate policy trackers

#### 3.5 Construct Relative Weights
- [ ] Compute w̃_{s,r,k} = S_{s,k} · R_{r,k} for all cells and drivers
- [ ] Create weight heatmaps (sector × region for each driver)
- [ ] Sanity check: highest weights align with intuition
- [ ] Document any manual adjustments

#### 3.6 Define Anchor Targets
- [ ] Define factor magnitude targets:
  - "Max cell movement under severe scenario should be ~2σ"
  - "Median cell movement under moderate scenario ~0.5-1σ"
- [ ] Define portfolio targets:
  - "EL uplift under Net Zero 2050 should be X-Y% of baseline EL"
  - "99th percentile loss uplift should be <Z% of portfolio EAD"
- [ ] Specify sector contribution targets:
  - "Coal + Oil&Gas should contribute >50% of transition scenario loss"
- [ ] Document rationale for each target

#### 3.7 Calibrate Scale Parameters λ
- [ ] Initialize λ_k = 1 for all drivers
- [ ] For each scenario:
  - Compute F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k
  - Check against factor magnitude targets
- [ ] Adjust λ_k iteratively to meet targets
- [ ] Document calibration iterations and final λ values
- [ ] Perform sensitivity analysis: vary λ_k ± 30%, observe impact

### Deliverables
✅ Sector sensitivity score library (S matrix: sectors × drivers)
✅ Region sensitivity score library (R matrix: regions × drivers)
✅ Relative weight library (w̃: cells × drivers)
✅ Calibration target specification document
✅ Final scale parameters λ with calibration report
✅ Sensitivity analysis report (impact of λ uncertainty)

### Success Criteria
- Sector/region scores have documented data sources/rationale
- Calibrated model meets anchor targets within tolerance
- λ sensitivity analysis shows reasonable stability

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
- [ ] Define `Portfolio` class (obligors, sector-region grid, β matrix)
- [ ] Define `Scenario` class (drivers C, standardized φ)
- [ ] Define `CalibrationParams` class (S, R, w̃, λ)
- [ ] Define `FactorMatrix` class (F_{s,r} for all cells)
- [ ] Use Eigen for matrix/vector operations

#### 4.4 Factor Mapping Module
- [ ] Implement `FactorMapper` class:
  - `computeFactors(Scenario, CalibrationParams) -> FactorMatrix`
  - Apply: F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k
- [ ] Vectorized operations using Eigen
- [ ] Factor diagnostics (min, max, mean by sector/region)
- [ ] Unit tests

#### 4.5 Monte Carlo Simulation Engine
- [ ] Implement `SimulationEngine` class with parallel execution:
  - `simulate(Portfolio, FactorMatrix, N_sims) -> SimulationResults`
  - For each simulation (parallelized with OpenMP):
    - Draw ε ~ N(0, I) using thread-safe RNG
    - Compute Z = β · vec(F) + ε
    - Determine defaults: D = (Z < τ), where τ = Φ^(-1)(PD)
    - Compute loss: L = Σ_i D_i · LGD_i · EAD_i
  - Collect loss distribution across simulations
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

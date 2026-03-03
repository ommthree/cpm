# Midas - Implementation Plan

**Product Name**: Midas (Model for Integrated Default Analysis with Scenarios)

**Last Updated**: 2026-02-27

---

## Overview

This plan implements a browser-based climate-aware credit portfolio risk model with:
- **C++ simulation engine** (high-performance Monte Carlo)
- **React dashboard** (interactive visualization and analysis)
- **REST API** (connects frontend to backend)

The plan follows a **build-first approach**: implement a working end-to-end system with placeholder calibration data, then refine calibration parameters as data becomes available.

**Timeline**: 10 weeks to production-ready v1.0

---

## Current Status

### ✅ Completed (Weeks 1-2)

**Phase 1: Portfolio Data Preparation** - COMPLETE
- Synthetic portfolio created (500 obligors, $5.8B EAD)
- 15 sectors × 7 regions taxonomy defined
- CSV templates created and documented

**Phase 2: Scenario Data Preparation** - COMPLETE
- NGFS Phase V scenarios downloaded and processed (3 scenarios)
- World Bank CCKP physical risk data integrated
- 8 drivers standardized (5 transition + 3 physical)
- 546 data points ready for model input

**Data Sources Simplified** - COMPLETE
- OECD Air Emissions Accounts (826 MB) for transition risk
- BIS Working Paper 1292 for physical risk
- Yahoo Finance ETF data for correlation matrices
- Legacy data archived

### 🔄 In Progress

**Phase 3: Calibration** - DEFERRED
- Placeholder calibration values in CSV files
- Will refine as OECD/BIS data is processed
- Not blocking development

### ⏳ Next Steps

**Phase 4-6: Build the Model** (This plan)
- Start implementation with existing data
- Use placeholder calibration initially
- Refine calibration in parallel

---

## Implementation Phases

### Phase Summary

| Phase | Timeline | Focus | Key Deliverables |
|-------|----------|-------|------------------|
| **1. MVP Core** | Weeks 1-2 | C++ fundamentals | Portfolio, Scenario, Calibration classes; CSV loaders; basic simulator |
| **2. MVP API** | Week 3 | REST endpoints | Crow server, 3 endpoints, JSON responses |
| **3. MVP Frontend** | Week 4 | Basic React UI | Portfolio/scenario selection, run button, results table |
| **4. Production Backend** | Weeks 5-7 | Performance & features | Async jobs, parallelization, SQLite storage |
| **5. Production Frontend** | Week 8 | Visualization | Charts (loss distribution, contributions, frontier) |
| **6. Polish & Deploy** | Weeks 9-10 | Quality & deployment | Tests, docs, Docker, CI/CD |

---

## Data Structure Overview

Before diving into implementation phases, understand what data flows through the system:

### Input Data Structure

```
data/
├── portfolios/
│   └── sample_portfolio_500.csv
│       Columns: ObligorID, Name, Sector, Region, PD, LGD, EAD, Beta_0...Beta_104
│       Rows: 500 obligors
│       Key: Each obligor has exposure to 1+ sector-region cells
│
├── scenarios/
│   ├── net_zero_2050.csv
│   ├── delayed_transition.csv
│   └── current_policies.csv
│       Columns: Scenario, Year, Driver, Region, Value
│       Rows: ~182 per file
│       Key: 8 drivers × 7 regions × multiple years
│
└── calibration/
    ├── sector_scores.csv          # 15 sectors × 8 drivers
    ├── region_scores.csv          # 7 regions × 8 drivers
    ├── lambda_parameters.csv      # 8 scale parameters
    └── residual_variance.csv      # 105×105 covariance matrix
```

### Core Data Model

```
Portfolio
├── Obligor[0]
│   ├── id: "OB-001"
│   ├── sector: 0 (Energy - Oil & Gas)
│   ├── region: 0 (North America)
│   ├── ead: $10M
│   ├── pd: 0.05
│   ├── lgd: 0.45
│   └── beta: [0.8, 0.1, ..., 0.05]  // 105×1 vector of factor loadings
├── Obligor[1]
│   └── ...
└── Obligor[499]

ClimateScenario
├── name: "Net Zero 2050"
├── year: 2030
└── drivers[region][driver_id]
    ├── drivers[0][0] = 1.5  // North America, CarbonPrice
    ├── drivers[0][1] = 0.8  // North America, CoalPrice
    └── ...
    // 7 regions × 8 drivers = 56 values

CalibrationData
├── S: 15×8 matrix        // Sector scores
│   └── S[0][0] = 0.90    // Energy-Oil&Gas sensitivity to CarbonPrice
├── R: 7×8 matrix         // Region scores
│   └── R[0][0] = 0.70    // North America sensitivity to CarbonPrice
├── λ: 8×1 vector         // Scale parameters
│   └── λ[0] = 1.0        // CarbonPrice scale
└── Σ_u: 105×105 matrix   // Residual covariance
```

### Simulation Flow

```
Input: Portfolio (500 obligors) + Scenario (Net Zero 2030)

Step 1: Generate Factors (deterministic)
  F_{s,r} = Σ_k λ_k · (S_{s,k} × R_{r,k}) · φ_k(C)
  Result: F = 105×1 vector (one per sector-region cell)
  Example: F[0] = -1.2σ (North America Oil&Gas factor)

Step 2: Monte Carlo Loop (N=10,000 simulations)
  For each simulation n:
    2a. Draw idiosyncratic shocks
        ε ~ N(0, Σ_u)  // 105×1 vector

    2b. Compute creditworthiness for each obligor i
        Z_i = Σ_j β_{i,j} × F_j + ε_i
        Result: Z = 500×1 vector

    2c. Detect defaults
        Default_i = (Z_i < Φ^(-1)(PD_i))
        Result: 500 boolean flags

    2d. Calculate loss
        Loss_n = Σ_i (Default_i × EAD_i × LGD_i)
        Result: Single loss value for simulation n

Step 3: Aggregate Results
  Loss distribution: [Loss_0, Loss_1, ..., Loss_9999]

  Compute metrics:
  - Expected Loss = mean(losses)
  - VaR 95% = 95th percentile(losses)
  - VaR 99% = 99th percentile(losses)
  - CVaR 99% = mean(losses where loss > VaR99)

  Compute contributions:
  - By sector: Group obligors by sector, sum EAD × Default_rate
  - By region: Group obligors by region, sum EAD × Default_rate

Output: SimulationResult
  {
    "expected_loss": 58000000,
    "var_95": 120000000,
    "var_99": 180000000,
    "cvar_99": 210000000,
    "sector_el": {0: 12000000, 1: 8000000, ...},
    "region_el": {0: 15000000, 1: 20000000, ...},
    "losses": [45000000, 62000000, ..., 41000000]  // All 10K values
  }
```

### Data Sizes

| Data Element | Size | Notes |
|--------------|------|-------|
| Portfolio | 500 obligors × (7 scalars + 105 betas) = ~420 KB | In memory |
| Scenario | 56 driver values × 8 bytes = ~500 bytes | Per scenario/year |
| Calibration | (15×8 + 7×8 + 8 + 105×105) ≈ 11K doubles ≈ 88 KB | Loaded once |
| Single simulation | 500 obligors × 1 double (Z_i) ≈ 4 KB | Per thread |
| Loss distribution | 10K simulations × 8 bytes = 80 KB | Per result |
| **Total memory** | ~2 GB | 10K sims with full history |

---

## Phase 1: MVP Core (Weeks 1-2)

**Goal**: Working command-line simulator that loads data and runs simulations

### Week 1: Data Foundation

#### 1.1 Project Setup
```bash
# Create directory structure
midas/
├── engine/
│   ├── CMakeLists.txt
│   ├── include/midas/
│   ├── src/core/
│   └── tests/
└── external/
    ├── eigen/
    └── json/
```

**Tasks**:
- [ ] Initialize Git repository
- [ ] Create CMakeLists.txt with C++20 standard
- [ ] Add Eigen3 as git submodule
- [ ] Add nlohmann/json as git submodule
- [ ] Configure build system
- [ ] Build "Hello World" to verify setup

**Deliverable**: `./build/midas_test` compiles and runs

---

#### 1.2 Portfolio Class

**File**: `engine/include/midas/portfolio.hpp`

**Tasks**:
- [ ] Define `Obligor` struct with all fields
- [ ] Define `Portfolio` class with vector of obligors
- [ ] Implement CSV loader using fast-cpp-csv-parser
- [ ] Add data validation (PD in [0,1], EAD > 0, etc.)
- [ ] Add accessor methods (get sector/region EAD totals)
- [ ] Write unit test: load sample_portfolio_500.csv

**Key Methods**:
```cpp
class Portfolio {
public:
    void load_from_csv(const std::string& path);
    size_t size() const;
    double total_ead() const;
    const Obligor& get_obligor(size_t i) const;

    // Analytics
    std::map<int, double> sector_ead() const;
    std::map<int, double> region_ead() const;
};
```

**Test**:
```cpp
TEST(PortfolioTest, LoadSamplePortfolio) {
    Portfolio p;
    p.load_from_csv("../../data/portfolios/sample_portfolio_500.csv");

    EXPECT_EQ(p.size(), 500);
    EXPECT_NEAR(p.total_ead(), 5.8e9, 1e6);
    EXPECT_EQ(p.get_obligor(0).sector_id, 0);
}
```

**Deliverable**: Portfolio loads from CSV, test passes

---

#### 1.3 Scenario Class

**File**: `engine/include/midas/scenario.hpp`

**Tasks**:
- [ ] Define `ClimateScenario` class
- [ ] Implement CSV loader for scenario files
- [ ] Store drivers in lookup table: `driver_values_[region][driver_id]`
- [ ] Add accessor: `get_driver(driver_id, region_id)`
- [ ] Create `ScenarioLibrary` to manage multiple scenarios
- [ ] Write unit test: load net_zero_2050.csv

**Key Methods**:
```cpp
class ClimateScenario {
public:
    void load_from_csv(const std::string& path);
    const std::string& name() const;
    int year() const;
    double get_driver(int driver_id, int region_id) const;
    Eigen::VectorXd get_regional_drivers(int region_id) const; // 8×1
};

class ScenarioLibrary {
public:
    void load_all(const std::string& dir);
    const ClimateScenario& get(const std::string& name, int year);
    std::vector<std::string> list_scenarios() const;
};
```

**Test**:
```cpp
TEST(ScenarioTest, LoadNetZero2050) {
    ClimateScenario s;
    s.load_from_csv("../../data/scenarios/net_zero_2050.csv");

    EXPECT_EQ(s.name(), "Net Zero 2050");
    EXPECT_DOUBLE_EQ(s.get_driver(0, 0), 1.5);  // Carbon price, NA
}
```

**Deliverable**: Scenario loads from CSV, test passes

---

#### 1.4 Calibration Class

**File**: `engine/include/midas/calibration.hpp`

**Tasks**:
- [ ] Define `CalibrationData` class
- [ ] Load sector_scores.csv → 15×8 Eigen matrix
- [ ] Load region_scores.csv → 7×8 Eigen matrix
- [ ] Load lambda_parameters.csv → 8×1 Eigen vector
- [ ] Load residual_variance.csv → 105×105 Eigen matrix
- [ ] Precompute weights: w̃_{s,r,k} = S_{s,k} × R_{r,k}
- [ ] Precompute Cholesky decomposition: Σ_u = L L^T
- [ ] Write unit test: verify dimensions

**Key Methods**:
```cpp
class CalibrationData {
public:
    void load(const std::string& calibration_dir);

    const Eigen::MatrixXd& sector_scores() const;     // 15×8
    const Eigen::MatrixXd& region_scores() const;     // 7×8
    const Eigen::VectorXd& lambda() const;            // 8×1
    const Eigen::MatrixXd& sigma_u() const;           // 105×105
    const Eigen::MatrixXd& cholesky() const;          // 105×105, L

    // Compute w̃ matrix (105×8)
    Eigen::MatrixXd compute_weights() const;
};
```

**Test**:
```cpp
TEST(CalibrationTest, LoadCalibrationData) {
    CalibrationData cal;
    cal.load("../../data/calibration");

    EXPECT_EQ(cal.sector_scores().rows(), 15);
    EXPECT_EQ(cal.sector_scores().cols(), 8);
    EXPECT_EQ(cal.lambda().size(), 8);
    EXPECT_EQ(cal.sigma_u().rows(), 105);
}
```

**Deliverable**: Calibration loads from CSV, test passes

---

### Week 2: Simulation Engine

#### 2.1 Factor Generator

**File**: `engine/include/midas/factor_generator.hpp`

**Tasks**:
- [ ] Implement factor generation equation
- [ ] F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k(C)
- [ ] Use Eigen matrix operations for efficiency
- [ ] Write unit test: verify factor values match hand calculation

**Key Method**:
```cpp
class FactorGenerator {
public:
    FactorGenerator(const CalibrationData& calibration);

    // Generate 105×1 factor vector for a scenario
    Eigen::VectorXd generate(const ClimateScenario& scenario) const;
};
```

**Test**:
```cpp
TEST(FactorGeneratorTest, GenerateFactors) {
    CalibrationData cal;
    cal.load("../../data/calibration");

    ClimateScenario scenario;
    scenario.load_from_csv("../../data/scenarios/net_zero_2050.csv");

    FactorGenerator fg(cal);
    Eigen::VectorXd F = fg.generate(scenario);

    EXPECT_EQ(F.size(), 105);
    // Check first cell (Energy-Oil&Gas × North America)
    // Should be negative (carbon price hurts fossil fuels)
    EXPECT_LT(F(0), 0.0);
}
```

**Deliverable**: Factor generator works, test passes

---

#### 2.2 Random Number Generator

**File**: `engine/include/midas/random.hpp`

**Tasks**:
- [ ] Wrap std::mt19937_64 in thread-safe class
- [ ] Implement N(0,1) sampling
- [ ] Implement multivariate normal: z = L × u where u ~ N(0,I)
- [ ] Write unit test: verify mean ≈ 0, std ≈ 1

**Key Methods**:
```cpp
class RandomGenerator {
public:
    explicit RandomGenerator(unsigned int seed = 0);

    double normal();  // N(0,1)
    Eigen::VectorXd normal_vector(int size);  // N(0,I)
    Eigen::VectorXd multivariate_normal(const Eigen::MatrixXd& L);  // N(0, LL^T)
};
```

**Test**:
```cpp
TEST(RandomGeneratorTest, NormalDistribution) {
    RandomGenerator rng(42);

    std::vector<double> samples;
    for (int i = 0; i < 10000; ++i) {
        samples.push_back(rng.normal());
    }

    double mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    EXPECT_NEAR(mean, 0.0, 0.05);
}
```

**Deliverable**: RNG works, test passes

---

#### 2.3 Monte Carlo Simulator

**File**: `engine/include/midas/simulator.hpp`

**Tasks**:
- [ ] Implement single simulation iteration
  - Generate factors F
  - Draw idiosyncratic shocks ε ~ N(0, Σ_u)
  - Compute Z_i = β_i^T F + ε_i for all obligors
  - Detect defaults: Z_i < Φ^(-1)(PD_i)
  - Calculate loss: Σ EAD_i × LGD_i × 1{default_i}
- [ ] Implement full simulation: repeat N times
- [ ] Store all loss values
- [ ] Write unit test: run 1000 simulations, verify EL > 0

**Key Class**:
```cpp
struct SimulationConfig {
    int num_simulations = 10000;
    int num_threads = 1;  // Single-threaded for now
    unsigned int random_seed = 0;
};

struct SimulationResult {
    std::vector<double> losses;  // All N loss values
    double expected_loss;
    double var_95;
    double var_99;
    double cvar_99;
    double max_loss;
    double elapsed_seconds;
};

class MonteCarloSimulator {
public:
    MonteCarloSimulator(
        const Portfolio& portfolio,
        const CalibrationData& calibration,
        const SimulationConfig& config
    );

    SimulationResult run(const ClimateScenario& scenario);

private:
    double simulate_once(
        RandomGenerator& rng,
        const Eigen::VectorXd& mean_factors
    );
};
```

**Test**:
```cpp
TEST(SimulatorTest, RunSimulation) {
    Portfolio portfolio;
    portfolio.load_from_csv("../../data/portfolios/sample_portfolio_500.csv");

    CalibrationData calibration;
    calibration.load("../../data/calibration");

    ClimateScenario scenario;
    scenario.load_from_csv("../../data/scenarios/net_zero_2050.csv");

    SimulationConfig config;
    config.num_simulations = 1000;

    MonteCarloSimulator sim(portfolio, calibration, config);
    SimulationResult result = sim.run(scenario);

    EXPECT_EQ(result.losses.size(), 1000);
    EXPECT_GT(result.expected_loss, 0);
    EXPECT_GT(result.var_99, result.var_95);
}
```

**Deliverable**: Simulator runs 1000 simulations, test passes

---

#### 2.4 Risk Metrics

**File**: `engine/include/midas/risk_metrics.hpp`

**Tasks**:
- [ ] Implement EL calculation: mean(losses)
- [ ] Implement VaR: sort losses, take percentile
- [ ] Implement CVaR: mean of losses above VaR
- [ ] Write unit test with known distribution

**Key Methods**:
```cpp
class RiskMetrics {
public:
    static double expected_loss(const std::vector<double>& losses);
    static double value_at_risk(const std::vector<double>& losses, double confidence);
    static double conditional_var(const std::vector<double>& losses, double confidence);
};
```

**Deliverable**: Risk metrics compute correctly, test passes

---

#### 2.5 Command-Line Tool

**File**: `engine/src/main.cpp`

**Tasks**:
- [ ] Parse command-line arguments (portfolio, scenario, year)
- [ ] Load all data
- [ ] Run simulation
- [ ] Print results to console

**Usage**:
```bash
$ ./build/midas_cli \
    --portfolio data/portfolios/sample_portfolio_500.csv \
    --scenario "Net Zero 2050" \
    --year 2030 \
    --simulations 10000

Loading portfolio... 500 obligors, $5.8B EAD
Loading scenario... Net Zero 2050 @ 2030
Loading calibration... 15 sectors, 7 regions, 8 drivers
Running 10,000 simulations...

Results:
  Expected Loss:    $58.0M
  VaR 95%:          $120.5M
  VaR 99%:          $180.2M
  CVaR 99%:         $210.8M
  Max Loss:         $350.4M

  Time elapsed: 8.5 seconds
```

**Deliverable**: CLI tool runs successfully

---

## Phase 2: MVP API (Week 3)

**Goal**: REST API that serves portfolio/scenario data and runs simulations

### 3.1 Crow Setup

**Tasks**:
- [ ] Add Crow as git submodule
- [ ] Create minimal HTTP server
- [ ] Test with curl: `curl http://localhost:8080/health`
- [ ] Add JSON response helper

**Deliverable**: Server starts and responds to /health

---

### 3.2 API Endpoints

**Tasks**:
- [ ] GET /api/portfolios
  - List all portfolio files
  - Return: `{"portfolios": ["sample_portfolio_500"]}`
- [ ] GET /api/scenarios
  - List all scenarios with years
  - Return: `{"scenarios": [{"name": "Net Zero 2050", "years": [2030, 2040, 2050]}]}`
- [ ] POST /api/simulate
  - Body: `{"portfolio": "sample_portfolio_500", "scenario": "Net Zero 2050", "year": 2030, "config": {"num_simulations": 10000}}`
  - Run simulation synchronously (blocks until complete)
  - Return: SimulationResult as JSON

**Test with curl**:
```bash
curl http://localhost:8080/api/portfolios

curl http://localhost:8080/api/scenarios

curl -X POST http://localhost:8080/api/simulate \
  -H "Content-Type: application/json" \
  -d '{"portfolio": "sample_portfolio_500", "scenario": "Net Zero 2050", "year": 2030, "config": {"num_simulations": 1000}}'
```

**Deliverable**: All 3 endpoints work, return valid JSON

---

### 3.3 JSON Serialization

**Tasks**:
- [ ] Implement `SimulationResult::to_json()` using nlohmann/json
- [ ] Test: deserialize in Python/curl, verify numbers match

**Example Output**:
```json
{
  "scenario_name": "Net Zero 2050",
  "scenario_year": 2030,
  "portfolio_name": "sample_portfolio_500",
  "num_simulations": 10000,
  "elapsed_seconds": 8.5,
  "expected_loss": 58000000,
  "var_95": 120500000,
  "var_99": 180200000,
  "cvar_99": 210800000,
  "max_loss": 350400000,
  "losses": [45123000, 62045000, ...]
}
```

**Deliverable**: API returns proper JSON

---

## Phase 3: MVP Frontend (Week 4)

**Goal**: Basic React dashboard that calls API and displays results

### 4.1 React Project Setup

**Tasks**:
- [ ] Create Vite + React + TypeScript project
- [ ] Install dependencies: react-query, zustand
- [ ] Set up Tailwind CSS
- [ ] Create basic layout (header, sidebar, main content)

**Commands**:
```bash
cd dashboard
npm create vite@latest . -- --template react-ts
npm install
npm install @tanstack/react-query zustand
npm install -D tailwindcss postcss autoprefixer
npx tailwindcss init -p
```

**Deliverable**: `npm run dev` starts blank React app

---

### 4.2 API Client

**File**: `dashboard/src/api/client.ts`

**Tasks**:
- [ ] Define TypeScript interfaces matching C++ structs
- [ ] Implement `MidasApiClient` class with 3 methods:
  - `listPortfolios()`
  - `listScenarios()`
  - `runSimulation(portfolio, scenario, year, config)`
- [ ] Test: call API from browser console

**Deliverable**: API client works in browser

---

### 4.3 UI Components

**Components to build**:
- [ ] `PortfolioSelector`: Dropdown to select portfolio
- [ ] `ScenarioSelector`: Dropdown to select scenario + year
- [ ] `ConfigPanel`: Inputs for num_simulations (slider or input)
- [ ] `RunButton`: Button to trigger simulation
- [ ] `LoadingSpinner`: Shows while simulation runs
- [ ] `ResultsTable`: Table showing EL, VaR95, VaR99, CVaR99

**UI Flow**:
```
┌────────────────────────────────┐
│  Portfolio: [sample_portfolio] │
│  Scenario:  [Net Zero 2050]    │
│  Year:      [2030]              │
│  Sims:      [10000]             │
│  [Run Simulation]               │
└────────────────────────────────┘

┌────────────────────────────────┐
│  Results                        │
│  ├─ Expected Loss: $58.0M       │
│  ├─ VaR 95%:       $120.5M      │
│  ├─ VaR 99%:       $180.2M      │
│  └─ CVaR 99%:      $210.8M      │
└────────────────────────────────┘
```

**Deliverable**: UI works, can run simulation and see results

---

### 4.4 State Management

**Tasks**:
- [ ] Create Zustand store for:
  - Selected portfolio
  - Selected scenario + year
  - Current results
- [ ] Use React Query for API calls
  - Cache portfolio/scenario lists
  - Handle loading/error states

**Deliverable**: State updates correctly when user interacts

---

### 4.5 Integration Test

**Manual Test Checklist**:
1. Start C++ server: `./build/midas_server`
2. Start React dev server: `npm run dev`
3. Open browser to `http://localhost:5173`
4. Select portfolio: "sample_portfolio_500"
5. Select scenario: "Net Zero 2050", year 2030
6. Click "Run Simulation"
7. Wait ~10 seconds (loading spinner shows)
8. Results table appears with metrics
9. Numbers match CLI output

**Deliverable**: ✅ End-to-end MVP works!

---

## Phase 4: Production Backend (Weeks 5-7)

**Goal**: Make backend fast, scalable, and production-ready

### Week 5: Async Job Queue

**Problem**: Simulation takes 10 seconds, UI freezes

**Solution**:
- POST /api/simulate returns `{"job_id": "uuid"}` immediately (HTTP 202)
- Simulation runs in background thread
- Client polls GET /api/results/{job_id} every 500ms
- Response has status: "queued" | "running" | "completed" | "failed"

**Tasks**:
- [ ] Create `SimulationJob` struct (job_id, status, config, result)
- [ ] Implement job queue with std::queue + std::mutex
- [ ] Create worker thread pool (4 threads)
- [ ] Modify POST /api/simulate to enqueue job
- [ ] Add GET /api/results/{job_id} endpoint
- [ ] Update React to poll for results

**Deliverable**: UI doesn't freeze during simulation

---

### Week 6: SQLite Storage

**Tasks**:
- [ ] Create SQLite schema:
  - `simulation_runs`: job_id, portfolio, scenario, year, status, created_at, completed_at
  - `simulation_results`: job_id, expected_loss, var_95, var_99, etc.
  - `loss_distribution`: job_id, simulation_idx, loss_value
- [ ] Store every completed simulation
- [ ] Add GET /api/history endpoint (list past runs)
- [ ] Update React to show history

**Deliverable**: Simulation history persists across restarts

---

### Week 7: Parallelization

**Tasks**:
- [ ] Add OpenMP to CMakeLists.txt
- [ ] Wrap simulation loop with `#pragma omp parallel for`
- [ ] Each thread uses thread-local RNG
- [ ] Benchmark: measure speedup (target 3-4x on 4 cores)
- [ ] Add performance metrics to results (sims/second)

**Performance Target**: 10K sims in <3 seconds (from 8-10s)

**Deliverable**: Simulations run 3-4x faster

---

## Phase 5: Production Frontend (Week 8)

**Goal**: Add charts and advanced visualizations

### 8.1 Loss Distribution Chart

**Tasks**:
- [ ] Install Recharts: `npm install recharts`
- [ ] Create histogram from losses array (50 bins)
- [ ] Add VaR 95/99 markers as vertical lines
- [ ] Style with colors (blue bars, red VaR lines)

**Deliverable**: Loss distribution chart shows histogram

---

### 8.2 Contribution Charts

**Tasks**:
- [ ] Sector contribution bar chart (15 sectors)
- [ ] Region contribution bar chart (7 regions)
- [ ] Sort by contribution (descending)
- [ ] Add tooltips with EAD and default rate

**Deliverable**: Two contribution bar charts work

---

### 8.3 Scenario Comparison

**Tasks**:
- [ ] UI to run 3 scenarios in parallel
- [ ] Table comparing VaR99 across scenarios
- [ ] Chart showing loss distributions overlaid

**Deliverable**: Can compare scenarios side-by-side

---

### 8.4 Efficient Frontier (Optional)

**Tasks**:
- [ ] Implement optimization in C++ (find portfolio weights)
- [ ] Add POST /api/efficient-frontier endpoint
- [ ] Create scatter plot with Plotly.js
- [ ] X-axis: risk (CVaR), Y-axis: return

**Deliverable**: Efficient frontier chart works

---

## Phase 6: Polish & Deploy (Weeks 9-10)

**Goal**: Production-ready with tests, docs, and deployment

### Week 9: Testing & Documentation

**C++ Tests**:
- [ ] Unit tests for all classes (>80% coverage)
- [ ] Integration test: full simulation workflow
- [ ] Performance test: verify <3s target

**React Tests** (optional):
- [ ] Component tests with React Testing Library
- [ ] API client tests

**Documentation**:
- [ ] API reference (OpenAPI/Swagger spec)
- [ ] User guide: "How to upload portfolio and run simulation"
- [ ] Developer guide: "How to build and extend Midas"

**Deliverable**: Comprehensive tests and docs

---

### Week 10: Deployment

**Option A: Docker Compose**:
```yaml
services:
  engine:
    build: ./engine
    ports: ["8080:8080"]
    volumes: ["./data:/app/data"]

  dashboard:
    build: ./dashboard
    ports: ["80:80"]
    environment:
      - VITE_API_URL=http://engine:8080
```

**Option B: Single Binary**:
- C++ serves static React files from /dist

**Option C: Cloud**:
- Deploy C++ to AWS EC2 / GCP Cloud Run
- Deploy React to S3 + CloudFront / Netlify

**CI/CD**:
- [ ] GitHub Actions: build and test on push
- [ ] Automated Docker build

**Deliverable**: One-command deployment (`docker-compose up`)

---

## Success Criteria

### MVP (Week 4)
- ✅ Load portfolio from CSV
- ✅ Select scenario and year
- ✅ Run 10K simulations
- ✅ Display risk metrics (EL, VaR, CVaR)
- ✅ Results match mathematical model

### Production (Week 8)
- ✅ Simulations complete in <3 seconds
- ✅ UI responsive (async jobs, no freezing)
- ✅ Loss distribution chart
- ✅ Sector/region contribution charts
- ✅ Compare multiple scenarios

### Deployment (Week 10)
- ✅ >80% test coverage
- ✅ Complete documentation
- ✅ One-command deployment
- ✅ Production-ready (error handling, logging)

---

## Risk Mitigation

| Risk | Mitigation |
|------|-----------|
| C++ development too slow | Start simple, use templates from architecture.md |
| Calibration data not ready | Use placeholder values, mark TODO |
| Performance target missed | Profile with `perf`, optimize hotspots (likely Cholesky) |
| Scope creep | Defer features to post-v1.0 (efficient frontier, etc.) |
| Integration issues | Test API with curl at each step |

---

## Post-v1.0 Roadmap

**Calibration Refinement**:
- Process OECD emissions data → sector carbon scores
- Extract BIS physical risk scores → sector vulnerability
- Compute correlation matrices from ETF data

**Advanced Features**:
- Portfolio optimizer (rebalance to minimize CVaR)
- Stress testing (what-if carbon price +50%)
- Monte Carlo convergence diagnostics
- Multi-portfolio comparison
- Export results to PDF report

**Performance**:
- GPU acceleration (CUDA)
- Distributed computing (multiple machines)
- Real-time updates (WebSocket instead of polling)

---

## Appendix: Quick Reference

### Build Commands

```bash
# C++ Engine
cd engine && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
./midas_server

# React Dashboard
cd dashboard
npm install
npm run dev

# Docker
docker-compose up --build
```

### File Locations

- Portfolio data: `data/portfolios/sample_portfolio_500.csv`
- Scenarios: `data/scenarios/*.csv`
- Calibration: `data/calibration/*.csv`
- C++ source: `engine/src/`
- React source: `dashboard/src/`
- Tests: `engine/tests/`, `dashboard/src/__tests__/`

### Key Equations

```
Factor generation:
  F_{s,r} = Σ_k λ_k · (S_{s,k} × R_{r,k}) · φ_k(C)

Creditworthiness:
  Z_i = β_i^T F + ε_i
  where ε ~ N(0, Σ_u)

Default condition:
  Default if Z_i < Φ^(-1)(PD_i)

Loss calculation:
  Loss = Σ_i EAD_i × LGD_i × 1{Z_i < threshold_i}
```

---

**End of Implementation Plan**

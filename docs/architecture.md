# Midas Architecture

**Document Version**: 1.0
**Last Updated**: 2026-02-27
**Status**: Design Specification

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [C++ Engine Design](#cpp-engine-design)
4. [React Frontend Design](#react-frontend-design)
5. [Data Flow](#data-flow)
6. [API Specification](#api-specification)
7. [Directory Structure](#directory-structure)
8. [Technology Stack](#technology-stack)
9. [Build and Deployment](#build-and-deployment)
10. [Performance Considerations](#performance-considerations)

---

## Overview

Midas is a client-server application for climate-aware credit portfolio risk analysis. The system consists of:

- **C++ Simulation Engine**: High-performance Monte Carlo engine with REST API
- **React Dashboard**: Browser-based UI for portfolio management, scenario analysis, and visualization
- **Data Layer**: CSV-based data interchange, SQLite for results persistence

### Design Principles

1. **Separation of Concerns**: Clear boundaries between computation (C++), presentation (React), and data (CSV/SQLite)
2. **Performance**: C++ engine targets 10K simulations × 500 obligors in <10 seconds
3. **Extensibility**: Modular design allows adding new scenarios, metrics, and analytics
4. **Developer Experience**: Simple API, type-safe interfaces, comprehensive testing
5. **Portability**: Cross-platform support (macOS, Linux, Windows)

---

## System Architecture

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                      Browser (React App)                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐          │
│  │  Portfolio   │  │   Scenario   │  │   Results    │          │
│  │  Management  │  │   Config     │  │  Visualizer  │          │
│  └──────────────┘  └──────────────┘  └──────────────┘          │
│         │                  │                  │                  │
│         └──────────────────┴──────────────────┘                  │
│                            │                                     │
│                     API Client (Fetch)                           │
└────────────────────────────┬────────────────────────────────────┘
                             │
                             │ HTTP/REST (JSON)
                             │ Port 8080
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                    C++ REST API Server                           │
│                      (Crow Framework)                            │
│                                                                  │
│  ┌────────────────────────────────────────────────────────┐    │
│  │  Request Handlers                                       │    │
│  │  • GET  /api/portfolios                                 │    │
│  │  • POST /api/portfolios                                 │    │
│  │  • GET  /api/scenarios                                  │    │
│  │  • POST /api/simulate                                   │    │
│  │  • GET  /api/results/{job_id}                           │    │
│  │  • POST /api/efficient-frontier                         │    │
│  └────────────────────────────────────────────────────────┘    │
│                            │                                     │
│  ┌────────────────────────▼────────────────────────────────┐   │
│  │            Simulation Job Queue                          │   │
│  │         (Thread Pool, Async Processing)                  │   │
│  └────────────────────────┬────────────────────────────────┘   │
│                            │                                     │
└────────────────────────────┼─────────────────────────────────────┘
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                   Core Simulation Engine                         │
│                                                                  │
│  ┌───────────────────┐  ┌───────────────────┐                  │
│  │   Data Layer      │  │  Model Components │                  │
│  │                   │  │                   │                  │
│  │ • Portfolio       │  │ • Factor          │                  │
│  │ • Scenario        │  │   Generator       │                  │
│  │ • Calibration     │  │ • Creditworthiness│                  │
│  │ • Results Store   │  │   Model           │                  │
│  └───────────────────┘  │ • Default         │                  │
│                         │   Simulator       │                  │
│  ┌───────────────────┐  │ • Loss Calculator │                  │
│  │   Analytics       │  └───────────────────┘                  │
│  │                   │                                          │
│  │ • Risk Metrics    │  ┌───────────────────┐                  │
│  │ • Return Metrics  │  │   Utilities       │                  │
│  │ • Efficient       │  │                   │                  │
│  │   Frontier        │  │ • Random Gen      │                  │
│  │ • Contribution    │  │ • Correlation     │                  │
│  │   Analysis        │  │ • Statistics      │                  │
│  └───────────────────┘  └───────────────────┘                  │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
                             │
                             │ Read/Write
                             │
┌────────────────────────────▼────────────────────────────────────┐
│                        Data Storage                              │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  CSV Files (Input)                                       │   │
│  │  • data/portfolios/sample_portfolio_500.csv              │   │
│  │  • data/scenarios/net_zero_2050.csv                      │   │
│  │  • data/calibration/sector_scores.csv                    │   │
│  │  • data/calibration/region_scores.csv                    │   │
│  │  • data/calibration/lambda_parameters.csv                │   │
│  │  • data/calibration/residual_variance.csv                │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                  │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │  SQLite Database (Results)                               │   │
│  │  • simulation_runs (job_id, timestamp, status, config)   │   │
│  │  • simulation_results (job_id, metrics, loss_dist)       │   │
│  │  • sector_contributions (job_id, sector_id, el, var)     │   │
│  └─────────────────────────────────────────────────────────┘   │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### Component Interactions

```
User Action (Browser)
    ↓
API Request (HTTP/JSON)
    ↓
Request Handler (Crow)
    ↓
Job Queue (Async)
    ↓
Simulation Engine (C++)
    ├→ Load Portfolio (CSV)
    ├→ Load Scenario (CSV)
    ├→ Load Calibration (CSV)
    ├→ Generate Factors (F_{s,r})
    ├→ Monte Carlo Loop (N simulations)
    │   ├→ Draw ε_i ~ N(0, Σ_u)
    │   ├→ Compute Z_i = β_i^T F + ε_i
    │   ├→ Detect defaults: Z_i < Φ^(-1)(PD_i)
    │   └→ Calculate loss: Σ EAD_i × LGD_i × 1{default_i}
    ├→ Compute Risk Metrics (VaR, CVaR, EL)
    ├→ Compute Contributions (sector, region)
    └→ Store Results (SQLite)
    ↓
Job Status Update
    ↓
API Response (JSON)
    ↓
React State Update
    ↓
UI Render (Charts, Tables)
```

---

## C++ Engine Design

### Core Classes

#### 1. Portfolio Management

```cpp
// engine/include/midas/portfolio.hpp

namespace midas {

struct Obligor {
    std::string id;              // Unique identifier
    std::string name;            // Company name
    int sector_id;               // 0-14 (15 sectors)
    int region_id;               // 0-6 (7 regions)
    double ead;                  // Exposure at Default (USD)
    double pd;                   // Probability of Default [0,1]
    double lgd;                  // Loss Given Default [0,1]
    Eigen::VectorXd beta;        // Factor loadings (105x1)

    // Computed fields
    double threshold;            // Φ^(-1)(PD) for default detection
    int cell_id;                 // Flattened sector-region index

    void compute_threshold();
    void compute_cell_id();
};

class Portfolio {
public:
    // Constructor
    Portfolio() = default;
    explicit Portfolio(const std::string& csv_path);

    // Data access
    const std::vector<Obligor>& obligors() const { return obligors_; }
    size_t size() const { return obligors_.size(); }
    double total_ead() const { return total_ead_; }

    // Analytics
    Eigen::MatrixXd concentration_matrix() const;  // 15x7 EAD heatmap
    std::map<int, double> sector_ead() const;
    std::map<int, double> region_ead() const;

    // I/O
    void load_from_csv(const std::string& path);
    void validate() const;

private:
    std::vector<Obligor> obligors_;
    double total_ead_;

    void compute_aggregates();
};

} // namespace midas
```

#### 2. Climate Scenarios

```cpp
// engine/include/midas/scenario.hpp

namespace midas {

struct DriverValue {
    std::string driver_name;     // "CarbonPrice", "HeatIndex", etc.
    int driver_id;               // 0-7
    int region_id;               // 0-6
    int year;                    // 2030, 2040, 2050
    double value;                // Standardized φ_k(C) value
};

class ClimateScenario {
public:
    ClimateScenario() = default;
    explicit ClimateScenario(const std::string& csv_path);

    // Accessors
    const std::string& name() const { return name_; }
    int year() const { return year_; }

    // Get driver values
    Eigen::VectorXd get_drivers(int region_id) const;  // 8x1 vector
    double get_driver(int driver_id, int region_id) const;

    // I/O
    void load_from_csv(const std::string& path);

private:
    std::string name_;           // "Net Zero 2050", "Delayed Transition", etc.
    int year_;                   // Projection year
    std::vector<DriverValue> drivers_;

    // Lookup table: driver_values_[region_id][driver_id]
    std::array<std::array<double, 8>, 7> driver_values_;
};

class ScenarioLibrary {
public:
    void load_all(const std::string& scenarios_dir);

    const ClimateScenario& get(const std::string& name, int year) const;
    std::vector<std::string> list_scenarios() const;

private:
    std::map<std::string, ClimateScenario> scenarios_;
};

} // namespace midas
```

#### 3. Calibration Data

```cpp
// engine/include/midas/calibration.hpp

namespace midas {

class CalibrationData {
public:
    CalibrationData() = default;

    // Load from CSV files
    void load(const std::string& calibration_dir);

    // Accessors
    const Eigen::MatrixXd& sector_scores() const { return S_; }     // 15x8
    const Eigen::MatrixXd& region_scores() const { return R_; }     // 7x8
    const Eigen::VectorXd& lambda() const { return lambda_; }       // 8x1
    const Eigen::MatrixXd& sigma_u() const { return sigma_u_; }     // 105x105

    // Compute relative weights: w̃_{s,r,k} = S_{s,k} × R_{r,k}
    Eigen::MatrixXd compute_weights() const;  // 105x8 (flattened)

    // Cholesky decomposition for sampling: Σ_u = L L^T
    const Eigen::MatrixXd& cholesky() const;

private:
    Eigen::MatrixXd S_;          // Sector scores (15 sectors × 8 drivers)
    Eigen::MatrixXd R_;          // Region scores (7 regions × 8 drivers)
    Eigen::VectorXd lambda_;     // Scale parameters (8 drivers)
    Eigen::MatrixXd sigma_u_;    // Residual covariance (105×105)

    mutable Eigen::MatrixXd L_;  // Cholesky factor (computed on demand)
    mutable bool cholesky_computed_ = false;

    void load_sector_scores(const std::string& path);
    void load_region_scores(const std::string& path);
    void load_lambda(const std::string& path);
    void load_sigma_u(const std::string& path);

    void compute_cholesky() const;
};

} // namespace midas
```

#### 4. Factor Generator

```cpp
// engine/include/midas/factor_generator.hpp

namespace midas {

class FactorGenerator {
public:
    FactorGenerator(const CalibrationData& calibration);

    // Generate factor realizations for a scenario
    // F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k(C)
    Eigen::VectorXd generate(const ClimateScenario& scenario) const;

    // Get mean factor value (deterministic component)
    Eigen::VectorXd mean_factors(const ClimateScenario& scenario) const;

private:
    const CalibrationData& calibration_;
    Eigen::MatrixXd weights_;    // 105x8 precomputed w̃ matrix
};

} // namespace midas
```

#### 5. Monte Carlo Simulator

```cpp
// engine/include/midas/simulator.hpp

namespace midas {

struct SimulationConfig {
    int num_simulations = 10000;
    int num_threads = 4;
    unsigned int random_seed = 0;  // 0 = use std::random_device
    bool store_full_distribution = true;
};

struct SimulationResult {
    // Metadata
    std::string job_id;
    std::string scenario_name;
    int scenario_year;
    std::string portfolio_name;
    int num_simulations;
    double elapsed_seconds;

    // Loss distribution
    std::vector<double> losses;  // All simulation losses (optional)

    // Risk metrics
    double expected_loss;
    double var_95;
    double var_99;
    double var_99_5;
    double cvar_95;
    double cvar_99;
    double max_loss;

    // Return metrics (if applicable)
    double expected_return;
    double portfolio_volatility;
    double sharpe_ratio;

    // Contributions
    std::map<int, double> sector_el;      // Expected loss by sector
    std::map<int, double> sector_var_99;  // VaR99 contribution by sector
    std::map<int, double> region_el;      // Expected loss by region
    std::map<int, double> region_var_99;  // VaR99 contribution by region

    // Convergence diagnostics
    double el_std_error;
    double var_99_std_error;

    // JSON serialization
    std::string to_json() const;
};

class MonteCarloSimulator {
public:
    MonteCarloSimulator(
        const Portfolio& portfolio,
        const CalibrationData& calibration,
        const SimulationConfig& config
    );

    // Run simulation
    SimulationResult run(const ClimateScenario& scenario);

    // Component methods (exposed for testing)
    Eigen::VectorXd generate_factors(const ClimateScenario& scenario);

    Eigen::VectorXd generate_idiosyncratic(
        RandomGenerator& rng,
        const Eigen::MatrixXd& cholesky
    );

    Eigen::VectorXd compute_creditworthiness(
        const Eigen::VectorXd& factors,
        const Eigen::VectorXd& idiosyncratic
    );

    std::vector<bool> detect_defaults(const Eigen::VectorXd& Z);

    double calculate_loss(const std::vector<bool>& defaults);

private:
    const Portfolio& portfolio_;
    const CalibrationData& calibration_;
    SimulationConfig config_;

    FactorGenerator factor_gen_;

    // Single simulation iteration
    double simulate_once(RandomGenerator& rng, const Eigen::VectorXd& mean_factors);

    // Parallel simulation
    std::vector<double> simulate_parallel(const ClimateScenario& scenario);

    // Post-processing
    void compute_risk_metrics(SimulationResult& result);
    void compute_contributions(SimulationResult& result, const ClimateScenario& scenario);
};

} // namespace midas
```

#### 6. Random Number Generation

```cpp
// engine/include/midas/random.hpp

namespace midas {

// Thread-safe random number generator
class RandomGenerator {
public:
    explicit RandomGenerator(unsigned int seed = 0);

    // Draw N(0,1) variates
    double normal();
    Eigen::VectorXd normal_vector(int size);

    // Draw from multivariate normal: N(0, L L^T)
    // Input: L = Cholesky factor
    Eigen::VectorXd multivariate_normal(const Eigen::MatrixXd& L);

    // Utility
    void set_seed(unsigned int seed);

private:
    std::mt19937_64 engine_;
    std::normal_distribution<double> normal_dist_;
};

} // namespace midas
```

#### 7. Risk Analytics

```cpp
// engine/include/midas/risk_metrics.hpp

namespace midas {

class RiskMetrics {
public:
    // Compute metrics from loss distribution
    static double expected_loss(const std::vector<double>& losses);
    static double value_at_risk(const std::vector<double>& losses, double confidence);
    static double conditional_var(const std::vector<double>& losses, double confidence);
    static double max_loss(const std::vector<double>& losses);

    // Standard errors (for convergence diagnostics)
    static double el_standard_error(const std::vector<double>& losses);
    static double var_standard_error(
        const std::vector<double>& losses,
        double confidence,
        int bootstrap_samples = 1000
    );
};

class ContributionAnalysis {
public:
    ContributionAnalysis(const Portfolio& portfolio);

    // Compute expected loss contribution by sector/region
    std::map<int, double> sector_contributions(
        const std::vector<double>& obligor_el
    );

    std::map<int, double> region_contributions(
        const std::vector<double>& obligor_el
    );

    // VaR contribution (marginal VaR approximation)
    std::map<int, double> sector_var_contributions(
        const std::vector<std::vector<bool>>& default_paths,
        double var_threshold
    );

private:
    const Portfolio& portfolio_;
};

} // namespace midas
```

#### 8. Efficient Frontier

```cpp
// engine/include/midas/efficient_frontier.hpp

namespace midas {

struct FrontierPoint {
    double expected_return;
    double expected_loss;
    double volatility;
    double sharpe_ratio;
    std::vector<double> weights;  // Portfolio weights
};

class EfficientFrontier {
public:
    EfficientFrontier(
        const Portfolio& base_portfolio,
        const CalibrationData& calibration
    );

    // Compute efficient frontier under a scenario
    std::vector<FrontierPoint> compute(
        const ClimateScenario& scenario,
        int num_points = 20,
        const SimulationConfig& config = {}
    );

private:
    const Portfolio& base_portfolio_;
    const CalibrationData& calibration_;

    // Optimization helper
    FrontierPoint optimize_for_target_return(
        double target_return,
        const ClimateScenario& scenario,
        const SimulationConfig& config
    );
};

} // namespace midas
```

### REST API Server

```cpp
// engine/src/api/server.cpp

#include <crow.h>
#include <midas/simulator.hpp>
#include <midas/portfolio.hpp>
#include <midas/scenario.hpp>

namespace midas::api {

struct SimulationJob {
    std::string job_id;
    std::string status;  // "queued", "running", "completed", "failed"
    std::string portfolio_name;
    std::string scenario_name;
    int scenario_year;
    SimulationConfig config;

    // Results (populated when complete)
    std::optional<SimulationResult> result;
    std::optional<std::string> error_message;

    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point completed_at;
};

class MidasServer {
public:
    MidasServer(int port = 8080);

    void start();
    void stop();

private:
    crow::SimpleApp app_;
    int port_;

    // Data
    std::map<std::string, Portfolio> portfolios_;
    ScenarioLibrary scenarios_;
    CalibrationData calibration_;

    // Job management
    std::map<std::string, SimulationJob> jobs_;
    std::mutex jobs_mutex_;

    // Thread pool for async simulation
    std::vector<std::thread> worker_threads_;
    std::queue<std::string> job_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    bool shutdown_ = false;

    // Route handlers
    void setup_routes();

    crow::response handle_list_portfolios();
    crow::response handle_upload_portfolio(const crow::request& req);
    crow::response handle_list_scenarios();
    crow::response handle_start_simulation(const crow::request& req);
    crow::response handle_get_results(const std::string& job_id);
    crow::response handle_efficient_frontier(const crow::request& req);

    // Worker thread
    void worker_loop();
    void process_job(const std::string& job_id);

    // Utilities
    std::string generate_job_id();
    void load_data();
};

} // namespace midas::api
```

---

## React Frontend Design

### Component Hierarchy

```
App
├── Layout
│   ├── Header (navigation, logo)
│   ├── Sidebar (portfolio/scenario selection)
│   └── MainContent
│       ├── PortfolioView
│       │   ├── PortfolioUploader
│       │   ├── PortfolioList
│       │   └── PortfolioSummary (concentration chart)
│       ├── SimulationView
│       │   ├── ScenarioSelector
│       │   ├── ConfigPanel (num_simulations, threads)
│       │   ├── RunButton
│       │   └── JobStatusPanel
│       └── ResultsView
│           ├── RiskMetricsTable
│           ├── LossDistributionChart (histogram)
│           ├── SectorContributionChart (bar chart)
│           ├── RegionContributionChart (bar chart)
│           └── EfficientFrontierChart (scatter plot)
```

### Key Components

#### 1. API Client

```typescript
// dashboard/src/api/client.ts

export interface Obligor {
  id: string;
  name: string;
  sector_id: number;
  region_id: number;
  ead: number;
  pd: number;
  lgd: number;
}

export interface Portfolio {
  name: string;
  obligors: Obligor[];
  total_ead: number;
}

export interface ClimateScenario {
  name: string;
  year: number;
  description: string;
}

export interface SimulationConfig {
  num_simulations: number;
  num_threads: number;
  random_seed: number;
}

export interface SimulationResult {
  job_id: string;
  scenario_name: string;
  scenario_year: number;
  portfolio_name: string;
  num_simulations: number;
  elapsed_seconds: number;

  // Risk metrics
  expected_loss: number;
  var_95: number;
  var_99: number;
  cvar_99: number;
  max_loss: number;

  // Contributions
  sector_el: Record<number, number>;
  sector_var_99: Record<number, number>;
  region_el: Record<number, number>;

  // Full distribution (optional)
  losses?: number[];
}

export interface SimulationJob {
  job_id: string;
  status: 'queued' | 'running' | 'completed' | 'failed';
  portfolio_name: string;
  scenario_name: string;
  result?: SimulationResult;
  error_message?: string;
}

class MidasApiClient {
  private baseUrl: string;

  constructor(baseUrl = 'http://localhost:8080') {
    this.baseUrl = baseUrl;
  }

  // Portfolio endpoints
  async listPortfolios(): Promise<string[]> {
    const response = await fetch(`${this.baseUrl}/api/portfolios`);
    return response.json();
  }

  async uploadPortfolio(file: File): Promise<{ name: string }> {
    const formData = new FormData();
    formData.append('portfolio', file);

    const response = await fetch(`${this.baseUrl}/api/portfolios`, {
      method: 'POST',
      body: formData,
    });
    return response.json();
  }

  // Scenario endpoints
  async listScenarios(): Promise<ClimateScenario[]> {
    const response = await fetch(`${this.baseUrl}/api/scenarios`);
    return response.json();
  }

  // Simulation endpoints
  async startSimulation(
    portfolio: string,
    scenario: string,
    year: number,
    config: SimulationConfig
  ): Promise<{ job_id: string }> {
    const response = await fetch(`${this.baseUrl}/api/simulate`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ portfolio, scenario, year, config }),
    });
    return response.json();
  }

  async getJobStatus(jobId: string): Promise<SimulationJob> {
    const response = await fetch(`${this.baseUrl}/api/results/${jobId}`);
    return response.json();
  }

  // Poll job until complete
  async waitForJob(
    jobId: string,
    onProgress?: (job: SimulationJob) => void
  ): Promise<SimulationResult> {
    while (true) {
      const job = await this.getJobStatus(jobId);

      if (onProgress) {
        onProgress(job);
      }

      if (job.status === 'completed') {
        return job.result!;
      }

      if (job.status === 'failed') {
        throw new Error(job.error_message || 'Simulation failed');
      }

      // Poll every 500ms
      await new Promise(resolve => setTimeout(resolve, 500));
    }
  }
}

export const apiClient = new MidasApiClient();
```

#### 2. Main App Component

```typescript
// dashboard/src/App.tsx

import { QueryClient, QueryClientProvider } from '@tanstack/react-query';
import { Layout } from './components/Layout';
import { PortfolioView } from './components/PortfolioView';
import { SimulationView } from './components/SimulationView';
import { ResultsView } from './components/ResultsView';
import { useAppStore } from './store/appStore';

const queryClient = new QueryClient();

export default function App() {
  const currentView = useAppStore(state => state.currentView);

  return (
    <QueryClientProvider client={queryClient}>
      <Layout>
        {currentView === 'portfolio' && <PortfolioView />}
        {currentView === 'simulation' && <SimulationView />}
        {currentView === 'results' && <ResultsView />}
      </Layout>
    </QueryClientProvider>
  );
}
```

#### 3. State Management

```typescript
// dashboard/src/store/appStore.ts

import { create } from 'zustand';
import { SimulationResult } from '../api/client';

interface AppState {
  // Navigation
  currentView: 'portfolio' | 'simulation' | 'results';
  setCurrentView: (view: AppState['currentView']) => void;

  // Selected data
  selectedPortfolio: string | null;
  selectedScenario: string | null;
  selectedYear: number | null;

  setSelectedPortfolio: (name: string) => void;
  setSelectedScenario: (name: string, year: number) => void;

  // Results
  currentResults: SimulationResult | null;
  resultHistory: SimulationResult[];

  setCurrentResults: (results: SimulationResult) => void;
  addToHistory: (results: SimulationResult) => void;
}

export const useAppStore = create<AppState>((set) => ({
  currentView: 'portfolio',
  setCurrentView: (view) => set({ currentView: view }),

  selectedPortfolio: null,
  selectedScenario: null,
  selectedYear: null,

  setSelectedPortfolio: (name) => set({ selectedPortfolio: name }),
  setSelectedScenario: (name, year) =>
    set({ selectedScenario: name, selectedYear: year }),

  currentResults: null,
  resultHistory: [],

  setCurrentResults: (results) => set({ currentResults: results }),
  addToHistory: (results) =>
    set((state) => ({
      resultHistory: [...state.resultHistory, results]
    })),
}));
```

#### 4. Visualization Components

```typescript
// dashboard/src/components/charts/LossDistributionChart.tsx

import { useMemo } from 'react';
import { BarChart, Bar, XAxis, YAxis, Tooltip, Legend } from 'recharts';
import { SimulationResult } from '../../api/client';

interface Props {
  result: SimulationResult;
}

export function LossDistributionChart({ result }: Props) {
  const histogramData = useMemo(() => {
    if (!result.losses) return [];

    // Create histogram bins
    const numBins = 50;
    const min = Math.min(...result.losses);
    const max = Math.max(...result.losses);
    const binWidth = (max - min) / numBins;

    const bins = Array(numBins).fill(0).map((_, i) => ({
      bin: min + i * binWidth,
      count: 0,
    }));

    result.losses.forEach(loss => {
      const binIndex = Math.min(
        Math.floor((loss - min) / binWidth),
        numBins - 1
      );
      bins[binIndex].count++;
    });

    return bins;
  }, [result.losses]);

  return (
    <div className="chart-container">
      <h3>Loss Distribution</h3>
      <BarChart width={800} height={400} data={histogramData}>
        <XAxis
          dataKey="bin"
          label={{ value: 'Loss (USD)', position: 'insideBottom', offset: -5 }}
          tickFormatter={(value) => `$${(value / 1e6).toFixed(0)}M`}
        />
        <YAxis label={{ value: 'Frequency', angle: -90, position: 'insideLeft' }} />
        <Tooltip
          formatter={(value: number) => [value, 'Count']}
          labelFormatter={(value: number) => `Loss: $${(value / 1e6).toFixed(1)}M`}
        />
        <Bar dataKey="count" fill="#3b82f6" />
      </BarChart>

      {/* VaR markers */}
      <div className="metrics-overlay">
        <div className="metric">
          <span>VaR 95%:</span>
          <span>${(result.var_95 / 1e6).toFixed(1)}M</span>
        </div>
        <div className="metric">
          <span>VaR 99%:</span>
          <span>${(result.var_99 / 1e6).toFixed(1)}M</span>
        </div>
      </div>
    </div>
  );
}
```

---

## Data Flow

### 1. Simulation Request Flow

```
User clicks "Run Simulation"
    ↓
React: Validate inputs (portfolio, scenario, config)
    ↓
React: POST /api/simulate
    {
      "portfolio": "sample_portfolio_500",
      "scenario": "Net Zero 2050",
      "year": 2030,
      "config": {
        "num_simulations": 10000,
        "num_threads": 4
      }
    }
    ↓
C++: Generate job_id = uuid()
    ↓
C++: Create SimulationJob { status: "queued" }
    ↓
C++: Add job to queue
    ↓
C++: Return { "job_id": "abc-123" } immediately (HTTP 202)
    ↓
React: Start polling GET /api/results/{job_id}
    ↓
C++: Worker thread picks job from queue
    ↓
C++: Update status to "running"
    ↓
C++: Load portfolio, scenario, calibration
    ↓
C++: Run Monte Carlo simulation (parallel)
    ├─ Thread 0: Simulations 0-2499
    ├─ Thread 1: Simulations 2500-4999
    ├─ Thread 2: Simulations 5000-7499
    └─ Thread 3: Simulations 7500-9999
    ↓
C++: Aggregate results, compute metrics
    ↓
C++: Store results in SQLite
    ↓
C++: Update job status to "completed"
    ↓
React: GET /api/results/{job_id} → status: "completed"
    ↓
React: Extract result, update state
    ↓
React: Render charts and tables
```

### 2. Data Transformation Pipeline

```
CSV Files
    ↓
[C++ CSV Parser]
    ↓
C++ Objects (Portfolio, Scenario, Calibration)
    ↓
[Simulation Engine]
    ↓
SimulationResult struct
    ↓
[JSON Serialization]
    ↓
HTTP Response (JSON)
    ↓
[React Fetch API]
    ↓
TypeScript interfaces
    ↓
[React State (Zustand)]
    ↓
React Components
    ↓
[Recharts/Plotly]
    ↓
SVG/Canvas Visualization
```

---

## API Specification

### Base URL

```
http://localhost:8080/api
```

### Endpoints

#### 1. List Portfolios

```http
GET /api/portfolios
```

**Response:**
```json
{
  "portfolios": [
    {
      "name": "sample_portfolio_500",
      "num_obligors": 500,
      "total_ead": 5800000000,
      "created_at": "2026-02-25T12:30:00Z"
    }
  ]
}
```

#### 2. Upload Portfolio

```http
POST /api/portfolios
Content-Type: multipart/form-data

Form Data:
- portfolio: <file> (CSV file)
```

**Response:**
```json
{
  "name": "my_portfolio",
  "num_obligors": 500,
  "status": "loaded"
}
```

#### 3. List Scenarios

```http
GET /api/scenarios
```

**Response:**
```json
{
  "scenarios": [
    {
      "name": "Net Zero 2050",
      "years": [2030, 2040, 2050],
      "description": "Immediate policy action, 1.5°C warming"
    },
    {
      "name": "Delayed Transition",
      "years": [2030, 2040, 2050],
      "description": "Late but strong policy, 2°C warming"
    },
    {
      "name": "Current Policies",
      "years": [2030, 2040, 2050],
      "description": "No new climate policies, 3°C+ warming"
    }
  ]
}
```

#### 4. Start Simulation

```http
POST /api/simulate
Content-Type: application/json

{
  "portfolio": "sample_portfolio_500",
  "scenario": "Net Zero 2050",
  "year": 2030,
  "config": {
    "num_simulations": 10000,
    "num_threads": 4,
    "random_seed": 0
  }
}
```

**Response:**
```json
{
  "job_id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "queued"
}
```

#### 5. Get Simulation Results

```http
GET /api/results/{job_id}
```

**Response (while running):**
```json
{
  "job_id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "running",
  "portfolio_name": "sample_portfolio_500",
  "scenario_name": "Net Zero 2050",
  "created_at": "2026-02-27T10:30:00Z"
}
```

**Response (completed):**
```json
{
  "job_id": "550e8400-e29b-41d4-a716-446655440000",
  "status": "completed",
  "portfolio_name": "sample_portfolio_500",
  "scenario_name": "Net Zero 2050",
  "scenario_year": 2030,
  "num_simulations": 10000,
  "elapsed_seconds": 8.5,
  "result": {
    "expected_loss": 58000000,
    "var_95": 120000000,
    "var_99": 180000000,
    "cvar_99": 210000000,
    "max_loss": 350000000,
    "sector_el": {
      "0": 12000000,
      "1": 8000000,
      ...
    },
    "region_el": {
      "0": 15000000,
      "1": 20000000,
      ...
    }
  },
  "completed_at": "2026-02-27T10:30:08Z"
}
```

#### 6. Compute Efficient Frontier

```http
POST /api/efficient-frontier
Content-Type: application/json

{
  "portfolio": "sample_portfolio_500",
  "scenario": "Net Zero 2050",
  "year": 2030,
  "num_points": 20,
  "config": {
    "num_simulations": 5000
  }
}
```

**Response:**
```json
{
  "job_id": "...",
  "status": "queued"
}
```

Results follow same polling pattern as simulation.

---

## Directory Structure

### Complete Project Structure

```
midas/
├── .gitignore
├── README.md
├── LICENSE
│
├── docs/
│   ├── architecture.md                    # This document
│   ├── credit_portfolio_model_climate_overlay.md
│   ├── model_critique.md
│   ├── implementation_plan.md
│   ├── data_sources.md
│   └── api_reference.md                   # Detailed API docs (to be created)
│
├── data/
│   ├── portfolios/
│   │   └── sample_portfolio_500.csv
│   ├── scenarios/
│   │   ├── net_zero_2050.csv
│   │   ├── delayed_transition.csv
│   │   └── current_policies.csv
│   ├── calibration/
│   │   ├── sector_scores.csv
│   │   ├── region_scores.csv
│   │   ├── lambda_parameters.csv
│   │   └── residual_variance.csv
│   ├── market_data/
│   │   ├── regional_etf_prices.csv
│   │   └── sector_etf_prices.csv
│   └── templates/
│       ├── portfolio_template.csv
│       ├── scenario_template.csv
│       └── calibration_template.csv
│
├── engine/                                 # C++ Backend
│   ├── CMakeLists.txt
│   ├── .clang-format
│   │
│   ├── include/
│   │   └── midas/
│   │       ├── portfolio.hpp
│   │       ├── scenario.hpp
│   │       ├── calibration.hpp
│   │       ├── factor_generator.hpp
│   │       ├── simulator.hpp
│   │       ├── random.hpp
│   │       ├── risk_metrics.hpp
│   │       ├── efficient_frontier.hpp
│   │       ├── csv_loader.hpp
│   │       ├── json_utils.hpp
│   │       └── types.hpp
│   │
│   ├── src/
│   │   ├── core/
│   │   │   ├── portfolio.cpp
│   │   │   ├── scenario.cpp
│   │   │   ├── calibration.cpp
│   │   │   ├── factor_generator.cpp
│   │   │   ├── simulator.cpp
│   │   │   └── random.cpp
│   │   │
│   │   ├── analytics/
│   │   │   ├── risk_metrics.cpp
│   │   │   ├── efficient_frontier.cpp
│   │   │   └── contribution_analysis.cpp
│   │   │
│   │   ├── io/
│   │   │   ├── csv_loader.cpp
│   │   │   ├── sqlite_store.cpp
│   │   │   └── json_utils.cpp
│   │   │
│   │   ├── api/
│   │   │   ├── server.cpp
│   │   │   ├── handlers.cpp
│   │   │   └── job_queue.cpp
│   │   │
│   │   └── main.cpp
│   │
│   ├── tests/
│   │   ├── CMakeLists.txt
│   │   ├── test_main.cpp
│   │   ├── test_portfolio.cpp
│   │   ├── test_scenario.cpp
│   │   ├── test_calibration.cpp
│   │   ├── test_factor_generator.cpp
│   │   ├── test_simulator.cpp
│   │   ├── test_risk_metrics.cpp
│   │   └── test_csv_loader.cpp
│   │
│   └── external/                           # Git submodules
│       ├── eigen/                          # Linear algebra
│       ├── crow/                           # REST API framework
│       ├── json/                           # nlohmann/json
│       └── googletest/                     # Unit testing
│
├── dashboard/                              # React Frontend
│   ├── package.json
│   ├── tsconfig.json
│   ├── vite.config.ts
│   ├── .eslintrc.json
│   ├── .prettierrc
│   │
│   ├── public/
│   │   ├── index.html
│   │   └── favicon.ico
│   │
│   └── src/
│       ├── main.tsx
│       ├── App.tsx
│       ├── index.css
│       │
│       ├── api/
│       │   └── client.ts                  # API client
│       │
│       ├── components/
│       │   ├── Layout.tsx
│       │   ├── Header.tsx
│       │   ├── Sidebar.tsx
│       │   ├── PortfolioView.tsx
│       │   ├── PortfolioUploader.tsx
│       │   ├── PortfolioList.tsx
│       │   ├── PortfolioSummary.tsx
│       │   ├── SimulationView.tsx
│       │   ├── ScenarioSelector.tsx
│       │   ├── ConfigPanel.tsx
│       │   ├── RunButton.tsx
│       │   ├── JobStatusPanel.tsx
│       │   ├── ResultsView.tsx
│       │   └── RiskMetricsTable.tsx
│       │
│       ├── charts/
│       │   ├── LossDistributionChart.tsx
│       │   ├── SectorContributionChart.tsx
│       │   ├── RegionContributionChart.tsx
│       │   └── EfficientFrontierChart.tsx
│       │
│       ├── hooks/
│       │   ├── useSimulation.ts
│       │   ├── usePortfolio.ts
│       │   └── useScenarios.ts
│       │
│       ├── store/
│       │   └── appStore.ts               # Zustand state management
│       │
│       ├── types/
│       │   └── api.ts                    # TypeScript interfaces
│       │
│       └── utils/
│           ├── formatters.ts
│           └── constants.ts
│
├── scripts/
│   ├── download_market_data.py
│   ├── process_calibration.py
│   └── generate_synthetic_portfolio.py
│
└── .github/
    └── workflows/
        ├── cpp_build.yml
        └── react_build.yml
```

---

## Technology Stack

### C++ Engine

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| Language | C++ | C++20 | Core language standard |
| Build System | CMake | 3.20+ | Cross-platform build configuration |
| Compiler | GCC/Clang/MSVC | GCC 10+, Clang 12+, MSVC 2019+ | C++20 support |
| Linear Algebra | Eigen3 | 3.4+ | Matrix operations, Cholesky decomposition |
| REST API | Crow | 1.0+ | HTTP server framework |
| JSON | nlohmann/json | 3.11+ | JSON serialization/deserialization |
| CSV Parsing | fast-cpp-csv-parser | Latest | CSV file reading |
| Database | SQLite3 | 3.35+ | Results persistence |
| Testing | Google Test | 1.13+ | Unit testing framework |
| Random | `<random>` (STL) | C++20 | Mersenne Twister PRNG |
| Parallelization | OpenMP or `<thread>` | C++20 | Multi-threading |

### React Frontend

| Component | Technology | Version | Purpose |
|-----------|-----------|---------|---------|
| Language | TypeScript | 5.0+ | Type-safe JavaScript |
| Framework | React | 18+ | UI framework |
| Build Tool | Vite | 5.0+ | Fast dev server and bundler |
| State Management | Zustand | 4.5+ | Lightweight state management |
| Data Fetching | TanStack Query | 5.0+ | Async state management |
| Charts | Recharts | 2.10+ | React-based charting library |
| Advanced Charts | Plotly.js | 2.27+ | Interactive 3D plots (efficient frontier) |
| UI Components | Tailwind CSS | 3.4+ | Utility-first CSS framework |
| Icons | Lucide React | Latest | Icon library |
| Formatting | Prettier | 3.0+ | Code formatting |
| Linting | ESLint | 8.50+ | Code linting |

### Development Tools

| Tool | Purpose |
|------|---------|
| Git | Version control |
| GitHub Actions | CI/CD pipelines |
| Docker | Containerization (optional) |
| clang-format | C++ code formatting |
| clang-tidy | C++ static analysis |
| Valgrind | Memory leak detection |
| gdb/lldb | Debugging |

---

## Build and Deployment

### C++ Engine Build

#### Prerequisites

```bash
# macOS (Homebrew)
brew install cmake eigen crow nlohmann-json sqlite3

# Ubuntu/Debian
sudo apt install cmake libeigen3-dev libsqlite3-dev

# Clone with submodules
git clone --recursive https://github.com/yourusername/midas.git
cd midas
```

#### Build Steps

```bash
# Create build directory
cd engine
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Install (optional)
sudo make install
```

#### CMakeLists.txt Structure

```cmake
# engine/CMakeLists.txt

cmake_minimum_required(VERSION 3.20)
project(midas VERSION 1.0.0 LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies
find_package(Eigen3 3.4 REQUIRED)
find_package(SQLite3 REQUIRED)

# Include directories
include_directories(include)
include_directories(external/crow/include)
include_directories(external/json/include)

# Source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

# Library
add_library(midas_core ${SOURCES})
target_link_libraries(midas_core
    PUBLIC Eigen3::Eigen
    PUBLIC SQLite::SQLite3
    PUBLIC pthread
)

# Executable
add_executable(midas_server src/main.cpp)
target_link_libraries(midas_server midas_core)

# OpenMP support (optional)
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(midas_core PUBLIC OpenMP::OpenMP_CXX)
endif()

# Tests
option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(external/googletest)
    add_subdirectory(tests)
endif()

# Install
install(TARGETS midas_server DESTINATION bin)
```

### React Frontend Build

#### Prerequisites

```bash
# Install Node.js 18+ and npm
# macOS: brew install node
# Ubuntu: sudo apt install nodejs npm

cd dashboard
npm install
```

#### Development

```bash
# Start dev server (with hot reload)
npm run dev

# Open browser to http://localhost:5173
```

#### Production Build

```bash
# Build for production
npm run build

# Preview production build
npm run preview

# Output in dashboard/dist/
```

#### package.json

```json
{
  "name": "midas-dashboard",
  "version": "1.0.0",
  "type": "module",
  "scripts": {
    "dev": "vite",
    "build": "tsc && vite build",
    "preview": "vite preview",
    "lint": "eslint src --ext ts,tsx",
    "format": "prettier --write src"
  },
  "dependencies": {
    "react": "^18.2.0",
    "react-dom": "^18.2.0",
    "zustand": "^4.5.0",
    "@tanstack/react-query": "^5.17.0",
    "recharts": "^2.10.0",
    "plotly.js": "^2.27.0",
    "react-plotly.js": "^2.6.0",
    "lucide-react": "^0.307.0"
  },
  "devDependencies": {
    "@types/react": "^18.2.48",
    "@types/react-dom": "^18.2.18",
    "@vitejs/plugin-react": "^4.2.1",
    "typescript": "^5.3.3",
    "vite": "^5.0.11",
    "eslint": "^8.56.0",
    "prettier": "^3.2.4",
    "tailwindcss": "^3.4.1",
    "autoprefixer": "^10.4.16",
    "postcss": "^8.4.33"
  }
}
```

### Deployment

#### Local Development

```bash
# Terminal 1: Start C++ server
cd engine/build
./midas_server

# Terminal 2: Start React dev server
cd dashboard
npm run dev

# Browser: http://localhost:5173 → connects to http://localhost:8080/api
```

#### Production Deployment

**Option 1: Single server (C++ serves static files)**
```cpp
// engine/src/main.cpp
app.route("/<path>").methods("GET"_method)
    ([](const crow::request& req, std::string path) {
        // Serve React build files from dashboard/dist/
        return serve_static_file(path);
    });
```

**Option 2: Separate services (Nginx reverse proxy)**
```nginx
# nginx.conf
server {
    listen 80;

    location /api/ {
        proxy_pass http://localhost:8080;
    }

    location / {
        root /var/www/midas/dashboard/dist;
        try_files $uri $uri/ /index.html;
    }
}
```

**Option 3: Docker Compose**
```yaml
# docker-compose.yml
version: '3.8'
services:
  engine:
    build: ./engine
    ports:
      - "8080:8080"
    volumes:
      - ./data:/app/data

  dashboard:
    build: ./dashboard
    ports:
      - "80:80"
    depends_on:
      - engine
    environment:
      - VITE_API_URL=http://engine:8080
```

---

## Performance Considerations

### C++ Engine Optimization

#### 1. Memory Management

- **Pre-allocate vectors**: Reserve capacity for `losses` vector (10K simulations)
- **Eigen matrix storage**: Use column-major for cache efficiency
- **Thread-local storage**: Each thread has own RNG, avoids mutex contention

```cpp
// Good: Pre-allocate
std::vector<double> losses;
losses.reserve(num_simulations);

// Good: Thread-local RNG
thread_local RandomGenerator rng(seed + thread_id);
```

#### 2. Parallelization Strategy

- **Embarrassingly parallel**: Each simulation is independent
- **Thread pool**: Use `num_threads = std::thread::hardware_concurrency()`
- **Load balancing**: Divide simulations evenly across threads

```cpp
#pragma omp parallel for
for (int i = 0; i < num_simulations; ++i) {
    thread_local RandomGenerator rng;
    losses[i] = simulate_once(rng, mean_factors);
}
```

#### 3. Linear Algebra

- **Cholesky decomposition**: Compute once, reuse for all simulations
- **Matrix-vector products**: Leverage Eigen's vectorization (SSE/AVX)
- **Avoid temporaries**: Use `noalias()` for in-place operations

```cpp
// Compute Cholesky once
Eigen::LLT<Eigen::MatrixXd> llt(sigma_u);
L_ = llt.matrixL();

// Fast sampling: z = L * u where u ~ N(0,I)
Eigen::VectorXd u = rng.normal_vector(105);
Eigen::VectorXd z = L_ * u;  // Eigen optimizes this
```

#### 4. I/O Optimization

- **Memory-mapped files**: Consider for large CSV files (>100MB)
- **Streaming CSV parsing**: Don't load entire file into memory
- **SQLite prepared statements**: Avoid SQL injection, faster inserts

#### 5. Profiling

- **CPU profiling**: Use `perf` (Linux) or Instruments (macOS)
- **Memory profiling**: Valgrind's massif for heap usage
- **Hotspot identification**: Focus optimization on <5% of code that takes >80% of time

### React Frontend Optimization

#### 1. Data Handling

- **Virtualized lists**: Use `react-window` for large portfolios (>1000 obligors)
- **Memoization**: Use `useMemo` for expensive computations (histogram binning)
- **Debouncing**: Debounce config changes to avoid excessive API calls

```typescript
const histogramData = useMemo(() => {
  return computeHistogram(result.losses);
}, [result.losses]);
```

#### 2. Chart Performance

- **Downsample data**: For >10K points, show every 10th point
- **Canvas vs SVG**: Use Canvas for large datasets (Plotly), SVG for small (Recharts)
- **Lazy loading**: Load charts only when visible (Intersection Observer)

#### 3. State Management

- **Normalize data**: Store results by `job_id` in flat object, not nested arrays
- **Selective subscriptions**: Components subscribe only to needed state slices
- **Persistent cache**: Use TanStack Query's cache for API responses

#### 4. Bundle Size

- **Code splitting**: Lazy load chart components
- **Tree shaking**: Import specific Plotly modules, not entire library
- **Compression**: Enable gzip/brotli on static assets

### Performance Targets

| Metric | Target | Notes |
|--------|--------|-------|
| **Simulation Time** | <10s | 10K sims × 500 obligors (4 threads) |
| **API Response Time** | <100ms | GET /api/scenarios |
| **Job Polling Latency** | <500ms | Client polls every 500ms |
| **Frontend Load Time** | <2s | Time to interactive (production build) |
| **Chart Render Time** | <100ms | Loss distribution (50 bins) |
| **Memory Usage (C++)** | <2GB | 10K simulations, 105×105 covariance |
| **Memory Usage (React)** | <100MB | With full loss distribution cached |

---

## Next Steps

### Phase 1: Minimal Viable Product (MVP)

**Goal**: Basic simulation engine + simple UI

1. **Week 1-2**: C++ Core
   - Implement Portfolio, Scenario, Calibration classes
   - CSV loaders
   - Factor generator
   - Basic simulator (single-threaded)
   - Unit tests

2. **Week 2-3**: C++ API
   - Crow REST server
   - Portfolio and scenario endpoints
   - Simulation endpoint (synchronous)
   - JSON serialization

3. **Week 3-4**: React UI
   - Project setup (Vite + TypeScript)
   - API client
   - Portfolio selection
   - Scenario selection
   - Run simulation button
   - Results table (risk metrics only)

### Phase 2: Production Features

4. **Week 5**: Async Job Queue
   - Thread pool
   - Job status tracking
   - SQLite results storage

5. **Week 6**: Visualizations
   - Loss distribution chart
   - Sector/region contribution charts

6. **Week 7**: Parallelization
   - OpenMP or std::thread
   - Performance testing

7. **Week 8**: Advanced Analytics
   - Efficient frontier
   - Contribution analysis

### Phase 3: Polish

8. **Week 9**: Testing & Documentation
   - Integration tests
   - API documentation
   - User guide

9. **Week 10**: Deployment
   - Docker containers
   - CI/CD pipeline
   - Production deployment

---

## Appendices

### A. CSV File Formats

#### Portfolio CSV

```csv
ObligorID,Name,Sector,Region,PD,LGD,EAD,Beta_0,Beta_1,...,Beta_104
OB-001,Acme Corp,0,0,0.05,0.45,10000000,0.8,0.1,...,0.05
```

#### Scenario CSV

```csv
Scenario,Year,Driver,Region,Value
Net Zero 2050,2030,CarbonPrice,0,1.5
Net Zero 2050,2030,CarbonPrice,1,1.8
```

#### Sector Scores CSV

```csv
Sector,CarbonPrice,CoalPrice,OilPrice,GasPrice,GDP,HeatIndex,FloodRisk,DroughtRisk
Energy - Coal,1.00,1.00,0.50,0.50,0.70,0.30,0.30,0.45
Energy - Oil & Gas,0.90,0.50,1.00,0.80,0.70,0.30,0.45,0.40
```

### B. Glossary

| Term | Definition |
|------|------------|
| **Obligor** | Entity that owes a debt (borrower, bond issuer) |
| **EAD** | Exposure at Default - total exposure if obligor defaults |
| **PD** | Probability of Default - likelihood of default in 1 year |
| **LGD** | Loss Given Default - fraction of EAD lost if default occurs |
| **VaR** | Value at Risk - loss threshold at given confidence level |
| **CVaR** | Conditional VaR - expected loss given loss exceeds VaR |
| **Factor** | Systematic risk driver (sector-region cell) |
| **Latent Variable** | Unobserved creditworthiness Z_i |
| **Climate Driver** | Observable climate scenario variable (carbon price, heat index) |
| **Scenario** | Deterministic climate pathway (NGFS) |

### C. References

- **NGFS Climate Scenarios**: https://www.ngfs.net/
- **Crow Framework**: https://crowcpp.org/
- **Eigen Documentation**: https://eigen.tuxfamily.org/
- **React Documentation**: https://react.dev/
- **TanStack Query**: https://tanstack.com/query/
- **Recharts**: https://recharts.org/

---

**End of Architecture Document**

# Credit Portfolio Model (CPM)

A climate-aware credit portfolio risk model with C++ engine and React dashboard for scenario analysis and risk-return optimization.

## Overview

This model implements a factor-based credit portfolio framework with deterministic climate scenario overlays. It computes portfolio loss distributions, risk metrics (VaR, CVaR), return analytics (expected return, Sharpe ratio), and efficient frontier visualization under multiple climate scenarios (NGFS pathways).

**Key Features:**
- Climate scenario integration (transition & physical risk)
- Monte Carlo simulation engine (C++20 with parallel processing)
- Risk and return analytics with Sharpe ratio
- Efficient frontier calculation
- Interactive React dashboard for visualization
- CSV-based data import/export
- SQLite database for storage

## Architecture

- **Backend:** C++ engine with REST API (Crow framework, Eigen3 for linear algebra)
- **Frontend:** React + TypeScript + Vite (Recharts, Plotly.js, D3.js for visualizations)
- **Data:** CSV files for I/O, SQLite for persistent storage

## Project Structure

```
cpm/
├── README.md                    # This file
├── docs/                        # Documentation
│   ├── credit_portfolio_model_climate_overlay.md  # Model methodology
│   ├── model_critique.md        # Critical review of approach
│   └── implementation_plan.md   # 16-18 week implementation guide
├── data/                        # Data files
│   ├── portfolios/              # Portfolio CSV files
│   │   └── sample_portfolio_500.csv  # 500 synthetic obligors
│   ├── scenarios/               # NGFS climate scenario data (to be added)
│   ├── calibration/             # Calibration parameters (to be added)
│   └── templates/               # CSV templates with documentation
│       ├── portfolio_template.csv
│       ├── scenario_template.csv
│       ├── sector_scores_template.csv
│       ├── region_scores_template.csv
│       └── lambda_parameters_template.csv
├── engine/                      # C++ backend (to be implemented)
├── dashboard/                   # React frontend (to be implemented)
└── external/                    # External C++ libraries (git submodules)
```

## Documentation

### Key Documents

- **[Model Methodology](docs/credit_portfolio_model_climate_overlay.md)** - Complete mathematical specification of the factor-based climate overlay approach
- **[Model Critique](docs/model_critique.md)** - Critical review identifying strengths, weaknesses, and implementation challenges
- **[Implementation Plan](docs/implementation_plan.md)** - Detailed 6-phase build plan with milestones and deliverables

### Taxonomy

**15 Sectors:**
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

**7 Regions:**
1. North America
2. Europe
3. Asia-Pacific (developed)
4. Asia-Pacific (emerging)
5. Latin America
6. Middle East & Africa
7. Global

## Quick Start

### Prerequisites
- C++20 compiler (GCC 10+, Clang 12+, or MSVC 2019+)
- CMake 3.20+
- SQLite3
- Node.js 18+ and npm
- Git with submodules support

### Build (Coming Soon)
```bash
# Clone with submodules
git clone --recursive https://github.com/ommthree/cpm.git

# Build C++ engine
cd engine && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j

# Build React dashboard
cd ../../dashboard
npm install
npm run dev
```

## Current Status

**Phase 1: Portfolio Data Preparation** - ✅ Complete
- Directory structure created
- CSV templates with full documentation
- Synthetic portfolio (500 obligors) generated

**Phase 2-6:** To be implemented per `implementation_plan.md`

## Data Templates

All CSV templates in `data/templates/` include:
- Column definitions
- Valid value ranges
- Validation rules
- Data sources and references
- Pre-filled examples

See templates for detailed guidance on data formatting.

## Model Equation Summary

**Factor mapping:**
```
F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k(C)
```

**Obligor creditworthiness:**
```
Z_i = β_i^T vec(F) + ε_i
```

**Default rule:**
```
Default if Z_i < Φ^(-1)(PD_i)
```

Where:
- F = sector-region factor matrix
- C = climate scenario drivers (deterministic)
- φ(C) = standardized drivers
- w̃ = relative weights (sector × region scores)
- λ = scale parameters
- β_i = obligor exposures to sector-region cells
- ε_i = idiosyncratic noise

## License

[To be determined]

## Contact

[Repository owner contact information]

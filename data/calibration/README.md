# Model Calibration Parameters

This directory contains calibration templates for the credit portfolio model with climate overlay.

## Overview

The model uses a **hybrid calibration approach** with structured parameters:

```
F_{s,r} = m_{s,r}(X) + u_{s,r}
```

Where:
- **m_{s,r}(X)**: Climate-driven mean = Σ_k λ_k · S_{s,k} · R_{r,k} · φ_k(X)
- **u_{s,r}**: Residual = a_s + b_r + ξ_{s,r}

## Calibration Files

### 1. sector_scores.csv (S matrix: 15 × 8)

**Purpose**: Relative sector exposure to each driver

**Structure**: Each S_{s,k} ∈ [0,1] represents: "How exposed is sector s to driver k?"

**15 Sectors**:
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

**8 Drivers**:
- **Transition (5)**: CarbonPrice, CoalPrice, OilPrice, GasPrice, GDP
- **Physical (3)**: HeatIndex, FloodRisk, DroughtRisk

**Calibration approach**:
- Emissions intensity (CDP, OECD data)
- Energy intensity (IEA statistics)
- Physical asset exposure (industry studies)
- Expert judgment with documented rationale

### 2. region_scores.csv (R matrix: 7 × 8)

**Purpose**: Relative region exposure to each driver

**Structure**: Each R_{r,k} ∈ [0,1] represents: "How exposed is region r to driver k?"

**7 Regions**:
1. North America
2. Europe
3. Asia-Pacific (developed)
4. Asia-Pacific (emerging)
5. Latin America
6. Middle East & Africa
7. Global

**Calibration approach**:
- Policy stringency (OECD Climate Policy Tracker)
- Energy mix (IEA World Energy Outlook)
- Physical hazard exposure (World Bank CCKP, IPCC AR6)
- GDP volatility (World Bank development indicators)

### 3. lambda_parameters.csv (λ vector: 8 scalars)

**Purpose**: Global magnitude control for climate-to-credit transmission

**Structure**: Each λ_k controls: "How strongly does a 1σ move in driver k translate to credit stress?"

**Calibration approach**:
- **Reasonableness targets**: Max exposed cell moves ~2σ under severe scenario
- **Portfolio anchors**: EL uplift should be 2-5x baseline under severe scenario
- **Confidence weighting**:
  - High λ (0.9-1.2): Well-understood (GDP, carbon price)
  - Medium λ (0.6-0.8): Established but uncertain (energy prices, physical risk)
  - Low λ (0.3-0.5): Emerging risks with weak empirical validation

**Proposed values**:
- CarbonPrice: 0.80
- CoalPrice: 0.70
- OilPrice: 0.75
- GasPrice: 0.65
- GDP: 1.00
- HeatIndex: 0.70
- FloodRisk: 0.60
- DroughtRisk: 0.70

### 4. residual_variance.csv (3 variance components)

**Purpose**: Variance decomposition for stochastic residuals

**Structure**: u_{s,r} = a_s + b_r + ξ_{s,r}
- **a_s ~ N(0, σ_a²)**: Sector-wide shocks
- **b_r ~ N(0, σ_b²)**: Region-wide shocks
- **ξ_{s,r} ~ N(0, σ_ξ²)**: Cell-specific shocks

**Induced correlations**:
- Within-sector: ρ_s = σ_a² / (σ_a² + σ_b² + σ_ξ²)
- Within-region: ρ_r = σ_b² / (σ_a² + σ_b² + σ_ξ²)

**Calibration approach**:
- Target empirical equity correlations from MSCI sector/regional indices
- Target ρ_s ≈ 0.40 (sector co-movement)
- Target ρ_r ≈ 0.25 (region co-movement)
- Total variance ≈ 0.64 (σ_total ≈ 0.80)

**Proposed values**:
- σ_a = 0.50 (sector std dev)
- σ_b = 0.40 (region std dev)
- σ_ξ = 0.47 (cell std dev)

## Total Parameter Count

**Hybrid approach**: 184 parameters
- S matrix: 15 sectors × 8 drivers = 120
- R matrix: 7 regions × 8 drivers = 56
- λ vector: 8 scale parameters
- Variance: 3 components (σ_a, σ_b, σ_ξ)

**Alternative (independent weights)**: 840 parameters
- Full w_{s,r,k} matrix: 15 × 7 × 8 = 840

**Alternative (full covariance)**: 5,460 parameters
- Full Σ_F|C covariance: 105 × 105 symmetric = 5,565 unique entries

## Calibration Workflow

1. **Start with templates**: Use provided CSV files as starting point
2. **Adjust S matrix**: Review sector exposures based on portfolio composition
3. **Adjust R matrix**: Review region exposures based on climate projections
4. **Run simulations**: Test scenarios (Net Zero 2050, Current Policies)
5. **Check targets**:
   - Factor magnitudes: Most exposed cells move ~2σ under severe scenarios
   - Portfolio EL: Uplift is 2-5x baseline under severe scenarios
   - Sectoral contributions: Brown sectors dominate transition losses
6. **Adjust λ**: Scale up/down to hit targets
7. **Validate variance**: Check if correlations match equity market empirics
8. **Document**: Record final values and rationale

## Validation and Sensitivity

All calibration parameters should undergo:
- **Expert review**: Sector specialists, climate scientists, risk managers
- **Sensitivity analysis**: Vary ±30%, check impact on EL and VaR
- **Peer comparison**: Compare with NGFS reference models
- **Back-testing**: As climate-credit data accumulates, validate predictions

## Governance

- **Annual review**: Routine model validation cycle
- **Event-triggered review**: Major climate events, IPCC reports, regulatory changes
- **Approval requirements**: Model Risk Management sign-off for material changes
- **Documentation**: All changes require rationale and impact analysis
- **Reporting**: Sensitivity ranges accompany all Board/regulator stress test results

## Data Sources

**Transition risk**:
- NGFS Phase V scenarios (November 2024)
- IIASA NGFS Scenario Explorer
- CDP emissions disclosures
- IEA sector energy statistics

**Physical risk**:
- World Bank Climate Change Knowledge Portal (CCKP)
- CMIP6 projections (ACCESS-CM2 model)
- IPCC AR6 regional assessments
- EM-DAT disaster database

**Empirical validation**:
- MSCI sector/regional equity indices
- iTraxx/CDX sector credit spreads
- Historical PD-GDP elasticities
- Academic factor models (Fama-French)

## Future Enhancements

- [ ] Time-dependent λ_k(t): Physical risk magnitude increases over time
- [ ] Scenario-dependent variance: Higher uncertainty in disorderly transitions
- [ ] Country-level granularity: Replace regional aggregation with country-specific exposures
- [ ] Ensemble averaging: Use multiple CMIP6 models instead of single model
- [ ] Statistical estimation: As data accumulates, move from expert judgment to regression-based calibration
- [ ] Non-linear climate functions: φ_k(X) could be non-linear for threshold effects

## References

- **Model Documentation**: `/docs/credit_portfolio_model_climate_overlay.md`
- **Implementation Plan**: `/docs/implementation_plan.md` (Phase 3)
- **Scenario Data**: `/data/scenarios/README.md`
- **NGFS Documentation**: https://www.ngfs.net/en/publications-and-statistics/publications/ngfs-climate-scenarios-central-banks-and-supervisors-phase-v

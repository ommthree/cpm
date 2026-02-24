# Climate Scenario Data

## Overview

This directory contains climate scenario data for both transition and physical risks.

**Sources:**
- **Transition Risk:** IIASA NGFS Phase V (November 2024)
- **Physical Risk:** World Bank Climate Change Knowledge Portal (CMIP6 projections)

**Extraction Date:** February 2026
**Variables:**
- Transition drivers: Carbon prices, energy prices, GDP
- Physical drivers: Heat stress, flood risk, drought risk

## Processed Scenario Files

### Available Scenarios

1. **net_zero_2050.csv** - Net Zero 2050 scenario
   - Orderly transition to 1.5°C
   - Stringent climate policies starting immediately
   - Global net zero CO2 emissions around 2050

2. **delayed_transition.csv** - Delayed Transition scenario
   - Disorderly transition to below 2°C
   - Annual emissions do not decrease until 2030
   - Requires strong policies after 2030 to limit warming

3. **current_policies.csv** - Current Policies scenario
   - High physical risk scenario (3°C+ warming by 2100)
   - Based on currently implemented policies only
   - No further climate action assumed

### Data Structure

Each CSV file contains:

| Column | Description |
|--------|-------------|
| **ScenarioName** | Name of the NGFS scenario |
| **Year** | Year for projection (2025, 2030, 2040, 2050) |
| **Driver** | Climate/economic driver name |
| **Region** | Geographic region (7-region taxonomy) |
| **Value** | Raw value in source units |
| **StandardizedValue** | Standardized φ value (σ units) |
| **Notes** | Data source attribution |

### Transition Risk Drivers (5 drivers)

1. **CarbonPrice** - Carbon price (US$2010/t CO2)
   - Source: NGFS IAM `Price|Carbon`
   - Key transition risk indicator
   - Shows policy stringency across scenarios

2. **CoalPrice** - Coal price index (US$2010/GJ)
   - Source: NGFS IAM `Price|Primary Energy|Coal`
   - Reflects transition away from fossil fuels

3. **OilPrice** - Oil price index (US$2010/GJ)
   - Source: NGFS IAM `Price|Primary Energy|Oil`
   - Energy transition impact on fossil fuel prices

4. **GasPrice** - Natural gas price (US$2010/GJ)
   - Source: NGFS IAM `Price|Primary Energy|Gas`
   - Bridge fuel pricing under transition

5. **GDP** - Gross Domestic Product (billion US$2010/yr, PPP)
   - Source: NGFS IAM `GDP|PPP|Counterfactual without damage`
   - Macroeconomic impact of transition policies

### Physical Risk Drivers (3 drivers)

6. **HeatIndex** - Days with heat index >35°C (days/year)
   - Source: World Bank CCKP `hi35` indicator
   - CMIP6 model: ACCESS-CM2
   - Heat stress impact on outdoor workers, agriculture, infrastructure

7. **FloodRisk** - Maximum 1-day precipitation (mm)
   - Source: World Bank CCKP `rx1day` indicator
   - CMIP6 model: ACCESS-CM2
   - Extreme precipitation and flood hazard severity

8. **DroughtRisk** - Consecutive dry days (days)
   - Source: World Bank CCKP `cdd` indicator
   - CMIP6 model: ACCESS-CM2
   - Drought severity and water stress

### Regional Coverage

Data is provided for our 7-region taxonomy:

| Our Region | NGFS Source Regions (aggregated) |
|------------|----------------------------------|
| **North America** | Canada, USA, North America (R5/R11) |
| **Europe** | EU-12, EU-15, Europe Non-EU, Europe (R5/R11) |
| **Asia-Pacific (developed)** | Japan, Australia/NZ, South Korea |
| **Asia-Pacific (emerging)** | China, India, Indonesia, SE Asia, Asia (R5) |
| **Latin America** | Argentina, Brazil, Colombia, Central America, Latin America (R5/R11) |
| **Middle East & Africa** | Africa regions (Eastern, Northern, Southern, Western), Middle East, MEA (R5) |
| **Global** | World |

Note: Where multiple NGFS sub-regions map to one of our regions, values are averaged.

### Temporal Coverage

**Transition Risk Years:** 2025, 2030, 2040, 2050
**Physical Risk Years:** 2030, 2050

These key years provide:
- **2025:** Near-term baseline (transition only)
- **2030:** Critical policy decision point
- **2040:** Mid-century transition phase (transition only)
- **2050:** Net zero target year

**Note:** Physical risk data uses World Bank 20-year climatology periods:
- 2020-2039 → mapped to 2030
- 2040-2059 → mapped to 2050

## Standardization Methodology

**Standardized values (φ)** are computed as:

```
φ_k = (Value - Baseline) / σ_k
```

Where:
- **Baseline** = Current Policies scenario value (for that driver-region-year)
- **σ_k** = Cross-scenario standard deviation (Net Zero, Delayed, Current Policies)

**Interpretation:**
- **φ = 0**: Current Policies level (baseline)
- **φ = +1**: One standard deviation above baseline (more stress)
- **φ = -1**: One standard deviation below baseline (less stress)
- **φ = +2**: Two standard deviations (severe stress)

For our credit model, standardized values are used in:
```
F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k
```

## Full Dataset

**ngfs_transition_drivers_full.csv** contains all scenarios with additional fields:
- Baseline values
- Sigma (standard deviation)
- Source units
- Source variable names

Use this for:
- Sensitivity analysis
- Additional scenario exploration
- Calibration validation

## Raw NGFS Data

**OriginalNGFS/** directory contains:
- **IAM_data.xlsx** (55 MB) - Integrated Assessment Models (GCAM, MESSAGE, REMIND)
- **NiGEM_data.xlsx** (26 MB) - Macroeconomic model (National Institute Global Econometric Model)
- **reference.xlsx** - Metadata (models, scenarios, regions, variables)
- **Downscaled_*.xlsx** - Downscaled regional projections

## Data Quality Notes

### Coverage
- ✅ All 3 key scenarios have data
- ✅ All 7 regions have data
- ✅ All 5 drivers have data
- ✅ All 4 time points have data
- **Total: 420 data points extracted**

### Limitations

1. **Regional Aggregation:** Some regions are averages across NGFS sub-regions (e.g., Asia-Pacific emerging includes China, India, Indonesia). Consider heterogeneity within aggregated regions.

2. **Model Variation:** NGFS scenarios use multiple Integrated Assessment Models (GCAM, MESSAGEix, REMIND). We averaged across models where applicable.

3. **Physical Risk:** This dataset contains **transition risk drivers only**. Physical risk indicators (temperature, precipitation, hazards) need to be added separately.

4. **Baseline Definition:** Standardization uses Current Policies as baseline. Alternative baselines (e.g., 2020 historical) could be computed if needed.

5. **Interpolation:** Data is provided at 4 time points. For annual simulations, interpolation between years may be needed.

## Usage in Model

These scenario files are ready to be loaded by the C++ engine via:
- `POST /api/scenarios/import` endpoint
- CSV import functions in `DatabaseManager` class

After import, scenarios can be selected for simulation:
- `POST /api/simulate` with `scenario_ids` parameter
- Returns risk metrics and efficient frontier

## Physical Risk Data Processing

**Processing Script:** `process_physical_risk.py`

### SSP to NGFS Scenario Mapping

Physical risk data uses CMIP6 Shared Socioeconomic Pathways (SSPs):
- **ssp126** → Net Zero 2050 (low warming, ~1.5°C)
- **ssp245** → Delayed Transition (moderate warming, ~2°C)
- **ssp585** → Current Policies (high warming, ~3-4°C)

### Data Quality and Limitations

**Coverage:**
- ✅ 3 physical risk indicators integrated
- ✅ 5 transition risk drivers (from Phase 1)
- ✅ 8 total climate drivers
- ✅ 3 scenarios × 7 regions
- **Total data points:**
  - Transition: 420 rows (5 drivers × 4 years × 7 regions × 3 scenarios)
  - Physical: 126 rows (3 drivers × 2 years × 7 regions × 3 scenarios)
  - **Combined: 546 data points**

**Current Limitations:**
1. **Regional Aggregation:** Physical risk data currently uses global climatology with regional variation factors. Future enhancement: implement country-level NetCDF extraction and proper country-to-region aggregation using spatial masks.

2. **Single Climate Model:** Using ACCESS-CM2 only. Best practice would be to ensemble average across multiple CMIP6 models (e.g., ACCESS-CM2, CanESM5, CMCC-ESM2).

3. **Temporal Resolution:** Using 20-year climatology periods. Annual or decadal data would provide finer temporal resolution.

4. **Indicator Selection:** Selected 3 core physical risk indicators. Additional indicators to consider:
   - Tropical storm frequency/intensity
   - Sea level rise
   - Wildfire risk
   - Water stress indices

## Next Steps (Future Enhancements)

- [ ] Implement proper country-level NetCDF extraction with spatial masking
- [ ] Add ensemble averaging across multiple CMIP6 models
- [ ] Add additional physical risk indicators (storms, sea level, wildfire)
- [ ] Create baseline scenario (2020 historical) for reference
- [ ] Add Below 2°C and Low Demand NGFS scenarios (currently have 3 of 7)
- [ ] Consider adding NiGEM macroeconomic data for additional GDP/employment variables

## References

- **NGFS Phase V Documentation:** https://www.ngfs.net/en/publications-and-statistics/publications/ngfs-climate-scenarios-central-banks-and-supervisors-phase-v
- **IIASA Scenario Explorer:** https://data.ene.iiasa.ac.at/ngfs/
- **Technical Documentation:** NGFS Climate Scenarios Technical Documentation V5.0 (November 2024)

## Contact

For questions about the data extraction or processing, refer to the implementation plan (`docs/implementation_plan.md`).

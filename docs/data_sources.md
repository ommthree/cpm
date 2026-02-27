# Data Sources

**Purpose**: Documentation of external data sources for the Midas credit portfolio model with climate overlay

**Last Updated**: 2026-02-27

---

## Quick Reference

| Data Type | Source | Status | Location |
|-----------|--------|--------|----------|
| **Climate Scenarios** | NGFS Phase V | ✅ Downloaded | `/data/scenarios/*.csv` |
| **Physical Risk Projections** | World Bank CCKP | ✅ Downloaded | `/data/scenarios/*.csv` |
| **Regional Correlations** | Yahoo Finance (7 ETFs) | ✅ Downloaded | `/data/market_data/regional_etf_prices.csv` |
| **Sector Correlations** | Yahoo Finance (13 ETFs) | ✅ Downloaded | `/data/market_data/sector_etf_prices.csv` |
| **Transition Risk Sensitivity** | OECD Air Emissions Accounts | ✅ Downloaded | `/data/calibration/raw_data/OECD.SDD...csv` |
| **Physical Risk Sensitivity** | BIS Working Paper 1292 | ✅ Downloaded | `/data/calibration/raw_data/BISwork1292.pdf` |

---

## Part 1: Climate Scenario Data

### 1.1 Transition Risk Scenarios (NGFS Phase V)

**Provider**: Network for Greening the Financial System (NGFS)
**URL**: https://www.ngfs.net/en/publications-and-statistics/publications/ngfs-climate-scenarios-central-banks-and-supervisors-phase-v
**Database**: IIASA NGFS Scenario Explorer
**Version**: Phase V (November 2024)
**Status**: ✅ **Downloaded & Processed** (February 2026)

#### Scenarios

| NGFS Scenario | SSP Pathway | Description |
|---------------|-------------|-------------|
| Net Zero 2050 | SSP1-2.6 | Immediate policy action, 1.5°C warming |
| Delayed Transition | SSP2-4.5 | Late but strong policy, 2°C warming |
| Current Policies | SSP5-8.5 | No new climate policies, 3°C+ warming |

#### Variables

**5 Transition Drivers** (annual, 2025/2030/2040/2050, 7 regions):

1. **Carbon Price** - `Price|Carbon` (USD per tonne CO₂)
2. **Coal Price** - `Price|Primary Energy|Coal` (USD per GJ)
3. **Oil Price** - `Price|Primary Energy|Oil` (USD per GJ)
4. **Gas Price** - `Price|Primary Energy|Gas` (USD per GJ)
5. **GDP** - `GDP|PPP` (Billion USD PPP)

**Total**: 5 drivers × 4 years × 7 regions × 3 scenarios = **420 data points**

#### Regions

1. North America (USA, Canada)
2. Europe (EU27 + UK + EFTA)
3. Asia-Pacific (developed) - Japan, Australia, NZ
4. Asia-Pacific (emerging) - China, India, ASEAN
5. Latin America - Brazil, Mexico, etc.
6. Middle East & Africa
7. Global (world aggregate)

#### Files Created

- `/data/scenarios/net_zero_2050.csv` (183 rows)
- `/data/scenarios/delayed_transition.csv` (183 rows)
- `/data/scenarios/current_policies.csv` (183 rows)
- Drivers standardized: φ_k(X) = (X - μ) / σ

---

### 1.2 Physical Risk Projections (World Bank CCKP)

**Provider**: World Bank Climate Change Knowledge Portal
**URL**: https://climateknowledgeportal.worldbank.org/
**Climate Model**: CMIP6 ACCESS-CM2
**Resolution**: 0.5° grid (~50 km)
**Status**: ✅ **Downloaded & Processed** (February 2026)

#### Indicators

**3 Physical Risk Indicators** (2030s and 2050s climatology):

1. **Heat Index** - `hi35` (Days per year with heat index >35°C)
2. **Flood Risk** - `rx1day` (Maximum 1-day precipitation in mm)
3. **Drought Risk** - `cdd` (Consecutive dry days, maximum annual)

**SSP Scenarios**: SSP1-2.6, SSP2-4.5, SSP5-8.5 (maps to our 3 NGFS scenarios)

**Integration**: Physical risk data integrated into scenario CSVs above
- `/data/scenarios/*.csv` includes all 8 drivers (5 transition + 3 physical)
- Regional averages extracted (7 regions)
- Climatology computed (2030s, 2050s)
- Standardized and ready for model input

---

## Part 2: Market Correlation Data

### 2.1 Regional Correlations (Yahoo Finance)

**Purpose**: Construct Corr_R (7×7 regional correlation matrix)
**Source**: Yahoo Finance via yfinance Python library
**Time Period**: 2015-01-01 to 2024-12-31 (120 months)
**Status**: ✅ **Downloaded 2026-02-25**

#### ETF Mappings

| Region | ETF Ticker | ETF Name | Observations |
|--------|------------|----------|--------------|
| North America | SPY | S&P 500 ETF | 120 ✅ |
| Europe | VGK | FTSE Europe ETF | 120 ✅ |
| Asia-Pacific (developed) | VPL | FTSE Pacific ETF | 120 ✅ |
| Asia-Pacific (emerging) | VWO | Emerging Markets ETF | 120 ✅ |
| Latin America | ILF | Latin America 40 | 120 ✅ |
| Middle East & Africa | AFK | Africa ETF | 120 ✅ |
| Global | VT | Total World Stock ETF | 120 ✅ |

**Data Location**: `/data/market_data/regional_etf_prices.csv`
**Data Quality**: ✅ No missing data, all 7 regions complete

---

### 2.2 Sector Correlations (Yahoo Finance)

**Purpose**: Construct Corr_S (15×15 sector correlation matrix)
**Source**: Yahoo Finance via yfinance Python library
**Time Period**: 2015-01-01 to 2024-12-31 (120 months)
**Status**: ✅ **Downloaded 2026-02-25** (13 of 15 sectors)

#### ETF Mappings

| Sector | ETF Ticker | ETF Name | Observations |
|--------|------------|----------|--------------|
| **Energy - Oil & Gas** | XLE | Energy Select Sector | 120 ✅ |
| **Energy - Coal** | - | No ETF (use XLE proxy) | - ⚠️ |
| **Energy - Renewables** | ICLN | Clean Energy ETF | 120 ✅ |
| **Utilities** | XLU | Utilities Select | 120 ✅ |
| **Financials** | XLF | Financial Select | 120 ✅ |
| **Consumer Goods** | XLP | Consumer Staples Select | 120 ✅ |
| **Healthcare** | XLV | Health Care Select | 120 ✅ |
| **Mining & Metals** | XLB | Materials Select | 120 ✅ |
| **Real Estate** | XLRE | Real Estate Select | 111 ⚠️ (9 missing) |
| **Technology & Services** | XLK | Technology Select | 120 ✅ |
| **Manufacturing - Heavy** | XLI | Industrials Select | 120 ✅ |
| **Manufacturing - Light** | XLY | Consumer Discretionary | 120 ✅ |
| **Transportation** | IYT | Transportation ETF | 120 ✅ |
| **Agriculture** | MOO | Agribusiness ETF | 120 ✅ |
| **Other** | - | Derived (avg of others) | - 🔄 |

**Data Location**: `/data/market_data/sector_etf_prices.csv`

**Data Quality**:
- ✅ 13 of 15 sectors complete
- ⚠️ Real Estate (XLRE): 9 missing observations (ETF launched Oct 2015) - needs forward fill
- ⚠️ Coal: No public ETF - will use correlation with Oil & Gas (XLE) × 0.85

**Processing needed**:
- Handle Real Estate missing data (forward fill)
- Compute log returns
- Calculate Corr_S and Corr_R matrices
- Validate positive definiteness

---

## Part 3: Calibration Data (Climate Sensitivity)

### 3.1 Transition Risk Sensitivity (OECD Air Emissions Accounts)

**Purpose**: Calibrate sector and region sensitivity to carbon pricing via GHG emissions intensity

**Approach**: Model latent factors as asset-to-equity ratios. Higher emissions → higher carbon price exposure → deteriorating equity cushion → higher default probability.

**Source**: OECD Air Emissions Accounts - Complete Dataset
**URL**: https://data-explorer.oecd.org (dataset SEEA_AEA_A v1.2)
**Collection Date**: 2026-02-26
**Status**: ✅ **Downloaded (826 MB)**

**File**: `/data/calibration/raw_data/OECD.SDD.NAD.SEEA,DSD_AEA@DF_AEA,1.2+all.csv`

#### Content

- **2,052,780 data points**
- Sectoral emissions by economic activity (ISIC Rev.4 classification)
- CO2-equivalent emissions (tonnes)
- Coverage: 63 countries, 1990-2021
- Multiple pollutants (CO2, CH4, N2O, F-gases)
- Breakdown by: Country, Year, Sector, Pollutant type

#### Data Structure

- Columns: Country, Year, Economic Activity Code, Pollutant, Unit, Emissions Value
- Economic activities mapped to ISIC sectors (e.g., Manufacturing, Transport, Energy)
- Ready for aggregation to our 15-sector taxonomy and 7-region taxonomy

#### Processing Steps

1. **Extract CO2-equivalent emissions** by ISIC sector and country (latest available year: 2021)
2. **Map ISIC activities to 15 Midas sectors**:
   - ISIC D35 (Electricity, gas, steam) → Energy sectors + Utilities
   - ISIC C10-C33 (Manufacturing) → Manufacturing Heavy/Light, Mining, etc.
   - ISIC H49-H53 (Transportation) → Transportation
   - ISIC A01-A03 (Agriculture, forestry, fishing) → Agriculture
   - And so on for all 15 sectors
3. **Aggregate countries to 7 Midas regions** (weighted by economic output)
4. **Compute emissions intensity** (tCO2e per $ output) for each sector × region cell
5. **Normalize to [0,1] scores** for:
   - **S_{s,carbon}**: Sector carbon scores (15 × 1 vector) - relative emissions across sectors
   - **R_{r,carbon}**: Regional carbon scores (7 × 1 vector) - relative emissions/policy stringency across regions
6. **Populate calibration matrices**:
   - `/data/calibration/sector_scores.csv` - column "CarbonPrice"
   - `/data/calibration/region_scores.csv` - column "CarbonPrice"

#### Interpretation for Model

- **w̃_{s,r,carbon}** = S_{s,carbon} × R_{r,carbon} (relative weight for sector s, region r, carbon driver)
- High emissions intensity → high w̃ → strong factor loading → carbon price shocks propagate to Z_i → higher PD
- Calibration assumes carbon cost pass-through reduces firm equity value (deteriorating A/E ratio in latent factor framework)

---

### 3.2 Physical Risk Sensitivity (BIS Working Paper 1292)

**Purpose**: Calibrate sector sensitivity to physical climate perils (heat, flood, drought)

**Approach**: Map sectoral exposure to physical hazards. Higher exposure → operational disruption → revenue/cost shocks → deteriorating financial ratios → higher default probability.

**Source**: Bank for International Settlements Working Paper No. 1292
**Title**: "Physical climate risks and the banking sector: Evidence from supervisory data"
**Authors**: BIS research team
**Publication Date**: 2024
**Status**: ✅ **Downloaded (1.7 MB)**

**File**: `/data/calibration/raw_data/BISwork1292.pdf`

#### Expected Content

- Sectoral vulnerability assessments to physical climate hazards
- Empirical analysis of physical risk transmission to financial sector
- Quantitative exposure metrics by sector/geography
- Supervisory stress test methodologies
- Sectoral damage functions or vulnerability scores

#### Processing Steps (to be executed)

1. **Extract sectoral vulnerability tables** from BIS paper
   - Identify heat, flood, drought sensitivity by sector
   - Note: May use NACE/ISIC classifications requiring mapping to Midas 15 sectors
2. **Map BIS sectors to 15 Midas sectors**
3. **Extract quantitative scores** (if available) or convert qualitative assessments:
   - HIGH vulnerability → 0.80-1.00
   - MEDIUM vulnerability → 0.40-0.70
   - LOW vulnerability → 0.10-0.30
4. **Populate three physical risk columns** in sector_scores.csv:
   - **S_{s,heat}**: Sector sensitivity to heat index (15 × 1 vector)
   - **S_{s,flood}**: Sector sensitivity to flood risk (15 × 1 vector)
   - **S_{s,drought}**: Sector sensitivity to drought risk (15 × 1 vector)
5. **Regional physical sensitivity** (R_{r,physical}):
   - Derived from World Bank CCKP regional hazard exposure (already in scenarios)
   - Or set to uniform (all regions equally exposed, differentiation through scenario drivers)
   - Populate `/data/calibration/region_scores.csv` - columns "HeatIndex", "FloodRisk", "DroughtRisk"

#### Interpretation for Model

- **w̃_{s,r,heat}** = S_{s,heat} × R_{r,heat} (if regional variation), else just S_{s,heat}
- High physical vulnerability → high w̃ → climate drivers (days >35°C, max precip, dry days) impact factor F_{s,r}
- Physical shocks propagate through latent factor Z_i = β_i^T vec(F) + ε_i
- Example: Agriculture sector (high drought sensitivity) in region experiencing increased dry days → factor deteriorates → Z_i falls → PD_i rises

---

## Data Lineage

```
SCENARIO DATA:
NGFS Phase V → IIASA Explorer → /data/scenarios/*.csv (5 transition drivers)
World Bank CCKP → NetCDF extraction → /data/scenarios/*.csv (3 physical drivers)
Total: 8 drivers × 4 years × 7 regions × 3 scenarios = 672 data points

CORRELATION DATA:
Yahoo Finance → yfinance API → /data/market_data/regional_etf_prices.csv (7 ETFs, 120 months)
Yahoo Finance → yfinance API → /data/market_data/sector_etf_prices.csv (13 ETFs, 120 months)
ETF prices → compute_correlations.py → Corr_S (15×15), Corr_R (7×7)

CALIBRATION DATA:
OECD Air Emissions → 826 MB CSV → sector/region carbon intensity → S_{s,carbon}, R_{r,carbon}
BIS WP 1292 → PDF extraction → sectoral physical vulnerability → S_{s,heat}, S_{s,flood}, S_{s,drought}
Scores → /data/calibration/sector_scores.csv (15 sectors × 8 drivers)
Scores → /data/calibration/region_scores.csv (7 regions × 8 drivers)
```

---

## Update Schedule

| Data Source | Update Frequency | Last Updated | Next Update |
|-------------|------------------|--------------|-------------|
| NGFS Scenarios | ~18 months | Nov 2024 | Mid 2026 |
| World Bank CCKP Physical Risk | ~5 years (IPCC cycle) | 2023 | 2028+ |
| Yahoo Finance ETFs | Daily (we use monthly) | 2026-02-25 | Annual recalibration |
| OECD Air Emissions Accounts | Annual | 2021 data | Check annually |
| BIS Research | Ad hoc | 2024 | Monitor new publications |

---

## References

### Scenario Data
- NGFS (2024). NGFS Climate Scenarios Phase V. https://www.ngfs.net/
- World Bank (2024). Climate Change Knowledge Portal. https://climateknowledgeportal.worldbank.org/
- IPCC (2021). Climate Change 2021: The Physical Science Basis. AR6 WGI.

### Market Data
- Yahoo Finance (2024). Historical Price Data. https://finance.yahoo.com/
- yfinance Python Library. https://pypi.org/project/yfinance/

### Calibration Data
- OECD (2024). Air Emissions Accounts by Economic Activity. SEEA_AEA dataset v1.2. https://data-explorer.oecd.org/
- Bank for International Settlements (2024). Physical climate risks and the banking sector: Evidence from supervisory data. BIS Working Paper No. 1292.

---

**Document Status**: Simplified to reflect selected data sources only
**Last Review**: 2026-02-27
**Legacy Data**: Previous calibration files moved to `/data/calibration/raw_data/legacy/`

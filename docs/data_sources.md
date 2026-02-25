# Data Sources Documentation

**Purpose**: Comprehensive documentation of all external data sources used in the credit portfolio model with climate overlay.

**Last Updated**: 2026-02-25

---

## Overview

The model requires three types of external data:

1. **Transition Risk Data**: Economic and energy drivers from climate scenarios (NGFS Phase V)
2. **Physical Risk Data**: Climate hazard indicators from climate projections (World Bank CCKP)
3. **Correlation Data**: Equity market correlations for residual covariance structure (Yahoo Finance)

---

## 1. Transition Risk Data

### Source: NGFS Phase V Climate Scenarios

**Provider**: Network for Greening the Financial System (NGFS)
**Database**: IIASA NGFS Scenario Explorer
**Version**: Phase V (November 2024)
**URL**: https://www.ngfs.net/en/publications-and-statistics/publications/ngfs-climate-scenarios-central-banks-and-supervisors-phase-v

### Scenarios Used

We use 3 of the 6 NGFS scenarios, mapped to CMIP6 SSP pathways:

| NGFS Scenario | SSP Pathway | Description | Climate Outcome |
|---------------|-------------|-------------|-----------------|
| **Net Zero 2050** | SSP1-2.6 | Immediate policy action, 1.5°C warming | Low physical risk, high transition risk |
| **Delayed Transition** | SSP2-4.5 | Late but strong policy, 2°C warming | Moderate physical risk, disorderly transition |
| **Current Policies** | SSP5-8.5 | No new climate policies, 3°C+ warming | High physical risk, low transition risk |

### Variables Downloaded

**5 Transition Drivers** (annual, 2025-2050, 7 regions):

1. **Carbon Price** (`Price|Carbon`)
   - Units: USD per tonne CO₂
   - Frequency: Annual (2025, 2030, 2040, 2050)
   - Regions: 7 (North America, Europe, Asia-Pacific dev, Asia-Pacific EM, Latin America, MEA, Global)
   - Total observations: 5 drivers × 4 years × 7 regions × 3 scenarios = 420 data points

2. **Coal Price** (`Price|Primary Energy|Coal`)
   - Units: USD per GJ
   - Derived from NGFS energy price projections

3. **Oil Price** (`Price|Primary Energy|Oil`)
   - Units: USD per GJ
   - Brent crude equivalent

4. **Gas Price** (`Price|Primary Energy|Gas`)
   - Units: USD per GJ
   - Natural gas prices

5. **GDP** (`GDP|PPP`)
   - Units: Billion USD (purchasing power parity)
   - Regional GDP projections

### Data Processing

**Location**: `/data/scenarios/` (3 CSV files, one per scenario)

**Processing Script**: `/scripts/process_ngfs_scenarios.py` (not yet created)

**Standardization**:
- All drivers standardized using Current Policies 2050 as baseline
- φ_k(X) = (X - μ_baseline) / σ_baseline
- Standardized values saved for simulation engine

### Data Quality

**Completeness**: 100% (all NGFS scenarios have complete time series)
**Regional Coverage**: Global, with detailed breakdowns for R5 regions
**Temporal Resolution**: 5-year intervals (2025, 2030, 2040, 2050)
**Consistency**: Internally consistent SSP-IAM pathways

### Access Method

1. **Download**: IIASA NGFS Scenario Explorer (web interface or API)
2. **Format**: CSV export from scenario database
3. **License**: Publicly available for research and policy use
4. **Citation**: NGFS (2024), NGFS Climate Scenarios for Central Banks and Supervisors, Phase V

---

## 2. Physical Risk Data

### Source: World Bank Climate Change Knowledge Portal (CCKP)

**Provider**: World Bank Group
**Database**: Climate Change Knowledge Portal
**Climate Model**: CMIP6 ACCESS-CM2 (Australia, CSIRO)
**URL**: https://climateknowledgeportal.worldbank.org/

### Data Downloaded

**3 Physical Risk Indicators** (2030, 2050, 7 regions):

1. **Heat Index** (`hi35`)
   - **Definition**: Number of days per year with heat index > 35°C
   - **Units**: Days per year
   - **Relevance**: Labor productivity, health impacts, energy demand
   - **Files**: 12 NetCDF files (3 scenarios × 4 decades × 1 indicator)
   - **Size**: ~23 MB total

2. **Flood Risk** (`rx1day`)
   - **Definition**: Maximum 1-day precipitation (mm)
   - **Units**: Millimeters
   - **Relevance**: Flood hazard proxy, infrastructure damage
   - **Files**: 12 NetCDF files
   - **Size**: ~23 MB total

3. **Drought Risk** (`cdd`)
   - **Definition**: Consecutive dry days (maximum annual)
   - **Units**: Days
   - **Relevance**: Water stress, agriculture, supply chains
   - **Files**: 12 NetCDF files
   - **Size**: ~23 MB total

**Total Physical Risk Data**: 36 NetCDF files, ~69 MB

### SSP Scenario Mapping

World Bank CCKP uses SSP scenarios. We map to NGFS:

| CCKP SSP | CMIP6 Pathway | Maps to NGFS |
|----------|---------------|--------------|
| SSP1-2.6 | Low emissions | Net Zero 2050 |
| SSP2-4.5 | Moderate emissions | Delayed Transition |
| SSP5-8.5 | High emissions | Current Policies |

### Time Periods

CCKP provides decadal climatology:
- **2030s**: 2030-2039 average → assigned to year 2030
- **2050s**: 2050-2059 average → assigned to year 2050

We use 2030 and 2050 projections (no data for 2025 or 2040).

### Regional Processing

**Original Resolution**: 0.5° × 0.5° global grid (~50 km)

**Regional Aggregation**:
1. Load NetCDF file with global grid
2. Define regional bounding boxes (lat/lon coordinates)
3. Extract grid cells within each region
4. Compute spatial average (area-weighted)
5. Result: 7 regional values per indicator per scenario per year

**Regional Definitions**:
- North America: 15°N-75°N, 170°W-50°W
- Europe: 35°N-70°N, 10°W-40°E
- Asia-Pacific (developed): Japan, Australia, NZ, Korea (specific coordinates)
- Asia-Pacific (emerging): South/Southeast/East Asia
- Latin America: 55°S-30°N, 120°W-30°W
- Middle East & Africa: 35°S-40°N, 20°W-60°E
- Global: All grid cells (global mean)

### Data Processing

**Processing Script**: `/scripts/process_physical_risk.py` (completed)

**Method**:
1. Download 36 NetCDF files from CCKP
2. Extract regional averages for each indicator
3. Compute climatology baseline (2020s) for standardization
4. Standardize: φ_k(X) = (X - μ_2020s) / σ_historical
5. Append to scenario CSV files (transition + physical combined)

**Output**: `/data/scenarios/*.csv` with 8 drivers (5 transition + 3 physical)

### Data Quality

**Completeness**: 100% (all SSP scenarios have global coverage)
**Spatial Resolution**: 0.5° (~50 km), adequate for regional aggregation
**Temporal Resolution**: Decadal climatology (reduces interannual noise)
**Model Uncertainty**: Single model (ACCESS-CM2); future versions should use multi-model ensemble

### Access Method

1. **Download**: World Bank CCKP web interface or API
2. **Format**: NetCDF (Network Common Data Form)
3. **License**: Open access for research and policy
4. **Citation**: World Bank (2024), Climate Change Knowledge Portal, CMIP6 ACCESS-CM2 projections

---

## 3. Correlation Data (Market Indices)

### Source: Yahoo Finance (via yfinance Python library)

**Provider**: Yahoo Finance
**Library**: yfinance 1.2.0 (Python)
**License**: Free for non-commercial use
**URL**: https://finance.yahoo.com

### Data Downloaded

**Purpose**: Construct empirical correlation matrices for residual covariance Σ_u

**Download Date**: 2026-02-25
**Time Period**: 2015-01-01 to 2024-12-31 (10 years)
**Frequency**: Monthly (end-of-month close prices)
**Total Observations**: 120 months

### Regional ETF Data (7 regions)

Used to construct **Corr_R (7×7 regional correlation matrix)**:

| Region | ETF Ticker | ETF Name | Provider | Status |
|--------|-----------|----------|----------|--------|
| North America | SPY | S&P 500 ETF | State Street | ✓ 120 obs |
| Europe | VGK | FTSE Europe ETF | Vanguard | ✓ 120 obs |
| Asia-Pacific (developed) | VPL | FTSE Pacific ETF | Vanguard | ✓ 120 obs |
| Asia-Pacific (emerging) | VWO | Emerging Markets ETF | Vanguard | ✓ 120 obs |
| Latin America | ILF | Latin America 40 ETF | iShares | ✓ 120 obs |
| Middle East & Africa | AFK | Africa ETF | VanEck | ✓ 120 obs |
| Global | VT | Total World Stock ETF | Vanguard | ✓ 120 obs |

**Data Quality**: No missing data, all 7 regions have complete 120-month history.

### Sector ETF Data (15 sectors)

Used to construct **Corr_S (15×15 sector correlation matrix)**:

| Sector | ETF Ticker | ETF Name | Provider | Status |
|--------|-----------|----------|----------|--------|
| **Energy - Oil & Gas** | XLE | Energy Select Sector | State Street | ✓ 120 obs |
| **Energy - Coal** | - | No ETF available | - | Proxy with XLE |
| **Energy - Renewables** | ICLN | Clean Energy ETF | iShares | ✓ 120 obs |
| **Utilities** | XLU | Utilities Select Sector | State Street | ✓ 120 obs |
| **Financials** | XLF | Financial Select Sector | State Street | ✓ 120 obs |
| **Consumer Goods** | XLP | Consumer Staples Select | State Street | ✓ 120 obs |
| **Healthcare** | XLV | Health Care Select Sector | State Street | ✓ 120 obs |
| **Mining & Metals** | XLB | Materials Select Sector | State Street | ✓ 120 obs |
| **Real Estate** | XLRE | Real Estate Select Sector | State Street | ⚠️ 111 obs (9 missing) |
| **Technology & Services** | XLK | Technology Select Sector | State Street | ✓ 120 obs |
| **Manufacturing - Heavy** | XLI | Industrials Select Sector | State Street | ✓ 120 obs |
| **Manufacturing - Light** | XLY | Consumer Discretionary Select | State Street | ✓ 120 obs |
| **Transportation** | IYT | Transportation ETF | iShares | ✓ 120 obs |
| **Agriculture** | MOO | Agribusiness ETF | VanEck | ✓ 120 obs |
| **Other** | - | Derived (avg of 14 others) | - | To be computed |

**Data Quality**:
- 13 of 15 sectors have complete data
- Real Estate (XLRE): 9 missing observations (7.5%) - ETF launched Oct 2015
- Coal: No public ETF (ESG divestment) - will proxy with Oil & Gas at 0.85 correlation
- "Other": Will compute as equal-weighted average of other 14 sectors

### Data Processing

**Download Script**: `/scripts/download_market_data.py` (completed)

**Method**:
1. Download adjusted close prices for each ETF (monthly)
2. Extract "Close" price (which includes dividend adjustment in yfinance 1.0+)
3. Save to CSV: `/data/market_data/regional_etf_prices.csv`, `/data/market_data/sector_etf_prices.csv`
4. Document metadata: `/data/market_data/download_metadata.txt`

**Future Processing** (Phase 3.4):
1. Handle Real Estate missing data (forward fill from Oct 2015)
2. Compute log returns: r_t = log(P_t / P_{t-1})
3. Compute correlation matrices: Corr_R = corr(r_regional), Corr_S = corr(r_sector)
4. Validate positive definiteness
5. Construct Σ_u using ω and η parameters

### Why ETFs Instead of MSCI Indices?

**Budget constraints**: Bloomberg/Refinitiv MSCI data costs $1,500-2,000/month
**ETF advantages**:
- Free and publicly accessible
- Highly liquid and representative
- Long history (10+ years for most)
- Adjusted for dividends

**ETF limitations**:
- Slightly different composition than MSCI (but highly correlated)
- Coal sector not available (no public coal ETF)
- Some ETFs launched after 2015 (e.g., XLRE in Oct 2015)

**Mitigation**: Use ETFs as best available free data source; document limitations; perform sensitivity analysis

### Access Method

1. **Library**: `pip install yfinance`
2. **API**: `yf.download(ticker, start, end, interval='1mo')`
3. **Rate Limits**: None for reasonable use
4. **License**: Free for personal and research use (check terms for commercial)
5. **Data Lag**: ~15 minutes delayed (sufficient for monthly data)

---

## Data Governance

### Version Control

All raw data and processing scripts are version-controlled in Git:
- `/data/scenarios/` - NGFS scenario data (CSV)
- `/data/physical_risk/` - World Bank CCKP data (NetCDF, not in Git due to size)
- `/data/market_data/` - Yahoo Finance ETF data (CSV)
- `/scripts/` - Data processing scripts (Python)

### Data Lineage

```
NGFS Phase V → process_ngfs_scenarios.py → /data/scenarios/net_zero_2050.csv
                                         → /data/scenarios/delayed_transition.csv
                                         → /data/scenarios/current_policies.csv

World Bank CCKP → process_physical_risk.py → (appended to scenario CSVs)

Yahoo Finance → download_market_data.py → /data/market_data/regional_etf_prices.csv
                                       → /data/market_data/sector_etf_prices.csv
```

### Update Frequency

| Data Source | Update Frequency | Last Updated | Next Update Due |
|-------------|------------------|--------------|-----------------|
| NGFS Scenarios | ~18 months | Nov 2024 (Phase V) | Mid 2026 (Phase VI) |
| CCKP Physical Risk | ~5 years (IPCC cycle) | 2023 (CMIP6) | 2028+ (CMIP7) |
| Yahoo Finance ETF Data | Daily (we use monthly) | 2026-02-25 | As needed for recalibration |

### Data Review Process

1. **Annual Review**: Check for NGFS updates, IPCC reports, major climate events
2. **Model Validation Cycle**: Recalibrate correlations annually with updated market data
3. **Scenario Updates**: When NGFS releases new phase, download and reprocess
4. **Physical Risk Updates**: When CMIP7 or new IPCC assessment released

### Data Quality Checks

**Transition Data (NGFS)**:
- [ ] All scenarios have complete time series (2025-2050)
- [ ] Regional values sum approximately to global (GDP, emissions)
- [ ] Carbon prices non-negative and increasing in Net Zero scenario
- [ ] Fossil fuel prices consistent with energy transition narrative

**Physical Data (CCKP)**:
- [ ] NetCDF files load without errors
- [ ] Regional averages within plausible ranges (e.g., heat index < 200 days)
- [ ] Time progression is monotonic for chronic hazards (heat, drought)
- [ ] Spatial coverage is global (no missing regions)

**Correlation Data (Yahoo Finance)**:
- [ ] No gaps in monthly time series
- [ ] Price series are non-negative
- [ ] Returns are plausible (< 50% monthly change, typical for ETFs)
- [ ] Correlations are in valid range [-1, 1]
- [ ] Correlation matrices are positive definite

---

## Data Limitations and Future Improvements

### Current Limitations

1. **Physical Risk - Single Climate Model**:
   - Using ACCESS-CM2 only (one of 30+ CMIP6 models)
   - Model uncertainty not quantified
   - **Future**: Use multi-model ensemble (e.g., mean of 5-10 CMIP6 models)

2. **Transition Risk - Regional Granularity**:
   - NGFS uses R5 regions (5 world regions)
   - We map to 7 regions with interpolation
   - **Future**: Use country-level NGFS data where available

3. **Correlation Data - ETF Proxies**:
   - Using free ETFs instead of MSCI institutional indices
   - Coal sector has no ETF (using proxy)
   - Real Estate has incomplete history
   - **Future**: If budget allows, upgrade to Bloomberg/MSCI data

4. **Temporal Resolution**:
   - Transition data: 5-year intervals (2025, 2030, 2040, 2050)
   - Physical data: Decadal climatology (2030s, 2050s)
   - Linear interpolation between years
   - **Future**: Annual resolution if available

5. **Static Correlations**:
   - Using 10-year historical correlation (2015-2024)
   - Assumes correlations are time-invariant
   - **Future**: Time-varying or scenario-dependent correlations

### Data Enhancement Roadmap

**Phase 1 (Current)**: MVP with best available free data
- ✓ NGFS Phase V scenarios
- ✓ World Bank CCKP single-model projections
- ✓ Yahoo Finance ETF correlations

**Phase 2 (6-12 months)**: Improved granularity
- [ ] NGFS country-level data (if available)
- [ ] Multi-model CCKP ensemble (5 CMIP6 models)
- [ ] Bloomberg/MSCI institutional data (if budget approved)

**Phase 3 (12-24 months)**: Advanced features
- [ ] High-resolution physical risk (0.25° grid, city-level)
- [ ] Credit spread correlations (iTraxx/CDX for validation)
- [ ] Time-varying correlations (regime-switching model)
- [ ] Non-linear climate response functions

---

## References

### NGFS Climate Scenarios
- NGFS (2024). "NGFS Climate Scenarios for Central Banks and Supervisors, Phase V." Network for Greening the Financial System. https://www.ngfs.net/en/publications

### World Bank CCKP
- World Bank (2024). "Climate Change Knowledge Portal." World Bank Group. https://climateknowledgeportal.worldbank.org/

### CMIP6 Climate Models
- Eyring, V., et al. (2016). "Overview of the Coupled Model Intercomparison Project Phase 6 (CMIP6)." Geoscientific Model Development, 9(5), 1937-1958.

### IPCC Assessments
- IPCC (2021). "Climate Change 2021: The Physical Science Basis." Sixth Assessment Report. https://www.ipcc.ch/report/ar6/wg1/

### Yahoo Finance Data
- Yahoo Finance (2024). Historical Price Data. https://finance.yahoo.com/
- yfinance Python Library: https://pypi.org/project/yfinance/

---

## Contact and Support

**Data Questions**: Refer to individual provider websites
**Model Questions**: See `/docs/credit_portfolio_model_climate_overlay.md`
**Processing Scripts**: See `/scripts/README.md` (to be created)

**Last Review**: 2026-02-25
**Next Review Due**: 2027-02-25 (annual cycle)

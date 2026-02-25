# Data Sources - Comprehensive Reference

**Purpose**: Complete documentation of all external data sources for the credit portfolio model with climate overlay

**Last Updated**: 2026-02-25

---

## Quick Reference

| Data Type | Source | Status | Location |
|-----------|--------|--------|----------|
| **Transition Risk Scenarios** | NGFS Phase V | ✅ Downloaded & Processed | `/data/scenarios/*.csv` (3 files) |
| **Physical Risk Projections** | World Bank CCKP | ✅ Downloaded & Processed | `/data/scenarios/*.csv` (integrated) |
| **Regional Correlations** | Yahoo Finance (7 ETFs) | ✅ Downloaded | `/data/market_data/regional_etf_prices.csv` |
| **Sector Correlations** | Yahoo Finance (13 ETFs) | ✅ Downloaded | `/data/market_data/sector_etf_prices.csv` |
| **Sector Carbon Scores** | CDP + OECD + IEA | ⏳ To be collected | `/data/calibration/sector_scores.csv` |
| **Regional Carbon Scores** | World Bank + OECD | ⏳ To be collected | `/data/calibration/region_scores.csv` |
| **Physical Vulnerability** | IPCC AR6 + Academic | ⏳ To be collected | `/data/calibration/sector_scores.csv` |

---

## Part 1: Climate Scenario Data

### 1.1 Transition Risk Data (NGFS Phase V)

**Provider**: Network for Greening the Financial System (NGFS)
**URL**: https://www.ngfs.net/en/publications-and-statistics/publications/ngfs-climate-scenarios-central-banks-and-supervisors-phase-v
**Database**: IIASA NGFS Scenario Explorer
**Version**: Phase V (November 2024)

#### Scenarios

| NGFS Scenario | SSP Pathway | Description |
|---------------|-------------|-------------|
| Net Zero 2050 | SSP1-2.6 | Immediate policy action, 1.5°C warming |
| Delayed Transition | SSP2-4.5 | Late but strong policy, 2°C warming |
| Current Policies | SSP5-8.5 | No new climate policies, 3°C+ warming |

#### Variables to Download

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

#### How to Download

1. Go to: https://data.ece.iiasa.ac.at/ngfs/
2. Select model: MESSAGE-GLOBIOM 1.1
3. Select scenarios: Net Zero 2050, Delayed Transition, Current Policies
4. Select variables: Price|Carbon, Price|Primary Energy|*, GDP|PPP
5. Select regions: All R5 regions
6. Download as CSV
7. Save to `/data/scenarios/ngfs_raw_download.csv`

**Status**: ✅ **Downloaded & Processed** (February 2026)

**Files Created**:
- `/data/scenarios/net_zero_2050.csv` (183 rows)
- `/data/scenarios/delayed_transition.csv` (183 rows)
- `/data/scenarios/current_policies.csv` (183 rows)
- Drivers standardized: φ_k(X) = (X - μ) / σ
- Ready for model input

---

### 1.2 Physical Risk Data (World Bank CCKP)

**Provider**: World Bank Climate Change Knowledge Portal
**URL**: https://climateknowledgeportal.worldbank.org/
**Climate Model**: CMIP6 ACCESS-CM2
**Resolution**: 0.5° grid (~50 km)

#### Indicators to Download

**3 Physical Risk Indicators** (2030s and 2050s climatology):

1. **Heat Index** - `hi35` (Days per year with heat index >35°C)
2. **Flood Risk** - `rx1day` (Maximum 1-day precipitation in mm)
3. **Drought Risk** - `cdd` (Consecutive dry days, maximum annual)

**SSP Scenarios**: SSP1-2.6, SSP2-4.5, SSP5-8.5 (maps to our 3 NGFS scenarios)

**Total**: 3 indicators × 2 time periods × 3 scenarios = **18 global NetCDF files** × 2 decadal periods = 36 files (~69 MB)

#### How to Download

1. Go to: https://climateknowledgeportal.worldbank.org/download-data
2. Select: Climate Projections → CMIP6 → ACCESS-CM2
3. For each indicator (hi35, rx1day, cdd):
   - Select SSP scenarios: 126, 245, 585
   - Select time periods: 2030-2039, 2050-2059
   - Download as NetCDF
4. Save to `/data/physical_risk/raw/`

**Status**: ✅ **Downloaded & Processed** (February 2026)

**Integration**: Physical risk data integrated into scenario CSVs above
- `/data/scenarios/*.csv` includes all 8 drivers (5 transition + 3 physical)
- Regional averages extracted (7 regions)
- Climatology computed (2030s, 2050s)
- Standardized and ready for model input
- Processing script: `/data/scenarios/process_physical_risk.py`

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

## Part 3: Calibration Data (Sensitivity Scores)

### 3.1 Sector Carbon Dependencies (S_{s,carbon})

**Purpose**: Calibrate how sensitive each sector is to carbon pricing

**Question**: "If carbon price increases by $50/tCO2, which sectors face the most credit stress?"

#### Data Sources

**Primary Source: CDP (Carbon Disclosure Project)**

- **URL**: https://www.cdp.net/en
- **What**: Company-level Scope 1+2 emissions disclosures, ~18,000 companies
- **Metric**: Emissions intensity (tCO2e per $M revenue)
- **Access**: Free registration required
- **Status**: ⏳ **To be downloaded**

**How to use**:
1. Register at CDP website (free)
2. Download company emissions data (Scope 1 + Scope 2)
3. Map companies to our 15 sectors
4. Compute sector median emissions intensity
5. Normalize to [0,1] scale

**Secondary Source: OECD Air Emissions Database**

- **URL**: https://stats.oecd.org/ → Environment → Air and Climate
- **What**: Sectoral GHG emissions by ISIC sector code
- **Access**: Free download
- **Status**: ⏳ **To be downloaded**

**How to use**:
1. Download "Air emissions accounts by industry"
2. Map ISIC codes to our 15 sectors
3. Compute emissions per $ output
4. Use to fill gaps in CDP data

**Validation Source: Academic Carbon Betas**

- **Papers**:
  - Görgen et al. (2020) "Carbon Risk" - sector carbon betas
  - Bolton & Kacperczyk (2021) "Do Investors Care About Carbon Risk?"
  - Ilhan et al. (2021) "Carbon Tail Risk"
- **Access**: Published papers (free via Google Scholar)
- **Status**: ⏳ **To be extracted**

**Expected Output**: S_{s,carbon} scores in `/data/calibration/sector_scores.csv`

```
Energy - Coal: 1.00 (maximum by definition)
Energy - Oil & Gas: 0.90
Heavy Manufacturing: 0.80
Utilities: 0.70
Mining & Metals: 0.70
Transportation: 0.80
...
```

---

### 3.2 Regional Carbon Dependencies (R_{r,carbon})

**Purpose**: Calibrate which regions face most exposure to carbon pricing policies

**Question**: "Which regions have the strongest carbon pricing policies today and in NGFS scenarios?"

#### Data Sources

**Primary Source: World Bank Carbon Pricing Dashboard**

- **URL**: https://carbonpricingdashboard.worldbank.org/
- **What**: Carbon pricing coverage (% emissions), effective carbon rates by country
- **Access**: Free (web scraping or manual download)
- **Status**: ⏳ **To be collected**

**How to use**:
1. Visit dashboard, extract coverage by country
2. Aggregate to our 7 regions (weighted by emissions)
3. Compute: R = (Coverage × Effective Rate) / max(Coverage × Rate)

**Secondary Source: OECD Effective Carbon Rates**

- **URL**: https://www.oecd.org/tax/tax-policy/effective-carbon-rates-2021.htm
- **What**: Explicit + implicit carbon pricing (incl. fuel taxes)
- **Access**: Free report download
- **Status**: ⏳ **To be downloaded**

**Validation Source: Climate Action Tracker**

- **URL**: https://climateactiontracker.org/
- **What**: Policy stringency ratings by country
- **Access**: Free
- **Status**: ⏳ **To be reviewed**

**Cross-Check: Our Own NGFS Data**

- Use NGFS 2030 carbon price projections (already have this data once downloaded)
- Ensure R_{r,carbon} ranking matches NGFS carbon price ranking

**Expected Output**: R_{r,carbon} scores in `/data/calibration/region_scores.csv`

```
Europe: 0.90 (EU ETS mature, high coverage)
North America: 0.70 (growing coverage)
Asia-Pacific (developed): 0.70
Asia-Pacific (emerging): 0.60 (China ETS, low price)
Latin America: 0.50
Middle East & Africa: 0.40
Global: 0.70
```

---

### 3.3 Sector Physical Risk Vulnerabilities

**Purpose**: Calibrate sector sensitivity to heat, flood, drought

**Question**: "Which sectors are most operationally vulnerable to physical climate hazards?"

#### Data Sources

**Primary Source: IPCC AR6 Working Group II**

- **URL**: https://www.ipcc.ch/report/ar6/wg2/
- **Chapters**:
  - Chapter 4: Water (drought)
  - Chapter 5: Food and agriculture
  - Chapter 7: Health (heat stress)
  - Chapter 8: Poverty and livelihoods (sectoral impacts)
- **What**: Qualitative vulnerability assessments (HIGH/MEDIUM/LOW confidence)
- **Access**: Free download (full report PDF)
- **Status**: ⏳ **To be reviewed**

**How to use**:
1. Download WGII report chapters 4, 5, 7, 8
2. Extract sectoral vulnerability tables
3. Map IPCC sectors to our 15 sectors
4. Convert qualitative (HIGH/MED/LOW) to quantitative (0.8-1.0 / 0.4-0.7 / 0.1-0.3)

**Secondary Source: Academic Literature**

**Heat Vulnerability**:
- Burke et al. (2015) "Global Non-Linear Effect of Temperature on Economic Production" - labor productivity by sector
- **Proxy**: Outdoor labor intensity (BLS/ILO data)

**Flood Vulnerability**:
- Hsiang et al. (2017) "Estimating Economic Damage from Climate Change" - sectoral damage functions
- **Proxy**: Fixed asset coastal exposure (inferred)

**Drought Vulnerability**:
- Kahn et al. (2021) "Long-Term Macroeconomic Effects of Climate Change" - water stress impacts
- **Proxy**: Water intensity (USGS water use data)

**Access**: Google Scholar (free)
**Status**: ⏳ **To be reviewed and extracted**

**Expected Output**: S_{s,physical} scores in `/data/calibration/sector_scores.csv`

**Heat (HeatIndex)**:
```
Agriculture: 0.90 (outdoor labor, crop stress)
Real Estate & Construction: 0.75 (outdoor work)
Transportation: 0.70
Utilities: 0.60
Manufacturing - Heavy: 0.50
Technology & Services: 0.20 (indoor, AC)
```

**Flood (FloodRisk)**:
```
Real Estate & Construction: 0.90 (fixed assets)
Transportation: 0.75 (infrastructure)
Agriculture: 0.70
Utilities: 0.60
Manufacturing - Heavy: 0.50
Technology & Services: 0.20
```

**Drought (DroughtRisk)**:
```
Agriculture: 0.95 (crop failure)
Utilities: 0.70 (hydropower, cooling water)
Manufacturing - Heavy: 0.45 (water-intensive)
Mining & Metals: 0.50
Technology & Services: 0.20
```

---

## Data Collection Priority and Status

### High Priority (Core Model Inputs)

| Data | Source | Status | Blocking? |
|------|--------|--------|-----------|
| ✅ Regional ETF prices | Yahoo Finance | Downloaded | No |
| ✅ Sector ETF prices | Yahoo Finance | Downloaded | No |
| ✅ NGFS transition scenarios | NGFS/IIASA | Downloaded & Processed | No |
| ✅ CCKP physical risk | World Bank | Downloaded & Processed | No |

### Medium Priority (Calibration Parameters)

| Data | Source | Status | Blocking? |
|------|--------|--------|-----------|
| ⏳ Sector carbon intensity | CDP + OECD | To collect | No (can use expert judgment) |
| ⏳ Regional carbon pricing | World Bank + OECD | To collect | No (can use NGFS data) |
| ⏳ Physical vulnerabilities | IPCC AR6 + Academic | To review | No (can use expert judgment) |

### Low Priority (Validation)

| Data | Source | Status | Blocking? |
|------|--------|--------|-----------|
| ⏳ Academic carbon betas | Papers | To extract | No |
| ⏳ Credit spread correlations | iTraxx/CDX | Optional | No |
| ⏳ Munich Re loss data | Proprietary | Optional | No |

---

## Next Steps

### Phase 1: Correlation Matrix Construction (READY)

**Step 1.1: Process ETF Data**
```bash
python scripts/compute_correlations.py
# Input: /data/market_data/regional_etf_prices.csv (7 regions)
#        /data/market_data/sector_etf_prices.csv (13 sectors)
# Output: /data/calibration/correlation_matrix_R.csv (7×7)
#         /data/calibration/correlation_matrix_S.csv (15×15)
```

**Step 1.2: Construct Σ_u**
- Choose ω = 0.5 (sector-vs-region mix)
- Choose η = 0.7 (structured-vs-cell share)
- Compute Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I_M

**Status**: ✅ Ready to proceed (all data downloaded)

---

### Phase 2: Calibration Data Collection (NON-BLOCKING)

**Step 2.1: Sector Carbon Scores**
- CDP registration + download
- OECD emissions data download
- Academic paper review
- Populate `/data/calibration/sector_scores.csv` (CarbonPrice column)

**Step 2.2: Regional Carbon Scores**
- World Bank dashboard data extraction
- OECD effective carbon rates report
- Climate Action Tracker review
- Populate `/data/calibration/region_scores.csv` (CarbonPrice column)

**Step 2.3: Physical Vulnerability Scores**
- IPCC AR6 WGII chapters 4, 5, 7, 8 review
- Academic paper review (Burke, Hsiang, Kahn)
- Populate `/data/calibration/sector_scores.csv` (HeatIndex, FloodRisk, DroughtRisk columns)

**Status**: ⏳ Can proceed with expert judgment if time-constrained

---

## Data Governance

### Update Schedule

| Data Source | Update Frequency | Last Updated | Next Update |
|-------------|------------------|--------------|-------------|
| NGFS Scenarios | ~18 months | Nov 2024 | Mid 2026 |
| CCKP Physical Risk | ~5 years (IPCC cycle) | 2023 | 2028+ |
| Yahoo Finance ETFs | Daily (we use monthly) | 2026-02-25 | Annual recalibration |
| CDP Emissions | Annual | TBD | Annual |
| World Bank Carbon Pricing | Annual | TBD | Annual |
| IPCC Assessments | 5-7 years | 2021-2022 | 2028+ |

### Data Lineage

```
SCENARIO DATA:
NGFS Phase V → process_ngfs_scenarios.py → /data/scenarios/*.csv (420 points)
World Bank CCKP → process_physical_risk.py → /data/scenarios/*.csv (126 points)

CORRELATION DATA:
Yahoo Finance → download_market_data.py → /data/market_data/*.csv (120 months)
ETF prices → compute_correlations.py → Corr_S (15×15), Corr_R (7×7)

CALIBRATION DATA:
CDP + OECD → sector_scores.csv (S matrix: 15×8)
World Bank + OECD → region_scores.csv (R matrix: 7×8)
IPCC + Academic → sector_scores.csv (physical vulnerability scores)
```

---

## References

### Scenario Data
- NGFS (2024). NGFS Climate Scenarios Phase V. https://www.ngfs.net/
- World Bank (2024). Climate Change Knowledge Portal. https://climateknowledgeportal.worldbank.org/
- IPCC (2021). Climate Change 2021: The Physical Science Basis. AR6 WGI.

### Market Data
- Yahoo Finance (2024). Historical Price Data. https://finance.yahoo.com/
- yfinance Python Library. https://pypi.org/project/yfinance/

### Calibration Data - Carbon
- CDP (2024). Corporate Climate Disclosures. https://www.cdp.net/
- OECD (2023). Air Emissions Accounts. https://stats.oecd.org/
- World Bank (2024). Carbon Pricing Dashboard. https://carbonpricingdashboard.worldbank.org/
- OECD (2021). Effective Carbon Rates 2021. https://www.oecd.org/tax/
- Bolton, P., & Kacperczyk, M. (2021). Do investors care about carbon risk? JFE, 142(2), 517-549.
- Görgen, M., et al. (2020). Carbon risk. SSRN Working Paper.

### Calibration Data - Physical Risk
- IPCC (2022). Climate Change 2022: Impacts, Adaptation and Vulnerability. AR6 WGII.
- Burke, M., Hsiang, S. M., & Miguel, E. (2015). Global non-linear effect of temperature. Nature, 527, 235-239.
- Hsiang, S., et al. (2017). Estimating economic damage from climate change in the US. Science, 356, 1362-1369.
- Kahn, M. E., et al. (2021). Long-term macroeconomic effects of climate change. RES, 88(4), 1802-1829.

---

**Document Status**: Living document, updated as data is collected
**Last Review**: 2026-02-25
**Next Review**: After Phase 1 downloads complete

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
| **Sector Carbon Scores** | IEA + OECD + UNECE | 🔄 Partially Downloaded | `/data/calibration/raw_data/` (5 files, 3 pending) |
| **Regional Carbon Scores** | World Bank Dashboard 2024 | ✅ Downloaded | `/data/calibration/raw_data/WorldBank_State_Trends_Carbon_Pricing_2024.pdf` |
| **Physical Vulnerability** | IPCC AR6 WGII | ⏳ Manual Download Required | Section 3.4 (download instructions) |

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
- **Access**: Free download (requires interactive data selection)
- **Status**: ⏳ **Manual download required** (instructions in Section 3.4)
- **Documentation**: ✅ Downloaded - `OECD_GHG_Data_Documentation_2024.pdf`

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

**Research Collected (2026-02-25)**:

**OECD Air Emissions Accounts**:
- Latest sectoral data: 2021 (not 2023)
- Database: stats.oecd.org/Index.aspx?DataSetCode=OECD-AEA
- Coverage: 63 countries, 32 years (1990-2021)
- Provides: GHG emissions by economic activity
- Status: Available but requires manual download/extraction

**IEA CO2 Emissions in 2022 Report**:
- ✅ Downloaded: `IEA_CO2_Emissions_2022.pdf` (665 KB) + `IEA_GHG_Documentation_2022.pdf` (1.7 MB)
- Key Finding: Cement, iron/steel, chemicals account for 27%, 25%, 14% of industrial CO2
- Emissions intensity trends (2019-2023):
  - Iron/steel/aluminum: 31% decrease
  - Chemicals: 8% increase
  - Cement/lime: 11% increase
- **Raw Data**: ⏳ Requires IEA account (instructions in Section 3.4)

**UNECE COP27 Report - Emissions Intensity by Revenue**:
- ✅ Downloaded: `UNECE_COP27_Industry_Brief.pdf` (7 KB)
- Cement: ~7 kg CO2 per $ revenue
- Steel: ~1.5 kg CO2 per $ revenue
- Chemicals: ~0.3 kg CO2 per $ revenue

**Recommended Scores** (based on emissions intensity and stranded asset risk):
```
Energy - Coal: 1.00 (maximum, highest stranded asset risk)
Energy - Oil & Gas: 0.90 (high emissions, direct carbon exposure)
Transportation: 0.80 (high oil dependency, fleet transition costs)
Manufacturing - Heavy: 0.80 (steel, cement, chemicals - energy intensive)
Utilities: 0.70 (mixed portfolio, coal/gas exposure)
Mining & Metals: 0.70 (extraction energy intensity)
Manufacturing - Light: 0.40 (lower energy intensity)
Consumer Goods: 0.40 (moderate)
Real Estate & Construction: 0.50 (materials + operational)
Agriculture: 0.30 (fertilizer/machinery, lower intensity)
Healthcare: 0.30 (essential services, moderate)
Energy - Renewables: 0.20 (benefits from carbon price)
Financial Services: 0.20 (indirect exposure)
Technology & Services: 0.20 (low direct emissions)
Other: 0.50 (average)
```

---

### 3.2 Regional Carbon Dependencies (R_{r,carbon})

**Purpose**: Calibrate which regions face most exposure to carbon pricing policies

**Question**: "Which regions have the strongest carbon pricing policies today and in NGFS scenarios?"

#### Data Sources

**Primary Source: World Bank Carbon Pricing Dashboard**

- **URL**: https://carbonpricingdashboard.worldbank.org/
- **What**: Carbon pricing coverage (% emissions), effective carbon rates by country
- **Access**: Free report download
- **Status**: ✅ **Downloaded** - `WorldBank_State_Trends_Carbon_Pricing_2024.pdf` (6.9 MB)

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

**Research Collected (2026-02-25)**:

**World Bank Carbon Pricing Dashboard 2024**:
- URL: carbonpricingdashboard.worldbank.org
- Global Coverage: 73 initiatives in 39 jurisdictions covering 23-28% of global GHG emissions
- Global Average Price: $32/tCO2
- Revenue Mobilized: >$100 billion in 2024

**Regional Carbon Pricing (World Bank 2024)**:

1. **Europe & Central Asia**:
   - Highest number of pricing initiatives
   - Average price: $50/tCO2
   - Coverage: Well-established, expanding
   - Assessment: R = 0.90 (highest)

2. **North America** (US + Canada):
   - 16 initiatives
   - Average price: $48/tCO2
   - Coverage: Growing (state/provincial level)
   - Political feasibility challenges
   - Assessment: R = 0.70

3. **Asia-Pacific**:
   - Developed (Japan, Korea, Australia): Strong pricing systems
   - Emerging (China ETS): Large but low price (~$10-15)
   - Assessment: R_dev = 0.70, R_em = 0.60

4. **Other Regions**:
   - Latin America: Limited (Mexico, Colombia, Chile)
   - Middle East & Africa: Minimal (South Africa only, oil producers resistant)
   - Assessment: R_latam = 0.50, R_mea = 0.40

**Cross-Validation**: Rankings consistent with NGFS 2030 carbon price projections

**Recommended Scores**:
```
Europe: 0.90 (EU ETS mature, $50/tCO2 average, high coverage)
North America: 0.70 ($48/tCO2, state/provincial initiatives)
Asia-Pacific (developed): 0.70 (Japan, Korea, Australia pricing)
Asia-Pacific (emerging): 0.60 (China ETS, large but low price)
Latin America: 0.50 (Mexico, Colombia, Chile)
Middle East & Africa: 0.40 (South Africa only, oil producers)
Global: 0.70 (weighted average)
```

---

### 3.3 Sector Physical Risk Vulnerabilities

**Purpose**: Calibrate sector sensitivity to heat, flood, drought

**Question**: "Which sectors are most operationally vulnerable to physical climate hazards?"

#### Data Sources

**Primary Source: IPCC AR6 Working Group II**

- **URL**: https://www.ipcc.ch/report/ar6/wg2/
- **Database**: AR6 WGII Observed and Projected Impact Assessment Database
- **Database URL**: https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/
- **Chapters** (qualitative assessments):
  - Chapter 4: Water (drought)
  - Chapter 5: Food and agriculture
  - Chapter 7: Health (heat stress)
  - Chapter 8: Poverty and livelihoods (sectoral impacts)
- **What**: Quantitative impact data + qualitative vulnerability assessments
- **Access**: Free download (Excel workbook 442 KB + PDF chapters)
- **Status**: ⏳ **Manual download required** (server timeout on 2026-02-25, instructions in Section 3.4)

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

**Research Collected (2026-02-25)**:

**IPCC AR6 Working Group II (2022)** - Key Findings:

**Agriculture**:
- Droughts cause yield reduction in ~75% of harvested areas (454M hectares)
- Production losses: US$66bn (1983-2009)
- Heat-induced labor productivity losses documented
- Assessment: VERY HIGH vulnerability to all physical hazards

**Infrastructure/Construction/Real Estate**:
- 44% of all disasters since 1970s are flood-related
- Projected flood damages higher by 1.4-2x at 2°C, 2.5-3.9x at 3°C vs 1.5°C
- Legacy infrastructure not designed for climate change = stranded assets
- Assessment: HIGH vulnerability to floods and heat

**Transportation**:
- Infrastructure compromised by extreme events (service disruptions, economic losses)
- Worker exposure to heat stress
- Assessment: HIGH vulnerability to heat and floods

**Utilities**:
- Hydropower: 4-5% reduction in plant utilization during droughts since 1980s
- Thermoelectric cooling water constraints
- Assessment: MEDIUM-HIGH vulnerability to drought and heat

**Services/Technology**:
- Indoor, climate-controlled operations
- Minimal direct physical exposure
- Assessment: LOW vulnerability to all hazards

**Recommended Scores** (based on IPCC assessments and sector characteristics):

**Heat (HeatIndex)**:
```
Agriculture: 0.90 (outdoor labor, crop stress)
Real Estate & Construction: 0.75 (outdoor work, material stress)
Transportation: 0.70 (infrastructure stress, worker exposure)
Utilities: 0.60 (cooling demand, thermal efficiency)
Manufacturing - Heavy: 0.50 (industrial cooling, worker productivity)
Mining & Metals: 0.55 (outdoor extraction)
Manufacturing - Light: 0.40 (some climate-controlled)
Energy - Renewables: 0.35 (solar efficiency loss)
Healthcare: 0.35 (patient care, AC critical)
Energy - Oil & Gas: 0.30 (some outdoor operations)
Consumer Goods: 0.30 (mixed)
Energy - Coal: 0.30
Financial Services: 0.20 (indoor, white-collar)
Technology & Services: 0.20 (indoor, flexible location)
Other: 0.40
```

**Flood (FloodRisk)**:
```
Real Estate & Construction: 0.90 (fixed assets in floodplains)
Transportation: 0.75 (infrastructure disruption)
Agriculture: 0.70 (crop damage, soil erosion)
Utilities: 0.60 (power plants near water)
Manufacturing - Heavy: 0.50 (factory flooding)
Mining & Metals: 0.50 (mine flooding, tailings risk)
Energy - Oil & Gas: 0.45 (coastal refineries)
Manufacturing - Light: 0.40 (warehouses)
Energy - Renewables: 0.40 (distributed assets)
Consumer Goods: 0.30
Healthcare: 0.30 (hospitals need protection)
Energy - Coal: 0.30 (mine flooding)
Financial Services: 0.20 (offices, can relocate)
Technology & Services: 0.20 (data centers protected)
Other: 0.40
```

**Drought (DroughtRisk)**:
```
Agriculture: 0.95 (crop failure, irrigation stress)
Utilities: 0.70 (hydropower, cooling water for thermal plants)
Mining & Metals: 0.50 (water-intensive extraction/processing)
Energy - Renewables: 0.50 (hydropower subset)
Energy - Coal: 0.45 (cooling water for coal plants)
Manufacturing - Heavy: 0.45 (water-intensive processes)
Energy - Oil & Gas: 0.40 (refining uses water)
Consumer Goods: 0.40 (food/beverage especially)
Manufacturing - Light: 0.35
Real Estate & Construction: 0.35 (some water use)
Transportation: 0.30 (water transport affected)
Healthcare: 0.30 (sanitation critical)
Technology & Services: 0.20 (data center cooling)
Financial Services: 0.20 (minimal water use)
Other: 0.40
```

---

### 3.4 Downloaded Files and Manual Download Instructions

**Location:** `/data/calibration/raw_data/`
**Collection Date:** 2026-02-25
**Status:** 5 files downloaded (11.2 MB), 3 files require manual download

#### Downloaded Files (5 files)

**1. UNECE_COP27_Industry_Brief.pdf (7 KB)**
- **Source:** UNECE Technology Brief on Carbon Neutral Energy Intensive Industries
- **URL:** https://unece.org/sites/default/files/2022-11/Industry%20brief_EN_2.pdf
- **Content:** Emissions intensity by revenue for cement (7 kg CO2/$), steel (1.5 kg/$), chemicals (0.3 kg/$)
- **Use Case:** Sector carbon dependencies (S_{s,carbon} for manufacturing sectors)

**2. IEA_GHG_Documentation_2022.pdf (1.7 MB)**
- **Source:** IEA Greenhouse Gas Emissions from Energy 2022 Database Documentation
- **URL:** https://iea.blob.core.windows.net/assets/f535fcce-abe8-49ff-9cc9-5c1d9d6eec07/WORLD_GHG_Documentation.pdf
- **Content:** Methodology for sectoral GHG emissions accounting
- **Use Case:** Understanding sectoral emissions intensity calibration approach

**3. IEA_CO2_Emissions_2022.pdf (665 KB)**
- **Source:** IEA CO2 Emissions in 2022 Report
- **URL:** https://iea.blob.core.windows.net/assets/3c8fa115-35c4-4474-b237-1b00424c8844/CO2Emissionsin2022.pdf
- **Content:** Cement, steel, chemicals account for 27%, 25%, 14% of industrial CO2
- **Use Case:** Sector carbon dependencies validation

**4. WorldBank_State_Trends_Carbon_Pricing_2024.pdf (6.9 MB)**
- **Source:** World Bank State and Trends of Carbon Pricing 2024
- **URL:** https://documents1.worldbank.org/curated/en/099081624122529330/pdf/P50228315fd8d1050186341ea02e1c107bc.pdf
- **Content:** Regional carbon pricing data (Europe $50/tCO2, North America $48/tCO2, global average $32/tCO2)
- **Use Case:** Regional carbon dependencies (R_{r,carbon} for all 7 regions)

**5. OECD_GHG_Data_Documentation_2024.pdf (1.7 MB)**
- **Source:** OECD Greenhouse Gas Emissions Data - Statistical Working Paper
- **URL:** https://www.oecd.org/content/dam/oecd/en/publications/reports/2024/06/greenhouse-gas-emissions-data_57bb38a1/b3e6c074-en.pdf
- **Content:** Overview of OECD GHG datasets including Air Emissions Accounts
- **Use Case:** Understanding sectoral emissions data availability and methodology

#### Files Requiring Manual Download (3 files)

**6. IPCC AR6 WGII Observed and Projected Impact Assessment Database (Excel, 442 KB)**
- **Source:** IPCC Data Distribution Centre (Columbia University SEDAC)
- **URL:** https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/
- **Direct Download:** https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/data/AR6-WGII-Observed_and_Projected_Impacts_Assessment.xlsx
- **Status:** Server connection timeout (2026-02-25) - requires manual download
- **Content:** Observed and projected sectoral impacts by climate hazard
- **Use Case:** Sector physical risk vulnerabilities (S_{s,heat}, S_{s,flood}, S_{s,drought})

**Download Instructions:**
1. Visit the main page: https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/
2. Click "Download" link for the Excel workbook
3. Save as `IPCC_AR6_WGII_Observed_Projected_Impacts.xlsx` in `/data/calibration/raw_data/`

**Alternative:** Try direct link with wget:
```bash
wget -O /Users/Owen/cpm/data/calibration/raw_data/IPCC_AR6_WGII_Observed_Projected_Impacts.xlsx \
  "https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/data/AR6-WGII-Observed_and_Projected_Impacts_Assessment.xlsx"
```

**7. OECD Air Emissions Accounts - Sectoral CO2 Data (CSV)**
- **Source:** OECD Data Explorer - Air Emissions Accounts (SEEA_AEA_A)
- **URL:** https://data-explorer.oecd.org
- **Status:** Requires interactive data selection (no direct download link)
- **Content:** Sectoral CO2 emissions by ISIC classification, 63 countries, 1990-2021
- **Use Case:** Sector carbon dependencies (S_{s,carbon} for all 15 sectors)

**Download Instructions:**
1. Visit OECD Data Explorer: https://data-explorer.oecd.org
2. Search for "Air Emissions Accounts" or navigate to dataset SEEA_AEA_A
3. Apply filters:
   - **Pollutant:** CO2 (carbon dioxide)
   - **Activity:** Select all ISIC Rev.4 sectors (or total by industry)
   - **Time:** 2021 (latest available)
   - **Countries:** All OECD countries or select representative sample
4. Click download icon (top right) → "Select data only (.csv)" or "with labels (.csv)"
5. Save as `OECD_Air_Emissions_Accounts_Sectoral_2021.csv` in `/data/calibration/raw_data/`

**API Alternative:** Use OECD SDMX API with custom query (requires building filter):
```bash
# Example API structure (customize filter based on data explorer selection):
curl "https://sdmx.oecd.org/public/rest/data/ESTAT,SEEA_AEA_A,1.4/{FILTER}?startPeriod=2021&endPeriod=2021&format=csvfilewithlabels" \
  -o /Users/Owen/cpm/data/calibration/raw_data/OECD_Air_Emissions_Accounts_Sectoral_2021.csv
```

**Note:** Get exact API query by clicking "Developer API" icon in data explorer after selecting data.

**8. IEA Greenhouse Gas Emissions from Energy Highlights (Excel)**
- **Source:** IEA Greenhouse Gas Emissions from Energy Highlights 2022 Edition
- **URL:** https://www.iea.org/data-and-statistics/data-product/greenhouse-gas-emissions-from-energy-highlights
- **Status:** Requires free IEA account registration
- **Content:** Sectoral GHG emissions for 190+ countries, 1971-2022
- **Use Case:** Sector carbon dependencies, cross-validation with OECD

**Download Instructions:**
1. Visit product page: https://www.iea.org/data-and-statistics/data-product/greenhouse-gas-emissions-from-energy-highlights
2. Create free IEA account or log in
3. Download September 2022 edition (XLSX format)
4. Save as `IEA_GHG_Emissions_Energy_Highlights_2022.xlsx` in `/data/calibration/raw_data/`

**Alternative - Data Explorer:**
1. Use interactive tool: https://www.iea.org/data-and-statistics/data-tools/greenhouse-gas-emissions-from-energy-data-explorer
2. Select:
   - **Countries:** World or regional aggregates
   - **Sectors:** All available (energy, industry, transport, etc.)
   - **Years:** 2015-2022
3. Export to Excel/CSV

#### Data Coverage Summary

| Data Type | Files | Status | Format | Size |
|-----------|-------|--------|--------|------|
| **Sector Carbon (S_{s,carbon})** | UNECE, IEA PDFs, OECD docs | ✅ Docs downloaded, ❌ Raw data pending | PDF + CSV (pending) | ~4 MB |
| **Regional Carbon (R_{r,carbon})** | World Bank 2024 | ✅ Downloaded | PDF | 7 MB |
| **Physical Vulnerability (S_{s,heat/flood/drought})** | IPCC AR6 | ❌ Manual download required | Excel (442 KB) | TBD |
| **Total Downloaded** | 5 files | ✅ Complete | PDF | 11.2 MB |
| **Total Pending** | 3 files | ❌ Manual required | Excel + CSV | ~1-2 MB |

#### Next Steps for Data Processing

**After Manual Downloads Complete:**
1. Extract relevant tables from PDFs (World Bank regional pricing, UNECE emissions intensity)
2. Process IPCC Excel workbook to extract sectoral vulnerability assessments
3. Aggregate OECD sectoral emissions to our 15-sector taxonomy
4. Aggregate IEA sectoral emissions to our 15-sector taxonomy
5. Cross-validate emissions intensity estimates across sources

**Calibration Matrix Population:**
1. Populate `sector_scores.csv` (S matrix, 15×8) using processed data
2. Populate `region_scores.csv` (R matrix, 7×8) using processed data
3. Document data-to-score mapping rationale in `/data/calibration/README.md`

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
| ✅ Sector carbon intensity | IEA + OECD + UNECE | Researched | No |
| ✅ Regional carbon pricing | World Bank Dashboard 2024 | Researched | No |
| ✅ Physical vulnerabilities | IPCC AR6 WGII | Researched | No |

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

### Phase 2: Calibration Data Collection (🔄 PARTIAL)

**Completed 2026-02-25:**

**Downloaded Files (5 files, 11.2 MB)** → `/data/calibration/raw_data/`:
- ✅ UNECE COP27 Industry Brief (7 KB) - emissions intensity data
- ✅ IEA CO2 Emissions 2022 Report (665 KB) - sectoral emissions
- ✅ IEA GHG Documentation 2022 (1.7 MB) - methodology
- ✅ World Bank State & Trends Carbon Pricing 2024 (6.9 MB) - regional pricing
- ✅ OECD GHG Data Documentation 2024 (1.7 MB) - emissions accounts overview

**Pending Manual Downloads (3 files)**:
- ⏳ IPCC AR6 WGII Impact Assessment Database (Excel, 442 KB) - server timeout, requires manual download
- ⏳ OECD Air Emissions Accounts sectoral CSV - requires interactive data selection
- ⏳ IEA GHG sectoral data (Excel) - requires free IEA account

**Detailed instructions**: See Section 3.4 above

**Research Findings**:
- ✅ Sector carbon scores: Recommended values documented in Section 3.1
- ✅ Regional carbon scores: Recommended values documented in Section 3.2
- ✅ Physical vulnerability scores: Recommended values documented in Section 3.3

**Status**: 🔄 Core documentation downloaded, 3 datasets pending manual download

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
Raw sources → /data/calibration/raw_data/*.pdf (5 files + 3 pending)
Processing scripts → sector_scores.csv (S matrix: 15×8)
Processing scripts → region_scores.csv (R matrix: 7×8)
See: Section 3.4 for complete file inventory and download instructions
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

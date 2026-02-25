# Calibration Data Inventory

**Location:** `/data/calibration/raw_data/`
**Purpose:** Raw data sources for calibrating sector/region sensitivity matrices (S and R)
**Collection Date:** 2026-02-25

---

## Downloaded Files (5 total, 11.2 MB)

### 1. UNECE_COP27_Industry_Brief.pdf (7 KB)
**Source:** UNECE Technology Brief on Carbon Neutral Energy Intensive Industries
**URL:** https://unece.org/sites/default/files/2022-11/Industry%20brief_EN_2.pdf
**Content:** Emissions intensity by revenue for cement (7 kg CO2/$), steel (1.5 kg/$), chemicals (0.3 kg/$)
**Use Case:** Sector carbon dependencies (S_{s,carbon} for manufacturing sectors)

### 2. IEA_GHG_Documentation_2022.pdf (1.7 MB)
**Source:** IEA Greenhouse Gas Emissions from Energy 2022 Database Documentation
**URL:** https://iea.blob.core.windows.net/assets/f535fcce-abe8-49ff-9cc9-5c1d9d6eec07/WORLD_GHG_Documentation.pdf
**Content:** Methodology for sectoral GHG emissions accounting
**Use Case:** Understanding sectoral emissions intensity calibration approach

### 3. IEA_CO2_Emissions_2022.pdf (665 KB)
**Source:** IEA CO2 Emissions in 2022 Report
**URL:** https://iea.blob.core.windows.net/assets/3c8fa115-35c4-4474-b237-1b00424c8844/CO2Emissionsin2022.pdf
**Content:** Cement, steel, chemicals account for 27%, 25%, 14% of industrial CO2
**Use Case:** Sector carbon dependencies validation

### 4. WorldBank_State_Trends_Carbon_Pricing_2024.pdf (6.9 MB)
**Source:** World Bank State and Trends of Carbon Pricing 2024
**URL:** https://documents1.worldbank.org/curated/en/099081624122529330/pdf/P50228315fd8d1050186341ea02e1c107bc.pdf
**Content:** Regional carbon pricing data (Europe $50/tCO2, North America $48/tCO2, global average $32/tCO2)
**Use Case:** Regional carbon dependencies (R_{r,carbon} for all 7 regions)

### 5. OECD_GHG_Data_Documentation_2024.pdf (1.7 MB)
**Source:** OECD Greenhouse Gas Emissions Data - Statistical Working Paper
**URL:** https://www.oecd.org/content/dam/oecd/en/publications/reports/2024/06/greenhouse-gas-emissions-data_57bb38a1/b3e6c074-en.pdf
**Content:** Overview of OECD GHG datasets including Air Emissions Accounts
**Use Case:** Understanding sectoral emissions data availability and methodology

---

## Files Requiring Manual Download (3 total)

### 6. IPCC AR6 WGII Observed and Projected Impact Assessment Database (Excel, 442 KB)
**Source:** IPCC Data Distribution Centre (Columbia University SEDAC)
**URL:** https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/
**Direct Download:** https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/data/AR6-WGII-Observed_and_Projected_Impacts_Assessment.xlsx
**Status:** Server connection timeout (2026-02-25) - requires manual download
**Content:** Observed and projected sectoral impacts by climate hazard
**Use Case:** Sector physical risk vulnerabilities (S_{s,heat}, S_{s,flood}, S_{s,drought})

**Download Instructions:**
1. Visit the main page: https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/
2. Click "Download" link for the Excel workbook
3. Save as `IPCC_AR6_WGII_Observed_Projected_Impacts.xlsx` in this directory

**Alternative:** Try direct link with wget:
```bash
wget -O IPCC_AR6_WGII_Observed_Projected_Impacts.xlsx \
  "https://sedac.ciesin.columbia.edu/ddc/impactsassess_ar6/data/AR6-WGII-Observed_and_Projected_Impacts_Assessment.xlsx"
```

### 7. OECD Air Emissions Accounts - Sectoral CO2 Data (CSV)
**Source:** OECD Data Explorer - Air Emissions Accounts (SEEA_AEA_A)
**URL:** https://data-explorer.oecd.org
**Status:** Requires interactive data selection (no direct download link)
**Content:** Sectoral CO2 emissions by ISIC classification, 63 countries, 1990-2021
**Use Case:** Sector carbon dependencies (S_{s,carbon} for all 15 sectors)

**Download Instructions:**
1. Visit OECD Data Explorer: https://data-explorer.oecd.org
2. Search for "Air Emissions Accounts" or navigate to dataset SEEA_AEA_A
3. Apply filters:
   - **Pollutant:** CO2 (carbon dioxide)
   - **Activity:** Select all ISIC Rev.4 sectors (or total by industry)
   - **Time:** 2021 (latest available)
   - **Countries:** All OECD countries or select representative sample
4. Click download icon (top right) → "Select data only (.csv)" or "with labels (.csv)"
5. Save as `OECD_Air_Emissions_Accounts_Sectoral_2021.csv` in this directory

**API Alternative:** Use OECD SDMX API with custom query (requires building filter):
```bash
# Example API structure (customize filter based on data explorer selection):
curl "https://sdmx.oecd.org/public/rest/data/ESTAT,SEEA_AEA_A,1.4/{FILTER}?startPeriod=2021&endPeriod=2021&format=csvfilewithlabels" \
  -o OECD_Air_Emissions_Accounts_Sectoral_2021.csv
```

**Note:** Get exact API query by clicking "Developer API" icon in data explorer after selecting data.

### 8. IEA Greenhouse Gas Emissions from Energy Highlights (Excel)
**Source:** IEA Greenhouse Gas Emissions from Energy Highlights 2022 Edition
**URL:** https://www.iea.org/data-and-statistics/data-product/greenhouse-gas-emissions-from-energy-highlights
**Status:** Requires free IEA account registration
**Content:** Sectoral GHG emissions for 190+ countries, 1971-2022
**Use Case:** Sector carbon dependencies, cross-validation with OECD

**Download Instructions:**
1. Visit product page: https://www.iea.org/data-and-statistics/data-product/greenhouse-gas-emissions-from-energy-highlights
2. Create free IEA account or log in
3. Download September 2022 edition (XLSX format)
4. Save as `IEA_GHG_Emissions_Energy_Highlights_2022.xlsx` in this directory

**Alternative - Data Explorer:**
1. Use interactive tool: https://www.iea.org/data-and-statistics/data-tools/greenhouse-gas-emissions-from-energy-data-explorer
2. Select:
   - **Countries:** World or regional aggregates
   - **Sectors:** All available (energy, industry, transport, etc.)
   - **Years:** 2015-2022
3. Export to Excel/CSV

---

## Data Coverage Summary

| Data Type | Files | Status | Format | Size |
|-----------|-------|--------|--------|------|
| **Sector Carbon (S_{s,carbon})** | UNECE, IEA PDFs, OECD docs | ✅ Docs downloaded, ❌ Raw data pending | PDF + CSV (pending) | ~4 MB |
| **Regional Carbon (R_{r,carbon})** | World Bank 2024 | ✅ Downloaded | PDF | 7 MB |
| **Physical Vulnerability (S_{s,heat/flood/drought})** | IPCC AR6 | ❌ Manual download required | Excel (442 KB) | TBD |
| **Total Downloaded** | 5 files | ✅ Complete | PDF | 11.2 MB |
| **Total Pending** | 3 files | ❌ Manual required | Excel + CSV | ~1-2 MB |

---

## Next Steps

### Immediate (Manual Downloads)
1. Download IPCC AR6 WGII database (Excel) - 5 minutes
2. Download OECD Air Emissions sectoral data (CSV) - 10 minutes
3. Download IEA GHG sectoral data (Excel) - 5 minutes with account

### Data Processing (After downloads complete)
1. Extract relevant tables from PDFs (World Bank regional pricing, UNECE emissions intensity)
2. Process IPCC Excel workbook to extract sectoral vulnerability assessments
3. Aggregate OECD sectoral emissions to our 15-sector taxonomy
4. Aggregate IEA sectoral emissions to our 15-sector taxonomy
5. Cross-validate emissions intensity estimates across sources

### Calibration Matrix Population
1. Populate `sector_scores.csv` (S matrix, 15×8) using processed data
2. Populate `region_scores.csv` (R matrix, 7×8) using processed data
3. Document data-to-score mapping rationale in `/data/calibration/README.md`

---

## References

- **Model Documentation:** `/docs/credit_portfolio_model_climate_overlay.md`
- **Data Sources Documentation:** `/docs/data_sources.md` (Part 3: Calibration Data)
- **Calibration Parameters Documentation:** `/data/calibration/README.md`
- **Implementation Plan:** `/docs/implementation_plan.md` (Phase 3)

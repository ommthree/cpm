# Sensitivity Calibration Data Sources

**Purpose**: Document external data sources for calibrating S and R exposure scores (sensitivity matrices)

**Scope**:
- Sector carbon dependencies (S matrix, carbon price column)
- Regional carbon dependencies (R matrix, carbon price column)
- Sector physical risk vulnerabilities (S matrix, heat/flood/drought columns)

**Last Updated**: 2026-02-25

---

## Overview

The model requires exposure scores S_{s,k} and R_{r,k} that quantify:
- How sensitive is **sector s** to **driver k**? (S matrix: 15 sectors × 8 drivers)
- How exposed is **region r** to **driver k**? (R matrix: 7 regions × 8 drivers)

This document focuses on **carbon price exposure** and **physical risk vulnerabilities**, which require external data to calibrate credibly.

---

## Part 1: Sector Carbon Dependencies

**Objective**: Calibrate S_{s,carbon} ∈ [0,1] for 15 sectors

**Question**: "If carbon price increases by 1 standard deviation, which sectors face the most credit stress?"

### Conceptual Framework

Sector carbon dependency has three components:

1. **Emissions Intensity**: Direct GHG emissions per unit revenue
2. **Carbon Cost Burden**: Carbon price × emissions as % of revenue/EBITDA
3. **Stranded Asset Risk**: Share of assets at risk under carbon transition

**Formula** (conceptual):
```
S_{s,carbon} = w1 × (Emissions Intensity)_s
             + w2 × (Carbon Cost Burden)_s
             + w3 × (Stranded Asset Risk)_s
```

Where weights w1, w2, w3 reflect judgment on which component matters most for credit stress.

---

### Data Source 1: CDP (Carbon Disclosure Project)

**URL**: https://www.cdp.net/en

**What They Provide**:
- Company-level Scope 1, 2, 3 emissions disclosures
- Emissions intensity metrics (tCO2e per $M revenue)
- Climate risk assessments and transition plans
- Coverage: ~18,000 companies globally

**How to Use**:
1. Download CDP company disclosures (requires free registration)
2. Filter by sector (use CDP's sector classification or map to our 15 sectors)
3. Compute sector median emissions intensity:
   ```
   Emissions Intensity_s = median(Scope1+2 / Revenue) for companies in sector s
   ```
4. Normalize to [0,1] by dividing by maximum across sectors

**Example Calculation**:
```
Coal sector:
- Median emissions: 800 tCO2e per $M revenue
- Max across all sectors: 1000 tCO2e per $M revenue
- S_{coal,carbon} = 800 / 1000 = 0.80 (before adjustment)

Healthcare:
- Median emissions: 50 tCO2e per $M revenue
- S_{healthcare,carbon} = 50 / 1000 = 0.05
```

**Advantages**:
- High-quality, audited data
- Company-level granularity allows sector aggregation
- Globally representative

**Limitations**:
- Not all companies disclose (coverage varies by sector)
- Scope 3 data is incomplete
- Self-reported (quality varies)

---

### Data Source 2: OECD Carbon Intensity Database

**URL**: https://stats.oecd.org/ → Environment → Air and Climate

**What They Provide**:
- Sectoral GHG emissions by country and sector
- Energy intensity by sector
- Coverage: OECD countries (38 member states)

**Relevant Datasets**:
- **"Air emissions accounts by industry"**: CO2 emissions by ISIC sector
- **"Energy intensity by sector"**: GJ per unit output

**How to Use**:
1. Download ISIC sector emissions data
2. Map ISIC codes to our 15-sector taxonomy:
   ```
   ISIC D → Mining & Metals
   ISIC 23 → Manufacturing - Heavy (Coke, petroleum, chemicals)
   ISIC 40-41 → Utilities
   ISIC 60-63 → Transportation
   ... (full mapping table needed)
   ```
3. Compute emissions intensity: tCO2 / $ gross output
4. Normalize to [0,1]

**Advantages**:
- Official government statistics
- Consistent methodology across countries
- Long time series (1990-2020)

**Limitations**:
- OECD only (excludes major emerging markets)
- Sector classification doesn't perfectly match ours
- Aggregated (no company-level detail)

---

### Data Source 3: IEA Energy Technology Perspectives

**URL**: https://www.iea.org/reports/energy-technology-perspectives-2023

**What They Provide**:
- Sector-level energy consumption and emissions
- Abatement cost curves by sector
- Technology transition pathways (coal phaseout, electrification, etc.)

**Relevant Data**:
- **Chapter: "Sectoral Roadmaps"**
  - Energy intensity by sector (GJ per $ output)
  - Emissions intensity by sector (tCO2 per GJ)
  - Abatement costs ($/tCO2 by sector)

**How to Use**:
1. Extract sector energy intensity from IEA roadmaps
2. Multiply by carbon price to get carbon cost as % of revenue:
   ```
   Carbon Cost Burden_s = (Energy Intensity_s) × (Emissions Factor_s) × (Carbon Price) / (Revenue)
   ```
3. Use as input to S_{s,carbon} calibration

**Example** (conceptual):
```
Steel (Heavy Manufacturing):
- Energy intensity: 20 GJ per tonne steel
- Emissions factor: 0.08 tCO2 per GJ (coal-based)
- Steel price: $800 per tonne
- Carbon price: $100 per tCO2
- Carbon cost: (20 × 0.08 × 100) / 800 = 20% of revenue
- High carbon exposure → S_{steel,carbon} = 0.85
```

**Advantages**:
- Authoritative source (IEA is gold standard for energy data)
- Forward-looking (includes transition scenarios)
- Global coverage with regional breakdowns

**Limitations**:
- Requires purchase ($150-300 for full report)
- Aggregated sector definitions (may need manual disaggregation)

---

### Data Source 4: Academic Literature - Carbon Beta Estimates

**Key Papers**:

1. **Görgen et al. (2020)**: "Carbon Risk"
   - Journal: SSRN Working Paper
   - Estimates "carbon beta" for sectors (sensitivity of stock returns to carbon price)
   - URL: https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2930897

2. **Bolton & Kacperczyk (2021)**: "Do Investors Care About Carbon Risk?"
   - Journal: Journal of Financial Economics
   - Finds carbon intensity predicts stock returns and credit spreads
   - Sector-level carbon intensity estimates

3. **Ilhan et al. (2021)**: "Carbon Tail Risk"
   - Journal: Review of Financial Studies
   - Carbon tail risk by sector (extreme scenarios)
   - Includes coal, oil & gas, utilities

**How to Use**:
1. Extract sector carbon betas from published tables
2. Use as proxy for S_{s,carbon}:
   ```
   If β_{coal,carbon} = 0.95 (high sensitivity)
   Then S_{coal,carbon} ≈ 0.95
   ```
3. Validate against emissions intensity data

**Advantages**:
- Peer-reviewed academic rigor
- Explicitly measures credit/equity sensitivity to carbon risk
- Sector-level estimates ready to use

**Limitations**:
- Historical estimates (may not reflect future transition)
- US/Europe biased (limited emerging market coverage)
- Different sector classifications across papers

---

### Data Source 5: World Bank Carbon Pricing Database

**URL**: https://carbonpricingdashboard.worldbank.org/

**What They Provide**:
- Carbon pricing coverage by country and sector
- Effective carbon rates (explicit + implicit)
- Sector exemptions and special provisions

**How to Use**:
1. Download sector coverage data by country
2. Compute weighted average carbon exposure:
   ```
   S_{s,carbon} = (Share of sector s output subject to carbon pricing) × (Effective carbon rate)
   ```
3. Use to validate CDP/OECD estimates

**Example**:
```
EU Steel:
- 100% of production covered by EU ETS
- Effective carbon rate: €80/tCO2
- High exposure → S_{steel,carbon} = 0.90

US Steel:
- 0% covered by federal carbon pricing
- Implicit carbon cost from state regulations: $10/tCO2 equivalent
- Lower exposure → S_{steel,carbon} = 0.60 (adjust for US region weight)
```

---

### Recommended Approach: Multi-Source Triangulation

**Step 1: Primary Calibration (CDP + OECD)**
- Use CDP emissions intensity as base
- Fill gaps with OECD sector emissions data
- Normalize to [0,1] scale

**Step 2: Validation (IEA + Academic)**
- Check if S_{s,carbon} ranks match IEA abatement costs (high cost → high exposure)
- Validate against academic carbon beta estimates
- Adjust if major discrepancies

**Step 3: Expert Judgment**
- Energy - Coal: S = 1.0 (highest exposure by definition)
- Energy - Oil & Gas: S = 0.90
- Energy - Renewables: S = 0.20 (low or inverted exposure)
- Utilities: S = 0.70
- Heavy Manufacturing: S = 0.80
- Transportation: S = 0.80
- Light Manufacturing: S = 0.40
- Agriculture: S = 0.30
- Real Estate: S = 0.50
- Financials: S = 0.20
- Technology: S = 0.20
- Consumer Goods: S = 0.40
- Healthcare: S = 0.30
- Mining & Metals: S = 0.70
- Other: S = 0.50

**Step 4: Sensitivity Analysis**
- Vary all S_{s,carbon} by ±30%
- Check portfolio impact
- Document uncertainty ranges

---

## Part 2: Regional Carbon Price Dependencies

**Objective**: Calibrate R_{r,carbon} ∈ [0,1] for 7 regions

**Question**: "Which regions face the most credit stress from carbon pricing policies?"

### Conceptual Framework

Regional carbon dependency has three components:

1. **Policy Stringency**: Likelihood and severity of carbon pricing
2. **Carbon Pricing Coverage**: % of emissions covered by explicit pricing
3. **Economic Structure**: Dependence on carbon-intensive sectors

---

### Data Source 1: World Bank Carbon Pricing Dashboard

**URL**: https://carbonpricingdashboard.worldbank.org/

**What They Provide**:
- **Map of carbon pricing initiatives** globally
  - ETS (Emissions Trading Systems)
  - Carbon taxes
  - Coverage by jurisdiction

- **Key Metrics by Country/Region**:
  - Share of GHG emissions covered by carbon pricing
  - Average effective carbon rate ($/tCO2)
  - Revenue generated from carbon pricing

**How to Use**:

1. **Download regional coverage data**:
   ```
   Europe:
   - EU ETS covers 40% of emissions
   - Average carbon price: €80/tCO2 (~$85)
   - Coverage score: 0.40 × (85/100) = 0.34

   North America:
   - RGGI (Northeast US) + California + Canada: ~10% of emissions
   - Average price: $40/tCO2
   - Coverage score: 0.10 × (40/100) = 0.04
   ```

2. **Normalize to [0,1]**:
   ```
   R_{r,carbon} = (Coverage × Effective Rate) / max(Coverage × Rate)
   ```

3. **Example Regional Scores**:
   ```
   Europe: R = 0.90 (high policy stringency, EU ETS mature)
   North America: R = 0.70 (growing coverage, state-level action)
   Asia-Pacific (developed): R = 0.70 (Japan, Korea, Australia have pricing)
   Asia-Pacific (emerging): R = 0.60 (China ETS, but low price)
   Latin America: R = 0.50 (Mexico, Colombia have carbon taxes)
   Middle East & Africa: R = 0.40 (South Africa only, oil producers resistant)
   Global: R = 0.70 (weighted average)
   ```

**Advantages**:
- Official World Bank data
- Comprehensive global coverage
- Updated annually
- Includes both explicit and implicit carbon pricing

**Limitations**:
- Current policy, not forward-looking (but we use NGFS scenarios for that)
- Aggregates vary in definition across countries

---

### Data Source 2: OECD Effective Carbon Rates

**URL**: https://www.oecd.org/tax/tax-policy/effective-carbon-rates-2021.htm

**What They Provide**:
- **Effective carbon rates** by country and sector
  - Explicit carbon pricing (taxes, ETS)
  - Implicit carbon pricing (fossil fuel taxes)
  - Energy taxes

- **Coverage**: 44 OECD + G20 countries, representing 80% of global emissions

**Key Metric**: "Carbon pricing gap"
- How much of emissions are priced below benchmark (e.g., €60/tCO2)?
- Larger gap = lower carbon price exposure

**How to Use**:

1. **Extract regional effective carbon rates**:
   ```
   OECD Database → "Effective Carbon Rates" → Download Excel
   Filter by region, aggregate by emissions-weighted average
   ```

2. **Compute regional exposure**:
   ```
   R_{r,carbon} = (Effective Carbon Rate_r) / (Benchmark Rate)

   Example with €60/tCO2 benchmark:
   - Europe avg: €50/tCO2 → R = 0.83
   - North America avg: €25/tCO2 → R = 0.42
   - Asia-Pacific EM avg: €10/tCO2 → R = 0.17
   ```

3. **Adjust for policy trajectory** (from NGFS scenarios):
   - Regions with strong policy commitments get higher R scores
   - Use NGFS carbon price projections as target

**Advantages**:
- Granular (country × sector level)
- Includes implicit carbon pricing (fuel taxes)
- Consistent methodology across countries

**Limitations**:
- OECD + G20 only (misses some emerging markets)
- Static snapshot (doesn't account for policy momentum)

---

### Data Source 3: Climate Action Tracker

**URL**: https://climateactiontracker.org/

**What They Provide**:
- **Policy assessments** for 40+ countries
  - Current policy trajectory
  - NDC (Nationally Determined Contributions) targets
  - Rating: "Critically insufficient" to "1.5°C compatible"

- **Regional Policy Stringency Scores**:
  - Policy coverage
  - Implementation strength
  - Ambition level

**How to Use**:

1. **Map CAT ratings to scores**:
   ```
   1.5°C compatible → R_{carbon} = 0.95
   2°C compatible → R_{carbon} = 0.80
   Insufficient → R_{carbon} = 0.60
   Highly insufficient → R_{carbon} = 0.40
   Critically insufficient → R_{carbon} = 0.20
   ```

2. **Aggregate by region**:
   ```
   Europe:
   - EU: "Insufficient" but strong implementation → 0.90
   - UK: "Insufficient" → 0.85
   → Regional avg: R_{europe,carbon} = 0.88

   Asia-Pacific (emerging):
   - China: "Highly insufficient" → 0.50
   - India: "Insufficient" → 0.55
   - Indonesia: "Highly insufficient" → 0.45
   → Regional avg: R_{asia_em,carbon} = 0.50
   ```

**Advantages**:
- Forward-looking (assesses policy trajectory, not just current)
- Qualitative assessment captures policy momentum
- Covers major emitters

**Limitations**:
- Qualitative (requires judgment to convert to scores)
- Country-level (requires aggregation to regions)
- Updated annually (may lag recent policy changes)

---

### Data Source 4: NGFS Scenario Carbon Prices (Our Own Data!)

**Source**: NGFS Phase V scenarios (already downloaded)

**What They Provide**:
- Regional carbon price projections (2025-2050)
- Three scenarios: Net Zero 2050, Delayed Transition, Current Policies

**How to Use**:

1. **Use NGFS 2030 carbon prices as proxy for policy stringency**:
   ```
   Net Zero 2050 scenario, carbon price in 2030:
   - Europe: $150/tCO2
   - North America: $120/tCO2
   - Asia-Pacific (dev): $100/tCO2
   - Asia-Pacific (EM): $80/tCO2
   - Latin America: $60/tCO2
   - Middle East & Africa: $40/tCO2
   ```

2. **Normalize to [0,1]**:
   ```
   R_{r,carbon} = (Carbon Price_r) / max(Carbon Price)

   R_{europe,carbon} = 150 / 150 = 1.00
   R_{NA,carbon} = 120 / 150 = 0.80
   R_{MEA,carbon} = 40 / 150 = 0.27
   ```

3. **Adjust for Current Policies scenario** (more realistic):
   ```
   Current Policies 2030:
   - Europe: $80/tCO2
   - North America: $40/tCO2
   - Asia-Pacific (EM): $20/tCO2
   - Middle East & Africa: $5/tCO2

   → Use geometric mean of Net Zero and Current Policies for baseline
   ```

**Advantages**:
- Consistent with our scenario framework
- Already have the data
- Forward-looking (2030 is medium-term)

**Limitations**:
- These are scenario projections, not current policy
- May overstate policy ambition (especially Net Zero)
- Need to cross-check with World Bank/OECD current policy data

---

### Recommended Approach: Hybrid Calibration

**Step 1: Current Policy Baseline (World Bank + OECD)**
- Use World Bank carbon pricing coverage as starting point
- Adjust for OECD effective carbon rates
- Result: Current policy exposure R_{r,carbon}^{current}

**Step 2: Policy Momentum Adjustment (Climate Action Tracker)**
- Identify regions with strong policy trajectory
- Upweight R scores for regions with strong NDCs and implementation
- Result: Forward-looking R_{r,carbon}^{2030}

**Step 3: Scenario Consistency Check (NGFS)**
- Compare R_{r,carbon} ranking with NGFS 2030 carbon prices
- Ensure consistency: High R regions should have high NGFS carbon prices
- Adjust if ranking doesn't match

**Step 4: Expert Judgment (Final Calibration)**

Proposed R_{r,carbon} scores:

```
Europe: 0.90
- EU ETS mature, high coverage (40%)
- Strong policy trajectory (Fit for 55 package)
- NGFS Net Zero 2030: $150/tCO2

North America: 0.70
- Growing coverage (RGGI, California, Canada)
- Federal action increasing (IRA)
- State-level momentum strong
- NGFS Net Zero 2030: $120/tCO2

Asia-Pacific (developed): 0.70
- Japan, Korea, Australia have carbon pricing
- Medium coverage (~20%)
- Strong policy commitments
- NGFS Net Zero 2030: $100/tCO2

Asia-Pacific (emerging): 0.60
- China ETS launched (largest in world by coverage)
- But low carbon price ($10-15/tCO2)
- India has coal cess (implicit carbon price)
- NGFS Net Zero 2030: $80/tCO2

Latin America: 0.50
- Mexico, Colombia, Chile have carbon taxes
- Low coverage (~10%)
- Moderate policy ambition
- NGFS Net Zero 2030: $60/tCO2

Middle East & Africa: 0.40
- South Africa only country with carbon tax
- Oil producers resistant to carbon pricing
- Low policy ambition
- NGFS Net Zero 2030: $40/tCO2

Global: 0.70
- Weighted average
- Represents multinational corporations
```

**Step 5: Documentation and Sensitivity**
- Document all data sources and assumptions
- Provide sensitivity ranges (±0.15 around central estimate)
- Validate against expert judgment

---

## Part 3: Sector Physical Risk Vulnerabilities

**Objective**: Calibrate S_{s,k} for physical risk drivers (k = heat, flood, drought)

**Question**: "Which sectors are most vulnerable to heat stress, flooding, and drought?"

### Conceptual Framework

Physical risk vulnerability depends on:

1. **Asset Exposure**: Are fixed assets physically exposed to hazard?
2. **Operational Sensitivity**: Does the hazard disrupt operations?
3. **Supply Chain Fragility**: Does the hazard disrupt critical inputs?
4. **Labor Productivity**: Does the hazard reduce worker efficiency?

Different for each hazard:
- **Heat**: Labor-intensive sectors, outdoor work, cooling costs
- **Flood**: Fixed assets in floodplains, transportation infrastructure
- **Drought**: Water-intensive sectors, agriculture, hydropower

---

### Data Source 1: IPCC AR6 Sectoral Vulnerability Assessments

**Source**: IPCC Sixth Assessment Report (2021-2022)
**URL**: https://www.ipcc.ch/report/ar6/

**Relevant Chapters**:
- **WGII Chapter 4**: "Water" (drought vulnerability by sector)
- **WGII Chapter 5**: "Food, Fiber, and Other Ecosystem Products" (agriculture)
- **WGII Chapter 7**: "Health, Wellbeing and the Changing Structure of Communities" (heat vulnerability)
- **WGII Chapter 8**: "Poverty, Livelihoods and Sustainable Development" (sectoral impacts)

**What They Provide**:
- Qualitative vulnerability assessments (high/medium/low)
- Confidence levels (high/medium/low)
- Regional variations in vulnerability

**How to Use**:

1. **Extract sectoral vulnerability ratings** from IPCC tables:
   ```
   Chapter 8, Table 8.3: "Sectoral Climate Risks"
   - Agriculture: HIGH vulnerability to drought, MEDIUM to heat
   - Infrastructure: HIGH vulnerability to flood, MEDIUM to heat
   - Manufacturing: LOW vulnerability to heat, MEDIUM to flood
   ```

2. **Map to our 15 sectors**:
   ```
   IPCC "Agriculture" → Our "Agriculture" sector
   IPCC "Infrastructure" → Our "Real Estate & Construction", "Transportation"
   IPCC "Manufacturing" → Our "Manufacturing - Heavy", "Manufacturing - Light"
   IPCC "Services" → Our "Financials", "Technology & Services"
   ```

3. **Convert qualitative to quantitative**:
   ```
   HIGH vulnerability → S_{s,k} = 0.80-1.00
   MEDIUM vulnerability → S_{s,k} = 0.40-0.70
   LOW vulnerability → S_{s,k} = 0.10-0.30
   ```

**Advantages**:
- Gold standard scientific assessment
- Peer-reviewed by 100s of experts
- Comprehensive coverage

**Limitations**:
- Qualitative (requires judgment to convert to scores)
- Sector definitions don't perfectly match ours
- Regional variations averaged out

---

### Data Source 2: Munich Re / Swiss Re Sector Risk Assessments

**Munich Re NatCatSERVICE**
**URL**: https://www.munichre.com/en/solutions/for-industry-clients/natcatservice.html

**What They Provide**:
- Historical natural disaster losses by sector
- Insured vs. uninsured losses
- Sector-level risk ratings

**Key Reports**:
- "Topics Geo" (annual natural catastrophe review)
- "Sector-specific risk assessments" (available on request)

**How to Use**:

1. **Extract loss ratios by sector and peril**:
   ```
   Example from Munich Re data (illustrative):

   Flood losses (% of sector value at risk):
   - Real Estate: 5% annual loss in high-risk areas
   - Transportation: 3% (infrastructure damage)
   - Agriculture: 2% (crop damage)
   - Manufacturing - Heavy: 2% (facility damage)
   - Financials: 0.5% (indirect exposure)

   Normalize to [0,1]:
   S_{real_estate,flood} = 5% / 5% = 1.00
   S_{transport,flood} = 3% / 5% = 0.60
   S_{financials,flood} = 0.5% / 5% = 0.10
   ```

2. **Use as input to S_{s,k} calibration**

**Advantages**:
- Real-world loss data
- Sector-specific
- Global coverage

**Limitations**:
- Proprietary (requires purchase or partnership)
- Historical losses may not reflect future climate
- Insured losses only (misses uninsured exposures)

---

### Data Source 3: Academic Literature - Sectoral Physical Risk

**Key Papers**:

1. **Kahn et al. (2021)**: "Long-Term Macroeconomic Effects of Climate Change"
   - Journal: Review of Economic Studies
   - Sector-level impact estimates for heat and water stress
   - Global coverage
   - Table 3: "Sectoral Vulnerability to Temperature and Precipitation"

2. **Burke et al. (2015)**: "Global Non-Linear Effect of Temperature on Economic Production"
   - Journal: Nature
   - Labor productivity losses from heat stress by sector
   - Outdoor vs. indoor work distinction

3. **Hsiang et al. (2017)**: "Estimating Economic Damage from Climate Change in the United States"
   - Journal: Science
   - Sector-level damage functions for heat, storms, sea level rise
   - Table S2: "Sectoral Impact Estimates"

4. **Carleton & Hsiang (2016)**: "Social and Economic Impacts of Climate"
   - Journal: Science
   - Meta-analysis of 200+ studies on sectoral climate impacts

**How to Use**:

1. **Extract sectoral impact coefficients**:
   ```
   Burke et al. (2015): Labor productivity vs. temperature
   - Outdoor labor: -2.5% per 1°C above 25°C
   - Indoor labor: -0.5% per 1°C above 25°C

   Map to our sectors:
   - Agriculture (outdoor): High heat sensitivity → S_{agri,heat} = 0.90
   - Construction (outdoor): High heat sensitivity → S_{real_estate,heat} = 0.75
   - Manufacturing (indoor): Medium heat sensitivity → S_{mfg_light,heat} = 0.40
   - Services (indoor): Low heat sensitivity → S_{tech,heat} = 0.20
   ```

2. **Combine with IPCC qualitative assessments** for validation

**Advantages**:
- Quantitative estimates
- Peer-reviewed
- Econometric rigor (causal identification)

**Limitations**:
- US-centric (many papers focus on US)
- Historical data (may not reflect future adaptation)
- Sector definitions vary across papers

---

### Data Source 4: S&P Global Trucost Physical Risk Dataset

**URL**: https://www.spglobal.com/esg/trucost/physical-risk

**What They Provide**:
- Company-level physical risk exposure scores
- Asset-level geographic exposure to hazards
- Sector aggregations available

**Coverage**:
- Heat stress
- Water stress (drought proxy)
- Flooding (coastal and riverine)
- Tropical cyclones
- Wildfires

**How to Use**:

1. **Request sector-level aggregated data** (may require subscription)
2. **Extract median risk scores by sector**:
   ```
   Example (illustrative):
   S&P Trucost Heat Stress Score (0-100):
   - Agriculture: 85
   - Construction: 70
   - Manufacturing: 45
   - Services: 20

   Normalize to [0,1]:
   S_{agri,heat} = 85 / 100 = 0.85
   S_{construction,heat} = 70 / 100 = 0.70
   ```

**Advantages**:
- Company-level granularity (can aggregate to any sector definition)
- Geographic specificity (asset-level exposure)
- Forward-looking (uses climate projections)

**Limitations**:
- Proprietary and expensive ($10k-50k per year)
- Data quality varies by company disclosure
- May require partnership for sector aggregations

---

### Data Source 5: Four Twenty Seven (Moody's) Climate Risk Scores

**URL**: https://www.moodys.com/web/en/us/capabilities/climate-risk.html

**What They Provide**:
- Physical risk scores for 2,000+ companies
- Sector-level vulnerability assessments
- Six hazards: Heat stress, water stress, hurricanes, floods, sea level rise, wildfires

**How to Use**:
1. Access via Moody's Analytics platform (requires subscription)
2. Extract sector median risk scores
3. Map to our 15 sectors

**Advantages**:
- Integrated with Moody's credit ratings
- Credible in financial community
- Sector aggregations readily available

**Limitations**:
- Proprietary and expensive
- Coverage limited to publicly traded companies
- US/Europe biased

---

### Data Source 6: NGFS Physical Risk Indicators (Our Own Supplement!)

**Source**: World Bank CCKP + NGFS

**What We Have**:
- Regional heat, flood, drought projections by scenario
- Can use to validate sector vulnerability assumptions

**How to Use**:

**Heat Vulnerability (Labor Intensity)**:
```
Proxy: Share of outdoor workers in sector

Data sources:
- BLS Occupational Employment Statistics (US)
- ILO statistics (global)

Example:
- Agriculture: 80% outdoor workers → S_{agri,heat} = 0.90
- Construction: 60% outdoor → S_{construction,heat} = 0.70
- Manufacturing: 20% outdoor → S_{mfg,heat} = 0.40
- Services: 5% outdoor → S_{services,heat} = 0.20
```

**Flood Vulnerability (Fixed Asset Exposure)**:
```
Proxy: Share of assets in flood-prone areas

Data sources:
- FEMA flood maps (US)
- EU Flood Directive data (Europe)
- Assume sectors with fixed infrastructure have higher exposure

Example:
- Real Estate: Fixed assets, coastal exposure → S_{real_estate,flood} = 0.90
- Transportation: Infrastructure in floodplains → S_{transport,flood} = 0.70
- Agriculture: Floodplain farming → S_{agri,flood} = 0.60
- Technology: Flexible locations, minimal exposure → S_{tech,flood} = 0.20
```

**Drought Vulnerability (Water Intensity)**:
```
Proxy: Water consumption per $ output

Data sources:
- USGS water use by sector (US)
- FAO Aquastat (global agriculture)

Example:
- Agriculture: 1000 gallons per $1000 output → S_{agri,drought} = 1.00
- Utilities (hydropower): 500 gallons per $1000 → S_{utilities,drought} = 0.60
- Manufacturing: 100 gallons per $1000 → S_{mfg,drought} = 0.30
- Services: 10 gallons per $1000 → S_{services,drought} = 0.10
```

---

### Recommended Approach: Multi-Source Expert Judgment

**Step 1: Literature Review**
- Extract sectoral vulnerability scores from IPCC AR6
- Extract impact coefficients from academic papers (Burke, Hsiang, Kahn)
- Create initial vulnerability matrix (15 sectors × 3 physical drivers)

**Step 2: Validation with Loss Data**
- Compare with Munich Re / Swiss Re loss ratios (if accessible)
- Adjust scores if major discrepancies

**Step 3: Proxy-Based Calibration**
- Heat: Use labor intensity (outdoor work %)
- Flood: Use fixed asset coastal exposure
- Drought: Use water intensity

**Step 4: Expert Judgment (Final Scores)**

**Heat Index (hi35 days >35°C)**:
```
High Vulnerability (S = 0.80-1.00):
- Agriculture: 0.90 (outdoor labor, crop stress)
- Construction (Real Estate): 0.75 (outdoor labor)
- Transportation: 0.70 (outdoor operations, infrastructure stress)

Medium Vulnerability (S = 0.40-0.70):
- Utilities: 0.60 (cooling demand, thermal efficiency loss)
- Manufacturing - Heavy: 0.50 (industrial cooling, worker productivity)
- Manufacturing - Light: 0.40 (some climate-controlled)
- Mining & Metals: 0.55 (outdoor extraction)

Low Vulnerability (S = 0.10-0.30):
- Energy - Oil & Gas: 0.30 (some outdoor operations)
- Energy - Renewables: 0.35 (solar efficiency loss, wind ok)
- Energy - Coal: 0.30
- Financials: 0.20 (indoor, white-collar)
- Technology & Services: 0.20 (indoor, flexible location)
- Consumer Goods: 0.30 (mixed)
- Healthcare: 0.35 (patient care, AC critical)
- Other: 0.40
```

**Flood Risk (rx1day precipitation)**:
```
High Vulnerability (S = 0.80-1.00):
- Real Estate & Construction: 0.90 (fixed assets in floodplains)
- Agriculture: 0.70 (crop damage, soil erosion)
- Transportation: 0.75 (infrastructure disruption)

Medium Vulnerability (S = 0.40-0.70):
- Utilities: 0.60 (power plants near water, distribution damage)
- Manufacturing - Heavy: 0.50 (factory flooding)
- Mining & Metals: 0.50 (mine flooding, tailings dam risk)
- Energy - Oil & Gas: 0.45 (refineries often coastal)
- Manufacturing - Light: 0.40 (warehouses)

Low Vulnerability (S = 0.10-0.30):
- Energy - Renewables: 0.40 (solar/wind often distributed)
- Energy - Coal: 0.30 (mines can flood but less exposed)
- Financials: 0.20 (offices, can relocate)
- Technology & Services: 0.20 (servers in data centers, protected)
- Consumer Goods: 0.30
- Healthcare: 0.30 (hospitals need flood protection)
- Other: 0.40
```

**Drought Risk (cdd consecutive dry days)**:
```
High Vulnerability (S = 0.80-1.00):
- Agriculture: 0.95 (crop failure, irrigation stress)
- Utilities: 0.70 (hydropower, cooling water for thermal plants)

Medium Vulnerability (S = 0.40-0.70):
- Energy - Renewables: 0.50 (hydropower subset)
- Energy - Coal: 0.45 (cooling water for coal plants)
- Energy - Oil & Gas: 0.40 (some refining uses water)
- Mining & Metals: 0.50 (water-intensive extraction/processing)
- Manufacturing - Heavy: 0.45 (water-intensive processes)
- Consumer Goods: 0.40 (food/beverage especially)

Low Vulnerability (S = 0.10-0.30):
- Real Estate & Construction: 0.35 (some water use)
- Transportation: 0.30 (water transport affected, but small)
- Manufacturing - Light: 0.35
- Financials: 0.20 (minimal water use)
- Technology & Services: 0.20 (data centers use water for cooling)
- Healthcare: 0.30 (sanitation critical)
- Other: 0.40
```

**Step 5: Documentation and Validation**
- Document all sources and assumptions
- Validate with portfolio-specific data if available
- Sensitivity analysis: vary by ±30%
- Cross-check rankings with expert judgment

---

## Summary: Data Sources by Calibration Task

| Calibration Task | Primary Sources | Secondary Sources | Effort |
|------------------|----------------|-------------------|--------|
| **Sector Carbon Dependency (S_{s,carbon})** | CDP emissions data, OECD emissions intensity | IEA Energy Perspectives, Academic carbon betas | 2-3 days |
| **Regional Carbon Dependency (R_{r,carbon})** | World Bank Carbon Pricing Dashboard, OECD Effective Carbon Rates | Climate Action Tracker, NGFS scenarios | 1-2 days |
| **Sector Heat Vulnerability (S_{s,heat})** | IPCC AR6, Academic literature (Burke et al.) | Labor intensity proxies (BLS/ILO) | 2-3 days |
| **Sector Flood Vulnerability (S_{s,flood})** | IPCC AR6, Munich Re/Swiss Re (if accessible) | Fixed asset exposure proxies | 2-3 days |
| **Sector Drought Vulnerability (S_{s,drought})** | IPCC AR6, Water intensity data (USGS/FAO) | Academic literature (Kahn et al.) | 2-3 days |

**Total Effort**: 10-15 days for full S and R matrix calibration with documentation

---

## Next Steps

1. **Prioritize calibration tasks**:
   - Start with carbon dependencies (most material for Net Zero scenario)
   - Follow with physical risk (most material for Current Policies scenario)

2. **Data collection**:
   - CDP: Register for free access, download sector emissions
   - World Bank: Download carbon pricing dashboard data
   - IPCC AR6: Extract vulnerability tables from Working Group II
   - Academic papers: Download key papers, extract sectoral coefficients

3. **Calibration workflow**:
   - Populate sector_scores.csv and region_scores.csv templates
   - Document sources and assumptions in calibration README
   - Run sensitivity analysis
   - Validate with expert judgment

4. **Model Risk approval**:
   - Present calibration methodology to Model Risk
   - Document uncertainty ranges
   - Obtain approval for production use

---

## References

### Emissions and Carbon Intensity
- CDP (2024). Corporate Climate Disclosures. https://www.cdp.net/
- OECD (2023). Air Emissions Accounts by Industry. https://stats.oecd.org/
- IEA (2023). Energy Technology Perspectives. https://www.iea.org/

### Carbon Pricing and Policy
- World Bank (2024). Carbon Pricing Dashboard. https://carbonpricingdashboard.worldbank.org/
- OECD (2021). Effective Carbon Rates 2021. https://www.oecd.org/tax/
- Climate Action Tracker (2024). Country Assessments. https://climateactiontracker.org/

### Academic Literature
- Bolton, P., & Kacperczyk, M. (2021). Do investors care about carbon risk? Journal of Financial Economics, 142(2), 517-549.
- Burke, M., Hsiang, S. M., & Miguel, E. (2015). Global non-linear effect of temperature on economic production. Nature, 527(7577), 235-239.
- Görgen, M., Jacob, A., Nerlinger, M., Riordan, R., Rohleder, M., & Wilkens, M. (2020). Carbon risk. SSRN Working Paper.
- Hsiang, S., et al. (2017). Estimating economic damage from climate change in the United States. Science, 356(6345), 1362-1369.
- Ilhan, E., Sautner, Z., & Vilkov, G. (2021). Carbon tail risk. Review of Financial Studies, 34(3), 1540-1571.
- Kahn, M. E., et al. (2021). Long-term macroeconomic effects of climate change: A cross-country analysis. Review of Economic Studies, 88(4), 1802-1829.

### Physical Risk
- IPCC (2022). Climate Change 2022: Impacts, Adaptation and Vulnerability. Sixth Assessment Report, Working Group II. https://www.ipcc.ch/report/ar6/wg2/
- Munich Re (2024). NatCatSERVICE. https://www.munichre.com/natcatservice
- S&P Global (2024). Trucost Physical Risk Analytics. https://www.spglobal.com/esg/trucost/

**Last Updated**: 2026-02-25

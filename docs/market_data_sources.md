# Market Data Sources for Correlation Calibration

## Overview

This document provides a detailed mapping between our model taxonomy (15 sectors, 7 regions) and available market indices for constructing empirical correlation matrices:
- **Corr_S (15×15)**: Sector correlation matrix
- **Corr_R (7×7)**: Region correlation matrix

## Summary of Approach

**Region mapping**: Straightforward - MSCI regional indices map cleanly to our 7 regions

**Sector mapping**: More complex - requires combining multiple data sources due to granularity differences

## 1. Region Correlation Matrix (Corr_R, 7×7)

### Direct MSCI Regional Index Mapping

| Model Region | MSCI Index | Ticker Symbol | Coverage |
|--------------|------------|---------------|----------|
| North America | MSCI North America | MXNA | US + Canada |
| Europe | MSCI Europe | MXEU | Developed Europe (EU + UK + CH + NO) |
| Asia-Pacific (developed) | MSCI Pacific | MXPJ | Japan, Australia, NZ, Singapore, Hong Kong |
| Asia-Pacific (emerging) | MSCI EM Asia | MXMS | China, India, Korea, Taiwan, ASEAN |
| Latin America | MSCI EM Latin America | MXLA | Brazil, Mexico, Chile, Colombia, Peru |
| Middle East & Africa | MSCI EM EMEA | MXEMEA | South Africa, Saudi Arabia, UAE, Qatar, Egypt |
| Global | MSCI ACWI | ACWI | All Country World Index (developed + emerging) |

### Data Sources for Regional Indices

**Primary Source**: MSCI
- **Access**: Bloomberg Terminal (ticker format: `MXNA Index`), Refinitiv Eikon, MSCI website (subscription)
- **Frequency**: Daily, monthly returns available
- **History**: 20+ years for most developed regions, 10-15 years for emerging
- **Format**: Total return indices (includes dividends)

**Alternative Sources**:
- **FTSE Regional Indices**: Similar coverage, free delayed data available
- **S&P Regional Indices**: Dow Jones regional indices
- **STOXX Regional Indices**: European-focused with global coverage

### Recommended Calibration Approach (Regions)

1. **Download monthly total returns** for all 7 MSCI regional indices
2. **Time period**: 10 years (2015-2024) - captures recent correlations, includes COVID
3. **Compute correlation matrix**: Use rolling 5-year windows to assess stability
4. **Validation**: Compare with historical averages (2000-2024) to check if recent period is representative

**Expected regional correlations** (based on typical equity market co-movement):
- North America ↔ Europe: ~0.75 (high integration)
- Developed ↔ Emerging (same region): ~0.65 (spillover effects)
- Latin America ↔ EM Asia: ~0.45 (commodity vs manufacturing)
- Middle East & Africa ↔ Others: ~0.35-0.50 (less integrated, oil-driven)

---

## 2. Sector Correlation Matrix (Corr_S, 15×15)

### Mapping Challenges

Our 15 sectors are more granular than standard GICS 11 sectors:
- We split Energy into 3 subsectors (Oil & Gas, Coal, Renewables)
- We split Manufacturing into 2 subsectors (Heavy, Light)
- We have standalone Agriculture (not a GICS sector)
- We combine Technology & Services

**Solution**: Use a **multi-source approach** combining:
1. MSCI/S&P sector indices where available
2. Sub-industry indices for energy subsectors
3. Specialized commodity/clean energy indices
4. Synthetic indices constructed from company-level data

### Detailed Sector Mapping

#### Group A: Direct GICS Sector Matches (7 sectors)

| Model Sector | MSCI GICS Index | Ticker | Alternative Sources |
|--------------|------------------|--------|---------------------|
| Utilities | MSCI World Utilities | MXWO0UTI | S&P Utilities Select (XLU), STOXX 600 Utilities |
| Financial Services | MSCI World Financials | MXWO0FN | S&P Financial Select (XLF), STOXX 600 Financials |
| Consumer Goods | MSCI World Consumer Staples | MXWO0CS | S&P Consumer Staples (XLP), STOXX 600 Staples |
| Healthcare | MSCI World Health Care | MXWO0HC | S&P Health Care Select (XLV), STOXX 600 Health |
| Mining & Metals | MSCI World Materials | MXWO0MT | S&P Materials Select (XLB), Bloomberg Commodity Metals |
| Real Estate & Construction | MSCI World Real Estate | MXWO0RE | S&P Real Estate Select (XLRE), FTSE EPRA/NAREIT |
| Technology & Services | Composite: MSCI IT + Comm Svc | MXWO0IT, MXWO0CM | S&P Technology (XLK), NASDAQ Composite |

#### Group B: Energy Subsectors (3 sectors requiring sub-industry data)

**Challenge**: MSCI Energy is a single sector; we need to split into Oil & Gas, Coal, and Renewables.

| Model Sector | Proposed Index | Ticker/Source | Notes |
|--------------|----------------|---------------|-------|
| **Energy - Oil & Gas** | S&P Oil & Gas Exploration | XOP | Excludes coal; integrated oil majors |
| | MSCI Energy (proxy) | MXWO0EN | Dominated by O&G (>90% weight) |
| | Alternative: XLE (Energy Select) | XLE | Broad energy, mostly oil & gas |
| **Energy - Coal** | **Synthetic index** | Custom | Construct from coal producer equities |
| | Example constituents | - | Peabody Energy, Arch Resources, China Coal |
| | Alternative: Coal ETF | KOL | VanEck Coal ETF (but illiquid) |
| **Energy - Renewables** | iShares Clean Energy | ICLN | Solar, wind, battery companies |
| | Invesco Solar ETF | TAN | Solar-focused |
| | S&P Global Clean Energy | SPGTCLTR | Broad clean energy index |

**Coal sector challenge**: No major index provider has a dedicated coal sector index due to ESG divestment.

**Solutions**:
1. **Construct synthetic index**: Use equal-weighted or cap-weighted average of 10-15 major coal producers
2. **Use credit spreads**: iTraxx/CDX may have coal company CDS spreads
3. **Proxy with MSCI Energy**: Assign Coal ≈ 0.9 correlation with Oil & Gas (conservative assumption)

#### Group C: Manufacturing Subsectors (2 sectors requiring aggregation)

| Model Sector | Proposed Index | Ticker/Source | Notes |
|--------------|----------------|---------------|-------|
| **Manufacturing - Heavy** | MSCI Industrials (subset) | MXWO0IN | Focus on capital goods, aerospace, machinery |
| | S&P Industrials Select | XLI | Broader industrials |
| | Alternative: S&P Capital Goods | - | Closer to "heavy" definition |
| **Manufacturing - Light** | Composite: Staples + Discretionary | MXWO0CS, MXWO0CD | Consumer goods manufacturing |
| | S&P Consumer Discretionary | XLY | Autos, durables, apparel |

**Manufacturing split challenge**: GICS "Industrials" mixes heavy (steel, machinery) and light (packaging, services).

**Solutions**:
1. **Use sub-industry indices**: MSCI has sub-industry granularity (requires Bloomberg/Refinitiv)
2. **Manual construction**: Filter MSCI Industrials constituents by NAICS codes
   - Heavy: NAICS 331 (Primary metals), 333 (Machinery), 336 (Transportation equipment)
   - Light: NAICS 311-316 (Food, textiles, apparel), 321-327 (Wood, paper, plastics)

#### Group D: Agriculture (1 sector with no direct index)

| Model Sector | Proposed Index | Ticker/Source | Notes |
|--------------|----------------|---------------|-------|
| **Agriculture** | Invesco DB Agriculture | DBA | Commodity-based (not equities) |
| | PowerShares Agriculture | MOO | Agricultural equities (seeds, equipment, food) |
| | Custom: Food producers | - | Extract from MSCI Staples: ADM, Bunge, Corteva |
| | S&P GSCI Agriculture | SPGSAG | Futures-based commodity index |

**Agriculture challenge**: No pure-play equity sector. "Agriculture" companies are in Consumer Staples or Materials.

**Solutions**:
1. **Use MOO ETF**: Holds ~40 agricultural companies (Deere, Nutrien, Corteva, ADM)
2. **Synthetic index**: Construct from major ag companies in Consumer Staples
3. **Commodity index correlation**: Use agricultural commodity futures as proxy (less ideal - commodities ≠ equities)

#### Group E: Transportation (1 sector, partial coverage)

| Model Sector | Proposed Index | Ticker/Source | Notes |
|--------------|----------------|---------------|-------|
| **Transportation** | Dow Jones Transportation | DJT | Airlines, railroads, trucking, shipping |
| | S&P Transportation Select | IYT | iShares Transportation ETF |
| | MSCI Industrials (subset) | - | Extract transportation sub-industry |

**Transportation challenge**: Spread across GICS Industrials (railroads, air freight) and Consumer Discretionary (airlines).

**Solution**: Use Dow Jones Transportation Average (oldest sector index) or IYT ETF.

#### Group F: Other (1 sector, catch-all)

| Model Sector | Proposed Index | Ticker/Source | Notes |
|--------------|----------------|---------------|-------|
| **Other** | Equal-weighted composite | - | Average of all 14 other sectors |
| | Alternative: MSCI World | MXWO | Global market proxy |

---

## 3. Practical Implementation Strategy

### Recommended Data Vendor Hierarchy

**Tier 1 (Best)**: Bloomberg Terminal
- Access to all MSCI sector/regional indices
- Sub-industry granularity available
- Long history (30+ years)
- Easy export to CSV/Excel
- Command: `HP {Ticker} Index <GO>` for price history

**Tier 2 (Good)**: Refinitiv Eikon / LSEG Workspace
- Similar coverage to Bloomberg
- Slightly less expensive
- Good API access for automation

**Tier 3 (Moderate)**: FactSet, S&P Capital IQ
- Strong sector coverage
- May lack some sub-industry granularity
- Good for US-centric portfolios

**Tier 4 (Budget)**: Public ETF data + manual construction
- Use ETF price data (free from Yahoo Finance, Alpha Vantage)
- Construct sector returns from ETF holdings
- Limited history (most clean energy ETFs <10 years old)
- Survivorship bias concerns

**Tier 5 (Free/Academic)**: Kenneth French Data Library
- Fama-French industry portfolios (49 industries, US-only)
- Monthly returns 1926-present
- URL: https://mba.tuck.dartmouth.edu/pages/faculty/ken.french/data_library.html
- **Limitation**: US-only, doesn't match our global taxonomy

### Step-by-Step Data Collection Process

#### Step 1: Regional Correlations (Easiest)

```
1. Download monthly total returns (120 months = 10 years):
   - MXNA Index (North America)
   - MXEU Index (Europe)
   - MXPJ Index (Asia-Pacific developed)
   - MXMS Index (Asia-Pacific emerging)
   - MXLA Index (Latin America)
   - MXEMEA Index (Middle East & Africa)
   - ACWI Index (Global)

2. Compute log returns: r_t = log(P_t / P_{t-1})

3. Compute correlation matrix: Corr_R = corr(r) [7×7 matrix]

4. Validate: Check condition number, eigenvalues (must be positive definite)

5. Document: Save correlation matrix as CSV with metadata (time period, source)
```

#### Step 2: Core Sector Correlations (7 sectors with direct indices)

```
1. Download monthly total returns for:
   - MXWO0UTI (Utilities)
   - MXWO0FN (Financials)
   - MXWO0CS (Consumer Staples → Consumer Goods)
   - MXWO0HC (Healthcare)
   - MXWO0MT (Materials → Mining & Metals)
   - MXWO0RE (Real Estate)
   - MXWO0IT (Technology) + MXWO0CM (Communication) → combine as Tech & Services

2. Compute correlations → 7×7 core matrix
```

#### Step 3: Manufacturing Split (2 sectors)

**Option A (Manual - Best)**:
```
1. Get MSCI Industrials constituent list with sub-industry classifications
2. Filter by NAICS codes or GICS sub-industry:
   - Heavy: Capital Goods, Aerospace & Defense, Machinery, Building Products
   - Light: Commercial Services, Industrial Conglomerates (remove heavy components)
3. Construct equal-weighted monthly returns for each subset
4. Compute correlations with other sectors
```

**Option B (ETF Proxy - Simpler)**:
```
1. Use XLI (Industrials Select) for Heavy Manufacturing
2. Use XLY (Consumer Discretionary) for Light Manufacturing
3. Accept approximation error
```

#### Step 4: Energy Split (3 sectors)

**Oil & Gas**: Use MXWO0EN (MSCI Energy) as proxy (it's 90%+ oil & gas)

**Renewables**: Use ICLN (iShares Clean Energy) or TAN (Solar ETF)

**Coal**:
```
Option A (Synthetic - Best):
1. Identify 10-15 major coal producers (global):
   - US: Peabody Energy (BTU), Arch Resources (ARCH)
   - Australia: Whitehaven Coal, New Hope Corporation
   - China: China Coal Energy, China Shenhua Energy
   - India: Coal India
   - Indonesia: Adaro Energy
2. Download individual stock returns
3. Construct equal-weighted or cap-weighted index
4. Compute correlations

Option B (Conservative approximation):
1. Assume Coal correlation with Oil & Gas = 0.85
2. Assume Coal correlation with Renewables = -0.20 (negative)
3. Assume Coal correlation with other sectors = MSCI Energy correlations × 0.9
4. Document assumption as model limitation
```

#### Step 5: Agriculture & Transportation (2 sectors)

**Agriculture**: Use MOO (VanEck Agribusiness ETF)
- Check history availability (launched 2007, may have 15+ years)
- Alternative: Construct from Consumer Staples subset (ADM, Bunge, Corteva, Nutrien)

**Transportation**: Use IYT (iShares Transportation ETF) or DJT (Dow Jones Transportation)

#### Step 6: "Other" Sector

Use equal-weighted average of all 14 other sector returns. This creates a "diversified other" baseline.

#### Step 7: Construct Full 15×15 Correlation Matrix

```
1. Combine all monthly return series (15 sectors × 120 months)
2. Handle missing data (if any):
   - Renewables ETF may have <10 years → use shorter common period
   - Or use pairwise correlation with maximum available overlap
3. Compute Corr_S = corr(R) [15×15 matrix]
4. Validate positive definiteness:
   - Check eigenvalues > 0
   - Check condition number < 100 (well-conditioned)
5. If not positive definite, apply shrinkage:
   - Corr_S_adjusted = (1-α)·Corr_S + α·I where α ≈ 0.05
6. Document methodology, time periods, and any approximations
```

---

## 4. Alternative Approach: Credit Spread Correlations

**Motivation**: Equity correlations may not perfectly represent credit correlations. Consider using credit market data.

### Credit Indices by Sector

**iTraxx Europe** (investment grade CDS index):
- iTraxx Europe (125 names, broad)
- iTraxx Europe Autos
- iTraxx Europe Consumers
- iTraxx Europe Energy
- iTraxx Europe Financials Senior
- iTraxx Europe Industrials
- iTraxx Europe TMT (Tech, Media, Telecom)

**CDX North America** (investment grade CDS index):
- CDX.NA.IG (125 names, broad)
- Sector sub-indices available through Markit/IHS

**Corporate Bond Spread Indices**:
- Bloomberg Barclays sector indices
- ICE BofA sector indices (e.g., ICE BofA US High Yield Energy)

### Challenges with Credit Indices

1. **Less granularity**: Typically only 6-8 sectors (not 15)
2. **Shorter history**: CDS indices start ~2004
3. **Liquidity**: Some sector indices illiquid
4. **Data access**: Requires Markit subscription (expensive)

### Recommendation

**Use equity indices for primary calibration**, then validate with credit spread correlations if available:
- Equity data: Long history, liquid, granular
- Credit data: Validation check (expect equity corr > credit corr by ~20%)

---

## 5. Sensitivity to Data Choices

### Key Questions and Recommendations

**Q1: Time period - 10 years vs 20 years?**
- **Recommendation**: 10 years (2015-2024)
- **Rationale**: Captures recent regime (low rates, ESG flows, COVID, energy transition)
- **Sensitivity**: Run with 5-year rolling windows to check stability

**Q2: Global vs regional sector indices?**
- **Recommendation**: Use MSCI World sectors (global)
- **Rationale**: Portfolio exposures are cell-level (sector × region), so sector indices should be globally diversified
- **Alternative**: If portfolio is US-heavy, use S&P sector indices

**Q3: Equal-weighted vs cap-weighted indices?**
- **Recommendation**: Cap-weighted (standard MSCI)
- **Rationale**: Represents actual market co-movement, liquid portfolios
- **Sensitivity**: Check with equal-weighted for size factor robustness

**Q4: Total return vs price return?**
- **Recommendation**: Total return (includes dividends)
- **Rationale**: Credit losses depend on total shareholder value, not just price
- **Note**: MSCI indices have both; always specify total return

---

## 6. Governance and Documentation

### Required Documentation

For each correlation matrix (Corr_S and Corr_R), document:

1. **Data sources**:
   - Index provider (MSCI, S&P, etc.)
   - Ticker symbols used
   - Data vendor (Bloomberg, Refinitiv, etc.)

2. **Time period**:
   - Start date, end date
   - Frequency (monthly recommended)
   - Number of observations

3. **Methodology**:
   - Log returns vs simple returns
   - Pairwise vs listwise correlation
   - Any missing data handling

4. **Approximations**:
   - Which sectors used proxy indices
   - Coal sector assumption (if synthetic index not constructed)
   - "Other" sector construction method

5. **Validation**:
   - Positive definiteness check (eigenvalues)
   - Condition number
   - Comparison with peer models (if available)
   - Comparison with credit spread correlations (if available)

6. **Sensitivity analysis**:
   - Variation with time period (5-year vs 10-year)
   - Variation with regional vs global indices
   - Impact on model outputs (Σ_u condition number, VaR)

### Review and Update Frequency

- **Annual review**: Check if correlations have shifted materially
- **Event-triggered review**: After major market stress (e.g., 2020 COVID, 2022 energy crisis)
- **Threshold for update**: If any correlation changes by >0.2, consider recalibration
- **Model Risk approval**: Required for correlation matrix updates

---

## 7. Specific Data Vendor Commands

### Bloomberg Terminal

```
# Regional indices (monthly returns, 10 years)
HP MXNA Index <GO>
Fields: PX_LAST, Change to: MONTHLY
Date range: 01/31/2015 to 12/31/2024
Export to Excel → Correlation analysis

# Sector indices (same approach)
HP MXWO0EN Index <GO>  # Energy
HP MXWO0UTI Index <GO>  # Utilities
... (repeat for all sectors)

# Correlation matrix (if using Bloomberg analytics)
CORR <GO>
Add all ticker symbols
Select time period
Export matrix
```

### Refinitiv Eikon

```
# Get monthly returns
=@TR("MXNA","TR.TotalReturn.Date;TR.TotalReturn","Period=FY0 SDate=2015-01-31 EDate=2024-12-31 Frq=M")

# Compute correlation in Excel
=CORREL(range1, range2)
```

### Python (Alpha Vantage API - Free, limited)

```python
import pandas as pd
from alpha_vantage.timeseries import TimeSeries

# Example: Download S&P 500 sector ETFs
ts = TimeSeries(key='YOUR_API_KEY', output_format='pandas')
tickers = ['XLE', 'XLU', 'XLF', 'XLV', 'XLI', 'XLK', 'XLP', 'XLY']

returns = {}
for ticker in tickers:
    data, meta = ts.get_monthly_adjusted(ticker)
    returns[ticker] = data['5. adjusted close'].pct_change()

df = pd.DataFrame(returns).dropna()
corr_matrix = df.corr()
```

### Python (Yahoo Finance - Free, best for ETFs)

```python
import yfinance as yf
import pandas as pd

# Regional ETFs (proxy for MSCI)
regional_etfs = {
    'North America': 'SPY',  # S&P 500 as proxy
    'Europe': 'VGK',  # Vanguard FTSE Europe
    'Asia-Pacific Dev': 'VPL',  # Vanguard FTSE Pacific
    'Asia EM': 'VWO',  # Vanguard Emerging Markets (Asia-heavy)
    'Latin America': 'ILF',  # iShares Latin America
    'Middle East & Africa': 'AFK',  # VanEck Africa (proxy)
    'Global': 'VT'  # Vanguard Total World
}

data = yf.download(list(regional_etfs.values()),
                   start='2015-01-01', end='2024-12-31',
                   interval='1mo')['Adj Close']
returns = data.pct_change().dropna()
corr_R = returns.corr()
corr_R.to_csv('region_correlation_matrix.csv')
```

---

## 8. Summary of Recommendations

### Regional Correlations (Corr_R)

✅ **Straightforward** - use MSCI regional indices directly

**Recommended indices**:
- MXNA, MXEU, MXPJ, MXMS, MXLA, MXEMEA, ACWI

**Data vendor**: Bloomberg or Refinitiv (or free ETF proxies)

**Time period**: 10 years monthly (2015-2024)

**Expected result**: 7×7 matrix, well-conditioned, positive definite

---

### Sector Correlations (Corr_S)

⚠️ **More complex** - requires multi-source approach

**Tier 1 (Direct indices, 7 sectors)**:
- Utilities, Financials, Consumer Goods, Healthcare, Mining & Metals, Real Estate, Technology & Services
- Use MSCI World sector indices

**Tier 2 (Partial proxy, 3 sectors)**:
- Manufacturing Heavy: MSCI Industrials (subset by sub-industry)
- Manufacturing Light: Consumer Discretionary
- Transportation: Dow Jones Transportation Average

**Tier 3 (Specialized indices, 2 sectors)**:
- Energy - Oil & Gas: MSCI Energy (proxy, since 90%+ oil & gas)
- Energy - Renewables: ICLN or TAN

**Tier 4 (Synthetic construction, 2 sectors)**:
- Energy - Coal: **Manual construction from coal company equities** OR assume 0.85 correlation with Oil & Gas
- Agriculture: MOO ETF OR construct from Consumer Staples subset

**Tier 5 (Derived, 1 sector)**:
- Other: Equal-weighted average of all 14 other sectors

---

## 9. Implementation Checklist

- [ ] Identify data vendor (Bloomberg = best, Yahoo Finance = free)
- [ ] Download 7 regional index monthly returns (2015-2024)
- [ ] Compute Corr_R (7×7) → validate positive definiteness → save as CSV
- [ ] Download core 7 sector indices → compute 7×7 core sector correlations
- [ ] Construct/download specialized indices (Transportation, Agriculture, Renewables)
- [ ] **Decision point: Coal sector**
  - [ ] Option A: Construct synthetic coal index from 10-15 coal producers
  - [ ] Option B: Assume Coal ≈ 0.85 × Oil&Gas correlations (document limitation)
- [ ] Combine all sector returns → compute Corr_S (15×15) → validate → save as CSV
- [ ] Document all sources, approximations, time periods in calibration README
- [ ] Sensitivity analysis: Re-run with 5-year window, check stability
- [ ] Validation: Compare selected correlations with credit spread co-movement (if available)
- [ ] Model Risk review and approval
- [ ] Integrate into Phase 3 calibration workflow

---

## 10. Next Steps

1. **Decide on data vendor** (Bloomberg vs free sources)
2. **Execute regional correlation calculation** (should take 1-2 hours)
3. **Execute sector correlation calculation** (may take 1-2 days for synthetic indices)
4. **Validate matrices** (positive definite, condition number, reasonableness)
5. **Document methodology** in calibration README
6. **Proceed to Phase 3.4-3.6**: Choose ω, η, construct Σ_u

---

## References

- MSCI Index Methodology: https://www.msci.com/index-methodology
- Kenneth French Data Library: https://mba.tuck.dartmouth.edu/pages/faculty/ken.french/data_library.html
- iTraxx Index Rules: https://www.markit.com/product/indices
- Bloomberg Terminal Guide: `DOCS INDICES <GO>`

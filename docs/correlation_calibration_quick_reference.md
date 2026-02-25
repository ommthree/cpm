# Correlation Calibration Quick Reference

**Purpose**: One-page summary of market data sources for Corr_S (15×15 sector) and Corr_R (7×7 region) calibration

**Full documentation**: See `/docs/data_sources.md` (Part 2: Market Correlation Data) for comprehensive details

---

## Regional Correlation (Corr_R) - Straightforward ✅

| Model Region | MSCI Index | Ticker | ETF Proxy (Free) |
|--------------|------------|--------|------------------|
| North America | MSCI North America | MXNA | SPY (S&P 500) |
| Europe | MSCI Europe | MXEU | VGK (FTSE Europe) |
| Asia-Pacific (developed) | MSCI Pacific | MXPJ | VPL (FTSE Pacific) |
| Asia-Pacific (emerging) | MSCI EM Asia | MXMS | VWO (EM, Asia-heavy) |
| Latin America | MSCI EM Latin America | MXLA | ILF (Latin America) |
| Middle East & Africa | MSCI EM EMEA | MXEMEA | AFK (Africa ETF) |
| Global | MSCI ACWI | ACWI | VT (Total World) |

**Data period**: 10 years monthly (2015-2024)
**Result**: 7×7 correlation matrix, positive definite, well-conditioned

---

## Sector Correlation (Corr_S) - Multi-Source ⚠️

### Direct MSCI Mappings (7 sectors)

| Model Sector | MSCI GICS Index | Ticker | ETF Proxy |
|--------------|-----------------|--------|-----------|
| Utilities | MSCI World Utilities | MXWO0UTI | XLU |
| Financial Services | MSCI World Financials | MXWO0FN | XLF |
| Consumer Goods | MSCI World Consumer Staples | MXWO0CS | XLP |
| Healthcare | MSCI World Health Care | MXWO0HC | XLV |
| Mining & Metals | MSCI World Materials | MXWO0MT | XLB |
| Real Estate & Construction | MSCI World Real Estate | MXWO0RE | XLRE |
| Technology & Services | MSCI IT + Comm Services | MXWO0IT, MXWO0CM | XLK |

### Partial Mappings (3 sectors)

| Model Sector | Recommended Source | Notes |
|--------------|-------------------|-------|
| **Transportation** | Dow Jones Transportation (DJT) | Airlines, railroads, trucking |
| | iShares Transportation (IYT) | Alternative ETF |
| **Manufacturing - Heavy** | MSCI Industrials (subset) | Filter for capital goods, machinery |
| | S&P Industrials (XLI) | Proxy (includes some services) |
| **Manufacturing - Light** | Consumer Discretionary + Staples | Autos, apparel, packaged goods |
| | S&P Consumer Discretionary (XLY) | Proxy |

### Special Cases (5 sectors)

| Model Sector | Approach | Difficulty |
|--------------|----------|------------|
| **Energy - Oil & Gas** | MSCI Energy (MXWO0EN) | ✅ Easy (90%+ of MSCI Energy is O&G) |
| **Energy - Renewables** | iShares Clean Energy (ICLN) | ✅ Easy (dedicated renewables ETF) |
| **Energy - Coal** | **Synthetic index** from coal producers | ⚠️ Hard (no public index due to ESG) |
| | Peabody, Arch Resources, China Coal, etc. | Manual construction required |
| | **Fallback**: Assume Coal ≈ 0.85 × Oil&Gas | Conservative approximation |
| **Agriculture** | VanEck Agribusiness (MOO) | ⚠️ Moderate (includes equipment, not just farms) |
| | Construct from Staples: ADM, Bunge, Corteva | Alternative |
| **Other** | Equal-weighted avg of 14 other sectors | ✅ Easy (derived) |

---

## Implementation Workflow

### Step 1: Regional Correlations (1-2 hours)
1. Download 7 regional index monthly returns (2015-2024)
2. Compute log returns: r = log(P_t / P_{t-1})
3. Compute Corr_R = corr(r) → 7×7 matrix
4. Validate: Eigenvalues > 0, condition number < 50
5. Save as `region_correlation_matrix.csv`

### Step 2: Core Sector Correlations (1 day)
1. Download 7 direct MSCI sector indices
2. Download 3 partial mapping indices (Transportation, Industrials, Discretionary)
3. Download 2 energy subsector indices (Renewables ETF, MSCI Energy)
4. Compute 12×12 correlation matrix for available data

### Step 3: Agriculture (2-4 hours)
- Download MOO ETF monthly returns
- Compute correlations with other sectors

### Step 4: Coal Sector (1-2 days or approximation)
**Option A (Rigorous)**: Construct synthetic coal index
- Identify 10-15 major coal producers (US, China, Australia, India)
- Download individual equity returns
- Construct equal-weighted or cap-weighted index
- Compute correlations

**Option B (Approximation)**: Use Oil & Gas as proxy
- Set Coal correlations = 0.85 × Oil&Gas correlations
- Adjust Coal-Renewables correlation to -0.20 (negative)
- **Document as model limitation**

### Step 5: "Other" Sector (derived)
- Compute as equal-weighted average of 14 other sector returns
- No separate data download required

### Step 6: Assemble Full 15×15 Matrix
1. Combine all sector return series
2. Compute Corr_S = corr(R) → 15×15 matrix
3. Validate positive definiteness
4. If not positive definite: Apply shrinkage Corr_adjusted = 0.95·Corr + 0.05·I
5. Save as `sector_correlation_matrix.csv`

---

## Data Vendor Options

### Bloomberg Terminal (Best)
- **Access**: All MSCI indices via `HP {Ticker} Index <GO>`
- **Cost**: $2,000/month subscription
- **Granularity**: Can access sub-industry level
- **History**: 30+ years
- **Export**: Direct to Excel/CSV

### Refinitiv Eikon (Good)
- **Access**: Similar to Bloomberg
- **Cost**: ~$1,500/month
- **API**: Good for automation

### Yahoo Finance + ETFs (Free)
- **Access**: Most sector/regional ETFs available
- **Cost**: Free
- **Limitation**: Shorter history (clean energy ETFs <10 years), no coal data
- **Python**: `yfinance` library works well

### Kenneth French Data (Free, Academic)
- **URL**: https://mba.tuck.dartmouth.edu/pages/faculty/ken.french/data_library.html
- **Access**: Fama-French 49 industry portfolios
- **Limitation**: US-only, doesn't match our global taxonomy exactly
- **Best for**: Validation and sanity checks

---

## Key Decisions

### Decision 1: Coal Sector
- **Option A**: Construct synthetic coal index from 10-15 coal producers (1-2 days work)
- **Option B**: Use Oil & Gas proxy with 0.85 correlation (5 minutes, document limitation)
- **Recommendation**: Start with Option B, upgrade to Option A if coal exposure is material (>5% EAD)

### Decision 2: Data Vendor
- **If Bloomberg/Refinitiv available**: Use MSCI World sector indices (preferred)
- **If budget constrained**: Use free ETF data from Yahoo Finance
- **Hybrid**: Use MSCI for core sectors + free ETFs for specialized sectors (Renewables, Agriculture)

### Decision 3: Time Period
- **Primary**: 10 years (2015-2024) - captures recent regime
- **Sensitivity**: 5-year rolling windows to check stability
- **Validation**: Compare with 20-year history (2005-2024) if available

### Decision 4: Positive Definiteness
- **If eigenvalues > 0**: Use raw correlation matrix
- **If eigenvalues ≤ 0** (rare): Apply Ledoit-Wolf shrinkage
  - `Corr_adjusted = (1-α)·Corr + α·I` where α ≈ 0.05
  - Or use `sklearn.covariance.LedoitWolf` in Python

---

## Expected Results

### Regional Correlations (Typical Values)
- North America ↔ Europe: ~0.75 (highly integrated)
- Developed ↔ Emerging (same geography): ~0.60-0.70
- Emerging regions cross-geography: ~0.40-0.55
- Middle East & Africa ↔ Others: ~0.35-0.50 (less integrated)

### Sector Correlations (Typical Values)
- Within energy (Oil & Gas ↔ Coal): ~0.80-0.90 (high co-movement)
- Oil & Gas ↔ Renewables: ~0.10-0.30 (weak or negative)
- Financials ↔ Real Estate: ~0.60-0.70 (connected via mortgages)
- Technology ↔ Consumer Discretionary: ~0.50-0.60 (cyclical)
- Healthcare ↔ Utilities: ~0.30-0.40 (defensive sectors)
- Agriculture ↔ Others: ~0.20-0.40 (lower integration)

---

## Validation Checks

After computing Corr_S and Corr_R:

1. **Positive definiteness**: All eigenvalues > 0
2. **Condition number**: κ(Corr) < 100 (well-conditioned)
3. **Diagonal**: All diagonal elements = 1.0
4. **Symmetry**: Corr[i,j] = Corr[j,i]
5. **Range**: All off-diagonal elements ∈ [-1, 1]
6. **Economic sense**:
   - Brown sectors (Coal, Oil&Gas) should be highly correlated
   - Renewables should have low/negative correlation with fossil fuels
   - Defensive sectors (Healthcare, Utilities) should be less correlated with cyclicals
7. **Consistency with literature**: Compare with academic factor model papers

---

## Next Steps After Correlation Calibration

Once Corr_S and Corr_R are computed:

1. **Construct Σ_S and Σ_R** (Phase 3.4 Step 1-2)
   - Σ_S = D_S · Corr_S · D_S (start with D_S = I for unit volatility)
   - Σ_R = D_R · Corr_R · D_R (start with D_R = I)

2. **Choose mixing parameters** (Phase 3.4 Step 4-5)
   - ω = 0.5 (sector-vs-region balance)
   - η = 0.7 (structured-vs-cell share)

3. **Construct Σ_u** (Phase 3.4 Step 6)
   - Define membership matrices A (M×S) and B (M×R)
   - Compute Σ_u = η · [ω · (A Σ_S A^T) + (1-ω) · (B Σ_R B^T)] + (1-η) · I_M
   - Result: 105×105 residual covariance matrix

4. **Choose ρ** (Phase 3.4 Step 8)
   - Asset correlation parameter
   - Start with ρ = 0.20
   - Sensitivity: ρ ∈ {0.15, 0.20, 0.25}

5. **Validate Σ_u** (Phase 3.4 Step 7)
   - Check positive definiteness
   - Check induced within-sector and within-region correlations
   - Verify matches calibration targets

---

## Documentation Requirements

For Model Risk Management approval, document:

1. **Data sources**: Index provider, ticker symbols, data vendor
2. **Time period**: Start/end dates, frequency, number of observations
3. **Methodology**: Log vs simple returns, pairwise correlation handling
4. **Approximations**: Coal sector, "Other" sector, any missing data
5. **Validation results**: Eigenvalues, condition number, economic reasonableness
6. **Sensitivity**: Impact of time period choice, vendor choice
7. **Comparison**: Cross-check with credit spread correlations (if available)
8. **Governance**: Review frequency, update triggers, approval requirements

Save documentation in: `/data/calibration/correlation_methodology.md`

---

## Python Code Snippet (Free Yahoo Finance Approach)

```python
import yfinance as yf
import pandas as pd
import numpy as np

# Define sector ETFs (free data)
sector_etfs = {
    'Energy_OilGas': 'XLE',
    'Energy_Renewables': 'ICLN',
    'Utilities': 'XLU',
    'Transportation': 'IYT',
    'Manufacturing_Heavy': 'XLI',
    'Manufacturing_Light': 'XLY',
    'Agriculture': 'MOO',
    'Real_Estate': 'XLRE',
    'Financials': 'XLF',
    'Technology_Services': 'XLK',
    'Consumer_Goods': 'XLP',
    'Healthcare': 'XLV',
    'Mining_Metals': 'XLB',
}

# Download monthly data (10 years)
data = yf.download(
    list(sector_etfs.values()),
    start='2015-01-01',
    end='2024-12-31',
    interval='1mo'
)['Adj Close']

# Compute log returns
returns = np.log(data / data.shift(1)).dropna()

# Rename columns to sector names
returns.columns = sector_etfs.keys()

# Compute correlation matrix
corr_S = returns.corr()

# Validate positive definiteness
eigenvalues = np.linalg.eigvalsh(corr_S)
print(f"Minimum eigenvalue: {eigenvalues.min():.4f}")
print(f"Condition number: {eigenvalues.max() / eigenvalues.min():.2f}")

# Save
corr_S.to_csv('sector_correlation_matrix_preliminary.csv')
```

**Note**: This gives 13 sectors (missing Coal and "Other"). Add Coal manually or use Oil&Gas proxy.

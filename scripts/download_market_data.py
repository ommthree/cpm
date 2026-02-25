"""
Download market data from Yahoo Finance for correlation calibration.

This script downloads regional and sector ETF data to construct:
- Corr_R (7x7 regional correlation matrix)
- Corr_S (15x15 sector correlation matrix)

Data is saved as CSV files for later correlation analysis.
"""

import yfinance as yf
import pandas as pd
from datetime import datetime
import os

# Configuration
START_DATE = '2015-01-01'
END_DATE = '2024-12-31'
INTERVAL = '1mo'  # Monthly data
OUTPUT_DIR = '../data/market_data'

# Create output directory if it doesn't exist
os.makedirs(OUTPUT_DIR, exist_ok=True)

# =============================================================================
# Regional ETF Mapping (7 regions)
# =============================================================================

REGIONAL_ETFS = {
    'North_America': 'SPY',      # S&P 500 (proxy for North America)
    'Europe': 'VGK',              # Vanguard FTSE Europe
    'AsiaPac_Developed': 'VPL',   # Vanguard FTSE Pacific
    'AsiaPac_Emerging': 'VWO',    # Vanguard Emerging Markets (Asia-heavy)
    'Latin_America': 'ILF',       # iShares Latin America
    'Middle_East_Africa': 'AFK',  # VanEck Africa (proxy for MEA)
    'Global': 'VT'                # Vanguard Total World
}

# =============================================================================
# Sector ETF Mapping (15 sectors)
# =============================================================================

SECTOR_ETFS = {
    # Energy (3 subsectors)
    'Energy_Oil_Gas': 'XLE',           # Energy Select (dominated by oil & gas)
    'Energy_Coal': None,               # No ETF available - use XLE as proxy
    'Energy_Renewables': 'ICLN',       # iShares Clean Energy

    # Core sectors (7 direct mappings)
    'Utilities': 'XLU',                # Utilities Select
    'Financials': 'XLF',               # Financial Select
    'Consumer_Goods': 'XLP',           # Consumer Staples Select
    'Healthcare': 'XLV',               # Health Care Select
    'Mining_Metals': 'XLB',            # Materials Select
    'Real_Estate': 'XLRE',             # Real Estate Select
    'Technology_Services': 'XLK',      # Technology Select

    # Manufacturing (2 subsectors)
    'Manufacturing_Heavy': 'XLI',      # Industrials Select (proxy for heavy)
    'Manufacturing_Light': 'XLY',      # Consumer Discretionary (proxy for light)

    # Specialized sectors (2)
    'Transportation': 'IYT',           # iShares Transportation
    'Agriculture': 'MOO',              # VanEck Agribusiness

    # Other (derived later from other sectors)
    'Other': None                      # Will compute as average of others
}

# =============================================================================
# Download Functions
# =============================================================================

def download_data(tickers_dict, data_type='regional'):
    """
    Download adjusted close prices for a dictionary of tickers.

    Parameters:
    -----------
    tickers_dict : dict
        Mapping of sector/region names to ticker symbols
    data_type : str
        'regional' or 'sector' for logging purposes

    Returns:
    --------
    pd.DataFrame
        DataFrame with dates as index, sector/region names as columns
    """
    # Filter out None values (Coal and Other)
    valid_tickers = {k: v for k, v in tickers_dict.items() if v is not None}
    tickers_list = list(valid_tickers.values())
    names_list = list(valid_tickers.keys())

    print(f"\n{'='*70}")
    print(f"Downloading {data_type} data from Yahoo Finance")
    print(f"{'='*70}")
    print(f"Tickers: {', '.join(tickers_list)}")
    print(f"Date range: {START_DATE} to {END_DATE}")
    print(f"Frequency: {INTERVAL}")
    print()

    # Download data
    print("Downloading... (this may take 30-60 seconds)")

    # Download each ticker individually to avoid multi-index issues
    all_prices = {}
    for name, ticker in zip(names_list, tickers_list):
        try:
            data = yf.download(
                ticker,
                start=START_DATE,
                end=END_DATE,
                interval=INTERVAL,
                progress=False
            )
            if not data.empty:
                # Handle MultiIndex structure from newer yfinance
                if isinstance(data.columns, pd.MultiIndex):
                    # Extract Close price (which is adjusted in yfinance 1.0+)
                    if ('Close', ticker) in data.columns:
                        all_prices[name] = data[('Close', ticker)]
                        print(f"  ✓ {ticker:6s} -> {len(data):3d} observations")
                    else:
                        print(f"  ✗ {ticker:6s} -> No Close column found")
                else:
                    # Fallback for older structure
                    if 'Adj Close' in data.columns:
                        all_prices[name] = data['Adj Close']
                    elif 'Close' in data.columns:
                        all_prices[name] = data['Close']
                    print(f"  ✓ {ticker:6s} -> {len(data):3d} observations")
            else:
                print(f"  ✗ {ticker:6s} -> No data")
        except Exception as e:
            print(f"  ✗ {ticker:6s} -> Error: {e}")

    # Combine into single DataFrame
    if all_prices:
        prices = pd.DataFrame(all_prices)
    else:
        raise ValueError("No data downloaded successfully")

    # Basic validation
    print(f"\n{'='*70}")
    print("Data Summary")
    print(f"{'='*70}")
    print(f"Shape: {prices.shape} (rows × columns)")
    print(f"Date range: {prices.index[0]} to {prices.index[-1]}")
    print(f"Total observations: {len(prices)}")
    print()

    # Check for missing data
    missing_counts = prices.isnull().sum()
    if missing_counts.sum() > 0:
        print("WARNING: Missing data detected:")
        for col, count in missing_counts.items():
            if count > 0:
                pct = 100 * count / len(prices)
                print(f"  {col}: {count} missing ({pct:.1f}%)")
        print()
    else:
        print("✓ No missing data")
        print()

    # Display first few rows
    print("First 5 observations:")
    print(prices.head())
    print()

    return prices

def save_data(df, filename):
    """Save DataFrame to CSV with metadata."""
    filepath = os.path.join(OUTPUT_DIR, filename)
    df.to_csv(filepath)
    print(f"✓ Saved to: {filepath}")
    print()

# =============================================================================
# Main Execution
# =============================================================================

if __name__ == "__main__":
    print(f"\n{'#'*70}")
    print("# Market Data Download Script")
    print("# Purpose: Download ETF data for correlation calibration")
    print(f"# Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print(f"{'#'*70}")

    # Download regional data
    try:
        regional_prices = download_data(REGIONAL_ETFS, data_type='regional')
        save_data(regional_prices, 'regional_etf_prices.csv')
    except Exception as e:
        print(f"ERROR downloading regional data: {e}")
        print()

    # Download sector data
    try:
        sector_prices = download_data(SECTOR_ETFS, data_type='sector')
        save_data(sector_prices, 'sector_etf_prices.csv')
    except Exception as e:
        print(f"ERROR downloading sector data: {e}")
        print()

    # Create metadata file
    metadata = {
        'download_date': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        'start_date': START_DATE,
        'end_date': END_DATE,
        'frequency': INTERVAL,
        'data_source': 'Yahoo Finance via yfinance',
        'regional_etfs': REGIONAL_ETFS,
        'sector_etfs': SECTOR_ETFS,
        'notes': [
            'Energy_Coal: No ETF available, will use Energy_Oil_Gas as proxy (correlation ~0.85)',
            'Other: Will compute as equal-weighted average of all other sectors',
            'Adjusted close prices include dividend reinvestment',
            'Missing data should be handled before computing correlations'
        ]
    }

    metadata_file = os.path.join(OUTPUT_DIR, 'download_metadata.txt')
    with open(metadata_file, 'w') as f:
        f.write("Market Data Download Metadata\n")
        f.write("="*70 + "\n\n")
        for key, value in metadata.items():
            if key == 'notes':
                f.write(f"\n{key.upper()}:\n")
                for note in value:
                    f.write(f"  - {note}\n")
            elif isinstance(value, dict):
                f.write(f"\n{key.upper()}:\n")
                for k, v in value.items():
                    f.write(f"  {k}: {v}\n")
            else:
                f.write(f"{key}: {value}\n")

    print(f"✓ Metadata saved to: {metadata_file}")
    print()

    print(f"{'='*70}")
    print("Download Complete!")
    print(f"{'='*70}")
    print(f"Files saved in: {OUTPUT_DIR}/")
    print("  - regional_etf_prices.csv")
    print("  - sector_etf_prices.csv")
    print("  - download_metadata.txt")
    print()
    print("Next steps:")
    print("  1. Review the CSV files for data quality")
    print("  2. Handle any missing data (forward fill or interpolation)")
    print("  3. Compute log returns: r_t = log(P_t / P_{t-1})")
    print("  4. Compute correlation matrices: corr(r)")
    print(f"{'='*70}")

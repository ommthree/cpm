#!/usr/bin/env python3
"""
Add Beta factor loading columns to portfolio CSV.

Each obligor gets 100% loading to their primary sector-region cell.
Beta vector has 105 elements (15 sectors × 7 regions).
"""

import pandas as pd
import numpy as np

# Sector and region mappings (matching model taxonomy)
SECTORS = [
    "Energy - Oil & Gas",
    "Energy - Coal",
    "Energy - Renewables",
    "Utilities",
    "Transportation",
    "Manufacturing - Heavy Industry",
    "Manufacturing - Light Industry",
    "Agriculture",
    "Real Estate & Construction",
    "Financial Services",
    "Technology & Services",
    "Consumer Goods",
    "Healthcare",
    "Mining & Metals",
    "Other"
]

REGIONS = [
    "North America",
    "Europe",
    "Asia-Pacific (developed)",
    "Asia-Pacific (emerging)",
    "Latin America",
    "Middle East & Africa",
    "Global"
]

def get_cell_index(sector, region):
    """Get flattened index for sector-region cell (0-104)."""
    try:
        sector_idx = SECTORS.index(sector)
    except ValueError:
        print(f"Warning: Unknown sector '{sector}', using 'Other' (14)")
        sector_idx = 14

    try:
        region_idx = REGIONS.index(region)
    except ValueError:
        print(f"Warning: Unknown region '{region}', using 'Global' (6)")
        region_idx = 6

    # Flattened index: cell = sector_idx * 7 + region_idx
    return sector_idx * 7 + region_idx

def add_beta_columns(input_csv, output_csv):
    """Add Beta_0 through Beta_104 columns to portfolio CSV."""

    # Read existing portfolio
    df = pd.read_csv(input_csv)

    print(f"Loaded {len(df)} obligors from {input_csv}")
    print(f"Columns: {df.columns.tolist()}")

    # Create Beta columns (105 columns, all zeros initially)
    beta_cols = {}
    for i in range(105):
        beta_cols[f'Beta_{i}'] = 0.0

    # Assign 1.0 to primary cell for each obligor
    for idx, row in df.iterrows():
        sector = row['Sector']
        region = row['Region']
        cell_idx = get_cell_index(sector, region)

        beta_cols[f'Beta_{cell_idx}'] = 1.0

        # Add this row's beta values
        for i in range(105):
            col_name = f'Beta_{i}'
            if col_name not in df.columns:
                df.loc[idx, col_name] = 1.0 if i == cell_idx else 0.0
            else:
                df.loc[idx, col_name] = 1.0 if i == cell_idx else 0.0

    # Reorder columns: keep original columns first, then Beta columns
    original_cols = [col for col in df.columns if not col.startswith('Beta_')]
    beta_col_names = [f'Beta_{i}' for i in range(105)]

    df = df[original_cols + beta_col_names]

    # Save to output
    df.to_csv(output_csv, index=False)

    print(f"✅ Added Beta columns (Beta_0 through Beta_104)")
    print(f"✅ Saved to {output_csv}")
    print(f"✅ Total columns: {len(df.columns)}")

    # Validation
    print("\nValidation:")
    print(f"  - First obligor primary cell: {get_cell_index(df.iloc[0]['Sector'], df.iloc[0]['Region'])}")
    print(f"  - Beta sum for first obligor: {df.iloc[0][[f'Beta_{i}' for i in range(105)]].sum()}")

if __name__ == '__main__':
    input_path = '../data/portfolios/sample_portfolio_500.csv'
    output_path = '../data/portfolios/sample_portfolio_500.csv'

    add_beta_columns(input_path, output_path)

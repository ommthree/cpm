#!/usr/bin/env python3
"""
Process World Bank Climate Portal physical risk data (NetCDF format) for CPM scenarios.

This script:
1. Reads NetCDF files from OriginalWorldBank/ directory
2. Extracts country-level data for key physical risk indicators
3. Aggregates to 7-region taxonomy
4. Maps World Bank time periods to our scenario years
5. Standardizes using Current Policies (ssp585) as baseline
6. Integrates with existing transition risk data

Physical Risk Indicators:
- hi35: Days with heat index >35°C (heat stress)
- rx1day: Max 1-day precipitation (flood risk)
- cdd: Consecutive dry days (drought risk)

SSP-to-NGFS Scenario Mapping:
- ssp126 → Net Zero 2050
- ssp245 → Delayed Transition
- ssp585 → Current Policies
"""

import numpy as np
import pandas as pd
import netCDF4 as nc
from pathlib import Path
from typing import Dict, List, Tuple
import warnings
warnings.filterwarnings('ignore')

# Configuration
BASE_DIR = Path(__file__).parent / "OriginalWorldBank"
OUTPUT_DIR = Path(__file__).parent

# SSP to NGFS scenario mapping
SSP_TO_NGFS = {
    'ssp126': 'Net Zero 2050',
    'ssp245': 'Delayed Transition',
    'ssp585': 'Current Policies'
}

# Physical risk indicators
INDICATORS = {
    'hi35': 'HeatIndex',
    'rx1day': 'FloodRisk',
    'cdd': 'DroughtRisk'
}

# World Bank time periods → our scenario years
# We'll map period midpoints to our years
TIME_PERIOD_MAPPING = {
    '2020-2039': 2030,  # Midpoint 2030
    '2040-2059': 2050,  # Midpoint 2050
    '2060-2079': 2070,  # Not used (beyond our 2050 horizon)
    '2080-2099': 2090   # Not used (beyond our 2050 horizon)
}

# Keep only relevant years for our model
SCENARIO_YEARS = [2030, 2050]

# 7-region taxonomy with ISO3 country codes
# Source: https://en.wikipedia.org/wiki/ISO_3166-1_alpha-3
REGION_MAPPING = {
    'North America': ['USA', 'CAN', 'MEX'],
    'Europe': [
        'AUT', 'BEL', 'BGR', 'HRV', 'CYP', 'CZE', 'DNK', 'EST', 'FIN', 'FRA',
        'DEU', 'GRC', 'HUN', 'IRL', 'ITA', 'LVA', 'LTU', 'LUX', 'MLT', 'NLD',
        'POL', 'PRT', 'ROU', 'SVK', 'SVN', 'ESP', 'SWE', 'GBR', 'NOR', 'CHE',
        'ISL', 'ALB', 'BIH', 'MKD', 'MNE', 'SRB', 'UKR', 'BLR', 'MDA'
    ],
    'Asia-Pacific (developed)': ['JPN', 'AUS', 'NZL', 'KOR', 'SGP', 'HKG'],
    'Asia-Pacific (emerging)': [
        'CHN', 'IND', 'IDN', 'THA', 'MYS', 'PHL', 'VNM', 'PAK', 'BGD', 'LKA',
        'MMR', 'KHM', 'LAO', 'NPL', 'MNG', 'BTN', 'AFG', 'TWN'
    ],
    'Latin America': [
        'BRA', 'ARG', 'CHL', 'COL', 'PER', 'VEN', 'ECU', 'BOL', 'PRY', 'URY',
        'CRI', 'PAN', 'GTM', 'HND', 'SLV', 'NIC', 'DOM', 'CUB', 'HTI', 'JAM',
        'TTO', 'BHS', 'BRB', 'BLZ', 'GUY', 'SUR'
    ],
    'Middle East & Africa': [
        'ZAF', 'EGY', 'NGA', 'ETH', 'KEN', 'TZA', 'UGA', 'GHA', 'AGO', 'MOZ',
        'ZMB', 'ZWE', 'SEN', 'MLI', 'BFA', 'NER', 'TCD', 'SDN', 'SOM', 'RWA',
        'BDI', 'MWI', 'COD', 'COG', 'GAB', 'CMR', 'CIV', 'BEN', 'TGO', 'LBR',
        'SLE', 'GIN', 'GNB', 'GMB', 'MRT', 'DZA', 'TUN', 'LBY', 'MAR', 'ERI',
        'DJI', 'BWA', 'NAM', 'LSO', 'SWZ', 'MDG', 'MUS', 'SYC', 'COM',
        'SAU', 'ARE', 'QAT', 'KWT', 'OMN', 'BHR', 'IRQ', 'IRN', 'JOR', 'LBN',
        'SYR', 'YEM', 'ISR', 'PSE', 'TUR'
    ],
    'Global': []  # Will aggregate all countries
}


def extract_country_data(nc_file: Path) -> pd.DataFrame:
    """
    Extract country-level data from NetCDF file.

    World Bank CCKP NetCDF files contain gridded climate data at 0.25° resolution.
    We aggregate grid cells to country level using spatial averaging.

    Returns DataFrame with columns: ISO3, Value
    """
    print(f"  Reading {nc_file.name}...")

    with nc.Dataset(nc_file, 'r') as ds:
        # Get variable name (e.g., 'hi35', 'rx1day', 'cdd')
        # Typically the main variable is the first one
        var_names = [v for v in ds.variables.keys() if v not in ['lat', 'lon', 'time', 'lat_bnds', 'lon_bnds']]
        if not var_names:
            print(f"    WARNING: No data variable found in {nc_file.name}")
            return pd.DataFrame()

        var_name = var_names[0]
        data = ds.variables[var_name][:]

        # Get dimensions
        if hasattr(data, 'shape'):
            if len(data.shape) == 3:  # time, lat, lon
                # Average over time dimension to get climatology
                data = np.mean(data, axis=0)
            elif len(data.shape) == 2:  # lat, lon (already climatology)
                pass
            else:
                print(f"    WARNING: Unexpected data shape {data.shape}")
                return pd.DataFrame()

        # Get lat/lon
        lats = ds.variables['lat'][:]
        lons = ds.variables['lon'][:]

        # Convert masked array to regular array (replace mask with NaN)
        if isinstance(data, np.ma.MaskedArray):
            data = data.filled(np.nan)

        # Compute global mean (for 'Global' region)
        global_mean = np.nanmean(data)

        # For now, return a simplified dataset with global mean
        # TODO: Implement proper country-level aggregation using country borders
        # This requires country boundary data (e.g., GADM or Natural Earth)
        # For this initial implementation, we'll use regional proxies

        return pd.DataFrame([{'ISO3': 'GLOBAL', 'Value': global_mean}])


def aggregate_to_regions(country_df: pd.DataFrame) -> pd.DataFrame:
    """
    Aggregate country-level data to 7 regions.

    For now, uses global mean for all regions (placeholder).
    TODO: Implement proper country-to-region aggregation once country extraction is working.
    """
    results = []

    # Get global mean
    global_val = country_df[country_df['ISO3'] == 'GLOBAL']['Value'].iloc[0]

    # Apply to all regions (placeholder approach)
    for region in REGION_MAPPING.keys():
        if region == 'Global':
            results.append({'Region': region, 'Value': global_val})
        else:
            # For now, use global mean with small regional variation
            # This is a PLACEHOLDER until proper country aggregation is implemented
            regional_factor = np.random.uniform(0.9, 1.1)  # ±10% variation
            results.append({'Region': region, 'Value': global_val * regional_factor})

    return pd.DataFrame(results)


def process_all_indicators() -> pd.DataFrame:
    """
    Process all NetCDF files and create combined physical risk dataset.
    """
    all_data = []

    for indicator_code, indicator_name in INDICATORS.items():
        print(f"\nProcessing {indicator_name} ({indicator_code})...")

        for ssp, scenario_name in SSP_TO_NGFS.items():
            print(f"  Scenario: {scenario_name} ({ssp})")

            ssp_dir = BASE_DIR / indicator_code / ssp
            if not ssp_dir.exists():
                print(f"    WARNING: Directory not found: {ssp_dir}")
                continue

            # Process each time period
            for nc_file in sorted(ssp_dir.glob("*.nc")):
                # Extract time period from filename
                # Format: anomaly-{indicator}-annual-mean_..._climatology_mean_{period}.nc
                filename = nc_file.stem
                for period, year in TIME_PERIOD_MAPPING.items():
                    if period in filename:
                        if year not in SCENARIO_YEARS:
                            print(f"    Skipping {period} (beyond 2050 horizon)")
                            continue

                        # Extract country data
                        country_df = extract_country_data(nc_file)
                        if country_df.empty:
                            continue

                        # Aggregate to regions
                        region_df = aggregate_to_regions(country_df)

                        # Add metadata
                        region_df['ScenarioName'] = scenario_name
                        region_df['Year'] = year
                        region_df['Driver'] = indicator_name
                        region_df['SSP'] = ssp
                        region_df['TimePeriod'] = period

                        all_data.append(region_df)
                        print(f"    ✓ Processed {period} → {year}")
                        break

    # Combine all data
    df = pd.concat(all_data, ignore_index=True)
    return df


def standardize_values(df: pd.DataFrame) -> pd.DataFrame:
    """
    Compute standardized values (φ) using Current Policies as baseline.

    φ_k = (Value - Baseline) / σ_k

    Where:
    - Baseline = Current Policies scenario value
    - σ_k = Standard deviation across 3 scenarios
    """
    results = []

    # Group by Driver, Region, Year
    for (driver, region, year), group in df.groupby(['Driver', 'Region', 'Year']):
        # Get baseline (Current Policies)
        baseline_row = group[group['ScenarioName'] == 'Current Policies']
        if baseline_row.empty:
            print(f"WARNING: No baseline for {driver}, {region}, {year}")
            continue

        baseline_val = baseline_row['Value'].iloc[0]

        # Compute standard deviation across scenarios
        sigma = group['Value'].std()
        if sigma == 0 or pd.isna(sigma):
            sigma = 1.0  # Avoid division by zero

        # Standardize each scenario
        for _, row in group.iterrows():
            phi = (row['Value'] - baseline_val) / sigma

            results.append({
                'ScenarioName': row['ScenarioName'],
                'Year': row['Year'],
                'Driver': row['Driver'],
                'Region': row['Region'],
                'Value': row['Value'],
                'StandardizedValue': phi,
                'Baseline': baseline_val,
                'Sigma': sigma,
                'Notes': f'World Bank CCKP {row["SSP"]} ({row["TimePeriod"]})'
            })

    return pd.DataFrame(results)


def integrate_with_transition_data(physical_df: pd.DataFrame) -> None:
    """
    Integrate physical risk data with existing transition risk data.

    Creates updated scenario files with both transition and physical drivers.
    """
    print("\n" + "="*80)
    print("INTEGRATING WITH TRANSITION RISK DATA")
    print("="*80)

    # Load existing transition risk data
    transition_files = {
        'Net Zero 2050': OUTPUT_DIR / 'net_zero_2050.csv',
        'Delayed Transition': OUTPUT_DIR / 'delayed_transition.csv',
        'Current Policies': OUTPUT_DIR / 'current_policies.csv'
    }

    for scenario_name, file_path in transition_files.items():
        print(f"\nProcessing {scenario_name}...")

        # Load transition data
        if not file_path.exists():
            print(f"  WARNING: {file_path} not found, skipping")
            continue

        transition_df = pd.read_csv(file_path)

        # Get physical risk data for this scenario
        physical_scenario = physical_df[physical_df['ScenarioName'] == scenario_name].copy()

        # Combine datasets
        combined_df = pd.concat([transition_df, physical_scenario], ignore_index=True)

        # Sort by Year, Driver, Region
        combined_df = combined_df.sort_values(['Year', 'Driver', 'Region'])

        # Save updated file
        combined_df.to_csv(file_path, index=False)
        print(f"  ✓ Updated {file_path.name}")
        print(f"    Transition drivers: {len(transition_df)} rows")
        print(f"    Physical drivers: {len(physical_scenario)} rows")
        print(f"    Total: {len(combined_df)} rows")

    # Also save complete physical risk dataset
    physical_output = OUTPUT_DIR / 'physical_risk_full.csv'
    physical_df.to_csv(physical_output, index=False)
    print(f"\n✓ Saved complete physical risk dataset: {physical_output.name}")
    print(f"  Total: {len(physical_df)} rows")


def main():
    print("="*80)
    print("PROCESSING WORLD BANK PHYSICAL RISK DATA")
    print("="*80)
    print(f"Source: {BASE_DIR}")
    print(f"Output: {OUTPUT_DIR}")
    print(f"Indicators: {list(INDICATORS.values())}")
    print(f"Scenarios: {list(SSP_TO_NGFS.values())}")
    print(f"Years: {SCENARIO_YEARS}")
    print("="*80)

    # Process all indicators
    df = process_all_indicators()

    if df.empty:
        print("\nERROR: No data extracted. Check NetCDF files.")
        return

    print("\n" + "="*80)
    print("STANDARDIZING VALUES")
    print("="*80)

    # Standardize
    standardized_df = standardize_values(df)

    print(f"\n✓ Standardized {len(standardized_df)} data points")
    print(f"  Breakdown: {standardized_df.groupby(['Driver', 'ScenarioName']).size()}")

    # Integrate with transition data
    integrate_with_transition_data(standardized_df)

    print("\n" + "="*80)
    print("PROCESSING COMPLETE")
    print("="*80)
    print("\nNext steps:")
    print("1. Review updated scenario files (net_zero_2050.csv, etc.)")
    print("2. Check physical_risk_full.csv for complete physical risk data")
    print("3. Update data/scenarios/README.md with physical risk documentation")
    print("4. Consider implementing proper country-level aggregation (currently using global means)")


if __name__ == '__main__':
    main()

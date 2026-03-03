#include "midas/portfolio.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <iostream>

namespace midas {

// Sector and region name mappings
static const std::vector<std::string> SECTOR_NAMES = {
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
};

static const std::vector<std::string> REGION_NAMES = {
    "North America",
    "Europe",
    "Asia-Pacific (developed)",
    "Asia-Pacific (emerging)",
    "Latin America",
    "Middle East & Africa",
    "Global"
};

// Obligor methods

void Obligor::compute_threshold() {
    // Compute Φ^(-1)(PD) using inverse normal CDF approximation
    // Using boost or C++20 would be better, but this is a simple approximation

    if (pd <= 0.0 || pd >= 1.0) {
        throw std::runtime_error("PD must be in (0, 1) for threshold calculation");
    }

    // Beasley-Springer-Moro approximation of inverse normal CDF
    // Source: Glasserman (2003), Monte Carlo Methods in Financial Engineering

    double p = pd;
    static const double a[4] = {2.50662823884, -18.61500062529, 41.39119773534, -25.44106049637};
    static const double b[4] = {-8.47351093090, 23.08336743743, -21.06224101826, 3.13082909833};
    static const double c[9] = {0.3374754822726147, 0.9761690190917186, 0.1607979714918209,
                                  0.0276438810333863, 0.0038405729373609, 0.0003951896511919,
                                  0.0000321767881768, 0.0000002888167364, 0.0000003960315187};

    double x, r;

    if (p < 0.5) {
        x = p;
    } else {
        x = 1.0 - p;
    }

    if (x > 0.08) {
        double q = x - 0.5;
        double r = q * q;
        threshold = q * (((a[3]*r + a[2])*r + a[1])*r + a[0]) /
                       ((((b[3]*r + b[2])*r + b[1])*r + b[0])*r + 1.0);
    } else {
        double r = std::sqrt(-std::log(x));
        threshold = (((((((c[8]*r + c[7])*r + c[6])*r + c[5])*r + c[4])*r + c[3])*r + c[2])*r + c[1])*r + c[0];
    }

    if (p < 0.5) {
        threshold = -threshold;
    }
}

void Obligor::compute_primary_cell() {
    primary_cell = sector_id * 7 + region_id;
}

// Portfolio methods

Portfolio::Portfolio(const std::string& csv_path) {
    load_from_csv(csv_path);
}

void Portfolio::load_from_csv(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open portfolio file: " + csv_path);
    }

    std::string line;

    // Read header
    if (!std::getline(file, line)) {
        throw std::runtime_error("Empty portfolio file");
    }

    // Parse header to find column indices
    std::vector<std::string> headers;
    std::stringstream header_ss(line);
    std::string header;
    while (std::getline(header_ss, header, ',')) {
        headers.push_back(header);
    }

    // Find required columns
    auto find_col = [&headers](const std::string& name) -> int {
        auto it = std::find(headers.begin(), headers.end(), name);
        if (it == headers.end()) {
            throw std::runtime_error("Missing required column: " + name);
        }
        return std::distance(headers.begin(), it);
    };

    int col_id = find_col("ObligorID");
    int col_name = find_col("Name");
    int col_sector = find_col("Sector");
    int col_region = find_col("Region");
    int col_pd = find_col("PD");
    int col_lgd = find_col("LGD");
    int col_ead = find_col("EAD");

    // Find Beta columns (Beta_0 through Beta_104)
    std::vector<int> beta_cols;
    for (int i = 0; i < 105; ++i) {
        std::string beta_name = "Beta_" + std::to_string(i);
        int col_idx = find_col(beta_name);
        beta_cols.push_back(col_idx);
    }

    // Read data rows
    int line_num = 1;
    while (std::getline(file, line)) {
        ++line_num;

        if (line.empty()) continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() != headers.size()) {
            std::cerr << "Warning: Line " << line_num << " has " << fields.size()
                      << " fields, expected " << headers.size() << std::endl;
            continue;
        }

        try {
            Obligor obligor;
            obligor.id = fields[col_id];
            obligor.name = fields[col_name];
            obligor.sector_id = parse_sector(fields[col_sector]);
            obligor.region_id = parse_region(fields[col_region]);
            obligor.pd = std::stod(fields[col_pd]);
            obligor.lgd = std::stod(fields[col_lgd]);
            obligor.ead = std::stod(fields[col_ead]);

            // Load Beta vector
            obligor.beta.resize(105);
            for (int i = 0; i < 105; ++i) {
                obligor.beta(i) = std::stod(fields[beta_cols[i]]);
            }

            // Compute derived fields
            obligor.compute_threshold();
            obligor.compute_primary_cell();

            obligors_.push_back(obligor);

        } catch (const std::exception& e) {
            std::cerr << "Error parsing line " << line_num << ": " << e.what() << std::endl;
            throw;
        }
    }

    file.close();

    // Compute aggregates
    compute_aggregates();

    std::cout << "Loaded " << obligors_.size() << " obligors from " << csv_path << std::endl;
}

void Portfolio::validate() const {
    for (size_t i = 0; i < obligors_.size(); ++i) {
        const auto& obl = obligors_[i];

        // Validate PD
        if (obl.pd < 0.0 || obl.pd > 1.0) {
            throw std::runtime_error("Obligor " + obl.id + ": PD must be in [0,1], got " + std::to_string(obl.pd));
        }

        // Validate LGD
        if (obl.lgd < 0.0 || obl.lgd > 1.0) {
            throw std::runtime_error("Obligor " + obl.id + ": LGD must be in [0,1], got " + std::to_string(obl.lgd));
        }

        // Validate EAD
        if (obl.ead <= 0.0) {
            throw std::runtime_error("Obligor " + obl.id + ": EAD must be positive, got " + std::to_string(obl.ead));
        }

        // Validate Beta vector sum (should be approximately 1.0)
        double beta_sum = obl.beta.sum();
        if (std::abs(beta_sum - 1.0) > 0.01) {
            throw std::runtime_error("Obligor " + obl.id + ": Beta sum must be ~1.0, got " + std::to_string(beta_sum));
        }

        // Validate sector/region IDs
        if (obl.sector_id < 0 || obl.sector_id >= 15) {
            throw std::runtime_error("Obligor " + obl.id + ": Invalid sector_id " + std::to_string(obl.sector_id));
        }
        if (obl.region_id < 0 || obl.region_id >= 7) {
            throw std::runtime_error("Obligor " + obl.id + ": Invalid region_id " + std::to_string(obl.region_id));
        }
    }
}

std::map<int, double> Portfolio::sector_ead() const {
    std::map<int, double> result;

    for (const auto& obl : obligors_) {
        result[obl.sector_id] += obl.ead;
    }

    return result;
}

std::map<int, double> Portfolio::region_ead() const {
    std::map<int, double> result;

    for (const auto& obl : obligors_) {
        result[obl.region_id] += obl.ead;
    }

    return result;
}

Eigen::MatrixXd Portfolio::concentration_matrix() const {
    Eigen::MatrixXd matrix = Eigen::MatrixXd::Zero(15, 7);

    for (const auto& obl : obligors_) {
        matrix(obl.sector_id, obl.region_id) += obl.ead;
    }

    return matrix;
}

void Portfolio::compute_aggregates() {
    total_ead_ = 0.0;

    for (const auto& obl : obligors_) {
        total_ead_ += obl.ead;
    }
}

int Portfolio::parse_sector(const std::string& sector_name) const {
    auto it = std::find(SECTOR_NAMES.begin(), SECTOR_NAMES.end(), sector_name);

    if (it != SECTOR_NAMES.end()) {
        return std::distance(SECTOR_NAMES.begin(), it);
    }

    // Default to "Other" if not found
    std::cerr << "Warning: Unknown sector '" << sector_name << "', using 'Other'" << std::endl;
    return 14; // Other
}

int Portfolio::parse_region(const std::string& region_name) const {
    auto it = std::find(REGION_NAMES.begin(), REGION_NAMES.end(), region_name);

    if (it != REGION_NAMES.end()) {
        return std::distance(REGION_NAMES.begin(), it);
    }

    // Default to "Global" if not found
    std::cerr << "Warning: Unknown region '" << region_name << "', using 'Global'" << std::endl;
    return 6; // Global
}

} // namespace midas

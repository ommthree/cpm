#include "midas/calibration.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <Eigen/Cholesky>

namespace midas {

void CalibrationData::load_from_directory(const std::string& calibration_dir) {
    namespace fs = std::filesystem;

    if (!fs::exists(calibration_dir)) {
        throw std::runtime_error("Calibration directory not found: " + calibration_dir);
    }

    // Load each calibration file
    std::string sector_path = calibration_dir + "/sector_scores.csv";
    std::string region_path = calibration_dir + "/region_scores.csv";
    std::string lambda_path = calibration_dir + "/lambda_parameters.csv";
    std::string variance_path = calibration_dir + "/residual_variance.csv";

    load_sector_scores(sector_path);
    load_region_scores(region_path);
    load_lambda_parameters(lambda_path);
    load_residual_variance(variance_path);

    // Validate dimensions
    validate();

    // Precompute derived data
    precompute_weights();
    precompute_cholesky();

    std::cout << "Calibration data loaded successfully:" << std::endl;
    std::cout << "  - Sector scores: " << sector_scores_.rows() << "×" << sector_scores_.cols() << std::endl;
    std::cout << "  - Region scores: " << region_scores_.rows() << "×" << region_scores_.cols() << std::endl;
    std::cout << "  - Lambda parameters: " << lambda_parameters_.size() << "×1" << std::endl;
    std::cout << "  - Residual variance: " << residual_variance_.rows() << "×" << residual_variance_.cols() << std::endl;
    std::cout << "  - Precomputed weights: " << weights_.rows() << "×" << weights_.cols() << std::endl;
    std::cout << "  - Cholesky decomposition: " << cholesky_.rows() << "×" << cholesky_.cols() << std::endl;
}

void CalibrationData::load_sector_scores(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open sector scores file: " + csv_path);
    }

    std::vector<std::vector<double>> data;
    std::string line;

    // Skip header
    std::getline(file, line);

    // Read data rows
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty and comment lines

        std::vector<double> row;
        std::stringstream ss(line);
        std::string field;

        // Skip first column (sector name)
        std::getline(ss, field, ',');

        // Read 8 driver scores
        while (std::getline(ss, field, ',')) {
            try {
                row.push_back(std::stod(field));
            } catch (...) {
                // Handle empty or invalid fields
                row.push_back(0.0);
            }
        }

        if (row.size() >= 8) {
            data.push_back(std::vector<double>(row.begin(), row.begin() + 8));
        }
    }

    file.close();

    // Convert to Eigen matrix
    if (data.empty()) {
        throw std::runtime_error("No data read from sector scores file");
    }

    int rows = data.size();
    sector_scores_.resize(rows, 8);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < 8; ++j) {
            sector_scores_(i, j) = data[i][j];
        }
    }

    std::cout << "Loaded sector scores: " << rows << " sectors × 8 drivers" << std::endl;
}

void CalibrationData::load_region_scores(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open region scores file: " + csv_path);
    }

    std::vector<std::vector<double>> data;
    std::string line;

    // Skip header
    std::getline(file, line);

    // Read data rows
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty and comment lines

        std::vector<double> row;
        std::stringstream ss(line);
        std::string field;

        // Skip first column (region name)
        std::getline(ss, field, ',');

        // Read 8 driver scores
        while (std::getline(ss, field, ',')) {
            try {
                row.push_back(std::stod(field));
            } catch (...) {
                row.push_back(0.0);
            }
        }

        if (row.size() >= 8) {
            data.push_back(std::vector<double>(row.begin(), row.begin() + 8));
        }
    }

    file.close();

    if (data.empty()) {
        throw std::runtime_error("No data read from region scores file");
    }

    int rows = data.size();
    region_scores_.resize(rows, 8);

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < 8; ++j) {
            region_scores_(i, j) = data[i][j];
        }
    }

    std::cout << "Loaded region scores: " << rows << " regions × 8 drivers" << std::endl;
}

void CalibrationData::load_lambda_parameters(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open lambda parameters file: " + csv_path);
    }

    std::vector<double> data;
    std::string line;

    // Skip header
    std::getline(file, line);

    // Read data rows
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty and comment lines

        std::stringstream ss(line);
        std::string field;

        // Skip first column (driver name)
        std::getline(ss, field, ',');

        // Read lambda value
        if (std::getline(ss, field, ',')) {
            try {
                data.push_back(std::stod(field));
            } catch (...) {
                // Skip invalid rows rather than adding 0.0
                continue;
            }
        }
    }

    file.close();

    if (data.empty()) {
        throw std::runtime_error("No data read from lambda parameters file");
    }

    lambda_parameters_.resize(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        lambda_parameters_(i) = data[i];
    }

    std::cout << "Loaded lambda parameters: " << data.size() << " drivers" << std::endl;
}

void CalibrationData::load_residual_variance(const std::string& csv_path) {
    std::ifstream file(csv_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open residual variance file: " + csv_path);
    }

    double sigma_a = 0.0;  // Sector std dev
    double sigma_b = 0.0;  // Region std dev
    double sigma_xi = 0.0; // Cell std dev

    std::string line;

    // Skip header
    std::getline(file, line);

    // Read variance components
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;  // Skip empty and comment lines

        std::stringstream ss(line);
        std::string component, symbol, value_str;

        std::getline(ss, component, ',');
        std::getline(ss, symbol, ',');
        std::getline(ss, value_str, ',');

        try {
            double value = std::stod(value_str);

            if (component == "Sector") {
                sigma_a = value;
            } else if (component == "Region") {
                sigma_b = value;
            } else if (component == "Cell") {
                sigma_xi = value;
            }
        } catch (...) {
            continue;
        }
    }

    file.close();

    if (sigma_a == 0.0 || sigma_b == 0.0 || sigma_xi == 0.0) {
        throw std::runtime_error("Failed to read all variance components (σ_a, σ_b, σ_ξ)");
    }

    std::cout << "Loaded variance components:" << std::endl;
    std::cout << "  σ_a (sector): " << sigma_a << std::endl;
    std::cout << "  σ_b (region): " << sigma_b << std::endl;
    std::cout << "  σ_ξ (cell): " << sigma_xi << std::endl;

    // Construct 105×105 covariance matrix
    // u_{s,r} = a_s + b_r + ξ_{s,r}
    // Cov[u_{s,r}, u_{s',r'}] = σ_a² I[s=s'] + σ_b² I[r=r'] + σ_ξ² I[s=s' AND r=r']

    residual_variance_.resize(105, 105);
    residual_variance_.setZero();

    for (int s = 0; s < 15; ++s) {         // sectors
        for (int r = 0; r < 7; ++r) {      // regions
            int cell = s * 7 + r;

            for (int s2 = 0; s2 < 15; ++s2) {
                for (int r2 = 0; r2 < 7; ++r2) {
                    int cell2 = s2 * 7 + r2;

                    double cov = 0.0;

                    // Same sector contribution
                    if (s == s2) {
                        cov += sigma_a * sigma_a;
                    }

                    // Same region contribution
                    if (r == r2) {
                        cov += sigma_b * sigma_b;
                    }

                    // Same cell contribution (idiosyncratic)
                    if (s == s2 && r == r2) {
                        cov += sigma_xi * sigma_xi;
                    }

                    residual_variance_(cell, cell2) = cov;
                }
            }
        }
    }

    std::cout << "Constructed residual variance: 105×105 covariance matrix" << std::endl;

    // Print sample correlations
    double var_total = residual_variance_(0, 0);
    double corr_sector = (sigma_a * sigma_a) / var_total;
    double corr_region = (sigma_b * sigma_b) / var_total;

    std::cout << "  Total variance: " << var_total << std::endl;
    std::cout << "  Implied within-sector correlation: " << corr_sector << std::endl;
    std::cout << "  Implied within-region correlation: " << corr_region << std::endl;
}

void CalibrationData::validate() const {
    // Check sector scores
    if (sector_scores_.rows() != 15 || sector_scores_.cols() != 8) {
        throw std::runtime_error("Sector scores must be 15×8, got " +
                                 std::to_string(sector_scores_.rows()) + "×" +
                                 std::to_string(sector_scores_.cols()));
    }

    // Check region scores
    if (region_scores_.rows() != 7 || region_scores_.cols() != 8) {
        throw std::runtime_error("Region scores must be 7×8, got " +
                                 std::to_string(region_scores_.rows()) + "×" +
                                 std::to_string(region_scores_.cols()));
    }

    // Check lambda parameters
    if (lambda_parameters_.size() != 8) {
        throw std::runtime_error("Lambda parameters must be 8×1, got " +
                                 std::to_string(lambda_parameters_.size()) + "×1");
    }

    // Check residual variance
    if (residual_variance_.rows() != 105 || residual_variance_.cols() != 105) {
        throw std::runtime_error("Residual variance must be 105×105, got " +
                                 std::to_string(residual_variance_.rows()) + "×" +
                                 std::to_string(residual_variance_.cols()));
    }

    // Check symmetry of residual variance
    double asymmetry = (residual_variance_ - residual_variance_.transpose()).norm();
    if (asymmetry > 1e-6) {
        std::cerr << "Warning: Residual variance matrix is not symmetric (asymmetry = "
                  << asymmetry << ")" << std::endl;
    }

    std::cout << "✓ Calibration data validation passed" << std::endl;
}

void CalibrationData::precompute_weights() {
    // Allocate 105×8 matrix
    weights_.resize(105, 8);

    // Compute w̃_{s,r,k} = S_{s,k} × R_{r,k}
    for (int s = 0; s < 15; ++s) {         // sectors
        for (int r = 0; r < 7; ++r) {      // regions
            int cell = cell_index(s, r);
            for (int k = 0; k < 8; ++k) {  // drivers
                weights_(cell, k) = sector_scores_(s, k) * region_scores_(r, k);
            }
        }
    }

    std::cout << "✓ Precomputed weights: 105×8 matrix" << std::endl;
}

void CalibrationData::precompute_cholesky() {
    // Compute Cholesky decomposition: Σ_u = L L^T
    Eigen::LLT<Eigen::MatrixXd> llt(residual_variance_);

    if (llt.info() != Eigen::Success) {
        throw std::runtime_error("Cholesky decomposition failed - matrix is not positive definite");
    }

    cholesky_ = llt.matrixL();

    std::cout << "✓ Precomputed Cholesky decomposition: 105×105 matrix" << std::endl;
}

double CalibrationData::get_weight(int sector_id, int region_id, int driver_id) const {
    if (sector_id < 0 || sector_id >= 15) {
        throw std::out_of_range("Sector ID must be in [0, 14], got " + std::to_string(sector_id));
    }
    if (region_id < 0 || region_id >= 7) {
        throw std::out_of_range("Region ID must be in [0, 6], got " + std::to_string(region_id));
    }
    if (driver_id < 0 || driver_id >= 8) {
        throw std::out_of_range("Driver ID must be in [0, 7], got " + std::to_string(driver_id));
    }

    int cell = cell_index(sector_id, region_id);
    return weights_(cell, driver_id);
}

Eigen::VectorXd CalibrationData::get_cell_weights(int sector_id, int region_id) const {
    if (sector_id < 0 || sector_id >= 15) {
        throw std::out_of_range("Sector ID must be in [0, 14], got " + std::to_string(sector_id));
    }
    if (region_id < 0 || region_id >= 7) {
        throw std::out_of_range("Region ID must be in [0, 6], got " + std::to_string(region_id));
    }

    int cell = cell_index(sector_id, region_id);
    return weights_.row(cell);
}

} // namespace midas

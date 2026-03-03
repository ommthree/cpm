#pragma once

#include <string>
#include <Eigen/Dense>

namespace midas {

/**
 * @brief Calibration data for climate-credit factor model
 *
 * Stores calibrated parameters for the factor model:
 *   F_{s,r} = Σ_k λ_k · w̃_{s,r,k} · φ_k(C)
 *
 * where:
 *   - F_{s,r}: Factor value for sector s, region r
 *   - λ_k: Lambda parameter for driver k (8 drivers)
 *   - w̃_{s,r,k}: Precomputed weight = S_{s,k} × R_{r,k}
 *   - φ_k(C): Standardized climate driver value
 */
class CalibrationData {
public:
    /**
     * @brief Default constructor
     */
    CalibrationData() = default;

    /**
     * @brief Load all calibration data from directory
     * @param calibration_dir Directory containing calibration CSV files
     *
     * Expected files:
     *   - sector_scores.csv: 15 sectors × 8 drivers
     *   - region_scores.csv: 7 regions × 8 drivers
     *   - lambda_parameters.csv: 8 drivers × 1
     *   - residual_variance.csv: 105×105 covariance matrix
     */
    void load_from_directory(const std::string& calibration_dir);

    /**
     * @brief Load individual calibration files
     */
    void load_sector_scores(const std::string& csv_path);
    void load_region_scores(const std::string& csv_path);
    void load_lambda_parameters(const std::string& csv_path);
    void load_residual_variance(const std::string& csv_path);

    /**
     * @brief Validate calibration data dimensions
     * @throws std::runtime_error if dimensions are invalid
     */
    void validate() const;

    // Accessors

    /**
     * @brief Get sector score matrix (15×8)
     */
    const Eigen::MatrixXd& sector_scores() const { return sector_scores_; }

    /**
     * @brief Get region score matrix (7×8)
     */
    const Eigen::MatrixXd& region_scores() const { return region_scores_; }

    /**
     * @brief Get lambda parameters vector (8×1)
     */
    const Eigen::VectorXd& lambda_parameters() const { return lambda_parameters_; }

    /**
     * @brief Get residual variance covariance matrix (105×105)
     */
    const Eigen::MatrixXd& residual_variance() const { return residual_variance_; }

    /**
     * @brief Get precomputed weights tensor (15×7×8)
     * @return Flattened 105×8 matrix where row = sector*7 + region
     */
    const Eigen::MatrixXd& weights() const { return weights_; }

    /**
     * @brief Get Cholesky decomposition of residual variance (105×105)
     * @return Lower triangular matrix L such that Σ_u = L L^T
     */
    const Eigen::MatrixXd& cholesky() const { return cholesky_; }

    /**
     * @brief Get weight for specific sector-region-driver combination
     * @param sector_id Sector index [0-14]
     * @param region_id Region index [0-6]
     * @param driver_id Driver index [0-7]
     * @return w̃_{s,r,k} = S_{s,k} × R_{r,k}
     */
    double get_weight(int sector_id, int region_id, int driver_id) const;

    /**
     * @brief Get all weights for a sector-region cell (8×1 vector)
     * @param sector_id Sector index [0-14]
     * @param region_id Region index [0-6]
     * @return Vector of 8 weights for all drivers
     */
    Eigen::VectorXd get_cell_weights(int sector_id, int region_id) const;

private:
    // Raw calibration data
    Eigen::MatrixXd sector_scores_;        // 15×8
    Eigen::MatrixXd region_scores_;        // 7×8
    Eigen::VectorXd lambda_parameters_;    // 8×1
    Eigen::MatrixXd residual_variance_;    // 105×105

    // Precomputed derived data
    Eigen::MatrixXd weights_;              // 105×8 (flattened w̃_{s,r,k})
    Eigen::MatrixXd cholesky_;             // 105×105 (L such that Σ_u = LL^T)

    /**
     * @brief Precompute weights from sector and region scores
     *
     * Computes w̃_{s,r,k} = S_{s,k} × R_{r,k} for all combinations
     * Stores in flattened 105×8 matrix where row = sector*7 + region
     */
    void precompute_weights();

    /**
     * @brief Precompute Cholesky decomposition of residual variance
     *
     * Computes L such that Σ_u = L L^T
     * Used for efficient sampling: u ~ N(0, Σ_u) via L × z where z ~ N(0, I)
     */
    void precompute_cholesky();

    /**
     * @brief Convert sector/region indices to flat cell index
     * @param sector_id Sector index [0-14]
     * @param region_id Region index [0-6]
     * @return Cell index [0-104]
     */
    int cell_index(int sector_id, int region_id) const {
        return sector_id * 7 + region_id;
    }
};

} // namespace midas

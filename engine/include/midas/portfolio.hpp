#pragma once

#include <string>
#include <vector>
#include <map>
#include <Eigen/Dense>

namespace midas {

/**
 * @brief Represents a single obligor (borrower) in the portfolio
 */
struct Obligor {
    std::string id;              // Unique identifier (e.g., "OBL001")
    std::string name;            // Company name
    int sector_id;               // Sector index [0-14]
    int region_id;               // Region index [0-6]
    double pd;                   // Probability of Default [0,1]
    double lgd;                  // Loss Given Default [0,1]
    double ead;                  // Exposure at Default (USD)

    Eigen::VectorXd beta;        // Factor loadings (105×1 vector)

    // Derived fields (computed after loading)
    double threshold;            // Default threshold: Φ^(-1)(PD)
    int primary_cell;            // Primary sector-region cell index [0-104]

    /**
     * @brief Compute threshold from PD
     * threshold = Φ^(-1)(PD) where Φ is standard normal CDF
     */
    void compute_threshold();

    /**
     * @brief Compute primary cell index from sector and region
     * cell = sector_id * 7 + region_id
     */
    void compute_primary_cell();
};

/**
 * @brief Portfolio of obligors with sector-region exposure analysis
 */
class Portfolio {
public:
    /**
     * @brief Default constructor
     */
    Portfolio() = default;

    /**
     * @brief Construct and load portfolio from CSV file
     * @param csv_path Path to portfolio CSV file
     */
    explicit Portfolio(const std::string& csv_path);

    /**
     * @brief Load portfolio from CSV file
     * @param csv_path Path to portfolio CSV file
     *
     * Expected CSV format:
     * ObligorID,Name,Sector,Region,PD,LGD,EAD,...,Beta_0,Beta_1,...,Beta_104
     */
    void load_from_csv(const std::string& csv_path);

    /**
     * @brief Validate portfolio data
     * @throws std::runtime_error if validation fails
     *
     * Checks:
     * - PD in [0, 1]
     * - LGD in [0, 1]
     * - EAD > 0
     * - Beta vector sums to approximately 1.0
     * - Sector/region IDs valid
     */
    void validate() const;

    // Accessors

    /**
     * @brief Get number of obligors
     */
    size_t size() const { return obligors_.size(); }

    /**
     * @brief Get total portfolio EAD
     */
    double total_ead() const { return total_ead_; }

    /**
     * @brief Get obligor by index
     */
    const Obligor& get_obligor(size_t i) const { return obligors_[i]; }

    /**
     * @brief Get all obligors
     */
    const std::vector<Obligor>& obligors() const { return obligors_; }

    // Analytics

    /**
     * @brief Get total EAD by sector
     * @return Map: sector_id -> total_ead
     */
    std::map<int, double> sector_ead() const;

    /**
     * @brief Get total EAD by region
     * @return Map: region_id -> total_ead
     */
    std::map<int, double> region_ead() const;

    /**
     * @brief Get concentration matrix (15 sectors × 7 regions)
     * @return Matrix where element (s,r) = total EAD in that cell
     */
    Eigen::MatrixXd concentration_matrix() const;

private:
    std::vector<Obligor> obligors_;
    double total_ead_;

    /**
     * @brief Compute aggregates (total_ead, etc.)
     */
    void compute_aggregates();

    /**
     * @brief Parse sector name to sector_id
     */
    int parse_sector(const std::string& sector_name) const;

    /**
     * @brief Parse region name to region_id
     */
    int parse_region(const std::string& region_name) const;
};

} // namespace midas

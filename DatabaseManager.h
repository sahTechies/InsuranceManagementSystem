#pragma once
#include "Models.h"
#include <string>
#include <vector>
#include <optional>

struct sqlite3; // Forward declaration

/**
 * @brief Singleton DatabaseManager to encapsulate SQLite operations
 * Uses prepared statements to prevent SQL injection.
 */
class DatabaseManager {
private:
    sqlite3* db = nullptr;
    
    // Private constructor/destructor for Singleton
    DatabaseManager();
    ~DatabaseManager();

    // Prevent copy
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    void seedData();

public:
    static DatabaseManager& getInstance();

    bool connect(const std::string& dbPath);
    bool initializeSchema();

    // User operations
    bool createUser(const User& user);
    std::optional<User> getUserByUsername(const std::string& username);

    // Policy operations
    bool createPolicy(const Policy& policy);
    std::vector<Policy> getAllPolicies();
    std::optional<Policy> getPolicyById(int id);

    // UserPolicy operations
    bool assignPolicyToUser(int userId, int policyId, double totalPremium, const std::string& activeAddons, const std::string& expiryDate);
    std::vector<UserPolicyView> getUserPolicies(int userId);

    // Claim operations
    bool createClaim(const Claim& claim);
    std::vector<Claim> getAllClaims();
    bool updateClaimStatus(int claimId, const std::string& status, const std::string& adminRemarks);
};

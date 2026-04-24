#pragma once
#include "Models.h"
#include <string>
#include <vector>
#include <optional>

struct sqlite3; // Forward declaration

// Why: This class encapsulates all DB access. If we switch to MySQL,
// only this class changes, not Admin or Customer (Business Logic).
class DatabaseManager {
private:
    sqlite3* db = nullptr;
    bool executeNonQuery(const std::string& sql);

public:
    DatabaseManager();
    ~DatabaseManager();

    bool connect(const std::string& dbPath);
    bool initializeSchema();

    // User operations
    bool createUser(const User& user);
    std::optional<User> getUserByUsername(const std::string& username);

    // Policy operations
    bool createPolicy(const Policy& policy);
    std::vector<Policy> getAllPolicies();

    // UserPolicy operations
    bool assignPolicyToUser(int userId, int policyId, const std::string& status);
    std::vector<UserPolicyView> getUserPolicies(int userId);

    // Claim operations
    bool createClaim(const Claim& claim);
    std::vector<Claim> getAllClaims();
    bool updateClaimStatus(int claimId, const std::string& status);
};

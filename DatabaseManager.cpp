#include "DatabaseManager.h"
#include <sqlite3.h>
#include <iostream>

DatabaseManager::DatabaseManager() : db(nullptr) {}
DatabaseManager::~DatabaseManager() {
    if (db) sqlite3_close(db);
}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

bool DatabaseManager::connect(const std::string& dbPath) {
    return sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK;
}

bool DatabaseManager::initializeSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS policies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            category TEXT NOT NULL,
            base_premium REAL NOT NULL CHECK(base_premium >= 0),
            base_coverage REAL NOT NULL
        );
        CREATE TABLE IF NOT EXISTS user_policies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            policy_id INTEGER,
            total_premium REAL NOT NULL,
            active_addons TEXT,
            expiry_date TEXT,
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(policy_id) REFERENCES policies(id)
        );
        CREATE TABLE IF NOT EXISTS claims (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            policy_id INTEGER,
            amount REAL NOT NULL,
            status TEXT,
            admin_remarks TEXT,
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(policy_id) REFERENCES policies(id)
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Schema Init Error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    
    seedData();
    return true;
}

void DatabaseManager::seedData() {
    std::string checkSql = "SELECT COUNT(*) FROM policies;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, checkSql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int count = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);
            if (count > 0) return; // Already seeded
        } else {
            sqlite3_finalize(stmt);
        }
    }
    
    std::vector<Policy> seedPolicies = {
        {0, "Term Life", "Life", 5000, 100000},
        {0, "Whole Life", "Life", 15000, 200000},
        {0, "Endowment", "Life", 12000, 150000},
        {0, "Structure Secure", "Home", 3000, 50000},
        {0, "Content Guard", "Home", 2000, 30000},
        {0, "Third Party", "Motor", 2500, 20000},
        {0, "Zero Dep", "Motor", 7000, 80000}
    };
    for (const auto& p : seedPolicies) {
        createPolicy(p);
    }
}

bool DatabaseManager::createUser(const User& user) {
    const char* sql = "INSERT INTO users (username, password, role) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.role.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> DatabaseManager::getUserByUsername(const std::string& username) {
    const char* sql = "SELECT id, username, password, role FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    std::optional<User> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::createPolicy(const Policy& policy) {
    const char* sql = "INSERT INTO policies (name, category, base_premium, base_coverage) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, policy.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, policy.category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, policy.base_premium);
    sqlite3_bind_double(stmt, 4, policy.base_coverage);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Policy> DatabaseManager::getAllPolicies() {
    std::vector<Policy> policies;
    const char* sql = "SELECT id, name, category, base_premium, base_coverage FROM policies;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return policies;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Policy p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.base_premium = sqlite3_column_double(stmt, 3);
        p.base_coverage = sqlite3_column_double(stmt, 4);
        policies.push_back(p);
    }
    sqlite3_finalize(stmt);
    return policies;
}

std::optional<Policy> DatabaseManager::getPolicyById(int id) {
    const char* sql = "SELECT id, name, category, base_premium, base_coverage FROM policies WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    
    sqlite3_bind_int(stmt, 1, id);
    
    std::optional<Policy> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Policy p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.base_premium = sqlite3_column_double(stmt, 3);
        p.base_coverage = sqlite3_column_double(stmt, 4);
        result = p;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::assignPolicyToUser(int userId, int policyId, double totalPremium, const std::string& activeAddons, const std::string& expiryDate) {
    const char* sql = "INSERT INTO user_policies (user_id, policy_id, total_premium, active_addons, expiry_date) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, policyId);
    sqlite3_bind_double(stmt, 3, totalPremium);
    sqlite3_bind_text(stmt, 4, activeAddons.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, expiryDate.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<UserPolicyView> DatabaseManager::getUserPolicies(int userId) {
    std::vector<UserPolicyView> userPolicies;
    const char* sql = "SELECT up.id, p.id, p.name, p.category, p.base_premium, up.total_premium, up.active_addons, up.expiry_date "
                      "FROM user_policies up "
                      "JOIN policies p ON up.policy_id = p.id "
                      "WHERE up.user_id = ?;";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return userPolicies;
    
    sqlite3_bind_int(stmt, 1, userId);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserPolicyView view;
        view.user_policy_id = sqlite3_column_int(stmt, 0);
        view.policy_id = sqlite3_column_int(stmt, 1);
        view.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        view.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        view.base_premium = sqlite3_column_double(stmt, 4);
        view.total_premium = sqlite3_column_double(stmt, 5);
        view.active_addons = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        view.expiry_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        userPolicies.push_back(view);
    }
    sqlite3_finalize(stmt);
    return userPolicies;
}

bool DatabaseManager::createClaim(const Claim& claim) {
    const char* sql = "INSERT INTO claims (user_id, policy_id, amount, status, admin_remarks) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, claim.user_id);
    sqlite3_bind_int(stmt, 2, claim.policy_id);
    sqlite3_bind_double(stmt, 3, claim.amount);
    sqlite3_bind_text(stmt, 4, claim.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, claim.admin_remarks.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Claim> DatabaseManager::getAllClaims() {
    std::vector<Claim> claims;
    const char* sql = "SELECT id, user_id, policy_id, amount, status, admin_remarks FROM claims;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return claims;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Claim c;
        c.id = sqlite3_column_int(stmt, 0);
        c.user_id = sqlite3_column_int(stmt, 1);
        c.policy_id = sqlite3_column_int(stmt, 2);
        c.amount = sqlite3_column_double(stmt, 3);
        c.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        const char* remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        c.admin_remarks = remarks ? remarks : "";
        claims.push_back(c);
    }
    sqlite3_finalize(stmt);
    return claims;
}

bool DatabaseManager::updateClaimStatus(int claimId, const std::string& status, const std::string& adminRemarks) {
    const char* sql = "UPDATE claims SET status = ?, admin_remarks = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, adminRemarks.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, claimId);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

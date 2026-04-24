#include "DatabaseManager.h"
#include <sqlite3.h>
#include <iostream>

DatabaseManager::DatabaseManager() : db(nullptr) {}

DatabaseManager::~DatabaseManager() {
    if (db) sqlite3_close(db);
}

// Why: Reusable helper for simple queries avoiding repetitive error handling.
bool DatabaseManager::executeNonQuery(const std::string& sql) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL Error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
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
            premium REAL NOT NULL CHECK(premium >= 0),
            coverage REAL NOT NULL
        );
        CREATE TABLE IF NOT EXISTS user_policies (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            policy_id INTEGER,
            status TEXT,
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(policy_id) REFERENCES policies(id)
        );
        CREATE TABLE IF NOT EXISTS claims (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            policy_id INTEGER,
            amount REAL NOT NULL,
            status TEXT,
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(policy_id) REFERENCES policies(id)
        );
    )";
    return executeNonQuery(sql);
}

bool DatabaseManager::createUser(const User& user) {
    std::string sql = "INSERT INTO users (username, password, role) VALUES ('" +
                      user.username + "', '" + user.password + "', '" + user.role + "');";
    return executeNonQuery(sql);
}

std::optional<User> DatabaseManager::getUserByUsername(const std::string& username) {
    std::string sql = "SELECT id, username, password, role FROM users WHERE username = '" + username + "';";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    
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
    std::string sql = "INSERT INTO policies (name, category, premium, coverage) VALUES ('" +
                      policy.name + "', '" + policy.category + "', " + 
                      std::to_string(policy.premium) + ", " + std::to_string(policy.coverage) + ");";
    return executeNonQuery(sql);
}

std::vector<Policy> DatabaseManager::getAllPolicies() {
    std::vector<Policy> policies;
    std::string sql = "SELECT id, name, category, premium, coverage FROM policies;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return policies;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Policy p;
        p.id = sqlite3_column_int(stmt, 0);
        p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        p.premium = sqlite3_column_double(stmt, 3);
        p.coverage = sqlite3_column_double(stmt, 4);
        policies.push_back(p);
    }
    sqlite3_finalize(stmt);
    return policies;
}

bool DatabaseManager::assignPolicyToUser(int userId, int policyId, const std::string& status) {
    std::string sql = "INSERT INTO user_policies (user_id, policy_id, status) VALUES (" +
                      std::to_string(userId) + ", " + std::to_string(policyId) + ", '" + status + "');";
    return executeNonQuery(sql);
}

// Why: Using a JOIN here fulfills the spec requirement to show customer specific policy names rather than IDs
std::vector<UserPolicyView> DatabaseManager::getUserPolicies(int userId) {
    std::vector<UserPolicyView> userPolicies;
    std::string sql = "SELECT up.id, p.id, p.name, p.category, p.premium, p.coverage, up.status "
                      "FROM user_policies up "
                      "JOIN policies p ON up.policy_id = p.id "
                      "WHERE up.user_id = " + std::to_string(userId) + ";";
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return userPolicies;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserPolicyView view;
        view.user_policy_id = sqlite3_column_int(stmt, 0);
        view.policy_id = sqlite3_column_int(stmt, 1);
        view.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        view.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        view.premium = sqlite3_column_double(stmt, 4);
        view.coverage = sqlite3_column_double(stmt, 5);
        view.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        userPolicies.push_back(view);
    }
    sqlite3_finalize(stmt);
    return userPolicies;
}

bool DatabaseManager::createClaim(const Claim& claim) {
    std::string sql = "INSERT INTO claims (user_id, policy_id, amount, status) VALUES (" +
                      std::to_string(claim.user_id) + ", " + std::to_string(claim.policy_id) + ", " + 
                      std::to_string(claim.amount) + ", '" + claim.status + "');";
    return executeNonQuery(sql);
}

std::vector<Claim> DatabaseManager::getAllClaims() {
    std::vector<Claim> claims;
    std::string sql = "SELECT id, user_id, policy_id, amount, status FROM claims;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) return claims;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Claim c;
        c.id = sqlite3_column_int(stmt, 0);
        c.user_id = sqlite3_column_int(stmt, 1);
        c.policy_id = sqlite3_column_int(stmt, 2);
        c.amount = sqlite3_column_double(stmt, 3);
        c.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        claims.push_back(c);
    }
    sqlite3_finalize(stmt);
    return claims;
}

bool DatabaseManager::updateClaimStatus(int claimId, const std::string& status) {
    std::string sql = "UPDATE claims SET status = '" + status + "' WHERE id = " + std::to_string(claimId) + ";";
    return executeNonQuery(sql);
}

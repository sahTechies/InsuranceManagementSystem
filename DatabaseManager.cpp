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
            role TEXT NOT NULL,
            full_name TEXT,
            age INTEGER,
            gender TEXT,
            marital_status TEXT,
            nominee_name TEXT,
            father_name TEXT,
            mother_name TEXT,
            balance REAL DEFAULT 0.0
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
            status TEXT DEFAULT 'Active',
            FOREIGN KEY(user_id) REFERENCES users(id),
            FOREIGN KEY(policy_id) REFERENCES policies(id)
        );
        CREATE TABLE IF NOT EXISTS claims (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER,
            policy_id INTEGER,
            amount REAL NOT NULL,
            reason TEXT NOT NULL,
            status TEXT,
            admin_remarks TEXT,
            is_notified INTEGER DEFAULT 0,
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
            if (count > 0) return; 
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
    const char* sql = "INSERT INTO users (username, password, role, full_name, age, gender, marital_status, nominee_name, father_name, mother_name) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.role.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.full_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, user.age);
    sqlite3_bind_text(stmt, 6, user.gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, user.marital_status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, user.nominee_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, user.father_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, user.mother_name.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> DatabaseManager::getUserByUsername(const std::string& username) {
    const char* sql = "SELECT id, username, password, role, full_name, age, gender, marital_status, nominee_name, father_name, mother_name, balance FROM users WHERE username = ?;";
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
        u.full_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        u.age = sqlite3_column_int(stmt, 5);
        u.gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        u.marital_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        u.nominee_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) : "";
        u.father_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        u.mother_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        u.balance = sqlite3_column_double(stmt, 11);
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

std::optional<User> DatabaseManager::getUserById(int id) {
    const char* sql = "SELECT id, username, password, role, full_name, age, gender, marital_status, nominee_name, father_name, mother_name, balance FROM users WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return std::nullopt;
    
    sqlite3_bind_int(stmt, 1, id);
    
    std::optional<User> result = std::nullopt;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        u.full_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        u.age = sqlite3_column_int(stmt, 5);
        u.gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        u.marital_status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        u.nominee_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)) : "";
        u.father_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) : "";
        u.mother_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) : "";
        u.balance = sqlite3_column_double(stmt, 11);
        result = u;
    }
    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::addBalanceToUser(int userId, double amount) {
    const char* sql = "UPDATE users SET balance = balance + ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, userId);
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
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
    const char* sql = "INSERT INTO user_policies (user_id, policy_id, total_premium, active_addons, expiry_date, status) VALUES (?, ?, ?, ?, ?, 'Active');";
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
    const char* sql = "SELECT up.id, p.id, p.name, p.category, p.base_premium, up.total_premium, up.active_addons, up.expiry_date, up.status "
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
        view.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        userPolicies.push_back(view);
    }
    sqlite3_finalize(stmt);
    return userPolicies;
}

bool DatabaseManager::markUserPolicyClaimed(int userId, int policyId) {
    const char* sql = "UPDATE user_policies SET status = 'CLAIMED' WHERE user_id = ? AND policy_id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_int(stmt, 2, policyId);
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

bool DatabaseManager::createClaim(const Claim& claim) {
    const char* sql = "INSERT INTO claims (user_id, policy_id, amount, reason, status, admin_remarks, is_notified) VALUES (?, ?, ?, ?, ?, ?, 0);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, claim.user_id);
    sqlite3_bind_int(stmt, 2, claim.policy_id);
    sqlite3_bind_double(stmt, 3, claim.amount);
    sqlite3_bind_text(stmt, 4, claim.reason.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, claim.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, claim.admin_remarks.c_str(), -1, SQLITE_TRANSIENT);
    
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<Claim> DatabaseManager::getAllClaims() {
    std::vector<Claim> claims;
    const char* sql = "SELECT id, user_id, policy_id, amount, reason, status, admin_remarks FROM claims;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return claims;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Claim c;
        c.id = sqlite3_column_int(stmt, 0);
        c.user_id = sqlite3_column_int(stmt, 1);
        c.policy_id = sqlite3_column_int(stmt, 2);
        c.amount = sqlite3_column_double(stmt, 3);
        c.reason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        c.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        const char* remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        c.admin_remarks = remarks ? remarks : "";
        claims.push_back(c);
    }
    sqlite3_finalize(stmt);
    return claims;
}

std::vector<PendingClaimView> DatabaseManager::getPendingClaims() {
    std::vector<PendingClaimView> pending;
    const char* sql = R"(
        SELECT c.id, c.user_id, c.policy_id, u.full_name, p.name, c.amount, c.reason 
        FROM claims c 
        JOIN users u ON c.user_id = u.id 
        JOIN policies p ON c.policy_id = p.id 
        WHERE c.status = 'Pending';
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return pending;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        PendingClaimView pv;
        pv.claim_id = sqlite3_column_int(stmt, 0);
        pv.user_id = sqlite3_column_int(stmt, 1);
        pv.policy_id = sqlite3_column_int(stmt, 2);
        pv.full_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        pv.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) : "";
        pv.amount = sqlite3_column_double(stmt, 5);
        pv.reason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        pending.push_back(pv);
    }
    sqlite3_finalize(stmt);
    return pending;
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

std::vector<UserClaimView> DatabaseManager::getUserClaims(int userId) {
    std::vector<UserClaimView> claims;
    const char* sql = R"(
        SELECT c.id, p.name, c.amount, c.status, c.admin_remarks
        FROM claims c
        JOIN policies p ON c.policy_id = p.id
        WHERE c.user_id = ?;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return claims;
    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserClaimView ucv;
        ucv.claim_id = sqlite3_column_int(stmt, 0);
        ucv.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ucv.amount = sqlite3_column_double(stmt, 2);
        ucv.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        ucv.admin_remarks = remarks ? remarks : "";
        claims.push_back(ucv);
    }
    sqlite3_finalize(stmt);
    return claims;
}

std::vector<UserClaimView> DatabaseManager::getUnnotifiedClaims(int userId) {
    std::vector<UserClaimView> claims;
    const char* sql = R"(
        SELECT c.id, p.name, c.amount, c.status, c.admin_remarks
        FROM claims c
        JOIN policies p ON c.policy_id = p.id
        WHERE c.user_id = ? AND c.status != 'Pending' AND c.is_notified = 0;
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return claims;
    sqlite3_bind_int(stmt, 1, userId);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        UserClaimView ucv;
        ucv.claim_id = sqlite3_column_int(stmt, 0);
        ucv.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        ucv.amount = sqlite3_column_double(stmt, 2);
        ucv.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        const char* remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        ucv.admin_remarks = remarks ? remarks : "";
        claims.push_back(ucv);
    }
    sqlite3_finalize(stmt);
    return claims;
}

bool DatabaseManager::markClaimNotified(int claimId) {
    const char* sql = "UPDATE claims SET is_notified = 1 WHERE id = ?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return false;
    sqlite3_bind_int(stmt, 1, claimId);
    bool result = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return result;
}

std::vector<ExportClaimView> DatabaseManager::getApprovedClaimsForExport() {
    std::vector<ExportClaimView> exportList;
    const char* sql = R"(
        SELECT c.id, u.full_name, u.age, p.name, c.amount, c.reason, c.admin_remarks, u.nominee_name, u.balance
        FROM claims c
        JOIN users u ON c.user_id = u.id
        JOIN policies p ON c.policy_id = p.id
        WHERE c.status = 'Approved' OR c.status = 'approved';
    )";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) return exportList;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ExportClaimView ev;
        ev.claim_id = sqlite3_column_int(stmt, 0);
        ev.full_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) : "";
        ev.age = sqlite3_column_int(stmt, 2);
        ev.policy_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) : "";
        ev.amount = sqlite3_column_double(stmt, 4);
        ev.reason = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) : "";
        ev.admin_remarks = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)) : "";
        ev.nominee_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) ? reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)) : "";
        ev.user_balance = sqlite3_column_double(stmt, 8);
        exportList.push_back(ev);
    }
    sqlite3_finalize(stmt);
    return exportList;
}

#pragma once
#include "DatabaseManager.h"
#include <memory>

class Admin {
private:
    std::shared_ptr<DatabaseManager> db;

    void addPolicy();
    void viewAllPolicies();
    void viewAllClaims();
    void approveRejectClaim();

public:
    explicit Admin(std::shared_ptr<DatabaseManager> dbManager);
    void displayMenu();
};

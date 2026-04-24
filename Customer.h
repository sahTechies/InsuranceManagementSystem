#pragma once
#include "DatabaseManager.h"
#include <memory>

class Customer {
private:
    std::shared_ptr<DatabaseManager> db;
    int loggedInUserId;

    void viewAvailablePolicies();
    void purchasePolicy();
    void viewMyPolicies();
    void fileClaim();

public:
    Customer(std::shared_ptr<DatabaseManager> dbManager, int userId);
    void displayMenu();
};

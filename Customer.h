#pragma once

/**
 * @brief Class managing customer interactions
 * Handles policy viewing, dynamic pricing selection workflow, and claims.
 */
class Customer {
private:
    int loggedInUserId;

    void viewAvailablePolicies();
    void purchasePolicy();
    void viewMyPolicies();
    void fileClaim();

public:
    Customer(int userId);
    void displayMenu();
};

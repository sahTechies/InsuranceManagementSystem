#pragma once

/**
 * @brief Class managing administrative tasks
 * Handles policy creation, viewing global claims, and managing status.
 */
class Admin {
private:
    void addPolicy();
    void viewAllPolicies();
    void viewAllClaims();
    void approveRejectClaim();

public:
    Admin();
    void displayMenu();
};

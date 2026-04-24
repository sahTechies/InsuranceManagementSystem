#pragma once

/**
 * @brief Class encapsulating the workflow for an administrative user.
 */
class Admin {
public:
    Admin();
    void displayMenu();
    void addPolicy();
    void viewAllPolicies();
    void viewAllClaims();
    void approveRejectClaim();
    void exportApprovedClaims();
};

#pragma once

/**
 * @brief Class encapsulating the workflow for a logged-in customer.
 */
class Customer {
private:
    int loggedInUserId;

public:
    Customer(int userId);
    void displayMenu();
    void viewAvailablePolicies();
    void purchasePolicy();
    void viewMyPolicies();
    void fileClaim();
    void viewMyClaims();
    void showNotifications();
};

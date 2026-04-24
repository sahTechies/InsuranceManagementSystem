#include "Customer.h"
#include <iostream>
#include <limits>

Customer::Customer(std::shared_ptr<DatabaseManager> dbManager, int userId) 
    : db(std::move(dbManager)), loggedInUserId(userId) {}

void Customer::displayMenu() {
    int choice = 0;
    while (choice != 5) {
        std::cout << "\n--- Customer Menu ---\n";
        std::cout << "1. View Available Policies\n2. Purchase Policy\n3. View My Policies\n4. File a Claim\n5. Logout\nChoice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: viewAvailablePolicies(); break;
            case 2: purchasePolicy(); break;
            case 3: viewMyPolicies(); break;
            case 4: fileClaim(); break;
            case 5: std::cout << "Logging out...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

void Customer::viewAvailablePolicies() {
    auto policies = db->getAllPolicies();
    std::cout << "\n--- Available Policies ---\n";
    for (const auto& p : policies) {
        std::cout << "ID: " << p.id << " | Name: " << p.name 
                  << " | Premium: $" << p.premium << " | Coverage: $" << p.coverage << "\n";
    }
}

void Customer::purchasePolicy() {
    int policyId;
    std::cout << "Enter Policy ID to purchase: ";
    std::cin >> policyId;

    if (db->assignPolicyToUser(loggedInUserId, policyId, "Active")) {
        std::cout << "Policy purchased successfully.\n";
    } else {
        std::cout << "Failed to purchase policy.\n";
    }
}

// Why: Fulfills spec by querying user-specific policies which leverages JOIN
void Customer::viewMyPolicies() {
    auto myPolicies = db->getUserPolicies(loggedInUserId);
    std::cout << "\n--- My Policies ---\n";
    for (const auto& p : myPolicies) {
        std::cout << "Policy: " << p.policy_name << " | Category: " << p.category 
                  << " | Premium: $" << p.premium << " | Status: " << p.status << "\n";
    }
}

void Customer::fileClaim() {
    Claim c;
    c.user_id = loggedInUserId;
    std::cout << "Enter Policy ID for claim: ";
    std::cin >> c.policy_id;
    std::cout << "Enter Claim Amount: $";
    std::cin >> c.amount;
    c.status = "Pending";

    // Why: Prevent invalid claim amounts at the entry point
    if (c.amount < 0) {
        std::cout << "Error: Claim amount cannot be negative.\n";
        return;
    }

    if (db->createClaim(c)) {
        std::cout << "Claim filed successfully.\n";
    } else {
        std::cout << "Failed to file claim.\n";
    }
}

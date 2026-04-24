#include "Admin.h"
#include <iostream>
#include <limits>

Admin::Admin(std::shared_ptr<DatabaseManager> dbManager) : db(std::move(dbManager)) {}

// Why: Using a central loop keeps menu logic contained and testable.
void Admin::displayMenu() {
    int choice = 0;
    while (choice != 5) {
        std::cout << "\n--- Admin Menu ---\n";
        std::cout << "1. Add New Policy\n2. View All Policies\n3. View All Claims\n4. Approve/Reject Claim\n5. Logout\nChoice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: addPolicy(); break;
            case 2: viewAllPolicies(); break;
            case 3: viewAllClaims(); break;
            case 4: approveRejectClaim(); break;
            case 5: std::cout << "Logging out...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

void Admin::addPolicy() {
    Policy p;
    std::cout << "Enter name: ";
    std::cin.ignore();
    std::getline(std::cin, p.name);
    std::cout << "Enter category: ";
    std::getline(std::cin, p.category);
    std::cout << "Enter premium: ";
    std::cin >> p.premium;
    
    // Why: Validation step to enforce business rule from specification
    if (p.premium < 0) {
        std::cout << "Error: Premium cannot be negative.\n";
        return;
    }
    
    std::cout << "Enter coverage: ";
    std::cin >> p.coverage;

    if (db->createPolicy(p)) {
        std::cout << "Policy created successfully.\n";
    }
}

void Admin::viewAllPolicies() {
    auto policies = db->getAllPolicies();
    std::cout << "\n--- All Policies ---\n";
    for (const auto& p : policies) {
        std::cout << "ID: " << p.id << " | Name: " << p.name << " | Premium: $" << p.premium << "\n";
    }
}

void Admin::viewAllClaims() {
    auto claims = db->getAllClaims();
    std::cout << "\n--- All Claims ---\n";
    for (const auto& c : claims) {
        std::cout << "Claim ID: " << c.id << " | Policy ID: " << c.policy_id 
                  << " | Amount: $" << c.amount << " | Status: " << c.status << "\n";
    }
}

void Admin::approveRejectClaim() {
    int claimId;
    std::string status;
    std::cout << "Enter Claim ID: ";
    std::cin >> claimId;
    std::cout << "Enter status (Approved/Rejected): ";
    std::cin >> status;

    if (db->updateClaimStatus(claimId, status)) {
        std::cout << "Claim status updated to " << status << ".\n";
    }
}

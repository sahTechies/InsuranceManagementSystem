#include "Admin.h"
#include "DatabaseManager.h"
#include <iostream>
#include <limits>
#include <iomanip>

Admin::Admin() {}

/**
 * @brief Main loop for admin interactions handling non-numeric inputs.
 */
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

/**
 * @brief Adds a new base policy to the database.
 */
void Admin::addPolicy() {
    Policy p;
    std::cout << "Enter name: ";
    std::cin.ignore();
    std::getline(std::cin, p.name);
    std::cout << "Enter category (Life/Home/Motor): ";
    std::getline(std::cin, p.category);
    std::cout << "Enter base premium: ";
    if (!(std::cin >> p.base_premium) || p.base_premium < 0) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error: Premium must be a positive number.\n";
        return;
    }
    
    std::cout << "Enter base coverage: ";
    if (!(std::cin >> p.base_coverage)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error: Invalid coverage input.\n";
        return;
    }

    if (DatabaseManager::getInstance().createPolicy(p)) {
        std::cout << "Policy created successfully.\n";
    }
}

/**
 * @brief Views all policies structured via iomanip alignment.
 */
void Admin::viewAllPolicies() {
    auto policies = DatabaseManager::getInstance().getAllPolicies();
    std::cout << "\n--- All Policies ---\n";
    if (policies.empty()) { std::cout << "No policies found.\n"; return; }
    std::cout << std::left << std::setw(5) << "ID" << std::setw(20) << "Name" 
              << std::setw(15) << "Category" << std::setw(15) << "Base Premium" << "\n";
    for (const auto& p : policies) {
        std::cout << std::left << std::setw(5) << p.id << std::setw(20) << p.name 
                  << std::setw(15) << p.category << std::setw(15) << p.base_premium << "\n";
    }
}

/**
 * @brief Views all claims structured via iomanip alignment.
 */
void Admin::viewAllClaims() {
    auto claims = DatabaseManager::getInstance().getAllClaims();
    std::cout << "\n--- All Claims ---\n";
    if (claims.empty()) { std::cout << "No claims found.\n"; return; }
    std::cout << std::left << std::setw(10) << "Claim ID" << std::setw(10) << "User ID"
              << std::setw(15) << "Amount" << std::setw(15) << "Status" 
              << std::setw(30) << "Remarks" << "\n";
    for (const auto& c : claims) {
        std::cout << std::left << std::setw(10) << c.id << std::setw(10) << c.user_id 
                  << std::setw(15) << c.amount << std::setw(15) << c.status 
                  << std::setw(30) << c.admin_remarks << "\n";
    }
}

/**
 * @brief Updates a claim's status and adds admin remarks.
 */
void Admin::approveRejectClaim() {
    int claimId;
    std::string status, remarks;
    std::cout << "Enter Claim ID: ";
    if (!(std::cin >> claimId)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    std::cout << "Enter status (Approved/Rejected): ";
    std::cin >> status;
    std::cout << "Enter admin remarks: ";
    std::cin.ignore();
    std::getline(std::cin, remarks);

    if (DatabaseManager::getInstance().updateClaimStatus(claimId, status, remarks)) {
        std::cout << "Claim status updated successfully.\n";
    } else {
        std::cout << "Failed to update claim.\n";
    }
}

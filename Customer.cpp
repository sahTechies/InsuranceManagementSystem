#include "Customer.h"
#include "DatabaseManager.h"
#include <iostream>
#include <limits>
#include <iomanip>

Customer::Customer(int userId) : loggedInUserId(userId) {}

void Customer::displayMenu() {
    showNotifications();

    int choice = 0;
    while (choice != 6) {
        std::cout << "\n--- Customer Menu ---\n";
        std::cout << "1. View Available Policies\n2. Purchase Policy\n3. View My Policies\n4. File a Claim\n5. View My Claims\n6. Logout\nChoice: ";
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
            case 5: viewMyClaims(); break;
            case 6: std::cout << "Logging out...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

void Customer::showNotifications() {
    auto unnotified = DatabaseManager::getInstance().getUnnotifiedClaims(loggedInUserId);
    for (const auto& c : unnotified) {
        std::cout << "\n[!] Notification: Your claim for '" << c.policy_name 
                  << "' has been " << c.status << ".\n";
        DatabaseManager::getInstance().markClaimNotified(c.claim_id);
    }
}

void Customer::viewAvailablePolicies() {
    auto policies = DatabaseManager::getInstance().getAllPolicies();
    std::cout << "\n--- Available Policies ---\n";
    std::cout << std::left << std::setw(5) << "ID" << std::setw(20) << "Name" 
              << std::setw(15) << "Category" << std::setw(15) << "Base Premium" 
              << std::setw(15) << "Base Coverage" << "\n";
    for (const auto& p : policies) {
        std::cout << std::left << std::setw(5) << p.id << std::setw(20) << p.name 
                  << std::setw(15) << p.category << std::setw(15) << p.base_premium 
                  << std::setw(15) << p.base_coverage << "\n";
    }
}

void Customer::purchasePolicy() {
    int policyId;
    std::cout << "Enter Policy ID to purchase: ";
    if (!(std::cin >> policyId)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input.\n";
        return;
    }

    auto policyOpt = DatabaseManager::getInstance().getPolicyById(policyId);
    if (!policyOpt) {
        std::cout << "Policy not found.\n";
        return;
    }
    Policy p = policyOpt.value();
    
    double totalPremium = p.base_premium;
    std::string activeAddons = "";

    std::cout << "Base Premium: " << p.base_premium << "\n";

    if (p.category == "Life") {
        char ch;
        std::cout << "Add Accidental Death (+500)? (y/n): ";
        std::cin >> ch;
        if (ch == 'y' || ch == 'Y') { totalPremium += 500; activeAddons += "Accidental Death, "; }
        
        std::cout << "Add Critical Illness (+1500)? (y/n): ";
        std::cin >> ch;
        if (ch == 'y' || ch == 'Y') { totalPremium += 1500; activeAddons += "Critical Illness, "; }
    } else if (p.category == "Home") {
        char ch;
        std::cout << "Add Fire Damage (+500)? (y/n): ";
        std::cin >> ch;
        if (ch == 'y' || ch == 'Y') { totalPremium += 500; activeAddons += "Fire Damage, "; }
        
        std::cout << "Add Theft Protection (+400)? (y/n): ";
        std::cin >> ch;
        if (ch == 'y' || ch == 'Y') { totalPremium += 400; activeAddons += "Theft Protection, "; }
    }
    
    if (activeAddons.empty()) activeAddons = "None";
    
    std::cout << "Final Premium calculated: " << totalPremium << "\n";
    
    if (DatabaseManager::getInstance().assignPolicyToUser(loggedInUserId, policyId, totalPremium, activeAddons, "2027-01-01")) {
        std::cout << "Policy purchased successfully with selected add-ons.\n";
    } else {
        std::cout << "Failed to purchase policy.\n";
    }
}

void Customer::viewMyPolicies() {
    auto myPolicies = DatabaseManager::getInstance().getUserPolicies(loggedInUserId);
    std::cout << "\n--- My Policies ---\n";
    if (myPolicies.empty()) {
        std::cout << "No policies found.\n";
        return;
    }
    std::cout << std::left << std::setw(6) << "ID" << std::setw(20) << "Policy Name" << std::setw(15) << "Category" 
              << std::setw(15) << "Total Premium" << std::setw(15) << "Status" << std::setw(30) << "Active Add-ons" << "\n";
    std::cout << std::string(101, '-') << "\n";
    for (const auto& p : myPolicies) {
        std::cout << std::left << std::setw(6) << p.user_policy_id << std::setw(20) << p.policy_name << std::setw(15) << p.category 
                  << std::setw(15) << p.total_premium << std::setw(15) << p.status << std::setw(30) << p.active_addons << "\n";
    }
}

void Customer::fileClaim() {
    Claim c;
    c.user_id = loggedInUserId;
    std::cout << "Enter the Policy Record ID from your 'My Policies' list: ";
    if (!(std::cin >> c.policy_id)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }
    
    std::cout << "Enter Claim Amount: $";
    if (!(std::cin >> c.amount)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    if (c.amount < 0) {
        std::cout << "Error: Claim amount cannot be negative.\n";
        return;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    while (true) {
        std::cout << "Enter Reason for Claim: ";
        std::getline(std::cin, c.reason);
        if (!c.reason.empty()) {
            break;
        }
        std::cout << "Reason cannot be empty. Please describe the incident.\n";
    }

    c.status = "Pending";
    c.admin_remarks = "None";

    if (DatabaseManager::getInstance().createClaim(c)) {
        std::cout << "Claim filed successfully under review.\n";
    } else {
        std::cout << "Failed to file claim.\n";
    }
}

void Customer::viewMyClaims() {
    auto claims = DatabaseManager::getInstance().getUserClaims(loggedInUserId);
    if (claims.empty()) {
        std::cout << "\nNo claims found in your history.\n";
        return;
    }
    std::cout << "\n--- Your Claims History ---\n";
    std::cout << std::left << std::setw(6) << "ID" << std::setw(20) << "Policy" 
              << std::setw(15) << "Amount" << std::setw(15) << "Status" 
              << std::setw(30) << "Admin Remarks" << "\n";
    std::cout << std::string(86, '-') << "\n";
    for (const auto& c : claims) {
        std::cout << std::left << std::setw(6) << c.claim_id << std::setw(20) << c.policy_name 
                  << std::setw(15) << c.amount << std::setw(15) << c.status 
                  << std::setw(30) << c.admin_remarks << "\n";
    }
}

#include "Customer.h"
#include "DatabaseManager.h"
#include <iostream>
#include <limits>
#include <iomanip>

Customer::Customer(int userId) : loggedInUserId(userId) {}

/**
 * @brief Displays the main customer menu
 * Interacts with cin, handles invalid non-numeric inputs via cin.clear().
 */
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

/**
 * @brief Views all seeded available policies from the db using left-aligned setw.
 */
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

/**
 * @brief Implements dynamic pricing selection workflow (Add-ons).
 * Retrieves base policy, asks specific category-based prompts, and computes total.
 */
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

/**
 * @brief Views user specific policies showing active addons and total premium.
 */
void Customer::viewMyPolicies() {
    auto myPolicies = DatabaseManager::getInstance().getUserPolicies(loggedInUserId);
    std::cout << "\n--- My Policies ---\n";
    if (myPolicies.empty()) {
        std::cout << "No policies found.\n";
        return;
    }
    std::cout << std::left << std::setw(20) << "Policy" << std::setw(15) << "Category" 
              << std::setw(15) << "Total Premium" << std::setw(30) << "Active Add-ons" << "\n";
    for (const auto& p : myPolicies) {
        std::cout << std::left << std::setw(20) << p.policy_name << std::setw(15) << p.category 
                  << std::setw(15) << p.total_premium << std::setw(30) << p.active_addons << "\n";
    }
}

/**
 * @brief Files a claim linked to the current logged in user.
 */
void Customer::fileClaim() {
    Claim c;
    c.user_id = loggedInUserId;
    std::cout << "Enter Policy ID for claim: ";
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
    c.status = "Pending";
    c.admin_remarks = "None";

    if (c.amount < 0) {
        std::cout << "Error: Claim amount cannot be negative.\n";
        return;
    }

    if (DatabaseManager::getInstance().createClaim(c)) {
        std::cout << "Claim filed successfully.\n";
    } else {
        std::cout << "Failed to file claim.\n";
    }
}

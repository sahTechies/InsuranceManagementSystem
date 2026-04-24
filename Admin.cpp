#include "Admin.h"
#include "DatabaseManager.h"
#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <ctime>

Admin::Admin() {}

void Admin::displayMenu() {
    int choice = 0;
    while (choice != 6) {
        std::cout << "\n--- Admin Menu ---\n";
        std::cout << "1. Add New Policy\n2. View All Policies\n3. View All Claims\n4. Approve/Reject Claim\n5. Export Approved Claims to CSV\n6. Logout\nChoice: ";
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
            case 5: exportApprovedClaims(); break;
            case 6: std::cout << "Logging out...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }
}

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

void Admin::viewAllClaims() {
    auto claims = DatabaseManager::getInstance().getAllClaims();
    std::cout << "\n--- All Claims ---\n";
    if (claims.empty()) { std::cout << "No claims found.\n"; return; }
    std::cout << std::left << std::setw(10) << "Claim ID" << std::setw(10) << "User ID"
              << std::setw(15) << "Amount" << std::setw(30) << "Reason" << std::setw(15) << "Status" 
              << std::setw(30) << "Remarks" << "\n";
    for (const auto& c : claims) {
        std::cout << std::left << std::setw(10) << c.id << std::setw(10) << c.user_id 
                  << std::setw(15) << c.amount << std::setw(30) << c.reason << std::setw(15) << c.status 
                  << std::setw(30) << c.admin_remarks << "\n";
    }
}

void Admin::approveRejectClaim() {
    auto pending = DatabaseManager::getInstance().getPendingClaims();
    if (pending.empty()) {
        std::cout << "No pending claims to process.\n";
        return;
    }

    std::cout << "\n--- Pending Claims ---\n";
    std::cout << std::left << std::setw(10) << "Claim ID" << std::setw(20) << "Customer Name"
              << std::setw(20) << "Policy Type" << std::setw(15) << "Amount" << std::setw(30) << "Reason" << "\n";
    std::cout << std::string(95, '-') << "\n";
    for (const auto& c : pending) {
        std::cout << std::left << std::setw(10) << c.claim_id << std::setw(20) << c.full_name 
                  << std::setw(20) << c.policy_name << std::setw(15) << c.amount << std::setw(30) << c.reason << "\n";
    }

    int claimId;
    std::cout << "\nEnter Claim ID to process: ";
    if (!(std::cin >> claimId)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    auto it = std::find_if(pending.begin(), pending.end(), [claimId](const PendingClaimView& p) { return p.claim_id == claimId; });
    if (it == pending.end()) {
        std::cout << "Invalid or non-pending Claim ID.\n";
        return;
    }
    
    auto userOpt = DatabaseManager::getInstance().getUserById(it->user_id);
    if (!userOpt) {
        std::cout << "Failed to fetch user KYC profile.\n";
        return;
    }
    User u = userOpt.value();

    std::cout << "\n--- Pending Claim Review ---\n";
    std::cout << "Claim ID: " << claimId << "\n";
    std::cout << "Customer: " << u.full_name << " (Age: " << u.age << ")\n";
    std::cout << "Gender: " << u.gender << " | Marital Status: " << u.marital_status << "\n";
    std::cout << "Nominee: " << u.nominee_name << " | Father: " << u.father_name << " | Mother: " << u.mother_name << "\n";
    std::cout << "Policy: " << it->policy_name << "\n";
    std::cout << "Claim Amount: $" << it->amount << "\n";
    std::cout << "Reason: " << it->reason << "\n";
    std::cout << std::string(50, '-') << "\n";

    std::string status, remarks;
    std::cout << "Action (Approved/Rejected): ";
    std::cin >> status;
    std::cout << "Enter admin remarks: ";
    std::cin.ignore();
    std::getline(std::cin, remarks);

    if (DatabaseManager::getInstance().updateClaimStatus(claimId, status, remarks)) {
        std::cout << "Claim status updated successfully.\n";
        if (status == "Approved" || status == "approved") {
            DatabaseManager::getInstance().markUserPolicyClaimed(it->user_id, it->policy_id);
            DatabaseManager::getInstance().addBalanceToUser(it->user_id, it->amount);
        }
    } else {
        std::cout << "Failed to update claim.\n";
    }
}

void Admin::exportApprovedClaims() {
    auto claims = DatabaseManager::getInstance().getApprovedClaimsForExport();
    if (claims.empty()) {
        std::cout << "No approved claims to export.\n";
        return;
    }

    std::ofstream outFile("approved_claims_report.csv");
    if (!outFile.is_open()) {
        std::cout << "Error: Could not open file for writing.\n";
        return;
    }

    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    char dateBuf[20];
    std::strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d", std::localtime(&now_time));
    std::string currentDate(dateBuf);

    outFile << "ClaimID,CustomerName,Age,Policy,Amount,Reason,Remarks,Nominee,CurrentBalance,PayoutDate\n";

    for (const auto& c : claims) {
        outFile << c.claim_id << ",\"" 
                << c.full_name << "\"," 
                << c.age << ",\"" 
                << c.policy_name << "\"," 
                << c.amount << ",\"" 
                << c.reason << "\",\"" 
                << c.admin_remarks << "\",\"" 
                << c.nominee_name << "\"," 
                << c.user_balance << ",\"" 
                << currentDate << "\"\n";
    }
    outFile.close();

    std::cout << "[Success] Report generated: approved_claims_report.csv. You can now open this in Excel.\n";
}

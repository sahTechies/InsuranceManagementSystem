#pragma once
#include <string>
#include <vector>
#include <optional>

struct User {
    int id = 0;
    std::string username;
    std::string password;
    std::string role; 
    std::string full_name;
    int age = 0;
    std::string gender;
    std::string marital_status;
    std::string nominee_name;
    std::string father_name;
    std::string mother_name;
    double balance = 0.0;
};

struct Policy {
    int id = 0;
    std::string name;
    std::string category;
    double base_premium = 0.0;
    double base_coverage = 0.0;
};

struct UserPolicy {
    int id = 0;
    int user_id = 0;
    int policy_id = 0;
    double total_premium = 0.0;
    std::string active_addons;
    std::string expiry_date;
    std::string status;
};

struct Claim {
    int id = 0;
    int user_id = 0;
    int policy_id = 0;
    double amount = 0.0;
    std::string reason;
    std::string status;
    std::string admin_remarks;
    int is_notified = 0;
};

struct UserPolicyView {
    int user_policy_id = 0;
    int policy_id = 0;
    std::string policy_name;
    std::string category;
    double base_premium = 0.0;
    double total_premium = 0.0;
    std::string active_addons;
    std::string expiry_date;
    std::string status;
};

struct PendingClaimView {
    int claim_id = 0;
    int user_id = 0;
    int policy_id = 0;
    std::string full_name;
    std::string policy_name;
    double amount = 0.0;
    std::string reason;
};

struct UserClaimView {
    int claim_id = 0;
    std::string policy_name;
    double amount = 0.0;
    std::string status;
    std::string admin_remarks;
};

struct ExportClaimView {
    int claim_id = 0;
    std::string full_name;
    int age = 0;
    std::string policy_name;
    double amount = 0.0;
    std::string reason;
    std::string admin_remarks;
    std::string nominee_name;
    double user_balance = 0.0;
};

#pragma once
#include <string>
#include <vector>
#include <optional>

struct User {
    int id = 0;
    std::string username;
    std::string password;
    std::string role; 
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
};

struct Claim {
    int id = 0;
    int user_id = 0;
    int policy_id = 0;
    double amount = 0.0;
    std::string status;
    std::string admin_remarks;
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
};

#pragma once
#include <string>
#include <vector>
#include <optional>

struct User {
    int id = 0;
    std::string username;
    std::string password;
    std::string role; // "admin" or "customer"
};

struct Policy {
    int id = 0;
    std::string name;
    std::string category;
    double premium = 0.0;
    double coverage = 0.0;
};

struct UserPolicy {
    int id = 0;
    int user_id = 0;
    int policy_id = 0;
    std::string status;
};

struct Claim {
    int id = 0;
    int user_id = 0;
    int policy_id = 0;
    double amount = 0.0;
    std::string status;
};

struct UserPolicyView {
    int user_policy_id = 0;
    int policy_id = 0;
    std::string policy_name;
    std::string category;
    double premium = 0.0;
    double coverage = 0.0;
    std::string status;
};

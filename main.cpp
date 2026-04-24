#include "DatabaseManager.h"
#include "Admin.h"
#include "Customer.h"
#include <iostream>
#include <limits>

/**
 * @brief Authenticates users via DB query.
 */
void login() {
    std::string username, password;
    std::cout << "Username: ";
    std::cin >> username;
    std::cout << "Password: ";
    std::cin >> password;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // FLUSH

    auto user = DatabaseManager::getInstance().getUserByUsername(username);
    if (user && user->password == password) {
        if (user->role == "admin") {
            Admin admin;
            admin.displayMenu();
        } else {
            Customer customer(user->id);
            customer.displayMenu();
        }
    } else {
        std::cout << "Invalid credentials.\n";
    }
}

/**
 * @brief Collects KYC personal details when registering a customer.
 */
void collectKYC(User& u) {
    std::cout << "\n--- Personal Details (KYC) ---\n";
    // We expect the buffer to be clean when entering this function now.
    
    while (true) {
        std::cout << "Full Name: "; 
        std::getline(std::cin, u.full_name);
        if (!u.full_name.empty()) break;
    }
    
    while (true) {
        std::cout << "Age: ";
        if (std::cin >> u.age && u.age > 0) {
            break;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Error: Age must be a positive integer.\n";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    while (true) {
        std::cout << "Gender: "; std::getline(std::cin, u.gender);
        if (!u.gender.empty()) break;
    }
    while (true) {
        std::cout << "Marital Status: "; std::getline(std::cin, u.marital_status);
        if (!u.marital_status.empty()) break;
    }
    while (true) {
        std::cout << "Nominee Name: "; std::getline(std::cin, u.nominee_name);
        if (!u.nominee_name.empty()) break;
    }
    while (true) {
        std::cout << "Father's Name: "; std::getline(std::cin, u.father_name);
        if (!u.father_name.empty()) break;
    }
    while (true) {
        std::cout << "Mother's Name: "; std::getline(std::cin, u.mother_name);
        if (!u.mother_name.empty()) break;
    }
}

/**
 * @brief Creates a new user record checking for duplicates and KYC.
 */
void registerUser() {
    User u;
    std::cout << "Username: ";
    std::cin >> u.username;
    
    if (DatabaseManager::getInstance().getUserByUsername(u.username)) {
        std::cout << "Error: Username already exists.\n";
        // Ensure buffer is clean before returning to menu
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    std::cout << "Password: ";
    std::cin >> u.password;
    std::cout << "Role (admin/customer): ";
    std::cin >> u.role;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // FLUSH

    if (u.role == "customer") {
        collectKYC(u);
    } else {
        u.full_name = u.username;
        u.age = 0;
        u.gender = "N/A";
        u.marital_status = "N/A";
        u.nominee_name = "N/A";
        u.father_name = "N/A";
        u.mother_name = "N/A";
    }

    if (DatabaseManager::getInstance().createUser(u)) {
        std::cout << "User registered successfully.\n";
    }
}

/**
 * @brief Application entry point. Initializes Schema, sets up Singleton DB.
 */
int main() {
    if (!DatabaseManager::getInstance().connect("insurance_system.db")) {
        std::cerr << "Failed to connect to database.\n";
        return 1;
    }
    DatabaseManager::getInstance().initializeSchema();

    int choice = 0;
    while (choice != 3) {
        std::cout << "\n--- Professional Insurance Management System ---\n";
        std::cout << "1. Login\n2. Register\n3. Exit\nChoice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: login(); break;
            case 2: registerUser(); break;
            case 3: std::cout << "Exiting...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}

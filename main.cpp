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
 * @brief Creates a new user record checking for duplicates.
 */
void registerUser() {
    User u;
    std::cout << "Username: ";
    std::cin >> u.username;
    
    if (DatabaseManager::getInstance().getUserByUsername(u.username)) {
        std::cout << "Error: Username already exists.\n";
        return;
    }

    std::cout << "Password: ";
    std::cin >> u.password;
    std::cout << "Role (admin/customer): ";
    std::cin >> u.role;

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

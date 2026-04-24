#include "DatabaseManager.h"
#include "Admin.h"
#include "Customer.h"
#include <iostream>
#include <memory>
#include <limits>

// Why: Keeps the main loop small and handles specific login logic
void login(std::shared_ptr<DatabaseManager> db) {
    std::string username, password;
    std::cout << "Username: ";
    std::cin >> username;
    std::cout << "Password: ";
    std::cin >> password;

    auto user = db->getUserByUsername(username);
    if (user && user->password == password) {
        if (user->role == "admin") {
            Admin admin(db);
            admin.displayMenu();
        } else {
            Customer customer(db, user->id);
            customer.displayMenu();
        }
    } else {
        std::cout << "Invalid credentials.\n";
    }
}

// Why: Separation of concern for account registration
void registerUser(std::shared_ptr<DatabaseManager> db) {
    User u;
    std::cout << "Username: ";
    std::cin >> u.username;
    
    // Why: Validation step to avoid duplicate usernames
    if (db->getUserByUsername(u.username)) {
        std::cout << "Error: Username already exists.\n";
        return;
    }

    std::cout << "Password: ";
    std::cin >> u.password;
    std::cout << "Role (admin/customer): ";
    std::cin >> u.role;

    if (db->createUser(u)) {
        std::cout << "User registered successfully.\n";
    }
}

int main() {
    auto db = std::make_shared<DatabaseManager>();
    if (!db->connect("insurance_system.db")) {
        std::cerr << "Failed to connect to database.\n";
        return 1;
    }
    
    db->initializeSchema();

    int choice = 0;
    while (choice != 3) {
        std::cout << "\n--- Insurance Management System ---\n";
        std::cout << "1. Login\n2. Register\n3. Exit\nChoice: ";
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: login(db); break;
            case 2: registerUser(db); break;
            case 3: std::cout << "Exiting...\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    }

    return 0;
}

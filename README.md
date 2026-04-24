# Professional C++ Insurance Management System (IMS)

A modular, console-based C++ application for managing insurance policies, customers, and claims. This project leverages SQLite for persistent data storage and is meticulously designed with clean architecture principles to separate the underlying database implementation from the core business logic.

## ✨ Key Features

- **Singleton Database Security:** Global, thread-safe access to the database using the Singleton pattern. All SQL queries use prepared statements (`sqlite3_bind_*`) to prevent SQL Injection attacks.
- **Auto-Seeding:** Automatically populates the database with initial Life, Home, and Motor insurance policies upon the first run.
- **Dynamic Pricing & Add-on Workflows:** Interactive policy purchase flow. Based on the selected policy category (Life or Home), the system automatically prompts the user for optional riders (e.g., Accidental Death, Fire Damage) and dynamically calculates the final premium.
- **Clean UI & Robust Error Handling:** Neatly formatted data tables utilizing `<iomanip>` `std::setw`. Infinite-loop protection via `std::cin.clear()` and `std::cin.ignore()` guarantees the app doesn't crash on invalid user inputs.

## 🗂️ File Structure & Architecture

Here is a breakdown of the files that make up the system and what each specific file is responsible for:

### 1. Data Models
* **`Models.h`**
  Defines the core data structures (POCOs - Plain Old C++ Objects) used throughout the system. It contains structures like `User`, `Policy`, `Claim`, `UserPolicy`, and `UserPolicyView`. These act as memory-safe Data Transfer Objects (DTOs) ensuring type safety when moving data between the database and the application layers.

### 2. Database Layer
* **`DatabaseManager.h`**
  The header file defining the public API for all database interactions. It exposes a Singleton instance (`getInstance()`) and abstracts away SQLite specifics, meaning the rest of the application only deals with C++ objects.
* **`DatabaseManager.cpp`**
  The concrete implementation of the database operations using `sqlite3`. It handles establishing connections, executing prepared SQL queries, auto-seeding data, initializing the schema on the first run, and mapping SQL rows back to the C++ structures defined in `Models.h`.

### 3. Business Logic
* **`Admin.h` & `Admin.cpp`**
  Contains the workflows specific to administrative users. It interacts with the `DatabaseManager` Singleton to execute tasks safely. Responsibilities include adding new base policies, viewing all users' policies, viewing global claims, and approving/rejecting pending claims with administrative remarks.
* **`Customer.h` & `Customer.cpp`**
  Contains the customer-facing workflows. It tracks the currently logged-in user's ID to provide session security. Responsibilities include viewing available policies, engaging in the interactive dynamic pricing Add-on workflow to purchase customized policies, viewing their specific purchased policies (using SQL `JOIN`s), and filing new claims.

### 4. Application Entry & Build
* **`main.cpp`**
  The entry point of the application. It is responsible for setting up the `DatabaseManager`, initializing the schema, and handling the core authentication loop (Login and Registration). Depending on the credentials and user role, it routes the flow to either the `Admin` or `Customer` menus via a `switch-case` architecture.
* **`Makefile`**
  A build configuration file that tells the `make` utility how to compile the project. It uses `g++` (C++17) and links the required `-lsqlite3` library, producing the final executable binary named `ims`.
* **`PROJECT_SPEC.md`**
  The original specification document outlining the strict requirements, database schema, constraints, dynamic pricing logic, and architecture of the project.

## 🚀 How to Run

1. Open your terminal in the project directory.
2. Compile the code by running:
   ```bash
   make
   ```
3. Execute the compiled application:
   ```bash
   ./ims
   ```

To clean up the compiled object files and the executable, you can run:
```bash
make clean
```

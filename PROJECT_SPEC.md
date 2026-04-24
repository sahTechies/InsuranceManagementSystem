# Project: C++ Console Insurance Management System (IMS)
## Database: SQLite3 (insurance_system.db)

### 1. File Architecture
The agent must follow a modular structure:
- `main.cpp`: Entry point and main menu loop.
- `DatabaseManager.h/cpp`: SQLite connection and raw SQL execution.
- `Admin.h/cpp`: Logic for administrative tasks.
- `Customer.h/cpp`: Logic for customer-facing tasks.
- `Models.h`: Data structures (User, Policy, Claim).

### 2. Database Schema
- **users**: id(INT PK), username(TEXT), password(TEXT), role(TEXT)
- **policies**: id(INT PK), name(TEXT), category(TEXT), premium(REAL), coverage(REAL)
- **user_policies**: id(INT PK), user_id(FK), policy_id(FK), status(TEXT)
- **claims**: id(INT PK), user_id(FK), policy_id(FK), amount(REAL), status(TEXT)

### 3. Core Logic Requirements
- **Validation:** No negative numbers for premiums; no duplicate usernames.
- **Security:** Basic session management (track logged-in user ID).
- **Queries:** Use JOINs to show customers their specific policy names rather than just IDs.

### 4. Build Instructions
- Use `g++` with the `-lsqlite3` flag.
- Provide a simple `Makefile` for compilation.
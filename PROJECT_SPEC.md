# Project: Professional C++ Insurance Management System (SQLite3)

## 1. Modular Architecture
- `main.cpp`: CLI Menu system (switch-case).
- `DatabaseManager.h/cpp`: Singleton class for SQLite logic. Uses prepared statements to prevent SQL injection.
- `Admin.h/cpp`: Logic for policy creation and claim approval.
- `Customer.h/cpp`: Logic for purchasing with add-ons and filing claims.
- `Models.h`: Structs for `User`, `Policy`, `Claim`, and `UserPolicy`.

## 2. Enhanced Database Schema
- **users**: id(PK), username, password, role.
- **policies**: id(PK), name, category, base_premium, base_coverage.
- **user_policies**: id(PK), user_id(FK), policy_id(FK), total_premium, active_addons(TEXT), expiry_date.
- **claims**: id(PK), user_id(FK), policy_id(FK), amount, status, admin_remarks.

## 3. Mandatory Policy Data (Seed Data)
Upon first run, the system must auto-populate the `policies` table with:
- **Life Insurance:** - 'Term Life' (Base: 5000), 'Whole Life' (Base: 15000), 'Endowment' (Base: 12000).
- **Home Insurance:** - 'Structure Secure' (Base: 3000), 'Content Guard' (Base: 2000).
- **Motor Insurance:** - 'Third Party' (Base: 2500), 'Zero Dep' (Base: 7000).

## 4. Dynamic Pricing & Add-on Logic
The `Customer::purchasePolicy()` function must implement a "Selection Workflow":
1. **Selection:** User picks a base policy from the table.
2. **Add-ons:** System prompts for optional riders:
   - **If Category == 'Life':** Offer 'Accidental Death' (+₹500), 'Critical Illness' (+₹1500).
   - **If Category == 'Home':** Offer 'Fire Damage' (+₹500), 'Theft Protection' (+₹400).
3. **Calculation:** $TotalPremium = BasePremium + \sum(Addons)$.
4. **Persistence:** Save the final calculated premium and a string of active add-ons to `user_policies`.

## 5. Software Engineering Standards
- **Memory Safety:** Use `std::vector<Policy>` to return data from the DB; no raw arrays.
- **Clean UI:** Use `std::setw` from `<iomanip>` to print tables. Do not allow "empty" table views.
- **Error Handling:** If a user enters a character in a numeric menu (e.g., 'g'), use `cin.clear()` and `cin.ignore()` to prevent infinite loops.
- **Comments:** Every function must have a header explaining its purpose and SQL interactions.
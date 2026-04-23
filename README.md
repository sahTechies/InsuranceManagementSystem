# Insurance Management System

A command-line Insurance Management System written in **C++** with **SQLite** persistent storage. The system lets you register clients, issue insurance policies (Health, Auto, Life), calculate premiums using actuarial formulas, and export records to CSV.

---

## Table of Contents

- [Features](#features)
- [Tech Stack](#tech-stack)
- [Prerequisites](#prerequisites)
- [Build Instructions](#build-instructions)
- [Running the Application](#running-the-application)
- [Sample Workflow](#sample-workflow)
- [Menu Options](#menu-options)
- [Policy Types](#policy-types)
- [Premium Calculation](#premium-calculation)
- [Data Validation Rules](#data-validation-rules)
- [Database Schema](#database-schema)
- [CSV Export Format](#csv-export-format)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

---

## Features

- **Client Registration** – Capture full KYC details including name, age, DOB, contact info, government ID, nominee, income, and medical history.
- **Policy Issuance** – Issue Health, Auto, or Life insurance policies linked to a registered client.
- **Actuarial Premium Calculator** – Compute Net Premium and Gross Premium using mortality rate, interest rate, expense/contingency/profit loadings.
- **Paid-Up & Surrender Value** – Calculate paid-up value and surrender value for applicable life products.
- **Policy Deactivation** – Mark any policy record as inactive without deleting it.
- **Search Client** – Look up a client's full profile and attached policies by Client ID.
- **Display All Records** – View all registered clients and their policies in the console.
- **Delete Client** – Remove a client and all associated policies from the database.
- **CSV Export** – Export all client and policy data to `insurance_export.csv`.
- **Demo Data Reset** – Wipe the database and seed one demo client for testing.
- **Persistent Storage** – All data is stored in an SQLite database (`insurance_data.db`) that survives restarts.

---

## Tech Stack

| Component | Technology |
|-----------|-----------|
| Language  | C++ (C++11 or later) |
| Database  | SQLite 3 |
| Build     | `g++` (GCC) |
| Platform  | Linux (uses `/proc/self/exe` for path resolution; macOS requires adaptation) |

---

## Prerequisites

- **g++** (GCC 7+) with C++11 support
- **SQLite 3** development libraries

### Install dependencies on Ubuntu / Debian

```bash
sudo apt update
sudo apt install g++ libsqlite3-dev
```

### Install dependencies on macOS (Homebrew)

```bash
brew install gcc sqlite
```

---

## Build Instructions

Clone the repository and compile `application.cpp`:

```bash
g++ -std=c++11 -o application application.cpp -lsqlite3
```

A pre-built binary (`application`) is included in the repository for convenience, but it is recommended to build from source to ensure compatibility with your system.

---

## Running the Application

```bash
./application
```

On first launch the application creates `insurance_data.db` in the same directory as the executable and reports how many clients were loaded.

```
Loaded 0 client(s) from SQLite database: /path/to/insurance_data.db
```

---

## Sample Workflow

Below is a quick end-to-end example that registers a client and issues a Life policy.

**Step 1 – Add a client (Option 1)**

```
Select option: 1
Enter Client Name: Jane Doe
Enter Date of Birth (DD-MM-YYYY): 15-06-1990
Enter Age: 34
Enter Phone Number (10 digits): 9876543210
Enter Email: jane@example.com
Enter Father's Name: John Doe
Enter Mother's Name: Mary Doe
Married? (y/n): n
Enter Address: 42 Main Street, Mumbai
Enter Occupation: Software Engineer
Enter Government ID (PAN/Aadhaar/Passport): ABCDE1234F
Enter Nominee Name: John Doe
Enter Nominee Relation: Father
Enter Annual Income: 1200000
Smoker? (y/n): n
Enter Medical History / Pre-existing Conditions: None
Client added successfully with ID: 101
```

**Step 2 – Purchase a Life policy (Option 2 → type 3)**

```
Select option: 2
Registered Clients:
ID: 101 | Name: Jane Doe

Enter Client ID to add policy: 101
Available Policies: 1. Health  2. Auto  3. Life
Select policy type (1-3): 3
Enter Coverage Amount (any decimal): 500000
Do you want monthly payment? (y/n): n

========== INSURANCE CONTRACT ==========
Client Name      : Jane Doe
Client ID        : 101
Policy Record ID : 1001
Policy ID        : 5003
Policy Category  : Life
Policy Plan      : Life
Coverage Amount  : ₹500000.00
Payment Mode     : Yearly
Premium Payable  : ₹8260.00 per year
Contract Status  : ACTIVE
========================================
```

> **Premium breakdown:** Base = `500000 × 0.01 × 1.4 = ₹7000`; Age risk factor for age 34 = `1 + (34 − 25) × 0.02 = 1.18`; Final = `7000 × 1.18 = ₹8260`. Actual value will depend on the exact age entered.

**Step 3 – Export records (Option 5)**

```
Select option: 5
Export complete. Records saved to: /path/to/insurance_export.csv
```

**Step 4 – Exit (Option 7)**

```
Select option: 7
```

---

## Menu Options

```
--- INSURANCE MANAGEMENT CONSOLE ---
1. Add Client
2. Purchase Policy
3. Display All Records
4. Delete Client Record
5. Export Records (CSV)
6. Deactivate Policy
7. Exit
8. Reset Demo Data
9. Search Client Record
```

| Option | Description |
|--------|-------------|
| **1. Add Client** | Register a new client (full KYC). A unique Client ID (starting at 101) is assigned automatically. |
| **2. Purchase Policy** | Link a Health, Auto, or Life policy to an existing client. Displays a printed insurance contract on completion. |
| **3. Display All Records** | Print every client's full profile and all attached policies. |
| **4. Delete Client Record** | Permanently remove a client and all their policies by Client ID. |
| **5. Export Records (CSV)** | Write all records to `insurance_export.csv` in the executable's directory. |
| **6. Deactivate Policy** | Mark a specific policy record as Inactive (the record is kept in the database). |
| **7. Exit** | Save data and quit. |
| **8. Reset Demo Data** | Delete all records and insert one sample demo client — useful for testing. |
| **9. Search Client Record** | Display the full profile of a single client by Client ID. |

---

## Policy Types

All policy types are accessed through **Menu Option 2 (Purchase Policy)**.  
After selecting a client you will be prompted to choose a policy type (1 = Health, 2 = Auto, 3 = Life).

### Health (Menu Option 2 → sub-option 1)

Six health/life product plans are available:

| Plan | Description |
|------|-------------|
| **Pure Term** | Death benefit only; no maturity value. |
| **TROP** (Term with Return of Premium) | Premiums returned at maturity if the insured survives. |
| **Whole Life** | Life-long cover; supports paid-up value and surrender value calculations. |
| **Endowment** | Death benefit + savings; supports paid-up value and surrender value. |
| **Money-Back** | Periodic survival payouts during the policy term. |
| **ULIP** | Unit-linked plan with equity allocation and expected market return inputs. |

### Auto (Menu Option 2 → sub-option 2)

Generic auto insurance with coverage amount input. Base premium = 1% of coverage amount.

### Life (Menu Option 2 → sub-option 3)

Generic life insurance. Base premium = 1.4% of coverage amount.

---

## Premium Calculation

### Base Premium by Policy Type

```
Base Premium = Coverage Amount × 0.01 × Type Factor
  Health  →  Type Factor = 1.2
  Auto    →  Type Factor = 1.0
  Life    →  Type Factor = 1.4
```

### Actuarial Net Premium (Health products)

```
Net Premium = (Sum Assured × Mortality Rate qx) / (1 + Interest Rate i)
```

### Gross Premium

```
Gross Premium = Net Premium
              + Net Premium × Expense Loading %
              + Net Premium × Contingency Margin %
              + Net Premium × Profit Loading %
```

### Age Risk Factor (Auto / Life)

```
Risk Factor = 1.0 + max(0, Age - 25) × 0.02
Final Premium = Base Premium × Risk Factor
```

### Paid-Up Value (Whole Life / Endowment / Money-Back)

```
Paid-Up Value = Sum Assured × (Premiums Paid / Total Premiums Payable)
```

### Surrender Value

```
Surrender Value = (Paid-Up Value + Accrued Bonuses) × (Surrender Factor % / 100)
```

---

## Data Validation Rules

| Field | Validation |
|-------|-----------|
| **Date of Birth** | Format `DD-MM-YYYY`; calendar-valid (leap year aware); year ≥ 1900. |
| **Phone Number** | Exactly 10 digits; must start with 6, 7, 8, or 9 (Indian mobile numbers). |
| **Email** | Must contain exactly one `@`, at least one `.` after `@`, no spaces. |
| **Government ID** | Accepts **PAN** (format `ABCDE1234F`: 5 letters + 4 digits + 1 letter), **Aadhaar** (12 digits), or **Passport** (format `A1234567`: 1 letter + 7 digits). Stored in uppercase. |
| **Age** | Integer between 1 and 120. |
| **Coverage / Premium / Income** | Must be ≥ 0.01 (coverage) or ≥ 0 (income). |
| **Mortality / Interest Rate** | Decimal between 0.0 and 1.0. |
| **Loading rates** | Percentage between 0.0 and 100.0. |

---

## Database Schema

All data is persisted in `insurance_data.db` (SQLite 3). The database contains two tables.

### `clients`

| Column | Type | Description |
|--------|------|-------------|
| `clientID` | INTEGER PK | Auto-assigned starting at 101 |
| `name` | TEXT | Full name |
| `age` | INTEGER | Age in years (1–120) |
| `dateOfBirth` | TEXT | Format `DD-MM-YYYY` |
| `phoneNumber` | TEXT | 10-digit Indian mobile number |
| `email` | TEXT | Email address |
| `fatherName` | TEXT | Father's name |
| `motherName` | TEXT | Mother's name |
| `isMarried` | INTEGER | 0 = No, 1 = Yes |
| `spouseName` | TEXT | Spouse name (`N/A` if unmarried) |
| `address` | TEXT | Residential address |
| `occupation` | TEXT | Occupation / job title |
| `governmentID` | TEXT | PAN / Aadhaar / Passport (uppercase) |
| `nomineeName` | TEXT | Policy nominee's name |
| `nomineeRelation` | TEXT | Relationship to nominee |
| `annualIncome` | REAL | Annual income (≥ 0) |
| `isSmoker` | INTEGER | 0 = No, 1 = Yes |
| `medicalHistory` | TEXT | Pre-existing conditions |

### `policies`

| Column | Type | Description |
|--------|------|-------------|
| `recordID` | INTEGER PK | Auto-assigned starting at 1001 |
| `clientID` | INTEGER FK | References `clients.clientID` |
| `policyID` | INTEGER | Policy type code (5001 Health, 5002 Auto, 5003 Life) |
| `category` | TEXT | `Health`, `Auto`, or `Life` |
| `planName` | TEXT | Plan name (e.g., `Pure Term`, `Whole Life`) |
| `coverageAmount` | REAL | Sum assured / coverage amount |
| `basePremium` | REAL | Computed base premium |
| `paymentMode` | TEXT | `Yearly` or `Monthly` |
| `premiumTermYears` | INTEGER | Number of years premiums are paid |
| `policyTermYears` | INTEGER | Total policy duration in years |
| `mortalityRate` | REAL | Mortality rate qx (0.0 – 1.0) |
| `interestRate` | REAL | Interest rate i (0.0 – 1.0) |
| `expensesRate` | REAL | Expense loading % (0.0 – 100.0) |
| `contingencyRate` | REAL | Contingency margin % (0.0 – 100.0) |
| `profitRate` | REAL | Profit loading % (0.0 – 100.0) |
| `accruedBonuses` | REAL | Accrued bonuses for surrender calculations |
| `surrenderValueFactor` | REAL | Surrender factor % used in surrender value |
| `isActive` | INTEGER | 0 = Inactive, 1 = Active |

---

## CSV Export Format

Option 5 writes every client–policy combination to `insurance_export.csv` in the executable's directory. Each row represents one policy. Clients with no policies get a single placeholder row.

```
ClientID, ClientName, Age, DOB, Phone, Email, FatherName, MotherName,
Married, SpouseName, Address, Occupation, GovernmentID, NomineeName,
NomineeRelation, AnnualIncome, Smoker, MedicalHistory,
PolicyRecordID, PolicyID, PolicyCategory, PolicyPlan,
PaymentMode, CoverageAmount, BasePremium, FinalPremium, Status
```

| Column | Description |
|--------|-------------|
| `ClientID` – `MedicalHistory` | Client KYC fields (see Database Schema above) |
| `PolicyRecordID` | Unique policy record identifier |
| `PolicyID` | Policy type code |
| `PolicyCategory` | `Health`, `Auto`, or `Life` |
| `PolicyPlan` | Plan name |
| `PaymentMode` | `Yearly` or `Monthly` |
| `CoverageAmount` | Sum assured |
| `BasePremium` | Base premium before age risk factor |
| `FinalPremium` | Base premium × age risk factor |
| `Status` | `Active` or `Inactive` |

> Clients with no policies have `N/A` in all policy columns and `0` for numeric policy fields.

---

## Project Structure

```
InsuranceManagementSystem/
├── application.cpp        # Full C++ source code
├── application            # Pre-built Linux binary
├── insurance_data.db      # SQLite database (auto-created on first run)
├── insurance_export.csv   # CSV export output (created on Option 5)
├── LICENSE                # MIT License
└── README.md              # This file
```

---

## Contributing

Contributions are welcome! Feel free to open an issue or submit a pull request. Areas actively being improved:

- Additional policy categories (Travel, Home, Marine)
- Report generation (PDF / HTML)
- GUI front-end (Qt or web-based)
- Unit test coverage

---

## License

This project is released under the [MIT License](LICENSE).

See the `LICENSE` file for the full license text.


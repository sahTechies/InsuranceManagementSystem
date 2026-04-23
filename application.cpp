#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <functional>
#include <unistd.h>
#include <sqlite3.h>

using namespace std;

// --- Components 1 & 2: Client and Policy Structure ---
class Policy {
public:
    int recordID;
    int policyID;
    string category; // e.g., Health, Life, Auto
    string planName;
    double coverageAmount;
    double basePremium;
    string paymentMode;
    int premiumTermYears;
    int policyTermYears;
    double mortalityRate;
    double interestRate;
    double expensesRate;
    double contingencyRate;
    double profitRate;
    double accruedBonuses;
    double surrenderValueFactor;
    bool isActive;

    Policy(int record, int id, const string& cat, const string& plan, double coverage, double base)
        : recordID(record),
          policyID(id),
          category(cat),
          planName(plan),
          coverageAmount(coverage),
          basePremium(base),
          paymentMode("Yearly"),
          premiumTermYears(1),
          policyTermYears(1),
          mortalityRate(0.0),
          interestRate(0.0),
          expensesRate(0.0),
          contingencyRate(0.0),
          profitRate(0.0),
          accruedBonuses(0.0),
          surrenderValueFactor(0.0),
          isActive(true) {}
};

class Client {
public:
    int clientID;
    string name;
    int age;
    string dateOfBirth;
    string phoneNumber;
    string email;
    string fatherName;
    string motherName;
    bool isMarried;
    string spouseName;
    string address;
    string occupation;
    string governmentID;
    string nomineeName;
    string nomineeRelation;
    double annualIncome;
    bool isSmoker;
    string medicalHistory;
    vector<Policy> activePolicies;

    Client(
        int id,
        const string& n,
        int a,
        const string& dob,
        const string& phone,
        const string& mail,
        const string& father,
        const string& mother,
        bool married,
        const string& spouse,
        const string& addr,
        const string& job,
        const string& govID,
        const string& nominee,
        const string& nomineeRel,
        double income,
        bool smoker,
        const string& medical
    )
        : clientID(id),
          name(n),
          age(a),
          dateOfBirth(dob),
          phoneNumber(phone),
          email(mail),
          fatherName(father),
          motherName(mother),
          isMarried(married),
          spouseName(spouse),
          address(addr),
          occupation(job),
          governmentID(govID),
          nomineeName(nominee),
          nomineeRelation(nomineeRel),
          annualIncome(income),
          isSmoker(smoker),
          medicalHistory(medical) {}

    // --- Component 3: Premium Calculator Logic ---
    // Example math: Premium increases by 2% for every year over 25
    double calculateTotalPremium(const Policy& p) const {
        double riskFactor = 1.0;
        if (age > 25) {
            riskFactor = 1.0 + ((age - 25) * 0.02);
        }
        return p.basePremium * riskFactor;
    }

    void displayDetails() const {
        cout << "\nClient: " << name << " (ID: " << clientID << ")";
        cout << "\nAge: " << age;
        cout << "\nDOB: " << dateOfBirth;
        cout << "\nPhone: " << phoneNumber;
        cout << "\nEmail: " << email;
        cout << "\nFather Name: " << fatherName;
        cout << "\nMother Name: " << motherName;
        cout << "\nMarried: " << (isMarried ? "Yes" : "No");
        if (isMarried) {
            cout << "\nSpouse Name: " << spouseName;
        }
        cout << "\nAddress: " << address;
        cout << "\nOccupation: " << occupation;
        cout << "\nGovernment ID: " << governmentID;
        cout << "\nNominee: " << nomineeName << " (" << nomineeRelation << ")";
        cout << "\nAnnual Income: ₹" << fixed << setprecision(2) << annualIncome;
        cout << "\nSmoker: " << (isSmoker ? "Yes" : "No");
        cout << "\nMedical History: " << medicalHistory;
        cout << "\nPolicies: ";
        for (const auto &p : activePolicies) {
            cout << "\n - [" << p.recordID << "] " << p.category << " / " << p.planName;
            cout << " | Status: " << (p.isActive ? "Active" : "Inactive");
            cout << " | Payment: " << p.paymentMode;
            cout << " | Coverage: ₹" << fixed << setprecision(2) << p.coverageAmount;
            cout << " | Premium: ₹" << calculateTotalPremium(p);
        }
        cout << "\n--------------------------" << endl;
    }
};

// --- Component 4: System Management ---
class InsuranceSystem {
private:
    vector<Client> database;
    int nextID = 101;
    int nextPolicyRecordID = 1001;
    string databaseFile;
    string exportFile;
    sqlite3* db = nullptr;

    Client* findClientById(int id) {
        for (auto &c : database) {
            if (c.clientID == id) {
                return &c;
            }
        }
        return nullptr;
    }

    void printRegisteredClients() const {
        cout << "\nRegistered Clients:" << endl;
        for (const auto &c : database) {
            cout << "ID: " << c.clientID << " | Name: " << c.name << endl;
        }
    }

    void writeClientCsvPrefix(ostream& out, const Client& c) const {
        out << c.clientID << "," << escapeCsvField(c.name) << "," << c.age << ","
            << escapeCsvField(c.dateOfBirth) << "," << escapeCsvField(c.phoneNumber) << "," << escapeCsvField(c.email) << ","
            << escapeCsvField(c.fatherName) << "," << escapeCsvField(c.motherName) << ","
            << escapeCsvField(c.isMarried ? "Yes" : "No") << "," << escapeCsvField(c.spouseName) << ","
            << escapeCsvField(c.address) << "," << escapeCsvField(c.occupation) << "," << escapeCsvField(c.governmentID) << ","
            << escapeCsvField(c.nomineeName) << "," << escapeCsvField(c.nomineeRelation) << ","
            << c.annualIncome << ","
            << escapeCsvField(c.isSmoker ? "Yes" : "No") << "," << escapeCsvField(c.medicalHistory) << ",";
    }

    string escapeCsvField(const string& value) const {
        if (value.find_first_of(",\"\n\r") == string::npos) {
            return value;
        }
        string escaped;
        escaped.reserve(value.size() + 2);
        escaped.push_back('"');
        for (char ch : value) {
            if (ch == '"') {
                escaped.push_back('"');
                escaped.push_back('"');
            } else {
                escaped.push_back(ch);
            }
        }
        escaped.push_back('"');
        return escaped;
    }

    template <typename T>
    void readNumericInput(const string& prompt, T& value) {
        while (true) {
            cout << prompt;
            if (cin >> value) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return;
            }
            cout << "Invalid input. Please enter a valid number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    template <typename T>
    void readNumericInputInRange(const string& prompt, T& value, T minValue, T maxValue) {
        while (true) {
            readNumericInput(prompt, value);
            if (value >= minValue && value <= maxValue) {
                return;
            }
            cout << "Value out of range. Please enter a value between "
                 << minValue << " and " << maxValue << "." << endl;
        }
    }

    template <typename T>
    void readNumericInputAtLeast(const string& prompt, T& value, T minValue) {
        while (true) {
            readNumericInput(prompt, value);
            if (value >= minValue) {
                return;
            }
            cout << "Value out of range. Please enter a value greater than or equal to "
                 << minValue << "." << endl;
        }
    }

    bool readYesNoInput(const string& prompt) {
        string input;
        while (true) {
            cout << prompt;
            if (!(cin >> input)) {
                cout << "Invalid input. Please enter y or n." << endl;
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                continue;
            }

            if (input.size() == 1 && (input[0] == 'y' || input[0] == 'Y')) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return true;
            }
            if (input.size() == 1 && (input[0] == 'n' || input[0] == 'N')) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return false;
            }

            cout << "Invalid choice. Please enter y or n." << endl;
        }
    }

    bool hasOnlyDigits(const string& value) const {
        if (value.empty()) {
            return false;
        }
        for (char ch : value) {
            if (!isdigit(static_cast<unsigned char>(ch))) {
                return false;
            }
        }
        return true;
    }

    bool isLeapYear(int year) const {
        return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    }

    bool isValidDateOfBirth(const string& dob) const {
        if (dob.size() != 10 || dob[2] != '-' || dob[5] != '-') {
            return false;
        }

        string dayPart = dob.substr(0, 2);
        string monthPart = dob.substr(3, 2);
        string yearPart = dob.substr(6, 4);

        if (!hasOnlyDigits(dayPart) || !hasOnlyDigits(monthPart) || !hasOnlyDigits(yearPart)) {
            return false;
        }

        int day = stoi(dayPart);
        int month = stoi(monthPart);
        int year = stoi(yearPart);

        if (year < 1900 || month < 1 || month > 12 || day < 1) {
            return false;
        }

        int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        if (month == 2 && isLeapYear(year)) {
            return day <= 29;
        }
        return day <= daysInMonth[month - 1];
    }

    bool isValidPhoneNumber(const string& phone) const {
        if (phone.size() != 10 || !hasOnlyDigits(phone)) {
            return false;
        }
        return phone[0] >= '6' && phone[0] <= '9';
    }

    bool isValidEmail(const string& email) const {
        size_t atPos = email.find('@');
        if (atPos == string::npos || atPos == 0 || atPos == email.size() - 1) {
            return false;
        }

        if (email.find(' ') != string::npos) {
            return false;
        }

        if (email.find('@', atPos + 1) != string::npos) {
            return false;
        }

        size_t dotAfterAt = email.find('.', atPos + 1);
        return dotAfterAt != string::npos && dotAfterAt < email.size() - 1;
    }

    string trim(const string& value) const {
        size_t start = value.find_first_not_of(" \t\n\r");
        if (start == string::npos) {
            return "";
        }
        size_t end = value.find_last_not_of(" \t\n\r");
        return value.substr(start, end - start + 1);
    }

    string toUpperCopy(const string& value) const {
        string result = value;
        transform(result.begin(), result.end(), result.begin(),
                  [](unsigned char ch) { return static_cast<char>(toupper(ch)); });
        return result;
    }

    void readNonEmptyLine(const string& prompt, string& value) {
        while (true) {
            cout << prompt;
            getline(cin, value);
            value = trim(value);
            if (!value.empty()) {
                return;
            }
            cout << "This field cannot be empty." << endl;
        }
    }

    void readValidatedLine(
        const string& prompt,
        string& value,
        const function<bool(const string&)>& validator,
        const string& errorMessage,
        const function<string(const string&)>& normalizer = nullptr
    ) {
        while (true) {
            cout << prompt;
            getline(cin, value);
            string candidate = trim(value);
            if (normalizer) {
                candidate = normalizer(candidate);
            }
            if (validator(candidate)) {
                value = candidate;
                return;
            }
            cout << errorMessage << endl;
        }
    }

    bool isValidPAN(const string& id) const {
        if (id.size() != 10) {
            return false;
        }

        for (size_t i = 0; i < 5; ++i) {
            if (!isalpha(static_cast<unsigned char>(id[i]))) {
                return false;
            }
        }
        for (size_t i = 5; i < 9; ++i) {
            if (!isdigit(static_cast<unsigned char>(id[i]))) {
                return false;
            }
        }
        return isalpha(static_cast<unsigned char>(id[9]));
    }

    bool isValidAadhaar(const string& id) const {
        return id.size() == 12 && hasOnlyDigits(id);
    }

    bool isValidPassport(const string& id) const {
        if (id.size() != 8) {
            return false;
        }
        if (!isalpha(static_cast<unsigned char>(id[0]))) {
            return false;
        }
        for (size_t i = 1; i < id.size(); ++i) {
            if (!isdigit(static_cast<unsigned char>(id[i]))) {
                return false;
            }
        }
        return true;
    }

    bool isValidGovernmentID(const string& id) const {
        return isValidPAN(id) || isValidAadhaar(id) || isValidPassport(id);
    }

    string getExecutableDirectory() const {
        char buffer[4096];
        ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
        if (length <= 0) {
            return ".";
        }

        buffer[length] = '\0';
        string executablePath(buffer);
        size_t slashPosition = executablePath.find_last_of('/');
        if (slashPosition == string::npos) {
            return ".";
        }

        return executablePath.substr(0, slashPosition);
    }

    string sqlEscape(const string& value) const {
        string escaped;
        escaped.reserve(value.size());
        for (char ch : value) {
            if (ch == '\'') {
                escaped.push_back('\'');
                escaped.push_back('\'');
            } else {
                escaped.push_back(ch);
            }
        }
        return escaped;
    }

    bool executeSql(const string& query) {
        char* errorMessage = nullptr;
        int rc = sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errorMessage);
        if (rc != SQLITE_OK) {
            if (errorMessage != nullptr) {
                cout << "SQLite error: " << errorMessage << endl;
                sqlite3_free(errorMessage);
            }
            return false;
        }
        return true;
    }

    string getSqliteText(sqlite3_stmt* stmt, int column) const {
        const unsigned char* text = sqlite3_column_text(stmt, column);
        if (text == nullptr) {
            return "";
        }
        return reinterpret_cast<const char*>(text);
    }

    vector<string> splitByPipe(const string& line) {
        vector<string> parts;
        string token;
        istringstream stream(line);
        while (getline(stream, token, '|')) {
            parts.push_back(token);
        }
        return parts;
    }

    double calculateNetPremium(double benefit, double mortalityRate, double interestRate) const {
        return (benefit * mortalityRate) / (1.0 + interestRate);
    }

    double calculateGrossPremium(double netPremium, double expensesRate, double contingencyRate, double profitRate) const {
        return netPremium
            + (netPremium * expensesRate / 100.0)
            + (netPremium * contingencyRate / 100.0)
            + (netPremium * profitRate / 100.0);
    }

    double calculatePaidUpValue(double originalSumAssured, int premiumsPaid, int totalPremiumsPayable) const {
        if (totalPremiumsPayable <= 0) {
            return 0.0;
        }
        return originalSumAssured * (static_cast<double>(premiumsPaid) / totalPremiumsPayable);
    }

    double calculateSurrenderValue(double paidUpValue, double accruedBonuses, double surrenderFactor) const {
        return (paidUpValue + accruedBonuses) * (surrenderFactor / 100.0);
    }

    void syncPolicyRecordCounter() {
        int maxRecordID = 1000;
        for (const auto &client : database) {
            for (const auto &policy : client.activePolicies) {
                if (policy.recordID > maxRecordID) {
                    maxRecordID = policy.recordID;
                }
            }
        }
        nextPolicyRecordID = maxRecordID + 1;
    }

    bool initializeDatabase() {
        string createClientsTable =
            "CREATE TABLE IF NOT EXISTS clients ("
            "clientID INTEGER PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "age INTEGER NOT NULL, "
            "dateOfBirth TEXT, "
            "phoneNumber TEXT, "
            "email TEXT, "
            "fatherName TEXT, "
            "motherName TEXT, "
            "isMarried INTEGER, "
            "spouseName TEXT, "
            "address TEXT, "
            "occupation TEXT, "
            "governmentID TEXT, "
            "nomineeName TEXT, "
            "nomineeRelation TEXT, "
            "annualIncome REAL, "
            "isSmoker INTEGER, "
            "medicalHistory TEXT);";

        string createPoliciesTable =
            "CREATE TABLE IF NOT EXISTS policies ("
            "recordID INTEGER PRIMARY KEY, "
            "clientID INTEGER NOT NULL, "
            "policyID INTEGER NOT NULL, "
            "category TEXT NOT NULL, "
            "planName TEXT NOT NULL, "
            "coverageAmount REAL, "
            "basePremium REAL, "
            "paymentMode TEXT, "
            "premiumTermYears INTEGER, "
            "policyTermYears INTEGER, "
            "mortalityRate REAL, "
            "interestRate REAL, "
            "expensesRate REAL, "
            "contingencyRate REAL, "
            "profitRate REAL, "
            "accruedBonuses REAL, "
            "surrenderValueFactor REAL, "
            "isActive INTEGER, "
            "FOREIGN KEY(clientID) REFERENCES clients(clientID));";

        return executeSql("PRAGMA foreign_keys = ON;")
            && executeSql(createClientsTable)
            && executeSql(createPoliciesTable);
    }

    void saveDataToFile() {
        if (db == nullptr) {
            cout << "Warning: SQLite database is not available." << endl;
            return;
        }

        if (!executeSql("BEGIN TRANSACTION;")) {
            return;
        }

        if (!executeSql("DELETE FROM policies;")) {
            executeSql("ROLLBACK;");
            return;
        }
        if (!executeSql("DELETE FROM clients;")) {
            executeSql("ROLLBACK;");
            return;
        }

        const char* clientSql =
            "INSERT INTO clients (clientID, name, age, dateOfBirth, phoneNumber, email, fatherName, motherName, isMarried, spouseName, address, occupation, governmentID, nomineeName, nomineeRelation, annualIncome, isSmoker, medicalHistory) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

        const char* policySql =
            "INSERT INTO policies (recordID, clientID, policyID, category, planName, coverageAmount, basePremium, paymentMode, premiumTermYears, policyTermYears, mortalityRate, interestRate, expensesRate, contingencyRate, profitRate, accruedBonuses, surrenderValueFactor, isActive) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

        sqlite3_stmt* clientStmt = nullptr;
        sqlite3_stmt* policyStmt = nullptr;

        if (sqlite3_prepare_v2(db, clientSql, -1, &clientStmt, nullptr) != SQLITE_OK ||
            sqlite3_prepare_v2(db, policySql, -1, &policyStmt, nullptr) != SQLITE_OK) {
            cout << "SQLite error (Prepare): " << sqlite3_errmsg(db) << endl;
            if (clientStmt != nullptr) sqlite3_finalize(clientStmt);
            if (policyStmt != nullptr) sqlite3_finalize(policyStmt);
            executeSql("ROLLBACK;");
            return;
        }

        for (const auto &c : database) {
            sqlite3_reset(clientStmt);
            sqlite3_clear_bindings(clientStmt);

            sqlite3_bind_int(clientStmt, 1, c.clientID);
            sqlite3_bind_text(clientStmt, 2, c.name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(clientStmt, 3, c.age);
            sqlite3_bind_text(clientStmt, 4, c.dateOfBirth.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 5, c.phoneNumber.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 6, c.email.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 7, c.fatherName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 8, c.motherName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(clientStmt, 9, c.isMarried ? 1 : 0);
            sqlite3_bind_text(clientStmt, 10, c.spouseName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 11, c.address.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 12, c.occupation.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 13, c.governmentID.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 14, c.nomineeName.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(clientStmt, 15, c.nomineeRelation.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(clientStmt, 16, c.annualIncome);
            sqlite3_bind_int(clientStmt, 17, c.isSmoker ? 1 : 0);
            sqlite3_bind_text(clientStmt, 18, c.medicalHistory.c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(clientStmt) != SQLITE_DONE) {
                cout << "SQLite error (Client Step): " << sqlite3_errmsg(db) << endl;
                sqlite3_finalize(clientStmt);
                sqlite3_finalize(policyStmt);
                executeSql("ROLLBACK;");
                return;
            }

            for (const auto &p : c.activePolicies) {
                sqlite3_reset(policyStmt);
                sqlite3_clear_bindings(policyStmt);

                sqlite3_bind_int(policyStmt, 1, p.recordID);
                sqlite3_bind_int(policyStmt, 2, c.clientID);
                sqlite3_bind_int(policyStmt, 3, p.policyID);
                sqlite3_bind_text(policyStmt, 4, p.category.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(policyStmt, 5, p.planName.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_double(policyStmt, 6, p.coverageAmount);
                sqlite3_bind_double(policyStmt, 7, p.basePremium);
                sqlite3_bind_text(policyStmt, 8, p.paymentMode.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(policyStmt, 9, p.premiumTermYears);
                sqlite3_bind_int(policyStmt, 10, p.policyTermYears);
                sqlite3_bind_double(policyStmt, 11, p.mortalityRate);
                sqlite3_bind_double(policyStmt, 12, p.interestRate);
                sqlite3_bind_double(policyStmt, 13, p.expensesRate);
                sqlite3_bind_double(policyStmt, 14, p.contingencyRate);
                sqlite3_bind_double(policyStmt, 15, p.profitRate);
                sqlite3_bind_double(policyStmt, 16, p.accruedBonuses);
                sqlite3_bind_double(policyStmt, 17, p.surrenderValueFactor);
                sqlite3_bind_int(policyStmt, 18, p.isActive ? 1 : 0);

                if (sqlite3_step(policyStmt) != SQLITE_DONE) {
                    cout << "SQLite error (Policy Step): " << sqlite3_errmsg(db) << endl;
                    sqlite3_finalize(clientStmt);
                    sqlite3_finalize(policyStmt);
                    executeSql("ROLLBACK;");
                    return;
                }
            }
        }

        sqlite3_finalize(clientStmt);
        sqlite3_finalize(policyStmt);
        executeSql("COMMIT;");
    }

    void loadDataFromFile() {
        if (db == nullptr) {
            return;
        }

        database.clear();
        const char* clientQuery = "SELECT clientID, name, age, dateOfBirth, phoneNumber, email, fatherName, motherName, isMarried, spouseName, address, occupation, governmentID, nomineeName, nomineeRelation, annualIncome, isSmoker, medicalHistory FROM clients ORDER BY clientID;";
        const char* policyQuery = "SELECT recordID, policyID, category, planName, coverageAmount, basePremium, paymentMode, premiumTermYears, policyTermYears, mortalityRate, interestRate, expensesRate, contingencyRate, profitRate, accruedBonuses, surrenderValueFactor, isActive FROM policies WHERE clientID = ? ORDER BY recordID;";
        sqlite3_stmt* clientStmt = nullptr;
        sqlite3_stmt* policyStmt = nullptr;

        if (sqlite3_prepare_v2(db, clientQuery, -1, &clientStmt, nullptr) != SQLITE_OK ||
            sqlite3_prepare_v2(db, policyQuery, -1, &policyStmt, nullptr) != SQLITE_OK) {
            cout << "SQLite error (Load Prepare): " << sqlite3_errmsg(db) << endl;
            if (clientStmt) sqlite3_finalize(clientStmt);
            if (policyStmt) sqlite3_finalize(policyStmt);
            return;
        }

        int maxValidClientID = 100;
        int maxValidPolicyRecordID = 1000;

        while (sqlite3_step(clientStmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(clientStmt, 0);
            string name = getSqliteText(clientStmt, 1);
            int age = sqlite3_column_int(clientStmt, 2);
            string dob = getSqliteText(clientStmt, 3);
            string phone = getSqliteText(clientStmt, 4);
            string email = getSqliteText(clientStmt, 5);
            string father = getSqliteText(clientStmt, 6);
            string mother = getSqliteText(clientStmt, 7);
            bool married = sqlite3_column_int(clientStmt, 8) != 0;
            string spouse = getSqliteText(clientStmt, 9);
            string address = getSqliteText(clientStmt, 10);
            string job = getSqliteText(clientStmt, 11);
            string govID = getSqliteText(clientStmt, 12);
            string nominee = getSqliteText(clientStmt, 13);
            string nomineeRel = getSqliteText(clientStmt, 14);
            double income = sqlite3_column_double(clientStmt, 15);
            bool smoker = sqlite3_column_int(clientStmt, 16) != 0;
            string medical = getSqliteText(clientStmt, 17);

            Client client(
                id,
                name,
                age,
                dob,
                phone,
                email,
                father,
                mother,
                married,
                spouse,
                address,
                job,
                govID,
                nominee,
                nomineeRel,
                income,
                smoker,
                medical
            );

            sqlite3_reset(policyStmt);
            sqlite3_clear_bindings(policyStmt);
                sqlite3_bind_int(policyStmt, 1, id);
                while (sqlite3_step(policyStmt) == SQLITE_ROW) {
                    Policy policy(
                        sqlite3_column_int(policyStmt, 0),
                        sqlite3_column_int(policyStmt, 1),
                        getSqliteText(policyStmt, 2),
                        getSqliteText(policyStmt, 3),
                        sqlite3_column_double(policyStmt, 4),
                        sqlite3_column_double(policyStmt, 5)
                    );
                    policy.paymentMode = getSqliteText(policyStmt, 6);
                    policy.premiumTermYears = sqlite3_column_int(policyStmt, 7);
                    policy.policyTermYears = sqlite3_column_int(policyStmt, 8);
                    policy.mortalityRate = sqlite3_column_double(policyStmt, 9);
                    policy.interestRate = sqlite3_column_double(policyStmt, 10);
                    policy.expensesRate = sqlite3_column_double(policyStmt, 11);
                    policy.contingencyRate = sqlite3_column_double(policyStmt, 12);
                    policy.profitRate = sqlite3_column_double(policyStmt, 13);
                    policy.accruedBonuses = sqlite3_column_double(policyStmt, 14);
                    policy.surrenderValueFactor = sqlite3_column_double(policyStmt, 15);
                    policy.isActive = sqlite3_column_int(policyStmt, 16) != 0;
                    client.activePolicies.push_back(policy);
                    if (policy.recordID > maxValidPolicyRecordID) {
                        maxValidPolicyRecordID = policy.recordID;
                    }
                }

            database.push_back(client);
            if (id > maxValidClientID) {
                maxValidClientID = id;
            }
        }

        sqlite3_finalize(clientStmt);
    sqlite3_finalize(policyStmt);
        nextID = maxValidClientID + 1;
        nextPolicyRecordID = maxValidPolicyRecordID + 1;
        syncPolicyRecordCounter();
    }

    double calculateBasePremiumByType(const string& policyType, double coverageAmount) {
        // Base premium is 1% of coverage, scaled by policy type risk.
        double typeFactor = 1.0;
        if (policyType == "Health") typeFactor = 1.2;
        else if (policyType == "Auto") typeFactor = 1.0;
        else if (policyType == "Life") typeFactor = 1.4;

        return coverageAmount * 0.01 * typeFactor;
    }

    void printContract(const Client& c, const Policy& p, bool monthlyPayment, double finalPremium) {
        cout << "\n========== INSURANCE CONTRACT ==========" << endl;
        cout << "Client Name      : " << c.name << endl;
        cout << "Client ID        : " << c.clientID << endl;
        cout << "Policy Record ID : " << p.recordID << endl;
        cout << "Policy ID        : " << p.policyID << endl;
        cout << "Policy Category  : " << p.category << endl;
        cout << "Policy Plan      : " << p.planName << endl;
        cout << "Coverage Amount  : ₹" << fixed << setprecision(2) << p.coverageAmount << endl;

        if (monthlyPayment) {
            cout << "Payment Mode     : Monthly" << endl;
            cout << "Monthly Premium  : ₹" << fixed << setprecision(2) << (finalPremium / 12.0) << endl;
            cout << "Annual Total     : ₹" << fixed << setprecision(2) << finalPremium << endl;
        } else {
            cout << "Payment Mode     : Yearly" << endl;
            cout << "Premium Payable  : ₹" << fixed << setprecision(2) << finalPremium << " per year" << endl;
        }

        cout << "Contract Status  : ACTIVE" << endl;
        cout << "========================================" << endl;
    }

    void issueHealthPolicy(Client& c) {
        cout << "\nHealth Product Catalog:" << endl;
        cout << "1. Pure Term" << endl;
        cout << "2. TROP (Return of Premium)" << endl;
        cout << "3. Whole Life" << endl;
        cout << "4. Endowment" << endl;
        cout << "5. Money-Back" << endl;
        cout << "6. ULIP" << endl;

        int planChoice;
        readNumericInputInRange("Select health product (1-6): ", planChoice, 1, 6);

        string planName;
        if (planChoice == 1) planName = "Pure Term";
        else if (planChoice == 2) planName = "TROP";
        else if (planChoice == 3) planName = "Whole Life";
        else if (planChoice == 4) planName = "Endowment";
        else if (planChoice == 5) planName = "Money-Back";
        else if (planChoice == 6) planName = "ULIP";
        else {
            cout << "Invalid health plan choice." << endl;
            return;
        }

        double coverageAmount;
        readNumericInputAtLeast("Enter Sum Assured / Coverage Amount (any decimal): ", coverageAmount, 0.01);

        int premiumTermYears;
        int policyTermYears;
        readNumericInputInRange("Enter Premium Term in years: ", premiumTermYears, 1, 100);
        readNumericInputInRange("Enter Policy Term in years: ", policyTermYears, premiumTermYears, 100);

        double mortalityRate;
        double interestRate;
        double expensesRate;
        double contingencyRate;
        double profitRate;
        readNumericInputInRange("Enter Mortality Rate qx (e.g. 0.01): ", mortalityRate, 0.0, 1.0);
        readNumericInputInRange("Enter Interest Rate i (e.g. 0.08): ", interestRate, 0.0, 1.0);
        readNumericInputInRange("Enter Expense Loading (% of Net Premium): ", expensesRate, 0.0, 100.0);
        readNumericInputInRange("Enter Contingency Margin (% of Net Premium): ", contingencyRate, 0.0, 100.0);
        readNumericInputInRange("Enter Profit Loading (% of Net Premium): ", profitRate, 0.0, 100.0);

        double netPremium = calculateNetPremium(coverageAmount, mortalityRate, interestRate);
        double grossPremium = calculateGrossPremium(netPremium, expensesRate, contingencyRate, profitRate);

        bool monthlyPayment = readYesNoInput("Do you want monthly payment? (y/n): ");
        string paymentMode = monthlyPayment ? "Monthly" : "Yearly";

        cout << "\n--- PREMIUM CALCULATION ---" << endl;
        cout << "Plan: " << planName << endl;
        cout << "Sum Assured: ₹" << fixed << setprecision(2) << coverageAmount << endl;
        cout << "Net Premium: ₹" << fixed << setprecision(2) << netPremium << endl;
        cout << "Gross Annual Premium: ₹" << fixed << setprecision(2) << grossPremium << endl;
        if (monthlyPayment) {
            cout << "Monthly Premium: ₹" << fixed << setprecision(2) << (grossPremium / 12.0) << endl;
        } else {
            cout << "Yearly Premium: ₹" << fixed << setprecision(2) << grossPremium << endl;
        }

        double accruedBonuses = 0.0;
        double surrenderFactor = 0.0;
        if (planName == "Whole Life" || planName == "Endowment" || planName == "Money-Back") {
            int premiumsPaid;
            int totalPremiumsPayable;
            readNumericInputInRange("Enter Total Number of Premiums Payable: ", totalPremiumsPayable, 1, 1000);
            readNumericInputInRange("Enter Number of Premiums Paid: ", premiumsPaid, 0, totalPremiumsPayable);
            readNumericInputAtLeast("Enter Accrued Bonuses: ₹", accruedBonuses, 0.0);
            readNumericInputInRange("Enter Surrender Value Factor (%): ", surrenderFactor, 0.0, 100.0);

            double paidUpValue = calculatePaidUpValue(coverageAmount, premiumsPaid, totalPremiumsPayable);
            double surrenderValue = calculateSurrenderValue(paidUpValue, accruedBonuses, surrenderFactor);
            cout << "Paid-Up Value: ₹" << fixed << setprecision(2) << paidUpValue << endl;
            cout << "Surrender Value: ₹" << fixed << setprecision(2) << surrenderValue << endl;
        }

        if (planName == "TROP") {
            double returnOfPremium = grossPremium * premiumTermYears;
            cout << "Return of Premium at maturity (if survived): ₹" << fixed << setprecision(2) << returnOfPremium << endl;
        }

        if (planName == "Money-Back") {
            int payoutIntervalYears;
            double payoutPercentage;
            readNumericInputInRange("Enter Money-Back Payout Interval (years): ", payoutIntervalYears, 1, policyTermYears);
            readNumericInputInRange("Enter Money-Back Payout Percentage per interval: ", payoutPercentage, 0.0, 100.0);
            int payoutCount = policyTermYears / payoutIntervalYears;
            double totalMoneyBackPayout = coverageAmount * (payoutPercentage / 100.0) * payoutCount;
            cout << "Estimated Money-Back Payouts: ₹" << fixed << setprecision(2) << totalMoneyBackPayout << endl;
        }

        if (planName == "ULIP") {
            double equityAllocation;
            double expectedMarketReturn;
            readNumericInputInRange("Enter Equity Allocation Percentage: ", equityAllocation, 0.0, 100.0);
            readNumericInputInRange("Enter Expected Market Return Percentage: ", expectedMarketReturn, -100.0, 100.0);
            double maturityValue = grossPremium * (1.0 + equityAllocation / 100.0) * (1.0 + expectedMarketReturn / 100.0);
            cout << "Estimated ULIP Maturity Value: ₹" << fixed << setprecision(2) << maturityValue << endl;
        }

        Policy policy(
            nextPolicyRecordID++,
            5001,
            "Health",
            planName,
            coverageAmount,
            grossPremium
        );
        policy.paymentMode = paymentMode;
        policy.premiumTermYears = premiumTermYears;
        policy.policyTermYears = policyTermYears;
        policy.mortalityRate = mortalityRate;
        policy.interestRate = interestRate;
        policy.expensesRate = expensesRate;
        policy.contingencyRate = contingencyRate;
        policy.profitRate = profitRate;
        policy.accruedBonuses = accruedBonuses;
        policy.surrenderValueFactor = surrenderFactor;

        c.activePolicies.push_back(policy);
        cout << "\nPolicy successfully linked to " << c.name << "!" << endl;
        printContract(c, policy, monthlyPayment, grossPremium);
    }

    void issueGenericPolicy(Client& c, const string& category, int policyCode) {
        double coverageAmount;
        readNumericInputAtLeast("Enter Coverage Amount (any decimal): ", coverageAmount, 0.01);

        double basePremium = calculateBasePremiumByType(category, coverageAmount);
        bool monthlyPayment = readYesNoInput("Do you want monthly payment? (y/n): ");
        string paymentMode = monthlyPayment ? "Monthly" : "Yearly";

        Policy policy(
            nextPolicyRecordID++,
            policyCode,
            category,
            category,
            coverageAmount,
            basePremium
        );
        policy.paymentMode = paymentMode;
        policy.basePremium = basePremium;

        c.activePolicies.push_back(policy);

        double finalPremium = c.calculateTotalPremium(policy);
        cout << "\nPolicy successfully linked to " << c.name << "!" << endl;
        printContract(c, policy, monthlyPayment, finalPremium);
    }

public:
    InsuranceSystem() {
        string baseDirectory = getExecutableDirectory();
        databaseFile = baseDirectory + "/insurance_data.db";
        exportFile = baseDirectory + "/insurance_export.csv";
        if (sqlite3_open(databaseFile.c_str(), &db) != SQLITE_OK) {
            cout << "Warning: Could not open SQLite database. Falling back to in-memory mode." << endl;
            db = nullptr;
            return;
        }
        if (!initializeDatabase()) {
            cout << "Warning: Could not initialize SQLite schema." << endl;
        }
        loadDataFromFile();
        cout << "Loaded " << database.size() << " client(s) from SQLite database: " << databaseFile << endl;
    }

    ~InsuranceSystem() {
        if (db != nullptr) {
            sqlite3_close(db);
            db = nullptr;
        }
    }

    void addClient() {
        string name;
        int age;
        string dateOfBirth;
        string phoneNumber;
        string email;
        string fatherName;
        string motherName;
        bool isMarried;
        string spouseName = "N/A";
        string address;
        string occupation;
        string governmentID;
        string nomineeName;
        string nomineeRelation;
        double annualIncome;
        bool isSmoker;
        string medicalHistory;

        readNonEmptyLine("Enter Client Name: ", name);

        readValidatedLine(
            "Enter Date of Birth (DD-MM-YYYY): ",
            dateOfBirth,
            [this](const string& input) { return isValidDateOfBirth(input); },
            "Invalid date format/value. Please enter DOB as DD-MM-YYYY."
        );

        readNumericInputInRange("Enter Age: ", age, 1, 120);

        readValidatedLine(
            "Enter Phone Number (10 digits): ",
            phoneNumber,
            [this](const string& input) { return isValidPhoneNumber(input); },
            "Invalid phone number. Enter a 10-digit number starting with 6-9."
        );

        readValidatedLine(
            "Enter Email: ",
            email,
            [this](const string& input) { return isValidEmail(input); },
            "Invalid email format. Please enter a valid email address."
        );

        readNonEmptyLine("Enter Father's Name: ", fatherName);

        readNonEmptyLine("Enter Mother's Name: ", motherName);

        isMarried = readYesNoInput("Married? (y/n): ");
        if (isMarried) {
            readNonEmptyLine("Enter Spouse Name: ", spouseName);
        }

        readNonEmptyLine("Enter Address: ", address);

        readNonEmptyLine("Enter Occupation: ", occupation);

        readValidatedLine(
            "Enter Government ID (PAN/Aadhaar/Passport): ",
            governmentID,
            [this](const string& input) { return isValidGovernmentID(input); },
            "Invalid Government ID format. Use PAN (AAAAA9999A), Aadhaar (12 digits), or Passport (A1234567).",
            [this](const string& input) { return toUpperCopy(input); }
        );

        readNonEmptyLine("Enter Nominee Name: ", nomineeName);

        readNonEmptyLine("Enter Nominee Relation: ", nomineeRelation);

        readNumericInputAtLeast("Enter Annual Income: ", annualIncome, 0.0);

        isSmoker = readYesNoInput("Smoker? (y/n): ");

        readNonEmptyLine("Enter Medical History / Pre-existing Conditions: ", medicalHistory);

        database.push_back(
            Client(
                nextID++,
                name,
                age,
                dateOfBirth,
                phoneNumber,
                email,
                fatherName,
                motherName,
                isMarried,
                spouseName,
                address,
                occupation,
                governmentID,
                nomineeName,
                nomineeRelation,
                annualIncome,
                isSmoker,
                medicalHistory
            )
        );
        cout << "Client added successfully with ID: " << nextID - 1 << endl;
        saveDataToFile();
    }

    void buyPolicy() {
        if (database.empty()) {
            cout << "\nError: No clients registered. Please add a client first (Option 1)." << endl;
            return;
        }

        printRegisteredClients();

        int id;
        readNumericInputAtLeast("Enter Client ID to add policy: ", id, 1);
        Client* client = findClientById(id);
        if (client == nullptr) {
            cout << "Error: ID " << id << " not found. Create an account first." << endl;
            return;
        }

        cout << "Available Policies: 1. Health  2. Auto  3. Life" << endl;
        int pChoice;
        readNumericInputInRange("Select policy type (1-3): ", pChoice, 1, 3);

        if (pChoice == 1) {
            issueHealthPolicy(*client);
        } else if (pChoice == 2) {
            issueGenericPolicy(*client, "Auto", 5002);
        } else if (pChoice == 3) {
            issueGenericPolicy(*client, "Life", 5003);
        } else {
            cout << "Invalid policy choice." << endl;
            return;
        }

        saveDataToFile();
    }

    void deleteClient() {
        if (database.empty()) {
            cout << "\nError: No client records to delete." << endl;
            return;
        }

        cout << "\nCurrent Client Records:" << endl;
        for (const auto &c : database) {
            c.displayDetails();
        }

        int id;
        readNumericInputAtLeast("Enter Client ID to delete: ", id, 1);

        for (size_t i = 0; i < database.size(); ++i) {
            if (database[i].clientID == id) {
                string deletedName = database[i].name;
                database.erase(database.begin() + i);
                cout << "Record deleted successfully for client: " << deletedName << " (ID: " << id << ")" << endl;
                saveDataToFile();
                return;
            }
        }

        cout << "Error: No client found with ID " << id << "." << endl;
    }

    void deactivatePolicy() {
        if (database.empty()) {
            cout << "\nError: No client records found." << endl;
            return;
        }

        printRegisteredClients();

        int clientId;
        readNumericInputAtLeast("Enter Client ID: ", clientId, 1);
        Client* client = findClientById(clientId);
        if (client == nullptr) {
            cout << "Error: No client found with ID " << clientId << "." << endl;
            return;
        }

        if (client->activePolicies.empty()) {
            cout << "No policies found for this client." << endl;
            return;
        }

        cout << "\nPolicies for " << client->name << ":" << endl;
        for (const auto &p : client->activePolicies) {
            cout << "Record ID: " << p.recordID
                 << " | Plan: " << p.category << " / " << p.planName
                 << " | Status: " << (p.isActive ? "Active" : "Inactive")
                 << " | Coverage: ₹" << fixed << setprecision(2) << p.coverageAmount
                 << endl;
        }

        int recordId;
        readNumericInputAtLeast("Enter Policy Record ID to deactivate: ", recordId, 1);

        for (auto &p : client->activePolicies) {
            if (p.recordID == recordId) {
                p.isActive = false;
                saveDataToFile();
                cout << "Policy record " << recordId << " deactivated successfully." << endl;
                return;
            }
        }

        cout << "Error: Policy record ID not found for this client." << endl;
    }

    void showAll() {
        if (database.empty()) {
            cout << "\nNo records found. Add clients first." << endl;
            return;
        }

        for (auto &c : database) {
            c.displayDetails();
        }
    }

    void searchClientRecord() {
        if (database.empty()) {
            cout << "\nNo records found. Add clients first." << endl;
            return;
        }

        int id;
        readNumericInputAtLeast("Enter Client ID to search: ", id, 1);

        for (const auto &c : database) {
            if (c.clientID == id) {
                c.displayDetails();
                return;
            }
        }

        cout << "No client found with ID " << id << "." << endl;
    }

    void exportRecordsToCSV() {
        if (database.empty()) {
            cout << "\nNo records available to export." << endl;
            return;
        }

        ofstream out(exportFile);
        if (!out.is_open()) {
            cout << "\nError: Could not create export file." << endl;
            return;
        }

        out << fixed << setprecision(2);
        out << "ClientID,ClientName,Age,DOB,Phone,Email,FatherName,MotherName,Married,SpouseName,Address,Occupation,GovernmentID,NomineeName,NomineeRelation,AnnualIncome,Smoker,MedicalHistory,PolicyRecordID,PolicyID,PolicyCategory,PolicyPlan,PaymentMode,CoverageAmount,BasePremium,FinalPremium,Status\n";

        for (const auto &c : database) {
            if (c.activePolicies.empty()) {
                writeClientCsvPrefix(out, c);
                out << "N/A,N/A,N/A,N/A,N/A,0,0,0,N/A\n";
                continue;
            }

            for (const auto &p : c.activePolicies) {
                double finalPremium = c.calculateTotalPremium(p);
                writeClientCsvPrefix(out, c);
                out << p.recordID << "," << p.policyID << "," << escapeCsvField(p.category) << "," << escapeCsvField(p.planName) << "," << escapeCsvField(p.paymentMode) << ","
                    << p.coverageAmount << ","
                    << p.basePremium << ","
                    << finalPremium << ","
                    << (p.isActive ? "Active" : "Inactive") << "\n";
            }
        }

        cout << "\nExport complete. Records saved to: " << exportFile << endl;
    }

    void resetDemoData() {
        bool confirmed = readYesNoInput("This will delete all clients and policies, then create one demo client. Continue? (y/n): ");
        if (!confirmed) {
            cout << "Reset cancelled." << endl;
            return;
        }

        database.clear();
        nextID = 101;
        nextPolicyRecordID = 1001;

        database.push_back(
            Client(
                nextID++,
                "Demo Client",
                30,
                "01-01-1996",
                "9876543210",
                "demo@example.com",
                "Demo Father",
                "Demo Mother",
                false,
                "N/A",
                "Demo Address",
                "Demo Job",
                "ABCDE1234F",
                "Demo Nominee",
                "Sibling",
                500000.0,
                false,
                "None"
            )
        );

        saveDataToFile();
        cout << "Demo reset complete. Database now contains 1 demo client." << endl;
    }
};

// --- Main Interface ---
int main() {
    InsuranceSystem ims;
    int choice;

    while (true) {
        cout << "\n--- INSURANCE MANAGEMENT CONSOLE ---";
        cout << "\n1. Add Client\n2. Purchase Policy\n3. Display All Records\n4. Delete Client Record\n5. Export Records (CSV)\n6. Deactivate Policy\n7. Exit\n8. Reset Demo Data\n9. Search Client Record";
        cout << "\nSelect option: ";
        if (!(cin >> choice)) {
            cout << "Invalid input. Please enter a number from 1 to 9." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        if (choice == 1) ims.addClient();
        else if (choice == 2) ims.buyPolicy();
        else if (choice == 3) ims.showAll();
        else if (choice == 4) ims.deleteClient();
        else if (choice == 5) ims.exportRecordsToCSV();
        else if (choice == 6) ims.deactivatePolicy();
        else if (choice == 7) break;
        else if (choice == 8) ims.resetDemoData();
        else if (choice == 9) ims.searchClientRecord();
        else cout << "Invalid option. Please select a valid option (1-9)." << endl;
    }

    return 0;
}
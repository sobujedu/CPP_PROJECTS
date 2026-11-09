#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>     // for runtime_error (exception handling)
using namespace std;

// ===================== BASE CLASS =====================
// CLASS + OBJECT: Account is the blueprint, each customer is an object
// ABSTRACTION: Account has pure virtual functions, so it cannot be created directly
class Account {
protected:   // ENCAPSULATION: data is hidden from outside, only member functions can touch it
    int accNo;
    string name;
    double balance;
public:
    // CONSTRUCTOR: sets the data when the object is created
    Account(int a, string n, double b) {
        accNo = a;
        name = n;
        balance = b;
    }
    virtual ~Account() {}   // virtual destructor, needed for polymorphism with delete

    // ENCAPSULATION: getters give controlled access to private data
    int getAccNo() { return accNo; }
    double getBalance() { return balance; }

    // POLYMORPHISM: pure virtual, each child class gives its own version
    virtual void withdraw(double amt) = 0;
    virtual string getType() = 0;

    void deposit(double amt) {
        // EXCEPTION HANDLING: throw an error if the amount is invalid
        if (amt <= 0)
            throw runtime_error("Invalid deposit amount");
        balance += amt;
        logTransaction("Deposit", amt);
    }

    // FILE HANDLING + CSV: append one line "accNo,type,amount" to transactions.csv
    void logTransaction(string type, double amt) {
        ofstream f("transactions.csv", ios::app);   // ios::app = add at the end
        f << accNo << "," << type << "," << amt << endl;
    }

    void display() {
        cout << getType() << " | No: " << accNo
             << " | Name: " << name
             << " | Balance: " << balance << endl;
    }

    // FILE HANDLING + CSV: write this account as one CSV line
    void save(ofstream &f) {
        f << getType() << "," << accNo << "," << name << "," << balance << endl;
    }
};

// ===================== DERIVED CLASS 1 =====================
// INHERITANCE: SavingsAccount inherits from Account (is-a relationship)
class SavingsAccount : public Account {
public:
    SavingsAccount(int a, string n, double b) : Account(a, n, b) {}

    string getType() { return "Savings"; }

    // POLYMORPHISM: overriding withdraw (Savings must keep minimum 500)
    void withdraw(double amt) {
        if (amt <= 0)
            throw runtime_error("Invalid withdraw amount");   // EXCEPTION HANDLING
        if (balance - amt < 500)
            throw runtime_error("Savings account must keep minimum balance 500");
        balance -= amt;
        logTransaction("Withdraw", amt);
    }
};

// ===================== DERIVED CLASS 2 =====================
// INHERITANCE: CurrentAccount also inherits from Account
class CurrentAccount : public Account {
public:
    CurrentAccount(int a, string n, double b) : Account(a, n, b) {}

    string getType() { return "Current"; }

    // POLYMORPHISM: different rule (overdraft allowed up to 1000)
    void withdraw(double amt) {
        if (amt <= 0)
            throw runtime_error("Invalid withdraw amount");   // EXCEPTION HANDLING
        if (balance - amt < -1000)
            throw runtime_error("Overdraft limit (1000) exceeded");
        balance -= amt;
        logTransaction("Withdraw", amt);
    }
};

// ===================== GLOBAL DATA =====================
// POLYMORPHISM: base class pointers can hold Savings or Current objects
Account* accounts[100];
int total = 0;

Account* findAccount(int no) {
    for (int i = 0; i < total; i++)
        if (accounts[i]->getAccNo() == no)
            return accounts[i];
    return NULL;
}

// ===================== FILE HANDLING (CSV) =====================
// Save all accounts to accounts.csv (ofstream = output file stream)
void saveAll() {
    ofstream f("accounts.csv");
    for (int i = 0; i < total; i++)
        accounts[i]->save(f);
}

// Load all accounts from accounts.csv (ifstream = input file stream)
void loadAll() {
    ifstream f("accounts.csv");
    if (!f) return;   // file does not exist on first run, so just skip

    string type, noStr, name, balStr;
    // getline(f, x, ',') reads text until the next comma (CSV reading)
    while (getline(f, type, ',')) {
        getline(f, noStr, ',');
        getline(f, name, ',');
        getline(f, balStr);   // last column: read until end of line

        int no = stoi(noStr);        // string to int
        double bal = stod(balStr);   // string to double

        if (type == "Savings")
            accounts[total++] = new SavingsAccount(no, name, bal);
        else
            accounts[total++] = new CurrentAccount(no, name, bal);
    }
}

// ===================== FUNCTIONS =====================
void createAccount() {
    int no, choice;
    string name;
    double bal;

    cout << "Account no: ";
    cin >> no;
    // EXCEPTION HANDLING: duplicate account number is an error
    if (findAccount(no) != NULL)
        throw runtime_error("Account already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "Initial balance: ";
    cin >> bal;
    if (bal < 0)
        throw runtime_error("Initial balance cannot be negative");

    cout << "1. Savings  2. Current: ";
    cin >> choice;

    // POLYMORPHISM: child objects stored in base class pointer
    if (choice == 1)
        accounts[total++] = new SavingsAccount(no, name, bal);
    else
        accounts[total++] = new CurrentAccount(no, name, bal);
    cout << "Account created\n";
}

void adminMenu() {
    int pass;
    cout << "Admin password: ";
    cin >> pass;
    if (pass != 1234)
        throw runtime_error("Wrong admin password");   // EXCEPTION HANDLING

    int ch;
    cout << "1. View all accounts\n2. View transaction history\nChoice: ";
    cin >> ch;

    if (ch == 1) {
        for (int i = 0; i < total; i++)
            accounts[i]->display();
    } else if (ch == 2) {
        int target;
        cout << "Account no: ";
        cin >> target;

        // FILE HANDLING + CSV: read transactions.csv line by line
        ifstream f("transactions.csv");
        if (!f)
            throw runtime_error("No transaction history found");   // EXCEPTION HANDLING

        string noStr, type, amtStr;
        while (getline(f, noStr, ',')) {
            getline(f, type, ',');
            getline(f, amtStr);
            if (stoi(noStr) == target)
                cout << type << " : " << amtStr << endl;
        }
    }
}

// ===================== MAIN =====================
int main() {
    loadAll();   // FILE HANDLING: load old data at program start
    int choice, no;
    double amt;

    do {
        cout << "\n--- BANK MENU ---\n";
        cout << "1. Create Account\n2. Deposit\n3. Withdraw\n";
        cout << "4. Check Balance\n5. Admin\n0. Exit\nChoice: ";
        cin >> choice;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (choice == 1) {
                createAccount();
            } else if (choice >= 2 && choice <= 4) {
                cout << "Account no: ";
                cin >> no;
                Account* acc = findAccount(no);
                if (acc == NULL)
                    throw runtime_error("Account not found");

                if (choice == 2) {
                    cout << "Amount: ";
                    cin >> amt;
                    acc->deposit(amt);
                    cout << "Deposit successful\n";
                } else if (choice == 3) {
                    cout << "Amount: ";
                    cin >> amt;
                    acc->withdraw(amt);   // POLYMORPHISM: runs Savings or Current version
                    cout << "Withdraw successful\n";
                } else {
                    acc->display();
                }
            } else if (choice == 5) {
                adminMenu();
            }
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }

        saveAll();   // FILE HANDLING: save after every action
    } while (choice != 0);

    // free memory of dynamically created objects
    for (int i = 0; i < total; i++)
        delete accounts[i];
    return 0;
}
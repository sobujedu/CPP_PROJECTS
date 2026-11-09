#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>     // EXCEPTION HANDLING: runtime_error
using namespace std;

// ===================== BASE CLASS =====================
// CLASS + OBJECT: Employee is the blueprint
// ABSTRACTION: it has pure virtual functions, so we cannot create a plain Employee
class Employee {
protected:   // ENCAPSULATION: hidden from outside, child classes can use it
    int id;
    string name;
    double basic;    // basic salary
    double extra;    // Manager: allowance | Developer: overtime hours | Intern: 0
public:
    // CONSTRUCTOR: setters are used so the values are validated
    Employee(int i, string n, double b, double e) {
        id = i;
        name = n;
        setBasic(b);
        setExtra(e);
    }
    virtual ~Employee() {}   // virtual destructor (safe delete through base pointer)

    // ENCAPSULATION: getters
    int getId() { return id; }
    string getName() { return name; }
    double getBasic() { return basic; }

    // ENCAPSULATION + EXCEPTION HANDLING: setters reject bad data
    void setName(string n) { name = n; }
    void setBasic(double b) {
        if (b < 0)
            throw runtime_error("Basic salary cannot be negative");
        basic = b;
    }
    void setExtra(double e) {
        if (e < 0)
            throw runtime_error("Extra value cannot be negative");
        extra = e;
    }

    // POLYMORPHISM: pure virtual, every child gives its own version
    virtual double calculateSalary() = 0;
    virtual void promote() = 0;
    virtual string getType() = 0;

    void display() {
        cout << getType() << " | ID: " << id
             << " | Name: " << name
             << " | Basic: " << basic
             << " | Salary: " << calculateSalary() << endl;
    }

    // FILE HANDLING + CSV: write this employee as one CSV line
    void save(ofstream &f) {
        f << getType() << "," << id << "," << name << ","
          << basic << "," << extra << endl;
    }
};

// ===================== DERIVED CLASS 1 =====================
// INHERITANCE: Manager is-a Employee
class Manager : public Employee {
public:
    Manager(int i, string n, double b, double e) : Employee(i, n, b, e) {}

    string getType() { return "Manager"; }

    // POLYMORPHISM: Manager salary = basic + 20% bonus + allowance
    double calculateSalary() { return basic + basic * 0.20 + extra; }

    // POLYMORPHISM: Manager promotion = 20% raise in basic
    void promote() { basic = basic * 1.20; }
};

// ===================== DERIVED CLASS 2 =====================
// INHERITANCE: Developer is-a Employee
class Developer : public Employee {
public:
    Developer(int i, string n, double b, double e) : Employee(i, n, b, e) {}

    string getType() { return "Developer"; }

    // POLYMORPHISM: Developer salary = basic + overtime hours * 500
    double calculateSalary() { return basic + extra * 500; }

    // POLYMORPHISM: Developer promotion = 15% raise in basic
    void promote() { basic = basic * 1.15; }
};

// ===================== DERIVED CLASS 3 =====================
// INHERITANCE: Intern is-a Employee
class Intern : public Employee {
public:
    Intern(int i, string n, double b, double e) : Employee(i, n, b, e) {}

    string getType() { return "Intern"; }

    // POLYMORPHISM: Intern gets only the stipend (basic)
    double calculateSalary() { return basic; }

    // POLYMORPHISM: Intern promotion = 10% raise in basic
    void promote() { basic = basic * 1.10; }
};

// ===================== GLOBAL DATA =====================
// POLYMORPHISM: base class pointers can hold Manager, Developer or Intern objects
Employee* emps[100];
int total = 0;

// returns index of the employee with this id, or -1 if not found
int findIndex(int id) {
    for (int i = 0; i < total; i++)
        if (emps[i]->getId() == id)
            return i;
    return -1;
}

// creates the right child object from the type name
Employee* makeEmployee(string type, int id, string name, double basic, double extra) {
    if (type == "Manager")   return new Manager(id, name, basic, extra);
    if (type == "Developer") return new Developer(id, name, basic, extra);
    if (type == "Intern")    return new Intern(id, name, basic, extra);
    throw runtime_error("Unknown employee type");     // EXCEPTION HANDLING
}

// ===================== FILE HANDLING (CSV) =====================
void saveAll() {
    ofstream f("employees.csv");    // ofstream = output file stream
    for (int i = 0; i < total; i++)
        emps[i]->save(f);
}

void loadAll() {
    ifstream f("employees.csv");    // ifstream = input file stream
    if (!f) return;                 // first run: file does not exist, skip

    string type, idStr, name, basicStr, extraStr;
    // getline(f, x, ',') reads until the next comma (CSV reading)
    while (getline(f, type, ',')) {
        getline(f, idStr, ',');
        getline(f, name, ',');
        getline(f, basicStr, ',');
        getline(f, extraStr);       // last column: read until end of line
        if (total < 100)
            emps[total++] = makeEmployee(type, stoi(idStr), name,
                                         stod(basicStr), stod(extraStr));
    }
}

// ===================== FUNCTIONS =====================
void addEmployee() {
    int id, choice;
    string name;
    double basic, extra;

    cout << "ID: ";
    cin >> id;
    if (total >= 100)
        throw runtime_error("Employee list is full");
    if (findIndex(id) != -1)
        throw runtime_error("An employee with this ID already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "1. Manager  2. Developer  3. Intern: ";
    cin >> choice;
    cout << "Basic salary: ";
    cin >> basic;
    cout << "Extra (Manager: allowance, Developer: overtime hours, Intern: 0): ";
    cin >> extra;

    string type;
    if (choice == 1) type = "Manager";
    else if (choice == 2) type = "Developer";
    else if (choice == 3) type = "Intern";
    else throw runtime_error("Invalid employee type");

    // POLYMORPHISM: child object stored in a base class pointer
    emps[total++] = makeEmployee(type, id, name, basic, extra);
    cout << "Employee added\n";
}

void updateEmployee() {
    int id, ch;
    cout << "ID to update: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Employee not found");

    cout << "1. Update name\n2. Update basic salary\n3. Update extra\nChoice: ";
    cin >> ch;

    if (ch == 1) {
        string n;
        cout << "New name (one word): ";
        cin >> n;
        emps[idx]->setName(n);
    } else if (ch == 2) {
        double b;
        cout << "New basic salary: ";
        cin >> b;
        emps[idx]->setBasic(b);     // setter checks the value
    } else if (ch == 3) {
        double e;
        cout << "New extra value: ";
        cin >> e;
        emps[idx]->setExtra(e);
    }
    cout << "Employee updated\n";
}

void promoteEmployee() {
    int id;
    cout << "ID to promote: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Employee not found");

    emps[idx]->promote();           // POLYMORPHISM: runs the version of the real type
    cout << "Promoted! New basic salary: " << emps[idx]->getBasic() << endl;
}

void salarySlip() {
    int id;
    cout << "ID: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Employee not found");

    emps[idx]->display();           // shows the salary calculated by the real type
}

void viewAll() {
    if (total == 0)
        throw runtime_error("No employee records");

    double sum = 0;
    for (int i = 0; i < total; i++) {
        emps[i]->display();
        sum += emps[i]->calculateSalary();     // POLYMORPHISM: same call, different rule
    }
    cout << "Total monthly payroll: " << sum << endl;
}

// ===================== MAIN =====================
int main() {
    // EXCEPTION HANDLING: loading can throw if the CSV has bad data
    try {
        loadAll();
    }
    catch (exception &e) {
        cout << "Error while loading file: " << e.what() << endl;
    }

    int choice;
    do {
        cout << "\n--- EMPLOYEE MENU ---\n";
        cout << "1. Add Employee\n2. Update Employee\n3. Promote Employee\n";
        cout << "4. Salary Slip\n5. View All (Payroll)\n0. Exit\nChoice: ";
        cin >> choice;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (choice == 1) addEmployee();
            else if (choice == 2) updateEmployee();
            else if (choice == 3) promoteEmployee();
            else if (choice == 4) salarySlip();
            else if (choice == 5) viewAll();
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }

        saveAll();      // FILE HANDLING: save after every action
    } while (choice != 0);

    // free memory of all dynamically created objects
    for (int i = 0; i < total; i++)
        delete emps[i];
    return 0;
}
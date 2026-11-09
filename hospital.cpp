#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>     // EXCEPTION HANDLING: runtime_error
using namespace std;

const double ROOM_CHARGE = 1000;    // room charge per day (used in billing)

// ===================== BASE CLASS =====================
// CLASS + OBJECT: Person holds the data common to doctors, patients and staff
class Person {
protected:   // ENCAPSULATION: hidden from outside, child classes can use it
    int id;
    string name;
    int age;
public:
    // CONSTRUCTOR: uses the setter so the age is validated
    Person(int i, string n, int a) {
        id = i;
        name = n;
        setAge(a);
    }
    virtual ~Person() {}     // virtual destructor (safe delete through base pointer)

    // ENCAPSULATION: getters
    int getId() { return id; }
    string getName() { return name; }

    // ENCAPSULATION + EXCEPTION HANDLING: setter rejects bad data
    void setAge(int a) {
        if (a <= 0 || a > 120)
            throw runtime_error("Invalid age");
        age = a;
    }

    virtual void display() {
        cout << "ID: " << id << " | Name: " << name << " | Age: " << age;
    }
};

// ===================== DERIVED CLASS 1 =====================
// INHERITANCE: Doctor is-a Person
class Doctor : public Person {
private:
    string specialization;
    double fee;             // consultation fee
public:
    // CONSTRUCTOR: passes id, name, age up to Person
    Doctor(int i, string n, int a, string sp, double f) : Person(i, n, a) {
        specialization = sp;
        if (f < 0)
            throw runtime_error("Fee cannot be negative");
        fee = f;
    }

    double getFee() { return fee; }

    // OVERRIDING: parent's display first, then the doctor's own details
    void display() {
        Person::display();
        cout << " | Doctor | " << specialization << " | Fee: " << fee << endl;
    }

    // FILE HANDLING + CSV: write this doctor as one CSV line
    void save(ofstream &f) {
        f << id << "," << name << "," << age << ","
          << specialization << "," << fee << endl;
    }
};

// ===================== DERIVED CLASS 2 =====================
// INHERITANCE: Patient is-a Person
class Patient : public Person {
private:    // ENCAPSULATION: status and days change only through setters
    string disease;
    string status;      // Admitted, Treatment or Discharged
    int doctorId;       // 0 means no doctor assigned yet
    int days;           // days stayed in the hospital
public:
    Patient(int i, string n, int a, string dis, string st, int doc, int d)
        : Person(i, n, a) {
        disease = dis;
        setStatus(st);
        doctorId = doc;
        setDays(d);
    }

    // ENCAPSULATION + EXCEPTION HANDLING: only valid statuses are allowed
    void setStatus(string s) {
        if (s != "Admitted" && s != "Treatment" && s != "Discharged")
            throw runtime_error("Status must be Admitted, Treatment or Discharged");
        status = s;
    }
    void setDays(int d) {
        if (d < 0)
            throw runtime_error("Days cannot be negative");
        days = d;
    }
    void assignDoctor(int docId) { doctorId = docId; }

    string getStatus() { return status; }
    int getDoctorId() { return doctorId; }
    int getDays() { return days; }

    void display() {
        Person::display();
        cout << " | Patient | " << disease
             << " | Status: " << status
             << " | Doctor ID: " << doctorId
             << " | Days: " << days << endl;
    }

    // FILE HANDLING + CSV
    void save(ofstream &f) {
        f << id << "," << name << "," << age << "," << disease << ","
          << status << "," << doctorId << "," << days << endl;
    }
};

// ===================== DERIVED CLASS 3 =====================
// INHERITANCE: Staff is-a Person
class Staff : public Person {
private:
    string role;        // Nurse, Cleaner, Receptionist ...
    double salary;
public:
    Staff(int i, string n, int a, string r, double s) : Person(i, n, a) {
        role = r;
        if (s < 0)
            throw runtime_error("Salary cannot be negative");
        salary = s;
    }

    void display() {
        Person::display();
        cout << " | Staff | " << role << " | Salary: " << salary << endl;
    }

    // FILE HANDLING + CSV
    void save(ofstream &f) {
        f << id << "," << name << "," << age << ","
          << role << "," << salary << endl;
    }
};

// ===================== GLOBAL DATA =====================
Doctor* doctors[50];
int doctorCount = 0;
Patient* patients[100];
int patientCount = 0;
Staff* staffs[50];
int staffCount = 0;

// each function returns the index of the record with this id, or -1 if not found
int findDoctor(int id) {
    for (int i = 0; i < doctorCount; i++)
        if (doctors[i]->getId() == id)
            return i;
    return -1;
}
int findPatient(int id) {
    for (int i = 0; i < patientCount; i++)
        if (patients[i]->getId() == id)
            return i;
    return -1;
}
int findStaff(int id) {
    for (int i = 0; i < staffCount; i++)
        if (staffs[i]->getId() == id)
            return i;
    return -1;
}

// ===================== FILE HANDLING (CSV) =====================
void saveAll() {
    ofstream d("doctors.csv");      // ofstream = output file stream
    for (int i = 0; i < doctorCount; i++)
        doctors[i]->save(d);

    ofstream p("patients.csv");
    for (int i = 0; i < patientCount; i++)
        patients[i]->save(p);

    ofstream s("staff.csv");
    for (int i = 0; i < staffCount; i++)
        staffs[i]->save(s);
}

void loadAll() {
    string a, b, c, d, e, f, g;     // temporary text columns

    // ----- doctors.csv -----
    ifstream df("doctors.csv");     // ifstream = input file stream
    if (df) {
        // getline(df, x, ',') reads until the next comma (CSV reading)
        while (getline(df, a, ',')) {
            getline(df, b, ',');
            getline(df, c, ',');
            getline(df, d, ',');
            getline(df, e);         // last column: read until end of line
            if (doctorCount < 50)
                doctors[doctorCount++] = new Doctor(stoi(a), b, stoi(c), d, stod(e));
        }
    }

    // ----- patients.csv -----
    ifstream pf("patients.csv");
    if (pf) {
        while (getline(pf, a, ',')) {
            getline(pf, b, ',');
            getline(pf, c, ',');
            getline(pf, d, ',');
            getline(pf, e, ',');
            getline(pf, f, ',');
            getline(pf, g);
            if (patientCount < 100)
                patients[patientCount++] = new Patient(stoi(a), b, stoi(c), d, e,
                                                       stoi(f), stoi(g));
        }
    }

    // ----- staff.csv -----
    ifstream sf("staff.csv");
    if (sf) {
        while (getline(sf, a, ',')) {
            getline(sf, b, ',');
            getline(sf, c, ',');
            getline(sf, d, ',');
            getline(sf, e);
            if (staffCount < 50)
                staffs[staffCount++] = new Staff(stoi(a), b, stoi(c), d, stod(e));
        }
    }
}

// ===================== DOCTOR FUNCTIONS =====================
void addDoctor() {
    int id, age;
    string name, spec;
    double fee;

    cout << "Doctor ID: ";
    cin >> id;
    if (doctorCount >= 50)
        throw runtime_error("Doctor list is full");
    if (findDoctor(id) != -1)
        throw runtime_error("A doctor with this ID already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "Age: ";
    cin >> age;
    cout << "Specialization (one word): ";
    cin >> spec;
    cout << "Consultation fee: ";
    cin >> fee;

    // the constructor validates the values and throws if they are wrong
    doctors[doctorCount++] = new Doctor(id, name, age, spec, fee);
    cout << "Doctor added\n";
}

void viewDoctors() {
    if (doctorCount == 0)
        throw runtime_error("No doctors found");
    for (int i = 0; i < doctorCount; i++)
        doctors[i]->display();
}

// ===================== PATIENT FUNCTIONS =====================
void addPatient() {
    int id, age;
    string name, disease;

    cout << "Patient ID: ";
    cin >> id;
    if (patientCount >= 100)
        throw runtime_error("Patient list is full");
    if (findPatient(id) != -1)
        throw runtime_error("A patient with this ID already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "Age: ";
    cin >> age;
    cout << "Disease (one word): ";
    cin >> disease;

    // new patient: status Admitted, no doctor yet (0), 0 days
    patients[patientCount++] = new Patient(id, name, age, disease, "Admitted", 0, 0);
    cout << "Patient added\n";
}

void assignDoctor() {
    int pid, did;
    cout << "Patient ID: ";
    cin >> pid;
    int pIdx = findPatient(pid);
    if (pIdx == -1)
        throw runtime_error("Patient not found");
    if (patients[pIdx]->getStatus() == "Discharged")
        throw runtime_error("Patient is already discharged");

    cout << "Doctor ID: ";
    cin >> did;
    int dIdx = findDoctor(did);
    if (dIdx == -1)
        throw runtime_error("Doctor not found");

    patients[pIdx]->assignDoctor(did);
    cout << "Doctor assigned\n";
}

void updateStatus() {
    int pid, ch, days;
    cout << "Patient ID: ";
    cin >> pid;
    int idx = findPatient(pid);
    if (idx == -1)
        throw runtime_error("Patient not found");

    cout << "1. Admitted  2. Treatment  3. Discharged: ";
    cin >> ch;
    string st;
    if (ch == 1) st = "Admitted";
    else if (ch == 2) st = "Treatment";
    else if (ch == 3) st = "Discharged";
    else throw runtime_error("Invalid choice");

    cout << "Total days stayed so far: ";
    cin >> days;

    patients[idx]->setStatus(st);       // setters check the values
    patients[idx]->setDays(days);
    cout << "Status updated\n";
}

void checkStatus() {
    int pid;
    cout << "Patient ID: ";
    cin >> pid;
    int idx = findPatient(pid);
    if (idx == -1)
        throw runtime_error("Patient not found");

    patients[idx]->display();

    // show the name of the assigned doctor (if any)
    int docId = patients[idx]->getDoctorId();
    if (docId == 0) {
        cout << "No doctor assigned yet\n";
    } else {
        int dIdx = findDoctor(docId);
        if (dIdx != -1)
            cout << "Assigned doctor: " << doctors[dIdx]->getName() << endl;
    }
}

void viewPatients() {
    if (patientCount == 0)
        throw runtime_error("No patients found");
    for (int i = 0; i < patientCount; i++)
        patients[i]->display();
}

// ===================== BILLING =====================
void generateBill() {
    int pid;
    cout << "Patient ID: ";
    cin >> pid;
    int idx = findPatient(pid);
    if (idx == -1)
        throw runtime_error("Patient not found");

    double doctorFee = 0;
    int docId = patients[idx]->getDoctorId();
    if (docId != 0) {
        int dIdx = findDoctor(docId);
        if (dIdx != -1)
            doctorFee = doctors[dIdx]->getFee();
    }
    double roomCharge = patients[idx]->getDays() * ROOM_CHARGE;
    double total = doctorFee + roomCharge;

    cout << "\n----- BILL -----\n";
    cout << "Patient        : " << patients[idx]->getName() << endl;
    cout << "Doctor fee     : " << doctorFee << endl;
    cout << "Room charge    : " << roomCharge << endl;
    cout << "Total          : " << total << endl;

    // FILE HANDLING + CSV: add one line to bills.csv
    ofstream f("bills.csv", ios::app);      // ios::app = add at the end
    f << pid << "," << patients[idx]->getName() << ","
      << doctorFee << "," << roomCharge << "," << total << endl;
    cout << "Bill saved to bills.csv\n";
}

// ===================== STAFF FUNCTIONS =====================
void addStaff() {
    int id, age;
    string name, role;
    double salary;

    cout << "Staff ID: ";
    cin >> id;
    if (staffCount >= 50)
        throw runtime_error("Staff list is full");
    if (findStaff(id) != -1)
        throw runtime_error("A staff member with this ID already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "Age: ";
    cin >> age;
    cout << "Role (one word, e.g. Nurse): ";
    cin >> role;
    cout << "Salary: ";
    cin >> salary;

    staffs[staffCount++] = new Staff(id, name, age, role, salary);
    cout << "Staff added\n";
}

void removeStaff() {
    int id;
    cout << "Staff ID to remove: ";
    cin >> id;
    int idx = findStaff(id);
    if (idx == -1)
        throw runtime_error("Staff member not found");

    delete staffs[idx];                     // free the memory
    for (int i = idx; i < staffCount - 1; i++)
        staffs[i] = staffs[i + 1];          // shift the rest one step left
    staffCount--;
    cout << "Staff member removed\n";
}

void viewStaff() {
    if (staffCount == 0)
        throw runtime_error("No staff found");
    for (int i = 0; i < staffCount; i++)
        staffs[i]->display();
}

// ===================== MAIN =====================
int main() {
    // EXCEPTION HANDLING: loading can throw if a CSV file has bad data
    try {
        loadAll();
    }
    catch (exception &e) {
        cout << "Error while loading files: " << e.what() << endl;
    }

    int choice;
    do {
        cout << "\n--- HOSPITAL MENU ---\n";
        cout << "1. Add Doctor\n2. Add Patient\n3. Assign Doctor to Patient\n";
        cout << "4. Update Patient Status\n5. Check Patient Status\n6. Generate Bill\n";
        cout << "7. Add Staff\n8. Remove Staff\n";
        cout << "9. View Doctors\n10. View Patients\n11. View Staff\n0. Exit\nChoice: ";
        cin >> choice;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (choice == 1) addDoctor();
            else if (choice == 2) addPatient();
            else if (choice == 3) assignDoctor();
            else if (choice == 4) updateStatus();
            else if (choice == 5) checkStatus();
            else if (choice == 6) generateBill();
            else if (choice == 7) addStaff();
            else if (choice == 8) removeStaff();
            else if (choice == 9) viewDoctors();
            else if (choice == 10) viewPatients();
            else if (choice == 11) viewStaff();
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }

        saveAll();      // FILE HANDLING: save after every action
    } while (choice != 0);

    // free memory of all dynamically created objects
    for (int i = 0; i < doctorCount; i++) delete doctors[i];
    for (int i = 0; i < patientCount; i++) delete patients[i];
    for (int i = 0; i < staffCount; i++) delete staffs[i];
    return 0;
}
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>     // EXCEPTION HANDLING: runtime_error
using namespace std;

// ===================== BASE CLASS =====================
// CLASS + OBJECT: Person holds the data common to every person
class Person {
protected:   // ENCAPSULATION: hidden from outside, but child classes can use it
    int id;
    string name;
public:
    // CONSTRUCTOR
    Person(int i, string n) {
        id = i;
        name = n;
    }
    virtual ~Person() {}    // virtual destructor (safe delete through base pointer)

    // ENCAPSULATION: getters
    int getId() { return id; }
    string getName() { return name; }

    virtual void display() {
        cout << "ID: " << id << " | Name: " << name;
    }
};

// ===================== DERIVED CLASS =====================
// INHERITANCE: Student is-a Person, so it gets id and name automatically
class Student : public Person {
private:    // ENCAPSULATION: marks and attendance can only change through setters
    double marks;
    int attended;
    int totalClasses;
public:
    // CONSTRUCTOR: passes id and name up to Person with : Person(i, n)
    Student(int i, string n, double m, int att, int tot) : Person(i, n) {
        setMarks(m);                // setters also validate the values
        setAttendance(att, tot);
    }

    // ENCAPSULATION + EXCEPTION HANDLING: setter rejects bad data
    void setMarks(double m) {
        if (m < 0 || m > 100)
            throw runtime_error("Marks must be between 0 and 100");
        marks = m;
    }

    void setAttendance(int att, int tot) {
        if (att < 0 || tot < 0 || att > tot)
            throw runtime_error("Invalid attendance (attended cannot be more than total classes)");
        attended = att;
        totalClasses = tot;
    }

    double getMarks() { return marks; }

    double getAttendancePercent() {
        if (totalClasses == 0) return 0;
        return attended * 100.0 / totalClasses;
    }

    string getGrade() {
        if (marks >= 80) return "A+";
        if (marks >= 70) return "A";
        if (marks >= 60) return "B";
        if (marks >= 50) return "C";
        if (marks >= 40) return "D";
        return "F";
    }

    // OVERRIDING: uses the parent's display first, then adds its own details
    void display() {
        Person::display();
        cout << " | Marks: " << marks
             << " | Grade: " << getGrade()
             << " | Attendance: " << getAttendancePercent() << "%" << endl;
    }

    // FILE HANDLING + CSV: write this student as one CSV line
    void save(ofstream &f) {
        f << id << "," << name << "," << marks << ","
          << attended << "," << totalClasses << endl;
    }
};

// ===================== GLOBAL DATA =====================
Student* students[100];
int total = 0;

// returns index of the student with this id, or -1 if not found
int findIndex(int id) {
    for (int i = 0; i < total; i++)
        if (students[i]->getId() == id)
            return i;
    return -1;
}

// ===================== FILE HANDLING (CSV) =====================
void saveAll() {
    ofstream f("students.csv");     // ofstream = output file stream
    for (int i = 0; i < total; i++)
        students[i]->save(f);
}

void loadAll() {
    ifstream f("students.csv");     // ifstream = input file stream
    if (!f) return;                 // first run: file does not exist, skip

    string idStr, name, marksStr, attStr, totStr;
    // getline(f, x, ',') reads until the next comma (CSV reading)
    while (getline(f, idStr, ',')) {
        getline(f, name, ',');
        getline(f, marksStr, ',');
        getline(f, attStr, ',');
        getline(f, totStr);         // last column: read until end of line
        if (total < 100)
            students[total++] = new Student(stoi(idStr), name, stod(marksStr),
                                            stoi(attStr), stoi(totStr));
    }
}

// ===================== FUNCTIONS =====================
void addStudent() {
    int id, att, tot;
    string name;
    double marks;

    cout << "ID: ";
    cin >> id;
    if (total >= 100)
        throw runtime_error("Student list is full");
    if (findIndex(id) != -1)
        throw runtime_error("A student with this ID already exists");

    cout << "Name (one word): ";
    cin >> name;
    cout << "Marks (0-100): ";
    cin >> marks;
    cout << "Classes attended: ";
    cin >> att;
    cout << "Total classes: ";
    cin >> tot;

    // the constructor validates the values and throws if they are wrong
    students[total++] = new Student(id, name, marks, att, tot);
    cout << "Student added\n";
}

void deleteStudent() {
    int id;
    cout << "ID to delete: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Student not found");

    delete students[idx];                  // free the memory
    for (int i = idx; i < total - 1; i++)
        students[i] = students[i + 1];     // shift the rest one step left
    total--;
    cout << "Student deleted\n";
}

void updateStudent() {
    int id, ch;
    cout << "ID to update: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Student not found");

    cout << "1. Update marks\n2. Update attendance\nChoice: ";
    cin >> ch;

    if (ch == 1) {
        double m;
        cout << "New marks: ";
        cin >> m;
        students[idx]->setMarks(m);        // setter checks the value
        cout << "Marks updated\n";
    } else if (ch == 2) {
        int att, tot;
        cout << "Classes attended: ";
        cin >> att;
        cout << "Total classes: ";
        cin >> tot;
        students[idx]->setAttendance(att, tot);
        cout << "Attendance updated\n";
    }
}

void viewAll() {
    if (total == 0)
        throw runtime_error("No student records");
    for (int i = 0; i < total; i++)
        students[i]->display();     // each object uses its own display()
}

// REPORT: prints a summary and also writes it to report.csv
void generateReport() {
    if (total == 0)
        throw runtime_error("No student records");

    ofstream r("report.csv");       // FILE HANDLING + CSV
    r << "ID,Name,Marks,Grade,Attendance%" << endl;    // header line

    double sum = 0;
    int top = 0;                    // index of the student with highest marks
    for (int i = 0; i < total; i++) {
        sum += students[i]->getMarks();
        if (students[i]->getMarks() > students[top]->getMarks())
            top = i;
        r << students[i]->getId() << "," << students[i]->getName() << ","
          << students[i]->getMarks() << "," << students[i]->getGrade() << ","
          << students[i]->getAttendancePercent() << endl;
    }

    cout << "\n----- REPORT -----\n";
    viewAll();
    cout << "Total students : " << total << endl;
    cout << "Average marks  : " << sum / total << endl;
    cout << "Topper         : " << students[top]->getName()
         << " (" << students[top]->getMarks() << ")" << endl;
    cout << "Report saved to report.csv\n";
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
        cout << "\n--- STUDENT MENU ---\n";
        cout << "1. Add Student\n2. Delete Student\n3. Update Student\n";
        cout << "4. View All\n5. Generate Report\n0. Exit\nChoice: ";
        cin >> choice;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (choice == 1) addStudent();
            else if (choice == 2) deleteStudent();
            else if (choice == 3) updateStudent();
            else if (choice == 4) viewAll();
            else if (choice == 5) generateReport();
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }

        saveAll();      // FILE HANDLING: save after every action
    } while (choice != 0);

    // free memory of all dynamically created objects
    for (int i = 0; i < total; i++)
        delete students[i];
    return 0;
}
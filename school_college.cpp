// ============================================================
//  SCHOOL / COLLEGE GRADING SYSTEM  (C++ - beginner friendly)
//  Concepts: Classes, Inheritance, File Handling (CSV),
//            Exception Handling, Encapsulation, Polymorphism
// ============================================================

#include <iostream>
#include <fstream>    // FILE HANDLING: ifstream / ofstream
#include <sstream>    // to split a CSV line
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>  // EXCEPTION HANDLING: invalid_argument, runtime_error
using namespace std;

const int NUM_SUBJECTS = 5;
const string SUBJECT_NAMES[NUM_SUBJECTS] = {"Math", "English", "Physics", "Chemistry", "CS"};

// ============================================================
// CLASS 1: Person  (BASE CLASS)
// Holds what every person in the school has: a name.
// ============================================================
class Person {
private:
    // ENCAPSULATION: private data, changed only through setters
    string name;

public:
    Person() { name = ""; }

    Person(string name) {
        setName(name);
    }

    virtual ~Person() {}   // virtual destructor (needed for inheritance with pointers)

    void setName(string newName) {
        if (newName.empty())
            throw invalid_argument("Name cannot be empty.");              // EXCEPTION thrown
        if (newName.find(',') != string::npos)
            throw invalid_argument("Name cannot contain a comma (CSV file).");
        name = newName;
    }

    string getName() const { return name; }

    // POLYMORPHISM: child classes give their own version of this
    virtual string getRole() const { return "Person"; }
};

// ============================================================
// CLASS 2: Teacher  (DERIVED CLASS)
// INHERITANCE: Teacher inherits from Person (gets name for free)
// ============================================================
class Teacher : public Person {
private:
    string subject;   // ENCAPSULATION

public:
    Teacher(string name, string subject) : Person(name) {   // calls base constructor
        this->subject = subject;
    }

    string getSubject() const { return subject; }

    // POLYMORPHISM: overriding the base class function
    string getRole() const override { return "Teacher"; }
};

// ============================================================
// CLASS 3: Student  (DERIVED CLASS)
// INHERITANCE: Student inherits from Person and adds
// an ID, marks, and GPA calculation.
// ============================================================
class Student : public Person {
private:
    // ENCAPSULATION: marks are private, so nobody can put 150 marks directly
    int id;
    double marks[NUM_SUBJECTS];

public:
    // CONSTRUCTOR: calls base class constructor Person(name)
    Student(int id, string name, double m[]) : Person(name) {
        setId(id);
        for (int i = 0; i < NUM_SUBJECTS; i++)
            setMark(i, m[i]);   // setter checks that marks are valid
    }

    // ---------- SETTERS with EXCEPTION HANDLING ----------
    void setId(int newId) {
        if (newId <= 0)
            throw invalid_argument("ID must be greater than 0.");
        id = newId;
    }

    void setMark(int subjectIndex, double mark) {
        if (subjectIndex < 0 || subjectIndex >= NUM_SUBJECTS)
            throw invalid_argument("Wrong subject number.");
        if (mark < 0 || mark > 100)
            throw invalid_argument("Marks must be between 0 and 100.");   // EXCEPTION thrown
        marks[subjectIndex] = mark;
    }

    // ---------- GETTERS ----------
    int getId() const { return id; }
    double getMark(int i) const { return marks[i]; }

    // POLYMORPHISM: overriding getRole()
    string getRole() const override { return "Student"; }

    // ---------- GRADE CALCULATIONS ----------
    // Converts marks to grade point (4.00 scale)
    static double markToPoint(double m) {
        if (m >= 80) return 4.00;
        if (m >= 75) return 3.75;
        if (m >= 70) return 3.50;
        if (m >= 65) return 3.25;
        if (m >= 60) return 3.00;
        if (m >= 55) return 2.75;
        if (m >= 50) return 2.50;
        if (m >= 45) return 2.25;
        if (m >= 40) return 2.00;
        return 0.00;
    }

    // Converts marks to letter grade
    static string markToLetter(double m) {
        if (m >= 80) return "A+";
        if (m >= 75) return "A";
        if (m >= 70) return "A-";
        if (m >= 65) return "B+";
        if (m >= 60) return "B";
        if (m >= 55) return "B-";
        if (m >= 50) return "C+";
        if (m >= 45) return "C";
        if (m >= 40) return "D";
        return "F";
    }

    // GPA = average of the grade points of all subjects
    double calculateGPA() const {
        double total = 0;
        for (int i = 0; i < NUM_SUBJECTS; i++)
            total += markToPoint(marks[i]);
        return total / NUM_SUBJECTS;
    }

    double totalMarks() const {
        double total = 0;
        for (int i = 0; i < NUM_SUBJECTS; i++)
            total += marks[i];
        return total;
    }

    // A student fails if any subject is below 40
    bool hasPassed() const {
        for (int i = 0; i < NUM_SUBJECTS; i++) {
            if (marks[i] < 40)
                return false;
        }
        return true;
    }

    // ---------- DISPLAY ----------
    void displayRow() const {
        cout << left << setw(6) << id << setw(16) << getName();
        for (int i = 0; i < NUM_SUBJECTS; i++)
            cout << setw(10) << marks[i];
        cout << fixed << setprecision(2) << setw(7) << calculateGPA()
             << setw(8) << (hasPassed() ? "PASS" : "FAIL") << endl;
    }

    void displayDetails() const {
        cout << "\nID   : " << id << endl;
        cout << "Name : " << getName() << endl;
        cout << "-----------------------------------\n";
        for (int i = 0; i < NUM_SUBJECTS; i++) {
            cout << left << setw(12) << SUBJECT_NAMES[i]
                 << setw(8) << marks[i]
                 << setw(5) << markToLetter(marks[i])
                 << fixed << setprecision(2) << markToPoint(marks[i]) << endl;
        }
        cout << "-----------------------------------\n";
        cout << "Total marks : " << totalMarks() << endl;
        cout << "GPA         : " << fixed << setprecision(2) << calculateGPA() << endl;
        cout << "Result      : " << (hasPassed() ? "PASS" : "FAIL") << endl;
    }

    // Converts the object into one CSV line (used by FILE HANDLING)
    string toCSV() const {
        string line = to_string(id) + "," + getName();
        for (int i = 0; i < NUM_SUBJECTS; i++)
            line += "," + to_string(marks[i]);
        return line;
    }
};

// ============================================================
// CLASS 4: GradeBook  (manages all students + file handling)
// ============================================================
class GradeBook {
private:
    vector<Student*> students;
    string fileName;

    int findIndex(int id) const {
        for (int i = 0; i < (int)students.size(); i++) {
            if (students[i]->getId() == id)
                return i;
        }
        return -1;
    }

public:
    // CONSTRUCTOR: sets file name and loads old records
    GradeBook(string file) {
        fileName = file;
        loadFromFile();
    }

    // DESTRUCTOR: saves data and frees memory
    ~GradeBook() {
        saveToFile();
        for (int i = 0; i < (int)students.size(); i++)
            delete students[i];
        students.clear();
    }

    // ---------- ADD STUDENT ----------
    void addStudent(int id, string name, double m[]) {
        if (findIndex(id) != -1)
            throw runtime_error("A student with ID " + to_string(id) + " already exists.");
        students.push_back(new Student(id, name, m));   // may throw if data is wrong
        cout << "Student added.\n";
    }

    // ---------- UPDATE ONE SUBJECT MARK ----------
    void updateMark(int id, int subjectNumber, double mark) {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Student not found.");
        students[index]->setMark(subjectNumber - 1, mark);
        cout << "Mark updated. New GPA: " << fixed << setprecision(2)
             << students[index]->calculateGPA() << endl;
    }

    // ---------- SEARCH ----------
    void showStudent(int id) const {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Student not found.");
        students[index]->displayDetails();
    }

    // ---------- DELETE ----------
    void deleteStudent(int id) {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Student not found.");
        delete students[index];
        students.erase(students.begin() + index);
        cout << "Student deleted.\n";
    }

    // ---------- TABLE HEADER ----------
    void printHeader() const {
        cout << left << setw(6) << "ID" << setw(16) << "Name";
        for (int i = 0; i < NUM_SUBJECTS; i++)
            cout << setw(10) << SUBJECT_NAMES[i];
        cout << setw(7) << "GPA" << setw(8) << "Result" << endl;
        cout << "-----------------------------------------------------------------------------------\n";
    }

    void showAll() const {
        if (students.empty()) {
            cout << "No students yet.\n";
            return;
        }
        printHeader();
        for (int i = 0; i < (int)students.size(); i++)
            students[i]->displayRow();
    }

    // ---------- PERFORMANCE REPORT ----------
    void generateReport(const Teacher& teacher) const {
        if (students.empty()) {
            cout << "No students yet.\n";
            return;
        }

        // copy the pointers so we can sort without changing the original order
        vector<Student*> sorted = students;

        // simple bubble sort by GPA (highest first) -> gives the rank list
        for (int i = 0; i < (int)sorted.size() - 1; i++) {
            for (int j = 0; j < (int)sorted.size() - 1 - i; j++) {
                if (sorted[j]->calculateGPA() < sorted[j + 1]->calculateGPA()) {
                    Student* temp = sorted[j];
                    sorted[j] = sorted[j + 1];
                    sorted[j + 1] = temp;
                }
            }
        }

        double gpaSum = 0;
        int passCount = 0;
        for (int i = 0; i < (int)students.size(); i++) {
            gpaSum += students[i]->calculateGPA();
            if (students[i]->hasPassed())
                passCount++;
        }
        int total = students.size();

        cout << "\n================ PERFORMANCE REPORT ================\n";
        cout << "Prepared by: " << teacher.getName() << " (" << teacher.getRole()
             << ", " << teacher.getSubject() << ")\n\n";

        cout << "Rank list:\n";
        for (int i = 0; i < (int)sorted.size(); i++) {
            cout << (i + 1) << ". " << left << setw(16) << sorted[i]->getName()
                 << "GPA " << fixed << setprecision(2) << sorted[i]->calculateGPA() << endl;
        }

        cout << "\nSubject averages:\n";
        for (int s = 0; s < NUM_SUBJECTS; s++) {
            double sum = 0;
            for (int i = 0; i < total; i++)
                sum += students[i]->getMark(s);
            cout << "  " << left << setw(12) << SUBJECT_NAMES[s]
                 << fixed << setprecision(2) << sum / total << endl;
        }

        cout << "\nTotal students : " << total << endl;
        cout << "Passed         : " << passCount << endl;
        cout << "Failed         : " << total - passCount << endl;
        cout << "Class avg GPA  : " << fixed << setprecision(2) << gpaSum / total << endl;
        cout << "Topper         : " << sorted[0]->getName() << " (GPA "
             << sorted[0]->calculateGPA() << ")\n";
    }

    // ---------- FILE HANDLING: SAVE TO CSV ----------
    void saveToFile() const {
        ofstream file(fileName);   // opens file for writing
        if (!file.is_open()) {
            cout << "Error: could not open file for saving.\n";
            return;
        }
        file << "id,name";
        for (int i = 0; i < NUM_SUBJECTS; i++)
            file << "," << SUBJECT_NAMES[i];
        file << "\n";                                   // CSV header line

        for (int i = 0; i < (int)students.size(); i++)
            file << students[i]->toCSV() << "\n";       // one line per student
        file.close();
        cout << "Records saved to " << fileName << endl;
    }

    // ---------- FILE HANDLING: LOAD FROM CSV ----------
    void loadFromFile() {
        ifstream file(fileName);   // opens file for reading
        if (!file.is_open()) {
            cout << "No saved file found. Starting empty.\n";
            return;
        }

        string line;
        getline(file, line);   // skip header

        while (getline(file, line)) {
            if (line.empty())
                continue;

            stringstream ss(line);
            string idText, name, markText;
            getline(ss, idText, ',');
            getline(ss, name, ',');

            // EXCEPTION HANDLING: a bad line must not crash the program
            try {
                int id = stoi(idText);   // stoi throws if text is not a number
                double m[NUM_SUBJECTS];
                for (int i = 0; i < NUM_SUBJECTS; i++) {
                    if (!getline(ss, markText, ','))
                        throw runtime_error("missing marks");
                    m[i] = stod(markText);
                }
                students.push_back(new Student(id, name, m));
            }
            catch (exception& e) {
                cout << "Skipped a bad line in file: " << line << endl;
            }
        }
        file.close();
        cout << students.size() << " record(s) loaded from " << fileName << endl;
    }
};

// ============================================================
// Helper functions for safe keyboard input (EXCEPTION HANDLING)
// ============================================================
int readInt(const string& message) {
    while (true) {
        cout << message;
        string text;
        if (!getline(cin, text))
            exit(0);
        try {
            size_t used;
            int value = stoi(text, &used);   // throws if not a number
            if (used != text.size())
                throw invalid_argument("extra characters");
            return value;
        }
        catch (exception& e) {
            cout << "Invalid number. Try again.\n";
        }
    }
}

double readDouble(const string& message) {
    while (true) {
        cout << message;
        string text;
        if (!getline(cin, text))
            exit(0);
        try {
            size_t used;
            double value = stod(text, &used);
            if (used != text.size())
                throw invalid_argument("extra characters");
            return value;
        }
        catch (exception& e) {
            cout << "Invalid number. Try again.\n";
        }
    }
}

string readText(const string& message) {
    cout << message;
    string text;
    if (!getline(cin, text))
        exit(0);
    return text;
}

// ============================================================
// MAIN: menu-driven program
// ============================================================
int main() {
    // Teacher object (INHERITANCE: Teacher is a Person)
    string teacherName = readText("Teacher name: ");
    string teacherSubject = readText("Teacher subject: ");
    Teacher teacher("Unknown", teacherSubject);   // default teacher
    try {
        teacher = Teacher(teacherName, teacherSubject);   // may throw if name is empty
    }
    catch (invalid_argument& e) {
        cout << "Input error: " << e.what() << " Using name 'Unknown'.\n";
    }

    GradeBook book("students.csv");   // constructor loads the file

    int choice;
    do {
        cout << "\n====== GRADING SYSTEM MENU ======\n";
        cout << "1. Add student with marks\n";
        cout << "2. Update a subject mark\n";
        cout << "3. Show one student (details + GPA)\n";
        cout << "4. Show all students\n";
        cout << "5. Delete student\n";
        cout << "6. Performance report\n";
        cout << "7. Save to file\n";
        cout << "0. Exit\n";
        choice = readInt("Enter choice: ");

        // EXCEPTION HANDLING: errors thrown inside the classes are caught here
        try {
            if (choice == 1) {
                int id = readInt("Student ID: ");
                string name = readText("Student name: ");
                double m[NUM_SUBJECTS];
                for (int i = 0; i < NUM_SUBJECTS; i++)
                    m[i] = readDouble("Marks in " + SUBJECT_NAMES[i] + " (0-100): ");
                book.addStudent(id, name, m);
            }
            else if (choice == 2) {
                int id = readInt("Student ID: ");
                for (int i = 0; i < NUM_SUBJECTS; i++)
                    cout << "  " << (i + 1) << ". " << SUBJECT_NAMES[i] << endl;
                int sub = readInt("Subject number: ");
                double mark = readDouble("New marks (0-100): ");
                book.updateMark(id, sub, mark);
            }
            else if (choice == 3) {
                int id = readInt("Student ID: ");
                book.showStudent(id);
            }
            else if (choice == 4) {
                book.showAll();
            }
            else if (choice == 5) {
                int id = readInt("Student ID: ");
                book.deleteStudent(id);
            }
            else if (choice == 6) {
                book.generateReport(teacher);
            }
            else if (choice == 7) {
                book.saveToFile();
            }
            else if (choice != 0) {
                cout << "Wrong choice.\n";
            }
        }
        catch (invalid_argument& e) {
            cout << "Input error: " << e.what() << endl;
        }
        catch (runtime_error& e) {
            cout << "Error: " << e.what() << endl;
        }

    } while (choice != 0);

    cout << "Goodbye!\n";
    return 0;   // 'book' goes out of scope -> its DESTRUCTOR saves the file
}
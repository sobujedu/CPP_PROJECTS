#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

using namespace std;

// ==========================================
// 1. BASE CLASS: Question (Inheritance & Polymorphism)
// ==========================================
class Question {
    // --- ENCAPSULATION ---
    // Protected attributes allow derived question types access while hiding from external code.
protected:
    int id;
    string text;
    string correctAnswer;
    string type;

public:
    Question(int qId, string qText, string qAns, string qType)
        : id(qId), text(qText), correctAnswer(qAns), type(qType) {}

    // Getters
    int getId() const { return id; }
    string getText() const { return text; }
    string getCorrectAnswer() const { return correctAnswer; }
    string getType() const { return type; }

    // --- POLYMORPHISM ---
    // Virtual function overridden by derived question types
    virtual void displayQuestion() const {
        cout << "\nQ" << id << ": " << text << " [" << type << "]\n";
    }

    // --- POLYMORPHISM ---
    virtual bool checkAnswer(const string& userAns) const {
        return toupper(userAns[0]) == toupper(correctAnswer[0]);
    }

    // Virtual serialization method for CSV export
    virtual string toCSV() const {
        return to_string(id) + "," + type + "," + text + "," + correctAnswer;
    }

    virtual ~Question() {}
};

// ==========================================
// 2. DERIVED CLASS: MultipleChoiceQuestion
// ==========================================
// --- INHERITANCE ---
class MultipleChoiceQuestion : public Question {
private:
    string optA, optB, optC, optD;

public:
    MultipleChoiceQuestion(int qId, string qText, string a, string b, string c, string d, string correct)
        : Question(qId, qText, correct, "MCQ"), optA(a), optB(b), optC(c), optD(d) {}

    // --- POLYMORPHISM ---
    void displayQuestion() const override {
        cout << "\nQ" << id << " (MCQ): " << text << "\n";
        cout << "  A. " << optA << "\n";
        cout << "  B. " << optB << "\n";
        cout << "  C. " << optC << "\n";
        cout << "  D. " << optD << "\n";
        cout << "Enter Choice (A/B/C/D): ";
    }

    string toCSV() const override {
        return to_string(id) + ",MCQ," + text + "," + correctAnswer + ","
               + optA + "," + optB + "," + optC + "," + optD;
    }
};

// ==========================================
// 3. DERIVED CLASS: TrueFalseQuestion
// ==========================================
// --- INHERITANCE ---
class TrueFalseQuestion : public Question {
public:
    TrueFalseQuestion(int qId, string qText, string correct)
        : Question(qId, qText, correct, "TF") {}

    // --- POLYMORPHISM ---
    void displayQuestion() const override {
        cout << "\nQ" << id << " (True/False): " << text << "\n";
        cout << "Enter Choice (T for True / F for False): ";
    }

    string toCSV() const override {
        return to_string(id) + ",TF," + text + "," + correctAnswer;
    }
};

// ==========================================
// 4. SYSTEM MANAGER: QuizSystem (File Handling & Operations)
// ==========================================
class QuizSystem {
private:
    vector<Question*> questionBank;
    const string bankFile = "quiz_bank.csv";
    const string resultFile = "quiz_results.csv";

public:
    QuizSystem() {
        loadBankFromCSV();
    }

    ~QuizSystem() {
        for (auto q : questionBank) {
            delete q;
        }
    }

    // --- FILE HANDLING (Read CSV) ---
    void loadBankFromCSV() {
        for (auto q : questionBank) delete q;
        questionBank.clear();

        ifstream file(bankFile);
        if (!file.is_open()) {
            // Seed starter questions if database doesn't exist yet
            questionBank.push_back(new MultipleChoiceQuestion(
                1, "Which OOP concept wraps data and functions?",
                "Inheritance", "Encapsulation", "Polymorphism", "Abstraction", "B"
            ));
            questionBank.push_back(new TrueFalseQuestion(
                2, "Constructors in C++ can have return types.", "F"
            ));
            questionBank.push_back(new MultipleChoiceQuestion(
                3, "Which keyword enables runtime polymorphism in C++?",
                "static", "virtual", "const", "inline", "B"
            ));
            saveBankToCSV();
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, type, text, ans;

            if (getline(ss, idStr, ',') &&
                getline(ss, type, ',') &&
                getline(ss, text, ',') &&
                getline(ss, ans, ',')) {
                try {
                    int id = stoi(idStr);
                    if (type == "MCQ") {
                        string a, b, c, d;
                        if (getline(ss, a, ',') && getline(ss, b, ',') &&
                            getline(ss, c, ',') && getline(ss, d, ',')) {
                            questionBank.push_back(new MultipleChoiceQuestion(id, text, a, b, c, d, ans));
                        }
                    } else if (type == "TF") {
                        questionBank.push_back(new TrueFalseQuestion(id, text, ans));
                    }
                } catch (...) {
                    continue; // Skip malformed rows
                }
            }
        }
        file.close();
    }

    // --- FILE HANDLING (Write CSV) ---
    void saveBankToCSV() {
        ofstream file(bankFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + bankFile + " for saving.");
        }

        for (const auto q : questionBank) {
            file << q->toCSV() << "\n";
        }
        file.close();
    }

    // Admin: Add new question
    void addMCQ(int id, const string& text, const string& a, const string& b, const string& c, const string& d, const string& ans) {
        // --- EXCEPTION HANDLING ---
        for (const auto q : questionBank) {
            if (q->getId() == id) {
                throw runtime_error("Error: Question ID already exists.");
            }
        }
        char upper = toupper(ans[0]);
        if (upper != 'A' && upper != 'B' && upper != 'C' && upper != 'D') {
            throw invalid_argument("Error: MCQ answer must be A, B, C, or D.");
        }

        questionBank.push_back(new MultipleChoiceQuestion(id, text, a, b, c, d, string(1, upper)));
        saveBankToCSV();
        cout << "MCQ Question #" << id << " added successfully!\n";
    }

    void addTF(int id, const string& text, const string& ans) {
        // --- EXCEPTION HANDLING ---
        for (const auto q : questionBank) {
            if (q->getId() == id) {
                throw runtime_error("Error: Question ID already exists.");
            }
        }
        char upper = toupper(ans[0]);
        if (upper != 'T' && upper != 'F') {
            throw invalid_argument("Error: True/False answer must be T or F.");
        }

        questionBank.push_back(new TrueFalseQuestion(id, text, string(1, upper)));
        saveBankToCSV();
        cout << "True/False Question #" << id << " added successfully!\n";
    }

    // User: Take Quiz, evaluate polymorphically, and save result
    void takeQuiz(const string& userName) {
        if (questionBank.empty()) {
            throw runtime_error("Error: Question bank is empty. Ask admin to add questions.");
        }

        int score = 0;
        vector<string> userAnswers;

        cout << "\n=====================================\n";
        cout << "       QUIZ STARTED: " << userName << "\n";
        cout << "=====================================\n";

        for (const auto q : questionBank) {
            // --- POLYMORPHISM IN ACTION ---
            // displayQuestion() resolves dynamically depending on MCQ vs TF
            q->displayQuestion();
            string ans;
            cin >> ans;

            userAnswers.push_back(ans);

            // checkAnswer() resolves polymorphically
            if (q->checkAnswer(ans)) {
                score++;
            }
        }

        cout << "\n========== QUIZ REPORT ==========\n";
        cout << "Student Name : " << userName << "\n";
        cout << "Total Score  : " << score << " / " << questionBank.size() << "\n";
        cout << "Percentage   : " << (score * 100.0) / questionBank.size() << "%\n";
        cout << "---------------------------------\n";
        cout << "Answer Review:\n";
        for (size_t i = 0; i < questionBank.size(); i++) {
            cout << "Q" << questionBank[i]->getId()
                 << " | Your Answer: " << userAnswers[i]
                 << " | Correct: " << questionBank[i]->getCorrectAnswer() << "\n";
        }
        cout << "=================================\n";

        // --- FILE HANDLING (Save Quiz Score to CSV) ---
        ofstream resOut(resultFile, ios::app);
        if (resOut.is_open()) {
            resOut << userName << "," << score << "," << questionBank.size() << "\n";
            resOut.close();
        }
    }

    // View past quiz score records
    void viewAllResults() const {
        ifstream file(resultFile);
        if (!file.is_open()) {
            cout << "No previous quiz results recorded.\n";
            return;
        }

        cout << "\n--- Student Quiz Leaderboard ---\n";
        cout << "Student, Score, Out Of\n";
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                cout << line << endl;
            }
        }
        file.close();
    }
};

// ==========================================
// 5. MAIN FUNCTION: Menu Driver
// ==========================================
int main() {
    QuizSystem system;

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "         ONLINE QUIZ SYSTEM          \n";
        cout << "=====================================\n";
        cout << "1. Take Quiz (Student)\n";
        cout << "2. View Quiz Scores & Leaderboard\n";
        cout << "3. Admin: Add Multiple Choice Question (MCQ)\n";
        cout << "4. Admin: Add True/False Question (TF)\n";
        cout << "5. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        // --- EXCEPTION HANDLING (Centralized Try-Catch) ---
        try {
            if (choice == 1) {
                string name;
                cout << "Enter Student Name: ";
                cin >> name;
                system.takeQuiz(name);
            } else if (choice == 2) {
                system.viewAllResults();
            } else if (choice == 3) {
                int id;
                string text, a, b, c, d, ans;
                cout << "Enter Question ID: "; cin >> id;
                cout << "Enter Question Text: "; cin.ignore(); getline(cin, text);
                cout << "Option A: "; getline(cin, a);
                cout << "Option B: "; getline(cin, b);
                cout << "Option C: "; getline(cin, c);
                cout << "Option D: "; getline(cin, d);
                cout << "Correct Answer (A/B/C/D): "; cin >> ans;
                system.addMCQ(id, text, a, b, c, d, ans);
            } else if (choice == 4) {
                int id;
                string text, ans;
                cout << "Enter Question ID: "; cin >> id;
                cout << "Enter Question Text: "; cin.ignore(); getline(cin, text);
                cout << "Correct Answer (T/F): "; cin >> ans;
                system.addTF(id, text, ans);
            } else if (choice == 5) {
                cout << "Exiting system. Best of luck on your lab exam!\n";
                break;
            } else {
                cout << "Invalid choice. Please select 1 through 5.\n";
            }
        } catch (const exception& e) {
            cout << "\n[Exception Caught] " << e.what() << "\n";
        }
    }

    return 0;
}
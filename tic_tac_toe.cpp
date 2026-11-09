#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <ctime>

using namespace std;

// ==========================================
// 1. BASE CLASS: Player (Inheritance & Polymorphism)
// ==========================================
class Player {
    // --- ENCAPSULATION ---
    // Protected attributes accessible to derived players, hidden from the outside.
protected:
    string name;
    char symbol; // 'X' or 'O'

public:
    Player(string pName, char pSymbol) : name(pName), symbol(pSymbol) {}

    string getName() const { return name; }
    char getSymbol() const { return symbol; }

    // --- POLYMORPHISM ---
    // Pure virtual or virtual function overridden to determine how a move is obtained
    virtual int getMove(const vector<char>& board) = 0;

    virtual ~Player() {}
};

// ==========================================
// 2. DERIVED CLASS: HumanPlayer
// ==========================================
// --- INHERITANCE ---
class HumanPlayer : public Player {
public:
    HumanPlayer(string pName, char pSymbol) : Player(pName, pSymbol) {}

    // --- POLYMORPHISM ---
    int getMove(const vector<char>& board) override {
        int cell;
        cout << name << " (" << symbol << "), enter cell (1-9): ";
        if (!(cin >> cell)) {
            cin.clear();
            string discard;
            cin >> discard;
            throw invalid_argument("Error: Input must be a number between 1 and 9.");
        }
        return cell;
    }
};

// ==========================================
// 3. DERIVED CLASS: BotPlayer (AI / Computer)
// ==========================================
// --- INHERITANCE ---
class BotPlayer : public Player {
public:
    BotPlayer(string pName, char pSymbol) : Player(pName, pSymbol) {}

    // --- POLYMORPHISM ---
    // Bot automatically selects an open cell
    int getMove(const vector<char>& board) override {
        cout << name << " (" << symbol << ") is making a move...\n";
        vector<int> freeCells;
        for (int i = 0; i < 9; i++) {
            if (board[i] != 'X' && board[i] != 'O') {
                freeCells.push_back(i + 1);
            }
        }
        if (freeCells.empty()) return -1;
        int choice = freeCells[rand() % freeCells.size()];
        cout << name << " picked cell " << choice << ".\n";
        return choice;
    }
};

// ==========================================
// 4. CLASS: Board (Encapsulation & Validation)
// ==========================================
class Board {
    // --- ENCAPSULATION ---
private:
    vector<char> grid;

public:
    Board() {
        reset();
    }

    void reset() {
        grid = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    }

    const vector<char>& getGrid() const {
        return grid;
    }

    void display() const {
        cout << "\n";
        cout << " " << grid[0] << " | " << grid[1] << " | " << grid[2] << "\n";
        cout << "---+---+---\n";
        cout << " " << grid[3] << " | " << grid[4] << " | " << grid[5] << "\n";
        cout << "---+---+---\n";
        cout << " " << grid[6] << " | " << grid[7] << " | " << grid[8] << "\n\n";
    }

    // Place a mark with Exception Handling
    void makeMove(int cell, char symbol) {
        // --- EXCEPTION HANDLING ---
        if (cell < 1 || cell > 9) {
            throw out_of_range("Error: Cell index must be between 1 and 9.");
        }
        if (grid[cell - 1] == 'X' || grid[cell - 1] == 'O') {
            throw runtime_error("Error: Cell " + to_string(cell) + " is already occupied.");
        }

        grid[cell - 1] = symbol;
    }

    bool checkWin(char s) const {
        // Winning row/column/diagonal combinations
        int winPatterns[8][3] = {
            {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // Rows
            {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // Columns
            {0, 4, 8}, {2, 4, 6}             // Diagonals
        };

        for (int i = 0; i < 8; i++) {
            if (grid[winPatterns[i][0]] == s &&
                grid[winPatterns[i][1]] == s &&
                grid[winPatterns[i][2]] == s) {
                return true;
            }
        }
        return false;
    }

    bool isFull() const {
        for (char c : grid) {
            if (c != 'X' && c != 'O') return false;
        }
        return true;
    }
};

// ==========================================
// 5. SYSTEM MANAGER: GameManager (File Handling & Loop)
// ==========================================
class GameManager {
private:
    Board board;
    const string scoreFile = "tictactoe_scores.csv";

public:
    // --- FILE HANDLING (Write CSV - Append Match Outcome) ---
    void logMatchResult(const string& p1, const string& p2, const string& winner) {
        ofstream file(scoreFile, ios::app);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + scoreFile + " for saving.");
        }
        file << p1 << "," << p2 << "," << winner << "\n";
        file.close();
    }

    // --- FILE HANDLING (Read CSV - Display Leaderboard/History) ---
    void viewMatchHistory() const {
        ifstream file(scoreFile);
        if (!file.is_open()) {
            cout << "No past match history found.\n";
            return;
        }

        cout << "\n--- Match History (CSV Log) ---\n";
        cout << "Player 1, Player 2, Winner\n";
        string line;
        while (getline(file, line)) {
            if (!line.empty()) {
                cout << line << endl;
            }
        }
        file.close();
    }

    // Main Gameplay Loop Demonstrating Polymorphic Calls
    void playGame(Player* p1, Player* p2) {
        board.reset();
        Player* current = p1;
        bool gameOver = false;

        cout << "\n--- Match Started: " << p1->getName() << " vs " << p2->getName() << " ---\n";

        while (!gameOver) {
            board.display();

            bool validMove = false;
            while (!validMove) {
                // --- EXCEPTION HANDLING (Move Validation) ---
                try {
                    // --- POLYMORPHISM IN ACTION ---
                    // getMove() is resolved dynamically at runtime for HumanPlayer or BotPlayer
                    int move = current->getMove(board.getGrid());
                    board.makeMove(move, current->getSymbol());
                    validMove = true;
                } catch (const exception& e) {
                    cout << "[Move Rejected] " << e.what() << " Try again.\n";
                }
            }

            // Check if current player just won
            if (board.checkWin(current->getSymbol())) {
                board.display();
                cout << "Congratulations! " << current->getName() << " (" << current->getSymbol() << ") wins!\n";
                logMatchResult(p1->getName(), p2->getName(), current->getName());
                gameOver = true;
            } else if (board.isFull()) {
                board.display();
                cout << "It's a draw!\n";
                logMatchResult(p1->getName(), p2->getName(), "Draw");
                gameOver = true;
            } else {
                // Switch turn
                current = (current == p1) ? p2 : p1;
            }
        }
    }
};

// ==========================================
// 6. MAIN FUNCTION: Menu Driver
// ==========================================
int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    GameManager manager;

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "         TIC-TAC-TOE SYSTEM          \n";
        cout << "=====================================\n";
        cout << "1. Human vs Human\n";
        cout << "2. Human vs Bot (Single Player)\n";
        cout << "3. View Match History (CSV)\n";
        cout << "4. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        try {
            if (choice == 1) {
                string name1, name2;
                cout << "Enter Player 1 Name (X): "; cin >> name1;
                cout << "Enter Player 2 Name (O): "; cin >> name2;

                HumanPlayer p1(name1, 'X');
                HumanPlayer p2(name2, 'O');
                manager.playGame(&p1, &p2);

            } else if (choice == 2) {
                string name1;
                cout << "Enter Your Name (X): "; cin >> name1;

                HumanPlayer p1(name1, 'X');
                BotPlayer p2("Computer_Bot", 'O');
                manager.playGame(&p1, &p2);

            } else if (choice == 3) {
                manager.viewMatchHistory();
            } else if (choice == 4) {
                cout << "Exiting Tic-Tac-Toe. Good luck on the exam!\n";
                break;
            } else {
                cout << "Invalid choice. Please pick between 1 and 4.\n";
            }
        } catch (const exception& e) {
            cout << "\n[Exception Caught] " << e.what() << "\n";
        }
    }

    return 0;
}
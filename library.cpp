#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>     // EXCEPTION HANDLING: runtime_error
using namespace std;

// ===================== BOOK CLASS =====================
// CLASS + OBJECT: Book is the blueprint, each real book is an object
class Book {
private:    // ENCAPSULATION: data is hidden, only member functions can change it
    int id;
    string title;
    string author;
    bool issued;
    string issuedTo;
public:
    // CONSTRUCTOR: sets data when a Book object is created
    // (issued and to have default values, so a new book is "not issued")
    Book(int i, string t, string a, bool iss = false, string to = "none") {
        id = i;
        title = t;
        author = a;
        issued = iss;
        issuedTo = to;
    }

    // ENCAPSULATION: getters give read-only access to private data
    int getId() { return id; }
    string getTitle() { return title; }
    bool isIssued() { return issued; }

    void issue(string user) {
        // EXCEPTION HANDLING: cannot issue a book that is already issued
        if (issued)
            throw runtime_error("Book is already issued");
        issued = true;
        issuedTo = user;
    }

    void giveBack() {
        // EXCEPTION HANDLING: cannot return a book that is not issued
        if (!issued)
            throw runtime_error("This book was not issued");
        issued = false;
        issuedTo = "none";
    }

    void display() {
        cout << "ID: " << id
             << " | " << title
             << " | " << author
             << " | " << (issued ? "Issued to " + issuedTo : "Available") << endl;
    }

    // FILE HANDLING + CSV: write this book as one CSV line
    void save(ofstream &f) {
        f << id << "," << title << "," << author << ","
          << issued << "," << issuedTo << endl;
    }
};

// ===================== LIBRARY CLASS =====================
// COMPOSITION: Library has many Book objects
class Library {
private:
    Book* books[100];   // array of pointers to Book objects
    int count;          // how many books exist now

    // returns index of the book with this id, or -1 if not found
    int findIndex(int id) {
        for (int i = 0; i < count; i++)
            if (books[i]->getId() == id)
                return i;
        return -1;
    }

    // FILE HANDLING + CSV: read all books from books.csv
    void load() {
        ifstream f("books.csv");    // ifstream = input file stream
        if (!f) return;             // first run: file does not exist yet, skip

        string idStr, title, author, issStr, to;
        // getline(f, x, ',') reads until the next comma
        while (getline(f, idStr, ',')) {
            getline(f, title, ',');
            getline(f, author, ',');
            getline(f, issStr, ',');
            getline(f, to);          // last column: read until end of line
            if (count < 100)
                books[count++] = new Book(stoi(idStr), title, author, issStr == "1", to);
        }
    }

    // FILE HANDLING + CSV: write all books to books.csv
    void save() {
        ofstream f("books.csv");    // ofstream = output file stream
        for (int i = 0; i < count; i++)
            books[i]->save(f);
    }

public:
    // CONSTRUCTOR: runs when Library object is created, loads old data
    Library() {
        count = 0;
        load();
    }

    // DESTRUCTOR: runs automatically when the object is destroyed (end of main)
    // saves data to the file and frees the memory of all Book objects
    ~Library() {
        save();
        for (int i = 0; i < count; i++)
            delete books[i];
    }

    // ----- Admin functions -----
    void addBook(int id, string title, string author) {
        if (id <= 0)
            throw runtime_error("Invalid book id");
        if (count >= 100)
            throw runtime_error("Library is full");
        if (findIndex(id) != -1)
            throw runtime_error("A book with this id already exists");
        books[count++] = new Book(id, title, author);
        cout << "Book added\n";
    }

    void removeBook(int id) {
        int idx = findIndex(id);
        if (idx == -1)
            throw runtime_error("Book not found");
        delete books[idx];                 // free memory of that book
        for (int i = idx; i < count - 1; i++)
            books[i] = books[i + 1];       // shift the rest one step left
        count--;
        cout << "Book removed\n";
    }

    void viewAll() {
        if (count == 0)
            throw runtime_error("No books in the library");
        for (int i = 0; i < count; i++)
            books[i]->display();
    }

    // ----- User functions -----
    void viewAvailable() {
        bool found = false;
        for (int i = 0; i < count; i++)
            if (!books[i]->isIssued()) {
                books[i]->display();
                found = true;
            }
        if (!found)
            throw runtime_error("No available books");
    }

    void search(string keyword) {
        bool found = false;
        for (int i = 0; i < count; i++)
            // find() returns string::npos when the text is not inside the title
            if (books[i]->getTitle().find(keyword) != string::npos) {
                books[i]->display();
                found = true;
            }
        if (!found)
            throw runtime_error("No book matches your search");
    }

    void issueBook(int id, string user) {
        int idx = findIndex(id);
        if (idx == -1)
            throw runtime_error("Book not found");
        books[idx]->issue(user);           // may throw if already issued
        cout << "Book issued to " << user << endl;
    }

    void returnBook(int id) {
        int idx = findIndex(id);
        if (idx == -1)
            throw runtime_error("Book not found");
        books[idx]->giveBack();            // may throw if not issued
        cout << "Book returned\n";
    }
};

// ===================== MENUS =====================
void adminMenu(Library &lib) {      // & means we work on the real library, not a copy
    int pass;
    cout << "Admin password: ";
    cin >> pass;
    if (pass != 1234)
        throw runtime_error("Wrong admin password");

    int ch;
    cout << "1. Add book\n2. Remove book\n3. View all books\nChoice: ";
    cin >> ch;

    if (ch == 1) {
        int id;
        string title, author;
        cout << "Book id: ";
        cin >> id;
        cin.ignore();                       // clear the leftover newline before getline
        cout << "Title (no commas): ";
        getline(cin, title);
        cout << "Author (no commas): ";
        getline(cin, author);
        lib.addBook(id, title, author);
    } else if (ch == 2) {
        int id;
        cout << "Book id to remove: ";
        cin >> id;
        lib.removeBook(id);
    } else if (ch == 3) {
        lib.viewAll();
    }
}

void userMenu(Library &lib) {
    int ch;
    cout << "1. View available books\n2. Search by title\n"
         << "3. Issue a book\n4. Return a book\nChoice: ";
    cin >> ch;

    if (ch == 1) {
        lib.viewAvailable();
    } else if (ch == 2) {
        string key;
        cout << "Search text: ";
        cin.ignore();
        getline(cin, key);
        lib.search(key);
    } else if (ch == 3) {
        int id;
        string user;
        cout << "Book id: ";
        cin >> id;
        cout << "Your name (one word): ";
        cin >> user;
        lib.issueBook(id, user);
    } else if (ch == 4) {
        int id;
        cout << "Book id: ";
        cin >> id;
        lib.returnBook(id);
    }
}

// ===================== MAIN =====================
int main() {
    Library lib;    // OBJECT: constructor runs here and loads books.csv
    int role;

    do {
        cout << "\n--- LIBRARY MENU ---\n1. Admin\n2. User\n0. Exit\nChoice: ";
        cin >> role;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (role == 1)
                adminMenu(lib);
            else if (role == 2)
                userMenu(lib);
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }
    } while (role != 0);

    return 0;   // lib goes out of scope here, so the DESTRUCTOR runs and saves the file
}
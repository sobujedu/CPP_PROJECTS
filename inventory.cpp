// ============================================================
//  INVENTORY MANAGEMENT SYSTEM  (C++ - beginner friendly)
//  Concepts: Classes, Objects, Constructors, Destructors,
//            File Handling (CSV), Exception Handling,
//            Encapsulation, Inheritance, Polymorphism
// ============================================================

#include <iostream>
#include <fstream>    // FILE HANDLING: ifstream / ofstream
#include <sstream>    // to split a CSV line
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>  // EXCEPTION HANDLING: invalid_argument, runtime_error
using namespace std;

// ============================================================
// CLASS 1: Item  (BASE CLASS)
// ============================================================
class Item {
private:
    // ENCAPSULATION: data members are PRIVATE.
    // Outside code cannot change them directly, only through
    // the public getter/setter functions (which check the data).
    int id;
    string name;
    int quantity;
    double price;

public:
    // CONSTRUCTOR (default): runs when we write  Item a;
    Item() {
        id = 0;
        name = "";
        quantity = 0;
        price = 0.0;
    }

    // CONSTRUCTOR (parameterized): runs when we write  Item a(1,"Pen",10,5.5);
    // It uses the setters so wrong data is rejected (exception thrown).
    Item(int id, string name, int quantity, double price) {
        setId(id);
        setName(name);
        setQuantity(quantity);
        setPrice(price);
    }

    // DESTRUCTOR: runs automatically when the object is deleted.
    // 'virtual' so the child class destructor also runs (INHERITANCE).
    virtual ~Item() {
        cout << "   [Destructor] Item '" << name << "' removed from memory.\n";
    }

    // ---------- SETTERS (ENCAPSULATION + EXCEPTION HANDLING) ----------
    void setId(int newId) {
        if (newId <= 0)
            throw invalid_argument("ID must be greater than 0.");   // EXCEPTION thrown
        id = newId;
    }
    void setName(string newName) {
        if (newName.empty())
            throw invalid_argument("Name cannot be empty.");        // EXCEPTION thrown
        name = newName;
    }
    void setQuantity(int newQty) {
        if (newQty < 0)
            throw invalid_argument("Quantity cannot be negative."); // EXCEPTION thrown
        quantity = newQty;
    }
    void setPrice(double newPrice) {
        if (newPrice < 0)
            throw invalid_argument("Price cannot be negative.");    // EXCEPTION thrown
        price = newPrice;
    }

    // ---------- GETTERS (ENCAPSULATION: read-only access) ----------
    int getId() const { return id; }
    string getName() const { return name; }
    int getQuantity() const { return quantity; }
    double getPrice() const { return price; }

    // ---------- STOCK FUNCTIONS ----------
    void addStock(int amount) {
        if (amount <= 0)
            throw invalid_argument("Amount to add must be positive.");
        quantity = quantity + amount;
    }

    void removeStock(int amount) {
        if (amount <= 0)
            throw invalid_argument("Amount to remove must be positive.");
        if (amount > quantity)
            throw runtime_error("Not enough stock! Only " + to_string(quantity) + " available.");
        quantity = quantity - amount;
    }

    // POLYMORPHISM: 'virtual' lets the child class give its own version.
    virtual void display() const {
        cout << left << setw(6) << id
             << setw(15) << name
             << setw(10) << quantity
             << setw(10) << price
             << setw(12) << "-" << endl;   // no expiry for normal items
    }

    // Converts the object to one CSV line (used for FILE HANDLING).
    virtual string toCSV() const {
        return "NORMAL," + to_string(id) + "," + name + "," +
               to_string(quantity) + "," + to_string(price) + ",-";
    }
};

// ============================================================
// CLASS 2: PerishableItem  (DERIVED CLASS)
// INHERITANCE: PerishableItem inherits everything from Item
// and adds one new thing: an expiry date.
// ============================================================
class PerishableItem : public Item {
private:
    string expiryDate;   // ENCAPSULATION: private data of the child class

public:
    // CONSTRUCTOR: calls the base class constructor first (Item(...))
    PerishableItem(int id, string name, int quantity, double price, string expiry)
        : Item(id, name, quantity, price) {
        expiryDate = expiry;
    }

    // DESTRUCTOR of the child class (runs first, then Item's destructor)
    ~PerishableItem() {
        cout << "   [Destructor] Perishable item expiry info cleared.\n";
    }

    string getExpiryDate() const { return expiryDate; }

    // POLYMORPHISM: overriding the base class display()
    void display() const override {
        cout << left << setw(6) << getId()
             << setw(15) << getName()
             << setw(10) << getQuantity()
             << setw(10) << getPrice()
             << setw(12) << expiryDate << endl;
    }

    // POLYMORPHISM: overriding toCSV() to also save the expiry date
    string toCSV() const override {
        return "PERISHABLE," + to_string(getId()) + "," + getName() + "," +
               to_string(getQuantity()) + "," + to_string(getPrice()) + "," + expiryDate;
    }
};

// ============================================================
// CLASS 3: Inventory  (manages all items + file handling)
// ============================================================
class Inventory {
private:
    vector<Item*> items;   // list of pointers to items (base pointer can hold child objects)
    string fileName;       // CSV file name

    // Finds the position of an item by ID. Returns -1 if not found.
    int findIndex(int id) const {
        for (int i = 0; i < (int)items.size(); i++) {
            if (items[i]->getId() == id)
                return i;
        }
        return -1;
    }

public:
    // CONSTRUCTOR: sets the file name and loads old data from the file.
    Inventory(string file) {
        fileName = file;
        loadFromFile();
    }

    // DESTRUCTOR: saves the data to the file and frees memory.
    ~Inventory() {
        cout << "\n[Destructor] Saving inventory and cleaning memory...\n";
        saveToFile();
        for (int i = 0; i < (int)items.size(); i++) {
            delete items[i];   // calls Item / PerishableItem destructor
        }
        items.clear();
    }

    // ---------- ADD A NEW PRODUCT ----------
    void addItem(Item* newItem) {
        if (findIndex(newItem->getId()) != -1) {
            int id = newItem->getId();
            delete newItem;   // we don't need it, avoid memory leak
            throw runtime_error("An item with ID " + to_string(id) + " already exists.");
        }
        items.push_back(newItem);
        cout << "Item added successfully.\n";
    }

    // ---------- UPDATE STOCK ----------
    void increaseStock(int id, int amount) {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Item not found.");
        items[index]->addStock(amount);
        cout << "Stock updated. New quantity: " << items[index]->getQuantity() << endl;
    }

    void decreaseStock(int id, int amount) {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Item not found.");
        items[index]->removeStock(amount);
        cout << "Stock updated. New quantity: " << items[index]->getQuantity() << endl;
    }

    // ---------- SEARCH ----------
    void searchItem(int id) const {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Item not found.");
        printHeader();
        items[index]->display();   // POLYMORPHISM: correct display() is called
    }

    // ---------- DELETE ----------
    void deleteItem(int id) {
        int index = findIndex(id);
        if (index == -1)
            throw runtime_error("Item not found.");
        delete items[index];
        items.erase(items.begin() + index);
        cout << "Item deleted.\n";
    }

    // ---------- INVENTORY REPORT ----------
    void printHeader() const {
        cout << left << setw(6) << "ID" << setw(15) << "Name"
             << setw(10) << "Qty" << setw(10) << "Price"
             << setw(12) << "Expiry" << endl;
        cout << "-----------------------------------------------------\n";
    }

    void generateReport() const {
        if (items.empty()) {
            cout << "Inventory is empty.\n";
            return;
        }
        double totalValue = 0;
        int totalUnits = 0;

        cout << "\n=============== INVENTORY REPORT ===============\n";
        printHeader();
        for (int i = 0; i < (int)items.size(); i++) {
            items[i]->display();   // POLYMORPHISM
            totalValue += items[i]->getQuantity() * items[i]->getPrice();
            totalUnits += items[i]->getQuantity();
        }
        cout << "-----------------------------------------------------\n";
        cout << "Total products   : " << items.size() << endl;
        cout << "Total units      : " << totalUnits << endl;
        cout << "Total stock value: " << totalValue << endl;
    }

    void lowStockReport(int limit) const {
        bool found = false;
        cout << "\nItems with stock <= " << limit << ":\n";
        printHeader();
        for (int i = 0; i < (int)items.size(); i++) {
            if (items[i]->getQuantity() <= limit) {
                items[i]->display();
                found = true;
            }
        }
        if (!found)
            cout << "No low stock items.\n";
    }

    // ---------- FILE HANDLING: SAVE TO CSV ----------
    void saveToFile() const {
        ofstream file(fileName);   // opens file for writing (old content is replaced)
        if (!file.is_open()) {
            cout << "Error: could not open file for saving.\n";
            return;
        }
        file << "type,id,name,quantity,price,expiry\n";   // CSV header line
        for (int i = 0; i < (int)items.size(); i++) {
            file << items[i]->toCSV() << "\n";             // one line per item
        }
        file.close();
        cout << "Data saved to " << fileName << endl;
    }

    // ---------- FILE HANDLING: LOAD FROM CSV ----------
    void loadFromFile() {
        ifstream file(fileName);   // opens file for reading
        if (!file.is_open()) {
            cout << "No saved file found. Starting with empty inventory.\n";
            return;
        }

        string line;
        getline(file, line);   // skip the header line

        while (getline(file, line)) {
            if (line.empty())
                continue;

            // Split the line by commas
            stringstream ss(line);
            string type, idText, name, qtyText, priceText, expiry;
            getline(ss, type, ',');
            getline(ss, idText, ',');
            getline(ss, name, ',');
            getline(ss, qtyText, ',');
            getline(ss, priceText, ',');
            getline(ss, expiry, ',');

            // EXCEPTION HANDLING: a bad line in the file must not crash the program
            try {
                int id = stoi(idText);          // stoi throws if not a number
                int qty = stoi(qtyText);
                double price = stod(priceText);

                if (type == "PERISHABLE")
                    items.push_back(new PerishableItem(id, name, qty, price, expiry));
                else
                    items.push_back(new Item(id, name, qty, price));
            }
            catch (exception& e) {
                cout << "Skipped a bad line in file: " << line << endl;
            }
        }
        file.close();
        cout << items.size() << " item(s) loaded from " << fileName << endl;
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
            exit(0);   // input ended
        try {
            size_t used;
            int value = stoi(text, &used);   // throws if text is not a number
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
    Inventory inv("inventory.csv");   // OBJECT created -> constructor loads the file

    int choice;
    do {
        cout << "\n====== INVENTORY MENU ======\n";
        cout << "1. Add normal product\n";
        cout << "2. Add perishable product\n";
        cout << "3. Increase stock\n";
        cout << "4. Decrease stock (sell)\n";
        cout << "5. Search product\n";
        cout << "6. Delete product\n";
        cout << "7. Inventory report\n";
        cout << "8. Low stock report\n";
        cout << "9. Save to file\n";
        cout << "0. Exit\n";
        choice = readInt("Enter choice: ");

        // EXCEPTION HANDLING: all errors thrown by the classes are caught here
        try {
            if (choice == 1) {
                int id = readInt("ID: ");
                string name = readText("Name: ");
                int qty = readInt("Quantity: ");
                double price = readDouble("Price: ");
                inv.addItem(new Item(id, name, qty, price));
            }
            else if (choice == 2) {
                int id = readInt("ID: ");
                string name = readText("Name: ");
                int qty = readInt("Quantity: ");
                double price = readDouble("Price: ");
                string expiry = readText("Expiry date (DD-MM-YYYY): ");
                inv.addItem(new PerishableItem(id, name, qty, price, expiry));
            }
            else if (choice == 3) {
                int id = readInt("Item ID: ");
                int amount = readInt("Amount to add: ");
                inv.increaseStock(id, amount);
            }
            else if (choice == 4) {
                int id = readInt("Item ID: ");
                int amount = readInt("Amount to remove: ");
                inv.decreaseStock(id, amount);
            }
            else if (choice == 5) {
                int id = readInt("Item ID: ");
                inv.searchItem(id);
            }
            else if (choice == 6) {
                int id = readInt("Item ID: ");
                inv.deleteItem(id);
            }
            else if (choice == 7) {
                inv.generateReport();
            }
            else if (choice == 8) {
                int limit = readInt("Show items with stock up to: ");
                inv.lowStockReport(limit);
            }
            else if (choice == 9) {
                inv.saveToFile();
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
    return 0;   // 'inv' goes out of scope here -> its DESTRUCTOR runs automatically
}
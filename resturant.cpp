#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. BASE CLASS: MenuItem (Encapsulation, Inheritance, Polymorphism)
// ==========================================
class MenuItem {
    // --- ENCAPSULATION ---
    // Protected attributes allow derived categories access while hiding them from external modification.
protected:
    int id;
    string name;
    double basePrice;
    string category;

public:
    MenuItem(int mId, string mName, double price, string cat)
        : id(mId), name(mName), basePrice(price), category(cat) {}

    // Getters
    int getId() const { return id; }
    string getName() const { return name; }
    double getBasePrice() const { return basePrice; }
    string getCategory() const { return category; }

    // --- POLYMORPHISM ---
    // Virtual function overridden by derived classes to compute final item pricing
    virtual double calculateItemPrice() const {
        return basePrice;
    }

    virtual void display() const {
        cout << "ID: " << id
             << " | " << name
             << " | Category: " << category
             << " | Base Price: $" << basePrice << endl;
    }

    virtual ~MenuItem() {}
};

// ==========================================
// 2. DERIVED CLASS: FoodItem (Inherits from MenuItem)
// ==========================================
// --- INHERITANCE ---
class FoodItem : public MenuItem {
private:
    double vatRate = 0.05; // 5% VAT on cooked food

public:
    FoodItem(int mId, string mName, double price)
        : MenuItem(mId, mName, price, "Food") {}

    // --- POLYMORPHISM ---
    double calculateItemPrice() const override {
        return basePrice + (basePrice * vatRate);
    }
};

// ==========================================
// 3. DERIVED CLASS: BeverageItem (Inherits from MenuItem)
// ==========================================
// --- INHERITANCE ---
class BeverageItem : public MenuItem {
private:
    double recyclingDeposit = 0.50; // Flat recycling deposit on beverages

public:
    BeverageItem(int mId, string mName, double price)
        : MenuItem(mId, mName, price, "Beverage") {}

    // --- POLYMORPHISM ---
    double calculateItemPrice() const override {
        return basePrice + recyclingDeposit;
    }
};

// ==========================================
// 4. CLASS: DiningTable (Encapsulation)
// ==========================================
class DiningTable {
    // --- ENCAPSULATION ---
private:
    int tableNumber;
    bool isOccupied;
    vector<MenuItem*> orderedItems;

public:
    DiningTable(int tNum) : tableNumber(tNum), isOccupied(false) {}

    int getTableNumber() const { return tableNumber; }
    bool getIsOccupied() const { return isOccupied; }
    const vector<MenuItem*>& getOrders() const { return orderedItems; }

    void occupy() { isOccupied = true; }

    void addOrderItem(MenuItem* item) {
        orderedItems.push_back(item);
    }

    void clearTable() {
        orderedItems.clear();
        isOccupied = false;
    }
};

// ==========================================
// 5. SYSTEM MANAGER: RestaurantSystem (File Handling & Operations)
// ==========================================
class RestaurantSystem {
private:
    vector<MenuItem*> menu;
    vector<DiningTable> tables;
    const string menuFile = "menu.csv";
    const string billsFile = "bills.csv";

public:
    RestaurantSystem(int totalTables = 5) {
        for (int i = 1; i <= totalTables; i++) {
            tables.push_back(DiningTable(i));
        }
        loadMenuFromCSV();
    }

    ~RestaurantSystem() {
        for (auto item : menu) {
            delete item;
        }
    }

    // --- FILE HANDLING (Read CSV) ---
    void loadMenuFromCSV() {
        for (auto item : menu) delete item;
        menu.clear();

        ifstream file(menuFile);
        if (!file.is_open()) {
            // Seed a starter menu if file does not exist yet
            addMenuItemDirect(101, "Burger", 6.50, "Food");
            addMenuItemDirect(102, "Pasta", 8.00, "Food");
            addMenuItemDirect(201, "Soda", 1.50, "Beverage");
            addMenuItemDirect(202, "Coffee", 2.50, "Beverage");
            saveMenuToCSV();
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string idStr, name, priceStr, category;

            if (getline(ss, idStr, ',') &&
                getline(ss, name, ',') &&
                getline(ss, priceStr, ',') &&
                getline(ss, category, ',')) {
                try {
                    int id = stoi(idStr);
                    double price = stod(priceStr);
                    addMenuItemDirect(id, name, price, category);
                } catch (...) {
                    continue; // Skip damaged rows
                }
            }
        }
        file.close();
    }

    void addMenuItemDirect(int id, const string& name, double price, const string& category) {
        if (category == "Beverage" || category == "beverage") {
            menu.push_back(new BeverageItem(id, name, price));
        } else {
            menu.push_back(new FoodItem(id, name, price));
        }
    }

    // --- FILE HANDLING (Write CSV) ---
    void saveMenuToCSV() {
        ofstream file(menuFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + menuFile + " for saving.");
        }

        for (const auto item : menu) {
            file << item->getId() << ","
                 << item->getName() << ","
                 << item->getBasePrice() << ","
                 << item->getCategory() << "\n";
        }
        file.close();
    }

    // Admin: Add or update menu item
    void addOrUpdateMenuItem(int id, const string& name, double price, const string& category) {
        // --- EXCEPTION HANDLING ---
        if (price <= 0) {
            throw invalid_argument("Error: Price must be greater than zero.");
        }

        for (size_t i = 0; i < menu.size(); i++) {
            if (menu[i]->getId() == id) {
                delete menu[i];
                if (category == "Beverage" || category == "beverage") {
                    menu[i] = new BeverageItem(id, name, price);
                } else {
                    menu[i] = new FoodItem(id, name, price);
                }
                saveMenuToCSV();
                cout << "Menu item updated successfully!\n";
                return;
            }
        }

        addMenuItemDirect(id, name, price, category);
        saveMenuToCSV();
        cout << "New menu item added successfully!\n";
    }

    // Display entire menu
    void displayMenu() const {
        if (menu.empty()) {
            cout << "Menu is currently empty.\n";
            return;
        }
        cout << "\n--- Restaurant Menu ---\n";
        for (const auto item : menu) {
            item->display();
        }
    }

    // View table statuses
    void viewTableStatus() const {
        cout << "\n--- Dining Tables ---\n";
        for (const auto& t : tables) {
            cout << "Table #" << t.getTableNumber() << ": "
                 << (t.getIsOccupied() ? "[Occupied]" : "[Available]")
                 << " | Items Ordered: " << t.getOrders().size() << endl;
        }
    }

    // Place an order for a specific table
    void placeOrder(int tableNum, int menuItemId) {
        // --- EXCEPTION HANDLING ---
        if (tableNum < 1 || tableNum > (int)tables.size()) {
            throw out_of_range("Error: Table number does not exist.");
        }

        MenuItem* foundItem = nullptr;
        for (auto item : menu) {
            if (item->getId() == menuItemId) {
                foundItem = item;
                break;
            }
        }

        if (!foundItem) {
            throw runtime_error("Error: Menu item ID not found.");
        }

        tables[tableNum - 1].occupy();
        tables[tableNum - 1].addOrderItem(foundItem);
        cout << "Added " << foundItem->getName() << " to Table #" << tableNum << "!\n";
    }

    // Generate bill, checkout, and save report
    void generateBill(int tableNum) {
        // --- EXCEPTION HANDLING ---
        if (tableNum < 1 || tableNum > (int)tables.size()) {
            throw out_of_range("Error: Table number does not exist.");
        }

        DiningTable& targetTable = tables[tableNum - 1];
        if (!targetTable.getIsOccupied() || targetTable.getOrders().empty()) {
            throw runtime_error("Error: Table has no active orders to bill.");
        }

        double total = 0.0;
        cout << "\n========== RESTAURANT BILL ==========\n";
        cout << "Table Number: #" << tableNum << "\n";
        cout << "--------------------------------------\n";

        for (const auto item : targetTable.getOrders()) {
            // --- POLYMORPHISM IN ACTION ---
            // calculateItemPrice() executes specific logic depending on FoodItem vs BeverageItem
            double itemCost = item->calculateItemPrice();
            total += itemCost;
            cout << item->getName() << " (" << item->getCategory() << "): $" << itemCost << "\n";
        }

        cout << "--------------------------------------\n";
        cout << "TOTAL PAYABLE: $" << total << "\n";
        cout << "======================================\n";

        // --- FILE HANDLING (Save bill to CSV) ---
        ofstream bFile(billsFile, ios::app);
        if (bFile.is_open()) {
            bFile << tableNum << "," << targetTable.getOrders().size() << "," << total << "\n";
            bFile.close();
        }

        targetTable.clearTable();
        cout << "Table #" << tableNum << " is now settled and available.\n";
    }

    // View past bills
    void viewBillingReports() const {
        ifstream file(billsFile);
        if (!file.is_open()) {
            cout << "No past billing transactions found.\n";
            return;
        }

        cout << "\n--- Master Restaurant Bills ---\n";
        cout << "Table#, Total Items, Total Paid\n";
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
// 6. MAIN FUNCTION: Menu Driver
// ==========================================
int main() {
    RestaurantSystem restaurant(5); // 5 tables available

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "    RESTAURANT MANAGEMENT SYSTEM     \n";
        cout << "=====================================\n";
        cout << "1. View Menu\n";
        cout << "2. View Table Status\n";
        cout << "3. Place Order for Table\n";
        cout << "4. Generate Bill & Checkout\n";
        cout << "5. Admin: Add/Update Menu Item\n";
        cout << "6. Admin: View Billing Reports\n";
        cout << "7. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        // --- EXCEPTION HANDLING ---
        try {
            if (choice == 1) {
                restaurant.displayMenu();
            } else if (choice == 2) {
                restaurant.viewTableStatus();
            } else if (choice == 3) {
                restaurant.displayMenu();
                int tNum, mId;
                cout << "Enter Table Number (1-5): ";
                cin >> tNum;
                cout << "Enter Menu Item ID to order: ";
                cin >> mId;
                restaurant.placeOrder(tNum, mId);
            } else if (choice == 4) {
                int tNum;
                cout << "Enter Table Number to checkout: ";
                cin >> tNum;
                restaurant.generateBill(tNum);
            } else if (choice == 5) {
                int id;
                double price;
                string name, category;
                cout << "Enter Item ID: "; cin >> id;
                cout << "Enter Item Name: "; cin >> name;
                cout << "Enter Base Price: "; cin >> price;
                cout << "Enter Category (Food / Beverage): "; cin >> category;
                restaurant.addOrUpdateMenuItem(id, name, price, category);
            } else if (choice == 6) {
                restaurant.viewBillingReports();
            } else if (choice == 7) {
                cout << "Exiting system. Best of luck on the lab exam!\n";
                break;
            } else {
                cout << "Invalid choice. Please pick between 1 and 7.\n";
            }
        } catch (const exception& e) {
            cout << "\n[Exception Caught] " << e.what() << "\n";
        }
    }

    return 0;
}
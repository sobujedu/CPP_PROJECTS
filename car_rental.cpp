#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. BASE CLASS: RentalCar (Inheritance & Polymorphism)
// ==========================================
class RentalCar {
    // --- ENCAPSULATION ---
    // Protected attributes allow derived car types access while hiding from outside classes.
protected:
    string carId;
    string model;
    double dailyRate;
    bool isRented;
    string category;

public:
    RentalCar(string id, string mModel, double rate, string cat, bool rented = false)
        : carId(id), model(mModel), dailyRate(rate), category(cat), isRented(rented) {}

    // Getters and Setters
    string getCarId() const { return carId; }
    string getModel() const { return model; }
    double getDailyRate() const { return dailyRate; }
    string getCategory() const { return category; }
    bool getIsRented() const { return isRented; }

    void setIsRented(bool status) { isRented = status; }

    // --- POLYMORPHISM ---
    // Virtual function overridden by child classes to compute total rental cost
    virtual double calculateRentalFee(int days) const {
        return days * dailyRate;
    }

    virtual void display() const {
        cout << "Car ID: " << carId
             << " | Model: " << model
             << " | Type: " << category
             << " | Rate: $" << dailyRate << "/day"
             << " | Status: " << (isRented ? "Rented" : "Available") << endl;
    }

    virtual ~RentalCar() {}
};

// ==========================================
// 2. DERIVED CLASS: EconomyCar (Inherits from RentalCar)
// ==========================================
// --- INHERITANCE ---
class EconomyCar : public RentalCar {
public:
    EconomyCar(string id, string mModel, double rate, bool rented = false)
        : RentalCar(id, mModel, rate, "Economy", rented) {}

    // --- POLYMORPHISM ---
    // Standard flat rate per day
    double calculateRentalFee(int days) const override {
        return days * dailyRate;
    }
};

// ==========================================
// 3. DERIVED CLASS: LuxuryCar (Inherits from RentalCar)
// ==========================================
// --- INHERITANCE ---
class LuxuryCar : public RentalCar {
private:
    double insuranceFeePerDay = 25.0; // Daily luxury insurance premium

public:
    LuxuryCar(string id, string mModel, double rate, bool rented = false)
        : RentalCar(id, mModel, rate, "Luxury", rented) {}

    // --- POLYMORPHISM ---
    // Luxury vehicles add daily mandatory insurance
    double calculateRentalFee(int days) const override {
        return (days * dailyRate) + (days * insuranceFeePerDay);
    }
};

// ==========================================
// 4. SYSTEM MANAGER: RentalSystem (File Handling & Operations)
// ==========================================
class RentalSystem {
private:
    vector<RentalCar*> inventory;
    const string carsFile = "cars_inventory.csv";
    const string billingFile = "rental_bills.csv";

public:
    RentalSystem() {
        loadCarsFromCSV();
    }

    ~RentalSystem() {
        for (auto car : inventory) {
            delete car;
        }
    }

    // --- FILE HANDLING (Read CSV) ---
    void loadCarsFromCSV() {
        for (auto car : inventory) delete car;
        inventory.clear();

        ifstream file(carsFile);
        if (!file.is_open()) {
            // Seed sample cars on first run if file not found
            addCarDirect("C101", "Toyota Corolla", 40.0, "Economy", false);
            addCarDirect("C102", "Honda Civic", 45.0, "Economy", false);
            addCarDirect("L201", "BMW 5 Series", 120.0, "Luxury", false);
            addCarDirect("L202", "Mercedes E-Class", 140.0, "Luxury", false);
            saveCarsToCSV();
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, model, rateStr, cat, rentedStr;

            if (getline(ss, id, ',') &&
                getline(ss, model, ',') &&
                getline(ss, rateStr, ',') &&
                getline(ss, cat, ',') &&
                getline(ss, rentedStr, ',')) {
                try {
                    double rate = stod(rateStr);
                    bool rented = (rentedStr == "1");
                    addCarDirect(id, model, rate, cat, rented);
                } catch (...) {
                    continue; // Skip malformed rows
                }
            }
        }
        file.close();
    }

    void addCarDirect(const string& id, const string& model, double rate, const string& cat, bool rented) {
        if (cat == "Luxury" || cat == "luxury") {
            inventory.push_back(new LuxuryCar(id, model, rate, rented));
        } else {
            inventory.push_back(new EconomyCar(id, model, rate, rented));
        }
    }

    // --- FILE HANDLING (Write CSV) ---
    void saveCarsToCSV() {
        ofstream file(carsFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + carsFile + " for saving.");
        }

        for (const auto car : inventory) {
            file << car->getCarId() << ","
                 << car->getModel() << ","
                 << car->getDailyRate() << ","
                 << car->getCategory() << ","
                 << (car->getIsRented() ? "1" : "0") << "\n";
        }
        file.close();
    }

    // Admin: Add a new car to inventory
    void addNewCar(const string& id, const string& model, double rate, const string& category) {
        // --- EXCEPTION HANDLING ---
        if (rate <= 0) {
            throw invalid_argument("Error: Daily rental rate must be positive.");
        }

        for (const auto car : inventory) {
            if (car->getCarId() == id) {
                throw runtime_error("Error: Car ID " + id + " already exists in system.");
            }
        }

        addCarDirect(id, model, rate, category, false);
        saveCarsToCSV();
        cout << "Car " << model << " added to inventory successfully!\n";
    }

    // View available cars for rent
    void viewAvailableCars() const {
        bool found = false;
        cout << "\n--- Available Cars for Rent ---\n";
        for (const auto car : inventory) {
            if (!car->getIsRented()) {
                car->display();
                found = true;
            }
        }
        if (!found) cout << "No cars currently available.\n";
    }

    // Rent a car
    void rentCar(const string& carId, const string& customerName) {
        RentalCar* target = nullptr;
        for (auto car : inventory) {
            if (car->getCarId() == carId) {
                target = car;
                break;
            }
        }

        // --- EXCEPTION HANDLING ---
        if (!target) {
            throw runtime_error("Error: Car ID not found in system.");
        }
        if (target->getIsRented()) {
            throw runtime_error("Error: Car is already rented out.");
        }

        target->setIsRented(true);
        saveCarsToCSV();
        cout << "Success: Car " << target->getModel() << " rented out to " << customerName << "!\n";
    }

    // Return car & generate final billing receipt
    void returnCar(const string& carId, const string& customerName, int days) {
        // --- EXCEPTION HANDLING ---
        if (days <= 0) {
            throw invalid_argument("Error: Rental days must be at least 1.");
        }

        RentalCar* target = nullptr;
        for (auto car : inventory) {
            if (car->getCarId() == carId) {
                target = car;
                break;
            }
        }

        if (!target) {
            throw runtime_error("Error: Car ID not found.");
        }
        if (!target->getIsRented()) {
            throw runtime_error("Error: Car is not currently rented.");
        }

        // --- POLYMORPHISM IN ACTION ---
        // Dynamically invokes calculateRentalFee based on EconomyCar vs LuxuryCar
        double totalBill = target->calculateRentalFee(days);
        target->setIsRented(false);
        saveCarsToCSV();

        // --- FILE HANDLING (Append Bill to CSV) ---
        ofstream billFile(billingFile, ios::app);
        if (billFile.is_open()) {
            billFile << customerName << "," << carId << "," << target->getModel()
                     << "," << target->getCategory() << "," << days << "," << totalBill << "\n";
            billFile.close();
        }

        cout << "\n========== RENTAL RETURN RECEIPT ==========\n";
        cout << "Customer Name : " << customerName << "\n";
        cout << "Car Model     : " << target->getModel() << " (" << target->getCategory() << ")\n";
        cout << "Days Rented   : " << days << "\n";
        cout << "Base Rate     : $" << target->getDailyRate() << "/day\n";
        cout << "Total Amount  : $" << totalBill << "\n";
        cout << "===========================================\n";
    }

    // View historical rental billing reports
    void viewBillingReports() const {
        ifstream file(billingFile);
        if (!file.is_open()) {
            cout << "No past billing records found.\n";
            return;
        }

        cout << "\n--- Master Rental Billing Log ---\n";
        cout << "Customer, Car ID, Model, Category, Days, Total Bill\n";
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
    RentalSystem system;

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "         CAR RENTAL SYSTEM           \n";
        cout << "=====================================\n";
        cout << "1. View Available Cars\n";
        cout << "2. Rent a Car\n";
        cout << "3. Return Car & Generate Bill\n";
        cout << "4. Admin: Add New Car to Fleet\n";
        cout << "5. Admin: View Billing Reports\n";
        cout << "6. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        // --- EXCEPTION HANDLING (Centralized Try-Catch) ---
        try {
            if (choice == 1) {
                system.viewAvailableCars();
            } else if (choice == 2) {
                system.viewAvailableCars();
                string id, name;
                cout << "Enter Car ID to rent: "; cin >> id;
                cout << "Enter Customer Name: "; cin >> name;
                system.rentCar(id, name);
            } else if (choice == 3) {
                string id, name;
                int days;
                cout << "Enter Car ID to return: "; cin >> id;
                cout << "Enter Customer Name: "; cin >> name;
                cout << "Enter Total Rental Days: "; cin >> days;
                system.returnCar(id, name, days);
            } else if (choice == 4) {
                string id, model, category;
                double rate;
                cout << "Enter Car ID: "; cin >> id;
                cout << "Enter Car Model: "; cin.ignore(); getline(cin, model);
                cout << "Enter Daily Rate ($): "; cin >> rate;
                cout << "Enter Category (Economy / Luxury): "; cin >> category;
                system.addNewCar(id, model, rate, category);
            } else if (choice == 5) {
                system.viewBillingReports();
            } else if (choice == 6) {
                cout << "Exiting system. Best of luck on the lab exam!\n";
                break;
            } else {
                cout << "Invalid choice. Please select 1 through 6.\n";
            }
        } catch (const exception& e) {
            cout << "\n[Exception Caught] " << e.what() << "\n";
        }
    }

    return 0;
}
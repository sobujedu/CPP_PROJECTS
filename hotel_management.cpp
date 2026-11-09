#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. BASE CLASS: Room (Inheritance, Polymorphism, Encapsulation)
// ==========================================
class Room {
    // --- ENCAPSULATION ---
    // Protected so derived room types can read them, but hidden from external code.
protected:
    int roomNumber;
    double pricePerNight;
    bool isBooked;
    string roomType;

public:
    Room(int rNum, double price, string type, bool booked = false)
        : roomNumber(rNum), pricePerNight(price), roomType(type), isBooked(booked) {}

    // Getters and Setters
    int getRoomNumber() const { return roomNumber; }
    double getPricePerNight() const { return pricePerNight; }
    string getRoomType() const { return roomType; }
    bool getIsBooked() const { return isBooked; }

    void setBooked(bool status) { isBooked = status; }

    // --- POLYMORPHISM ---
    // Virtual function overridden by derived classes to calculate room-specific bills.
    virtual double calculateBill(int nights) const {
        return pricePerNight * nights;
    }

    virtual void display() const {
        cout << "Room #" << roomNumber
             << " | Type: " << roomType
             << " | Rate: $" << pricePerNight << "/night"
             << " | Status: " << (isBooked ? "Occupied" : "Available") << endl;
    }

    virtual ~Room() {}
};

// ==========================================
// 2. DERIVED CLASS: StandardRoom
// ==========================================
// --- INHERITANCE ---
class StandardRoom : public Room {
public:
    StandardRoom(int rNum, double price, bool booked = false)
        : Room(rNum, price, "Standard", booked) {}

    // --- POLYMORPHISM ---
    // Standard flat rate
    double calculateBill(int nights) const override {
        return pricePerNight * nights;
    }
};

// ==========================================
// 3. DERIVED CLASS: DeluxeRoom
// ==========================================
// --- INHERITANCE ---
class DeluxeRoom : public Room {
private:
    double luxuryTaxRate = 0.10; // 10% tax on deluxe rooms

public:
    DeluxeRoom(int rNum, double price, bool booked = false)
        : Room(rNum, price, "Deluxe", booked) {}

    // --- POLYMORPHISM ---
    // Deluxe rooms include service tax in their bill calculation
    double calculateBill(int nights) const override {
        double subtotal = pricePerNight * nights;
        return subtotal + (subtotal * luxuryTaxRate);
    }
};

// ==========================================
// 4. SYSTEM MANAGER: HotelSystem (File Handling & Operations)
// ==========================================
class HotelSystem {
private:
    vector<Room*> rooms;
    const string roomFile = "rooms.csv";
    const string billFile = "billing_reports.csv";

public:
    HotelSystem() {
        loadRoomsFromCSV();
    }

    ~HotelSystem() {
        // Free dynamically allocated room objects
        for (auto r : rooms) {
            delete r;
        }
    }

    // --- FILE HANDLING (Read CSV) ---
    void loadRoomsFromCSV() {
        for (auto r : rooms) delete r;
        rooms.clear();

        ifstream file(roomFile);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string rNumStr, priceStr, type, bookedStr;

            if (getline(ss, rNumStr, ',') &&
                getline(ss, priceStr, ',') &&
                getline(ss, type, ',') &&
                getline(ss, bookedStr, ',')) {
                try {
                    int rNum = stoi(rNumStr);
                    double price = stod(priceStr);
                    bool booked = (bookedStr == "1");

                    // Creating derived objects based on CSV record
                    if (type == "Deluxe") {
                        rooms.push_back(new DeluxeRoom(rNum, price, booked));
                    } else {
                        rooms.push_back(new StandardRoom(rNum, price, booked));
                    }
                } catch (...) {
                    continue; // Skip malformed rows
                }
            }
        }
        file.close();
    }

    // --- FILE HANDLING (Write CSV) ---
    void saveRoomsToCSV() {
        ofstream file(roomFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + roomFile + " for saving.");
        }

        for (const auto r : rooms) {
            file << r->getRoomNumber() << ","
                 << r->getPricePerNight() << ","
                 << r->getRoomType() << ","
                 << (r->getIsBooked() ? "1" : "0") << "\n";
        }
        file.close();
    }

    // Admin: Add a new room
    void addRoom(int rNum, double price, const string& type) {
        // --- EXCEPTION HANDLING ---
        if (price <= 0) {
            throw invalid_argument("Error: Price per night must be positive.");
        }
        for (const auto r : rooms) {
            if (r->getRoomNumber() == rNum) {
                throw runtime_error("Error: Room number already exists.");
            }
        }

        if (type == "Deluxe" || type == "deluxe") {
            rooms.push_back(new DeluxeRoom(rNum, price, false));
        } else {
            rooms.push_back(new StandardRoom(rNum, price, false));
        }

        saveRoomsToCSV();
        cout << "Room #" << rNum << " added successfully!\n";
    }

    // User: View available rooms
    void viewAvailableRooms() const {
        bool any = false;
        cout << "\n--- Available Rooms ---\n";
        for (const auto r : rooms) {
            if (!r->getIsBooked()) {
                r->display();
                any = true;
            }
        }
        if (!any) cout << "No rooms currently available.\n";
    }

    // User: Book room (Check-In)
    void checkIn(int rNum) {
        Room* target = nullptr;
        for (auto r : rooms) {
            if (r->getRoomNumber() == rNum) {
                target = r;
                break;
            }
        }

        // --- EXCEPTION HANDLING ---
        if (!target) {
            throw runtime_error("Error: Room number not found.");
        }
        if (target->getIsBooked()) {
            throw runtime_error("Error: Room is already occupied.");
        }

        target->setBooked(true);
        saveRoomsToCSV();
        cout << "Check-in successful for Room #" << rNum << "!\n";
    }

    // User / Admin: Check-Out and generate bill
    void checkOut(int rNum, const string& guestName, int nights) {
        // --- EXCEPTION HANDLING ---
        if (nights <= 0) {
            throw invalid_argument("Error: Number of nights must be at least 1.");
        }

        Room* target = nullptr;
        for (auto r : rooms) {
            if (r->getRoomNumber() == rNum) {
                target = r;
                break;
            }
        }

        if (!target) {
            throw runtime_error("Error: Room number not found.");
        }
        if (!target->getIsBooked()) {
            throw runtime_error("Error: Room is not currently occupied.");
        }

        // --- POLYMORPHISM IN ACTION ---
        // calculateBill() resolves dynamically at runtime based on room type
        double totalBill = target->calculateBill(nights);
        target->setBooked(false);
        saveRoomsToCSV();

        // --- FILE HANDLING (Save billing record to CSV) ---
        ofstream billOut(billFile, ios::app);
        if (billOut.is_open()) {
            billOut << guestName << "," << rNum << "," << target->getRoomType() << ","
                    << nights << "," << totalBill << "\n";
            billOut.close();
        }

        cout << "\n========== CHECKOUT RECEIPT ==========\n";
        cout << "Guest Name : " << guestName << "\n";
        cout << "Room Number: " << rNum << " (" << target->getRoomType() << ")\n";
        cout << "Nights     : " << nights << "\n";
        cout << "Total Due  : $" << totalBill << "\n";
        cout << "======================================\n";
    }

    // Admin: View all billing records
    void viewBillingReports() const {
        ifstream file(billFile);
        if (!file.is_open()) {
            cout << "No billing records found.\n";
            return;
        }

        cout << "\n--- Master Billing Report ---\n";
        cout << "Guest, Room#, Type, Nights, Total Bill\n";
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
// 5. MAIN FUNCTION: Menu System
// ==========================================
int main() {
    HotelSystem hotel;

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "      HOTEL MANAGEMENT SYSTEM        \n";
        cout << "=====================================\n";
        cout << "1. View Available Rooms\n";
        cout << "2. Check-In (Book Room)\n";
        cout << "3. Check-Out & Generate Bill\n";
        cout << "4. Admin: Add New Room\n";
        cout << "5. Admin: View Billing Reports\n";
        cout << "6. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        // --- EXCEPTION HANDLING (Centralized Try-Catch) ---
        try {
            if (choice == 1) {
                hotel.viewAvailableRooms();
            } else if (choice == 2) {
                int rNum;
                cout << "Enter Room Number to check-in: ";
                cin >> rNum;
                hotel.checkIn(rNum);
            } else if (choice == 3) {
                int rNum, nights;
                string name;
                cout << "Enter Room Number to check-out: ";
                cin >> rNum;
                cout << "Enter Guest Name: ";
                cin >> name;
                cout << "Enter Number of Nights: ";
                cin >> nights;
                hotel.checkOut(rNum, name, nights);
            } else if (choice == 4) {
                int rNum;
                double price;
                string type;
                cout << "Enter New Room Number: ";
                cin >> rNum;
                cout << "Enter Price per Night: ";
                cin >> price;
                cout << "Enter Room Type (Standard / Deluxe): ";
                cin >> type;
                hotel.addRoom(rNum, price, type);
            } else if (choice == 5) {
                hotel.viewBillingReports();
            } else if (choice == 6) {
                cout << "Exiting system. Best of luck on the lab test!\n";
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
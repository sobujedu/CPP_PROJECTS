#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. BASE CLASS: Vehicle (Inheritance & Polymorphism)
// ==========================================
class Vehicle {
    // --- ENCAPSULATION ---
    // Protected attributes allow derived vehicle classes access while hiding them from outside.
protected:
    string licensePlate;
    string vehicleType;
    double hourlyRate;

public:
    Vehicle(string plate, string type, double rate)
        : licensePlate(plate), vehicleType(type), hourlyRate(rate) {}

    // Getters
    string getLicensePlate() const { return licensePlate; }
    string getVehicleType() const { return vehicleType; }
    double getHourlyRate() const { return hourlyRate; }

    // --- POLYMORPHISM ---
    // Virtual function overridden by child classes to calculate vehicle-specific parking charges.
    virtual double calculateFee(int hours) const {
        return hours * hourlyRate;
    }

    virtual void display() const {
        cout << "Plate: " << licensePlate
             << " | Type: " << vehicleType
             << " | Base Rate: $" << hourlyRate << "/hr" << endl;
    }

    virtual ~Vehicle() {}
};

// ==========================================
// 2. DERIVED CLASS: Bike (Inherits from Vehicle)
// ==========================================
// --- INHERITANCE ---
class Bike : public Vehicle {
public:
    // Bikes have an economical flat base rate of $2.00/hr
    Bike(string plate) : Vehicle(plate, "Bike", 2.0) {}

    // --- POLYMORPHISM ---
    double calculateFee(int hours) const override {
        return hours * hourlyRate;
    }
};

// ==========================================
// 3. DERIVED CLASS: Car (Inherits from Vehicle)
// ==========================================
// --- INHERITANCE ---
class Car : public Vehicle {
public:
    // Standard cars have a base rate of $5.00/hr
    Car(string plate) : Vehicle(plate, "Car", 5.0) {}

    // --- POLYMORPHISM ---
    double calculateFee(int hours) const override {
        return hours * hourlyRate;
    }
};

// ==========================================
// 4. DERIVED CLASS: Bus (Inherits from Vehicle)
// ==========================================
// --- INHERITANCE ---
class Bus : public Vehicle {
private:
    double heavyVehicleSurcharge = 5.0; // Fixed surcharge for oversized parking slots

public:
    // Heavy vehicles have a base rate of $10.00/hr plus an oversized vehicle surcharge
    Bus(string plate) : Vehicle(plate, "Bus", 10.0) {}

    // --- POLYMORPHISM ---
    double calculateFee(int hours) const override {
        return (hours * hourlyRate) + heavyVehicleSurcharge;
    }
};

// ==========================================
// 5. CLASS: ParkingSlot (Encapsulation)
// ==========================================
class ParkingSlot {
    // --- ENCAPSULATION ---
private:
    int slotId;
    bool isOccupied;
    Vehicle* parkedVehicle;

public:
    ParkingSlot(int id) : slotId(id), isOccupied(false), parkedVehicle(nullptr) {}

    int getSlotId() const { return slotId; }
    bool getIsOccupied() const { return isOccupied; }
    Vehicle* getParkedVehicle() const { return parkedVehicle; }

    void park(Vehicle* v) {
        parkedVehicle = v;
        isOccupied = true;
    }

    void vacate() {
        delete parkedVehicle;
        parkedVehicle = nullptr;
        isOccupied = false;
    }

    ~ParkingSlot() {
        if (parkedVehicle != nullptr) {
            delete parkedVehicle;
        }
    }
};

// ==========================================
// 6. SYSTEM MANAGER: ParkingSystem (File Handling & Business Logic)
// ==========================================
class ParkingSystem {
private:
    vector<ParkingSlot*> slots;
    const string parkingStateFile = "parking_slots.csv";
    const string reportsFile = "parking_reports.csv";

public:
    ParkingSystem(int totalSlots = 5) {
        // Initialize default empty slots
        for (int i = 1; i <= totalSlots; i++) {
            slots.push_back(new ParkingSlot(i));
        }
        loadStateFromCSV();
    }

    ~ParkingSystem() {
        for (auto slot : slots) {
            delete slot;
        }
    }

    // --- FILE HANDLING (Read CSV) ---
    void loadStateFromCSV() {
        ifstream file(parkingStateFile);
        if (!file.is_open()) return;

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string slotIdStr, plate, type;

            if (getline(ss, slotIdStr, ',') &&
                getline(ss, plate, ',') &&
                getline(ss, type, ',')) {
                try {
                    int slotId = stoi(slotIdStr);
                    for (auto slot : slots) {
                        if (slot->getSlotId() == slotId && !slot->getIsOccupied()) {
                            Vehicle* v = nullptr;
                            if (type == "Bike") v = new Bike(plate);
                            else if (type == "Car") v = new Car(plate);
                            else if (type == "Bus") v = new Bus(plate);

                            if (v != nullptr) slot->park(v);
                            break;
                        }
                    }
                } catch (...) {
                    continue; // Skip malformed rows
                }
            }
        }
        file.close();
    }

    // --- FILE HANDLING (Write CSV) ---
    void saveStateToCSV() {
        ofstream file(parkingStateFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + parkingStateFile + " for saving.");
        }

        for (const auto slot : slots) {
            if (slot->getIsOccupied() && slot->getParkedVehicle() != nullptr) {
                file << slot->getSlotId() << ","
                     << slot->getParkedVehicle()->getLicensePlate() << ","
                     << slot->getParkedVehicle()->getVehicleType() << "\n";
            }
        }
        file.close();
    }

    // Check available parking slots
    void viewAvailableSpaces() const {
        int freeCount = 0;
        cout << "\n--- Parking Slot Status ---\n";
        for (const auto slot : slots) {
            cout << "Slot #" << slot->getSlotId() << ": ";
            if (slot->getIsOccupied()) {
                cout << "[OCCUPIED] by " << slot->getParkedVehicle()->getVehicleType()
                     << " (" << slot->getParkedVehicle()->getLicensePlate() << ")\n";
            } else {
                cout << "[AVAILABLE]\n";
                freeCount++;
            }
        }
        cout << "Total Available Slots: " << freeCount << "/" << slots.size() << endl;
    }

    // Park a vehicle
    void parkVehicle(int slotId, const string& plate, const string& type) {
        // --- EXCEPTION HANDLING ---
        if (slotId < 1 || slotId > (int)slots.size()) {
            throw out_of_range("Error: Selected slot ID is out of bounds.");
        }

        // Duplicate plate check across all slots
        for (const auto slot : slots) {
            if (slot->getIsOccupied() && slot->getParkedVehicle()->getLicensePlate() == plate) {
                throw runtime_error("Error: Vehicle with plate " + plate + " is already parked.");
            }
        }

        ParkingSlot* target = slots[slotId - 1];
        if (target->getIsOccupied()) {
            throw runtime_error("Error: Slot #" + to_string(slotId) + " is already taken.");
        }

        Vehicle* v = nullptr;
        if (type == "Bike" || type == "bike") {
            v = new Bike(plate);
        } else if (type == "Car" || type == "car") {
            v = new Car(plate);
        } else if (type == "Bus" || type == "bus") {
            v = new Bus(plate);
        } else {
            throw invalid_argument("Error: Invalid vehicle type. Choose Bike, Car, or Bus.");
        }

        target->park(v);
        saveStateToCSV();
        cout << "Vehicle " << plate << " successfully parked in Slot #" << slotId << "!\n";
    }

    // Remove vehicle and collect fee
    void releaseVehicle(int slotId, int hours) {
        // --- EXCEPTION HANDLING ---
        if (slotId < 1 || slotId > (int)slots.size()) {
            throw out_of_range("Error: Invalid slot ID.");
        }
        if (hours <= 0) {
            throw invalid_argument("Error: Parked duration must be at least 1 hour.");
        }

        ParkingSlot* target = slots[slotId - 1];
        if (!target->getIsOccupied()) {
            throw runtime_error("Error: Slot #" + to_string(slotId) + " is already empty.");
        }

        Vehicle* v = target->getParkedVehicle();

        // --- POLYMORPHISM IN ACTION ---
        // Dynamically calls calculateFee() based on whether v points to Bike, Car, or Bus
        double totalFee = v->calculateFee(hours);

        // --- FILE HANDLING (Append exit record to CSV) ---
        ofstream rep(reportsFile, ios::app);
        if (rep.is_open()) {
            rep << slotId << "," << v->getLicensePlate() << ","
                << v->getVehicleType() << "," << hours << "," << totalFee << "\n";
            rep.close();
        }

        cout << "\n========== PARKING RECEIPT ==========\n";
        cout << "Slot ID     : #" << slotId << "\n";
        cout << "Plate Number: " << v->getLicensePlate() << "\n";
        cout << "Vehicle Type: " << v->getVehicleType() << "\n";
        cout << "Total Hours : " << hours << " hr(s)\n";
        cout << "Total Fee   : $" << totalFee << "\n";
        cout << "=====================================\n";

        target->vacate();
        saveStateToCSV();
    }

    // Admin: Increase total capacity
    void addParkingSlot() {
        int nextId = (int)slots.size() + 1;
        slots.push_back(new ParkingSlot(nextId));
        cout << "Admin: Added new Slot #" << nextId << " to the parking lot.\n";
    }

    // Admin: View historical parking records
    void viewParkingReports() const {
        ifstream file(reportsFile);
        if (!file.is_open()) {
            cout << "No past parking session records found.\n";
            return;
        }

        cout << "\n--- Master Parking Logs ---\n";
        cout << "Slot#, Plate, Type, Duration(hrs), Fee Paid\n";
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
// 7. MAIN FUNCTION: Interactive Console Interface
// ==========================================
int main() {
    ParkingSystem system(5); // Default with 5 slots

    int choice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "       VEHICLE PARKING SYSTEM        \n";
        cout << "=====================================\n";
        cout << "1. View Available Parking Spaces\n";
        cout << "2. Park Vehicle (Check-In)\n";
        cout << "3. Release Vehicle & Pay Fee (Check-Out)\n";
        cout << "4. Admin: Add New Parking Slot\n";
        cout << "5. Admin: View Parking Reports\n";
        cout << "6. Exit\n";
        cout << "Enter option: ";

        if (!(cin >> choice)) break;

        // --- EXCEPTION HANDLING (Centralized Try-Catch) ---
        try {
            if (choice == 1) {
                system.viewAvailableSpaces();
            } else if (choice == 2) {
                int slotId;
                string plate, type;
                cout << "Enter Slot ID to park in: ";
                cin >> slotId;
                cout << "Enter License Plate: ";
                cin >> plate;
                cout << "Enter Vehicle Type (Bike / Car / Bus): ";
                cin >> type;
                system.parkVehicle(slotId, plate, type);
            } else if (choice == 3) {
                int slotId, hours;
                cout << "Enter Slot ID to release: ";
                cin >> slotId;
                cout << "Enter Total Parked Hours: ";
                cin >> hours;
                system.releaseVehicle(slotId, hours);
            } else if (choice == 4) {
                system.addParkingSlot();
            } else if (choice == 5) {
                system.viewParkingReports();
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
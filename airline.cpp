#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==========================================
// 1. BASE CLASS: User (Demonstrating Inheritance)
// ==========================================
class User {
    // --- ENCAPSULATION ---
    // Protected attributes allow derived classes to access them,
    // while keeping them hidden from direct outside modification.
protected:
    string username;
    string role;

public:
    User(string uName, string uRole) : username(uName), role(uRole) {}

    // Virtual function to demonstrate Polymorphism / Inheritance behavior
    virtual void displayRole() const {
        cout << "User: " << username << " | Role: " << role << endl;
    }

    string getUsername() const { return username; }
    string getRole() const { return role; }
    virtual ~User() {}
};

// ==========================================
// 2. DERIVED CLASS: Admin (Inherits from User)
// ==========================================
// --- INHERITANCE ---
class Admin : public User {
public:
    Admin(string uName) : User(uName, "Admin") {}

    void displayRole() const override {
        cout << "[Admin Panel] Logged in as: " << username << endl;
    }
};

// ==========================================
// 3. DERIVED CLASS: Passenger (Inherits from User)
// ==========================================
// --- INHERITANCE ---
class Passenger : public User {
public:
    Passenger(string uName) : User(uName, "Passenger") {}

    void displayRole() const override {
        cout << "[Passenger Panel] Logged in as: " << username << endl;
    }
};

// ==========================================
// 4. CLASS: Flight (Demonstrating Encapsulation)
// ==========================================
class Flight {
    // --- ENCAPSULATION ---
    // Private variables with public getter and setter methods
private:
    string flightNumber;
    string origin;
    string destination;
    int availableSeats;

public:
    Flight() : flightNumber(""), origin(""), destination(""), availableSeats(0) {}
    Flight(string fNum, string orig, string dest, int seats)
        : flightNumber(fNum), origin(orig), destination(dest), availableSeats(seats) {}

    // Getters
    string getFlightNumber() const { return flightNumber; }
    string getOrigin() const { return origin; }
    string getDestination() const { return destination; }
    int getAvailableSeats() const { return availableSeats; }

    // Setters
    void setOrigin(string orig) { origin = orig; }
    void setDestination(string dest) { destination = dest; }
    void setAvailableSeats(int seats) { availableSeats = seats; }

    void bookSeat() {
        // --- EXCEPTION HANDLING (Validation) ---
        if (availableSeats <= 0) {
            throw runtime_error("Error: No seats available on flight " + flightNumber);
        }
        availableSeats--;
    }

    void display() const {
        cout << "Flight: " << flightNumber
             << " | Route: " << origin << " -> " << destination
             << " | Available Seats: " << availableSeats << endl;
    }
};

// ==========================================
// 5. SYSTEM MANAGER: AirlineSystem (File Handling & Logic)
// ==========================================
class AirlineSystem {
private:
    vector<Flight> flights;
    const string flightFile = "flights.csv";
    const string bookingFile = "bookings.csv";

public:
    AirlineSystem() {
        loadFlightsFromCSV();
    }

    // --- FILE HANDLING (CSV Read) ---
    void loadFlightsFromCSV() {
        flights.clear();
        ifstream file(flightFile);
        if (!file.is_open()) {
            // File might not exist on first run; will be created on save
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string fNum, orig, dest, seatsStr;

            // Parsing comma-separated values (CSV)
            if (getline(ss, fNum, ',') &&
                getline(ss, orig, ',') &&
                getline(ss, dest, ',') &&
                getline(ss, seatsStr, ',')) {
                try {
                    int seats = stoi(seatsStr);
                    flights.push_back(Flight(fNum, orig, dest, seats));
                } catch (...) {
                    // Skip corrupt rows
                    continue;
                }
            }
        }
        file.close();
    }

    // --- FILE HANDLING (CSV Write) ---
    void saveFlightsToCSV() {
        ofstream file(flightFile);
        // --- EXCEPTION HANDLING ---
        if (!file.is_open()) {
            throw runtime_error("Error: Could not open " + flightFile + " for writing.");
        }

        for (const auto& f : flights) {
            file << f.getFlightNumber() << ","
                 << f.getOrigin() << ","
                 << f.getDestination() << ","
                 << f.getAvailableSeats() << "\n";
        }
        file.close();
    }

    // Admin: Add a flight
    void addFlight(const string& fNum, const string& orig, const string& dest, int seats) {
        // --- EXCEPTION HANDLING ---
        if (seats < 0) {
            throw invalid_argument("Error: Seat count cannot be negative.");
        }
        for (const auto& f : flights) {
            if (f.getFlightNumber() == fNum) {
                throw runtime_error("Error: Flight number already exists.");
            }
        }

        flights.push_back(Flight(fNum, orig, dest, seats));
        saveFlightsToCSV();
        cout << "Flight added successfully!\n";
    }

    // Admin: Delete a flight
    void deleteFlight(const string& fNum) {
        bool found = false;
        for (size_t i = 0; i < flights.size(); i++) {
            if (flights[i].getFlightNumber() == fNum) {
                flights.erase(flights.begin() + i);
                found = true;
                break;
            }
        }

        if (!found) {
            throw runtime_error("Error: Flight number not found.");
        }

        saveFlightsToCSV();
        cout << "Flight deleted successfully!\n";
    }

    // Admin: Update flight information
    void updateFlight(const string& fNum, const string& newOrig, const string& newDest, int newSeats) {
        if (newSeats < 0) {
            throw invalid_argument("Error: Seat count cannot be negative.");
        }

        bool found = false;
        for (auto& f : flights) {
            if (f.getFlightNumber() == fNum) {
                f.setOrigin(newOrig);
                f.setDestination(newDest);
                f.setAvailableSeats(newSeats);
                found = true;
                break;
            }
        }

        if (!found) {
            throw runtime_error("Error: Flight not found to update.");
        }

        saveFlightsToCSV();
        cout << "Flight updated successfully!\n";
    }

    // Passenger: View / Search flights
    void viewAllFlights() const {
        if (flights.empty()) {
            cout << "No flights currently scheduled.\n";
            return;
        }
        cout << "\n--- Available Flights ---\n";
        for (const auto& f : flights) {
            f.display();
        }
    }

    // Passenger: Book a flight
    void bookTicket(const string& passengerName, const string& fNum) {
        bool found = false;
        for (auto& f : flights) {
            if (f.getFlightNumber() == fNum) {
                found = true;
                // Will throw runtime_error if seats == 0
                f.bookSeat();

                // Save updated flight inventory
                saveFlightsToCSV();

                // --- FILE HANDLING (Append Booking to CSV) ---
                ofstream bFile(bookingFile, ios::app);
                if (!bFile.is_open()) {
                    throw runtime_error("Error: Unable to open bookings record file.");
                }
                bFile << passengerName << "," << fNum << "\n";
                bFile.close();

                cout << "Ticket booked successfully for " << passengerName << " on flight " << fNum << "!\n";
                return;
            }
        }

        if (!found) {
            throw runtime_error("Error: Flight " + fNum + " does not exist.");
        }
    }

    // Passenger: View bookings
    void viewBookings(const string& passengerName) const {
        ifstream bFile(bookingFile);
        if (!bFile.is_open()) {
            cout << "No previous bookings found.\n";
            return;
        }

        string line;
        bool hasBookings = false;
        cout << "\n--- Bookings for " << passengerName << " ---\n";
        while (getline(bFile, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string user, fNum;
            if (getline(ss, user, ',') && getline(ss, fNum, ',')) {
                if (user == passengerName) {
                    cout << "Booked Flight: " << fNum << endl;
                    hasBookings = true;
                }
            }
        }
        if (!hasBookings) {
            cout << "No active reservations on record.\n";
        }
        bFile.close();
    }
};

// ==========================================
// 6. MAIN FUNCTION: Interactive Console Menu
// ==========================================
int main() {
    AirlineSystem system;

    int mainChoice = 0;
    while (true) {
        cout << "\n=====================================\n";
        cout << "     AIRLINE RESERVATION SYSTEM      \n";
        cout << "=====================================\n";
        cout << "1. Admin Portal\n";
        cout << "2. Passenger Portal\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";
        if (!(cin >> mainChoice)) break;

        if (mainChoice == 1) {
            Admin admin("Admin_01");
            admin.displayRole();

            int adminChoice;
            cout << "\n1. Add Flight\n2. Update Flight\n3. Delete Flight\n4. View All Flights\nEnter choice: ";
            cin >> adminChoice;

            // --- EXCEPTION HANDLING ---
            try {
                if (adminChoice == 1) {
                    string fNum, orig, dest;
                    int seats;
                    cout << "Flight Number: "; cin >> fNum;
                    cout << "Origin: "; cin >> orig;
                    cout << "Destination: "; cin >> dest;
                    cout << "Total Seats: "; cin >> seats;
                    system.addFlight(fNum, orig, dest, seats);
                } else if (adminChoice == 2) {
                    string fNum, orig, dest;
                    int seats;
                    cout << "Flight Number to Update: "; cin >> fNum;
                    cout << "New Origin: "; cin >> orig;
                    cout << "New Destination: "; cin >> dest;
                    cout << "New Seats: "; cin >> seats;
                    system.updateFlight(fNum, orig, dest, seats);
                } else if (adminChoice == 3) {
                    string fNum;
                    cout << "Flight Number to Delete: "; cin >> fNum;
                    system.deleteFlight(fNum);
                } else if (adminChoice == 4) {
                    system.viewAllFlights();
                } else {
                    cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                cout << "[Exception Caught] " << e.what() << endl;
            }

        } else if (mainChoice == 2) {
            string name;
            cout << "Enter your name: ";
            cin >> name;

            Passenger passenger(name);
            passenger.displayRole();

            int passChoice;
            cout << "\n1. View Available Flights\n2. Book a Ticket\n3. View My Bookings\nEnter choice: ";
            cin >> passChoice;

            // --- EXCEPTION HANDLING ---
            try {
                if (passChoice == 1) {
                    system.viewAllFlights();
                } else if (passChoice == 2) {
                    system.viewAllFlights();
                    string fNum;
                    cout << "Enter Flight Number to Book: ";
                    cin >> fNum;
                    system.bookTicket(name, fNum);
                } else if (passChoice == 3) {
                    system.viewBookings(name);
                } else {
                    cout << "Invalid choice.\n";
                }
            } catch (const exception& e) {
                cout << "[Exception Caught] " << e.what() << endl;
            }

        } else if (mainChoice == 3) {
            cout << "Exiting system. Best of luck in your exam!\n";
            break;
        } else {
            cout << "Invalid choice. Please enter 1, 2, or 3.\n";
        }
    }

    return 0;
}
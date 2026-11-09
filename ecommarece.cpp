#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>     // EXCEPTION HANDLING: runtime_error
using namespace std;

// ===================== PRODUCT (BASE CLASS) =====================
// CLASS + OBJECT: Product is the blueprint, each listing is an object
// ABSTRACTION: pure virtual functions, so a plain Product cannot be created
class Product {
protected:   // ENCAPSULATION: hidden from outside, child classes can use it
    int id;
    string name;
    double price;
    int stock;
public:
    // CONSTRUCTOR: uses setters so the values are validated
    Product(int i, string n, double p, int s) {
        id = i;
        name = n;
        setPrice(p);
        setStock(s);
    }
    virtual ~Product() {}    // virtual destructor (safe delete through base pointer)

    // ENCAPSULATION: getters
    int getId() { return id; }
    string getName() { return name; }
    double getPrice() { return price; }
    int getStock() { return stock; }

    // ENCAPSULATION + EXCEPTION HANDLING: setters reject bad data
    void setName(string n) {
        if (n.empty())
            throw runtime_error("Name cannot be empty");
        name = n;
    }
    void setPrice(double p) {
        if (p <= 0)
            throw runtime_error("Price must be greater than 0");
        price = p;
    }
    void setStock(int s) {
        if (s < 0)
            throw runtime_error("Stock cannot be negative");
        stock = s;
    }

    // POLYMORPHISM: each product type gives its own version
    virtual double finalPrice() = 0;
    virtual string getType() = 0;

    void display() {
        cout << "ID: " << id << " | " << getType()
             << " | " << name
             << " | Price: " << finalPrice()
             << " | Stock: " << stock << endl;
    }

    // FILE HANDLING + CSV: write this product as one CSV line
    void save(ofstream &f) {
        f << getType() << "," << id << "," << name << ","
          << price << "," << stock << endl;
    }
};

// ===================== PRODUCT CHILD CLASSES =====================
// INHERITANCE: Electronics is-a Product
class Electronics : public Product {
public:
    Electronics(int i, string n, double p, int s) : Product(i, n, p, s) {}
    string getType() { return "Electronics"; }
    // POLYMORPHISM: Electronics has 15% tax added
    double finalPrice() { return price * 1.15; }
};

// INHERITANCE: Clothing is-a Product
class Clothing : public Product {
public:
    Clothing(int i, string n, double p, int s) : Product(i, n, p, s) {}
    string getType() { return "Clothing"; }
    // POLYMORPHISM: Clothing has 10% discount
    double finalPrice() { return price * 0.90; }
};

// INHERITANCE: Grocery is-a Product
class Grocery : public Product {
public:
    Grocery(int i, string n, double p, int s) : Product(i, n, p, s) {}
    string getType() { return "Grocery"; }
    // POLYMORPHISM: Grocery price stays the same
    double finalPrice() { return price; }
};

// ===================== PAYMENT (BASE CLASS) =====================
// ABSTRACTION: Payment only says WHAT a payment must do
class Payment {
public:
    virtual ~Payment() {}
    virtual double pay(double amt) = 0;    // returns the amount actually charged
    virtual string getName() = 0;
};

// INHERITANCE: CashPayment is-a Payment
class CashPayment : public Payment {
public:
    string getName() { return "Cash"; }
    // POLYMORPHISM: cash on delivery, no extra fee
    double pay(double amt) {
        cout << "Cash on delivery. Please pay: " << amt << endl;
        return amt;
    }
};

// INHERITANCE: CardPayment is-a Payment
class CardPayment : public Payment {
public:
    string getName() { return "Card"; }
    // POLYMORPHISM: card payment asks for a card number and adds a 2% fee
    double pay(double amt) {
        string card;
        cout << "Card number (16 digits): ";
        cin >> card;
        if (card.length() != 16)
            throw runtime_error("Invalid card number");    // EXCEPTION HANDLING
        double charged = amt * 1.02;
        cout << "Paid by card (2% fee). Charged: " << charged << endl;
        return charged;
    }
};

// ===================== GLOBAL DATA =====================
// POLYMORPHISM: base class pointers can hold any product type
Product* products[100];
int total = 0;

// the cart stores product ids and quantities (two parallel arrays)
int cartIds[50];
int cartQty[50];
int cartCount = 0;

int orderNo = 0;    // last order number used

// returns index of the product with this id, or -1 if not found
int findIndex(int id) {
    for (int i = 0; i < total; i++)
        if (products[i]->getId() == id)
            return i;
    return -1;
}

// creates the right child object from the type name
Product* makeProduct(string type, int id, string name, double price, int stock) {
    if (type == "Electronics") return new Electronics(id, name, price, stock);
    if (type == "Clothing")    return new Clothing(id, name, price, stock);
    if (type == "Grocery")     return new Grocery(id, name, price, stock);
    throw runtime_error("Unknown product type");    // EXCEPTION HANDLING
}

// ===================== FILE HANDLING (CSV) =====================
void saveAll() {
    ofstream f("products.csv");     // ofstream = output file stream
    for (int i = 0; i < total; i++)
        products[i]->save(f);
}

void loadAll() {
    ifstream f("products.csv");     // ifstream = input file stream
    if (!f) return;                 // first run: file does not exist, skip

    string type, idStr, name, priceStr, stockStr;
    // getline(f, x, ',') reads until the next comma (CSV reading)
    while (getline(f, type, ',')) {
        getline(f, idStr, ',');
        getline(f, name, ',');
        getline(f, priceStr, ',');
        getline(f, stockStr);       // last column: read until end of line
        if (total < 100)
            products[total++] = makeProduct(type, stoi(idStr), name,
                                            stod(priceStr), stoi(stockStr));
    }
}

// count old orders so new order numbers continue from the last one
void loadOrderCount() {
    ifstream f("orders.csv");
    string line;
    while (getline(f, line))
        if (!line.empty())
            orderNo++;
}

// ===================== ADMIN FUNCTIONS =====================
void addProduct() {
    int id, choice, stock;
    double price;
    string name;

    cout << "Product id: ";
    cin >> id;
    if (total >= 100)
        throw runtime_error("Product list is full");
    if (findIndex(id) != -1)
        throw runtime_error("A product with this id already exists");

    cout << "1. Electronics  2. Clothing  3. Grocery: ";
    cin >> choice;
    cout << "Price: ";
    cin >> price;
    cout << "Stock: ";
    cin >> stock;
    cin.ignore();                   // clear the leftover newline before getline
    cout << "Name (no commas): ";
    getline(cin, name);

    string type;
    if (choice == 1) type = "Electronics";
    else if (choice == 2) type = "Clothing";
    else if (choice == 3) type = "Grocery";
    else throw runtime_error("Invalid product type");

    // POLYMORPHISM: child object stored in a base class pointer
    products[total++] = makeProduct(type, id, name, price, stock);
    cout << "Product added\n";
}

void updateProduct() {
    int id, ch;
    cout << "Product id to update: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Product not found");

    cout << "1. Update price\n2. Update stock\nChoice: ";
    cin >> ch;
    if (ch == 1) {
        double p;
        cout << "New price: ";
        cin >> p;
        products[idx]->setPrice(p);     // setter checks the value
    } else if (ch == 2) {
        int s;
        cout << "New stock: ";
        cin >> s;
        products[idx]->setStock(s);
    }
    cout << "Product updated\n";
}

void deleteProduct() {
    int id;
    cout << "Product id to delete: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Product not found");

    delete products[idx];                   // free the memory
    for (int i = idx; i < total - 1; i++)
        products[i] = products[i + 1];      // shift the rest one step left
    total--;
    cout << "Product deleted\n";
}

void adminMenu() {
    int pass;
    cout << "Admin password: ";
    cin >> pass;
    if (pass != 1234)
        throw runtime_error("Wrong admin password");

    int ch;
    cout << "1. Add product\n2. Update product\n3. Delete product\nChoice: ";
    cin >> ch;
    if (ch == 1) addProduct();
    else if (ch == 2) updateProduct();
    else if (ch == 3) deleteProduct();
}

// ===================== CUSTOMER FUNCTIONS =====================
void browseProducts() {
    if (total == 0)
        throw runtime_error("No products available");
    for (int i = 0; i < total; i++)
        products[i]->display();
}

void addToCart() {
    int id, qty;
    cout << "Product id: ";
    cin >> id;
    int idx = findIndex(id);
    if (idx == -1)
        throw runtime_error("Product not found");

    cout << "Quantity: ";
    cin >> qty;
    if (qty <= 0)
        throw runtime_error("Quantity must be at least 1");

    // if the product is already in the cart, just increase its quantity
    for (int i = 0; i < cartCount; i++)
        if (cartIds[i] == id) {
            if (cartQty[i] + qty > products[idx]->getStock())
                throw runtime_error("Not enough stock");
            cartQty[i] += qty;
            cout << "Cart updated\n";
            return;
        }

    if (qty > products[idx]->getStock())
        throw runtime_error("Not enough stock");
    if (cartCount >= 50)
        throw runtime_error("Cart is full");
    cartIds[cartCount] = id;
    cartQty[cartCount] = qty;
    cartCount++;
    cout << "Added to cart\n";
}

// total price of the cart (uses finalPrice, so POLYMORPHISM again)
double cartTotal() {
    double sum = 0;
    for (int i = 0; i < cartCount; i++) {
        int idx = findIndex(cartIds[i]);
        if (idx == -1)
            throw runtime_error("A product in your cart is no longer available");
        sum += products[idx]->finalPrice() * cartQty[i];
    }
    return sum;
}

void viewCart() {
    if (cartCount == 0)
        throw runtime_error("Cart is empty");
    double sum = cartTotal();       // checks that every product still exists
    for (int i = 0; i < cartCount; i++) {
        int idx = findIndex(cartIds[i]);
        cout << products[idx]->getName() << " x " << cartQty[i]
             << " = " << products[idx]->finalPrice() * cartQty[i] << endl;
    }
    cout << "Cart total: " << sum << endl;
}

void placeOrder() {
    if (cartCount == 0)
        throw runtime_error("Cart is empty");

    // 1) check stock first, so nothing is changed if something is wrong
    for (int i = 0; i < cartCount; i++) {
        int idx = findIndex(cartIds[i]);
        if (idx == -1)
            throw runtime_error("A product in your cart is no longer available");
        if (cartQty[i] > products[idx]->getStock())
            throw runtime_error("Not enough stock for " + products[idx]->getName());
    }

    double sum = cartTotal();
    cout << "Order total: " << sum << endl;

    // 2) choose a payment method
    int ch;
    cout << "1. Cash  2. Card\nChoice: ";
    cin >> ch;

    CashPayment cash;
    CardPayment card;
    Payment* p;                     // POLYMORPHISM: base pointer holds either one
    if (ch == 1) p = &cash;
    else if (ch == 2) p = &card;
    else throw runtime_error("Invalid payment method");

    double paid = p->pay(sum);      // POLYMORPHISM: Cash or Card version runs

    // 3) payment done: reduce stock
    for (int i = 0; i < cartCount; i++) {
        int idx = findIndex(cartIds[i]);
        products[idx]->setStock(products[idx]->getStock() - cartQty[i]);
    }

    // 4) FILE HANDLING + CSV: add one line "orderNo,method,amount" to orders.csv
    orderNo++;
    ofstream f("orders.csv", ios::app);     // ios::app = add at the end
    f << orderNo << "," << p->getName() << "," << paid << endl;

    cartCount = 0;                  // empty the cart
    cout << "Order placed! Order no: " << orderNo << endl;
}

void userMenu() {
    int ch;
    cout << "1. Browse products\n2. Add to cart\n3. View cart\n"
         << "4. Clear cart\n5. Place order\nChoice: ";
    cin >> ch;
    if (ch == 1) browseProducts();
    else if (ch == 2) addToCart();
    else if (ch == 3) viewCart();
    else if (ch == 4) { cartCount = 0; cout << "Cart cleared\n"; }
    else if (ch == 5) placeOrder();
}

// ===================== MAIN =====================
int main() {
    // EXCEPTION HANDLING: loading can throw if the CSV has bad data
    try {
        loadAll();
    }
    catch (exception &e) {
        cout << "Error while loading file: " << e.what() << endl;
    }
    loadOrderCount();

    int role;
    do {
        cout << "\n--- E-COMMERCE MENU ---\n1. Admin\n2. Customer\n0. Exit\nChoice: ";
        cin >> role;

        // EXCEPTION HANDLING: any throw inside try jumps to catch
        try {
            if (role == 1) adminMenu();
            else if (role == 2) userMenu();
        }
        catch (exception &e) {
            cout << "Error: " << e.what() << endl;
        }

        saveAll();      // FILE HANDLING: save after every action (stock changes too)
    } while (role != 0);

    // free memory of all dynamically created objects
    for (int i = 0; i < total; i++)
        delete products[i];
    return 0;
}
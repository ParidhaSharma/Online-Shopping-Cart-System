#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <stdexcept>
using namespace std;

// ----------------- Product (Abstract Base) -----------------
class Product {
protected:
    int id;
    string name;
    double basePrice;
    int stock;
public:
    Product(int id, const string &name, double price, int stock)
        : id(id), name(name), basePrice(price), stock(stock) {}
    virtual ~Product() {}
    int getId() const { return id; }
    string getName() const { return name; }
    int getStock() const { return stock; }
    void reduceStock(int q) {
        if (q > stock) throw runtime_error("Not enough stock");
        stock -= q;
    }
    void increaseStock(int q) { stock += q; }
    virtual double getPrice(int quantity) const {
        return basePrice * quantity;
    }
    virtual string getType() const = 0;
    virtual void display() const {
        cout << left << setw(4) << id << setw(20) << name
             << setw(10) << fixed << setprecision(2) << basePrice
             << setw(8) << stock << setw(12) << getType() << '\n';
    }
};

// ----------------- PhysicalProduct (derived) -----------------
class PhysicalProduct : public Product {
    double shippingPerItem;
public:
    PhysicalProduct(int id, const string &name, double price, int stock, double shipPer)
        : Product(id, name, price, stock), shippingPerItem(shipPer) {}
    virtual double getPrice(int quantity) const override {
        return (basePrice * quantity) + (shippingPerItem * quantity);
    }
    virtual string getType() const override { return "Physical"; }
};

// ----------------- DigitalProduct (derived) -----------------
class DigitalProduct : public Product {
public:
    DigitalProduct(int id, const string &name, double price, int stock = 9999)
        : Product(id, name, price, stock) {}
    virtual double getPrice(int quantity) const override {
        double total = basePrice * quantity;
        if (quantity >= 5) total *= 0.9; // 10% off for bulk digital items
        return total;
    }
    virtual string getType() const override { return "Digital"; }
};

// ----------------- Cart -----------------
class Cart {
    map<int,int> items;
    map<int, shared_ptr<Product>> &catalog;
public:
    Cart(map<int, shared_ptr<Product>> &cat) : catalog(cat) {}
    Cart& operator+=(pair<int,int> pid_qty) {
        int pid = pid_qty.first;
        int qty = pid_qty.second;
        if (catalog.find(pid) == catalog.end()) {
            throw invalid_argument("Product ID not found.");
        }
        if (qty <= 0) throw invalid_argument("Quantity must be positive.");
        if (catalog[pid]->getStock() < qty) throw runtime_error("Insufficient stock.");
        items[pid] += qty;
        return *this;
    }
    void removeItem(int pid) {
        if (items.erase(pid))
            cout << "Item removed.\n";
        else
            cout << "Item not found in cart.\n";
    }
    bool empty() const { return items.empty(); }
    double getTotal() const {
        double total = 0.0;
        for (auto &p : items) {
            int pid = p.first, qty = p.second;
            total += catalog.at(pid)->getPrice(qty);
        }
        return total;
    }
    void displayCart() const {
        if (items.empty()) { cout << "Cart is empty.\n"; return; }
        cout << left << setw(4) << "ID" << setw(20) << "Name"
             << setw(8) << "Qty" << setw(12) << "UnitPrice" << setw(12) << "Subtotal\n";
        cout << string(60,'-') << '\n';
        for (auto &p : items) {
            int pid = p.first, qty = p.second;
            const auto &prod = catalog.at(pid);
            double unit = prod->getPrice(1);
            double sub = prod->getPrice(qty);
            cout << left << setw(4) << pid << setw(20) << prod->getName()
                 << setw(8) << qty << setw(12) << fixed << setprecision(2) << unit
                 << setw(12) << sub << '\n';
        }
        cout << string(60,'-') << '\n';
        cout << "Total: " << fixed << setprecision(2) << getTotal() << '\n';
    }
    void checkout(const string &customerName) {
        if (items.empty()) throw runtime_error("Cart is empty.");
        for (auto &p : items) {
            int pid = p.first, qty = p.second;
            if (catalog.at(pid)->getStock() < qty)
                throw runtime_error("Stock changed. Item unavailable.");
        }
        for (auto &p : items)
            catalog.at(p.first)->reduceStock(p.second);
        double total = getTotal();
        ofstream ofs("receipt.txt");
        ofs << "Receipt for " << customerName << '\n';
        ofs << left << setw(4) << "ID" << setw(20) << "Name"
            << setw(8) << "Qty" << setw(12) << "UnitPrice" << setw(12) << "Subtotal\n";
        ofs << string(60,'-') << '\n';
        for (auto &p : items) {
            int pid = p.first, qty = p.second;
            const auto &prod = catalog.at(pid);
            double unit = prod->getPrice(1);
            double sub = prod->getPrice(qty);
            ofs << left << setw(4) << pid << setw(20) << prod->getName()
                << setw(8) << qty << setw(12) << fixed << setprecision(2) << unit
                << setw(12) << sub << '\n';
        }
        ofs << string(60,'-') << '\n';
        ofs << "Total: " << fixed << setprecision(2) << total << '\n';
        ofs.close();
        cout << "Checkout complete! Receipt saved to 'receipt.txt'.\n";
        items.clear();
    }
};

// ----------------- Helpers -----------------
void showCatalog(const map<int, shared_ptr<Product>> &catalog) {
    cout << left << setw(4) << "ID" << setw(20) << "Name"
         << setw(10) << "Price" << setw(8) << "Stock" << setw(12) << "Type\n";
    cout << string(60,'-') << '\n';
    for (auto &p : catalog) p.second->display();
}
map<int, shared_ptr<Product>> loadSampleCatalog() {
    map<int, shared_ptr<Product>> catalog;
    catalog[101] = make_shared<PhysicalProduct>(101, "Wireless Mouse", 599.00, 20, 30.0);
    catalog[102] = make_shared<PhysicalProduct>(102, "Mechanical Keyboard", 2499.00, 10, 50.0);
    catalog[103] = make_shared<DigitalProduct>(103, "Antivirus License", 799.00, 9999);
    catalog[104] = make_shared<DigitalProduct>(104, "E-book: Learn C++", 199.00, 9999);
    catalog[105] = make_shared<PhysicalProduct>(105, "USB-C Charger", 999.00, 15, 25.0);
    return catalog;
}

// ----------------- MAIN -----------------
int main() {
    cout << "=== Simple Online Shopping Cart ===\n";
    auto catalog = loadSampleCatalog();
    Cart cart(catalog);

    while (true) {
        cout << "\nMenu:\n"
             << "1. Show catalog\n"
             << "2. Add to cart\n"
             << "3. Remove from cart\n"
             << "4. View cart\n"
             << "5. Checkout\n"
             << "6. Exit\n"
             << "Choose (1-6): ";
        int ch;
        if (!(cin >> ch)) { cin.clear(); cin.ignore(10000,'\n'); continue; }

        try {
            if (ch == 1) showCatalog(catalog);
            else if (ch == 2) {
                cout << "Enter Product ID: "; int pid; cin >> pid;
                cout << "Enter quantity: "; int qty; cin >> qty;
                cart += make_pair(pid, qty);
                cout << "Added to cart.\n";
            }
            else if (ch == 3) {
                cout << "Enter Product ID to remove: "; int pid; cin >> pid;
                cart.removeItem(pid);
            }
            else if (ch == 4) cart.displayCart();
            else if (ch == 5) {
                if (cart.empty()) { cout << "Cart empty.\n"; continue; }
                cout << "Enter your name for receipt: ";
                string name; cin.ignore(); getline(cin, name);
                cart.checkout(name);
            }
            else if (ch == 6) { cout << "Goodbye!\n"; break; }
            else cout << "Invalid choice.\n";
        } catch (const exception &ex) {
            cout << "Error: " << ex.what() << '\n';
        }
    }
    return 0;
}

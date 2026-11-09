#include <iostream>
#include <fstream>
#include <string>
using namespace std;
class Person {                       // parent / base class
protected:                           // child can access protected
    string name;
public:
    Person(string n) {
        name = n;
        cout << "Person constructor\n";
    }
    ~Person() { cout << "Person destructor\n"; }
    void showName() { cout << "Name: " << name << endl; }
};

class Student : public Person {      // child / derived class
    int roll;
public:
    Student(string n, int r) : Person(n) {   // passes name up to the parent
        roll = r;
        cout << "Student constructor\n";
    }
    ~Student() { cout << "Student destructor\n"; }
    void show() {
        showName();                  // inherited function
        cout << "Roll: " << roll << endl;
    }
};

int main() {
    Student s("Sobuj", 12);
    s.show();
}
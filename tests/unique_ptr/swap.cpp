#include "../../unique_ptr.hpp"

#include <iostream>
 
struct Foo {
    Foo(int _val) : val(_val) { std::cout << "Foo...\n"; }
    ~Foo() { std::cout << "~Foo...\n"; }
    std::string print() { return std::to_string(val); }
    int val;
};

void test_member() {
    gkxx::unique_ptr<Foo> up1(new Foo(1));
    gkxx::unique_ptr<Foo> up2(new Foo(2));
 
    up1.swap(up2);
 
    std::cout << "up1->val:" << up1->val << '\n';
    std::cout << "up2->val:" << up2->val << '\n';
}

void test_nonmember() {
    gkxx::unique_ptr<Foo> p1 = gkxx::make_unique<Foo>(100);
    gkxx::unique_ptr<Foo> p2 = gkxx::make_unique<Foo>(200);
    auto print = [&]() {
        std::cout << " p1=" << (p1 ? p1->print() : "nullptr");
        std::cout << " p2=" << (p2 ? p2->print() : "nullptr") << '\n';  
    };
    print();
 
    std::swap(p1, p2);
    print();
 
    p1.reset();
    print();
 
    std::swap(p1, p2);
    print();
}

int main() {
    test_member();
    test_nonmember();
    return 0;
}
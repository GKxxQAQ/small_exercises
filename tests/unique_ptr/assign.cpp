#include "../../unique_ptr.hpp"

#include <iostream>
 
struct Foo
{
    int id;
    Foo(int id) : id(id) { std::cout << "Foo " << id << '\n'; }
    ~Foo() { std::cout << "~Foo " << id << '\n'; }
};
 
int main() 
{
    gkxx::unique_ptr<Foo> p1(gkxx::make_unique<Foo>(1));
 
    {
        std::cout << "Creating new Foo...\n";
        gkxx::unique_ptr<Foo> p2(gkxx::make_unique<Foo>(2));
        // p1 = p2; // Error ! can't copy unique_ptr
        p1 = std::move(p2);
        std::cout << "About to leave inner block...\n";
 
        // Foo instance will continue to live, 
        // despite p2 going out of scope
    }
 
    std::cout << "About to leave program...\n";
}
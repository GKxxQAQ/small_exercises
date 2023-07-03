#include "../../unique_ptr.hpp"

#include <functional>
#include <iostream>
 
struct Foo
{
    Foo(int num) : nr(num) { std::cout << "Foo(" << nr << ")\n"; }
 
    ~Foo() { std::cout << "~Foo()\n"; }
 
    bool operator==(const Foo &other) const { return nr == other.nr; };
 
    int nr;
};
 
int main()
{
    std::cout << std::boolalpha << std::hex;
 
    Foo* foo = new Foo(5);
    gkxx::unique_ptr<Foo> up(foo); 
    std::cout << "hash(up):    " << std::hash<gkxx::unique_ptr<Foo>>()(up) << '\n'
              << "hash(foo):   " << std::hash<Foo*>()(foo) << '\n'
              << "*up==*foo:   " << (*up == *foo) << "\n\n";
 
    gkxx::unique_ptr<Foo> other = gkxx::make_unique<Foo>(5);
    std::cout << "hash(up):    " << std::hash<gkxx::unique_ptr<Foo>>()(up) << '\n'
              << "hash(other): " << std::hash<gkxx::unique_ptr<Foo>>()(other) << '\n'
              << "*up==*other: " <<(*up == *other) << "\n\n";
}
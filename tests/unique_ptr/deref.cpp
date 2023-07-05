#include "../../memory/unique_ptr.hpp"

#include <iostream>
 
struct Foo {
    void bar() { std::cout << "Foo::bar\n"; }
};
 
void f(const Foo&) 
{
    std::cout << "f(const Foo&)\n";
}
 
int main() 
{
    gkxx::unique_ptr<Foo> ptr(new Foo);
 
    ptr->bar();
    f(*ptr);
}
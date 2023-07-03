#include "../../unique_ptr.hpp"

#include <iostream>
 
class Foo {};
 
int main()
{
    auto p = gkxx::make_unique<Foo>();
    std::cout << p << '\n';
    std::cout << p.get() << '\n';
}
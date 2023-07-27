#include "../../function.hpp"

#include <iostream>
 
void foo(const char* str, int x) {
    std::cout << "foo(\"" << str << "\", " << x << ")\n";
}
 
void bar(const char* str, int x) {
    std::cout << "bar(\"" << str << "\", " << x << ")\n";
}
 
int main()
{
    gkxx::function<void(const char*, int)> f1{ foo };
    gkxx::function<void(const char*, int)> f2{ bar };
 
    f1("f1", 1);
    f2("f2", 2);
 
    std::cout << "std::swap(f1, f2);\n";
    std::swap(f1, f2);
 
    f1("f1", 1);
    f2("f2", 2);
}
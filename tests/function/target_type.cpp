#include "../../function.hpp"

#include <iostream>
 
int f(int a) { return -a; }
void g(double) {}
int main()
{
    // fn1 and fn2 have the same type, but their targets do not
    gkxx::function<int(int)> fn1(f),
                            fn2([](int a) {return -a;});
    std::cout << fn1.target_type().name() << '\n'
              << fn2.target_type().name() << '\n';
 
    // since C++17 deduction guides (CTAD) can avail
    std::cout << gkxx::function{g}.target_type().name() << '\n';
}
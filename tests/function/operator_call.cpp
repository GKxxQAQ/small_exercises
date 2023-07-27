#include "../../function.hpp"

#include <iostream>
 
void call(gkxx::function<int()> f) // can be passed by value
{ 
    std::cout << f() << '\n';
}
 
int normal_function()
{
    return 42;
}
 
int main()
{
    int n = 1;
    gkxx::function<int()> f = [&n](){ return n; };
    call(f);
 
    n = 2;
    call(f);
 
    f = normal_function;
    call(f);
}
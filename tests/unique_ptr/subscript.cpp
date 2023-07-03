#include "../../unique_ptr.hpp"

#include <iostream>
 
int main() 
{
    const int size = 10; 
    gkxx::unique_ptr<int[]> fact(new int[size]);
 
    for (int i = 0; i < size; ++i)
        fact[i] = (i == 0) ? 1 : i * fact[i - 1];
 
    for (int i = 0; i < size; ++i)
        std::cout << i << "! = " << fact[i] << '\n';
}
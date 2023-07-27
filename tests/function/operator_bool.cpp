#include "../../function.hpp"

#include <iostream>
 
void sampleFunction()
{
    std::cout << "This is the sample function!\n";
}
 
void checkFunc( gkxx::function<void()> const &func )
{
    // Use operator bool to determine if callable target is available.
    if( func )  
    {
        std::cout << "Function is not empty! Calling function.\n";
        func();
    }
    else
    {
        std::cout << "Function is empty. Nothing to do.\n";
    }
}
 
int main()
{
    gkxx::function<void()> f1;
    gkxx::function<void()> f2( sampleFunction );
 
    std::cout << "f1: ";
    checkFunc( f1 );
 
    std::cout << "f2: ";
    checkFunc( f2 );
}
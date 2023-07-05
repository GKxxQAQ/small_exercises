#include "../../memory/unique_ptr.hpp"

#include <iostream>
 
int main () 
{
    auto deleter = [](int* ptr){
        std::cout << "[deleter called]\n";
        delete ptr;
    };
 
    gkxx::unique_ptr<int,decltype(deleter)> uniq(new int, deleter);
    std::cout << (uniq ? "not empty\n" : "empty\n");
    uniq.reset();
    std::cout << (uniq ? "not empty\n" : "empty\n");
}
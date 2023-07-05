#include "../../memory/unique_ptr.hpp"

#include <cassert>
#include <iostream>
 
struct Foo
{
    Foo() { std::cout << "Foo\n"; }
    ~Foo() { std::cout << "~Foo\n"; }
};
 
// Ownership of the Foo resource is transferred when calling this function
void legacy_api(Foo* owning_foo)
{
    std::cout << __func__ << '\n';
    // [legacy code that no one understands or dares touch anymore]
    // [...]
    delete owning_foo;
}
 
int main()
{
    gkxx::unique_ptr<Foo> managed_foo(new Foo);
    // [code that might return or throw or some such]
    // [...]
    legacy_api(managed_foo.release());
 
    assert(managed_foo == nullptr);
}
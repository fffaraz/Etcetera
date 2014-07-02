#include <iostream>
#include <string>
using namespace std;
 
void operator"" _print(const char* str, unsigned int len)
{
    std::cout << str;
}
 
int main()
{
    "hello"_print;
 
    cin.get();
    return 0;
}

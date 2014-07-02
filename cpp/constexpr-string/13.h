#include <iostream>
#include <string>
using namespace std;
 
string operator"" _enc(const char* str, size_t len)
{
    string ans(str, len);
    for(size_t i=0; i<ans.length(); ++i)
        ans[i] = ans[i] ^ 0x55;
    return ans;
}
 
// runtime decrypter
string dec(string str)
{
    for(size_t i=0; i<str.length(); ++i)
        str[i] = str[i] ^ 0x55;
    return str;
}
 
int main()
{
    cout << dec("hello"_enc);
 
    cin.get();
    return 0;
}

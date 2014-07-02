#include <iostream>
using namespace std;


template<typename T, T... args> 
auto operator"" _my() -> const char (&)[sizeof...(args) + 1] 
{ 
	static const char data[] = { (args ^ 0x55)..., 0 };
	return data;
}

int main() 
{
	string s("my_string"_my); 
	for_each(begin(s), end(s), [](auto &c){ c ^= 0x55; });
	cout <<s;
	return 0;
}

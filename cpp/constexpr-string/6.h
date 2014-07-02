#include <iostream>
#include <string>
using namespace std;

#include <boost/preprocessor/repetition/repeat.hpp>


template<char T>
inline string _TC()
{
    string str;
    str += (char)(T ^ 0x55);
    return str;
}

template<char T0, char T1, char... T>
inline string _TC()
{
    string str;
    if(T0 != 0)
    {
        str += _TC<T0>();
        str += _TC<T1, T...>();
    }
    return str;
}

#define _TT(...) _TR(_TC<__VA_ARGS__>())

#define AT1(L,I) (I < sizeof(L)) ? L[I] : 0

#define DECL(z, n, L) AT2(L,n),
#define _TS(L) _TT(BOOST_PP_REPEAT(64, DECL, L) '\0') //sizeof(L)


string _TR(string str)
{
    for (size_t i = 0; i < str.size(); ++i)
        str[i] = str[i] ^ 0x55;
    return str;
}

int main()
{
    cout << _TT('s', 'a', 'l', 'a', 'm') << '.' << endl;
    cout << _TS("salam") << '.' << endl;
    return 0;
}

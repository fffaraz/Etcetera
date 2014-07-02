
#include "stdafx.h"

#include <array>
#include <iostream>
#include <string>

using namespace std;

#include <boost/preprocessor/repetition/repeat.hpp>


template<char C, size_t I>
struct Pair
{
	static const char first = C;
	static const size_t second = I;
};

template<typename T>
struct EncryptByte
{
	static const char value = T::first ^ 0x55;
};

template<typename...T>
struct EncryptHelper
{
	static const char value[sizeof...(T)];
};

template<typename...T>
const char EncryptHelper<T...>::value[sizeof...(T)] = {
	EncryptByte<T>::value...
};

#define AT(L,I) (I < sizeof(L)) ? char(L[I]) : char('\0')
#define DECL(z, n, L) Pair<AT(L,n),n>,
#define ENCSTR(L) EncryptHelper<BOOST_PP_REPEAT(sizeof(L), DECL, L)>::value
#define SAFESTR(L) Decrypt(ENCSTR(L))

string Decrypt(string str)
{
	for (unsigned int i = 0; i < str.length(); ++i)
		str[i] = str[i] ^ 0x55;
	return str;
}

int main()
{
	cout << SAFESTR("AES-encrypted string") << endl;
}

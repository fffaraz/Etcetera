// Binary code obfuscation through C++ template metaprogramming

template<char C, size_t I>
struct Pair {
static const char first = C;
static const size_t second = I;
};

template<template<typename> class BlockCipher, typename Key, typename T>
struct EncryptByte {
static const u32 L = BlockCipher<Key>::block_length;
typedef typename BlockCipher<Key>::template EncryptCtr<T::second / L> Block;
static const char value = T::first ^ Block::template Byte<T::second % L>::value;
};

template<template<typename> class BlockCipher, typename Key, typename...T>
struct EncryptHelper {
static const char value[sizeof...(T)];
};

template<template<typename> class BlockCipher, typename Key, typename...T>
const char EncryptHelper<BlockCipher, Key, T...>::value[sizeof...(T)] = {
EncryptByte<BlockCipher, Key, T>::value...
};

# define THRESHOLD 256
# define AT(L,I) (I < sizeof(L)) ? char(L[I]) : char(’\0’)
# define DECL(z, n, L) Pair<AT(L,n),n>,
# define ENCSTR(K0,K1,K2,K3,L) EncryptHelper<AES128, AESKey<K0,K1,K2,K3>, \
BOOST_PP_REPEAT(THRESHOLD, DECL, L) DECL(0, THRESHOLD, L)>::value
# define SAFERSTR(L,K0,K1,K2,K3) DecryptCtr<AES128, AESKey<K0,K1,K2,K3>>\
(ENCSTR(K0,K1,K2,K3,L),THRESHOLD)
# define SAFESTR(L) SAFERSTR(L,__RAND__,__LINE__,__COUNTER__,0)

// std::cout << SAFESTR("AES-encrypted string") << std::endl;

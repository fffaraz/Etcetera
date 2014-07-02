#include <array>
#include <iostream>
#include <string>
 
template<std::size_t N>
constexpr std::array<char, N - 1> encrypt(char const (&s)[N], char secret = 0x55) {
	std::array<char, N - 1> result{};
	for (unsigned int i = 0; i < N; ++i) result[i] = s[i] ^ secret;
	return result;
}
 
template<std::size_t N>
std::string decrypt(std::array<char, N> s, char secret = 0x55) {
	std::string result{};
	result.reserve(N);
	for (unsigned int i = 0; i < N; ++i) result.push_back(s[i] ^ secret);
	return result;
}
 
#define S(x) decrypt(encrypt(x))
 
int main() {
	std::cout << S("Hello World!") << std::endl;
}
 
 
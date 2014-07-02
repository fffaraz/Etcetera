//schurr_cpp11_tools_for_class_authors.pdf

class str_const { // constexpr string
private:
const char* const p_;
const std::size_t sz_;
public:
template<std::size_t N>
constexpr str_const(const char(&a)[N]) : // ctor
p_(a), sz_(N-1) {}
constexpr char operator[](std::size_t n) { // []
return n < sz_ ? p_[n] :
throw std::out_of_range("");
}
constexpr std::size_t size() { return sz_; } // size()
};

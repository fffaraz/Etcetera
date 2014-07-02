#include <iostream>

// helper function
constexpr unsigned c_strlen( char const* str, unsigned count = 0 )
{
    return ('\0' == str[0]) ? count : c_strlen(str+1, count+1);
}

// helper "function" struct
template < char t_c, char... tt_c >
struct rec_print
{
    static void print()
    {
        std::cout << t_c;
        rec_print < tt_c... > :: print ();
    }
};
    template < char t_c >
    struct rec_print < t_c >
    {
        static void print() { std::cout << t_c; }
    };


// destination "template string" type
template < char... tt_c >
struct exploded_string
{
    static void print()
    {
        rec_print < tt_c... > :: print();
    }
};

// struct to explode a `char const*` to an `exploded_string` type
template < typename T_StrProvider, unsigned t_len, char... tt_c >
struct explode_impl
{
    using result =
        typename explode_impl < T_StrProvider, t_len-1,
                                T_StrProvider::str()[t_len-1],
                                tt_c... > :: result;
};

    template < typename T_StrProvider, char... tt_c >
    struct explode_impl < T_StrProvider, 0, tt_c... >
    {
         using result = exploded_string < tt_c... >;
    };

// syntactical sugar
template < typename T_StrProvider >
using explode =
    typename explode_impl < T_StrProvider,
                            c_strlen(T_StrProvider::str()) > :: result;


int main()
{
    // the trick is to introduce a type which provides the string, rather than
    // storing the string itself
    struct my_str_provider
    {
        constexpr static char const* str() { return "hello world"; }
    };

    auto my_str = explode < my_str_provider >{};    // as a variable
    using My_Str = explode < my_str_provider >;    // as a type

    str.print();
}

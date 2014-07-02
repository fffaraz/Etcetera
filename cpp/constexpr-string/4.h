//Arrange strings contiguously in memory at compile-time from string literals.
//All free functions prefixed with "my" to faciliate grepping the symbol tree
//(none of them should show up).

#include <iostream>

using std::size_t;

//wrapper for const char* to "allocate" space for it at compile-time
template<size_t N>
struct String {
    //C arrays can only be initialised with a comma-delimited list
    //of values in curly braces. Good thing the compiler expands
    //parameter packs into comma-delimited lists. Now we just have
    //to get a parameter pack of char into the constructor.
    template<typename... Args>
    constexpr String(Args... args):_str{ args... } { }
    const char _str[N];
};

//takes variadic number of chars, creates String object from it.
//i.e. myMakeStringFromChars('f', 'o', 'o', '\0') -> String<4>::_str = "foo"
template<typename... Args>
constexpr auto myMakeStringFromChars(Args... args) -> String<sizeof...(Args)> {
    return String<sizeof...(args)>(args...);
}

//This struct is here just because the iteration is going up instead of
//down. The solution was to mix traditional template metaprogramming
//with constexpr to be able to terminate the recursion since the template
//parameter N is needed in order to return the right-sized String<N>.
//This class exists only to dispatch on the recursion being finished or not.
//The default below continues recursion.
template<bool TERMINATE>
struct RecurseOrStop {
    template<size_t N, size_t I, typename... Args>
    static constexpr String<N> recurseOrStop(const char* str, Args... args);
};

//Specialisation to terminate recursion when all characters have been
//stripped from the string and converted to a variadic template parameter pack.
template<>
struct RecurseOrStop<true> {
    template<size_t N, size_t I, typename... Args>
    static constexpr String<N> recurseOrStop(const char* str, Args... args);
};

//Actual function to recurse over the string and turn it into a variadic
//parameter list of characters.
//Named differently to avoid infinite recursion.
template<size_t N, size_t I = 0, typename... Args>
constexpr String<N> myRecurseOrStop(const char* str, Args... args) {
    //template needed after :: since the compiler needs to distinguish
    //between recurseOrStop being a function template with 2 paramaters
    //or an enum being compared to N (recurseOrStop < N)
    return RecurseOrStop<I == N>::template recurseOrStop<N, I>(str, args...);
}

//implementation of the declaration above
//add a character to the end of the parameter pack and recurse to next character.
template<bool TERMINATE>
template<size_t N, size_t I, typename... Args>
constexpr String<N> RecurseOrStop<TERMINATE>::recurseOrStop(const char* str,
                                                            Args... args) {
    return myRecurseOrStop<N, I + 1>(str, args..., str[I]);
}

//implementation of the declaration above
//terminate recursion and construct string from full list of characters.
template<size_t N, size_t I, typename... Args>
constexpr String<N> RecurseOrStop<true>::recurseOrStop(const char* str,
                                                       Args... args) {
    return myMakeStringFromChars(args...);
}

//takes a compile-time static string literal and returns String<N> from it
//this happens by transforming the string literal into a variadic paramater
//pack of char.
//i.e. myMakeString("foo") -> calls myMakeStringFromChars('f', 'o', 'o', '\0');
template<size_t N>
constexpr String<N> myMakeString(const char (&str)[N]) {
    return myRecurseOrStop<N>(str);
}

//Simple tuple implementation. The only reason std::tuple isn't being used
//is because its only constexpr constructor is the default constructor.
//We need a constexpr constructor to be able to do compile-time shenanigans,
//and it's easier to roll our own tuple than to edit the standard library code.

//use MyTupleLeaf to construct MyTuple and make sure the order in memory
//is the same as the order of the variadic parameter pack passed to MyTuple.
template<typename T>
struct MyTupleLeaf {
    constexpr MyTupleLeaf(T value):_value(value) { }
    T _value;
};

//Use MyTupleLeaf implementation to define MyTuple.
//Won't work if used with 2 String<> objects of the same size but this
//is just a toy implementation anyway. Multiple inheritance guarantees
//data in the same order in memory as the variadic parameters.
template<typename... Args>
struct MyTuple: public MyTupleLeaf<Args>... {
    constexpr MyTuple(Args... args):MyTupleLeaf<Args>(args)... { }
};

//Helper function akin to std::make_tuple. Needed since functions can deduce
//types from parameter values, but classes can't.
template<typename... Args>
constexpr MyTuple<Args...> myMakeTuple(Args... args) {
    return MyTuple<Args...>(args...);
}

//Takes a variadic list of string literals and returns a tuple of String<> objects.
//These will be contiguous in memory. Trailing '\0' adds 1 to the size of each string.
//i.e. ("foo", "foobar") -> (const char (&arg1)[4], const char (&arg2)[7]) params ->
//                       ->  MyTuple<String<4>, String<7>> return value
template<size_t... Sizes>
constexpr auto myMakeStrings(const char (&...args)[Sizes]) -> MyTuple<String<Sizes>...> {
    //expands into myMakeTuple(myMakeString(arg1), myMakeString(arg2), ...)
    return myMakeTuple(myMakeString(args)...);
}

//Prints tuple of strings
template<typename T> //just to avoid typing the tuple type of the strings param
void printStrings(const T& strings) {
    //No std::get or any other helpers for MyTuple, so intead just cast it to
    //const char* to explore its layout in memory. We could add iterators to
    //myTuple and do "for(auto data: strings)" for ease of use, but the whole
    //point of this exercise is the memory layout and nothing makes that clearer
    //than the ugly cast below.
    const char* const chars = reinterpret_cast<const char*>(&strings);
    std::cout << "Printing strings of total size " << sizeof(strings);
    std::cout << " bytes:\n";
    std::cout << "-------------------------------\n";

    for(size_t i = 0; i < sizeof(strings); ++i) {
        chars[i] == '\0' ? std::cout << "\n" : std::cout << chars[i];
    }

    std::cout << "-------------------------------\n";
    std::cout << "\n\n";
}

int main() {
    {
        constexpr auto strings = myMakeStrings("foo", "foobar",
                                               "strings at compile time");
        printStrings(strings);
    }

    {
        constexpr auto strings = myMakeStrings("Some more strings",
                                               "just to show Jeff to not try",
                                               "to challenge C++11 again :P",
                                               "with more",
                                               "to show this is variadic");
        printStrings(strings);
    }

    std::cout << "Running 'objdump -t |grep my' should show that none of the\n";
    std::cout << "functions defined in this file (except printStrings()) are in\n";
    std::cout << "the executable. All computations are done by the compiler at\n";
    std::cout << "compile-time. printStrings() executes at run-time.\n";
}

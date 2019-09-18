# static-regex
static-regex is a static regular expression engine that can convert a regular expression to the corresponding DFA at compile time, and execute the DFA at run time. DFAs are translated into C++ control flow and no memory space for state transition table is needed. It is a project to practice using template metaprogramming.

Example:

    #include <iostream>
    #include "regex.hpp"
    
    using namespace std;
    
    int main() {
        using namespace static_regex;
        using decimal = regex<
            regex_begin, //option
            option<
                select<
                    character<'+'>,
                    character<'-'>
                >
            >,
            digit,
            closure<digit>,
            option<
                concat<
                    character<'.'>,
                    closure<digit>
                >
            >,
            option<
                concat<
                    select<
                        character<'e'>,
                        character<'E'>
                    >,
                    option<
                        select<
                            character<'+'>,
                            character<'-'>
                        >
                    >,
                    digit,
                    closure<digit>
                >
            >,
            regex_end //option
        >;
        cout << decimal::match("abc") << endl; // 0
        cout << decimal::match("1.") << endl; // 1
        cout << decimal::match("-1.1") << endl; // 1
        cout << decimal::match("+1.1e+2") << endl; // 1
        cout << decimal::match("+1.1e-2") << endl; // 1
        cout << decimal::match("+1.1E2") << endl; // 1
        return 0;
    };
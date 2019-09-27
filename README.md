# static-regex
static-regex is a static regular expression engine that can convert a regular expression to the corresponding DFA at compile time, and execute the DFA at run time / compile time. It is a project to practice using template metaprogramming.

Example:

```c++
#include <iostream>
#include "util.hpp"
#include "regex.hpp"
#include "parser.hpp"

using namespace std;

int main() {
    using Decimal = Regex<Concat<
        Begin,
        Option<CharClass<'+', '-'>>,
        Digit,
        Star<Digit>,
        Option<Concat<Char<'.'>, Star<Digit>>>,
        Option<Concat<CharClass<'e', 'E'>, Option<CharClass<'+', '-'>>, Plus<Digit>>>,
        End
    >>;
    using Decimal2 = ParseRegex<'^', '[', '+', '\\', '-', ']', '?', '[', '0', '-', '9', ']', '+',
                                '(', '.', '[', '0', '-', '9', ']', '*', ')', '?', '(', '[', 'e',
                                'E', ']', '[', '+', '\\', '-', ']', '?', '[', '0', '-', '9', ']',
                                '+', ')', '?', '$'>;
    using Decimal3 = REGEX("^[+\\-]?[0-9]+(.[0-9]*)?([eE][+\\-]?[0-9]+)?$");
    // Same as Decimal and Decimal2, but it's based on a Clang and G++ extension, NOT C++ STANDARD.
    cout << Decimal::match("123") << endl; // 1
    cout << Decimal::match("abc") << endl; // 0
    cout << Decimal::match("1.") << endl; // 1
    cout << Decimal::match("-1.1") << endl; // 1
    cout << Decimal::match("+1.1e+2") << endl; // 1
    cout << Decimal::match("+1.1e-2") << endl; // 1
    cout << Decimal::match("+1.1E2") << endl; // 1
    static_assert(!Decimal::match("abc"));
    static_assert(Decimal::match("+1.1E2"));
    return 0;
}
```
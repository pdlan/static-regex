#pragma once
#include "regex.hpp"
#include "parser.hpp"

enum class ParserState {
    Initial,
    ReduceUnion,
    ReduceEnd,
    ReduceParentheses,
    RepeatInitial,
    RepeatUpperbound,
    CharClassInitial,
    CharClassRange,
    CharClassReduce,
    NegCharClassInitial,
    NegCharClassRange,
    NegCharClassReduce
};

static constexpr char GetEscapeChar(char c) {
    switch (c) {
        case 'n': return '\n';
        case 'r': return '\r';
    }
    return c;
}

struct LeftParenthesesToken {};
struct UnionToken {};
struct LeftBracketToken {};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    ParserState State,
    char... Buffer
>
struct ParserIteration;

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '+',
    Buffer...
> {
    using NextStack = TypeList::New<
        Plus<typename Stack::Head>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '?',
    Buffer...
> {
    using NextStack = TypeList::New<
        Option<typename Stack::Head>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '*',
    Buffer...
> {
    using NextStack = TypeList::New<
        Star<typename Stack::Head>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '{',
    Buffer...
> {
    using NextStack = TypeList::New<
        IntWrapper<0>,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::RepeatInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatInitial,
    C,
    Buffer...
> {
    static_assert(C >= '0' && C <= '9');
    constexpr static int Num = Stack::Head::Value;
    constexpr static int NextNum = Num * 10 + C - '0';
    using NextStack = TypeList::New<
        IntWrapper<NextNum>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::RepeatInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatInitial,
    ',',
    Buffer...
> {
    using NextStack = TypeList::New<
        IntWrapper<0>,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::RepeatUpperbound,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatInitial,
    '}',
    Buffer...
> {
    static constexpr int N = Stack::Head::Value;
    using NextStack = TypeList::New<
        Repeat<TypeList::Get<Stack, 1>, N>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatInitial,
    ',', '}',
    Buffer...
> {
    static constexpr int N = Stack::Head::Value;
    using NextStack = TypeList::New<
        AtLeast<TypeList::Get<Stack, 1>, N>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatUpperbound,
    C,
    Buffer...
> {
    static_assert(C >= '0' && C <= '9');
    constexpr static int Num = Stack::Head::Value;
    constexpr static int NextNum = Num * 10 + C - '0';
    using NextStack = TypeList::New<
        IntWrapper<NextNum>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::RepeatUpperbound,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::RepeatUpperbound,
    '}',
    Buffer...
> {
    static constexpr int LowerBound = TypeList::Get<Stack, 1>::Value;
    static constexpr int UpperBound = TypeList::Get<Stack, 0>::Value;
    using NewStack = TypeList::New<
        Repeat<TypeList::Get<Stack, 2>, LowerBound, UpperBound>,
        typename Stack::Rest::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NewStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '[',
    Buffer...
> {
    using NextStack = TypeList::New<
        LeftBracketToken,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::CharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassInitial,
    '\\', C,
    Buffer...
> {
    using NextStack = TypeList::New<
        Char<GetEscapeChar(C)>,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::CharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassInitial,
    C,
    Buffer...
> {
    template <char C_>
    struct SelectSymbol {
        using Type = Char<C_>;
    };
    template <>
    struct SelectSymbol<'^'> {
        using Type = Begin;
    };
    template <>
    struct SelectSymbol<'$'> {
        using Type = End;
    };
    using NextStack = TypeList::New<
        typename SelectSymbol<C>::Type,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::CharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassInitial,
    '-',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::CharClassRange,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassRange,
    C,
    Buffer...
> {
    static constexpr int C1 = Stack::Head::Value;
    using NextStack = TypeList::New<
        Range<C1, C>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::CharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassInitial,
    ']',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        TypeList::Get<Stack, 0>,
        TypeList::Get<Stack, 1>,
        ParenthesesCount,
        ParserState::CharClassReduce,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::CharClassReduce,
    Buffer...
> {
    using NextStack = TypeList::New<
        CharClassUnion<StackTopFirst, StackTopSecond>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        TypeList::Get<NextStack, 0>,
        TypeList::Get<NextStack, 1>,
        ParenthesesCount,
        ParserState::CharClassReduce,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    LeftBracketToken,
    ParenthesesCount,
    ParserState::CharClassReduce,
    Buffer...
> {
    using NextStack = TypeList::New<
        StackTopFirst,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '[', '^',
    Buffer...
> {
    using NextStack = TypeList::New<
        LeftBracketToken,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::NegCharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassInitial,
    '\\', C,
    Buffer...
> {
    using NextStack = TypeList::New<
        Char<GetEscapeChar(C)>,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::NegCharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassInitial,
    C,
    Buffer...
> {
    template <char C_>
    struct SelectSymbol {
        using Type = Char<C_>;
    };
    template <>
    struct SelectSymbol<'^'> {
        using Type = Begin;
    };
    template <>
    struct SelectSymbol<'$'> {
        using Type = End;
    };
    using NextStack = TypeList::New<
        typename SelectSymbol<C>::Type,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::NegCharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassInitial,
    '-',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::NegCharClassRange,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassRange,
    C,
    Buffer...
> {
    static constexpr int C1 = Stack::Head::Value;
    using NextStack = TypeList::New<
        Range<C1, C>,
        typename Stack::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::NegCharClassInitial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassInitial,
    ']',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        TypeList::Get<Stack, 0>,
        TypeList::Get<Stack, 1>,
        ParenthesesCount,
        ParserState::NegCharClassReduce,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::NegCharClassReduce,
    Buffer...
> {
    using NextStack = TypeList::New<
        CharClassComplement<CharClassUnion<StackTopFirst, StackTopSecond>>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        TypeList::Get<NextStack, 0>,
        TypeList::Get<NextStack, 1>,
        ParenthesesCount,
        ParserState::NegCharClassReduce,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    LeftBracketToken,
    ParenthesesCount,
    ParserState::NegCharClassReduce,
    Buffer...
> {
    using NextStack = TypeList::New<
        StackTopFirst,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '\\', C,
    Buffer...
> {
    using NextStack = TypeList::New<
        Char<GetEscapeChar(C)>,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char C,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    C,
    Buffer...
> {
    template <char C_>
    struct SelectSymbol {
        using Type = Char<C_>;
    };
    template <>
    struct SelectSymbol<'^'> {
        using Type = Begin;
    };
    template <>
    struct SelectSymbol<'$'> {
        using Type = End;
    };
    using NextStack = TypeList::New<
        typename SelectSymbol<C>::Type,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '(',
    Buffer...
> {
    using NextStack = TypeList::New<
        LeftParenthesesToken,
        Stack
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount + 1,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '|',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        TypeList::Get<Stack, 0>,
        TypeList::Get<Stack, 1>,
        ParenthesesCount,
        ParserState::ReduceUnion,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::ReduceUnion,
    Buffer...
> {
    using NextStack = TypeList::New<
        Concat<StackTopSecond, StackTopFirst>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        TypeList::Get<NextStack, 0>,
        TypeList::Get<NextStack, 1>,
        ParenthesesCount,
        ParserState::ReduceUnion,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    LeftParenthesesToken,
    ParenthesesCount,
    ParserState::ReduceUnion,
    Buffer...
> {
    using NextStack = TypeList::New<
        UnionToken,
        TypeList::New<
            StackTopFirst,
            typename Stack::Rest
        >
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    UnionToken,
    ParenthesesCount,
    ParserState::ReduceUnion,
    Buffer...
> {
    using NextStack = TypeList::New<
        UnionToken,
        TypeList::New<
            Union<StackTopFirst, TypeList::Get<Stack, 2>>,
            typename Stack::Rest::Rest::Rest
        >
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    Nil,
    ParenthesesCount,
    ParserState::ReduceUnion,
    Buffer...
> {
    using NextStack = TypeList::New<
        UnionToken,
        TypeList::New<
            StackTopFirst,
            Nil
        >
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    '\0'
> {
    static_assert(ParenthesesCount == 0, "Parentheses don't match!");
    using Type = typename ParserIteration<
        Stack,
        TypeList::Get<Stack, 0>,
        TypeList::Get<Stack, 1>,
        ParenthesesCount,
        ParserState::ReduceEnd,
        '\0'
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::ReduceEnd,
    '\0'
> {
    using NextStack = TypeList::New<
        Concat<StackTopSecond, StackTopFirst>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        TypeList::Get<NextStack, 0>,
        TypeList::Get<NextStack, 1>,
        ParenthesesCount,
        ParserState::ReduceEnd,
        '\0'
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    UnionToken,
    ParenthesesCount,
    ParserState::ReduceEnd,
    '\0'
> {
    using Type = Union<StackTopFirst, TypeList::Get<Stack, 2>>;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    Nil,
    ParenthesesCount,
    ParserState::ReduceEnd,
    '\0'
> {
    using Type = StackTopFirst;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::Initial,
    ')',
    Buffer...
> {
    using Type = typename ParserIteration<
        Stack,
        TypeList::Get<Stack, 0>,
        TypeList::Get<Stack, 1>,
        ParenthesesCount,
        ParserState::ReduceParentheses,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    typename StackTopSecond,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    StackTopSecond,
    ParenthesesCount,
    ParserState::ReduceParentheses,
    Buffer...
> {
    using NextStack = TypeList::New<
        Concat<StackTopSecond, StackTopFirst>,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        TypeList::Get<NextStack, 0>,
        TypeList::Get<NextStack, 1>,
        ParenthesesCount,
        ParserState::ReduceParentheses,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    LeftParenthesesToken,
    ParenthesesCount,
    ParserState::ReduceParentheses,
    Buffer...
> {
    using NextStack = TypeList::New<
        StackTopFirst,
        typename Stack::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount - 1,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template
<
    typename Stack,
    typename StackTopFirst,
    int ParenthesesCount,
    char... Buffer
>
struct ParserIteration<
    Stack,
    StackTopFirst,
    UnionToken,
    ParenthesesCount,
    ParserState::ReduceParentheses,
    Buffer...
> {
    static_assert(std::is_same<TypeList::Get<Stack, 3>, LeftParenthesesToken>::value, "Parentheses don't match!");
    using NextStack =TypeList::New<
        Union<StackTopFirst, TypeList::Get<Stack, 2>>,
        typename Stack::Rest::Rest::Rest::Rest
    >;
    using Type = typename ParserIteration<
        NextStack,
        Nil,
        Nil,
        ParenthesesCount - 1,
        ParserState::Initial,
        Buffer...
    >::Type;
};

template <char... Str>
using ParseRegex = Regex<
    typename ParserIteration<
        Nil, Nil, Nil, 0, ParserState::Initial, Str..., '\0'
    >::Type
>;

// Clang and GCC Extension
// Not C++ Standard
template <typename C, C... Args>
ParseRegex<Args...> operator ""_regex();

#define REGEX(r) decltype(r##_regex)
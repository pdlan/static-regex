#pragma once
#include <iostream>
#include <array>
#include <tuple>
#include "util.h"

constexpr int BeginSymbol = 256;
constexpr int EndSymbol = 257;
constexpr int Epsilon = 258;
constexpr int SymbolsCount = 259;

template <int StartState_, int FinalState_, int States_, typename TransitionTable_>
struct NFA {
    static constexpr int StartState = StartState_;
    static constexpr int FinalState = FinalState_;
    static constexpr int States = States_;
    using TransitionTable = TransitionTable_;
};

using EmptyNFA = NFA<0, 1, 2,
    TypeList::New<
        IntTypeMap::Set<Nil, Epsilon, IntSet::New<1, Nil>>,
    Nil>
>;

template <int Symbol>
using SymbolNFA = NFA<0, 1, 2,
    TypeList::New<
        IntTypeMap::Set<Nil, Symbol, IntSet::New<1, Nil>>,
    Nil>
>;

template <int N>
struct TransitionTableAdder {
    template <int Key, typename Value>
    using Add = IntSet::Add<Value, N>;
    template <typename M>
    using Type = IntTypeMap::Apply<M, Add>;
};

template <typename NFA1, typename NFA2>
using ConcatNFA = NFA<0, NFA1::States + NFA2::FinalState - 1, NFA1::States + NFA2::States - 1,
    TypeList::Merge<
        typename NFA1::TransitionTable,
        TypeList::Apply<
            typename NFA2::TransitionTable,
            TransitionTableAdder<NFA1::States - 1>::template Type
        >
    >
>;

template <typename... Args>
struct UnionNFAImpl {
    template <typename...>
    struct StatesCounter;
    template <typename Head, typename... Rest>
    struct StatesCounter<Head, Rest...> {
        static constexpr int States = StatesCounter<Rest...>::States + Head::States;
        static constexpr int StartState = StatesCounter<Rest...>::StartState + Head::States;
    };
    template <typename Head>
    struct StatesCounter<Head> {
        static constexpr int States = Head::States + 2;
        static constexpr int StartState = 1;
    };
    static constexpr int States = StatesCounter<Args...>::States;
    template <typename...>
    struct MergeTransitionTable;
    template <typename Head, typename... Rest>
    struct MergeTransitionTable<Head, Rest...> {
        static constexpr int States = MergeTransitionTable<Rest...>::States + Head::States;
        using Type = TypeList::Merge<
            typename MergeTransitionTable<Rest...>::Type,
            TypeList::Append<
                TypeList::Apply<
                    typename Head::TransitionTable,
                    TransitionTableAdder<MergeTransitionTable<Rest...>::States>::template Type
                >,
                IntTypeMap::Set<
                    Nil,
                    Epsilon,
                    IntSet::New<
                        StatesCounter<Args...>::States - 1,
                        Nil
                    >
                >
            >
        >;
    };
    template <typename Head>
    struct MergeTransitionTable<Head> {
        static constexpr int States = Head::States + 1;
        using Type = TypeList::Append<
            TypeList::Apply<
                typename Head::TransitionTable,
                TransitionTableAdder<1>::template Type
            >,
            IntTypeMap::Set<
                Nil,
                Epsilon,
                IntSet::New<
                    StatesCounter<Args...>::States - 1,
                    Nil
                >
            >
        >;
    };
    template <typename...>
    struct FirstTransitionSet;
    template <typename Head, typename... Rest>
    struct FirstTransitionSet<Head, Rest...> {
        using Type = IntSet::Insert<
            typename FirstTransitionSet<Rest...>::Type,
            StatesCounter<Head, Rest...>::StartState
        >;
    };
    template <typename Head>
    struct FirstTransitionSet<Head> {
        using Type = IntSet::New<
            StatesCounter<Head>::StartState,
            Nil
        >;
    };
    using MergedTransitionTable = typename MergeTransitionTable<Args...>::Type;
    using FirstSet = typename FirstTransitionSet<Args...>::Type;
    using TransitionTable = TypeList::New<
        IntTypeMap::Set<Nil, Epsilon, FirstSet>,
        MergedTransitionTable
    >;
    using Type = NFA<0, StatesCounter<Args...>::States - 1, StatesCounter<Args...>::States,
        TransitionTable
    >;
};

template <typename... Args>
using UnionNFA = typename UnionNFAImpl<Args...>::Type;

template <typename NFA_>
using StarNFA = NFA<0, NFA_::States + 1, NFA_::States + 2,
    TypeList::Merge<
        TypeList::New<
            IntTypeMap::Set<Nil, Epsilon,
                IntSet::New<
                    1,
                    IntSet::New<NFA_::States + 1, Nil>
                >
            >,
            Nil
        >,
        TypeList::Apply<
            typename NFA_::TransitionTable,
            TransitionTableAdder<1>::template Type
        >,
        TypeList::New<
            IntTypeMap::Set<Nil, Epsilon,
                IntSet::New<
                    1,
                    IntSet::New<NFA_::States + 1, Nil>
                >
            >,
            Nil
        >
    >
>;

template <typename NFA_>
using PlusNFA = NFA<0, NFA_::States, NFA_::States + 1,
    TypeList::Append<
        typename NFA_::TransitionTable,
        IntTypeMap::Set<Nil, Epsilon,
            IntSet::New<
                0,
                IntSet::New<NFA_::States, Nil>
            >
        >
    >
>;

template <typename NFA_>
using OptionNFA = NFA<0, NFA_::States, NFA_::States + 1,
    TypeList::New<
        IntTypeMap::Set<Nil, Epsilon,
            IntSet::New<
                1,
                IntSet::New<NFA_::States, Nil>
            >
        >,
        TypeList::Apply<
            typename NFA_::TransitionTable,
            TransitionTableAdder<1>::template Type
        >
    >
>;

template <int... Args>
struct CharClassNFAImpl {
    template <int...>
    struct TransitionMap;
    template <int Head, int... Rest>
    struct TransitionMap<Head, Rest...> {
        using Type = IntTypeMap::Set<
            typename TransitionMap<Rest...>::Type,
            Head,
            IntSet::New<1, Nil>
        >;
    };
    template <int Head>
    struct TransitionMap<Head> {
        using Type = IntTypeMap::Set<
            Nil,
            Head,
            IntSet::New<1, Nil>
        >;
    };
    using Type = NFA<0, 1, 2,
        TypeList::New<
            typename TransitionMap<Args...>::Type,
            Nil
        >
    >;
};

template <typename Set>
struct CharClassFromSetNFAImpl {
    template <typename Set_>
    struct TransitionMap {
        using Type = IntTypeMap::Set<
            typename TransitionMap<typename Set_::Rest>::Type,
            Set_::Head,
            IntSet::New<1, Nil>
        >;
    };
    template <>
    struct TransitionMap<Nil> {
        using Type = Nil;
    };
    using Type = NFA<0, 1, 2,
        TypeList::New<
            typename TransitionMap<Set>::Type,
            Nil
        >
    >;
};

template <int... Args>
using CharClassNFA = typename CharClassNFAImpl<Args...>::Type;

template <typename Set>
using CharClassFromSetNFA = typename CharClassFromSetNFAImpl<Set>::Type;

template <size_t States, size_t Symbols>
using TransitionTable = std::array<std::array<int, Symbols>, States>;

template <typename Subsets, typename Map, size_t Symbols>
struct BuildDFATransitionMapImpl {
    static constexpr std::array<int, Symbols> Build() {
        std::array<int, Symbols> M = BuildDFATransitionMapImpl<
            Subsets,
            typename Map::Rest,
            Symbols
        >::Build();
        M[Map::HeadKey] = TypeList::Find<Subsets, typename Map::HeadValue>::Value;
        return M;
    }
};

template <typename Subsets, size_t Symbols>
struct BuildDFATransitionMapImpl<Subsets, Nil, Symbols> {
    static constexpr std::array<int, Symbols> Build() {
        std::array<int, Symbols> M = {};
        for (size_t i = 0; i < Symbols; ++i) {
            M[i] = -1;
        }
        return M;
    }
};

template <typename Subsets, typename Map, size_t Symbols>
constexpr std::array<int, Symbols> BuildDFATransitionMap() {
    return BuildDFATransitionMapImpl<Subsets, Map, Symbols>::Build();
}

template <typename Subsets, typename SubsetTransitionTable>
struct BuildDFATransitionTableImpl {
    static constexpr int Length = TypeList::Length<Subsets>::Value;
    static constexpr TransitionTable<Length, SymbolsCount> Build() {
        auto Table = BuildDFATransitionTableImpl<
            Subsets,
            typename SubsetTransitionTable::Rest
        >::Build();
        //using Pair = typename SubsetTransitionTable::Head;
        int DFAState = TypeList::Find<Subsets, typename SubsetTransitionTable::HeadKey>::Value;
        std::array<int, SymbolsCount> Map = BuildDFATransitionMap<
            Subsets,
            typename SubsetTransitionTable::HeadValue,
            SymbolsCount
        >();
        Table[DFAState] = Map;
        return Table;
    }
};

template <typename Subsets>
struct BuildDFATransitionTableImpl<Subsets, Nil> {
    static constexpr int Length = TypeList::Length<Subsets>::Value;
    static constexpr TransitionTable<Length, SymbolsCount> Build() {
        return TransitionTable<Length, SymbolsCount>();
    }
};

template <typename Subsets, typename SubsetTransitionTable>
struct BuildDFATransitionTable {
    static constexpr auto Table = BuildDFATransitionTableImpl<Subsets, SubsetTransitionTable>::Build();
};

template <int StartState_, typename FinalStates_, typename TransitionTable_>
struct DFA {
    static constexpr int StartState = StartState_;
    using FinalStates = FinalStates_;
    using TransitionTable = TransitionTable_;
};

template <typename NFA_>
struct NFAToDFAImpl {
    using NFATransitionTable = typename NFA_::TransitionTable;
    template <bool HasNext_, typename NewStates, typename Visited>
    struct EpsilonClosureImpl {
        template <int S, typename T>
        using Merge = IntSet::Union<
            T,
            IntTypeMap::Get<
                TypeList::Get<
                    NFATransitionTable,
                    S
                >,
                Epsilon
            >
        >;
        using NextNewStates = IntSet::Diff<
            IntSet::Reduce<NewStates, Merge>,
            Visited
        >;
        using NextVisited = IntSet::Union<
            Visited,
            NextNewStates
        >;
        static constexpr bool HasNext = !std::is_same<NextNewStates, Nil>::value;
        using Type = typename EpsilonClosureImpl<
            HasNext,
            NextNewStates,
            NextVisited
        >::Type;
    };
    template <typename NewStates, typename Visited>
    struct EpsilonClosureImpl<false, NewStates, Visited> {
        using Type = Visited;
    };
    template <typename States>
    using EpsilonClosure = typename EpsilonClosureImpl<true, States, States>::Type;
    using StartStates = EpsilonClosure<IntSet::New<NFA_::StartState, Nil>>;
    template <typename Subsets_, typename SubsetTransitionTable_>
    struct IterationResult {
        using Subsets = Subsets_;
        using SubsetTransitionTable = SubsetTransitionTable_;
    };
    template <bool HasNext_, typename NewSubsets, typename Visited, typename SubsetTransitionTable>
    struct Iteration {
        template <typename Subset>
        struct NewSTFromSubset {
            template <int S, typename T>
            using FindSymbols = IntSet::Union<
                T,
                IntTypeMap::Keys<TypeList::Get<NFATransitionTable, S>>
            >;
            using Symbols = IntSet::Remove<
                IntSet::Reduce<Subset, FindSymbols>,
                Epsilon
            >;
            template <typename S>
            struct F {
                template <int St, typename T>
                using G = IntSet::Union<
                    T,
                    IntTypeMap::Get<TypeList::Get<NFATransitionTable, St>, S::Head>
                >;
                using NewSubset = EpsilonClosure<IntSet::Reduce<Subset, G>>;
                using Subsets_ = TypeList::New<
                    NewSubset,
                    typename F<typename S::Rest>::Subsets_
                >;
                using TransitionMap = IntTypeMap::Set<
                    typename F<typename S::Rest>::TransitionMap,
                    S::Head,
                    NewSubset
                >;
            };
            template <>
            struct F<Nil> {
                using Subsets_ = Nil;
                using TransitionMap = Nil;
            };
            using Type = F<Symbols>;
        };
        template <typename Subsets>
        struct F {
            using NewTL = typename NewSTFromSubset<typename Subsets::Head>::Type;
            using NextNewSubsets_ = TypeList::MergeNoRepeat<
                typename F<typename Subsets::Rest>::NextNewSubsets_,
                typename NewTL::Subsets_
            >;
            using NextSubsetTransitionTable_ = TypeTypeMap::Set<
                typename F<typename Subsets::Rest>::NextSubsetTransitionTable_,
                typename Subsets::Head,
                typename NewTL::TransitionMap
            >;
        };
        template <>
        struct F<Nil> {
            using NextNewSubsets_ = Nil;
            using NextSubsetTransitionTable_ = SubsetTransitionTable;
        };
        using Next = F<NewSubsets>;
        using NextNewSubsets = TypeList::Diff<
            TypeList::RemoveDuplicated<
                typename Next::NextNewSubsets_
            >,
            Visited
        >;
        using NextTransitionTable = typename Next::NextSubsetTransitionTable_;
        using NextVisited = TypeList::Merge<Visited, NextNewSubsets>;
        static constexpr bool HasNext = !std::is_same<NextNewSubsets, Nil>::value;
        using Type = typename Iteration<HasNext, NextNewSubsets, NextVisited, NextTransitionTable>::Type;
    };
    template <typename NewSubsets, typename Visited, typename SubsetTransitionTable>
    struct Iteration<false, NewSubsets, Visited, SubsetTransitionTable> {
        using Type = IterationResult<
            Visited,
            SubsetTransitionTable
        >;
    };
    using ST = typename Iteration<
        true,
        TypeList::New<StartStates, Nil>,
        TypeList::New<StartStates, Nil>,
        Nil
    >::Type;
    template <typename Subsets, int N>
    struct FinalStates {
        template <bool IsHeadFinal, typename S, int N_>
        struct FinalStatesBranch;
        template <typename S, int N_>
        struct FinalStatesBranch<true, S, N_> {
            using Type = IntSet::Insert<
                typename FinalStates<typename S::Rest, N_ + 1>::Type,
                N_
            >;
        };
        template <typename S, int N_>
        struct FinalStatesBranch<false, S, N_> {
            using Type = typename FinalStates<typename S::Rest, N_ + 1>::Type;
        };
        using Type = typename FinalStatesBranch<
            IntSet::In<
                typename Subsets::Head,
                NFA_::FinalState
            >::Value,
            Subsets,
            N
        >::Type;
    };
    template <int N>
    struct FinalStates<Nil, N> {
        using Type = Nil;
    };
    using DFAFinalStates = typename FinalStates<typename ST::Subsets, 0>::Type;
    using Type = DFA<
        0,
        DFAFinalStates,
        BuildDFATransitionTable<typename ST::Subsets, typename ST::SubsetTransitionTable>
    >;
};

template <typename NFA_>
using NFAToDFA = typename NFAToDFAImpl<NFA_>::Type;

template <size_t N, typename T>
constexpr std::array<typename T::value_type, N> TruncateArray(T In) {
    std::array<typename T::value_type, N> Result = {};
    for (size_t i = 0; i < N; ++i) {
        Result[i] = In[i];
    }
    return Result;
}

template <typename DFA_>
struct MinimizeDFAImpl {
    static constexpr int OldStates = std::tuple_size<decltype(DFA_::TransitionTable::Table)>::value;
    static constexpr std::tuple<
        std::array<
            std::array<int, SymbolsCount>,
            OldStates
        >,                              // transition table
        int,                            // states count
        std::array<
        int,
            IntSet::Length<
                typename DFA_::FinalStates
            >::Value
        >,                              // final states
        int                             // final states count
    > MinimizeTransitionTable() {
        auto Table = DFA_::TransitionTable::Table;
        int States = OldStates;
        std::array<int, OldStates> StatesToMerge = {};
        std::array<int, OldStates> OldNewStatesMap = {};
        auto FinalStates = IntSet::ToArray<typename DFA_::FinalStates>::Array();
        int FinalStatesCount = std::tuple_size<decltype(FinalStates)>::value;
        for (int i = 0; i < States; ) {
            int StatesToMergeCount = 0;
            bool IsIFinal = false;
            for (int k = 0; k < FinalStatesCount; ++k) {
                if (i == FinalStates[k]) {
                    IsIFinal = true;
                    break;
                }
            }
            for (int j = i + 1; j < States; ++j) {
                bool IsJFinal = false;
                for (int k = 0; k < FinalStatesCount; ++k) {
                    if (j == FinalStates[k]) {
                        IsJFinal = true;
                        break;
                    }
                }
                if (IsIFinal != IsJFinal) {
                    continue;
                }
                bool IsSame = true;
                for (int s = 0; s < SymbolsCount; ++s) {
                    if (Table[i][s] != Table[j][s]) {
                        IsSame = false;
                        break;
                    }
                }
                if (!IsSame) {
                    continue;
                }
                StatesToMerge[StatesToMergeCount] = j;
                ++StatesToMergeCount;
            }
            if (StatesToMergeCount == 0) {
                ++i;
                continue;
            }
            int j = 0, k = 0, l = 0;
            for (; j < States; ++j) {
                for (; StatesToMerge[k] < j; ++k);
                bool IsToMerge = StatesToMerge[k] == j;
                if (IsToMerge) {
                    OldNewStatesMap[j] = i;
                    ++k;
                } else {
                    OldNewStatesMap[j] = l;
                    ++l;
                }
            }
            j = 0, k = 0, l = 0;
            for (; j < States; ++j) {
                for (; StatesToMerge[k] < j; ++k);
                bool IsToMerge = StatesToMerge[k] == j;
                if (IsToMerge) {
                    ++k;
                } else {
                    for (int s = 0; s < SymbolsCount; ++s) {
                        if (Table[j][s] == -1) {
                            Table[l][s] = -1;
                        } else {
                            Table[l][s] = OldNewStatesMap[Table[j][s]];
                        }
                    }
                    ++l;
                }
            }
            States = l;
            j = 0; k = 0; l = 0;
            for (; j < FinalStatesCount; ++j) {
                for (; StatesToMerge[k] < FinalStates[j]; ++k);
                bool IsToMerge = StatesToMerge[k] == FinalStates[j];
                if (IsToMerge) {
                    ++k;
                } else {
                    FinalStates[l] = OldNewStatesMap[FinalStates[j]];
                    ++l;
                }
            }
            FinalStatesCount = l;
            i = 0;
        }
        return std::make_tuple(Table, States, FinalStates, FinalStatesCount);
    }
    static constexpr auto NewT = MinimizeTransitionTable();
    struct FinalStatesTemp {
        static constexpr auto Array = TruncateArray<
            std::get<3>(NewT),
            std::array<int,
                IntSet::Length<
                    typename DFA_::FinalStates
                >::Value
            >
        >(std::get<2>(NewT));
    };
    using FinalStates = IntSet::FromArray<FinalStatesTemp>;
    struct TransitionTable {
        static constexpr auto Table = TruncateArray<
            std::get<1>(NewT),
            std::array<std::array<int, SymbolsCount>, OldStates>
        >(std::get<0>(NewT));
    };
    using Type = DFA<0, FinalStates, TransitionTable>;
};

template <typename DFA_>
using MinimizeDFA = typename MinimizeDFAImpl<DFA_>::Type;

template <char C>
struct Char {
    using NFA_ = SymbolNFA<C>;
    using Set = IntSet::New<C, Nil>;
    static constexpr int Value = C;
};

struct Begin {
    using NFA_ = SymbolNFA<BeginSymbol>;
    using Set = IntSet::New<BeginSymbol, Nil>;
    static constexpr int Value = BeginSymbol;
};

struct End {
    using NFA_ = SymbolNFA<EndSymbol>;
    using Set = IntSet::New<EndSymbol, Nil>;
    static constexpr int Value = EndSymbol;
};

struct Empty {
    using NFA_ = EmptyNFA;
};

template <typename... Args>
struct Union {
    using NFA_ = UnionNFA<typename Args::NFA_...>;
};

template <typename R>
struct Star {
    using NFA_ = StarNFA<typename R::NFA_>;
};

template <typename R>
struct Plus {
    using NFA_ = PlusNFA<typename R::NFA_>;
};

template <typename R>
struct Option {
    using NFA_ = OptionNFA<typename R::NFA_>;
};

template <typename... Args>
struct Concat;

template <typename Head, typename... Rest>
struct Concat<Head, Rest...> {
    using NFA_ = ConcatNFA<typename Head::NFA_, typename Concat<Rest...>::NFA_>;
};

template <typename Head>
struct Concat<Head> {
    using NFA_ = typename Head::NFA_;;
};

template <typename R, int... N>
struct Repeat;

template <typename R, int N>
struct Repeat<R, N> {
    using NFA_ = ConcatNFA<typename R::NFA_, typename Repeat<R, N - 1>::NFA_>;
};

template <typename R>
struct Repeat<R, 1> {
    using NFA_ = typename R::NFA_;
};

template <typename R>
struct Repeat<R, 0> {
    using NFA_ = EmptyNFA;
};

template <typename R, int N1, int N2>
struct Repeat<R, N1, N2> {
    using NFA_ = ConcatNFA<typename Repeat<R, N1>::NFA_, typename Repeat<Option<R>, N2 - N1>::NFA_>;
};

template <typename R, int N>
using AtLeast = Concat<Repeat<R, N>, Star<R>>;

template <char... Args>
struct CharClass {
    using NFA_ = CharClassNFA<Args...>;
    using Set = IntSet::FromVaradic<Args...>;
};

template <unsigned char C1, unsigned char C2>
struct Range {
    using NFA_ = CharClassFromSetNFA<IntSet::Range<C1, C2 + 1>>;
    using Set = IntSet::Range<C1, C2 + 1>;
};

template <typename... Args>
struct CharClassUnion {
    using Set = IntSet::Union<typename Args::Set...>;
    using NFA_ = CharClassFromSetNFA<Set>;
};

template <typename Set_>
struct CharClassFromSet {
    using Set = Set_;
    using NFA_ = CharClassFromSetNFA<Set>;
};

template <typename C>
struct CharClassComplement {
    using Set = IntSet::Diff<
        IntSet::Range<0, SymbolsCount>,
        typename C::Set
    >;
    using NFA_ = CharClassFromSetNFA<Set>;
};

using Any = Range<0, 255>;
using Digit = Range<'0', '9'>;
using LowerCase = Range<'a', 'z'>;
using UpperCase = Range<'A', 'Z'>;
using Letter = CharClassUnion<LowerCase, UpperCase>;

template <typename R>
class Regex {
private:
    using NFA_ = typename R::NFA_;
    using DFA_ = NFAToDFA<NFA_>;
    using MinimalDFA = DFA_;
    template <typename Set>
    static constexpr bool is_final(int state) {
        return Set::Head == state || is_final<typename Set::Rest>(state);
    }
    template <>
    static constexpr bool is_final<Nil>(int state) {
        return false;
    }
public:
    static bool match(const std::string &str) {
        int state = MinimalDFA::StartState;
        state = MinimalDFA::TransitionTable::Table[state][BeginSymbol];
        for (char c : str) {
            if (state == -1) {
                return false;
            }
            state = MinimalDFA::TransitionTable::Table[state][c];
        }
        if (state == -1) {
            return false;
        }
        state = MinimalDFA::TransitionTable::Table[state][EndSymbol];
        return is_final<typename MinimalDFA::FinalStates>(state);
    }
    static constexpr bool match(const char *str, size_t length) {
        int state = MinimalDFA::StartState;
        state = MinimalDFA::TransitionTable::Table[state][BeginSymbol];
        for (size_t i = 0; i < length; ++i) {
            if (state == -1) {
                return false;
            }
            state = MinimalDFA::TransitionTable::Table[state][str[i]];
        }
        if (state == -1) {
            return false;
        }
        state = MinimalDFA::TransitionTable::Table[state][EndSymbol];
        return is_final<typename MinimalDFA::FinalStates>(state);
    }
    static constexpr bool match(const char *str) {
        int state = MinimalDFA::StartState;
        state = MinimalDFA::TransitionTable::Table[state][BeginSymbol];
        for (size_t i = 0; str[i]; ++i) {
            if (state == -1) {
                return false;
            }
            state = MinimalDFA::TransitionTable::Table[state][str[i]];
        }
        if (state == -1) {
            return false;
        }
        state = MinimalDFA::TransitionTable::Table[state][EndSymbol];
        return is_final<typename MinimalDFA::FinalStates>(state);
    }
};
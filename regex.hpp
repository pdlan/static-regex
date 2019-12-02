#pragma once
#include <iostream>
#include <array>
#include <tuple>
#include "util.hpp"

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
    TypeList::List<
        TypeTypeMap::Map<TypeTypePair<std::integral_constant<int, Epsilon>, IntSet::Set<1>>>
    >
>;

template <int Symbol>
using SymbolNFA = NFA<0, 1, 2,
    TypeList::List<
        TypeTypeMap::Map<TypeTypePair<std::integral_constant<int, Symbol>, IntSet::Set<1>>>
    >
>;

template <int N>
struct TransitionTableAdder {
    template <typename Key, typename Value>
    using Add = IntSet::Add<Value, N>;
    template <typename M>
    using Type = TypeTypeMap::Transform<M, Add>;
};

template <typename NFA1, typename NFA2>
using ConcatNFA = NFA<0, NFA1::States + NFA2::FinalState - 1, NFA1::States + NFA2::States - 1,
    TypeList::Merge<
        typename NFA1::TransitionTable,
        TypeList::Transform<
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
                TypeList::Transform<
                    typename Head::TransitionTable,
                    TransitionTableAdder<MergeTransitionTable<Rest...>::States>::template Type
                >,
                TypeTypeMap::Map<
                    TypeTypePair<std::integral_constant<int, Epsilon>, IntSet::Set<StatesCounter<Args...>::States - 1>>
                >
            >
        >;
    };
    template <typename Head>
    struct MergeTransitionTable<Head> {
        static constexpr int States = Head::States + 1;
        using Type = TypeList::Append<
            TypeList::Transform<
                typename Head::TransitionTable,
                TransitionTableAdder<1>::template Type
            >,
            TypeTypeMap::Map<
                TypeTypePair<std::integral_constant<int, Epsilon>, IntSet::Set<StatesCounter<Args...>::States - 1>>
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
        using Type = IntSet::Set<StatesCounter<Head>::StartState>;
    };
    using MergedTransitionTable = typename MergeTransitionTable<Args...>::Type;
    using FirstSet = typename FirstTransitionSet<Args...>::Type;
    using TransitionTable = TypeList::PushFront<
        MergedTransitionTable,
        TypeTypeMap::Map<
            TypeTypePair<std::integral_constant<int, Epsilon>, FirstSet>
        >
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
        TypeList::List<
            TypeTypeMap::Map<
                TypeTypePair<
                    std::integral_constant<int, Epsilon>,
                    IntSet::Set<1, NFA_::States + 1>
                >
            >
        >,
        TypeList::Transform<
            typename NFA_::TransitionTable,
            TransitionTableAdder<1>::template Type
        >,
        TypeList::List<
            TypeTypeMap::Map<
                TypeTypePair<
                    std::integral_constant<int, Epsilon>,
                    IntSet::Set<1, NFA_::States + 1>
                >
            >
        >
    >
>;

template <typename NFA_>
using PlusNFA = NFA<0, NFA_::States, NFA_::States + 1,
    TypeList::Append<
        typename NFA_::TransitionTable,
        TypeTypeMap::Map<
            TypeTypePair<
                std::integral_constant<int, Epsilon>,
                IntSet::Set<0, NFA_::States>
            >
        >
    >
>;

template <typename NFA_>
using OptionNFA = NFA<0, NFA_::States, NFA_::States + 1,
    TypeList::PushFront<
        TypeList::Transform<
            typename NFA_::TransitionTable,
            TransitionTableAdder<1>::template Type
        >,
        TypeTypeMap::Map<
            TypeTypePair<
                std::integral_constant<int, Epsilon>,
                IntSet::Set<1, NFA_::States>
            >
        >
    >
>;

template <int... Args>
using CharClassNFA = NFA<0, 1, 2,
    TypeList::List<
        TypeTypeMap::Map<
            TypeTypePair<
                std::integral_constant<int, Args>,
                IntSet::Set<1>
            >...
        >
    >
>;

template <typename S>
struct CharClassFromSetNFAImpl;

template <template <int...> class S, int... Args>
struct CharClassFromSetNFAImpl<S<Args...>> {
    using Type = CharClassNFA<Args...>;
};

template <typename S>
using CharClassFromSetNFA = typename CharClassFromSetNFAImpl<S>::Type;

template <size_t States, size_t Symbols>
using TransitionTable = ConstexprArray<ConstexprArray<int, Symbols>, States>;

template <typename Subsets, typename Map, size_t Symbols, typename Index>
struct BuildDFATransitionMapImpl;

template <typename Subsets, typename Map, size_t Symbols, size_t... I>
struct BuildDFATransitionMapImpl<Subsets, Map, Symbols, std::index_sequence<I...>> {
    static constexpr ConstexprArray<int, Symbols> Build() {
        return ConstexprArray<int, Symbols> (
            int(TypeSet::Find<Subsets, TypeTypeMap::Get<Map, std::integral_constant<int, I>>>::Value)...
        );
    }
};

template <typename Subsets, typename Map, size_t Symbols>
static constexpr ConstexprArray<int, Symbols> BuildDFATransitionMap() {
    return BuildDFATransitionMapImpl<Subsets, Map, Symbols, std::make_index_sequence<Symbols>>::Build();
}

template <typename Subsets, typename SubsetTransitionTable>
struct BuildDFATransitionTable;

template <template <typename...> class L, typename... Args, typename SubsetTransitionTable>
struct BuildDFATransitionTable<L<Args...>, SubsetTransitionTable> {
    static constexpr size_t Size = L<Args...>::Size;
    static constexpr TransitionTable<L<Args...>::Size, SymbolsCount> Table = TransitionTable<L<Args...>::Size, SymbolsCount>(
        BuildDFATransitionMap<
            L<Args...>,
            TypeTypeMap::Get<
                SubsetTransitionTable,
                Args,
                TypeTypeMap::Map<>
            >,
            SymbolsCount
        >()...
    );
};

template <template <typename...> class L, typename... Args, typename SubsetTransitionTable>
constexpr TransitionTable<L<Args...>::Size, SymbolsCount> BuildDFATransitionTable<L<Args...>, SubsetTransitionTable>::Table;

template <typename L>
struct ExtractIntImpl {
    template <typename T, typename R>
    using F = IntSet::Insert<R, T::value>;
    using Type = TypeList::Reduce<L, F, IntSet::Set<>>;
};

template <typename L>
using ExtractInt = typename ExtractIntImpl<L>::Type;

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
            TypeTypeMap::Get<
                TypeList::Get<
                    NFATransitionTable,
                    S,
                    TypeTypeMap::Map<>
                >,
                std::integral_constant<int, Epsilon>,
                IntSet::Set<>
            >
        >;
        using NextNewStates = IntSet::Diff<
            IntSet::Reduce<NewStates, Merge, IntSet::Set<>>,
            Visited
        >;
        using NextVisited = IntSet::Union<
            Visited,
            NextNewStates
        >;
        static constexpr bool HasNext = !std::is_same<NextNewStates, IntSet::Set<>>::value;
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
    using StartStates = EpsilonClosure<IntSet::Set<NFA_::StartState>>;
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
                ExtractInt<TypeTypeMap::Keys<TypeList::Get<NFATransitionTable, S, TypeTypeMap::Map<>>>>
            >;
            using Symbols = IntSet::Remove<
                IntSet::Reduce<Subset, FindSymbols, IntSet::Set<>>,
                Epsilon
            >;
            struct EmptyST {
                using Subsets_ = TypeSet::Set<>;
                using TransitionMap = TypeTypeMap::Map<>;
            };
            template <int S, typename T>
            struct F {
                template <int St, typename R>
                using G = IntSet::Union<
                    R,
                    TypeTypeMap::Get<
                        TypeList::Get<NFATransitionTable, St, TypeTypeMap::Map<>>,
                        std::integral_constant<int, S>,
                        IntSet::Set<>
                    >
                >;
                using NewSubset = EpsilonClosure<IntSet::Reduce<Subset, G, IntSet::Set<>>>;
                using Subsets_ = TypeSet::Insert<typename T::Subsets_, NewSubset>;
                using TransitionMap = TypeTypeMap::Set<
                    typename T::TransitionMap,
                    std::integral_constant<int, S>,
                    NewSubset
                >;
            };
            using Type = IntSet::Reduce<Symbols, F, EmptyST>;
        };
        struct EmptyNext {
            using NextNewSubsets_ = TypeSet::Set<>;
            using NextSubsetTransitionTable_ = SubsetTransitionTable;
        };
        template <typename Subset, typename R>
        struct F {
            using NewTL = typename NewSTFromSubset<Subset>::Type;
            using NextNewSubsets_ = TypeSet::Union<
                typename R::NextNewSubsets_,
                typename NewTL::Subsets_
            >;
            using NextSubsetTransitionTable_ = TypeTypeMap::Set<
                typename R::NextSubsetTransitionTable_,
                Subset,
                typename NewTL::TransitionMap
            >;
        };
        using Next = TypeList::Reduce<NewSubsets, F, EmptyNext>;
        using NextNewSubsets = TypeSet::Diff<
            typename Next::NextNewSubsets_,
            Visited
        >;
        using NextTransitionTable = typename Next::NextSubsetTransitionTable_;
        using NextVisited = TypeSet::Union<Visited, typename Next::NextNewSubsets_>;
        static constexpr bool HasNext = !std::is_same<NextNewSubsets, TypeSet::Set<>>::value;
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
        TypeSet::Set<StartStates>,
        TypeSet::Set<StartStates>,
        TypeTypeMap::Map<>
    >::Type;
    template <typename T, typename R>
    struct FinalStatesImpl {
        template <bool>
        struct Branch {
            using Type = IntSet::Insert<R, TypeSet::Find<typename ST::Subsets, T>::Value>;
        };
        template <>
        struct Branch<false> {
            using Type = R;
        };
        using Type = typename Branch<IntSet::Contains<T, NFA_::FinalState>::Value>::Type;
    };
    template <typename T, typename R>
    using FinalStates = typename FinalStatesImpl<T, R>::Type;
    using DFAFinalStates = TypeList::Reduce<typename ST::Subsets, FinalStates, IntSet::Set<>>;
    using Type = DFA<
        0,
        DFAFinalStates,
        BuildDFATransitionTable<typename ST::Subsets, typename ST::SubsetTransitionTable>
    >;
};

template <typename NFA_>
using NFAToDFA = typename NFAToDFAImpl<NFA_>::Type;

template <size_t N, typename T>
constexpr ConstexprArray<typename T::value_type, N> TruncateArray(T In) {
    ConstexprArray<typename T::value_type, N> Result;
    for (size_t i = 0; i < N; ++i) {
        Result[i] = In[i];
    }
    return Result;
}

template <typename DFA_>
struct MinimizeDFAImpl {
    static constexpr int OldStates = DFA_::TransitionTable::Table.size();
    static constexpr std::tuple<
        ConstexprArray<
            ConstexprArray<int, SymbolsCount>,
            OldStates
        >,                              // transition table
        int,                            // states count
        ConstexprArray<
            int,
            DFA_::FinalStates::Size
        >,                              // final states
        int                             // final states count
    > MinimizeTransitionTable() {
        auto Table = DFA_::TransitionTable::Table;
        int States = OldStates;
        ConstexprArray<int, OldStates> StatesToMerge;
        ConstexprArray<int, OldStates> OldNewStatesMap;
        auto FinalStates = DFA_::FinalStates::Array;
        int FinalStatesCount = FinalStates.size();
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
            ConstexprArray<
                int,
                DFA_::FinalStates::Size
            >
        >(std::get<2>(NewT));
    };
    using FinalStates = IntSet::FromArray<FinalStatesTemp>;
    struct TransitionTable {
        static constexpr auto Table = TruncateArray<
            std::get<1>(NewT),
            ConstexprArray<ConstexprArray<int, SymbolsCount>, OldStates>
        >(std::get<0>(NewT));
    };
    using Type = DFA<0, FinalStates, TransitionTable>;
};

template <typename DFA_>
using MinimizeDFA = typename MinimizeDFAImpl<DFA_>::Type;

template <char C>
struct Char {
    using NFA_ = SymbolNFA<C>;
    using Set = IntSet::Set<C>;
    static constexpr int Value = C;
};

struct Begin {
    using NFA_ = SymbolNFA<BeginSymbol>;
    using Set = IntSet::Set<BeginSymbol>;
    static constexpr int Value = BeginSymbol;
};

struct End {
    using NFA_ = SymbolNFA<EndSymbol>;
    using Set = IntSet::Set<EndSymbol>;
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
    using Set = IntSet::Set<Args...>;
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
    static constexpr bool is_final(int state) {
        for (size_t i = 0; i < MinimalDFA::FinalStates::Array.size(); ++i) {
            if (MinimalDFA::FinalStates::Array[i] == state) {
                return true;
            }
        }
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
        return is_final(state);
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
        return is_final(state);
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
        return is_final(state);
    }
    static size_t match_prefix(const std::string &str) {
        int state = MinimalDFA::StartState;
        for (size_t i = 0; i < str.length(); ++i) {
            if (state == -1) {
                return i;
            }
            state = MinimalDFA::TransitionTable::Table[state][str[i]];
        }
        return str.length();
    }
    static constexpr size_t match_prefix(const char *str, size_t length) {
        int state = MinimalDFA::StartState;
        for (size_t i = 0; i < length; ++i) {
            if (state == -1) {
                return i;
            }
            state = MinimalDFA::TransitionTable::Table[state][str[i]];
        }
        return length;
    }
    static constexpr size_t match_prefix(const char *str) {
        int state = MinimalDFA::StartState;
        size_t i = 0;
        for (; str[i]; ++i) {
            if (state == -1) {
                return i;
            }
            state = MinimalDFA::TransitionTable::Table[state][str[i]];
        }
        return i;
    }
};
#pragma once
#include <array>
#include <type_traits>

using Nil = void;

namespace TypeList {
template <typename Head_, typename Rest_>
struct New {
    using Head = Head_;
    using Rest = Rest_;
};

template <bool IsEnd, typename List, template <typename T> class Func>
struct ApplyImpl {
    using Type = TypeList::New<
        Func<typename List::Head>,
        typename ApplyImpl<
            std::is_same<typename List::Rest, Nil>::value,
            typename List::Rest,
            Func
        >::Type
    >;
};

template <typename List, template <typename T> class Func>
struct ApplyImpl<true, List, Func> {
    using Type = Nil;
};

template <typename List, template <typename T> class Func>
using Apply = typename ApplyImpl<
    std::is_same<List, Nil>::value,
    List,
    Func
>::Type;

template <typename Head, typename... Rest>
struct MergeImpl {
    using Type = typename MergeImpl<Head, typename MergeImpl<Rest...>::Type>::Type;
};

template <typename List1, typename List2>
struct MergeImpl<List1, List2> {
    using Type = New<
        typename List1::Head,
        typename MergeImpl<typename List1::Rest, List2>::Type
    >;
};

template <typename List2>
struct MergeImpl<Nil, List2> {
    using Type = List2;
};

template <typename... Args>
using Merge = typename MergeImpl<Args...>::Type;

template <typename List, typename Value>
struct AppendImpl {
    using Type = New<
        typename List::Head,
        typename AppendImpl<typename List::Rest, Value>::Type
    >;
};

template <typename Value>
struct AppendImpl<Nil, Value> {
    using Type = New<Value, Nil>;
};

template <typename List, typename Value>
using Append = typename AppendImpl<List, Value>::Type;

template <typename List, int N>
struct GetImpl {
    using Type = typename GetImpl<typename List::Rest, N - 1>::Type;
};

template <typename List>
struct GetImpl<List, 0> {
    using Type = typename List::Head;
};

template <int N>
struct GetImpl<Nil, N> {
    using Type = Nil;
};

template <>
struct GetImpl<Nil, 0> {
    using Type = Nil;
};

template <typename List, int N>
using Get = typename GetImpl<List, N>::Type;

template <typename Set, typename Value_>
struct In {
    static constexpr bool Value = std::is_same<typename Set::Head, Value_>::value ||
        In<typename Set::Rest, Value_>::Value;
};

template <typename Value_>
struct In<Nil, Value_> {
    static constexpr bool Value = false;
};

template <typename List1, typename List2>
struct DiffImpl {
    template <bool AppendHead>
    struct DiffImplBranch;
    template <>
    struct DiffImplBranch<false> {
        using Type = New<
            typename List1::Head,
            typename DiffImpl<typename List1::Rest, List2>::Type
        >;
    };
    template <>
    struct DiffImplBranch<true> {
        using Type = typename DiffImpl<typename List1::Rest, List2>::Type;
    };
    using Type = typename DiffImplBranch<In<List2, typename List1::Head>::Value>::Type;
};

template <typename List2>
struct DiffImpl<Nil, List2> {
    using Type = Nil;
};

template <typename List1, typename List2>
using Diff = typename DiffImpl<List1, List2>::Type;

template <typename List1, typename List2>
using MergeNoRepeat = Merge<List1, Diff<List2, List1>>;

template <typename List>
struct Length {
    static constexpr int Value = Length<typename List::Rest>::Value + 1;
};

template <>
struct Length<Nil> {
    static constexpr int Value = 0;
};

template <typename List, typename Value_>
struct Find {
    template <bool IsInHead, typename List_, typename Value__>
    struct FindBranch;
    template <typename List_, typename Value__>
    struct FindBranch<true, List_, Value__> {
        static constexpr int V = 0;
    };
    template <typename List_, typename Value__>
    struct FindBranch<false, List_, Value__> {
        static constexpr int R = Find<typename List_::Rest, Value__>::Value;
        static constexpr int V = R == -1 ? -1 : R + 1;
    };
    static constexpr int Value = FindBranch<
        std::is_same<typename List::Head, Value_>::value,
        List,
        Value_
    >::V;
};

template <typename Value_>
struct Find<Nil, Value_> {
    static constexpr int Value = -1;
};

template <typename List>
struct RemoveDuplicatedImpl {
    template <bool RestHasHead, typename List_>
    struct RemoveDuplicatedImplBranch;
    template <typename List_>
    struct RemoveDuplicatedImplBranch<true, List_> {
        using Type = typename RemoveDuplicatedImpl<
            typename List_::Rest
        >::Type;
    };
    template <typename List_>
    struct RemoveDuplicatedImplBranch<false, List_> {
        using Type = New<
            typename List_::Head,
            typename RemoveDuplicatedImpl<
                typename List_::Rest
            >::Type
        >;
    };
    using Type = typename RemoveDuplicatedImplBranch<
        Find<
            typename List::Rest,
            typename List::Head
        >::Value != -1,
        List
    >::Type;
};


template <>
struct RemoveDuplicatedImpl<Nil> {
    using Type = Nil;
};

template <typename List>
using RemoveDuplicated = typename RemoveDuplicatedImpl<List>::Type;
}

template <typename First_, typename Second_>
struct TypeTypePair {
    using First = First_;
    using Second = Second_;
};

template <int First_, typename Second_>
struct IntTypePair {
    static constexpr int First = First_;
    using Second = Second_;
};

namespace TypeTypeMap {
template <typename Map, typename Key, typename Value>
struct Set {
    using HeadKey = Key;
    using HeadValue = Value;
    using Rest = Map;
    template <typename K>
    struct Get {
        using Type = typename Map::template Get<K>::Type;
    };
    template <>
    struct Get<Key> {
        using Type = Value;
    };
};

template <typename Key, typename Value>
struct Set<Nil, Key, Value> {
    using HeadKey = Key;
    using HeadValue = Value;
    using Rest = Nil;
    template <typename K>
    struct Get {
        using Type = Nil;
    };
    template <>
    struct Get<Key> {
        using Type = Value;
    };
};

template <typename Map, typename Key>
struct GetImpl {
    using Type = typename Map::template Get<Key>::Type;
};

template <typename Key>
struct GetImpl<Nil, Key> {
    using Type = Nil;
};

template <typename Map, typename Key>
using Get = typename GetImpl<Map, Key>::Type;
}

namespace IntSet {
template <int Head_, typename Rest_>
struct New {
    static constexpr int Head = Head_;
    using Rest = Rest_;
};

template <typename Set, int N>
struct AddImpl {
    using Type = New<
        Set::Head + N,
        typename AddImpl<typename Set::Rest, N>::Type
    >;
};

template <int N>
struct AddImpl<Nil, N> {
    using Type = Nil;
};

template <typename Set, int N>
using Add = typename AddImpl<Set, N>::Type;

template <typename Set, int Value>
struct InsertImpl {
    template <int Branch, typename S>
    struct InsertImplBranch;
    template <typename S>
    struct InsertImplBranch<0, S> {
        using Type = S;
    };
    
    template <typename S>
    struct InsertImplBranch<1, S> {
        using Type = IntSet::New<
            Value,
            S
        >;
    };
    
    template <typename S>
    struct InsertImplBranch<2, S> {
        using Type = IntSet::New<
            S::Head,
            typename InsertImpl<typename S::Rest, Value>::Type
        >;
    };
    static constexpr int Branch = Set::Head == Value ? 0 :
        (Set::Head > Value ? 1 : 2);
    using Type = typename InsertImplBranch<Branch, Set>::Type;
};

template <int Value>
struct InsertImpl<Nil, Value> {
    using Type = IntSet::New<Value, Nil>;
};

template <typename Set, int Value>
using Insert = typename InsertImpl<Set, Value>::Type;

template <int I, int J>
struct RangeImpl {
    using Type = New<
        I,
        typename RangeImpl<I + 1, J>::Type
    >;
};

template <int I>
struct RangeImpl<I, I> {
    using Type = Nil;
};

template <int I, int J>
using Range = typename RangeImpl<I, J>::Type;

template <typename Set, int Value_>
struct In {
    static constexpr bool Value = Set::Head == Value_ ||
        In<typename Set::Rest, Value_>::Value;
};

template <int Value_>
struct In<Nil, Value_> {
    static constexpr bool Value = false;
};

template <typename Set, template <int S, typename T> class Func>
struct ReduceImpl {
    using Type = Func<Set::Head, typename ReduceImpl<typename Set::Rest, Func>::Type>;
};

template <template <int S, typename T> class Func>
struct ReduceImpl<Nil, Func> {
    using Type = Nil;
};

template <typename Set, template <int S, typename T> class Func>
using Reduce = typename ReduceImpl<Set, Func>::Type;

template <typename... Args>
struct UnionImpl;

template <typename First, typename... Rest>
struct UnionImpl<First, Rest...> {
    using Type = typename UnionImpl<First, UnionImpl<Rest...>>::Type;
};

template <typename Set1, typename Set2>
struct UnionImpl<Set1, Set2> {
    using Type = Insert<
        typename UnionImpl<Set1, typename Set2::Rest>::Type,
        Set2::Head
    >;
};

template <typename Set1>
struct UnionImpl<Set1, Nil> {
    using Type = Set1;
};

template <typename... Args>
using Union = typename UnionImpl<Args...>::Type;

template <typename Set1, typename Set2>
struct DiffImpl {
    template <bool IsHeadEqual, typename S1, typename S2>
    struct DiffImplBranch;
    template <typename S1, typename S2>
    struct DiffImplBranch<true, S1, S2> {
        using Type = typename DiffImpl<typename S1::Rest, S2>::Type;
    };
    template <typename S1, typename S2>
    struct DiffImplBranch<false, S1, S2> {
        using Type = New<
            S1::Head,
            typename DiffImpl<typename S1::Rest, S2>::Type
        >;
    };
    using Type = typename DiffImplBranch<Set1::Head == Set2::Head, Set1, Set2>::Type;
};

template <typename Set1>
struct DiffImpl<Set1, Nil> {
    using Type = Set1;
};

template <typename Set2>
struct DiffImpl<Nil, Set2> {
    using Type = Nil;
};

template <>
struct DiffImpl<Nil, Nil> {
    using Type = Nil;
};

template <typename Set1, typename Set2>
using Diff = typename DiffImpl<Set1, Set2>::Type;

template <typename Set, int Value>
struct RemoveImpl {
    template <bool IsInHead, typename Set_, int Value_>
    struct RemoveImplBranch;
    template <typename Set_, int Value_>
    struct RemoveImplBranch<true, Set_, Value_> {
        using Type = typename Set_::Rest;
    };
    template <typename Set_, int Value_>
    struct RemoveImplBranch<false, Set_, Value_> {
        using Type = New<
            Set_::Head,
            typename RemoveImpl<typename Set_::Rest, Value>::Type
        >;
    };
    using Type = typename RemoveImplBranch<Set::Head == Value, Set, Value>::Type;
};

template <int Value>
struct RemoveImpl<Nil, Value> {
    using Type = Nil;
};

template <typename Set, int Value>
using Remove = typename RemoveImpl<Set, Value>::Type;

template <typename Set>
struct Length {
    static constexpr int Value = Length<typename Set::Rest>::Value + 1;
};

template <>
struct Length<Nil> {
    static constexpr int Value = 0;
};

template <typename Set>
struct ToArray {
    static constexpr std::array<int, Length<Set>::Value> Array() {
        std::array<int, Length<Set>::Value> Result = {};
        Result[0] = Set::Head;
        auto Rest = ToArray<typename Set::Rest>::Array();
        for (int i = 0; i < Length<typename Set::Rest>::Value; ++i) {
            Result[i + 1] = Rest[i];
        }
        return Result;
    }
};

template <>
struct ToArray<Nil> {
    static constexpr std::array<int, 0> Array() {
        return std::array<int, 0>();
    }
};

template <typename Arr, int i>
struct FromArrayImpl {
    using Type = Insert<
        typename FromArrayImpl<Arr, i - 1>::Type,
        Arr::Array[i - 1]
    >;
};

template <typename Arr>
struct FromArrayImpl<Arr, 0> {
    using Type = Nil;
};

template <typename Arr>
using FromArray = typename FromArrayImpl<
    Arr,
    std::tuple_size<decltype(Arr::Array)>::value
>::Type;

template <int... Args>
struct FromVaradicImpl;

template <int Head, int... Rest>
struct FromVaradicImpl<Head, Rest...> {
    using Type = IntSet::Insert<
        typename FromVaradicImpl<Rest...>::Type,
        Head
    >;
};

template <int Head>
struct FromVaradicImpl<Head> {
    using Type = IntSet::New<Head, Nil>;
};

template <int... Args>
using FromVaradic = typename FromVaradicImpl<Args...>::Type;
}

namespace IntTypeMap {
template <typename Map, int Key, typename Value>
struct Set {
    static constexpr int HeadKey = Key;
    using HeadValue = Value;
    using Rest = Map;
    template <int K>
    struct Get {
        using Type = typename Map::template Get<K>::Type;
    };
    template <>
    struct Get<Key> {
        using Type = Value;
    };
};

template <int Key, typename Value>
struct Set<Nil, Key, Value> {
    static constexpr int HeadKey = Key;
    using HeadValue = Value;
    using Rest = Nil;
    template <int K>
    struct Get {
        using Type = Nil;
    };
    template <>
    struct Get<Key> {
        using Type = Value;
    };
};

template <typename Map, int Key>
struct GetImpl {
    using Type = typename Map::template Get<Key>::Type;
};

template <int Key>
struct GetImpl<Nil, Key> {
    using Type = Nil;
};

template <typename Map, int Key>
using Get = typename GetImpl<Map, Key>::Type;

template <typename Map>
struct KeysImpl {
    using Type = IntSet::Insert<
        typename KeysImpl<typename Map::Rest>::Type,
        Map::HeadKey
    >;
};

template <>
struct KeysImpl<Nil> {
    using Type = Nil;
};

template <typename Map>
using Keys = typename KeysImpl<Map>::Type;

template <typename Map, template <int K, typename V> class Func>
struct ApplyImpl {
    using Type = Set<
        typename ApplyImpl<typename Map::Rest, Func>::Type,
        Map::HeadKey,
        Func<Map::HeadKey, typename Map::HeadValue>
    >;
};

template <template <int K, typename V> class Func>
struct ApplyImpl<Nil, Func> {
    using Type = Nil;
};

template <typename Map, template <int K, typename V> class Func>
using Apply = typename ApplyImpl<
    Map,
    Func
>::Type;
}

/*
Deprecated for low efficiency

namespace IntTypeMapOld {
template <typename Map, int Key>
struct GetImpl {
    template <bool IsInHead, typename M>
    struct GetImplBranch;
    template <typename M>
    struct GetImplBranch<true, M> {
        using Type = typename M::Head::Second;
    };
    
    template <typename M>
    struct GetImplBranch<false, M> {
        using Type = typename GetImpl<typename M::Rest, Key>::Type;
    };
    static constexpr bool IsInHead = Map::Head::First == Key;
    using Type = typename GetImplBranch<IsInHead, Map>::Type;
};

template <int Key>
struct GetImpl<Nil, Key> {
    using Type = Nil;
};

template <typename Map, int Key>
using Get = typename GetImpl<Map, Key>::Type;

template <typename Map, int Key, typename Value>
struct SetImpl {
    template <bool IsInHead, typename M>
    struct SetImplBranch;
    template <typename M>
    struct SetImplBranch<true, M> {
        using Type = TypeList::New<
            IntTypePair<M::Head::First, Value>,
            typename M::Rest
        >;
    };
    
    template <typename M>
    struct SetImplBranch<false, M> {
        using Type = TypeList::New<
            typename M::Head,
            typename SetImpl<typename M::Rest, Key, Value>::Type
        >;
    };
    static constexpr bool IsInHead = Map::Head::First == Key;
    using Type = typename SetImplBranch<IsInHead, Map>::Type;
};

template <int Key, typename Value>
struct SetImpl<Nil, Key, Value> {
    using Type = TypeList::New<IntTypePair<Key, Value>, Nil>;
};

template <typename Map, int Key, typename Value>
using Set = typename SetImpl<Map, Key, Value>::Type;

template <typename Map>
struct KeysImpl {
    using Type = IntSet::Insert<
        typename KeysImpl<typename Map::Rest>::Type,
        Map::Head::First
    >;
};

template <>
struct KeysImpl<Nil> {
    using Type = Nil;
};

template <typename Map>
using Keys = typename KeysImpl<Map>::Type;
}
*/

template <int V>
struct IntWrapper {
    static constexpr int Value = V;
};
#pragma once
#include <array>
#include <utility>
#include <type_traits>
#include <initializer_list>

template <typename T, size_t N>
struct ConstexprArray {
    using value_type = T;
    T data_[N];
    constexpr ConstexprArray() : data_{} {}
    template <typename... S>
    constexpr ConstexprArray(S... v) : data_{v...} {}
    constexpr T &operator[] (size_t i) {
        return data_[i];
    }
    constexpr const T &operator[] (size_t i) const {
        return data_[i];
    }
    constexpr size_t size() const {
        return N;
    }
    constexpr const T *data() const {
        return data_;
    }
};

template <typename T>
struct Identity {
    using Type = T;
};

template <typename F, typename S>
struct TypeTypePair {
    using First = F;
    using Second = S;
};

template <typename... T>
struct Inherit : T... {};

namespace TypeTypeMap {
template <typename... T>
struct Map {
    static constexpr size_t Size = sizeof...(T);
};

template <typename M, typename Key, typename Default=void>
struct GetImpl;

template <template <typename...> class M, typename... T, typename Key, typename Default>
struct GetImpl<M<T...>, Key, Default> {
    using U = Inherit<Identity<T>...>;
    template <typename V>
    static Identity<V> f(Identity<TypeTypePair<Key, V>> *);
    static Identity<Default> f(...);
    using Type = typename decltype(f((U *)0))::Type;
};

template <typename M, typename Key, typename Default=void>
using Get = typename GetImpl<M, Key, Default>::Type;

template <typename M, typename Key, typename Index>
struct FindImpl;

template <template <typename...> class M, typename... Args, typename Key, size_t... I>
struct FindImpl<M<Args...>, Key, std::index_sequence<I...>> {
    using U = Inherit<TypeTypePair<typename Args::First, std::integral_constant<size_t, I>>...>;
    template <size_t J>
    static constexpr ssize_t f(TypeTypePair<Key, std::integral_constant<size_t, J>> *) {
        return J;
    }
    static constexpr ssize_t f(...) {
        return -1;
    }
    static constexpr ssize_t Value = f((U *)0);
};

template <typename S, typename Key>
struct Find;
template <template <typename...> class M, typename... Args, typename Key>
struct Find<M<Args...>, Key> {
    static constexpr ssize_t Value = FindImpl<M<Args...>, Key, std::make_index_sequence<sizeof...(Args)>>::Value;
};
}

namespace TypeList {
template <typename... T>
struct List {
    static constexpr size_t Size = sizeof...(T);
};

template <typename L, template <typename...> class F>
struct TransformImpl;

template <template <typename...> class L, template <typename...> class F, typename... Args>
struct TransformImpl<L<Args...>, F> {
    using Type = L<F<Args>...>;
};

template <typename L, template <typename...> class F>
using Transform = typename TransformImpl<L, F>::Type;

template <typename L, template <typename...> class F, typename Z>
struct ReduceImpl;

template <template <typename...> class L, template <typename...> class F, typename T, typename... Args, typename Z>
struct ReduceImpl<L<T, Args...>, F, Z> {
    using Type = F<T, typename ReduceImpl<L<Args...>, F, Z>::Type>;
};

template <template <typename...> class L, template <typename...> class F, typename Z>
struct ReduceImpl<L<>, F, Z> {
    using Type = Z;
};

template <typename L, template <typename...> class F, typename Z>
using Reduce = typename ReduceImpl<L, F, Z>::Type;

template <typename... L>
struct MergeImpl;

template <typename Head>
struct MergeImpl<Head> {
    using Type = Head;
};

template <typename Head, typename... Rest>
struct MergeImpl<Head, Rest...> {
    using Type = typename MergeImpl<Head, typename MergeImpl<Rest...>::Type>::Type;
};

template <
    template <typename...> class L,
    typename... Args1,
    typename... Args2
>
struct MergeImpl<L<Args1...>, L<Args2...>> {
    using Type = L<Args1..., Args2...>;
};

template <typename... L>
using Merge = typename MergeImpl<L...>::Type;

template <typename L, typename T>
struct AppendImpl;

template <template <typename...> class L, typename T, typename... Args>
struct AppendImpl<L<Args...>, T> {
    using Type = L<Args..., T>;
};

template <typename L, typename T>
using Append = typename AppendImpl<L, T>::Type;

template <typename L, typename T>
struct PushFrontImpl;

template <template <typename...> class L, typename T, typename... Args>
struct PushFrontImpl<L<Args...>, T> {
    using Type = L<T, Args...>;
};

template <typename L, typename T>
using PushFront = typename PushFrontImpl<L, T>::Type;

template <typename L, typename Index>
struct ToMapImpl;

template <template <typename...> class L, typename... T, size_t... I>
struct ToMapImpl<L<T...>, std::index_sequence<I...>> {
    using Type = TypeTypeMap::Map<TypeTypePair<std::integral_constant<size_t, I>, T>...>;
};

template <typename L>
using ToMap = typename ToMapImpl<L, std::make_index_sequence<L::Size>>::Type;

template <typename L, size_t Index, typename Default=void>
struct GetImpl;

template <template <typename...> class L, typename... Args, size_t Index, typename Default>
struct GetImpl<L<Args...>, Index, Default> {
    using M = ToMap<L<Args...>>;
    using Type = TypeTypeMap::Get<M, std::integral_constant<size_t, Index>, Default>;
};

template <typename L, size_t Index, typename Default=void>
using Get = typename GetImpl<L, Index, Default>::Type;

template <typename L, ssize_t Index, typename T, typename IndexList>
struct SetImpl;

template <template <typename...> class L, typename... Args, ssize_t Index, typename T, size_t... I>
struct SetImpl<L<Args...>, Index, T, std::index_sequence<I...>> {
    template <ssize_t J>
    struct Getter {
        using Type = Get<L<Args...>, J>;
    };

    template <>
    struct Getter<Index> {
        using Type = T;
    };

    using Type = L<typename Getter<I>::Type...>;
};

template <typename L, ssize_t Index, typename T>
using Set = typename SetImpl<L, Index, T, std::make_index_sequence<L::Size>>::Type;

template <typename L, typename Index, size_t I>
struct RemoveIndexImpl;

template <template <typename...> class L, typename... Args, size_t... I, size_t J>
struct RemoveIndexImpl<L<Args...>, std::index_sequence<I...>, J> {
    static constexpr size_t new_index(size_t index) {
        if (index < J) {
            return index;
        } else {
            return index + 1;
        }
    }
    using Type = L<TypeList::Get<L<Args...>, new_index(I)>...>;
};

template <typename L, size_t I>
struct RemoveImpl;

template <template <typename...> class L, typename... Args, size_t I>
struct RemoveImpl<L<Args...>, I> {
    using Index = std::make_index_sequence<(sizeof...(Args) - 1 > 0) ? (sizeof...(Args) - 1) : 0>;
    using Type = typename RemoveIndexImpl<L<Args...>, Index, I>::Type;
};

template <typename L, size_t I>
using Remove = typename RemoveImpl<L, I>::Type;

template <typename L, size_t N, typename Index>
struct TailImpl;

template <template <typename...> class L, typename... Args, size_t N, size_t... I>
struct TailImpl<L<Args...>, N, std::index_sequence<I...>> {
    using Type = L<Get<L<Args...>, I + N>...>;
};

template <typename L, size_t N = 1>
using Tail = typename TailImpl<L, N, std::make_index_sequence<(L::Size - N > 0) ? (L::Size - N) : 0>>::Type;
}

namespace TypeTypeMap {
template <typename M, typename Key, typename Value, ssize_t Index>
struct SetImpl {
    using Type = TypeList::Set<M, Index, TypeTypePair<Key, Value>>;
};

template <typename M, typename Key, typename Value>
struct SetImpl<M, Key, Value, -1> {
    using Type = TypeList::Append<M, TypeTypePair<Key, Value>>;
};

template <typename M, typename Key, typename Value>
using Set = typename SetImpl<M, Key, Value, Find<M, Key>::Value>::Type;

template <typename M, template <typename...> class F>
struct TransformImpl;

template <template <typename...> class M, typename... Args, template <typename...> class F>
struct TransformImpl<M<Args...>, F> {
    using Type = M<TypeTypePair<typename Args::First, F<typename Args::First, typename Args::Second>>...>;
};

template <typename M, template <typename...> class F>
using Transform = typename TransformImpl<M, F>::Type;

template <typename M>
struct KeysImpl;

template <template <typename...> class M, typename... Args>
struct KeysImpl<M<Args...>> {
    using Type = TypeList::List<typename Args::First...>;
};

template <typename M>
using Keys = typename KeysImpl<M>::Type;
}

namespace TypeSet {
template <typename... T>
struct Set {
    static constexpr size_t Size = sizeof...(T);
};

template <typename S, typename T>
struct Contains;

template <template <typename...> class S, typename... Args, typename T>
struct Contains<S<Args...>, T> {
    using U = Inherit<Identity<Args>...>;
    static constexpr bool Value = std::is_base_of<Identity<T>, U>::value;
};

template <typename S, typename T, bool HasInserted>
struct InsertImpl {
    using Type = S;
};

template <template <typename...> class S, typename... Args, typename T>
struct InsertImpl<S<Args...>, T, false> {
    using Type = S<Args..., T>;
};

template <typename S, typename T>
using Insert = typename InsertImpl<S, T, Contains<S, T>::Value>::Type;

template <typename S1, typename S2>
struct UnionImpl;

template <template <typename...> class S, typename... Args>
struct UnionImpl<S<Args...>, Set<>> {
    using Type = S<Args...>;
};

template <template <typename...> class S, typename... Args1, typename T, typename... Args2>
struct UnionImpl<S<Args1...>, S<T, Args2...>> {
    using Type = typename UnionImpl<Insert<S<Args1...>, T>, S<Args2...>>::Type;
};

template <typename S1, typename S2>
using Union = typename UnionImpl<S1, S2>::Type;

template <typename S, typename T, typename Index>
struct FindImpl;

template <template <typename...> class S, typename... Args, typename T, size_t... I>
struct FindImpl<S<Args...>, T, std::index_sequence<I...>> {
    using U = Inherit<TypeTypePair<Args, std::integral_constant<size_t, I>>...>;
    template <size_t J>
    static constexpr ssize_t f(TypeTypePair<T, std::integral_constant<size_t, J>> *) {
        return J;
    }
    static constexpr ssize_t f(...) {
        return -1;
    }
    static constexpr ssize_t Value = f((U *)0);
};

template <typename S, typename T>
struct Find;
template <template <typename...> class S, typename... Args, typename T>
struct Find<S<Args...>, T> {
    static constexpr ssize_t Value = FindImpl<S<Args...>, T, std::make_index_sequence<sizeof...(Args)>>::Value;
};

template <typename S, ssize_t I>
struct RemoveImpl {
    using Type = TypeList::Remove<S, I>;
};

template <typename S>
struct RemoveImpl<S, -1> {
    using Type = S;
};

template <typename S, typename T>
using Remove = typename RemoveImpl<S, Find<S, T>::Value>::Type;

template <typename S1, typename S2>
struct DiffImpl;

template <template <typename...> class S, typename... Args1, typename T, typename... Args2>
struct DiffImpl<S<Args1...>, S<T, Args2...>> {
    using Type = typename DiffImpl<Remove<S<Args1...>, T>, S<Args2...>>::Type;
};

template <template <typename...> class S, typename... Args>
struct DiffImpl<S<Args...>, S<>> {
    using Type = S<Args...>;
};

template <typename S1, typename S2>
using Diff = typename DiffImpl<S1, S2>::Type;

template <typename L>
struct FromListImpl;

template <template <typename...> class L, typename T, typename... Args>
struct FromListImpl<L<T, Args...>> {
    using Type = Insert<typename FromListImpl<L<Args...>>::Type, T>;
};

template <template <typename...> class L>
struct FromListImpl<L<>> {
    using Type = Set<>;
};

template <typename L>
using FromList = typename FromListImpl<L>::Type;

template <typename S>
struct ToListImpl;

template <template <typename...> class S, typename... Args>
struct ToListImpl<S<Args...>> {
    using Type = TypeList::List<Args...>;
};

template <typename S>
using ToList = typename ToListImpl<S>::Type;
}

namespace IntSet {
template <int... V>
struct Set {
    static constexpr size_t Size = sizeof...(V);
    static constexpr ConstexprArray<int, sizeof...(V)> Array = ConstexprArray<int, sizeof...(V)>(V...);
};

template <int... V>
constexpr ConstexprArray<int, sizeof...(V)> Set<V...>::Array;

template <typename T>
constexpr ssize_t find(T arr, int v) {
    for (size_t i = 0; i < arr.size(); ++i) {
        if (arr[i] == v) {
            return i;
        } 
    }
    return -1;
}

template <typename S, int V>
struct Find {
    static constexpr ssize_t Value = find(S::Array, V);
};

template <typename S, int V>
struct Contains {
    static constexpr bool Value = find(S::Array, V) != -1;
};

template <typename S, int V, typename Index, bool IsInSet>
struct InsertImpl {
    using Type = S;
};

template <typename S, int V, size_t... I>
struct InsertImpl<S, V, std::index_sequence<I...>, false> {
    static constexpr ConstexprArray<int, S::Size + 1> get_new_array(ConstexprArray<int, S::Size> arr) {
        ConstexprArray<int, S::Size + 1> new_arr = {0};
        size_t i = 0;
        for (; i < S::Size && arr[i] < V; ++i) {
            new_arr[i] = arr[i];
        }
        new_arr[i] = V;
        for (++i; i < S::Size + 1; ++i) {
            new_arr[i] = arr[i - 1];
        }
        return new_arr;
    }
    static constexpr ConstexprArray<int, S::Size + 1> Array = get_new_array(S::Array);
    using Type = Set<Array[I]...>;
};

template <typename S, int V>
using Insert = typename InsertImpl<S, V, std::make_index_sequence<S::Size + 1>, Contains<S, V>::Value>::Type;

template <typename S, int V, typename Index, ssize_t I>
struct RemoveIndexImpl;

template <typename S, int V, size_t... I>
struct RemoveIndexImpl<S, V, std::index_sequence<I...>, -1> {
    using Type = S;
};

template <typename S, int V, size_t... I, ssize_t J>
struct RemoveIndexImpl<S, V, std::index_sequence<I...>, J> {
    static constexpr size_t get_index(size_t i) {
        if (i < J) {
            return i;
        }
        return i + 1;
    }
    using Type = Set<S::Array[get_index(I)]...>;
};

template <typename S, int V>
struct RemoveImpl {
    using Type = typename RemoveIndexImpl<S, V, std::make_index_sequence<S::Size - 1>, Find<S, V>::Value>::Type;
};

template <int V>
struct RemoveImpl<IntSet::Set<>, V> {
    using Type = IntSet::Set<>;
};

template <typename S, int V>
using Remove = typename RemoveImpl<S, V>::Type;

template <typename... S>
struct UnionImpl;

template <typename Head>
struct UnionImpl<Head> {
    using Type = Head;
};

template <typename Head, typename... Rest>
struct UnionImpl<Head, Rest...> {
    using Type = typename UnionImpl<Head, typename UnionImpl<Rest...>::Type>::Type;
};

template <typename S1, typename S2>
struct UnionImpl<S1, S2> {
    static constexpr std::pair<size_t, ConstexprArray<int, S1::Size + S2::Size>> get_new_array(
        ConstexprArray<int, S1::Size> arr1, ConstexprArray<int, S2::Size> arr2
    ) {
        size_t size = 0;
        size_t i = 0;
        size_t j = 0;
        ConstexprArray<int, S1::Size + S2::Size> res;
        int last = 0;
        while (i < S1::Size && j < S2::Size) {
            if (arr1[i] < arr2[j]) {
                if (last != arr1[i] || size == 0) {
                    res[size] = arr1[i];
                    last = arr1[i];
                    ++size;
                }
                ++i;
            } else {
                if (last != arr2[j] || size == 0) {
                    res[size] = arr2[j];
                    last = arr2[j];
                    ++size;
                }
                ++j;
            }
        }
        while (i < S1::Size) {
            if (last != arr1[i] || size == 0) {
                res[size] = arr1[i];
                last = arr1[i];
                ++size;
            }
            ++i;
        }
        while (j < S2::Size) {
            if (last != arr2[j] || size == 0) {
                res[size] = arr2[j];
                last = arr2[j];
                ++size;
            }
            ++j;
        }
        return std::make_pair(size, res);
    }
    static constexpr std::pair<size_t, ConstexprArray<int, S1::Size + S2::Size>> SizeArrayPair = get_new_array(S1::Array, S2::Array);
    template <typename Index>
    struct ArrayToSet;

    template <size_t... I>
    struct ArrayToSet<std::index_sequence<I...>> {
        using Type = Set<SizeArrayPair.second[I]...>;
    };
    using Type = typename ArrayToSet<std::make_index_sequence<SizeArrayPair.first>>::Type;
};

template <typename... S>
using Union = typename UnionImpl<S...>::Type;

template <typename S1, typename S2>
struct DiffImpl {
    static constexpr std::pair<size_t, ConstexprArray<int, S1::Size>> get_new_array(
        ConstexprArray<int, S1::Size> arr1, ConstexprArray<int, S2::Size> arr2
    ) {
        size_t size = 0;
        ConstexprArray<int, S1::Size> res;
        size_t i = 0;
        size_t j = 0;
        for (; i < S1::Size && j < S2::Size; ) {
            if (arr1[i] < arr2[j]) {
                res[size] = arr1[i];
                ++size;
                ++i;
            } else if (arr1[i] == arr2[j]) {
                ++i;
                ++j;
            } else {
                ++j;
            }
        }
        for (; i < S1::Size; ++i) {
            res[size] = arr1[i];
            ++size;
        }
        return std::make_pair(size, res);
    }
    static constexpr std::pair<size_t, ConstexprArray<int, S1::Size>> SizeArrayPair = get_new_array(S1::Array, S2::Array);
    template <typename Index>
    struct ArrayToSet;

    template <size_t... I>
    struct ArrayToSet<std::index_sequence<I...>> {
        using Type = Set<SizeArrayPair.second[I]...>;
    };
    using Type = typename ArrayToSet<std::make_index_sequence<SizeArrayPair.first>>::Type;
};

template <typename S1, typename S2>
using Diff = typename DiffImpl<S1, S2>::Type;

template <typename T, size_t N, size_t I>
struct FromArrayImpl {
    using Type = Insert<typename FromArrayImpl<T, N, I + 1>::Type, T::Array[I]>;
};

template <typename T, size_t N>
struct FromArrayImpl<T, N, N> {
    using Type = Set<>;
};

template <typename T>
using FromArray = typename FromArrayImpl<T, T::Array.size(), 0>::Type;

template <typename S, int V>
struct AddImpl;

template <int... Args, int V>
struct AddImpl<Set<Args...>, V> {
    using Type = IntSet::Set<(Args + V)...>;
};

template <typename S, int V>
using Add = typename AddImpl<S, V>::Type;

template <typename S, template <int, typename> class F, typename Z>
struct ReduceImpl;

template <template <int, typename> class F, int V, int... Args, typename Z>
struct ReduceImpl<Set<V, Args...>, F, Z> {
    using Type = F<V, typename ReduceImpl<Set<Args...>, F, Z>::Type>;
};

template <template <int, typename> class F, typename Z>
struct ReduceImpl<Set<>, F, Z> {
    using Type = Z;
};

template <typename S, template <int, typename> class F, typename Z>
using Reduce = typename ReduceImpl<S, F, Z>::Type;

template <int From, typename Index>
struct RangeImpl;

template <int From, size_t... I>
struct RangeImpl<From, std::index_sequence<I...>> {
    using Type = Set<(I + From)...>;
};

template <int From, int To>
using Range = typename RangeImpl<From, std::make_index_sequence<To - From + 1>>::Type;
}
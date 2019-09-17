#pragma once
#include <type_traits>

namespace static_regex {
template <int n>
struct int_wrapper {
    static constexpr int value = n;
};

template <bool first, typename t1, typename t2>
struct select_type_impl {
    using type = t1;
};

template <typename t1, typename t2>
struct select_type_impl<false, t1, t2> {
    using type = t2;
};

template <bool first, typename t1, typename t2>
using select_type = typename select_type_impl<first, t1, t2>::type;

typedef void type_list_nil;

template <typename head_, typename rest_>
struct type_list {
    using head = head_;
    using rest = rest_;
};

template <typename... args>
struct type_list_new_impl {};

template <typename head, typename... rest>
struct type_list_new_impl<head, rest...> {
    using type = type_list<head, typename type_list_new_impl<rest...>::type>;
};

template <typename head>
struct type_list_new_impl<head> {
    using type = type_list<head, type_list_nil>;
};

template <>
struct type_list_new_impl<> {
    using type = type_list_nil;
};

template <typename... args>
using type_list_new = typename type_list_new_impl<args...>::type;

template <typename list_, int n>
struct type_list_get_impl {
    using type = typename type_list_get_impl<typename list_::rest, n - 1>::type;
};

template <typename list_>
struct type_list_get_impl<list_, 0> {
    using type = typename list_::head;
};

template <int n>
struct type_list_get_impl<type_list_nil, n> {
    using type = void;
};

template <>
struct type_list_get_impl<type_list_nil, 0> {
    using type = void;
};

template <typename list_, int n>
using type_list_get = typename type_list_get_impl<list_, n>::type;

template <typename head, typename... rest>
struct type_list_merge_impl {
    using type = type_list<
        typename head::head,
        typename type_list_merge_impl<typename head::rest, rest...>::type
    >;
};

template <typename... args>
struct type_list_merge_impl<type_list_nil, args...> {
    using type = typename type_list_merge_impl<args...>::type;
};

template <>
struct type_list_merge_impl<type_list_nil> {
    using type = type_list_nil;
};

template <typename head, typename... rest>
using type_list_merge = typename type_list_merge_impl<head, rest...>::type;

template <typename list_, typename t>
struct type_list_push_impl {
    using type = type_list<
        typename list_::head,
        typename type_list_push_impl<typename list_::rest, t>::type
    >;
};

template <typename t>
struct type_list_push_impl<type_list_nil, t> {
    using type = type_list<t, type_list_nil>;
};

template <typename list_, typename t>
using type_list_push = typename type_list_push_impl<list_, t>::type;

template <typename list_, typename t>
struct type_list_search {
    static constexpr int value = std::is_same<typename list_::head, t>::value ? 0 :
        (type_list_search<typename list_::rest, t>::value == -1 ?
             -1 : type_list_search<typename list_::rest, t>::value + 1);
};

template <typename t>
struct type_list_search<type_list_nil, t> {
    static constexpr int value = -1;
};

template <typename list_, typename t>
struct type_list_push_if_not_exist_impl {
    using type = select_type<
        std::is_same<typename list_::head, t>::value,
        list_,
        typename type_list_push_if_not_exist_impl<typename list_::rest, t>::type
    >;
};

template <typename t>
struct type_list_push_if_not_exist_impl<type_list_nil, t> {
    using type = type_list<t, type_list_nil>;
};

template <typename list_, typename t>
using type_list_push_if_not_exist = typename type_list_push_if_not_exist_impl<list_, t>::type;

template <typename l1, typename l2>
struct type_list_diff_impl {
    using type = select_type<type_list_search<l1, typename l2::head>::value != -1,
        typename type_list_diff_impl<l1, typename l2::rest>::type,
        type_list<typename l2::head, typename type_list_diff_impl<l1, typename l2::rest>::type>
    >;
};

template <typename l1>
struct type_list_diff_impl<l1, type_list_nil> {
    using type = type_list_nil;
};

template <typename l1, typename l2>
using type_list_diff = typename type_list_diff_impl<l1, l2>::type;

template <typename l1, typename l2>
using type_list_merge_if_not_exist = type_list_merge<
    l1,
    type_list_diff<l1, l2>
>;

template <typename list_>
struct type_list_size {
    static constexpr int value = type_list_size<typename list_::rest>::value + 1;
};

template <>
struct type_list_size<type_list_nil> {
    static constexpr int value = 0;
};

template <typename list_, typename t>
struct type_list_count {
    static constexpr int value = std::is_same<typename list_::head, t>::value ? 1 : 0 +
        type_list_count<typename list_::rest, t>::value;
};

template <typename t>
struct type_list_count<type_list_nil, t> {
    static constexpr int value = 0;
};

template <bool is_same_size, typename l1, typename l2, int i>
struct is_type_list_equal_;

template <typename l1, typename l2, int i>
struct is_type_list_equal_<true, l1, l2, i> {
    static constexpr bool value = 
        type_list_count<l1,
            type_list_get<l2, i>
        >::value == type_list_count<l2,
            type_list_get<l2, i>
        >::value &&
        is_type_list_equal_<true, l1, l2, i - 1>::value;
};

template <typename l1, typename l2>
struct is_type_list_equal_<true, l1, l2, -1> {
    static constexpr bool value = true;
};

template <typename l1, typename l2, int i>
struct is_type_list_equal_<false, l1, l2, i> {
    static constexpr bool value = false;
};

template <typename l1, typename l2>
struct is_type_list_equal {
    static constexpr bool value = is_type_list_equal_<
        type_list_size<l1>::value == type_list_size<l2>::value,
        l1, l2, type_list_size<l2>::value - 1
    >::value;
};

template <typename first_, typename second_>
struct type_pair {
    using first = first_;
    using second = second_;
};

template <typename map_, typename key>
struct type_map_get_impl {
    using type = select_type<std::is_same<typename map_::head::first, key>::value,
        typename map_::head::second,
        typename type_map_get_impl<typename map_::rest, key>::type
    >;
};

template <typename key>
struct type_map_get_impl<type_list_nil, key> {
    using type = void;
};

template <typename map_, typename key>
using type_map_get = typename type_map_get_impl<map_, key>::type;

template <typename map_, typename key, typename value>
struct type_map_set_impl {
    using type = select_type<std::is_same<typename map_::head::first, key>::value,
        type_list<
            type_pair<typename map_::head::first, value>,
            typename map_::rest
        >,
        type_list<
            typename map_::head,
            typename type_map_set_impl<typename map_::rest, key, value>::type
        >
    >;
};

template <typename key, typename value>
struct type_map_set_impl<type_list_nil, key, value> {
    using type = type_list<
        type_pair<key, value>,
        type_list_nil
    >;
};

template <typename map_, typename key, typename value>
using type_map_set = typename type_map_set_impl<map_, key, value>::type;

template <typename map_>
struct type_map_key_list_impl {
    using type = type_list<typename map_::head::first,
        typename type_map_key_list_impl<typename map_::rest>::type
    >;
};

template <>
struct type_map_key_list_impl<type_list_nil> {
    using type = type_list_nil;
};

template <typename map_>
using type_map_key_list = typename type_map_key_list_impl<map_>::type;

template <typename map_>
struct type_map_value_list_impl {
    using type = type_list<
        typename map_::head::second,
        typename type_map_value_list_impl<typename map_::rest>::type
    >;
};

template <>
struct type_map_value_list_impl<type_list_nil> {
    using type = type_list_nil;
};

template <typename map_>
using type_map_value_list = typename type_map_value_list_impl<map_>::type;

template <typename map_, typename value_>
struct type_map_search_value_impl {
    using type = select_type<
        std::is_same<typename map_::head::second, value_>::value,
        type_list<
            typename map_::head::first,
            typename type_map_search_value_impl<typename map_::rest, value_>::type
        >,
        typename type_map_search_value_impl<typename map_::rest, value_>::type
    >;
};

template <typename value_>
struct type_map_search_value_impl<type_list_nil, value_> {
    using type = type_list_nil;
};

template <typename map_, typename value_>
using type_map_search_value = typename type_map_search_value_impl<map_, value_>::type;

template <int head_, typename rest_>
struct int_set {
    static constexpr int head = head_;
    using rest = rest_;
};

//struct int_set_nil {};
typedef void int_set_nil;

template <int... args>
struct int_set_new_impl {};

template <int head, int... rest>
struct int_set_new_impl<head, rest...> {
    using type = int_set<head, typename int_set_new_impl<rest...>::type>;
};

template <int head>
struct int_set_new_impl<head> {
    using type = int_set<head, int_set_nil>;
};

template <>
struct int_set_new_impl<> {
    using type = int_set_nil;
};

template <int... args>
using int_set_new = typename int_set_new_impl<args...>::type;

template <typename set_, int n>
struct int_set_get {
    static constexpr int value = int_set_get<typename set_::rest, n - 1>::value;
};

template <typename set_>
struct int_set_get<set_, 0> {
    static constexpr int value = set_::head;
};

template <typename set_, int value>
struct int_set_insert_impl {
    using type = select_type<(value == set_::head),
        set_,
        select_type<(value < set_::head),
            int_set<value, set_>,
            int_set<set_::head, typename int_set_insert_impl<typename set_::rest, value>::type>
        >
    >;
};

template <int value>
struct int_set_insert_impl<int_set_nil, value> {
    using type = int_set<value, int_set_nil>;
};

template <typename set_, int value>
using int_set_insert = typename int_set_insert_impl<set_, value>::type;

template <typename set_, int value>
struct int_set_remove_impl {
    using type = select_type<(value == set_::head),
        typename set_::rest,
        int_set<set_::head, typename int_set_remove_impl<typename set_::rest, value>::type>
    >;
};

template <int value>
struct int_set_remove_impl<int_set_nil, value> {
    using type = int_set_nil;
};

template <typename set_, int value>
using int_set_remove = typename int_set_remove_impl<set_, value>::type;

template <typename set_, int n>
struct int_set_add_impl {
    using type = int_set<set_::head + n, typename int_set_add_impl<typename set_::rest, n>::type>;
};

template <int n>
struct int_set_add_impl<int_set_nil, n> {
    using type = int_set_nil;
};

template <typename set_, int n>
using int_set_add = typename int_set_add_impl<set_, n>::type;

template <typename head, typename... rest>
struct int_set_union_impl {
    using type = typename int_set_union_impl<
        head,
        typename int_set_union_impl<rest...>::type
    >::type;
};

template <typename set1, typename set2>
struct int_set_union_impl<set1, set2> {
    using type = int_set_insert<
        typename int_set_union_impl<set1, typename set2::rest>::type,
        set2::head
    >;
};

template <typename set_>
struct int_set_union_impl<set_, int_set_nil> {
    using type = set_;
};

template <typename set_>
struct int_set_union_impl<int_set_nil, set_> {
    using type = set_;
};

template <>
struct int_set_union_impl<int_set_nil, int_set_nil> {
    using type = int_set_nil;
};

template <typename head, typename... rest>
using int_set_union = typename int_set_union_impl<head, rest...>::type;

template <typename set_, int v>
struct int_set_has {
    static constexpr bool value = (v == set_::head) ||
        int_set_has<typename set_::rest, v>::value;
};

template <int v>
struct int_set_has<int_set_nil, v> {
    static constexpr bool value = false;
};

template <typename set1, typename set2>
struct int_set_diff_impl {
    using type = select_type<
        int_set_has<set1, set2::head>::value,
        typename int_set_diff_impl<set1, typename set2::rest>::type,
        int_set_insert<
            typename int_set_diff_impl<set1, typename set2::rest>::type,
            set2::head
        >
    >;
};

template <typename set1>
struct int_set_diff_impl<set1, int_set_nil> {
    using type = int_set_nil;
};

template <typename set1, typename set2>
using int_set_diff = typename int_set_diff_impl<set1, set2>::type;

template <int n, int i>
struct int_set_range_impl {
    using type = int_set<
        n - i,
        typename int_set_range_impl<n, i - 1>::type
    >;
};

template <int n>
struct int_set_range_impl<n, 0> {
    using type = int_set_nil;
};

template <int n>
using int_set_range = typename int_set_range_impl<n, n>::type;

template <typename set_, int n>
struct int_set_search {
    static constexpr int value = set_::head == n ? 0 :
        (int_set_search<typename set_::rest, n>::value == -1 ?
             -1 : int_set_search<typename set_::rest, n>::value + 1);
};

template <int n>
struct int_set_search<int_set_nil, n> {
    static constexpr int value = -1;
};

template <typename set_, int from, int to>
using int_set_replace = select_type<
    int_set_has<set_, from>::value,
    int_set_insert<
        int_set_remove<set_, from>,
        to
    >,
    set_
>;
}
#pragma once
#include <type_traits>

namespace static_regex {
template <int n>
struct int_wrapper {
    static constexpr int value = n;
};

template <bool first, typename t1, typename t2>
struct select_type {
    typedef t1 type;
};

template <typename t1, typename t2>
struct select_type<false, t1, t2> {
    typedef t2 type;
};

typedef void type_list_nil;

template <typename head_, typename rest_>
struct type_list {
    typedef head_ head;
    typedef rest_ rest;
};

template <typename... args>
struct type_list_new {};

template <typename head, typename... rest>
struct type_list_new<head, rest...> {
    typedef type_list<head, typename type_list_new<rest...>::type> type;
};

template <typename head>
struct type_list_new<head> {
    typedef type_list<head, type_list_nil> type;
};

template <>
struct type_list_new<> {
    typedef type_list_nil type;
};

template <typename list_, int n>
struct type_list_get {
    typedef typename type_list_get<typename list_::rest, n - 1>::type type;
};

template <typename list_>
struct type_list_get<list_, 0> {
    typedef typename list_::head type;
};

template <int n>
struct type_list_get<type_list_nil, n> {
    typedef void type;
};

template <>
struct type_list_get<type_list_nil, 0> {
    typedef void type;
};

template <typename head, typename... rest>
struct type_list_merge {
    typedef type_list<typename head::head,
        typename type_list_merge<typename head::rest, rest...>::type> type;
};

template <typename... args>
struct type_list_merge<type_list_nil, args...> {
    typedef typename type_list_merge<args...>::type type;
};

template <>
struct type_list_merge<type_list_nil> {
    typedef type_list_nil type;
};

template <typename list_, typename t>
struct type_list_push {
    typedef type_list<typename list_::head,
        typename type_list_push<typename list_::rest, t>::value> type;
};

template <typename t>
struct type_list_push<type_list_nil, t> {
    typedef type_list<t, type_list_nil> type;
};

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
struct type_list_push_if_not_exist {
    typedef typename select_type<
        std::is_same<typename list_::head, t>::value,
        list_,
        typename type_list_push_if_not_exist<typename list_::rest, t>::type
    >::type type;
};

template <typename t>
struct type_list_push_if_not_exist<type_list_nil, t> {
    typedef type_list<t, type_list_nil> type;
};

template <typename l1, typename l2>
struct type_list_diff {
    typedef typename select_type<type_list_search<l1, typename l2::head>::value != -1,
        typename type_list_diff<l1, typename l2::rest>::type,
        type_list<typename l2::head, typename type_list_diff<l1, typename l2::rest>::type>
    >::type type;
};

template <typename l1>
struct type_list_diff<l1, type_list_nil> {
    typedef type_list_nil type;
};


template <typename l1, typename l2>
struct type_list_merge_if_not_exist {
    typedef typename type_list_merge<
        l1,
        typename type_list_diff<l1, l2>::type
    >::type type;
};

template <typename list_>
struct type_list_size {
    static constexpr int value = type_list_size<typename list_::rest>::value + 1;
};

template <>
struct type_list_size<type_list_nil> {
    static constexpr int value = 0;
};

template <typename l1, typename l2>
struct is_type_list_equal_ {
    static constexpr bool value = type_list_search<l1, typename l2::head>::value != -1 &&
        is_type_list_equal_<l1, typename l2::rest>::value;
};

template <typename l1>
struct is_type_list_equal_<l1, type_list_nil> {
    static constexpr bool value = true;
};

template <typename l1, typename l2>
struct is_type_list_equal {
    static constexpr bool value = type_list_size<l1>::value == type_list_size<l2>::value &&
        is_type_list_equal_<l1, l2>::value;
};

template <typename first_, typename second_>
struct type_pair {
    typedef first_ first;
    typedef second_ second;
};

template <typename map_, typename key>
struct type_map_get {
    typedef typename select_type<std::is_same<typename map_::head::first, key>::value,
        typename map_::head::second,
        typename type_map_get<typename map_::rest, key>::type
    >::type type;
};

template <typename map_, typename key, typename value>
struct type_map_set {
    typedef typename select_type<std::is_same<typename map_::head::first, key>::value,
        type_list<
            type_pair<typename map_::head::first, value>,
            typename map_::rest
        >,
        type_list<
            typename map_::head,
            typename type_map_set<typename map_::rest, key, value>::type
        >
    >::type type;
};

template <typename key, typename value>
struct type_map_set<type_list_nil, key, value> {
    typedef type_list<
        type_pair<key, value>,
        type_list_nil
    > type;
};

template <typename map_>
struct type_map_key_list {
    typedef type_list<typename map_::head::first,
        typename type_map_key_list<typename map_::rest>::type
    > type;
};

template <>
struct type_map_key_list<type_list_nil> {
    typedef type_list_nil type;
};

template <typename map_>
struct type_map_value_list {
    typedef type_list<
        typename map_::head::second,
        typename type_map_value_list<typename map_::rest>::type
    > type;
};

template <>
struct type_map_value_list<type_list_nil> {
    typedef type_list_nil type;
};

template <typename key>
struct type_map_get<type_list_nil, key> {
    typedef void type;
};

template <typename map_, typename value_>
struct type_map_search_value {
    typedef typename select_type<
        std::is_same<typename map_::head::second, value_>::value,
        type_list<
            typename map_::head::first,
            typename type_map_search_value<typename map_::rest, value_>::type
        >,
        typename type_map_search_value<typename map_::rest, value_>::type
    >::type type;
};

template <typename value_>
struct type_map_search_value<type_list_nil, value_> {
    typedef type_list_nil type;
};

template <int head_, typename rest_>
struct int_set {
    static constexpr int head = head_;
    typedef rest_ rest;
};

//struct int_set_nil {};
typedef void int_set_nil;

template <int... args>
struct int_set_new {};

template <int head, int... rest>
struct int_set_new<head, rest...> {
    typedef int_set<head, typename int_set_new<rest...>::type> type;
};

template <int head>
struct int_set_new<head> {
    typedef int_set<head, int_set_nil> type;
};

template <>
struct int_set_new<> {
    typedef int_set_nil type;
};

template <typename set_, int n>
struct int_set_get {
    static constexpr int value = int_set_get<typename set_::rest, n - 1>::value;
};

template <typename set_>
struct int_set_get<set_, 0> {
    static constexpr int value = set_::head;
};

template <typename set_, int value>
struct int_set_insert {
    typedef typename select_type<(value == set_::head),
        set_,
        typename select_type<(value < set_::head),
            int_set<value, set_>,
            int_set<set_::head, typename int_set_insert<typename set_::rest, value>::type>
        >::type
    >::type type;
};

template <int value>
struct int_set_insert<int_set_nil, value> {
    typedef int_set<value, int_set_nil> type;
};

template <typename set_, int value>
struct int_set_remove {
    typedef typename select_type<(value == set_::head),
        typename set_::rest,
        int_set<set_::head, typename int_set_remove<typename set_::rest, value>::type>
    >::type type;
};

template <int value>
struct int_set_remove<int_set_nil, value> {
    typedef int_set_nil type;
};

template <typename set_, int n>
struct int_set_add {
    typedef int_set<set_::head + n, typename int_set_add<typename set_::rest, n>::type> type;
};

template <int n>
struct int_set_add<int_set_nil, n> {
    typedef int_set_nil type;
};

template <typename head, typename... rest>
struct int_set_union {
    typedef typename int_set_union<
        head,
        typename int_set_union<rest...>::type
    >::type type;
};

template <typename set1, typename set2>
struct int_set_union<set1, set2> {
    typedef typename int_set_insert<
        typename int_set_union<set1, typename set2::rest>::type,
        set2::head
    >::type type;
};

template <typename set_>
struct int_set_union<set_, int_set_nil> {
    typedef set_ type;
};

template <typename set_>
struct int_set_union<int_set_nil, set_> {
    typedef set_ type;
};

template <>
struct int_set_union<int_set_nil, int_set_nil> {
    typedef int_set_nil type;
};

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
struct int_set_diff {
    typedef typename select_type<
        int_set_has<set1, set2::head>::value,
        typename int_set_diff<set1, typename set2::rest>::type,
        typename int_set_insert<
            typename int_set_diff<set1, typename set2::rest>::type,
            set2::head
        >::type
    >::type type;
};

template <typename set1>
struct int_set_diff<set1, int_set_nil> {
    typedef int_set_nil type;
};

template <int n, int i>
struct int_set_range_ {
    typedef int_set<
        n - i,
        typename int_set_range_<n, i - 1>::type
    > type;
};

template <int n>
struct int_set_range_<n, 0> {
    typedef int_set_nil type;
};

template <int n>
struct int_set_range {
    typedef typename int_set_range_<n, n>::type type;
};

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
}
#pragma once
#include <vector>
#include <string>
#include <utility>
#include "util.hpp"

namespace static_regex {

template <int q_, int f_, typename transition_>
struct nfa {
    static constexpr int q = q_;
    static constexpr int f = f_;
    static constexpr int states = f - q + 1;
    typedef transition_ transition;
};

template <int c>
struct symbol_condition {
    inline static bool condition(int input) {
        return c == input;
    }
};

template <int c1, int c2>
struct range_condition {
    inline static bool condition(int input) {
        return c1 <= input && c2 >= input;
    }
};

struct epsilon {};

using empty_nfa = nfa<0, 1,
   type_list_new<
       type_list_new<
           type_pair<epsilon, int_set_new<1>>
       >
   >
 >;

template <typename transition, int n>
struct transition_item_add_impl {
    using type = type_list<
        type_pair<typename transition::head::first,
            int_set_add<typename transition::head::second, n>
        >,
        typename transition_item_add_impl<typename transition::rest, n>::type
    >;
};

template <int n>
struct transition_item_add_impl<type_list_nil, n> {
    using type = type_list_nil;
};

template <typename transition, int n>
using transition_item_add = typename transition_item_add_impl<transition, n>::type;

template <typename transition, int n>
struct transition_add_impl {
    using type = type_list<
        transition_item_add<typename transition::head, n>,
        typename transition_add_impl<typename transition::rest, n>::type
    >;
};

template <int n>
struct transition_add_impl<type_list_nil, n> {
    using type = type_list_nil;
};

template <typename transition, int n>
using transition_add = typename transition_add_impl<transition, n>::type;

template <typename condition>
using condition_nfa = nfa<0, 1,
    type_list_new<
        type_list_new<
            type_pair<condition, int_set_new<1>>
        >
    >
>;

template <typename n1, typename n2>
struct union_nfa_impl {
    static constexpr int q = 0;
    static constexpr int f = n1::states + n2::states + 1;
    static constexpr int n1_q = 1;
    static constexpr int n1_f = n1::f + 1;
    static constexpr int n2_q = n1::states + 1;
    static constexpr int n2_f = n1::states + n2::states;
    using type = nfa<q, f,
        type_list_merge<
            type_list_new<
                type_list_new<
                    type_pair<epsilon, int_set_new<n1_q, n2_q>>
                >
            >,
            transition_add<typename n1::transition, n1_q>,
            type_list_new<
                type_list_new<
                    type_pair<epsilon, int_set_new<f>>
                >
            >,
            transition_add<typename n2::transition, n2_q>,
            type_list_new<
                type_list_new<
                    type_pair<epsilon, int_set_new<f>>
                >
            >
        >
    >;
};

template <typename n1, typename n2>
using union_nfa = typename union_nfa_impl<n1, n2>::type;

template <typename n1, typename n2>
struct concat_nfa_impl {
    static constexpr int q = 0;
    static constexpr int f = n1::states + n2::states - 2;
    static constexpr int n1_q = 0;
    static constexpr int n2_q = n1::states - 1;
    using type = nfa<q, f,
        type_list_merge<
            transition_add<typename n1::transition, n1_q>,
            transition_add<typename n2::transition, n2_q>
        >
    >;
};

template <typename n1, typename n2>
using concat_nfa = typename concat_nfa_impl<n1, n2>::type;

template <typename n>
struct closure_nfa_impl {
    static constexpr int q = 0;
    static constexpr int f = n::states + 1;
    static constexpr int n_q = 1;
    static constexpr int n_f = n::states;
    using type = nfa<q, f,
        type_list_merge<
            type_list_new<
                type_list_new<
                    type_pair<epsilon, int_set_new<n_q, f>>
                >
            >,
            transition_add<typename n::transition, n_q>,
            type_list_new<
                type_list_new<
                    type_pair<epsilon, int_set_new<n_q, f>>
                >
            >
        >
    >;
};

template <typename n>
using closure_nfa = typename closure_nfa_impl<n>::type;

template <typename transition, typename set_>
struct epsilon_closure_set_impl {
    using head_closure = type_map_get<
        type_list_get<transition, set_::head>,
        epsilon
    >;
    using type = int_set_union<
        set_,
        head_closure,
        typename epsilon_closure_set_impl<
            transition,
            head_closure
        >::type,
        typename epsilon_closure_set_impl<transition, typename set_::rest>::type
    >;
};

template <typename transition>
struct epsilon_closure_set_impl<transition, int_set_nil> {
    using type = int_set_nil;
};

template <typename transition, typename set_>
using epsilon_closure_set = typename epsilon_closure_set_impl<transition, set_>::type;

template <typename transition, int n>
using epsilon_closure = epsilon_closure_set<
    transition,
    int_set_new<n>
>;

template <typename transition, typename state, typename condition>
struct next_dfa_state_impl {
    using type = int_set_union<
        typename next_dfa_state_impl<transition, typename state::rest, condition>::type,
        epsilon_closure_set<
            transition,
            type_map_get<
                type_list_get<transition, state::head>,
                condition
            >
        >
    >;
};

template <typename transition, typename condition>
struct next_dfa_state_impl<transition, int_set_nil, condition> {
    using type = int_set_nil;
};

template <typename transition, typename state, typename condition>
using next_dfa_state = typename next_dfa_state_impl<transition, state, condition>::type;

template <typename transition, typename state>
struct dfa_state_conditions_impl {
    using type = type_list_merge_if_not_exist<
        type_list_diff<
            type_list<
                epsilon,
                type_list_nil
            >,
            type_map_key_list<
                type_list_get<transition, state::head>
            >
        >,
        typename dfa_state_conditions_impl<transition, typename state::rest>::type
    >;
};

template <typename transition>
struct dfa_state_conditions_impl<transition, int_set_nil> {
    using type = type_list_nil;
};

template <typename transition, typename state>
using dfa_state_conditions = typename dfa_state_conditions_impl<transition, state>::type;

template <typename transition, typename state, typename condition>
struct dfa_condition_map_impl {
    using type = type_list<
        type_pair<typename condition::head,
            next_dfa_state<transition, state, typename condition::head>
        >,
        typename dfa_condition_map_impl<transition, state, typename condition::rest>::type
    >;
};

template <typename transition, typename state>
struct dfa_condition_map_impl<transition, state, type_list_nil> {
    using type = type_list_nil;
};

template <typename transition, typename state, typename condition>
using dfa_condition_map = typename dfa_condition_map_impl<transition, state, condition>::type;

template <typename transition, typename states>
struct dfa_transition_impl {
    using type = type_list<
        type_pair<
            typename states::head,
            dfa_condition_map<
                transition, typename states::head,
                dfa_state_conditions<transition, typename states::head>
            >
        >,
        typename dfa_transition_impl<transition, typename states::rest>::type
    >;
};

template <typename transition>
struct dfa_transition_impl<transition, type_list_nil> {
    using type = type_list_nil;
};

template <typename transition, typename states>
using dfa_transition = typename dfa_transition_impl<transition, states>::type;

template <typename transition>
struct dfa_new_states_impl {
    using type = type_list_merge<
        type_map_value_list<typename transition::head::second>,
        typename dfa_new_states_impl<typename transition::rest>::type
    >;
};

template<>
struct dfa_new_states_impl<type_list_nil> {
    using type = type_list_nil;
};

template <typename transition>
using dfa_new_states = typename dfa_new_states_impl<transition>::type;

template <typename m1, typename m2>
struct condition_map_merge_impl {
    using rest = typename condition_map_merge_impl<m1, typename m2::rest>::type;
    using v1 =  type_map_get<m1, typename m2::head::first>;
    using v =  int_set_union<v1, typename m2::head::second>;
    using type = type_map_set<rest, typename m2::head::first, v>;
};

template <typename m1>
struct condition_map_merge_impl<m1, type_list_nil> {
    using type = m1;
};

template <typename m1, typename m2>
using condition_map_merge = typename condition_map_merge_impl<m1, m2>::type;

template <typename t1, typename t2>
struct merge_dfa_transition_impl {
    using rest = typename merge_dfa_transition_impl<t1, typename t2::rest>::type;
    using m = type_map_get<t1, typename t2::head::first>;
    using merged = condition_map_merge<m, typename t2::head::second>;
    using type = type_map_set<
        rest,
        typename t2::head::first,
        merged
    >;
};

template <typename t>
struct merge_dfa_transition_impl<t, type_list_nil> {
    using type = t;
};

template <typename t1, typename t2>
using merge_dfa_transition = typename merge_dfa_transition_impl<t1, t2>::type;

template <typename states_, typename transition_>
struct nfa_to_dfa_loop_result {
    using states = states_;
    using transition = transition_;
};

template <bool has_next_, typename nfa, typename states, typename transition>
struct nfa_to_dfa_loop_impl {
    using new_transition = dfa_transition<typename nfa::transition, states>;
    using new_states = dfa_new_states<new_transition>;
    using next_states = type_list_merge_if_not_exist<states, new_states>;
    using next_transition = merge_dfa_transition<transition, new_transition>;
    static constexpr bool has_next = !std::is_same<states, next_states>::value;
    using type = typename nfa_to_dfa_loop_impl<
        has_next,
        nfa,
        next_states,
        next_transition
    >::type;
};

template <typename nfa, typename states, typename transition>
struct nfa_to_dfa_loop_impl<false, nfa, states, transition> {
    using type = nfa_to_dfa_loop_result<states, transition>;
};

template <typename nfa, typename states, typename transition>
using nfa_to_dfa_loop = typename nfa_to_dfa_loop_impl<true, nfa, states, transition>::type;

template <typename all_states, typename map_>
struct condition_map_int_impl {
    using type = type_list<
        type_pair<
            typename map_::head::first,
            int_wrapper<type_list_search<all_states, typename map_::head::second>::value>
        >,
        typename condition_map_int_impl<all_states, typename map_::rest>::type
    >;
};

template <typename all_states>
struct condition_map_int_impl<all_states, type_list_nil> {
    using type = type_list_nil;
};

template <typename all_states, typename map_>
using condition_map_int = typename condition_map_int_impl<all_states, map_>::type;

template <typename all_states, typename states, typename transition>
struct dfa_transition_int_impl {
    using type = type_list<
        condition_map_int<
            all_states,
            type_map_get<transition, typename states::head>
        >,
        typename dfa_transition_int_impl<all_states, typename states::rest, transition>::type
    >;
};

template <typename all_states, typename transition>
struct dfa_transition_int_impl<all_states, type_list_nil, transition> {
    using type = type_list_nil;
};

template <typename all_states, typename states, typename transition>
using dfa_transition_int = typename dfa_transition_int_impl<all_states, states, transition>::type;

template <typename all_states, typename states, int f>
struct dfa_final_states_impl {
    using type = select_type<
        int_set_has<typename states::head, f>::value,
        int_set_insert<
            typename dfa_final_states_impl<all_states, typename states::rest, f>::type,
            type_list_search<all_states, typename states::head>::value
        >,
        typename dfa_final_states_impl<all_states, typename states::rest, f>::type
    >;
};

template <typename all_states, int f>
struct dfa_final_states_impl<all_states, type_list_nil, f> {
    using type = int_set_nil;
};

template <typename all_states, typename states, int f>
using dfa_final_states = typename dfa_final_states_impl<all_states, states, f>::type;

template <int q_, typename f_, typename transition_>
struct dfa {
    static constexpr int q = q;
    using f = f_;
    using transition = transition_;
    using states = int_set_range<type_list_size<transition_>::value>;
    template <typename map_>
    inline static int find_state(int c, int default_state = -1) {
        if (map_::head::first::condition(c)) {
            return map_::head::second::value;
        } else {
            return find_state<typename map_::rest>(c, default_state);
        }
    }
    template <>
    inline static int find_state<type_list_nil>(int c, int default_state = -1) {
        return default_state;
    }
    template <typename transition_left>
    inline static int next_state(int state, int c, int default_state = -1) {
        if (state == 0) {
            return find_state<typename transition_left::head>(c, default_state);
        } else {
            return next_state<typename transition_left::rest>(state - 1, c, default_state);
        }
    }
    template <>
    inline static int next_state<type_list_nil>(int state, int c, int default_state = -1) {
        return default_state;
    }
    template <typename f_left>
    inline static bool is_final(int state) {
        return f_left::head == state ||
            is_final<typename f_left::rest>(state);
    }
    template <>
    inline static bool is_final<int_set_nil>(int state) {
        return false;
    }
};

template <typename nfa_>
struct nfa_to_dfa_impl {
    using result = nfa_to_dfa_loop<
        nfa_,
        type_list_new<
            epsilon_closure<typename nfa_::transition, nfa_::q>
        >,
        type_list_nil
    >;
    using transition = dfa_transition_int<
        typename result::states,
        typename result::states,
        typename result::transition
    >;
    using final_states = dfa_final_states<
        typename result::states,
        typename result::states,
        nfa_::f
    >;
    using type = dfa<0, final_states, transition>;
};

template <typename nfa_>
using nfa_to_dfa = typename nfa_to_dfa_impl<nfa_>::type;

template <char c>
struct character {
    static constexpr int tag_type = 0;
    static constexpr char value = c;
};

struct regex_begin {
    static constexpr int tag_type = 0;
    static constexpr int value = -1;
};

struct regex_end {
    static constexpr int tag_type = 0;
    static constexpr int value = -2;
};

template <char... args>
struct static_string;

template <char c>
struct static_string<c> {
    static constexpr int tag_type = 2;
    static constexpr char head = c;
    using rest = void;
};

template <char head_, char... rest_>
struct static_string<head_, rest_...> {
    static constexpr int tag_type = 1;
    static constexpr char head = head_;
    using rest = static_string<rest_...>;
};

template <typename r>
struct option {
    static constexpr int tag_type = 3;
    using type = r;
};

template <typename r, int times>
struct repeat {
    static constexpr int tag_type = 4;
    using head = r;
    using rest = repeat<r, times - 1>;
};

template <typename r>
struct repeat<r, 0> {
    static constexpr int tag_type = 5;
};

template <typename head, typename... rest>
struct select {
    static constexpr int tag_type = 6;
    using r1 = head;
    using r2 = select<rest...>;
};

template <typename r1_, typename r2_>
struct select<r1_, r2_> {
    static constexpr int tag_type = 6;
    using r1 = r1_;
    using r2 = r2_;
};

template <typename r>
struct closure {
    static constexpr int tag_type = 7;
    using type = r;
};

template <char c1_, char c2_>
struct range {
    static constexpr int tag_type = 8;
    static constexpr char c1 = c1_;
    static constexpr char c2 = c2_;
};

template <typename... args>
struct concat;

template <typename head_, typename... rest_>
struct concat<head_, rest_...> {
    static constexpr int tag_type = 9;
    using head = head_;
    using rest = concat<rest_...>;
};

template <typename r>
struct concat<r> {
    static constexpr int tag_type = 10;
    using head = r;
};

typedef range<'0', '9'> digit;
typedef range<'a', 'z'> letter_lower_case;
typedef range<'A', 'Z'> letter_upper_case;
typedef select<range<'a', 'z'>, range<'A', 'Z'>> letter;

template <typename... args>
struct regex_to_nfa_impl;

template <typename head, typename... rest>
struct regex_to_nfa_impl<head, rest...> {
    using type = concat_nfa<
        typename regex_to_nfa_impl<head>::type,
        typename regex_to_nfa_impl<rest...>::type
    >;
};

template <int tag_type, typename r>
struct regex_to_nfa_one;

template <typename r>
struct regex_to_nfa_impl<r> {
    using type = typename regex_to_nfa_one<r::tag_type, r>::type;
};

template <typename... args>
using regex_to_nfa = typename regex_to_nfa_impl<args...>::type;

template <typename r>
struct regex_to_nfa_one<0, r> {
    using type = condition_nfa<symbol_condition<r::value>>;
};

template <typename r>
struct regex_to_nfa_one<1, r> {
    using type = concat_nfa<
        condition_nfa<symbol_condition<r::head>>,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >;
};

template <typename r>
struct regex_to_nfa_one<2, r> {
    using type = condition_nfa<symbol_condition<r::head>>;
};

template <typename r>
struct regex_to_nfa_one<3, r> {
    using type = union_nfa<empty_nfa,
        typename regex_to_nfa_one<
            r::type::tag_type,
            typename r::type
        >::type
    >;
};

template <typename r>
struct regex_to_nfa_one<4, r> {
    using type = concat_nfa<
        typename regex_to_nfa_one<
            r::head::tag_type,
            typename r::head
        >::type,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >;
};

template <typename r>
struct regex_to_nfa_one<5, r> {
    using type = empty_nfa;
};

template <typename r>
struct regex_to_nfa_one<6, r> {
    using type = union_nfa<
        typename regex_to_nfa_one<r::r1::tag_type, typename r::r1>::type,
        typename regex_to_nfa_one<r::r2::tag_type, typename r::r2>::type
    >;
};

template <typename r>
struct regex_to_nfa_one<7, r> {
    using type = closure_nfa<
        typename regex_to_nfa_one<r::type::tag_type, typename r::type>::type
    >;
};

template <typename r>
struct regex_to_nfa_one<8, r> {
    using type = condition_nfa<range_condition<r::c1, r::c2>>;
};

template <typename r>
struct regex_to_nfa_one<9, r> {
    using type = concat_nfa<
        typename regex_to_nfa_one<r::head::tag_type, typename r::head>::type,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >;
};

template <typename r>
struct regex_to_nfa_one<10, r> {
    using type = typename regex_to_nfa_one<r::head::tag_type, typename r::head>::type;
};

template <typename transition, int s1, int s2>
struct is_nondisting {
    static constexpr bool value = is_type_list_equal<
        type_list_get<transition, s1>,
        type_list_get<transition, s2>
    >::value;
};

template <bool is_final, typename f, typename transition, int state, int i>
struct duplicated_states_impl {
    static constexpr int states = type_list_size<transition>::value;
    using type = select_type<
        is_nondisting<
            transition,
            i,
            state
        >::value &&
            (state != i) &&
            (is_final ? int_set_has<f, i>::value : !int_set_has<f, i>::value),
        int_set_insert<
            typename duplicated_states_impl<
                is_final, f, transition, state, i - 1
            >::type,
            i
        >,
        typename duplicated_states_impl<
            is_final, f, transition, state, i - 1
        >::type
    >;
};

template <bool is_final, typename f, typename transition, int state>
struct duplicated_states_impl<is_final, f, transition, state, -1> {
    using type = int_set_nil;
};

template <typename f, typename transition, int state>
using duplicated_states = typename duplicated_states_impl<
    int_set_has<f, state>::value,
    f, transition, state, type_list_size<transition>::value - 1
>::type;

template <typename old_states, int state, typename states>
struct replace_states_impl {
    using type = typename replace_states_impl<
        int_set_replace<
            old_states,
            states::head,
            state
        >,
        state,
        typename states::rest
    >::type;
};

template <typename old_states, int state>
struct replace_states_impl<old_states, state, int_set_nil> {
    using type = old_states;
};

template <typename old_states, int state, typename states>
using replace_states = typename replace_states_impl<old_states, state, states>::type;

template <typename next_states, typename states>
struct to_new_states_impl {
    using type = int_set_insert<
        typename to_new_states_impl<next_states, typename states::rest>::type,
        int_set_search<next_states, states::head>::value
    >;
};

template <typename next_states>
struct to_new_states_impl<next_states, int_set_nil> {
    using type = int_set_nil;
};

template <typename next_states, typename states>
using to_new_states = typename to_new_states_impl<next_states, states>::type;

template <typename next_states, typename map_, int state, typename states>
struct replace_transition_impl {
    using type = type_list<
        type_pair<
            typename map_::head::first,
            int_wrapper<
                int_set_has<states, map_::head::second::value>::value ?
                int_set_search<
                    next_states,
                    int_wrapper<state>::value
                >::value :
                int_set_search<
                    next_states,
                    map_::head::second::value
                >::value
            >
        >,
        typename replace_transition_impl<next_states, typename map_::rest, state, states>::type
    >;
};

template <typename next_states, int state, typename states>
struct replace_transition_impl<next_states, type_list_nil, state, states> {
    using type = type_list_nil;
};

template <typename next_states, typename map_, int state, typename states>
using replace_transition = typename replace_transition_impl<next_states, map_, state, states>::type;

template <typename next_states, typename transition, int state, typename states, int i>
struct merge_states_transition_impl {
    using type = select_type<
        int_set_has<next_states, i>::value,
        type_list<
            replace_transition<
                next_states, typename transition::head, state, states
            >,
            typename merge_states_transition_impl<
                next_states,
                typename transition::rest,
                state,
                states,
                i + 1
            >::type
        >,
        typename merge_states_transition_impl<
            next_states,
            typename transition::rest,
            state,
            states,
            i + 1
        >::type
    >;
};

template <typename next_states, int state, typename states, int i>
struct merge_states_transition_impl<next_states, type_list_nil, state, states, i> {
    using type = type_list_nil;
};

template <typename next_states, typename transition, int state, typename states, int i>
using merge_states_transition = typename merge_states_transition_impl<
    next_states, transition, state, states, i
>::type;

template <typename transition, int state, typename states>
struct merge_states {
    using next_states = int_set_diff<
        states,
        int_set_range<type_list_size<transition>::value>
    >;
    using next_transition = merge_states_transition<
        next_states,
        transition,
        state,
        states,
        0
    >;
};

template <bool has_next_, typename dfa_, int i>
struct minimize_dfa_loop {
    using duplicated = duplicated_states<
        typename dfa_::f,
        typename dfa_::transition,
        i
    >;
    static constexpr bool has_duplicated = !std::is_same<duplicated, int_set_nil>::value;
    static constexpr bool has_next = has_duplicated ||
        (i < type_list_size<typename dfa_::transition>::value - 1);
    static constexpr bool move_state = !has_duplicated;
    static constexpr int next_i = move_state ? i + 1 : 0;
    using next_states_transition = merge_states<
        typename dfa_::transition,
        i,
        duplicated
    >;
    using next_states = typename next_states_transition::next_states;
    using next_transition = typename next_states_transition::next_transition;
    using next_f = to_new_states<
        next_states,
        replace_states<typename dfa_::f, i, duplicated>
    >;
    using next_dfa = dfa<
        int_set_search<next_states, dfa_::q>::value,
        next_f,
        next_transition
    >;
    using type = typename minimize_dfa_loop<
        has_next,
        next_dfa,
        next_i
    >::type;
};

template <typename dfa_, int i>
struct minimize_dfa_loop<false, dfa_, i> {
    using type = dfa_;
};

template <typename dfa_>
using minimize_dfa = typename minimize_dfa_loop<true, dfa_, 0>::type;

template <typename transition>
struct transition_has_begin_end {
    static constexpr bool has_begin = !std::is_void<
        type_map_get<typename transition::head, symbol_condition<-1>>
    >::value || transition_has_begin_end<typename transition::rest>::has_begin;
    static constexpr bool has_end = !std::is_void<
        type_map_get<typename transition::head, symbol_condition<-2>>
    >::value || transition_has_begin_end<typename transition::rest>::has_end;
};

template <>
struct transition_has_begin_end<type_list_nil> {
    static constexpr bool has_begin = false;
    static constexpr bool has_end = false;
};

template <typename dfa_>
struct dfa_has_begin_end {
    static constexpr bool has_begin = transition_has_begin_end<
        typename dfa_::transition
    >::has_begin;
    static constexpr bool has_end = transition_has_begin_end<
        typename dfa_::transition
    >::has_end;
};

template <typename... args>
class regex {
private:
    using nfa_ = regex_to_nfa<args...>;
    using dfa_ = nfa_to_dfa<nfa_>;
    using dfa_minimal = minimize_dfa<dfa_>;
    using has_begin_end = dfa_has_begin_end<dfa_minimal>;
public:
    static bool match(const std::string &str) {
        int state = dfa_minimal::q;
        if (has_begin_end::has_begin) {
            state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -1);
        }
        for (char c : str) {
            if (state == -1) {
                return false;
            }
            state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, c);
        }
        if (has_begin_end::has_end) {
            state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -2);
        }
        return dfa_minimal::template is_final<typename dfa_minimal::f>(state);
    }
    static std::pair<size_t, size_t> find(const std::string &str, size_t from = 0, bool greedy = true) {
        int start = dfa_minimal::q;
        int state = dfa_minimal::q;
        size_t begin;
        bool has_accepted = false;
        if (from == 0) {
            begin = 0;
        } else {
            begin = from;
            from += 1;
        }
        for (size_t i = from; i < str.length() + 2; ++i) {
            if (i == 0) {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -1);
            } else if (i == str.length() + 1) {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -2);
            } else {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, str[i - 1]);
            }
            bool has_failed = false;
            if (state == -1) {
                state = start;
                has_failed = true;
            }
            if (greedy) {
                bool prev_has_accepted = has_accepted;
                has_accepted = !has_failed && dfa_minimal::template is_final<typename dfa_minimal::f>(state) &&
                    i != str.length() + 1;
                if (prev_has_accepted && !has_accepted) {
                    return std::make_pair(begin, i - begin - 1);
                }
            } else {
                if (!has_failed && dfa_minimal::template is_final<typename dfa_minimal::f>(state)) {
                    size_t end;
                    if (i == str.length() + 1) {
                        end = str.length();
                    } else {
                        end = i;
                    }
                    return std::make_pair(begin, end - begin);
                }
            }
            if (has_failed) {
                begin = i;
            }
        }
        return std::make_pair(std::string::npos, 0);
    }
    static std::vector<std::pair<size_t, size_t>> find_all(const std::string &str, size_t from = 0) {
        std::vector<std::pair<size_t, size_t>> res;
        int start = dfa_minimal::q;
        int state = dfa_minimal::q;
        size_t begin;
        bool has_accepted = false;
        if (from == 0) {
            begin = 0;
        } else {
            begin = from;
            from += 1;
        }
        for (size_t i = from; i < str.length() + 2; ++i) {
            if (i == 0) {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -1);
            } else if (i == str.length() + 1) {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, -2);
            } else {
                state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, str[i - 1]);
            }
            bool has_failed = false;
            if (state == -1) {
                state = start;
                has_failed = true;
            }
            bool prev_has_accepted = has_accepted;
            has_accepted = !has_failed && dfa_minimal::template is_final<typename dfa_minimal::f>(state) &&
                i != str.length() + 1;
            if (prev_has_accepted && !has_accepted) {
                res.emplace_back(begin, i - begin - 1);
                begin = i;
            }
            if (has_failed) {
                begin = i;
            }
        }
        return res;
    }
};
}
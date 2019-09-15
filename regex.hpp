#pragma once
#include <string>
#include "util.hpp"

namespace static_regex {

template <int q_, int f_, typename transition_>
struct nfa {
    static constexpr int q = q_;
    static constexpr int f = f_;
    static constexpr int states = f - q + 1;
    typedef transition_ transition;
};

template <char c>
struct symbol_condition {
    inline static bool condition(char input) {
        return c == input;
    }
};

template <char c1, char c2>
struct range_condition {
    inline static bool condition(char input) {
        return c1 <= input && c2 >= input;
    }
};

struct epsilon {};

typedef nfa<0, 1,
   typename type_list_new<
       typename type_list_new<
           type_pair<epsilon, int_set_new<1>::type>
       >::type
   >::type
 > empty_nfa;

template <typename transition, int n>
struct transition_item_add {
    typedef type_list<
        type_pair<typename transition::head::first,
            typename int_set_add<typename transition::head::second, n>::type
        >,
        typename transition_item_add<typename transition::rest, n>::type
    > type;
};

template <int n>
struct transition_item_add<type_list_nil, n> {
    typedef type_list_nil type;
};

template <typename transition, int n>
struct transition_add {
    typedef type_list<
        typename transition_item_add<typename transition::head, n>::type,
        typename transition_add<typename transition::rest, n>::type
    > type;
};

template <int n>
struct transition_add<type_list_nil, n> {
    typedef type_list_nil type;
};

template <typename condition>
struct condition_nfa {
    typedef nfa<0, 1,
        typename type_list_new<
            typename type_list_new<
                type_pair<condition, int_set_new<1>::type>
            >::type
        >::type
    > type;
};

template <typename n1, typename n2>
struct union_nfa {
    static constexpr int q = 0;
    static constexpr int f = n1::states + n2::states + 1;
    static constexpr int n1_q = 1;
    static constexpr int n1_f = n1::f + 1;
    static constexpr int n2_q = n1::states + 1;
    static constexpr int n2_f = n1::states + n2::states;
    typedef nfa<q, f,
        typename type_list_merge<
            typename type_list_new<
                typename type_list_new<
                    type_pair<epsilon, typename int_set_new<n1_q, n2_q>::type>
                >::type
            >::type,
            typename transition_add<typename n1::transition, n1_q>::type,
            typename type_list_new<
                typename type_list_new<
                    type_pair<epsilon, typename int_set_new<f>::type>
                >::type
            >::type,
            typename transition_add<typename n2::transition, n2_q>::type,
            typename type_list_new<
                typename type_list_new<
                    type_pair<epsilon, typename int_set_new<f>::type>
                >::type
            >::type
        >::type
    > type;
};

template <typename n1, typename n2>
struct concat_nfa {
    static constexpr int q = 0;
    static constexpr int f = n1::states + n2::states - 2;
    static constexpr int n1_q = 0;
    static constexpr int n2_q = n1::states - 1;
    typedef nfa<q, f,
        typename type_list_merge<
            typename transition_add<typename n1::transition, n1_q>::type,
            typename transition_add<typename n2::transition, n2_q>::type
        >::type
    > type;
};

template <typename n>
struct closure_nfa {
    static constexpr int q = 0;
    static constexpr int f = n::states + 1;
    static constexpr int n_q = 1;
    static constexpr int n_f = n::states;
    typedef nfa<q, f,
        typename type_list_merge<
            typename type_list_new<
                typename type_list_new<
                    type_pair<epsilon, typename int_set_new<n_q, f>::type>
                >::type
            >::type,
            typename transition_add<typename n::transition, n_q>::type,
            typename type_list_new<
                typename type_list_new<
                    type_pair<epsilon, typename int_set_new<n_q, f>::type>
                >::type
            >::type
        >::type
    > type;
};

template <typename transition, typename set_>
struct epsilon_closure_set {
    typedef typename type_map_get<
        typename type_list_get<transition, set_::head>::type,
        epsilon
    >::type head_closure;
    typedef typename int_set_union<
        set_,
        head_closure,
        typename epsilon_closure_set<
            transition,
            head_closure
        >::type,
        typename epsilon_closure_set<transition, typename set_::rest>::type
    >::type type;
};

template <typename transition>
struct epsilon_closure_set<transition, int_set_nil> {
    typedef int_set_nil type;
};

template <typename transition, int n>
struct epsilon_closure {
    typedef typename epsilon_closure_set<
        transition,
        typename int_set_new<n>::type
    >::type type;
};

template <typename transition, typename state, typename condition>
struct next_dfa_state {
    typedef typename int_set_union<
        typename next_dfa_state<transition, typename state::rest, condition>::type,
        typename epsilon_closure_set<
            transition,
            typename type_map_get<
                typename type_list_get<transition, state::head>::type,
                condition
            >::type
        >::type
    >::type type;
};

template <typename transition, typename condition>
struct next_dfa_state<transition, int_set_nil, condition> {
    typedef int_set_nil type;
};

template <typename transition, typename state>
struct dfa_state_conditions {
    typedef typename type_list_merge_if_not_exist<
        typename type_list_diff<
            type_list<
                epsilon,
                type_list_nil
            >,
            typename type_map_key_list<
                typename type_list_get<transition, state::head>::type
            >::type
        >::type,
        typename dfa_state_conditions<transition, typename state::rest>::type
    >::type type;
};

template <typename transition>
struct dfa_state_conditions<transition, int_set_nil> {
    typedef type_list_nil type;
};

template <typename transition, typename state, typename condition>
struct dfa_condition_map {
    typedef type_list<
        type_pair<typename condition::head,
            typename next_dfa_state<transition, state, typename condition::head>::type
        >,
        typename dfa_condition_map<transition, state, typename condition::rest>::type
    > type;
};

template <typename transition, typename state>
struct dfa_condition_map<transition, state, type_list_nil> {
    typedef type_list_nil type;
};

template <typename transition, typename states>
struct dfa_transition {
    typedef type_list<
        type_pair<
            typename states::head,
            typename dfa_condition_map<
                transition, typename states::head,
                typename dfa_state_conditions<transition, typename states::head>::type
            >::type
        >,
        typename dfa_transition<transition, typename states::rest>::type
    > type;
};

template <typename transition>
struct dfa_transition<transition, type_list_nil> {
    typedef type_list_nil type;
};

template <typename transition>
struct dfa_new_states {
    typedef typename type_list_merge<
        typename type_map_value_list<typename transition::head::second>::type,
        typename dfa_new_states<typename transition::rest>::type
    >::type type;
};

template<>
struct dfa_new_states<type_list_nil> {
    typedef type_list_nil type;
};

template <typename m1, typename m2>
struct condition_map_merge {
    typedef typename condition_map_merge<m1, typename m2::rest>::type rest;
    typedef typename type_map_get<m1, typename m2::head::first>::type v1;
    typedef typename int_set_union<v1, typename m2::head::second>::type v;
    typedef typename type_map_set<rest, typename m2::head::first, v>::type type;
};

template <typename m1>
struct condition_map_merge<m1, type_list_nil> {
    typedef m1 type;
};

template <typename t1, typename t2>
struct merge_dfa_transition {
    typedef typename merge_dfa_transition<t1, typename t2::rest>::type rest;
    typedef typename type_map_get<t1, typename t2::head::first>::type m;
    typedef typename condition_map_merge<m, typename t2::head::second>::type merged;
    typedef typename type_map_set<
        rest,
        typename t2::head::first,
        merged
    >::type type;
};

template <typename t>
struct merge_dfa_transition<t, type_list_nil> {
    typedef t type;
};

template <typename states_, typename transition_>
struct nfa_to_dfa_loop_result {
    typedef states_ states;
    typedef transition_ transition;
};

template <bool has_next_, typename nfa, typename states, typename transition>
struct nfa_to_dfa_loop {
    typedef typename dfa_transition<typename nfa::transition, states>::type new_transition;
    typedef typename dfa_new_states<new_transition>::type new_states;
    typedef typename type_list_merge_if_not_exist<states, new_states>::type next_states;
    typedef typename merge_dfa_transition<transition, new_transition>::type next_transition;
    static constexpr bool has_next = !std::is_same<states, next_states>::value;
    typedef typename nfa_to_dfa_loop<
        has_next,
        nfa,
        next_states,
        next_transition
    >::type type;
};

template <typename nfa, typename states, typename transition>
struct nfa_to_dfa_loop<false, nfa, states, transition> {
    typedef nfa_to_dfa_loop_result<states, transition> type;
};

template <typename all_states, typename map_>
struct condition_map_int {
    typedef type_list<
        type_pair<
            typename map_::head::first,
            int_wrapper<type_list_search<all_states, typename map_::head::second>::value>
        >,
        typename condition_map_int<all_states, typename map_::rest>::type
    > type;
};

template <typename all_states>
struct condition_map_int<all_states, type_list_nil> {
    typedef type_list_nil type;
};

template <typename all_states, typename states, typename transition>
struct dfa_transition_int {
    typedef type_list<
        typename condition_map_int<
            all_states,
            typename type_map_get<transition, typename states::head>::type
        >::type,
        typename dfa_transition_int<all_states, typename states::rest, transition>::type
    > type;
};

template <typename all_states, typename transition>
struct dfa_transition_int<all_states, type_list_nil, transition> {
    typedef type_list_nil type;
};

template <typename all_states, typename states, int f>
struct dfa_final_states {
    typedef typename select_type<
        int_set_has<typename states::head, f>::value,
        typename int_set_insert<
            typename dfa_final_states<all_states, typename states::rest, f>::type,
            type_list_search<all_states, typename states::head>::value
        >::type,
        typename dfa_final_states<all_states, typename states::rest, f>::type
    >::type type;
};

template <typename all_states, int f>
struct dfa_final_states<all_states, type_list_nil, f> {
    typedef int_set_nil type;
};

template <int q_, typename f_, typename transition_>
struct dfa {
    static constexpr int q = q;
    typedef f_ f;
    typedef transition_ transition;
    typedef typename int_set_range<type_list_size<transition_>::value>::type states;
    template <typename map_>
    inline static int find_state(char c) {
        if (map_::head::first::condition(c)) {
            return map_::head::second::value;
        } else {
            return find_state<typename map_::rest>(c);
        }
    }
    template <>
    inline static int find_state<type_list_nil>(char c) {
        return -1;
    }
    template <typename transition_left>
    inline static int next_state(int state, char c) {
        if (state == 0) {
            return find_state<typename transition_left::head>(c);
        } else {
            return next_state<typename transition_left::rest>(state - 1, c);
        }
    }
    template <>
    inline static int next_state<type_list_nil>(int state, char c) {
        return -1;
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
struct nfa_to_dfa {
    typedef typename nfa_to_dfa_loop<
        true,
        nfa_,
        typename type_list_new<
            typename epsilon_closure<typename nfa_::transition, nfa_::q>::type
        >::type,
        type_list_nil
    >::type result;
    typedef typename dfa_transition_int<
        typename result::states,
        typename result::states,
        typename result::transition
    >::type transition;
    typedef typename dfa_final_states<
        typename result::states,
        typename result::states,
        nfa_::f
    >::type final_states;
    typedef dfa<0, final_states, transition> type;
};

template <char c>
struct character {
    static constexpr int tag_type = 0;
    static constexpr char value = c;
};

template <char... args>
struct static_string;

template <char c>
struct static_string<c> {
    static constexpr int tag_type = 2;
    static constexpr char head = c;
    typedef void rest;
};

template <char head_, char... rest_>
struct static_string<head_, rest_...> {
    static constexpr int tag_type = 1;
    static constexpr char head = head_;
    typedef static_string<rest_...> rest;
};

template <typename r>
struct option {
    static constexpr int tag_type = 3;
    typedef r type;
};

template <typename r, int times>
struct repeat {
    static constexpr int tag_type = 4;
    typedef r head;
    typedef repeat<r, times - 1> rest;
};

template <typename r>
struct repeat<r, 0> {
    static constexpr int tag_type = 5;
};

template <typename r1_, typename r2_>
struct select {
    static constexpr int tag_type = 6;
    typedef r1_ r1;
    typedef r2_ r2;
};

template <typename r>
struct closure {
    static constexpr int tag_type = 7;
    typedef r type;
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
    typedef head_ head;
    typedef concat<rest_...> rest;
};

template <typename r>
struct concat<r> {
    static constexpr int tag_type = 10;
    typedef r head;
};

typedef range<'0', '9'> digit;
typedef range<'a', 'b'> letter_lower_case;
typedef range<'A', 'B'> letter_upper_case;
typedef select<range<'a', 'b'>, range<'A', 'B'>> letter;

template <typename... args>
struct regex_to_nfa;

template <typename head, typename... rest>
struct regex_to_nfa<head, rest...> {
    typedef typename concat_nfa<
        typename regex_to_nfa<head>::type,
        typename regex_to_nfa<rest...>::type
    >::type type;
};

template <int tag_type, typename r>
struct regex_to_nfa_one;

template <typename r>
struct regex_to_nfa_one<0, r> {
    typedef typename condition_nfa<symbol_condition<r::value>>::type type;
};

template <typename r>
struct regex_to_nfa_one<1, r> {
    typedef typename concat_nfa<
        typename condition_nfa<symbol_condition<r::head>>::type,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<2, r> {
    typedef typename condition_nfa<symbol_condition<r::head>>::type type;
};

template <typename r>
struct regex_to_nfa_one<3, r> {
    typedef typename union_nfa<empty_nfa,
        typename regex_to_nfa_one<
            r::type::tag_type,
            typename r::type
        >::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<4, r> {
    typedef typename concat_nfa<
        typename regex_to_nfa_one<
            r::head::tag_type,
            typename r::head
        >::type,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<5, r> {
    typedef empty_nfa type;
};

template <typename r>
struct regex_to_nfa_one<6, r> {
    typedef typename union_nfa<
        typename regex_to_nfa_one<r::r1::tag_type, typename r::r1>::type,
        typename regex_to_nfa_one<r::r2::tag_type, typename r::r2>::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<7, r> {
    typedef typename closure_nfa<
        typename regex_to_nfa_one<r::type::tag_type, typename r::type>::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<8, r> {
    typedef typename condition_nfa<range_condition<r::c1, r::c2>>::type type;
};

template <typename r>
struct regex_to_nfa_one<9, r> {
    typedef typename concat_nfa<
        typename regex_to_nfa_one<r::head::tag_type, typename r::head>::type,
        typename regex_to_nfa_one<
            r::rest::tag_type,
            typename r::rest
        >::type
    >::type type;
};

template <typename r>
struct regex_to_nfa_one<10, r> {
    typedef typename regex_to_nfa_one<r::head::tag_type, typename r::head>::type type;
};

template <typename r>
struct regex_to_nfa<r> {
    typedef typename regex_to_nfa_one<r::tag_type, r>::type type;
};

template <typename transition, int s1, int s2>
struct is_nondisting {
    typedef typename type_map_search_value<
        typename transition::head,
        int_wrapper<s1>
    >::type k1;
    typedef typename type_map_search_value<
        typename transition::head,
        int_wrapper<s2>
    >::type k2;
    static constexpr bool is_head_nondisting = is_type_list_equal<k1, k2>::value;
    static constexpr bool value = is_head_nondisting && is_nondisting<
        typename transition::rest,
        s1, s2
    >::value;
};

template <int s1, int s2>
struct is_nondisting<type_list_nil, s1, s2> {
    static constexpr bool value = true;
};

template <bool is_final, typename f, typename transition, int state, int i>
struct duplicated_states_ {
    static constexpr int states = type_list_size<transition>::value;
    typedef typename select_type<
        is_nondisting<
            transition,
            i,
            state
        >::value &&
            (state != i) &&
            (is_final ? int_set_has<f, i>::value : !int_set_has<f, i>::value),
        typename int_set_insert<
            typename duplicated_states_<
                is_final, f, transition, state, i - 1
            >::type,
            i
        >::type,
        typename duplicated_states_<
            is_final, f, transition, state, i - 1
        >::type
    >::type type;
};

template <bool is_final, typename f, typename transition, int state>
struct duplicated_states_<is_final, f, transition, state, -1> {
    typedef int_set_nil type;
};

template <typename f, typename transition, int state>
struct duplicated_states {
    typedef typename duplicated_states_<
        int_set_has<f, state>::value,
        f, transition, state, type_list_size<transition>::value - 1
    >::type type;
};

template <typename old_states, int state, typename states>
struct replace_states {
    typedef typename select_type<
        int_set_has<states, old_states::head>::value,
        typename int_set_insert<
            typename int_set_remove<
                typename replace_states<old_states, state, typename states::rest>::type,
                old_states::head
            >::type,
            state
        >::type,
        typename int_set_remove<
            typename replace_states<old_states, state, typename states::rest>::type,
            old_states::head
        >::type
    >::type type;
};

template <int state, typename states>
struct replace_states<int_set_nil, state, states> {
    typedef int_set_nil type;
};

template <typename old_states, int state>
struct replace_states<old_states, state, int_set_nil> {
    typedef old_states type;
};

template <int state>
struct replace_states<int_set_nil, state, int_set_nil> {
    typedef int_set_nil type;
};

template <typename map_, int state, typename states>
struct replace_transition {
    typedef type_list<
        type_pair<
            typename map_::head::first,
            typename select_type<
                int_set_has<states, map_::head::second::value>::value,
                int_wrapper<state>,
                int_wrapper<map_::head::second::value>
            >::type
        >,
        typename replace_transition<typename map_::rest, state, states>::type
    > type;
};

template <int state, typename states>
struct replace_transition<type_list_nil, state, states> {
    typedef type_list_nil type;
};

template <typename next_states, typename transition, int state, typename states, int i>
struct merge_states_transition {
    typedef typename select_type<
        int_set_has<next_states, i>::value,
        type_list<
            typename replace_transition<typename transition::head, state, states>::type,
            typename merge_states_transition<
                next_states,
                typename transition::rest,
                state,
                states,
                i + 1
            >::type
        >,
        typename merge_states_transition<
            next_states,
            typename transition::rest,
            state,
            states,
            i + 1
        >::type
    >::type type;
};

template <typename next_states, int state, typename states, int i>
struct merge_states_transition<next_states, type_list_nil, state, states, i> {
    typedef type_list_nil type;
};

template <typename transition, int state, typename states>
struct merge_states {
    typedef typename int_set_diff<
        states,
        typename int_set_range<type_list_size<transition>::value>::type
    >::type next_states;
    typedef typename merge_states_transition<
        next_states,
        transition,
        state,
        states,
        0
    >::type next_transition;
};

template <bool has_next_, typename dfa_, int i>
struct minimize_dfa_loop {
    typedef typename duplicated_states<
        typename dfa_::f,
        typename dfa_::transition,
        i
    >::type duplicated;
    static constexpr bool has_next = !std::is_same<duplicated, int_set_nil>::value;
    typedef merge_states<
        typename dfa_::transition,
        i,
        duplicated
    > next_states_transition;
    typedef typename next_states_transition::next_states next_states;
    typedef typename next_states_transition::next_transition next_transition;
    typedef typename replace_states<typename dfa_::f, i, duplicated>::type next_f;
    typedef dfa<
        int_set_search<next_states, dfa_::q>::value,
        next_f,
        next_transition
    > next_dfa;
    typedef typename minimize_dfa_loop<
        has_next,
        next_dfa,
        i + 1
    >::type type;
};

template <typename dfa_, int i>
struct minimize_dfa_loop<false, dfa_, i> {
    typedef dfa_ type;
};

template <typename dfa_>
struct minimize_dfa {
    typedef typename minimize_dfa_loop<true, dfa_, 0>::type type;
};

template <typename... args>
class regex {
private:
    typedef typename regex_to_nfa<args...>::type nfa_;
    typedef typename nfa_to_dfa<nfa_>::type dfa_;
    typedef typename minimize_dfa<dfa_>::type dfa_minimal;
public:
    static bool match(const std::string &str) {
        int state = dfa_minimal::q;
        for (char c : str) {
            if (state == -1) {
                return false;
            }
            state = dfa_minimal::template next_state<typename dfa_minimal::transition>(state, c);
        }
        return dfa_minimal::template is_final<typename dfa_minimal::f>(state);
    }
};
}
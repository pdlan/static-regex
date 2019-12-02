#include <utility>
#include "regex.hpp"
#include "util.hpp"
#include "parser.hpp"

template <typename Patterns>
class Lexer;

struct ConstexprToken {
    constexpr ConstexprToken() : start(0), length(0), type(0) {}
    size_t start;
    size_t length;
    int type;
};

template <char... C>
struct StaticString {
    static constexpr char String[] = {C..., '\0'};
};

template <typename C, C... Args>
StaticString<Args...> operator ""_staticstring();

#define STATICSTRING(r) decltype(r##_staticstring)

template <template <typename...> class M, template <typename, typename> class P, typename... R, typename... T>
class Lexer<M<P<R, T>...>> {
private:
    static constexpr size_t BufferSize = 128;
    static constexpr size_t Patterns = sizeof...(R);
    static constexpr bool IsPatternVoid[Patterns] = {std::is_void<T>::value...};
    static constexpr std::tuple<bool, size_t, size_t, ConstexprArray<ConstexprToken, BufferSize>> constexpr_lex(const char *input, size_t pos) {
        ConstexprArray<ConstexprToken, BufferSize> res;
        size_t size = 0;
        bool has_finished = false;
        while (size < BufferSize) {
            if (!input[pos]) {
                has_finished = true;
                break;
            }
            size_t match_size[Patterns] = {R::match_prefix(input + pos)...};
            size_t max_size = 0;
            size_t max_pattern = 0;
            for (size_t i = 0; i < Patterns; ++i){
                if (match_size[i] > max_size) {
                    max_size = match_size[i];
                    max_pattern = i;
                }
            }
            if (max_size == 0) {
                break;
            }
            if (IsPatternVoid[max_pattern]) {
                pos += max_size;
                continue;
            }
            ConstexprToken token;
            token.start = pos;
            token.length = max_size;
            token.type = max_pattern;
            res[size] = token;
            pos += max_size;
            ++size;
        }
        return std::make_tuple(has_finished, pos, size, res);
    }
    using TokenGetterList = TypeList::List<T...>;
    template <const char *S>
    struct LexImpl {
        template <bool, typename Tokens, size_t Pos>
        struct Iteration {
            static constexpr std::tuple<
                bool, size_t, size_t,
                ConstexprArray<ConstexprToken, BufferSize>
            > ConstexprLexResult = constexpr_lex(S, Pos);
            static constexpr bool HasFinished = std::get<0>(ConstexprLexResult);
            static constexpr size_t NextPos = std::get<1>(ConstexprLexResult);
            static constexpr size_t Size = std::get<2>(ConstexprLexResult);
            static constexpr ConstexprArray<ConstexprToken, BufferSize> ConstexprTokens = std::get<3>(ConstexprLexResult);
            static constexpr bool HasNext = !HasFinished;
            template <typename Index>
            struct GetNewTokens;
            template <size_t... I>
            struct GetNewTokens<std::index_sequence<I...>> {
                using Type = TypeList::List<
                    typename TypeList::Get<TokenGetterList, ConstexprTokens[I].type>::template Get<
                        S, ConstexprTokens[I].start, ConstexprTokens[I].length
                    >...
                >;
            };
            using NewTokens = typename GetNewTokens<std::make_index_sequence<Size>>::Type;
            using NextTokens = TypeList::Merge<Tokens, NewTokens>;
            using Type = typename Iteration<HasNext, NextTokens, NextPos>::Type;

        };
        template <typename Tokens, size_t pos>
        struct Iteration<false, Tokens, pos> {
            using Type = Tokens;
        };
        using Type = typename Iteration<true, TypeList::List<>, 0>::Type;
    };
public:
    template <typename S>
    using Lex = typename LexImpl<S::String>::Type;
    template <const char *S>
    using LexCharArray = typename LexImpl<S>::Type;
};

template <typename T>
struct SimpleTokenGetter {
    template <const char *S, size_t P, size_t L>
    using Get = T;
};
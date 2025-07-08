#ifndef CONCEPTS_HPP
#define CONCEPTS_HPP

#include <type_traits>

namespace ecsys {
template <class...>
struct areTypesUnique : std::true_type {};

template <class T, class... Ts>
struct areTypesUnique<T, Ts...>
    : std::conjunction<std::negation<std::disjunction<std::is_same<T, Ts>...>>,
                       areTypesUnique<Ts...>> {};

template <class... Ts>
concept UniqueTypes = areTypesUnique<Ts...>::value;
}  // namespace ecsys

#endif
/*
 * lift C++ higher order convenience functions
 *
 * Copyright © Björn Fahller 2017,2018
 *
 *  Use, modification and distribution is subject to the
 *  Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt)
 *
 * Project home: https://github.com/rollbear/lift
 */

#ifndef HIGHER_ORDER_FUNCTIONS_LIFT_HPP
#define HIGHER_ORDER_FUNCTIONS_LIFT_HPP

#include <utility>
#include <tuple>

#define LIFT_FUNCTION(f)                                                  \
    LIFT_ba7b8453f262e429575e23dcb2192b33(                                \
        a_ba7b8453f262e429575e23dcb2192b33,                               \
        (f)(::std::forward<decltype(a_ba7b8453f262e429575e23dcb2192b33)>( \
            a_ba7b8453f262e429575e23dcb2192b33)...))

#define LIFT_ba7b8453f262e429575e23dcb2192b33(a, f_of_a)              \
    [&](auto&&... a) noexcept(noexcept(f_of_a)) -> decltype(f_of_a) { \
        return f_of_a;                                                \
    }

#define LIFT_THRICE(...)                                 \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) \
  {                                                      \
    return __VA_ARGS__;                                  \
  }

#define LIFT_FWD(x) std::forward<decltype(x)>(x)
#ifndef LIFT
#define LIFT LIFT_FUNCTION
#endif

namespace lift {

template <typename F>
inline
constexpr
auto
compose(
  F&& f
)
  noexcept
-> F
{
  return std::forward<F>(f);
}

namespace detail {

  template <typename P1, typename P2, typename F, typename Tail, typename ... T>
  void
  compose(P1, P2, F&&, Tail&&, T&& ...)
  {
    constexpr auto unitail = std::is_invocable_v<Tail, T...>;
    constexpr auto multitail = (std::is_invocable_v<Tail, T> && ...);
    if constexpr (unitail)
    {
      using tail_type = std::invoke_result_t<Tail, T...>;
      static_assert(std::is_invocable_v<F, tail_type>,
                    "Function not callable with result of next function");
    }
    else if constexpr (multitail)
    {
      static_assert(std::is_invocable_v<F, std::invoke_result<Tail, T>...>,
                    "Function not callable with results from multiple calls of unary function");
    }
    static_assert(unitail || multitail, "function not callable");
    static_assert(sizeof...(T) == 1U || !(unitail && multitail), "ambigous composition");
  }

  template <typename P, typename F, typename Tail, typename ... T>
  inline
  constexpr
  auto
  compose(
    std::true_type,
    P,
    F&& f,
    Tail&& tail,
    T&& ... objs)
  noexcept(noexcept(f(tail(std::forward<T>(objs)...))))
  -> decltype(f(tail(std::forward<T>(objs)...)))
  {
    return f(tail(std::forward<T>(objs)...));
  }

  template <typename F, typename Tail, typename ... T>
  inline
  constexpr
  auto
  compose(
    std::false_type,
    std::true_type,
    F&& f,
    Tail&& tail,
    T&& ... objs)
  noexcept(noexcept(f(tail(std::forward<T>(objs))...)))
  -> decltype(f(tail(std::forward<T>(objs))...))
  {
    return f(tail(std::forward<T>(objs))...);
  }
}


template <typename F, typename ... Fs>
inline
constexpr
auto
compose(
  F&& f,
  Fs&&... fs)
{
  return [f = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)]
    (auto&& ... objs)
    noexcept(noexcept(detail::compose(typename std::is_invocable<decltype(compose(std::forward<Fs>(fs)...)), decltype(objs)...>::type{},
                                      std::bool_constant<(std::is_invocable_v<decltype(compose(std::forward<Fs>(fs)...)), decltype(objs)>  && ...)>{},
                                      f,
                                      compose(std::forward<Fs>(fs)...),
                                      LIFT_FWD(objs)...)))
    -> decltype(detail::compose(typename std::is_invocable<decltype(compose(std::forward<Fs>(fs)...)), decltype(objs)...>::type{},
                                std::bool_constant<(std::is_invocable_v<decltype(compose(std::forward<Fs>(fs)...)), decltype(objs)> && ...)>{},
                                f,
                                compose(std::forward<Fs>(fs)...),
                                LIFT_FWD(objs)...))
  {
    using tail_type = decltype(compose(std::forward<Fs>(fs)...));
    // gcc-7.2 ICEs if tail_type is visible in the surrounding scope
    // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=83644

    constexpr auto unitail = typename std::is_invocable<tail_type, decltype(objs)...>::type{};
    constexpr auto multitail = (std::is_invocable_v<tail_type, decltype(objs)> && ...);

    return detail::compose(unitail, std::bool_constant<multitail>{}, f, tail, LIFT_FWD(objs)...);
  };
}

template <typename F>
inline
constexpr
auto
negate(
  F&& f)
{
  return
    [f = std::forward<F>(f)](auto&& ... obj) LIFT_THRICE(!f(LIFT_FWD(obj)...));
}

template <typename T>
inline
constexpr
auto
equal(
  T &&t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj == t);
}

template <typename T>
inline
constexpr
auto
not_equal(
  T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj != t);
}

template <typename T>
inline
constexpr
auto
less_than(
  T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj < t);
}

template <typename T>
inline
constexpr
auto
less_equal(
  T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj <= t);
}

template <typename T>
inline
constexpr
auto
greater_than(
  T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj > t);
}

template <typename T>
inline
auto
constexpr
greater_equal(
  T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj >= t);
}

namespace detail
{
  template <typename Fs, std::size_t ... I, typename ... T>
  inline
  constexpr
  bool
  when_all(
    Fs& fs,
    std::index_sequence <I...>,
    const T& ... t)
  noexcept(noexcept((std::get<I>(fs)(t...) && ...)))
  {
    return (std::get<I>(fs)(t...) && ...);
  }
}

template <typename ... Fs>
inline
constexpr
auto
when_all(
  Fs&&... fs)
{
  return
    [funcs = std::tuple(std::forward<Fs>(fs)...)]
      (const auto& ... obj)
    noexcept(noexcept((std::forward<Fs>(fs)(obj...) && ...)))
  -> bool
  {
    return detail::when_all(
      funcs,
      std::index_sequence_for<Fs...>{},
      obj...
    );
  };
}

namespace detail
{
  template <typename Fs, std::size_t ... I, typename ... T>
  inline
  constexpr
  bool
  when_any(
    Fs& fs,
    std::index_sequence<I...>,
    const T& ... t)
  noexcept(noexcept((std::get<I>(fs)(t...) || ...)))
  {
    return (std::get<I>(fs)(t...) || ...);
  }
}

template <typename ... Fs>
inline
constexpr
auto
when_any(
  Fs&& ... fs)
{
  return
    [funcs = std::tuple(std::forward<Fs>(fs)...)]
      (const auto& ... obj)
    noexcept(noexcept((std::forward<Fs>(fs)(obj...) || ...)))
  -> bool
  {
    return detail::when_any(
      funcs,
      std::index_sequence_for<Fs...>{},
      obj...
    );
  };
}

template <typename ... Fs>
inline
constexpr
auto
when_none(
  Fs&& ... fs)
{
  return negate(when_any(std::forward<Fs>(fs)...));
}

template <typename Predicate, typename Action>
inline
constexpr
auto
if_then(
  Predicate&& predicate,
  Action&& action)
{
  return [
    predicate = std::forward<Predicate>(predicate),
    action = std::forward<Action>(action)
  ]
    (auto&& ... obj)
  mutable
    noexcept(
    noexcept(true == predicate(obj...))
    && noexcept(action(LIFT_FWD(obj)...)))
    -> void
  {
    if (predicate(obj...))
    {
      action(LIFT_FWD(obj)...);
    }
  };
}

template <typename Predicate, typename TAction, typename FAction>
inline
constexpr
auto
if_then_else(
  Predicate&& predicate,
  TAction&& t_action,
  FAction&& f_action)
{
  return [
    predicate = std::forward<Predicate>(predicate),
    t_action = std::forward<TAction>(t_action),
    f_action = std::forward<FAction>(f_action)
  ]
    (auto&& ... obj)
  mutable
    noexcept(
    noexcept(true == predicate(obj...))
    && noexcept(t_action(LIFT_FWD(obj)...))
    && noexcept(f_action(LIFT_FWD(obj)...)))
  -> std::common_type_t<
      decltype(t_action(LIFT_FWD(obj)...)),
      decltype(f_action(LIFT_FWD(obj)...))
    >
  {
    if (predicate(obj...))
    {
      return t_action(LIFT_FWD(obj)...);
    }
    else
    {
      return f_action(LIFT_FWD(obj)...);
    }
  };
}

namespace detail
{
  template <typename Fs, std::size_t ... I, typename ... T>
  inline
  constexpr
  void
  do_all(
    Fs& fs,
    std::index_sequence<I...>,
    const T& ... t)
  {
    ((void)(std::get<I>(fs)(t...)),  ...);
  }
}


template <typename ... Fs>
inline
constexpr
auto
do_all(
  Fs&& ... fs)
{
  return
    [funcs = std::tuple(std::forward<Fs>(fs)...)]
      (const auto& ... obj)
      mutable
      noexcept((noexcept(fs(obj...)) && ...))
  -> void
  {
    detail::do_all(
      funcs,
      std::index_sequence_for<Fs...>{},
      obj...
    );
  };
}

}

#endif //HIGHER_ORDER_FUNCTIONS_LIFT_HPP

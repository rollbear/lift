/*
 * lift C++ higher order convenience functions
 *
 * Copyright © Björn Fahller 2017
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

#define LIFT_THRICE(...)                \
        noexcept(noexcept(__VA_ARGS__)) \
        -> decltype(__VA_ARGS__)        \
        {                               \
          return __VA_ARGS__;           \
        }

#define LIFT_FWD(x) std::forward<decltype(x)>(x)

namespace lift {

template <typename F, typename ... Fs>
inline
constexpr
auto compose(
  F&& f,
  Fs&&... fs)
{
  if constexpr (sizeof...(fs) == 0)
  {
    return std::forward<F>(f);
  }
  else
  {
    return [f = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)]
      (auto&& ... objs)
    {
      using tail_type = decltype(tail);
      constexpr bool multitail = (std::is_invocable_v<tail_type, decltype(objs)>
                                  && ...);
      constexpr bool unitail = std::is_invocable_v<tail_type, decltype(objs)...>;

      static_assert(sizeof...(objs) == 1 || !(multitail && unitail),
                    "ambigous composition");

      if constexpr (unitail)
      {
        using tail_rt = decltype(tail(LIFT_FWD(objs)...));
        constexpr bool match = std::is_invocable_v<F, tail_rt>;
        static_assert(match, "type mismatch, can't compose");
        if constexpr (match)
        {
          return f(tail(LIFT_FWD(objs)...));
        }
      }
      else if constexpr(multitail)
      {
        constexpr bool match =
          std::is_invocable_v<F, decltype(tail(LIFT_FWD(objs)))...>;

        static_assert(match, "type mismatch, can't compose");
        if constexpr (match)
        {
          return f(tail(LIFT_FWD(objs))...);
        }
      }
      else
      {
        static_assert(multitail || unitail, "arity mismatch, can't compose");
      }
    };
  }
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
    T&& ... t)
  noexcept(noexcept((std::get<I>(fs)(std::forward<T>(t)...) && ...)))
  {
    return (std::get<I>(fs)(std::forward<T>(t)...) && ...);
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
      std::index_sequence_for<Fs...>{}, LIFT_FWD(obj)...
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
    T&& ... t)
  noexcept(noexcept((std::get<I>(fs)(std::forward<T>(t)...) || ...)))
  {
    return (std::get<I>(fs)(std::forward<T>(t)...) || ...);
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
      std::index_sequence_for<Fs...>{}, LIFT_FWD(obj)...
    );
  };
}

template <typename ... Fs>
inline
constexpr
auto
when_none(
  Fs ... fs)
{
  return negate(when_any(std::move(fs)...));
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
    && noexcept(t_action(LIFT_FWD(obj)...)))
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
    T&& ... t)
  {
    ((void)(std::get<I>(fs)(std::forward<T>(t)...)),  ...);
  }
}


template <typename ... Fs>
inline
constexpr
auto
do_all(
  Fs ... fs)
{
  return
    [funcs = std::tuple(std::forward<Fs>(fs)...)]
      (const auto& ... obj)
      mutable
      noexcept(noexcept(((void)(LIFT_FWD(obj)), ...)))
  -> void
  {
    detail::do_all(
      funcs,
      std::index_sequence_for<Fs...>{},
      LIFT_FWD(obj)...
    );
  };
}

}
#endif //HIGHER_ORDER_FUNCTIONS_LIFT_HPP

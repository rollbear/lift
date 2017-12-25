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

namespace detail {
  template <typename ... T>
  using first_type = std::tuple_element_t<0, std::tuple<T...>>;
}

template <typename F, typename ... Fs>
inline auto compose(F&& f, Fs&&... fs)
{
  if constexpr (sizeof...(fs) == 0)
  {
    return std::forward<F>(f);
  }
  else
  {
    return [f = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)]
      (auto&& ... obj)
    mutable
    {
      using tailtype = decltype(tail);
      constexpr bool multitail = std::is_invocable_v<tailtype, detail::first_type<decltype(obj)...>>;
      constexpr bool unitail = std::is_invocable_v<tailtype, decltype(obj)...>;
      if constexpr (unitail)
      {
        constexpr bool handled = std::is_invocable_v<F, decltype(tail(LIFT_FWD(obj)...))>;
        static_assert(handled, "can't compose");
        if constexpr (handled)
        {
          return f(tail(LIFT_FWD(obj)...));
        }
      }
      else if constexpr(multitail)
      {
        constexpr bool handled = std::is_invocable_v<F, decltype(tail(LIFT_FWD(obj)))...>;
        static_assert(handled, "can't compose");
        if constexpr (handled)
        {
          return f(tail(LIFT_FWD(obj))...);
        }
      }
      else
      {
        static_assert(multitail || unitail, "can't compose");
      }
    };
  }
}

template <typename F>
inline auto negate(F&& f)
{
  return [f = std::forward<F>(f)](auto&& ... obj) mutable
         LIFT_THRICE(!f(LIFT_FWD(obj)...));
}

template <typename T>
inline auto equals(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj == t);
}

template <typename T>
inline auto not_equal(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj != t);
}

template <typename T>
inline auto less_than(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj < t);
}

template <typename T>
inline auto less_equal(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj <= t);
}

template <typename T>
inline auto greater_than(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj > t);
}

template <typename T>
inline auto greater_equal(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) mutable LIFT_THRICE(obj >= t);
}

namespace detail
{
  template <typename Fs, std::size_t ... I, typename ... T>
  inline bool when_all(Fs& fs, std::index_sequence <I...>, T&& ... t)
  {
    return ((std::get<I>(fs)(std::forward<T>(t)...)) && ...);
  }
}

template <typename ... Fs>
inline auto when_all(Fs&&... fs)
{
  return [funcs = std::tuple(std::forward<Fs>(fs)...)](const auto& ... obj)
    mutable
    noexcept(noexcept((std::forward<Fs>(fs)(obj...) && ...)))
  -> bool
  {
    return detail::when_all(funcs, std::index_sequence_for<Fs...>{}, LIFT_FWD(obj)...);
  };
}

namespace detail
{
  template <typename Fs, std::size_t ... I, typename ... T>
  inline bool when_any(Fs& fs, std::index_sequence<I...>, T&& ... t)
  {
    return (std::get<I>(fs)(std::forward<T>(t)...) || ...);
  }
}
template <typename ... Fs>
inline auto when_any(Fs&& ... fs)
{
  return [funcs = std::tuple(std::forward<Fs>(fs)...)](const auto& ... obj)
    mutable
    noexcept(noexcept((std::forward<Fs>(fs)(obj...) || ...)))
  -> bool
  {
    return detail::when_any(funcs, std::index_sequence_for<Fs...>{}, LIFT_FWD(obj)...);
  };
}

template <typename ... Fs>
inline auto when_none(Fs ... fs)
{
  return negate(when_any(std::move(fs)...));
}

template <typename Predicate, typename Action>
inline auto if_then(Predicate&& predicate, Action&& action)
{
  return [
    predicate = std::forward<Predicate>(predicate),
    action = std::forward<Action>(action)]
    (auto&& ... obj)
  mutable
    noexcept(noexcept(true == predicate(obj...))
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
inline auto if_then_else(Predicate&& predicate,
                         TAction&& t_action,
                         FAction&& f_action)
{
  return [
    predicate = std::forward<Predicate>(predicate),
    t_action = std::forward<TAction>(t_action),
    f_action = std::forward<FAction>(f_action)
  ](auto&& ... obj)
  mutable
  noexcept(noexcept(true == predicate(obj...))
           && noexcept(t_action(LIFT_FWD(obj)...))
           && noexcept(t_action(LIFT_FWD(obj)...)))
  {
    if (predicate(obj...))
    {
      t_action(LIFT_FWD(obj)...);
    }
    else
    {
      f_action(LIFT_FWD(obj)...);
    }
  };
}

namespace detail
{
template <typename Fs, std::size_t ... I, typename ... T>
inline void do_all(Fs& fs, std::index_sequence<I...>, T&& ... t)
{
   ((void)(std::get<I>(fs)(std::forward<T>(t)...)),  ...);
}
}


template <typename ... Fs>
inline auto do_all(Fs ... fs)
{
  return [funcs = std::tuple(std::forward<Fs>(fs)...)](const auto& ... obj)
    mutable
    noexcept(noexcept(((void)(LIFT_FWD(obj)), ...)))
  -> void
  {
    detail::do_all(funcs, std::index_sequence_for<Fs...>{}, LIFT_FWD(obj)...);
  };
}

}
#endif //HIGHER_ORDER_FUNCTIONS_LIFT_HPP

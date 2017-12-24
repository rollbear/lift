#ifndef HIGHER_ORDER_FUNCTIONS_LIFT_HPP
#define HIGHER_ORDER_FUNCTIONS_LIFT_HPP

#include <utility>

#define LIFT_THRICE(...)                \
        noexcept(noexcept(__VA_ARGS__)) \
        -> decltype(__VA_ARGS__)        \
        {                               \
          return __VA_ARGS__;           \
        }

namespace lift {

template <typename F, typename ... Fs>
inline auto compose(F&& f, Fs&&... fs)
{
  if constexpr (sizeof...(fs) == 0)
  {
    return f;
  }
  else
  {
    return [f = std::forward<F>(f), tail = compose(std::forward<Fs>(fs)...)]
      (auto&& ... obj)
    noexcept(noexcept(f(compose(std::forward<Fs>(fs)...)(std::forward<decltype(obj)>(obj)...))))
    -> decltype(f(compose(std::forward<Fs>(fs)...)(std::forward<decltype(obj)>(obj)...)))
    {
      return f(tail(std::forward<decltype(obj)>(obj)...));
    };
  }
}

template <typename F>
inline auto negate(F&& f)
{
  return [f = std::forward<F>(f)](auto&& ... obj)
         LIFT_THRICE(!f(std::forward<decltype(obj)>(obj)...));
}

template <typename T>
inline auto equals(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj == t);
}

template <typename T>
inline auto less_than(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj < t);
}

template <typename T>
inline auto less_equal(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj <= t);
}

template <typename T>
inline auto greater_than(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj > t);
}

template <typename T>
inline auto greater_equal(T&& t)
{
  return [t = std::forward<T>(t)](const auto& obj) LIFT_THRICE(obj >= t);
}

template <typename ... Fs>
inline auto when_all(Fs... fs)
{
  return [=](const auto& ... obj) LIFT_THRICE((fs(obj...) && ...));
}

template <typename ... Fs>
inline auto when_any(Fs ... fs)
{
  return [=](const auto& ... obj) LIFT_THRICE((fs(obj...) || ...));
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
    noexcept(noexcept(true == predicate(obj...))
             && noexcept(action(std::forward<decltype(obj)>(obj)...)))
    -> void
  {
    if (predicate(obj...))
    {
      action(std::forward<decltype(obj)>(obj)...);
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
  noexcept(noexcept(true == predicate(obj...))
           && noexcept(t_action(std::forward<decltype(obj)>(obj)...))
           && noexcept(t_action(std::forward<decltype(obj)>(obj)...)))
  {
    if (predicate(obj...))
    {
      t_action(std::forward<decltype(obj)>(obj)...);
    }
    else
    {
      f_action(std::forward<decltype(obj)>(obj)...);
    }
  };
}

template <typename ... Fs>
inline auto do_all(Fs ... fs)
{
  return [=](auto&& ... obj)
  noexcept(noexcept(((void)(fs(std::forward<decltype(obj)>(obj)...)),...)))
  {
    ((void)(fs(std::forward<decltype(obj)>(obj)...)),...);
  };
}

}
#endif //HIGHER_ORDER_FUNCTIONS_LIFT_HPP

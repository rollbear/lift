#ifndef HIGHER_ORDER_FUNCTIONS_LIFT_HPP
#define HIGHER_ORDER_FUNCTIONS_LIFT_HPP

#include <utility>

namespace lift {

template <typename F, typename ... Fs>
inline auto compose(F f, Fs... fs)
{
  if constexpr (sizeof...(fs) == 0)
  {
    return f;
  }
  else
  {
    auto tail = compose(std::move(fs)...);
    return [f = std::move(f), tail = std::move(tail)]
      (auto&& ... obj)
    {
      return f(tail(std::forward<decltype(obj)>(obj)...));
    };
  }
}

template <typename F>
inline auto negate(F f)
{
  return [f = std::move(f)](auto&& ... obj)
  {
    return !(f(std::forward<decltype(obj)>(obj)...));
  };
}

template <typename T>
inline auto equals(T t)
{
  return [t = std::move(t)](const auto& obj) { return obj == t; };
}

template <typename T>
inline auto less_than(T t)
{
  return [t = std::move(t)](const auto& obj) { return obj < t; };
}

template <typename T>
inline auto less_equal(T t)
{
  return [t = std::move(t)](const auto& obj) { return obj <= t; };
}

template <typename T>
inline auto greater_than(T t)
{
  return [t = std::move(t)](const auto& obj) { return obj > t; };
}

template <typename T>
inline auto greater_equal(T t)
{
  return [t = std::move(t)](const auto& obj) { return obj >= t; };
}

template <typename ... Fs>
inline auto when_all(Fs... fs)
{
  return [=](const auto& ... obj) { return (fs(obj...) && ...);};
}

template <typename ... Fs>
inline auto when_any(Fs ... fs)
{
  return [=](const auto& ... obj) { return (fs(obj...) || ...);};
}

template <typename ... Fs>
inline auto when_none(Fs ... fs)
{
  return negate(when_any(std::move(fs)...));
}

template <typename Predicate, typename Action>
inline auto if_then(Predicate predicate, Action action)
{
  return [predicate = std::move(predicate), action = std::move(action)]
    (auto&& ... obj)
  {
    if (predicate(obj...))
    {
      action(std::forward<decltype(obj)>(obj)...);
    }
  };
}

template <typename Predicate, typename TAction, typename FAction>
inline auto if_then_else(Predicate predicate,
                         TAction t_action,
                         FAction f_action)
{
  return [
    predicate = std::move(predicate),
    t_action = std::move(t_action),
    f_action = std::move(f_action)
  ](auto&& ... obj)
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
  {
    ((void)(fs(std::forward<decltype(obj)>(obj)...)),...);
  };
}

}
#endif //HIGHER_ORDER_FUNCTIONS_LIFT_HPP

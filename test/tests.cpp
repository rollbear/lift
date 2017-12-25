#include <lift.hpp>
#include <catch.hpp>

TEST_CASE("compose")
{
  auto to_string = [](auto t) { return std::to_string(t);};
  WHEN("called with several unary functions") {
    THEN("they are chained last to first") {
      auto string_plus_one_twice = lift::compose(to_string,
                                                 [](int i) { return i + i; },
                                                 [](int i) { return i + 1; });
      REQUIRE(string_plus_one_twice(2) == "6");
    }
  }
  AND_WHEN("called with a unary and a binary function") {
    THEN("the composition is binary, calling the last with two values and the first with the result") {
      auto string_add = lift::compose(to_string,
                                      [](int x, int y) { return x + y; });
      REQUIRE(string_add(3, 2) == "5");
    }
  }
  AND_WHEN("called with a binary and a unary function")
  {
    THEN("the composition is binary, calling the last with each value, and the first with the two results")
    {
      auto cmp = lift::compose(std::less<>{},
                               [](const auto& x) { return x.first;});
      REQUIRE(cmp(std::pair(1,3), std::pair(3,1)));
    }
  }
  AND_WHEN("functions are non-copyable")
  {
    THEN("they are moved")
    {
      auto f1 = [x = std::make_unique<int>(3)](int p) mutable { *x += p; return std::move(x);};
      auto f2 = [y = std::make_unique<std::string>("foo")](auto p) { return *y + std::to_string(*p);};
      auto func=lift::compose(std::move(f2), std::move(f1));
      REQUIRE(func(5) == "foo8");
    }
  }
}

TEST_CASE("negate logically inverts the return value of its function")
{
  auto is_three = [](int n) { return n == 3; };
  REQUIRE(lift::negate(is_three)(2));
  REQUIRE_FALSE(lift::negate(is_three)(3));
}

TEST_CASE("equals")
{
  int* p = new int{3};
  REQUIRE_FALSE(lift::equals(std::unique_ptr<int>(p))(std::unique_ptr<int>()));
  REQUIRE(lift::equals(std::unique_ptr<int>())(nullptr));
}

TEST_CASE("when_all")
{
  WHEN("all predicates are true")
  {
    THEN("they are called in order")
    {
      int num = 0;

      REQUIRE(lift::when_all([&](int i) { num += i; return num == 1;},
                             [&](int i) { num += i; return num == 2;},
                             [&](int i) { num += i; return num == 3;})(1));
      REQUIRE(num == 3);
    }
  }
  WHEN("some predicates are false")
  {
    THEN("if first is false it's the only one tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_all([&](int i) { return ++num == i;},
                                   [&](int i) { return ++num == i;},
                                   [&](int i) { return ++num == i;})(0));
      REQUIRE(num == 1);
    }
    THEN("if all but last are true, all are tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_all([&](int i) { return ++num < i;},
                                   [&](int i) { return ++num < i;},
                                   [&](int i) { return ++num < i;})(3));
      REQUIRE(num == 3);
    }
    THEN("if first is the only true one, the two first are tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_all([&](int i) { return ++num == i;},
                                   [&](int i) { return ++num == i;},
                                   [&](int i) { return ++num == i;})(1));
      REQUIRE(num == 2);
    }
  }
  WHEN("a predicate is not copyable")
  {
    THEN("it is moved")
    {
      auto pred = lift::when_all([x = std::make_unique<int>(3)](auto& p) { return p == x;});
      REQUIRE_FALSE(pred(nullptr));
    }
  }
}

TEST_CASE("when_any")
{
  WHEN("all predicates are false")
  {
    THEN("they are called in order")
    {
      int num = 0;

      REQUIRE_FALSE(lift::when_any([&](int i) { num += i; return num == 0;},
                                   [&](int i) { num += i; return num == 0;},
                                   [&](int i) { num += i; return num == 0;})(1));
      REQUIRE(num == 3);
    }
  }
  WHEN("some predicates are true")
  {
    THEN("if first is true it's the only one tested")
    {
      int num = 0;
      REQUIRE(lift::when_any([&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;})(1));
      REQUIRE(num == 1);
    }
    THEN("if all but last are false, all are tested")
    {
      int num = 0;
      REQUIRE(lift::when_any([&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;})(3));
      REQUIRE(num == 3);
    }
    THEN("if first is the only false one, the two first are tested")
    {
      int num = 0;
      REQUIRE(lift::when_any([&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;},
                             [&](int i) { return ++num == i;})(2));
      REQUIRE(num == 2);
    }
  }
  WHEN("a predicate is not copyable")
  {
    THEN("it is moved")
    {
      auto pred = lift::when_any([x = std::make_unique<int>(3)](auto& p) { return p == x;});
      REQUIRE_FALSE(pred(nullptr));
    }
  }
}

TEST_CASE("when_none")
{
  WHEN("all predicates are false")
  {
    THEN("they are called in order")
    {
      int num = 0;

      REQUIRE(lift::when_none([&](int i) { num++; return num == i;},
                              [&](int i) { num++; return num == i;},
                              [&](int i) { num++; return num == i;})(0));
      REQUIRE(num == 3);
    }
  }
  WHEN("some predicates are true")
  {
    THEN("if first is true it's the only one tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_none([&](int i) { return ++num == i;},
                                    [&](int i) { return ++num == i;},
                                    [&](int i) { return ++num == i;})(1));
      REQUIRE(num == 1);
    }
    THEN("if all but last are false, all are tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_none([&](int i) { return ++num == i;},
                                    [&](int i) { return ++num == i;},
                                    [&](int i) { return ++num == i;})(3));
      REQUIRE(num == 3);
    }
    THEN("if first is the only false one, the two first are tested")
    {
      int num = 0;
      REQUIRE_FALSE(lift::when_none([&](int i) { return ++num > i;},
                                    [&](int i) { return ++num > i;},
                                    [&](int i) { return ++num > i;})(1));
      REQUIRE(num == 2);
    }
  }
  WHEN("a predicate is not copyable")
  {
    THEN("it is moved")
    {
      auto pred = lift::when_none([x = std::make_unique<int>(3)](auto& p) { return p == x;});
      REQUIRE(pred(nullptr));
    }
  }
}

TEST_CASE("if_then")
{
  WHEN("predicate is true")
  {
    THEN("action is called with value")
    {
      int num = 0;
      lift::if_then(lift::equals(3), [&](int i) { num = i;})(3);
      REQUIRE(num == 3);
    }
    AND_THEN("multi parameter action is called with all values")
    {
      int num = 0;
      lift::if_then(lift::compose(lift::equals(3),
                                  [](int x, int y) { return x + y;}),
                    [&](int x, int y) { num = x - y;})(4,-1);
      REQUIRE(num == 5);
    }
  }
  WHEN("predicate is false")
  {
    THEN("action is not called")
    {
      int num = 0;
      lift::if_then(lift::equals(3), [&](int i) { num = i;})(4);
      REQUIRE(num == 0);

    }
  }
}

TEST_CASE("if_then_else")
{
  int tnum = 0;
  int fnum = 0;
  auto condition_if_3 = lift::if_then_else(lift::equals(3),
                                           [&](int i) { tnum = i;},
                                           [&](int i) { fnum = i;});
  WHEN("predicate is true")
  {
    THEN("true_action is called with value")
    {
      condition_if_3(3);
      REQUIRE(tnum == 3);
      REQUIRE(fnum == 0);
    }
  }
  WHEN("predicate is false")
  {
    THEN("action is not called")
    {
      condition_if_3(4);
      REQUIRE(tnum == 0);
      REQUIRE(fnum == 4);
    }
  }
}

TEST_CASE("do_all")
{
  WHEN("there are several functions") {
    THEN("they are called in sequence") {
      int num = 0;
      lift::do_all([&](int i) {
                     num += i;
                     REQUIRE(num == 1);
                   },
                   [&](int i) {
                     num += i;
                     REQUIRE(num == 2);
                   },
                   [&](int i) {
                     num += i;
                     REQUIRE(num == 3);
                   })(1);
      REQUIRE(num == 3);
    }
  }
  WHEN("functions are non copyable")
  {
    THEN("they are moved")
    {
      int n = 0;
      lift::do_all([x=std::make_unique<int>(3),&n](auto p) { n=p+*x;})(5);
      REQUIRE(n == 8);
    }
  }
}


# *lift*

A C++17 library of simple `constexpr` higher order functions of predicates
and for making functional composition easier. These help reduce code
duplication and improve clarity, for example in code using STL
`<algorithm>`.

*lift* uses advanced C++17 features not supported by all compilers.
On 2018-04-29, it is known to work with:
* clang++-5, clang++-6
* gcc-7

Notably, it has been shown not to work with:
* any version of MSVC
* gcc-trunk (9.0.0 20180428 ) ICEs
* gcc-8 prebuild (svn 259748) [Bug 85569](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85569)
* clang++-7 trunk (trunk 331144) (llvm/trunk 331161) [Bug 37290](https://bugs.llvm.org/show_bug.cgi?id=37290)


### Example of use:

```Cpp
struct Employee {
  std::string name;
  unsigned    number;
};

const std::string& select_name(const Employee& e) { return e.name; }
unsigned select_number(const Employee& e) { return e.number; }

std::vector<Employee> staff;

// sort employees by name
std::sort(staff.begin(), staff.end(),
          lift::compose(std::less<>{}, select_name);
         
// retire employee number 5
auto i = std::find_if(staff.begin(), staff.end(),
                      lift::compose(lift::equal(5),
                                    select_number));
if (i != staff.end()) staff.erase(i);
```

## Videos
* Intro to the ideas, recorded at [SwedenC++](https://www.meetup.com/swedencpp) Stockholm meetup in
January 2018. [YouTube (30m)](https://www.youtube.com/watch?v=r1N3PElFDeI) 
* Conference talk - *"Higher Order Functions for Ordinary C++ Developers"*, recorded at
[NDC{Oslo}](https://ndcoslo.com/talk/higher-order-functions-for-ordinary-c-developers/)
 June 2018. [YouTube (1h)](https://www.youtube.com/watch?v=kcBlSmo3Xlk)

## Higher order functions

* [`equal`](#equal)
* [`not_equal`](#not_equal)
* [`less_than`](#less_than)
* [`less_equal`](#less_equal)
* [`greater_than`](#greater_than)
* [`greater_equal`](#greater_equal)
* [`negate`](#negate)
* [`compose`](#compose)
* [`when_all`](#when_all)
* [`when_any`](#when_any)
* [`when_none`](#when_none)
* [`if_then`](#if_then)
* [`if_then_else`](#if_then_else)
* [`do_all`](#do_all)

## Macros

* [`LIFT_FUNCTION`, `LIFT`](#LIFT_FUNCTION)

### <A name="equal"/>`lift::equal(value)`

Returns a predicate that compares its argument for equality with `value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is equality comparable with `value`.
The comparison is made as `argument == value`.

#### Example

```Cpp
std::vector<int> v;
...
if (std::any_of(std::begin(v), std::end(v), lift::equal(0)))
{
  ...
}
```

### <A name="not_equal"/>`lift::not_equal(value)`

Returns a predicate that compares its argument for inequality with `value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is not-equal comparable with `value`.
The comparison is made as `argument != value`.

#### Example

```Cpp
std::vector<int> v;
...
auto num = std::count_if(std::begin(v), std::end(v), lift::not_equal(0));
```

### <A name="less_than"/>`lift::less_than(value)`

Returns a predicate that tells if its argument is less than `value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is less-than comparable with `value`.
The comparison is made as `argument < value`.

#### Example

```Cpp
std::vector<int> v;
...
auto i = std::remove_if(std::begin(v), std::end(v), lift::less_than(0)));
v.erase(i, v.end());
```

### <A name="less_equal"/>`lift::less_equal(value)`

Returns a predicate that tells if its argument is less than or equal to`value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is less-equal comparable with `value`.
The comparison is made as `argument <= value`.

#### Example

```Cpp
std::vector<int> v;
...
auto i = std::remove_if(std::begin(v), std::end(v), lift::less_equal(0)));
v.erase(i, v.end());
```

### <A name="greater_than"/>`lift::greater_than(value)`

Returns a predicate that tells if its argument is greater than `value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is greater-than comparable with `value`.
The comparison is made as `argument > value`.

#### Example

```Cpp
std::vector<int> v;
...
auto i = std::remove_if(std::begin(v), std::end(v), lift::greater_than(0)));
v.erase(i, v.end());
```

### <A name="greater_equal"/>`lift::greater_equal(value)`

Returns a predicate that tells if its argument is greater than or equal to`value`.
`value` is forwarded into the predicate. The predicate is templated
and works for any type that is greater-equal comparable with `value`.
The comparison is made as `argument >= value`.

#### Example

```Cpp
std::vector<int> v;
...
auto i = std::remove_if(std::begin(v), std::end(v), lift::greater_equal(0)));
v.erase(i, v.end());
```

### <A name="negate"/>`lift::negate(in_predicate)`

Returns a predicate that is the logical negation of its argument `in_predicate`.
The returned predicate has the same arity as `in_predicate`. `in_predicate`
is forwarded into the returned predicate. The predicate is templated and
can be called with all types that `in_predicate` can be called with.
`in_predicate` may not mutate its state when called.

#### Example

```Cpp
constexpr auto unary_not_3 = lift::negate(lift::equal(3));
static_assert(unary_not_3(5));
constexpr auto binary_ge = lift::negate(std::less<>{});
static_assert(binary_ge(4,3));
```

### <A name="compose"/>`lift::compose(functions...)`

Returns a function object that returns the value of calling each
of the functions with the return value of the next.

For example, for unary functions `f1`, `f2`, `f3`,
`lift::compose(f1, f2, f3)(value)` calls `f1(f2(f3(value)))`.

For N-ary functions (where N > 1), it is a bit more complicated.

For example, with functions `f1(int, int)->int` and `f2(int)->int`,
`lift::compose(f1, f2)` yields a binary function object. When
called with `value1` and `value2`, it calls `f1(f2(value1), f2(value2))`.

Composition in the other order, `lift::compose(f2, f1)` yields a binary
function object which, when called with `value1` and `value2`, calls
`f2(f1(value1, value2))`.

The depth can be increased, utilizing both forms of composition. If there
is also a unary funcion `f3(int) -> int`, then
`compose(f3, f1, f2)` yields a binary function, which when called with
`value1` and `value2` calls `f3(f1(f2(value1), f2(value2)))`.

It is not possible to compose several N-ary functions (where N > 1.)

None of the `functions` may mutate their state when called.

#### Example

```Cpp
struct Employee {
  std::string name;
  unsigned    number;
};

const std::string& select_name(const Employee& e) { return e.name; }

std::vector<Employee> staff;

auto by_name = lift::compose(std::less<>{}, // less than comparison on any type
                             select_name); // gets the name member from Employee
std::sort(std::begin(staff), std::end(staff), by_name);

auto i = std::find_if(std::begin(staff), std::end(staff),
                      lift::compose(lift::equal("Santa Claus"),
                                    select_name));
if (i != std::end(staff)) // santa works here!
...                                    
```
### <A name="when_all"/>`lift::when_all(predicates...)`

Returns a predicate that is true when all predicates are true. All
predicates must have the same arity. Normal logical short circuiting
applies, so if any predicate returns false, the predicates after are
not called. The returned predicate is templated and can be called with
any types that all predicates can be called with. The predicates may
not mutate their state when called.

#### Example

```Cpp
auto points_to = [](auto value) {
 return lift::when_all([](auto* p) { return p; },
                       [=](auto* p) { return *p == value; });                
};

int n = 4;
int m = 3;

assert(points_to(3)(nullptr) == false); // 2nd predicate not called
assert(points_to(3)(&n) == false); // 2nd predicate returns false
assert(points_to(3)(&m)); // both predicates returns true
```

### <A name="when_any"/>`lift::when_any(predicates...)`

Returns a predicate that is true when at least one of the predicates
are true. All predicates must have the same arity. Normal logical short
circuiting applies, so if any predicate returns true, the predicates
after are not called. The returned predicate is templated and can be
called with any types that all predicates can be called with. The
predicates may not mutate their state when called.

#### Example

```Cpp
constexpr auto null_or_empty = lift::when_any(
                                 [](const char* p) { return !p; },
                                 [](const char* p) { return *p == '\0'; });                


static_assert(null_or_empty(nullptr));
static_assert(null_or_empty(""));
static_assert(!null_or_empty("foo"));
```

### <A name="when_none"/>`lift::when_none(predicates...)`

Returns a predicate that is true when none of the predicates
are true, i.e., it is the negation of [`when_any`](#when_any).
All predicates must have the same arity. Normal logical short
circuiting applies, so if any predicate returns true, the predicates
after are not called. The returned predicate is templated and can be
called with any types that all predicates can be called with. The
predicates may not mutate their state when called.

#### Example

```Cpp
constexpr auto nonempty_string = lift::when_none(
                                 [](const char* p) { return !p; },
                                 [](const char* p) { return *p == '\0'; });                


static_assert(!nonempty_string(nullptr));
static_assert(!nonempty_string(""));
static_assert(nonempty_string("foo"));
```

### <A name="if_then"/>`lift::if_then(predicate, action)`

Returns a function object that calls `action` if a call to `predicate`
returns true. `action` and `predicate` must be callable with the same
parameters. The function object is templated and can be called with
any types that `action` and `predicate` can be called with.

`predicate` must not mutate its state when called.
`action` may mutate its state when called.

The returned function object does not return anything.

#### Example

```Cpp
std::vector<int> v;
...
auto make_positive = lift::if_then(lift::less_than(0),
                                   [](auto& x) { x = -x;});
std::for_each(std::begin(v), std::end(v), make_positive);                                   
```

### <A name="if_then_else"/>`lift::if_then_else(predicate, action1, action2)`

Returns a function object that calls `action1` if a call to `predicate`
returns true and calls `action2` otherwise. `action1`, `action2` and
`predicate` must be callable with the same parameters. The function
object is templated and can be called with any types that `action1`,
`action2` and `predicate` can be called with.

`predicate` must not mutate its state when called.
`action1` may mutate its state when called.
`action2` may mutate its state when called.

The returned function returns the common type of `action1` and `action2`.

#### Example

```Cpp
std::vector<int> v;
...
auto negatives_to_zero = lift::if_then_else(lift::less_than(0),
                                            [](const auto&) { return 0;},
                                            [](const auto& x) { return x;});
std::vector<int> result;
std::transform(std::begin(v), std::end(v),
               std::back_inserter(result),
               negatives_to_zero);                                            
```

### <A name="do_all"/>`lift::do_all(actions...)`

Returns a function object that calls all `actions` in order.
All `actions` must be callable with the same parameters. The function
object is templated and can be called with any types that all `actions`
can be called with.

`actions` may mutate their state when called.

The returned function does not return anything when called.

#### Example

```Cpp
auto print_dots = [](auto& stream, size_t width) {
  return lift::do_all([&stream,width](const std::string& s){while (width-- > s.length()) os << '.';},
                      [&stream](const std::string& s) { stream << s; });
};
std::vector<std::string> v;
...
std::for_each(std::begin(v), std::end(v),print_dots(std::cout, 20));
```

### <A name="LIFT_FUNCTION"/>`LIFT_FUNCTION(function)`

Lifts overloaded functions named `function` to one callable that can
be used with other higher order functions. A short form `LIFT(function)`
is also available.

#### Example

```Cpp
std::vector<int> vi;
...
std::vector<std::string> vs;
std::transform(std::begin(vi), std::end(vi),
               std::back_inserter(vs),
               LIFT(std::to_string)); // lift overloaded set of 9 functions
```

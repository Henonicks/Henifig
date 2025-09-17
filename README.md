# Henifig

## Introduction

The name (pronounced as "H*e*n-if-ig") comes from my nickname, Henonicks
(pronounced as "Hen-*o*h-nicks"), combined with "config".
This C++ program interprets a given file in this format:

```henifig
# One-line comment
[# Multi
   line
   comment
#]

# Variable:
/var \[str\]\   | "\"val" [# the strings will be concatenated. #] "values"
#~~~~^~~~~^
/var \{char\}\  | 'v'
#~~~~^~~~~~^
# Escaping needed for the brackets if they're meant as part of the name.

/var int\       | -16
/var number\    | 1.618
/var bool\      | true
/declaration\ # can be useful as a runtime "#ifdef" check
# The type at the end is purely a part of the name.


# Array:
/arr["Try", 2, "erase", 8, .67, false]\
#~~~~~~~~~~~~~~~~~~~~~~~~~~^
# Treated as "0.67".

# Map (name-keyed array):
/map{$"if there's" | "so much", $"love", $"in this place" | {$"well then" | "why?!"},
     $"Are we overwhelmed" | ["with", "this", "hate?"]}\

# Multi-declaration:
/var1\ | "I won't be forsaken"; /var2\ | "The beast has been awakened"
[#~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
Important if you want to declare multiple variables in the same line!
Otherwise omittable.#]
```

The indentation was added to "prettify" the pseudo config file. It's recommended to use spaces instead of
tabs, although both are allowed.
It doesn't matter where and how many spaces there are unless they're used as values/keys.
For example, in the snippet:

```henifig
/var string\ | "insane concept" [# comment #] "ain't it"
[#~~^~~~~~~~~~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
The only spaces taken into account #]
```

All the non-string spaces are completely ignored.
This project uses CMake with C++17. It is currently in-development, so some of the features are missing
and this project may be unstable, though it is usable now.

One of the planned features is conversion to json.

## Usage

You can read data in multiple ways:

```cpp
henifig::config_t cfg_by_filename("config.hfg");
// recommended as whenever a parsing error occurs, a henifig config constructed
// like this will include the filename in the exception.

std::ifstream file("config.hfg");
henifig::config_t cfg_by_ifstream;
cfg_by_ifstream << file;

std::string content = "/decl\\\n/hello\\ | \"Hello, \" \"World!";
henifig::config_t cfg_by_content;
cfg << content;
```

For future reference, we'll have a `henifig::config_t` object called `cfg`, already initialised.

Now let's pull some values out. We'd do that similarly to an `std::string`-keyed `std::map`:

```cpp
const std::string& hello = cfg["hello"];
```

To use the library you should probably learn the types it provides.
The `[]` operator of `henifig::config_t` returns a `const` variant wrapper:

```cpp
const henifig::value_t& operator [](std::string_view key);
// a wrapper around value_variant

using value_array = std::vector <value_t>;
// an array index struct, convertible to a value_array
using value_map = std::map <std::string, value_t>;
// same as array_t but for a value_map

using value_variant = std::variant
<unset_t, declaration_t, std::string, char, double, bool, array_t, map_t>;
//^ an empty struct, used as a way for the library to know that the value hasn't been assigned
//~~~~~~~~^ an empty struct, used as a way to know that the object which doesn't need a value exists
// ...
class value_t {
public:
	value_variant value;
// ...
```

As you can see, `henifig::value_t` contains an `std::variant` inside and wraps around it
so you can use it like a normal variant. Why wrap around then? I did this so that
C-style arrays and integers are comparable against them. This means I allowed this:

```cpp
const bool is_hello = cfg["hello"] == "hello";
// gets an std::string, converts it into a const char*, compares to "hello".
const bool is_1 = cfg["1"] == 1;
// gets a double, converts it into an int, compares to `1`.
```

And as you could see above, if you want to access an array or a map, simply do:

```cpp
henifig::value_array arr = cfg["arr"];
// gets an array_t, converts into `henifig::value_array`
henifig::value_map   map = cfg["map"];
// gets a  map_t,   converts into `henifig::value_map`
```

Since the values are obtained from an `std::variant` you can also pull by reference:

```cpp
const double& number = cfg["number"];

const double& number_get = cfg["number"].get <double>();
// does the same thing with one less call. may be needed to resolve ambiguity issues,
// for example when using std::cout

#include "henifig/get.hpp" // not included by default
const auto& number_stdget = std::get <double>(cfg["number"]);
// get.hpp has an overload for std::get to work with value_t objects
// if you'd like to use it for some reason... it just calls value_t::get <T>().
```

## Testing

If you'd like to test if a henifig can be parsed before using it in your program, build the code in
`henifig/test` and run it with an argument being the path to the file:

    ./cfgtest path/to/config.hfg

If left with no arguments, it will test against test.hfg alongside the `main.cpp`.

## Handling errors

Whenever a parse attempt fails, a `henifig::parse_exception` is thrown. A termination report may look something like:

```
terminate called after throwing an instance of 'henifig::parse_exception'
  what():  Parsing error in `../test.hfg` on 4:12 - unexpected expression (code: 33).
```

You will only get the filename given back to you if you've read the config using the constructor of
`henifig::config_t` which takes an `std::string_view`.

## Compilation

todo

## Debugging

Throughout the parsing stage of a henifig config, there are calls to `henifig::cout` (which is an `std::cout` wrapper)
which print information about the iteration. You can turn the wrapper on/off using the
`henifig::process_logger::set_enabled(bool);` function. You can also use `henifig::cout` if you
`#include "henifig/internal/logger.hpp"`.

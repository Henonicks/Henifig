# Henifig

## Introduction

The name (pronounced as "H*e*n-if-ig") comes from my nickname, Henonicks
(pronounced as "Hen-*o*h-nicks"), combined with "config".
This C++ program interprets a given file in this format:

```
# One-line comment
[# Multi
   line
   comment
#]

# Variable:
/var \[str\]\ | "\"val" [# the strings will be concatenated. #] "values"
#~~~~^~~~~^
/var \{char\}\  | 'v'
#~~~~^~~~~~^
# Escaping needed for the brackets if they're meant as part of the name.

/var int\     | -16
/var number\  | 1.618
/var bool\    | true
/declaration\ # can be useful as a runtime "#ifdef" check
# The type at the end is purely a part of the name.


# Array:
/arr["Try\"", "to", "erase"]\

# Tuple:
/tuple{"We all must", 'b', "dreaming this life", 8, .67, false}\
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
# Treated as "0.67".

# Multi-declaration:
/var1\ | "I won't be forsaken"; /var2\ | "The beast has been awakened"
[#~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
Important if you want to declare multiple variables in the same line!
Otherwise omittable.#]
```

The indentation was added to "prettify" the pseudo config file.
It doesn't matter where and how many spaces there are unless they're used as values/keys.

This project uses CMake with C++-\<version decided later\>. It is currently in-development so
a lot of stuff is missing, including most of the functionality, therefore
this project is not just unstable, but is unusable.

One of the planned features is conversion to json.

## Compilation

todo

## Debugging

todo

#  Musings on PlainCalc3

## To-dos

Before release, make sure all my sources have the license comment.

Static analysis gives some warnings about `std::move` vs. `std::forward`.
Figure out what that's about, or at least report it.

Currently using Boost 1.88, because I get a compile error in the parser
with Boost 1.89.

Can I get rid of the maps in Built-ins.cpp?  I do a reverse lookup, finding
the function given the name, in `BinaryFuncNode::ToDictionary()`,
`UnaryFuncNode::ToDictionary`, `NaryFuncNode::ToDictionary`.  This issue
does not arise for iterations.  Also, `BuildTreeFromDictionary` does forward
lookups in a situation without a context parameter.

Maybe `UserFuncNode` should record the right hand side expression, so it
would not need to be looked up during evaluation?  But then in a recursive
function, I would not have a tree any more, there would be back links.


## Representation of syntax trees as dictionaries

In PlainCalc2, the right hand side of a user-defined function is just stored
as a string both in `SCalcState` and in the file format.  But in PlainCalc3,
that won't work.  For example, suppose you did this:

```
T=100 =
100
what(x) = 2 x =
Defined function 'what'
foo(z) = what(T) + z =
Defined function 'foo'
T=1 =
1
foo(3) =
203
```

Let's say you first restore variables, so `T` is restored to `1`, and then
you try to restore functions by reconstructing function definitions and
calculating them.  If you do it alphabetically, attempting to define `foo`
before `what` has been defined will fail.  If you somehow arrange to do it
in the other order, the definitions will succeed, but `foo(z)` will be
computing `2 + z` instead of `200 + z`.  So we need a way to have a faithful
representation of a calculation syntax tree as a dictionary or string.


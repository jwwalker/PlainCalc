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

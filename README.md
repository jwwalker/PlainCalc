# README #

PlainCalc is a buttonless, algebraic-entry, mathematical calculator for macOS
with variables and functions. To use PlainCalc, type an expression or
assignment and press Return or Enter. Unlike most calculators I've seen,
PlainCalc allows you to indicate multiplication by juxtaposition, e.g., you
can write 2πr instead of 2 * π * r.  Choose **PlainCalc Help** from the
**Help** menu of the app for more information.

The major changes in PlainCalc 3.0 relative to version 2.5 include:

* Rewritten parsing engine that gives better error messages.
* New built-in functions for statistical median, standard deviation, etc.
* Ability to calculate mathematical summation formulas.
* Ability to print the worksheet.

To build this project with Xcode, you will also need [Boost](https://www.boost.org),
whose Parser parsing library is used in the calculation engine.  In Xcode's
settings, under Locations, you will need to define a custom path named "Boost"
pointing at your Boost hierarchy.  At this writing, I am using Boost 1.89.0.

The project is set to deploy on macOS 13.7 or later.

The source code is under the zlib/libPNG public license.

PlainCalc is available for free from the Mac App Store.

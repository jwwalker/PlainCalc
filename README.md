# README #

PlainCalc is a buttonless, algebraic-entry, mathematical calculator for Mac OS X with variables and functions. To use PlainCalc, type an expression or assignment and press Return or Enter. Unlike most calculators I've seen, PlainCalc allows you to indicate multiplication by juxtaposition, e.g., you can write 2πr instead of 2 * π * r.

To build this project with Xcode, you will also need [Boost](http://www.boost.org), whose Spirit parsing library is used in the calculation engine.  In Xcode's preferences, under Locations, you will need to define a custom path named "Boost" pointing at your Boost hierarchy.

The project is set to deploy on Mac OS X 10.7 or later, with x86_64 architecture.

The source code is under the zlib/libPNG public license.
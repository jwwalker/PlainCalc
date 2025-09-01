//  Built-ins.cpp
//  PlainCalc3
//
//  Created by James Walker on 8/22/25.
//  
//
/*
	Copyright (c) 2025 James W. Walker

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from
	the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software
		in a product, an acknowledgment in the product documentation would be
		appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and must not be
		misrepresented as being the original software.

	3. This notice may not be removed or altered from any source distribution.
*/

#import "Built-ins.hpp"

#import "Average.hpp"
#import "GeometricMean.hpp"
#import "HarmonicMean.hpp"
#import "Max.hpp"
#import "Median.hpp"
#import "Min.hpp"
#import "Product.hpp"
#import "StandardDeviation.hpp"
#import "Sum.hpp"
#import "Variance.hpp"

#import <math.h>

static double rad( double inDegrees )
{
	return inDegrees * (M_PI / 180.0);
}

static double deg( double inRadians )
{
	return inRadians * (180.0 / M_PI);
}


const std::map< std::string, double >&	BuiltInConstants()
{
	static std::map< std::string, double > constants
	{
			{ "pi", M_PI },
			{ "π", M_PI },		// the pi character in UTF-8
			{ "%", 0.01 },
			{ "e", exp( 1.0 ) }
	};
	return constants;
}

const symbolsJW< UnaryFunc >&	BuiltInUnarySyms()
{
	static symbolsJW< UnaryFunc > funcs
	{
		{ "atan", ::atan },
		{ "acos", ::acos },
		{ "asin", ::asin },
		{ "sin", ::sin },
		{ "cos", ::cos },
		{ "tan", ::tan },
		{ "ceil", ::ceil },
		{ "floor", ::floor },
		{ "round", round },
		{ "fabs", ::fabs },
		{ "abs", ::fabs },
		{ "log", ::log },
		{ "ln", ::log },
		{ "log10", ::log10 },
		{ "log2", log2 },
		{ "exp", ::exp },
		{ "rad", rad },
		{ "deg", deg },
		{ "sqrt", std::sqrt },
		{ "√", std::sqrt }	// sqrt character in UTF-8
	};
	return funcs;
}

const symbolsJW< BinaryFunc >&	BuiltInBinarySyms()
{
	static symbolsJW< BinaryFunc > funcs
	{
		{ "atan2", ::atan2 },
		{ "hypot", hypot },
		{ "pow", ::pow }
	};
	return funcs;
}

const symbolsJW< NaryFunc >&	BuiltInNarySyms()
{
	static symbolsJW< NaryFunc > funcs
	{
		{ "average", Average },
		{ "mean", Average },
		{ "GM", GeometricMean },
		{ "HM", HarmonicMean },
		{ "max", Max },
		{ "min", Min },
		{ "product", Product },
		{ "SD", StandardDeviation },
		{ "σ", StandardDeviation },
		{ "sum", Sum },
		{ "Var", Variance },
		{ "median", Median },
	};
	return funcs;
}


const boost::parser::symbols< IterationKind >&	BuiltInIterationSyms()
{
	static boost::parser::symbols< IterationKind > funcs
	{
		{ "summation", IterationKind::summation },
		{ "∑", IterationKind::summation },
		{ "multiplication", IterationKind::multiplication },
		{ "∏", IterationKind::multiplication }
	};
	return funcs;
}

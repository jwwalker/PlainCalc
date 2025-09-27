//  Built-ins.hpp
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

#ifndef Built_ins_hpp
#define Built_ins_hpp

#import <map>
#import <string>
#import <algorithm>
#import <stdexcept>
#import <optional>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang diagnostic ignored "-Wunsafe-buffer-usage"
#import <boost/parser/parser.hpp>
#pragma clang diagnostic pop

using UnaryFunc	=	double (*)( double );
using BinaryFunc =	double (*)( double, double );
using NaryFunc =	double (*)( const std::vector<double>& );

enum class IterationKind : char
{
	summation,
	multiplication
};

/*
	This symbol table template is my own version of the symbols<T> template
	provided by Boost.Parser. The standard symbol table class didn't provide
	a way to do lookups without having a parser "Context" parameter, or to
	iterate over all the symbol/value pairs in the table.
*/
template<typename T>
struct symbolsJW : boost::parser::parser_interface<boost::parser::symbol_parser<T>>
{
	symbolsJW( std::initializer_list<std::pair<const std::string, T>> il)
	{
		this->parser_.initial_elements_.resize(il.size());
		std::copy(il.begin(), il.end(),
				  this->parser_.initial_elements_.begin());
	}
	
	bool Contains( std::string_view str ) const
	{
		std::optional<T> maybeValue = Lookup( str );
		return maybeValue.has_value();
	}
	
	T at( std::string_view str ) const
	{
		std::optional<T> maybeValue = Lookup( str );
		
		if (not maybeValue.has_value())
		{
			std::string msg( "Symbol " );
			msg += str;
			msg += " not found";
			throw std::out_of_range( msg );
		}
		return maybeValue.value();
	}
	
	std::optional<T> Lookup( std::string_view str ) const
	{
		std::optional<T> result;
		auto foundIt = std::find_if( this->parser_.initial_elements_.begin(),
			this->parser_.initial_elements_.end(),
			[str]( const auto& aPair )->bool
			{
				return aPair.first == str;
			});
		if (foundIt != this->parser_.initial_elements_.end())
		{
			result = foundIt->second;
		}
		return result;
	}
	
	std::vector<std::pair<std::string, T>> & Pairs() const noexcept
	{
		return this->parser_.initial_elements_;
	}
};


const std::map< std::string, double >&	BuiltInConstants();


const symbolsJW< UnaryFunc >&	BuiltInUnarySyms();

const symbolsJW< BinaryFunc >&	BuiltInBinarySyms();

const symbolsJW< NaryFunc >&	BuiltInNarySyms();

const boost::parser::symbols< IterationKind >&	BuiltInIterationSyms();

#endif /* Built_ins_hpp */

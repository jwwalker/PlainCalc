//  DoBinOp.hpp
//  PlainCalc2
//
//  Created by James Walker on 3/27/23.
//  
//
/*
	Copyright (c) 2006-2023 James W. Walker

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

#ifndef DoBinOp_hpp
#define DoBinOp_hpp

#include "SCalcState.hpp"

#include <functional>
#include <cmath>

struct PowerFunctor
{
	double	operator()( double inLHS, double inRHS ) const
			{
				return std::pow( inLHS, inRHS );
			}
};

/*!
	@class		DoBinOp
	@abstract	Functor template for a semantic action that computes a
				binary operation.
	@discussion	The template parameter Op should be a class with a
				method of the signature:
				
				double	operator()( double inLHS, double inRHS ) const;
*/
template <class Op>
struct DoBinOp
{
			DoBinOp( SCalcState& ioState ) : mState( ioState ) {}
			DoBinOp( const DoBinOp& inOther ) : mState( inOther.mState ) {}
	
	void	operator()( const char*, const char* ) const
			{
				ThrowIfEmpty_( mState.mValStack );
				double	rhs = mState.mValStack.back();
				mState.mValStack.pop_back();
				ThrowIfEmpty_( mState.mValStack );
				double	lhs = mState.mValStack.back();
				mState.mValStack.pop_back();
				double	theVal = Op()( lhs, rhs );
				mState.mValStack.push_back( theVal );
			}
	
	SCalcState&		mState;
};


typedef	DoBinOp< std::plus<double> >		DoPlus;
typedef	DoBinOp< std::minus<double> >		DoMinus;
typedef	DoBinOp< std::multiplies<double> >	DoTimes;
typedef	DoBinOp< std::divides<double> >		DoDivide;
typedef	DoBinOp< PowerFunctor >		DoPower;


#endif /* DoBinOp_hpp */

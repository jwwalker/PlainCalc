//  MiniCalc.hpp
//  MiniCalcTests
//
//  Created by James Walker on 3/27/23.
//  
//

#ifndef MiniCalc_hpp
#define MiniCalc_hpp


/*!
	@function	MiniCalc
	
	@abstract	Do a calculation similar to what ParseCalcLine would do, but with a minimal grammar
				that is only able to add.
	
	@param		inLine		A NUL-terminated line of text.
	@param		ioState		A calculator object reference.
	@result		The calculated result, or NaN on failure.
*/
double MiniCalc( const char* inLine, struct SCalcState* ioState );

#endif /* MiniCalc_hpp */

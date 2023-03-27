//  CalcException.h
//  PlainCalc2
//
//  Created by James Walker on 3/26/23.
//  
//

#ifndef CalcException_h
#define CalcException_h

#include <exception>

class CalcException : public std::exception
{
public:
	CalcException() throw() {}
	CalcException(const CalcException& inOther) throw()
			: std::exception( static_cast<const std::exception&>( inOther ) )
			{}
	CalcException& operator=(const CalcException& inOther ) throw()
			{
				std::exception::operator=( static_cast<const std::exception&>( inOther ) );
				return *this;
			}
};


#endif /* CalcException_h */

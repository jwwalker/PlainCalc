#pragma once

/*!
	@header		autoCF.h
	
	This header defines wrappers for Core Foundation types.  The wrappers are
	similar to std::auto_ptr, but assignment and destruction are modified
	to take advantage of the CF reference counts.  That is, when you destroy
	the wrapper, it releases the CF object, and when you assign one wrapper
	to another, the CF object is retained.
*/

/*
	Copyright (c) 2006-2015 James W. Walker

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

#if __MACH__
	#include <CoreFoundation/CoreFoundation.h>
#else
	#include <CFBase.h>
#endif

template <class X>
class autoCF
{
public:
	typedef X element_type;

	explicit autoCF(X* p = 0) throw();
	autoCF( const autoCF& inOther ) throw();
	
	autoCF& operator=( const autoCF<X>& inOther ) throw();

	~autoCF() throw();

	X* get() const throw();
	X* release() throw();
	void reset(X* p = 0) throw();

private:
	X* ptr_;
};

template<class X>
inline
autoCF<X>::autoCF(X* p) throw()
	: ptr_(p)
{
}

template<class X>
inline
autoCF<X>::autoCF( const autoCF<X>& a ) throw()
	: ptr_(a.get())
{
	if (get() != NULL)
	{
		::CFRetain( get() );
	}
}

template<class X>
inline
autoCF<X>& autoCF<X>::operator=( const autoCF<X>& inOther ) throw()
{
	reset( inOther.get() );
	if (get() != NULL)
	{
		::CFRetain( get() );
	}
	return *this;
}

template<class X>
inline
autoCF<X>::~autoCF() throw()
{
	if (get() != NULL)
	{
		::CFRelease( get() );
	}
}

template <class X>
inline
X* autoCF<X>::get() const throw()
{
	return ptr_;
}

template <class X>
inline
X* autoCF<X>::release() throw()
{
	X* tmp = ptr_;
	ptr_ = NULL;
	return tmp;
}

template <class X>
inline
void autoCF<X>::reset(X* p) throw()
{
	if (ptr_ != p)
	{
		if (ptr_ != NULL)
		{
			::CFRelease( ptr_ );
		}
		ptr_ = p;
	}
}



// Convenience typedefs for specific CF types
typedef autoCF<const struct __CFString>			autoCFStringRef;
typedef autoCF<struct __CFString>				autoCFMutableStringRef;

typedef autoCF<struct __CFBundle>				autoCFBundleRef;

typedef autoCF<const struct __CFURL>			autoCFURLRef;

typedef autoCF<const struct __CFData>			autoCFDataRef;
typedef autoCF<struct __CFData>					autoCFMutableDataRef;

typedef autoCF<const struct __CFDictionary>		autoCFDictionaryRef;
typedef autoCF<struct __CFDictionary>			autoCFMutableDictionaryRef;

typedef autoCF<const struct __CFArray>			autoCFArrayRef;
typedef autoCF<struct __CFArray>				autoCFMutableArrayRef;

typedef autoCF<const struct __CFNumber>			autoCFNumberRef;
typedef autoCF<const struct __CFBoolean>		autoCFBooleanRef;


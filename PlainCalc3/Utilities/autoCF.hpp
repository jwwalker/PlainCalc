#pragma once

#import <CoreFoundation/CoreFoundation.h>
#import <cctype>

/*!
	@header		autoCF.h
	
	This header defines wrappers for Core Foundation types.  The wrappers are
	similar to std::shared_ptr, but assignment and destruction are modified
	to take advantage of the CF reference counts.  That is, when you destroy
	the wrapper, it releases the CF object, and when you assign one wrapper
	to another, the CF object is retained.
*/
/*
	Copyright (c) 2004 James W. Walker

	This software is provided 'as-is', without any express or implied warranty.
	In no event will the author be held liable for any damages arising from the
	use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

template <class X>
class autoCF
{
public:
	autoCF() noexcept;
	explicit autoCF(X* p) noexcept;
	autoCF( const autoCF& inOther ) noexcept;
	
	void swap( autoCF<X>& ioOther ) noexcept;
	
	autoCF& operator=( const autoCF<X>& inOther ) noexcept;

	autoCF( autoCF&& inOther ) noexcept;

	autoCF& operator=( autoCF<X>&& inOther ) noexcept;

	~autoCF() noexcept;

	X* get() const noexcept;
	X* release() noexcept;

	void reset(X* p = nullptr) noexcept;

private:
	X* ptr_;
};

template<class X>
inline
autoCF<X>::autoCF() noexcept
	: ptr_(nullptr)
{
}

template<class X>
inline
autoCF<X>::autoCF(X* p) noexcept
	: ptr_(p)
{
}

template<class X>
inline
autoCF<X>::autoCF( const autoCF<X>& a ) noexcept
	: ptr_(a.get())
{
	if (get() != nullptr)
	{
		::CFRetain( get() );
	}
}

#if __cplusplus >= 201103L
template<class X>
inline
autoCF<X>::autoCF( autoCF<X>&& inOther ) noexcept
	: ptr_( inOther.ptr_ )
{
	inOther.ptr_ = nullptr;
}
#endif

template<class X>
inline
void autoCF<X>::swap( autoCF<X>& ioOther ) noexcept
{
	X* temp = ptr_;
	ptr_ = ioOther.ptr_;
	ioOther.ptr_ = temp;
}

template<class X>
inline
autoCF<X>& autoCF<X>::operator=( const autoCF<X>& inOther ) noexcept
{
	autoCF<X> temp( inOther );
	swap( temp );
	return *this;
}

template<class X>
inline
autoCF<X>& autoCF<X>::operator=( autoCF<X>&& inOther ) noexcept
{
	if (ptr_ != nullptr)
	{
		::CFRelease( ptr_ );
	}
	
	ptr_ = inOther.ptr_;
	inOther.ptr_ = nullptr;
	return *this;
}

template<class X>
inline
autoCF<X>::~autoCF() noexcept
{
	if (get() != nullptr)
	{
		::CFRelease( get() );
	}
}

template <class X>
inline
X* autoCF<X>::get() const noexcept
{
	return ptr_;
}


template <class X>
inline
X* autoCF<X>::release() noexcept
{
	X* tmp = ptr_;
	ptr_ = nullptr;
	return tmp;
}

template <class X>
inline
void autoCF<X>::reset( X* p ) noexcept
{
	if (ptr_ != nullptr)
	{
		// Previously, I had some logic to avoid releasing the
		// pointer if reset( p ) was called twice with the same p.
		// However, what if it was called with something like
		// reset( CFRetain( CFSTR("") ) ), where the pointer is
		// the same but there is another retain?  That situation
		// might happen in correct code, whereas an extra call to
		// reset( p ) without an extra retain would be incorrect.
		// Furthermore, the safeguard code led to warnings from
		// the Clang static analyzer.
		::CFRelease( ptr_ );
	}
	
	ptr_ = p;
}



//MARK: Convenience typedefs for specific CF types
using autoCFDictionaryRef = autoCF<const struct __CFDictionary>;

//MARK: convenience for converting temporary autoCFDictionaryRef to NSDictionary*

#define CF_NS( autoCFDict )		CFBridgingRelease( autoCFDict.release() )

//MARK: convenience for converting NSDictionary* to autoCFDictionaryRef

#define	NS_CF( ndDict )		autoCFDictionaryRef( (CFDictionaryRef) CFBridgingRetain( ndDict ) )


#pragma once

#include <exception>


/*!
	@header		CCarbonEventAdaptor.h
	
	CCarbonEventAdaptor is a template class that installs a Carbon Event handler
	and delegates the handling of that event to a method of a given object.
	When the adaptor is destroyed, the event handler is removed.
	
	Example:
	
	class MyController
	{
	public:
					MyController( WindowRef inWindow );
		
		OSStatus	ShowHandler( EventHandlerCallRef inHandlerCallRef,
								EventRef inEvent );
	
	private:
		WindowRef							mWindow;
		CCarbonEventAdaptor<MyController>	mShowAdaptor;
	};
	
	MyController::MyController( WindowRef inWindow )
		: mWindow( inWindow ),
		mShowAdaptor( kEventClassWindow, kEventWindowShown,
			::GetWindowEventTarget( mWindow ),
			this, &MyController::ShowHandler )
	{
	}
*/
template <class T>
class CCarbonEventAdaptor
{
public:
	class InstallException : public std::exception
	{
	public:
					InstallException( OSStatus inErr ) throw()
						: mErr( inErr ) {}
		
		OSStatus	GetErr() const { return mErr; }

	private:
		OSStatus	mErr;
	};

	typedef OSStatus (T::*HandlerMethod)(
					EventHandlerCallRef inHandlerCallRef,
					EventRef inEvent );


							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethod inMethod );
							
							~CCarbonEventAdaptor();

	static pascal OSStatus 	Callback(
									EventHandlerCallRef inHandlerCallRef,
									EventRef inEvent,
									void* inUserData );

private:
	// Unimplemented methods
							CCarbonEventAdaptor( const CCarbonEventAdaptor& inOther );
	CCarbonEventAdaptor&	operator=( const CCarbonEventAdaptor& inOther );
	
	T*				mObject;
	HandlerMethod	mMethod;
	EventHandlerUPP	mHandlerUPP;
	EventHandlerRef	mHandlerRef;
};


template <class T>
inline
CCarbonEventAdaptor<T>::CCarbonEventAdaptor(
							UInt32 inEventClass,
							UInt32 inEventKind,
							EventTargetRef inTarget,
							T* inObject,
							HandlerMethod inMethod )
	: mObject( inObject ),
	mMethod( inMethod ),
	mHandlerUPP( ::NewEventHandlerUPP( Callback ) )
{
	EventTypeSpec	eventSpec = {
		inEventClass, inEventKind
	};
	
	OSStatus	err = ::InstallEventHandler( inTarget, mHandlerUPP,
		1, &eventSpec, this, &mHandlerRef );
	
	if (err != noErr)
	{
		throw InstallException( err );
	}
}

template <class T>
inline
CCarbonEventAdaptor<T>::~CCarbonEventAdaptor()
{
	::RemoveEventHandler( mHandlerRef );
	::DisposeEventHandlerUPP( mHandlerUPP );
}

template <class T>
inline
pascal OSStatus CCarbonEventAdaptor<T>::Callback(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent,
							void* inUserData )
{
	OSStatus	err = eventNotHandledErr;
	CCarbonEventAdaptor*	me = static_cast<CCarbonEventAdaptor*>( inUserData );
	
	try
	{
		T*	theObject = me->mObject;
		HandlerMethod	theMethod = me->mMethod;
		
		err = (theObject->*theMethod)( inHandlerCallRef, inEvent );
	}
	catch (...)
	{
	}
	return err;
}


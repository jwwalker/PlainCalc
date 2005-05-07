#pragma once

#include <exception>


/*!
	@header		CCarbonEventAdaptor.h
	
	CCarbonEventAdaptor is a template class that installs a Carbon Event handler
	and delegates the handling of that event to a method of a given object.
	When the adaptor is destroyed, the event handler is removed.
	
	The method can match any of several prototypes.
	
	Example:
	
	class MyController
	{
	public:
					MyController( WindowRef inWindow );
		
	private:
		OSStatus	ShowHandler( EventHandlerCallRef inHandlerCallRef,
								EventRef inEvent );
	
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
class autoHandlerUPP
{
public:
					autoHandlerUPP( EventHandlerProcPtr inProc )
						: mUPP( NewEventHandlerUPP( inProc ) ) {}
						
					~autoHandlerUPP()
						{
							DisposeEventHandlerUPP( mUPP );
						}
					
					operator EventHandlerUPP() const
						{
							return mUPP;
						}

private:
	EventHandlerUPP	mUPP;
};


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
					
	typedef OSStatus (T::*HandlerMethod1)(
					EventRef inEvent );

	typedef OSStatus (T::*HandlerMethod0)();

	typedef OSStatus (T::*HandlerMethodCmdExtended)(
					const HICommandExtended& inCmd );
					
	typedef OSStatus (T::*HandlerMethodCmd)(
					const HICommand& inCmd );


							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethod inMethod );

							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethod1 inMethod );

							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethod0 inMethod );

							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethodCmd inMethod );

							CCarbonEventAdaptor(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget,
									T* inObject,
									HandlerMethodCmdExtended inMethod );
							
							
							~CCarbonEventAdaptor();

	void					Remove();

	static pascal OSStatus 	Callback(
									EventHandlerCallRef inHandlerCallRef,
									EventRef inEvent,
									void* inUserData );

private:
	void					Install(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget );
	
	// Unimplemented methods
							CCarbonEventAdaptor( const CCarbonEventAdaptor& inOther );
	CCarbonEventAdaptor&	operator=( const CCarbonEventAdaptor& inOther );
	
	T*					mObject;
	HandlerMethod		mMethod;
	HandlerMethod1		mMethod1;
	HandlerMethod0		mMethod0;
	HandlerMethodCmd	mMethodCmd;
	autoHandlerUPP		mHandlerUPP;
	EventHandlerRef		mHandlerRef;
};

template <class T>
inline
CCarbonEventAdaptor<T>::CCarbonEventAdaptor(
							UInt32 inEventClass,
							UInt32 inEventKind,
							EventTargetRef inTarget,
							T* inObject,
							HandlerMethod1 inMethod )
	: mObject( inObject ),
	mMethod( NULL ),
	mMethod1( inMethod ),
	mMethod0( NULL ),
	mMethodCmd( NULL ),
	mHandlerUPP( Callback )
{
	Install( inEventClass, inEventKind, inTarget );
}

template <class T>
inline
CCarbonEventAdaptor<T>::CCarbonEventAdaptor(
							UInt32 inEventClass,
							UInt32 inEventKind,
							EventTargetRef inTarget,
							T* inObject,
							HandlerMethod0 inMethod )
	: mObject( inObject ),
	mMethod( NULL ),
	mMethod1( NULL ),
	mMethod0( inMethod ),
	mMethodCmd( NULL ),
	mHandlerUPP( Callback )
{
	Install( inEventClass, inEventKind, inTarget );
}

template <class T>
inline
CCarbonEventAdaptor<T>::CCarbonEventAdaptor(
							UInt32 inEventClass,
							UInt32 inEventKind,
							EventTargetRef inTarget,
							T* inObject,
							HandlerMethodCmd inMethod )
	: mObject( inObject ),
	mMethod( NULL ),
	mMethod1( NULL ),
	mMethod0( NULL ),
	mMethodCmd( inMethod ),
	mHandlerUPP( Callback )
{
	Install( inEventClass, inEventKind, inTarget );
}

template <class T>
inline
CCarbonEventAdaptor<T>::CCarbonEventAdaptor(
							UInt32 inEventClass,
							UInt32 inEventKind,
							EventTargetRef inTarget,
							T* inObject,
							HandlerMethodCmdExtended inMethod )
	: mObject( inObject ),
	mMethod( NULL ),
	mMethod1( NULL ),
	mMethod0( NULL ),
	mMethodCmd( (HandlerMethodCmd) inMethod ),
	mHandlerUPP( Callback )
{
	Install( inEventClass, inEventKind, inTarget );
}

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
	mMethod1( NULL ),
	mMethod0( NULL ),
	mMethodCmd( NULL ),
	mHandlerUPP( Callback )
{
	Install( inEventClass, inEventKind, inTarget );
}

template <class T>
inline
void	CCarbonEventAdaptor<T>::Install(
									UInt32 inEventClass,
									UInt32 inEventKind,
									EventTargetRef inTarget )
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
	Remove();
}

template <class T>
inline
void	CCarbonEventAdaptor<T>::Remove()
{
	if (mHandlerRef != NULL)
	{
		::RemoveEventHandler( mHandlerRef );
		mHandlerRef = NULL;
	}
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
		
		if (me->mMethod != NULL)
		{
			HandlerMethod	theMethod = me->mMethod;
			
			err = (theObject->*theMethod)( inHandlerCallRef, inEvent );
		}
		else if (me->mMethod1 != NULL)
		{
			HandlerMethod1	theMethod = me->mMethod1;
			
			err = (theObject->*theMethod)( inEvent );
		}
		else if (me->mMethod0 != NULL)
		{
			HandlerMethod0	theMethod = me->mMethod0;
			
			err = (theObject->*theMethod)();
		}
		else if (me->mMethodCmd != NULL)
		{
			HICommand	theCmd;
			::GetEventParameter( inEvent, kEventParamDirectObject, typeHICommand, NULL,
				sizeof(theCmd), NULL, &theCmd );
			
			HandlerMethodCmd	theMethod = me->mMethodCmd;
			
			err = (theObject->*theMethod)( theCmd );
		}
	}
	catch (...)
	{
	}
	return err;
}


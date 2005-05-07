#pragma once

#include <exception>
#include <map>

/*!
	@header		CCommandHandlerMap.h
	
	This template class allows me to dispatch kEventCommandProcess and
	kEventCommandUpdateStatus Carbon Events by command ID without using
	a big switch statement.  To do this, I create a CCommandHandlerMap
	object for each of the two Carbon Events.  The Carbon Event handler
	can call DoMethod to dispatch commands to previously-registered
	methods.
	
	Example:
	
	class MyController
	{
		...
	private:
		OSStatus		HandleOK( const HICommandExtended& inCmd );
		
		CCommandHandlerMap<MyController>							mHICommandProcessMap;
		CCarbonEventAdaptor< CCommandHandlerMap<MyController> >		mCommandAdaptor;
		CCommandHandlerMap<MyController>							mHICommandStatusMap;
		CCarbonEventAdaptor< CCommandHandlerMap<MyController> >		mStatusAdaptor;
	};
	
	
	MyController::MyController()
		: mHICommandProcessMap( this ),
		mCommandAdaptor( kEventClassCommand, kEventCommandProcess,
			::GetWindowEventTarget( mMacWindow ), &mHICommandProcessMap,
			&CCommandHandlerMap<MyController>::DoMethod ),
		mHICommandStatusMap( this ),
		mStatusAdaptor( kEventClassCommand, kEventCommandUpdateStatus,
			::GetWindowEventTarget( mMacWindow ), &mHICommandStatusMap,
			&CCommandHandlerMap<MyController>::DoMethod )
	{
		mHICommandProcessMap.RegisterCommand( kHICommandOK,
			&MyController::HandleOK );
		...
	}
*/
template <class T>
class CCommandHandlerMap
{
public:
	typedef	OSStatus	(T::*CmdMethod)( const HICommand& inCmd );
	typedef	OSStatus	(T::*CmdMethodExtended)( const HICommandExtended& inCmd );
	typedef std::map< UInt32, CmdMethodExtended >	CmdMap;
	
					CCommandHandlerMap(
							T* inObject );
					
	void			RegisterCommand( UInt32 inCmdID, CmdMethod inMethod );
	void			RegisterCommand( UInt32 inCmdID, CmdMethodExtended inMethod );
	
	void			RegisterDefault( CmdMethodExtended inMethod );
	
	OSStatus		DoMethod( const HICommandExtended& inCmd );

private:
	T*				mObject;
	CmdMap			mMap;
	CmdMethodExtended	mDefaultHandler;
};


template <class T>
inline
CCommandHandlerMap<T>::CCommandHandlerMap(
							T* inObject )
	: mObject( inObject ),
	mDefaultHandler( NULL )
{
}

template <class T>
inline
void	CCommandHandlerMap<T>::RegisterCommand( UInt32 inCmdID, CmdMethodExtended inMethod )
{
	mMap.insert( typename CmdMap::value_type( inCmdID, inMethod ) );
}

template <class T>
inline
void	CCommandHandlerMap<T>::RegisterCommand( UInt32 inCmdID, CmdMethod inMethod )
{
	RegisterCommand( inCmdID, (CmdMethodExtended)inMethod );
}

template <class T>
inline
void	CCommandHandlerMap<T>::RegisterDefault( CmdMethodExtended inMethod )
{
	mDefaultHandler = inMethod;
}

template <class T>
inline
OSStatus	CCommandHandlerMap<T>::DoMethod( const HICommandExtended& inCmd )
{
	OSStatus	err = eventNotHandledErr;
	
	typename CmdMap::iterator	found;
	found = mMap.find( inCmd.commandID );
	if (found != mMap.end())
	{
		CmdMethodExtended	theMethod = (*found).second;
		if (theMethod != NULL)
		{
			err = (mObject->*theMethod)( inCmd );
		}
	}
	else if (mDefaultHandler != NULL)
	{
		err = (mObject->*mDefaultHandler)( inCmd );
	}
	return err;
}

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
*/
template <class T>
class CCommandHandlerMap
{
public:
	typedef	OSStatus	(T::*CmdMethod)( const HICommand& inCmd );
	typedef std::map< UInt32, CmdMethod >	CmdMap;
	
					CCommandHandlerMap(
							T* inObject );
					
	void			RegisterCommand( UInt32 inCmdID, CmdMethod inMethod );
	
	OSStatus		DoMethod( const HICommand& inCmd );

private:
	T*				mObject;
	CmdMap			mMap;
};


template <class T>
inline
CCommandHandlerMap<T>::CCommandHandlerMap(
							T* inObject )
	: mObject( inObject )
{
}

template <class T>
inline
void	CCommandHandlerMap<T>::RegisterCommand( UInt32 inCmdID, CmdMethod inMethod )
{
	mMap.insert( typename CmdMap::value_type( inCmdID, inMethod ) );
}

template <class T>
inline
OSStatus	CCommandHandlerMap<T>::DoMethod( const HICommand& inCmd )
{
	OSStatus	err = eventNotHandledErr;
	
	typename CmdMap::iterator	found;
	found = mMap.find( inCmd.commandID );
	if (found != mMap.end())
	{
		CmdMethod	theMethod = (*found).second;
		err = (mObject->*theMethod)( inCmd );
	}
	return err;
}

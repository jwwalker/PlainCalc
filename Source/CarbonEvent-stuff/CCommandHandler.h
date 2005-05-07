#pragma once

#include "CCarbonEventAdaptor.h"
#include "CCommandHandlerMap.h"


/*!
	@class		CCommandHandler
	
	@abstract	Template to simplify handling HICommand Carbon Events.
	
	@discussion	This lets me handle HICommand Carbon Events (class
				kEventClassCommand, kinds kEventCommandProcess and
				kEventCommandUpdateStatus) without switch statements.

				Example:
				
				<blockquote><pre><code>
				class MyController
				{
				public:
								MyController( WindowRef inWindow );
				
				private:
					OSStatus	OKProcess( const HICommandExtended& inCmd );
					OSStatus	OKStatus( const HICommandExtended& inCmd );

					WindowRef	mWindow;
					CCommandHandler<MyController>	mCmdHandler;
				};
				
				MyController::MyController( WindowRef inWindow )
					: mWindow( inWindow ),
					mCmdHandler( this, ::GetWindowEventTarget( mWindow ) )
				{
					mCmdHandler.RegisterCommand( kHICommandOK,
						&MyController::OKProcess, &MyController::OKStatus );
				}
				</code></pre></blockquote>
*/
template <class T>
class CCommandHandler
{
public:
	typedef	OSStatus	(T::*CmdMethod)( const HICommandExtended& inCmd );
	
					CCommandHandler(
							T* inObject,
							EventTargetRef inTarget );

	void			RegisterCommand( UInt32 inCmdID,
							CmdMethod inProcessMethod,
							CmdMethod inStatusMethod );

	void			RegisterDefault(
							CmdMethod inProcessMethod,
							CmdMethod inStatusMethod );

private:
	CCommandHandlerMap<T>	mHICommandProcessMap;
	CCommandHandlerMap<T>	mHICommandStatusMap;
	CCarbonEventAdaptor< CCommandHandlerMap<T> >	mHICommandProcessAdaptor;
	CCarbonEventAdaptor< CCommandHandlerMap<T> >	mHICommandStatusAdaptor;
};

/*!
	@function	CCommandHandler
	@abstract	Constructor.
	@param		inObject	The object whose methods will handle commands.
	@param		inTarget	The target on which event handlers will be installed.
*/
template <class T>
inline
CCommandHandler<T>::CCommandHandler(
							T* inObject,
							EventTargetRef inTarget )
	: mHICommandProcessMap( inObject ),
	mHICommandStatusMap( inObject ),
	mHICommandProcessAdaptor( kEventClassCommand, kEventCommandProcess,
		inTarget, &mHICommandProcessMap,
		&CCommandHandlerMap<T>::DoMethod ),
	mHICommandStatusAdaptor( kEventClassCommand, kEventCommandUpdateStatus,
		inTarget, &mHICommandStatusMap,
		&CCommandHandlerMap<T>::DoMethod )
{
}

/*!
	@function	RegisterCommand
	@abstract	Register a command to be handled.
	@param		inCmdID				Command ID to be handled.
	@param		inProcessMethod		Method to handle kEventCommandProcess
									for this command.  May be NULL.
	@param		inStatusMethod		Method to handle kEventCommandUpdateStatus
									for this command.  May be NULL.
*/
template <class T>
inline
void	CCommandHandler<T>::RegisterCommand( UInt32 inCmdID,
						CmdMethod inProcessMethod,
						CmdMethod inStatusMethod )
{
	mHICommandProcessMap.RegisterCommand( inCmdID, inProcessMethod );

	mHICommandStatusMap.RegisterCommand( inCmdID, inStatusMethod );
}

/*!
	@function	RegisterDefault
	@abstract	Register a handler for commands not explicitly mapped.
	@param		inProcessMethod		Method to handle kEventCommandProcess
									for this command.  May be NULL.
	@param		inStatusMethod		Method to handle kEventCommandUpdateStatus
									for this command.  May be NULL.
*/
template <class T>
inline
void	CCommandHandler<T>::RegisterDefault(
							CmdMethod inProcessMethod,
							CmdMethod inStatusMethod )
{
	mHICommandProcessMap.RegisterDefault( inProcessMethod );

	mHICommandStatusMap.RegisterDefault( inStatusMethod );
}

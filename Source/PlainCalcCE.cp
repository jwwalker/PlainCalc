
#include "CCalcWindow.h"
#include "CCarbonEventAdaptor.h"
#include "CCommandHandlerMap.h"
#include "GetOSVersion.h"

#include <MacTextEditor.h>
#include <MacHelp.h>
#include <Menus.h>

#if DEBUGGING
	#include <DataViewer.h>
#endif

namespace
{
	const ResID		rMBAR_OS9_MainBar	= 128;
	const ResID		rMBAR_OSX_MainBar	= 129;
	
	const MenuCommand	kHelpCommandID	= 1001;
	
	const UInt32	kAutoDisableMenuItemFlag	= 'AutD';
}

#pragma mark class PlainCalcApp
class PlainCalcApp
{
public:
				PlainCalcApp();
				~PlainCalcApp();
			
	void		Run();

private:
	OSStatus	CmdUpdateStatus(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent );
	OSStatus	CmdProcess(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent );
	
	OSStatus	NewProcess( const HICommand& );
	OSStatus	AboutProcess( const HICommand& );
	OSStatus	HelpProcess( const HICommand& );
	OSStatus	OpenProcess( const HICommand& );
	
	CCommandHandlerMap<PlainCalcApp>	mCmdUpdateStatusMap;
	CCommandHandlerMap<PlainCalcApp>	mCmdProcessMap;

	CCarbonEventAdaptor<PlainCalcApp>	mCmdUpdateStatus;
	CCarbonEventAdaptor<PlainCalcApp>	mCmdProcess;
};

/*
	kEventClassCommand
		kEventCommandUpdateStatus
	
	kEventClassMenu
		kEventMenuEnableItems
*/

PlainCalcApp::PlainCalcApp()
	: mCmdUpdateStatusMap( this ),
	mCmdProcessMap( this ),
	mCmdUpdateStatus( kEventClassCommand, kEventCommandUpdateStatus,
		::GetApplicationEventTarget(), this, &PlainCalcApp::CmdUpdateStatus ),
	mCmdProcess( kEventClassCommand, kEventCommandProcess,
		::GetApplicationEventTarget(), this, &PlainCalcApp::CmdProcess )
{
	mCmdProcessMap.RegisterCommand( kHICommandNew, &PlainCalcApp::NewProcess );
	mCmdProcessMap.RegisterCommand( kHICommandAbout, &PlainCalcApp::AboutProcess );
	mCmdProcessMap.RegisterCommand( kHelpCommandID, &PlainCalcApp::HelpProcess );
	mCmdProcessMap.RegisterCommand( kHICommandOpen, &PlainCalcApp::OpenProcess );


	::InitCursor();
	::TXNInitTextension( NULL, 0, 0 );
	
	if (GetOSVersion() >= 0x1000)
	{
		::SetMenuBar(::GetNewMBar(rMBAR_OSX_MainBar));
	}
	else
	{
		::SetMenuBar(::GetNewMBar(rMBAR_OS9_MainBar));
	}
	
	MenuRef	helpMenu;
	MenuItemIndex	itemNum;
	if (noErr == ::HMGetHelpMenu( &helpMenu, &itemNum ))
	{
		::AppendMenuItemTextWithCFString( helpMenu, CFSTR("PlainCalc Help"),
			0, kHelpCommandID, NULL );
	}
}

PlainCalcApp::~PlainCalcApp()
{
	
	::TXNTerminateTextension();
}

void	PlainCalcApp::Run()
{
	HICommand			newCommand = { 0, kHICommandNew };
	::ProcessHICommand( &newCommand );

	::RunApplicationEventLoop();
}

OSStatus	PlainCalcApp::CmdUpdateStatus(
					EventHandlerCallRef inHandlerCallRef,
					EventRef inEvent )
{
#pragma unused( inHandlerCallRef )
	OSStatus err = eventNotHandledErr;
	HICommandExtended		command;
	::GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL,
					sizeof(HICommand), NULL, &command);
	
	err = mCmdUpdateStatusMap.DoMethod( command );
	
	if ( (err == eventNotHandledErr) and ((command.attributes & kHICommandFromMenu) != 0) )
	{
		UInt32	theRefCon;
		::GetMenuItemRefCon( command.source.menu.menuRef, command.source.menu.menuItemIndex,
			&theRefCon );
		
		if (theRefCon == kAutoDisableMenuItemFlag)
		{
			::DisableMenuCommand( NULL, command.commandID );
			err = noErr;
		}
	}

	return err;
}

OSStatus	PlainCalcApp::CmdProcess(
					EventHandlerCallRef inHandlerCallRef,
					EventRef inEvent )
{
#pragma unused( inHandlerCallRef )
	OSStatus err = eventNotHandledErr;
	HICommandExtended		command;
	::GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL,
					sizeof(HICommandExtended), NULL, &command);

	err = mCmdProcessMap.DoMethod( command );
	
	return err;
}



OSStatus	PlainCalcApp::NewProcess( const HICommand& )
{
	CCalcWindow* theWindow = new CCalcWindow;
	theWindow->Show();
	return noErr;
}

OSStatus	PlainCalcApp::AboutProcess( const HICommand& )
{
	SInt16	hit;
	AlertStdAlertParamRec	alertParam = {
		true, false,
		NULL,
		(ConstStringPtr)kAlertDefaultOKText,
		NULL,
		NULL,
		1,
		0,
		kWindowDefaultPosition
	};
	::StandardAlert( kAlertNoteAlert, "\pPlainCalc",
		"\pThis simple calculator is freeware.\rCopyright 2003 James W. Walker.\r"
		"http://www.jwwalker.com",
		&alertParam, &hit );
	return noErr;
}

OSStatus	PlainCalcApp::HelpProcess( const HICommand& )
{
	Handle	textH = ::GetResource( 'TEXT', 128 );
	if (textH != NULL)
	{
		::HLock( textH );
		new CCalcWindow( *textH, ::GetHandleSize(textH) );
		::ReleaseResource( textH );
	}
	return noErr;
}

OSStatus	PlainCalcApp::OpenProcess( const HICommand& )
{
	NavReplyRecord	theReply;
	theReply.version = kNavReplyRecordVersion;
	theReply.validRecord = false;
	
	NavDialogOptions	theOptions;
	::NavGetDefaultDialogOptions( &theOptions );
	theOptions.dialogOptionFlags |= kNavNoTypePopup;
	
	NavTypeList	theType = {
		kNavGenericSignature, 0, 1, 'PlCl'
	};
	Handle	typeHandle = NULL;
	::PtrToHand( &theType, &typeHandle, sizeof(theType) );
	
	OSStatus	err = ::NavGetFile( NULL, &theReply, &theOptions, NULL, NULL, NULL,
		(NavTypeListHandle)typeHandle, NULL );
	
	if (typeHandle != NULL)
	{
		::DisposeHandle( typeHandle );
	}
	
	if (err == noErr)
	{
		FSSpec		theSpec;
		AEKeyword	keyword;
		DescType	actualType;
		Size		actualSize;
		
		err = ::AEGetNthPtr( &theReply.selection, 1, typeFSS, &keyword,
			&actualType, &theSpec, sizeof(theSpec), &actualSize );
	
		if (err == noErr)
		{
			CCalcWindow*	theWindow = new CCalcWindow;
			
			theWindow->LoadDocument( theSpec );
			theWindow->Show();
		}
	}
	
	return noErr;
}



int main(void)
{
	#if DEBUGGING
		DataViewLibInit();
	#endif

	try
	{
		PlainCalcApp	app;
		
		app.Run();
	}
	catch (...)
	{
	}
	
	return 0;
}

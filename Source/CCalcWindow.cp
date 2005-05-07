#include "CCalcWindow.h"

#include "autoCF.h"
#include "CCarbonEventAdaptor.h"
#include "CCommandHandler.h"
#include "CCommandHandlerMap.h"
#include "CFStringToUTF8.h"
#include "GetOSVersion.h"
#include "ParseCalcLine.h"
#include "UTF8ToCFString.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

#define	ThrowIfCFFail_( x ) do { if ((x) == NULL) throw std::bad_alloc(); } while (false)

namespace
{
	const Rect	kInitialWinBounds	= {
		40, 40, 240, 240
	};
	
	const Rect	kHelpWinBounds	= {
		40, 40, 440, 440
	};
	
	
	const SInt32	kRangeSize				= 100;

	const RGBColor	kAnswerColor = { 0, 0x7777, 0 };
	const RGBColor	kErrorColor = { 0x7777, 0, 0 };

	#pragma mark class auto_WindowRef
	class auto_WindowRef
	{
	public:
					auto_WindowRef( WindowRef inWindow )
						: mWindow( inWindow ) {}
					
					~auto_WindowRef()
						{
							if (mWindow != NULL)
							{
								::DisposeWindow( mWindow );
							}
						}
	
		WindowRef	get() const { return mWindow; }
	
	private:
		WindowRef	mWindow;
	};
	
	#pragma mark class auto_MLTE
	class auto_MLTE
	{
	public:
					auto_MLTE( WindowRef inWindow );
					~auto_MLTE();
		
		TXNObject	get() const { return mMLTE; }
		TXNFrameID	getFrameID() const { return mFrameID; }
	
	private:
		TXNObject	mMLTE;
		TXNFrameID	mFrameID;
	};
}

auto_MLTE::auto_MLTE( WindowRef inWindow )
{
	OSStatus	err = ::TXNNewObject( NULL, inWindow, NULL,
		kTXNDrawGrowIconMask | kTXNWantVScrollBarMask |
			kOutputTextInUnicodeEncodingMask | kTXNAlwaysWrapAtViewEdgeMask |
			kTXNDontDrawCaretWhenInactiveMask,
		kTXNTextEditStyleFrameType, kTXNUnicodeTextFile,
		kTXNUnicodeEncoding,
		&mMLTE, &mFrameID, NULL );
	if (err != noErr)
	{
		throw std::exception();
	}
}

auto_MLTE::~auto_MLTE()
{
	::TXNDeleteObject( mMLTE );
}

static WindowRef	MakeDocWindow( const Rect& inBounds )
{
	WindowRef	theWindow = NULL;
	OSStatus err = ::CreateNewWindow( kDocumentWindowClass,
		kWindowStandardDocumentAttributes | kWindowStandardHandlerAttribute,
		&inBounds, &theWindow );
	if (err != noErr)
	{
		throw std::exception();
	}
	ControlRef	root;
	::CreateRootControl( theWindow, &root );
	
	return theWindow;
}

#pragma mark class XCalcWindowImp
class XCalcWindowImp
{
public:

					XCalcWindowImp( CCalcWindow* inSelf,
							const Rect& inWindowBounds );
					~XCalcWindowImp();

	SInt32			SetText( TXNOffset inStart, TXNOffset inEnd,
						const std::string& outUTF8Text );
	
	WindowRef		GetWindow();
	
	void			Show();
	
	void			MoveToTop();

	void			LoadDocDictionary( CFDictionaryRef inDict );
	void			SpecifyFile( const FSSpec& inFile );

private:
	OSStatus		KeyHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		ActivateHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		DeactivateHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		DrawHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		BoundsChangedHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		ClickHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );
	OSStatus		CloseHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent );

	OSStatus		CloseStatus( const HICommandExtended& inCmd );
	OSStatus		CloseProcess( const HICommandExtended& inCmd );
	
	OSStatus		EnableIfSelectionStatus( const HICommandExtended& inCmd );
	OSStatus		PasteStatus( const HICommandExtended& inCmd );
	OSStatus		SelectAllStatus( const HICommandExtended& inCmd );
	
	OSStatus		CutProcess( const HICommandExtended& );
	OSStatus		CopyProcess( const HICommandExtended& );
	OSStatus		ClearProcess( const HICommandExtended& );
	OSStatus		PasteProcess( const HICommandExtended& );
	OSStatus		SelectAllProcess( const HICommandExtended& );
	
	OSStatus		SaveStatus( const HICommandExtended& inCmd );
	OSStatus		SaveAsStatus( const HICommandExtended& inCmd );
	OSStatus		SaveProcess( const HICommandExtended& inCmd );
	OSStatus		SaveAsProcess( const HICommandExtended& inCmd );
	
	OSStatus		FormatHexStatus( const HICommandExtended& inCmd );
	OSStatus		FormatDecStatus( const HICommandExtended& inCmd );
	OSStatus		FormatHexProcess( const HICommandExtended& inCmd );
	OSStatus		FormatDecProcess( const HICommandExtended& inCmd );
	
	
	void			SaveFile( const FSSpec& inFile );
	CFDictionaryRef	CreateDocDictionary() const;

	void			SetTextMargins();
	void			TypeCarriageReturn();
	void			DoCalculate();
	SInt32			FindPrevReturn( SInt32 inOffset );
	bool			FindPrevLine( TXNOffset inStartOffset,
							TXNOffset& outLineStart,
							TXNOffset& outLineEnd );
	
	CFStringRef		CreateCFText( TXNOffset inStart, TXNOffset inEnd ) const;
	void			GetText( TXNOffset inStart, TXNOffset inEnd,
						std::string& outUTF8Text );
	void			GetChars( TXNOffset inStart, TXNOffset inEnd,
						UniChar* outBuffer );
	void			SetTextColor( TXNOffset inStart, TXNOffset inEnd,
						const RGBColor& inColor );
	SInt32			SetCFText( TXNOffset inStart, TXNOffset inEnd,
						CFStringRef inText );

	CCalcWindow*	mSelf;
	auto_WindowRef	mWindow;
	auto_MLTE		mMLTE;
	CalcState		mCalculator;
	ItemCount		mTextSaveChangeCount;
	bool			mFormatIntegersAsHex;
	
	CCommandHandler<XCalcWindowImp>		mCmdAdaptor;

	CCarbonEventAdaptor<XCalcWindowImp>	mKeyAdaptor;
	CCarbonEventAdaptor<XCalcWindowImp>	mBoundsChanged;
	CCarbonEventAdaptor<XCalcWindowImp>	mWindowClosed;
	
	// The following events happen automagically on OS X (at least 10.2),
	// but not in OS 9.
	std::auto_ptr< CCarbonEventAdaptor<XCalcWindowImp> >	mActivate;
	std::auto_ptr< CCarbonEventAdaptor<XCalcWindowImp> >	mDeactivate;
	std::auto_ptr< CCarbonEventAdaptor<XCalcWindowImp> >	mDraw;
	std::auto_ptr< CCarbonEventAdaptor<XCalcWindowImp> >	mClick;
};

XCalcWindowImp::XCalcWindowImp( CCalcWindow* inSelf, const Rect& inWindowBounds )
	: mSelf( inSelf ),
	mWindow( MakeDocWindow( inWindowBounds ) ),
	mMLTE( mWindow.get() ),
	mCalculator( CreateCalcState() ),
	mTextSaveChangeCount( 0 ),
	mFormatIntegersAsHex( false ),
	mCmdAdaptor( this, ::GetWindowEventTarget( mWindow.get() ) ),
	mKeyAdaptor( kEventClassTextInput, kEventTextInputUnicodeForKeyEvent,
		::GetWindowEventTarget( mWindow.get() ), this, &XCalcWindowImp::KeyHandler ),
	mBoundsChanged( kEventClassWindow, kEventWindowBoundsChanged,
		::GetWindowEventTarget( mWindow.get() ), this, &XCalcWindowImp::BoundsChangedHandler ),
	mWindowClosed( kEventClassWindow, kEventWindowClose,
		::GetWindowEventTarget( mWindow.get() ), this, &XCalcWindowImp::CloseHandler )
{
	::RepositionWindow( mWindow.get(), NULL, kWindowCascadeOnMainScreen );
	SetTextMargins();
	
	mCmdAdaptor.RegisterCommand( kHICommandClose, &XCalcWindowImp::CloseProcess,
		&XCalcWindowImp::CloseStatus );
	mCmdAdaptor.RegisterCommand( kHICommandSave, &XCalcWindowImp::SaveProcess,
		&XCalcWindowImp::SaveStatus );
	mCmdAdaptor.RegisterCommand( kHICommandSaveAs, &XCalcWindowImp::SaveAsProcess,
		&XCalcWindowImp::SaveAsStatus );

	mCmdAdaptor.RegisterCommand( kHICommandCut, &XCalcWindowImp::CutProcess,
		&XCalcWindowImp::EnableIfSelectionStatus );
	mCmdAdaptor.RegisterCommand( kHICommandCopy, &XCalcWindowImp::CopyProcess,
		&XCalcWindowImp::EnableIfSelectionStatus );
	mCmdAdaptor.RegisterCommand( kHICommandClear, &XCalcWindowImp::ClearProcess,
		&XCalcWindowImp::EnableIfSelectionStatus );
	mCmdAdaptor.RegisterCommand( kHICommandPaste, &XCalcWindowImp::PasteProcess,
		&XCalcWindowImp::PasteStatus );
	mCmdAdaptor.RegisterCommand( kHICommandSelectAll, &XCalcWindowImp::SelectAllProcess,
		&XCalcWindowImp::SelectAllStatus );
	
	mCmdAdaptor.RegisterCommand( kCommandID_IntegersAsHex, &XCalcWindowImp::FormatHexProcess,
		&XCalcWindowImp::FormatHexStatus );
	mCmdAdaptor.RegisterCommand( kCommandID_IntegersAsDecimal, &XCalcWindowImp::FormatDecProcess,
		&XCalcWindowImp::FormatDecStatus );

	if (GetOSVersion() < 0x1020)
	{
		mActivate.reset( new CCarbonEventAdaptor<XCalcWindowImp>( kEventClassWindow,
			kEventWindowActivated, ::GetWindowEventTarget( mWindow.get() ), this,
			&XCalcWindowImp::ActivateHandler ) );
		mDeactivate.reset( new CCarbonEventAdaptor<XCalcWindowImp>( kEventClassWindow,
			kEventWindowDeactivated, ::GetWindowEventTarget( mWindow.get() ), this,
			&XCalcWindowImp::DeactivateHandler ) );
		mDraw.reset( new CCarbonEventAdaptor<XCalcWindowImp>( kEventClassWindow,
			kEventWindowDrawContent, ::GetWindowEventTarget( mWindow.get() ), this,
			&XCalcWindowImp::DrawHandler ) );
		mClick.reset( new CCarbonEventAdaptor<XCalcWindowImp>( kEventClassWindow,
			kEventWindowClickContentRgn, ::GetWindowEventTarget( mWindow.get() ), this,
			&XCalcWindowImp::ClickHandler ) );
	}
	
	ProcessInfoRec	pInfo;
	FSSpec	appSpec;
	pInfo.processInfoLength = sizeof(pInfo);
	pInfo.processName = NULL;
	pInfo.processAppSpec = &appSpec;
	ProcessSerialNumber	psn = {
		kNoProcess, kCurrentProcess
	};
	::GetProcessInformation( &psn, &pInfo );
	::SetWindowProxyCreatorAndType( GetWindow(), 'PLCL', 'PlCl', appSpec.vRefNum );
}

XCalcWindowImp::~XCalcWindowImp()
{
	DisposeCalcState( mCalculator );
}

void	XCalcWindowImp::Show()
{
	::ShowWindow( mWindow.get() );
}

WindowRef	XCalcWindowImp::GetWindow()
{
	return mWindow.get();
}

void	XCalcWindowImp::SetTextMargins()
{
	TXNMargins	margins = {
		10, 10, 10, 10
	};
	TXNControlData	controlData;
	controlData.marginsPtr = &margins;
	TXNControlTag	theTag = kTXNMarginsTag;
	
	::TXNSetTXNObjectControls( mMLTE.get(), false, 1, &theTag, &controlData );
}

void	XCalcWindowImp::TypeCarriageReturn()
{
	TXNOffset	selStart, selEnd;
	::TXNGetSelection( mMLTE.get(), &selStart, &selEnd );
	
	SetText( selStart, selEnd, std::string("\r") );
}

void	XCalcWindowImp::DoCalculate()
{
	TXNOffset	selStart, selEnd, lineStart, lineEnd;
	::TXNGetSelection( mMLTE.get(), &selStart, &selEnd );
	if ( (selStart == selEnd) && (selStart >= 2) )
	{
		if (FindPrevLine( selStart, lineStart, lineEnd ))
		{
			std::string	theText;
			GetText( lineStart, lineEnd, theText );
			
			std::string		newText;
			double	theValue;
			long	stopOffset;
			
			if (ParseCalcLine( theText.c_str(), mCalculator, &theValue, &stopOffset ))
			{
				std::ostringstream	oss;
				if ( mFormatIntegersAsHex and
					(std::abs(theValue - std::round(theValue)) < FLT_EPSILON) )
				{
					oss << "= 0x" << std::hex << std::uppercase <<
						std::lround(theValue) << "\r";
				}
				else
				{
					oss << "= " << std::setprecision(12) << theValue << "\r";
				}
				newText += oss.str();
				
				SInt32 numUniChars = SetText( selStart, selEnd, newText );
				
				SetTextColor( selStart, selStart + numUniChars - 1, kAnswerColor );
			}
			else
			{
				newText += "Syntax error.\r";
				SetText( selStart, selEnd, newText );
				SetTextColor( selStart, selStart + newText.size() - 1, kErrorColor );
				::TXNSetSelection( mMLTE.get(),
					lineStart + stopOffset, lineEnd );
			}
		}
	}
}

void	XCalcWindowImp::SetTextColor( TXNOffset inStart, TXNOffset inEnd,
						const RGBColor& inColor )
{
	TXNTypeAttributes	atts = {
		kTXNQDFontColorAttribute,
		sizeof(RGBColor)
	};
	atts.data.dataPtr = (void*)&inColor;
	::TXNSetTypeAttributes( mMLTE.get(), 1, &atts, inStart, inEnd );
}

void	XCalcWindowImp::MoveToTop()
{
	::TXNSetSelection( mMLTE.get(), 0, 0 );
	::TXNShowSelection( mMLTE.get(), false );
}

void	XCalcWindowImp::GetText( TXNOffset inStart, TXNOffset inEnd,
						std::string& outUTF8Text )
{
	outUTF8Text.clear();
	
	autoCFStringRef	stringRef( CreateCFText( inStart, inEnd ) );
	
	if (stringRef.get() != NULL)
	{
		outUTF8Text = CFStringToUTF8( stringRef.get() );
	}
}

CFStringRef		XCalcWindowImp::CreateCFText( TXNOffset inStart, TXNOffset inEnd ) const
{
	autoCFStringRef		theText;
	
	Handle	textH = NULL;
	::TXNGetDataEncoded( mMLTE.get(), inStart, inEnd,
		&textH, kTXNUnicodeTextData );
	
	if (textH != NULL)
	{
		Size	textLength = (Size) (::GetHandleSize(textH) / sizeof(UniChar));
		::HLock(textH);
		
		theText.reset( ::CFStringCreateWithCharacters( NULL,
			(UniChar*) *textH, textLength ) );
		
		::DisposeHandle( textH );
	}
	
	return theText.release();
}

void	XCalcWindowImp::GetChars( TXNOffset inStart, TXNOffset inEnd,
						UniChar* outBuffer )
{
	Handle	textH = NULL;
	::TXNGetDataEncoded( mMLTE.get(), inStart, inEnd,
		&textH, kTXNUnicodeTextData );
	
	if (textH != NULL)
	{
		::BlockMoveData( *textH, outBuffer, (inEnd - inStart) * sizeof(UniChar) );
		
		::DisposeHandle( textH );
	}
}

SInt32	XCalcWindowImp::SetText( TXNOffset inStart, TXNOffset inEnd,
						const std::string& inUTF8Text )
{
	autoCFStringRef	cfText( UTF8ToCFString( inUTF8Text.c_str() ) );
	
	return SetCFText( inStart, inEnd, cfText.get() );
}

SInt32	XCalcWindowImp::SetCFText( TXNOffset inStart, TXNOffset inEnd,
						CFStringRef inText )
{
	SInt32	numUniChars = 0;
	
	if (inText != NULL)
	{
		numUniChars = ::CFStringGetLength( inText );
		std::vector<UniChar>	uniText( numUniChars );
		::CFStringGetCharacters( inText, CFRangeMake( 0, numUniChars ),
			&uniText.front() );
		
		::TXNSetData( mMLTE.get(), kTXNUnicodeTextData,
			&uniText.front(), numUniChars * sizeof(UniChar), inStart, inEnd );
	}
	return numUniChars;
}

bool	XCalcWindowImp::FindPrevLine( TXNOffset inStartOffset,
							TXNOffset& outLineStart,
							TXNOffset& outLineEnd )
{
	UniChar	buffer[ kRangeSize ];
	bool	foundEnd = false;
	bool	foundStart = false;
	TXNOffset	rangeEnd, rangeStart;
	SInt32		offset;
	rangeEnd = inStartOffset;
	
	while (rangeEnd > 0)
	{
		rangeStart = (rangeEnd >= kRangeSize)? rangeEnd - kRangeSize : 0;

		GetChars( rangeStart, rangeEnd, buffer );
		
		for (offset = static_cast<SInt32>(rangeEnd) - 1;
			offset >= static_cast<SInt32>(rangeStart); --offset)
		{
			if (buffer[ offset - rangeStart ] == 0x0D)
			{
				if (foundEnd)
				{
					outLineStart = offset + 1;
					foundStart = true;
					break;
				}
				else
				{
					outLineEnd = offset;
					foundEnd = true;
				}
			}
		}
		
		if (foundStart)
		{
			break;
		}
		
		rangeEnd = rangeStart;
	}
	
	if (foundEnd && (not foundStart))
	{
		outLineStart = 0;
	}
	
	return foundEnd && (outLineEnd > outLineStart);
}

/*!
	@function		FindPrevReturn
	
	@abstract		Starting at a given offset, find the last carriage return
					character preceding that offset.
	
	@param			inOffset	Starting position for search.
	@result			Offset of the carriage return, or -1 if none was found.
*/
SInt32	XCalcWindowImp::FindPrevReturn( SInt32 inOffset )
{
	SInt32	foundOff = -1;
	
	SInt32	rangeEnd = inOffset;
	
	UniChar	theReturn = 0x0D;
	TXNMatchTextRecord	toMatch = {
		&theReturn, sizeof(theReturn), kTextEncodingUnicodeDefault
	};
	
	while ( (rangeEnd > 0) && (foundOff < 0) )
	{
		SInt32	rangeStart = rangeEnd - kRangeSize;
		if (rangeStart < 0)
		{
			rangeStart = 0;
		}
		
		while (rangeStart < rangeEnd)
		{
			TXNOffset	foundStart, foundEnd;
			
			::TXNFind( mMLTE.get(), &toMatch, kTXNTextData, 0,
				rangeStart, rangeEnd, NULL, NULL, &foundStart, &foundEnd );
			
			if (foundStart == kTXNUseCurrentSelection)
			{
				break;
			}
			else
			{
				foundOff = foundStart;
				rangeStart = foundEnd;
			}
		}
		
		rangeEnd -= kRangeSize;
	}
	
	return foundOff;
}


OSStatus	XCalcWindowImp::KeyHandler(
							EventHandlerCallRef inHandlerCallRef,
							EventRef inEvent )
{
	OSStatus err = eventNotHandledErr;
	UniChar	theChar;
	UInt32	actualSize = 0;
	::GetEventParameter( inEvent, kEventParamTextInputSendText, typeUnicodeText,
		NULL, sizeof(theChar), &actualSize, &theChar );
	if (actualSize <= sizeof(theChar))
	{
		if ( (theChar == 0x03) || (theChar == 0x0D) )
		{
			err = noErr;
			TypeCarriageReturn();
			
			DoCalculate();
		}
	}
	
	if (err == eventNotHandledErr)
	{
		err = ::CallNextEventHandler( inHandlerCallRef, inEvent );
	}
	
	if (::TXNGetChangeCount( mMLTE.get() ) > mTextSaveChangeCount)
	{
		::SetWindowModified( GetWindow(), true );
	}
	
	return err;
}

OSStatus	XCalcWindowImp::ActivateHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef, inEvent )
	::TXNActivate( mMLTE.get(), mMLTE.getFrameID(), true);
	::TXNFocus( mMLTE.get(), true);
	return noErr;
}

OSStatus	XCalcWindowImp::DeactivateHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef, inEvent )
	::TXNFocus( mMLTE.get(), false );
	return noErr;
}
OSStatus	XCalcWindowImp::DrawHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef, inEvent )
	::TXNDraw(mMLTE.get(), NULL);
	return noErr;
}

OSStatus	XCalcWindowImp::ClickHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef )
	EventRef		mouseEvent;
	EventRecord		er;
	
	OSStatus err = ::GetEventParameter(inEvent, 'mous', typeEventRef, NULL,
			sizeof(EventRef), NULL, &mouseEvent);
	
	if (err == noErr)
	{
		::ConvertEventRefToEventRecord(mouseEvent, &er);
		::TXNClick( mMLTE.get(), &er );
	}
	return noErr;
}

OSStatus	XCalcWindowImp::BoundsChangedHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef, inEvent )
	Rect		bounds;
	
	::GetWindowBounds( mWindow.get(), kWindowContentRgn, &bounds );
	::TXNResizeFrame( mMLTE.get(), bounds.right - bounds.left,
		bounds.bottom - bounds.top, mMLTE.getFrameID() );
	return noErr;
}

OSStatus	XCalcWindowImp::CloseHandler(
						EventHandlerCallRef inHandlerCallRef,
						EventRef inEvent )
{
#pragma unused( inHandlerCallRef, inEvent )
	delete mSelf;
	return noErr;
}

OSStatus	XCalcWindowImp::CloseStatus( const HICommandExtended& inCmd )
{
	::EnableMenuCommand( NULL, inCmd.commandID );
	return noErr;
}

OSStatus	XCalcWindowImp::CloseProcess( const HICommandExtended& /* inCmd */ )
{
	delete mSelf;
	return noErr;
}

OSStatus	XCalcWindowImp::EnableIfSelectionStatus( const HICommandExtended& inCmd )
{
	if (::TXNIsSelectionEmpty(mMLTE.get()))
		::DisableMenuCommand(NULL, inCmd.commandID);
	else
		::EnableMenuCommand(NULL, inCmd.commandID);
	return noErr;
}

OSStatus	XCalcWindowImp::PasteStatus( const HICommandExtended& inCmd )
{
	if (::TXNIsScrapPastable())
		::EnableMenuCommand(NULL, inCmd.commandID);
	else
		::DisableMenuCommand(NULL, inCmd.commandID);
	
	return noErr;
}

OSStatus	XCalcWindowImp::SelectAllStatus( const HICommandExtended& inCmd )
{
	if (::TXNDataSize(mMLTE.get()) > 0)
		::EnableMenuCommand(NULL, inCmd.commandID);
	else
		::DisableMenuCommand(NULL, inCmd.commandID);
	
	return noErr;
}

OSStatus	XCalcWindowImp::CutProcess( const HICommandExtended& )
{
	::TXNCut( mMLTE.get() );
	return noErr;
}

OSStatus	XCalcWindowImp::CopyProcess( const HICommandExtended& )
{
	::TXNCopy( mMLTE.get() );
	return noErr;
}

OSStatus	XCalcWindowImp::ClearProcess( const HICommandExtended& )
{
	::TXNClear( mMLTE.get() );
	return noErr;
}

OSStatus	XCalcWindowImp::PasteProcess( const HICommandExtended& )
{
	::TXNPaste( mMLTE.get() );
	return noErr;
}

OSStatus	XCalcWindowImp::SelectAllProcess( const HICommandExtended& )
{
	::TXNSelectAll( mMLTE.get() );
	return noErr;
}

OSStatus	XCalcWindowImp::SaveStatus( const HICommandExtended& inCmd )
{
	if (::TXNGetChangeCount( mMLTE.get() ) > mTextSaveChangeCount)
	{
		::EnableMenuCommand( NULL, inCmd.commandID );
	}
	else
	{
		::DisableMenuCommand( NULL, inCmd.commandID );
	}
	return noErr;
}

OSStatus	XCalcWindowImp::SaveAsStatus( const HICommandExtended& inCmd )
{
	::EnableMenuCommand( NULL, inCmd.commandID );
	return noErr;
}

OSStatus	XCalcWindowImp::SaveProcess( const HICommandExtended& )
{
	FSSpec	fileSpec;
	OSStatus	err;
	
	err = ::GetWindowProxyFSSpec( GetWindow(), &fileSpec );
	
	if (err == noErr)
	{
		SaveFile( fileSpec );
	}
	else
	{
		HICommandExtended	dummyCmd;
		SaveAsProcess( dummyCmd );
		err = noErr;
	}
	
	return err;
}

OSStatus	XCalcWindowImp::SaveAsProcess( const HICommandExtended& )
{
	NavReplyRecord	theReply;
	theReply.version = kNavReplyRecordVersion;
	theReply.validRecord = false;
	
	OSStatus	err = ::NavPutFile( NULL, &theReply, NULL, NULL, 'PlCl', 'PLCL', NULL );
	if (err == noErr)
	{
		if (theReply.validRecord)
		{
			FSSpec		theSpec;
			AEKeyword	keyword;
			DescType	actualType;
			Size		actualSize;
			
			err = ::AEGetNthPtr( &theReply.selection, 1, typeFSS, &keyword,
				&actualType, &theSpec, sizeof(theSpec), &actualSize );
			
			if (err == noErr)
			{
				if (theReply.replacing)
				{
					::FSpDelete( &theSpec );
				}
				
				SaveFile( theSpec );
				SpecifyFile( theSpec );
			}
		}
		
		::NavDisposeReply( &theReply );
	}
	
	return err;
}


OSStatus	XCalcWindowImp::FormatHexStatus( const HICommandExtended& inCmd )
{
	::EnableMenuCommand( NULL, inCmd.commandID );
	::CheckMenuItem( inCmd.source.menu.menuRef, inCmd.source.menu.menuItemIndex, mFormatIntegersAsHex );
	return noErr;
}

OSStatus	XCalcWindowImp::FormatDecStatus( const HICommandExtended& inCmd )
{
	::EnableMenuCommand( NULL, inCmd.commandID );
	::CheckMenuItem( inCmd.source.menu.menuRef, inCmd.source.menu.menuItemIndex, not mFormatIntegersAsHex );
	return noErr;
}

OSStatus	XCalcWindowImp::FormatHexProcess( const HICommandExtended& inCmd )
{
#pragma unused( inCmd )
	mFormatIntegersAsHex = true;
	return noErr;
}

OSStatus	XCalcWindowImp::FormatDecProcess( const HICommandExtended& inCmd )
{
#pragma unused( inCmd )
	mFormatIntegersAsHex = false;
	return noErr;
}


void	XCalcWindowImp::SpecifyFile( const FSSpec& inFile )
{
	AliasHandle	theAlias = NULL;
	OSStatus	err;
	err = ::NewAlias( NULL, &inFile, &theAlias );
	if (err == noErr)
	{
		::SetWindowProxyAlias( GetWindow(), theAlias );
		::DisposeHandle( (Handle)theAlias );
		
		::SetWTitle( GetWindow(), inFile.name );
	}
}

void	XCalcWindowImp::SaveFile( const FSSpec& inFile )
{
	autoCFDictionaryRef	docDict( CreateDocDictionary() );
	if (docDict.get() != NULL)
	{
		autoCFDataRef	docData( ::CFPropertyListCreateXMLData( NULL, docDict.get() ) );
		
		if (docData.get() != NULL)
		{
			CFIndex		dataLen = ::CFDataGetLength( docData.get() );
			const UInt8 *	dataPtr = ::CFDataGetBytePtr( docData.get() );
			
			OSStatus	err = ::FSpCreate( &inFile, 'PLCL', 'PlCl', smRoman );
			if (err == noErr)
			{
				short	refNum = -1;
				err = ::FSpOpenDF( &inFile, fsRdWrPerm, &refNum );
				if (err == noErr)
				{
					::SetEOF( refNum, 0 );
					err = ::FSWrite( refNum, &dataLen, dataPtr );
					
					::FSClose( refNum );
					
					::SetWindowModified( GetWindow(), false );
					mTextSaveChangeCount = ::TXNGetChangeCount( mMLTE.get() );
				}
			}
		}
	}
}

CFDictionaryRef	XCalcWindowImp::CreateDocDictionary() const
{
	autoCFDictionaryRef	theVariables( CopyCalcVariables( mCalculator ) );
	ThrowIfCFFail_( theVariables.get() );
	autoCFStringRef		theText( CreateCFText( kTXNStartOffset, kTXNEndOffset ) );
	ThrowIfCFFail_( theText.get() );
	
	CFStringRef		theKeys[] = {
		CFSTR("Text"),
		CFSTR("Variables")
	};
	CFTypeRef		theValues[] = {
		theText.get(),
		theVariables.get()
	};
	autoCFDictionaryRef	docDict( ::CFDictionaryCreate( NULL,
		(CFTypeRef*)theKeys, theValues,
		sizeof(theValues)/sizeof(theValues[0]),
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	
	return docDict.release();
}

void	XCalcWindowImp::LoadDocDictionary( CFDictionaryRef inDict )
{
	CFStringRef	textCF = (CFStringRef)::CFDictionaryGetValue( inDict, CFSTR("Text") );
	CFDictionaryRef	variableDict = (CFDictionaryRef)::CFDictionaryGetValue( inDict,
		CFSTR("Variables") );
	
	if ( (textCF != NULL) && (::CFGetTypeID( textCF ) == ::CFStringGetTypeID()) )
	{
		SetCFText( 0, kTXNEndOffset, textCF );
		::SetWindowModified( GetWindow(), false );
		mTextSaveChangeCount = ::TXNGetChangeCount( mMLTE.get() );
	}
	
	if ( (variableDict != NULL) &&
		(::CFGetTypeID( variableDict ) == ::CFDictionaryGetTypeID()) )
	{
		SetCalcVariables( variableDict, mCalculator );
	}
}


#pragma mark -
CCalcWindow::CCalcWindow()
	: mImp( new XCalcWindowImp( this, kInitialWinBounds ) )
{
	::SetWTitle( mImp->GetWindow(), "\pPlainCalc" );
}

CCalcWindow::CCalcWindow( Ptr inText, SInt32 inLength )
	: mImp( new XCalcWindowImp( this, kHelpWinBounds ) )
{
	::SetWTitle( mImp->GetWindow(), "\pPlainCalc Help" );
	std::string	theText( inText, inLength );
	mImp->SetText( kTXNEndOffset, kTXNEndOffset, theText );
	mImp->MoveToTop();
	mImp->Show();
}


CCalcWindow::~CCalcWindow()
{
}

void	CCalcWindow::Show()
{
	mImp->Show();
}

void	CCalcWindow::LoadDocument( const FSSpec& inFile )
{
	OSStatus	err;
	short		refNum;
	long		dataLen;
	autoCFMutableDataRef	dataRef;
	
	err = ::FSpOpenDF( &inFile, fsRdPerm, &refNum );
	if (err == noErr)
	{
		::GetEOF( refNum, &dataLen );
		dataRef.reset( ::CFDataCreateMutable( NULL, dataLen ) );
		if (dataRef.get() != NULL)
		{
			::CFDataSetLength( dataRef.get(), dataLen );
			UInt8*	dataPtr = ::CFDataGetMutableBytePtr( dataRef.get() );
			if (::CFDataGetLength( dataRef.get() ) == dataLen)
			{
				::FSRead( refNum, &dataLen, dataPtr );
			}
		}
		
		::FSClose( refNum );
	}
	
	if (dataRef.get() != NULL)
	{
		CFPropertyListRef	thePList = ::CFPropertyListCreateFromXMLData( NULL,
			dataRef.get(), kCFPropertyListImmutable, NULL );
		if ( (thePList != NULL) &&
			(::CFGetTypeID( thePList ) == ::CFDictionaryGetTypeID()) )
		{
			autoCFDictionaryRef	theDict( static_cast<CFDictionaryRef>( thePList ) );
			
			mImp->LoadDocDictionary( theDict.get() );
			
			mImp->SpecifyFile( inFile );
		}
	}
}

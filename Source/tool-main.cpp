/*
 *  tool-main.cpp
 *  PlainCalc2
 *
 *  Created by James Walker on 1/3/10.
 *  Copyright 2010 James W. Walker. All rights reserved.
 *
 */

#include "tool-main.h"

#import "autoCF.h"
#import "ParseCalcLine.h"
#import "PreventCrashes.h"
#import "UTF8ToCFString.h"

#import <cstdio>
#import <string>

#import <CoreFoundation/CoreFoundation.h>


static void ProcessLine( const char* inLine, CalcState ioCalc )
{
	double calculatedValue = 0.0;
	long parseStop = 0;
	const char* definedSymbol = "";
	
	ECalcResult res = ParseCalcLine( inLine, ioCalc, &calculatedValue,
		&parseStop, &definedSymbol );
	
	CFTypeRef theKeys[] =
	{
		CFSTR("ResultKind"),
		CFSTR("Result"),
		CFSTR("Stop"),
		CFSTR("Symbol")
	};
	
	int resKind = res;
	autoCFNumberRef resKindRef( CFNumberCreate( NULL, kCFNumberIntType,
		&resKind ) );
	autoCFNumberRef resultRef( CFNumberCreate( NULL, kCFNumberDoubleType,
		&calculatedValue ) );
	autoCFNumberRef stopRef( CFNumberCreate( NULL, kCFNumberLongType,
		&parseStop ) );
	autoCFStringRef symRef( UTF8ToCFString( definedSymbol ) );
	
	CFTypeRef theValues[] =
	{
		resKindRef.get(),
		resultRef.get(),
		stopRef.get(),
		symRef.get()
	};
	
	autoCFDictionaryRef theDict( CFDictionaryCreate( NULL, theKeys, theValues,
		sizeof(theValues)/sizeof(theValues[0]),
		&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks ) );
	
	autoCFDataRef theData( CFPropertyListCreateXMLData( NULL, theDict.get() ) );
	int dataLen = CFDataGetLength( theData.get() );
	const char* dataBytes = reinterpret_cast<const char*>(
		CFDataGetBytePtr( theData.get() ) );
	
	fwrite( dataBytes, 1, dataLen, stdout );
	fflush( stdout );
}

static CFDictionaryRef CreateDictFromXML( const char* inDictXML )
{
	autoCFDataRef xmlData( CFDataCreate( NULL,
		reinterpret_cast<const UInt8*>(inDictXML), strlen(inDictXML) ) );
	
	CFDictionaryRef theDict( static_cast<CFDictionaryRef>(
		CFPropertyListCreateFromXMLData( NULL, xmlData.get(),
			kCFPropertyListImmutable, NULL ) ) );
	
	return theDict;
}

static void InitVariables( const char* inVarDictXML, CalcState ioCalc )
{
	autoCFDictionaryRef varDict( CreateDictFromXML( inVarDictXML ) );
	
	if (varDict.get() != NULL)
	{
		SetCalcVariables( varDict.get(), ioCalc );
	}
}


static void InitFunctions( const char* inFunDictXML, CalcState ioCalc )
{
	autoCFDictionaryRef funDict( CreateDictFromXML( inFunDictXML ) );
	
	if (funDict.get() != NULL)
	{
		SetCalcFunctions( funDict.get(), ioCalc );
	}
}

int main (int argc, char * const argv[])
{
	int	returnCode = 0;
	
	PreventCrashes();
	
	try
	{
		CalcState calculator = CreateCalcState();
		
		if (argc >= 2)
		{
			InitVariables( argv[1], calculator );
		}
		if (argc >= 3)
		{
			InitFunctions( argv[2], calculator );
		}
		
		std::string inBuf;
		char inputChar;

		while ( (inputChar = getchar()) != EOF )
		{
			if ( (inputChar == '\0') or (inputChar == '\n') )
			{
				ProcessLine( inBuf.c_str(), calculator );
				inBuf.clear();
			}
			else
			{
				inBuf += inputChar;
			}
		}
		
		DisposeCalcState( calculator );
	}
	catch (int& exNum)
	{
		returnCode = exNum;
	}
	catch (...)
	{
		returnCode = -666;
	}
	
    return 0;
}

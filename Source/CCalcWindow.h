#pragma once


#include <memory>
class XCalcWindowImp;

const MenuCommand	kCommandID_IntegersAsHex		= 'IHex';
const MenuCommand	kCommandID_IntegersAsDecimal	= 'IDec';


class CCalcWindow
{
public:
					CCalcWindow();
					CCalcWindow( Ptr inText, SInt32 inLength );
					
					~CCalcWindow();
	
	void			Show();
					
	void			LoadDocument( const FSSpec& inFile );
	
private:
	std::auto_ptr<XCalcWindowImp>	mImp;
};

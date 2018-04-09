#ifndef ZCD_CS_H
#define ZCD_CS_H

class CZCD_CS
{
private:
	LPCRITICAL_SECTION _pCS;
public:
	CZCD_CS( LPCRITICAL_SECTION pCS )
	{
		_pCS = pCS;
		EnterCriticalSection( _pCS );
	}
	~CZCD_CS()
	{
		LeaveCriticalSection( _pCS );
	}
};

#endif

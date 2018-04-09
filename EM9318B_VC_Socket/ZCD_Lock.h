#ifndef _ZCD_LOCK_
#define _ZCD_LOCK_



class CZCD_Lock
{
public:
	CZCD_Lock()
	{
		InitializeCriticalSection( &_lock );
	}
	~CZCD_Lock()
	{
		DeleteCriticalSection( &_lock );
	}
public:
	void Lock()
	{
		EnterCriticalSection( &_lock );
	}
	void Unlock()
	{
		LeaveCriticalSection( &_lock );
	}
private:
	CRITICAL_SECTION _lock;
};

#endif
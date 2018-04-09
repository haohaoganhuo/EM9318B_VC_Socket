#ifndef TIMEINERVAL_H
#define TIMEINERVAL_H

class CTimeInterval
{
private:
	LARGE_INTEGER liStart;
public:
	CTimeInterval()
	{
		QueryPerformanceCounter( &liStart );
	}
	double GetIntervalMS()
	{
		LARGE_INTEGER liStop;
		QueryPerformanceCounter( &liStop );
		LARGE_INTEGER freq;
		QueryPerformanceFrequency( &freq );
		return (liStop.QuadPart - liStart.QuadPart) / (freq.QuadPart / 1000.0);
	}
	double GetIntervalUS()
	{
		return GetIntervalMS() * 1000.0;
	}

	double GetIntervalS()
	{
		return GetIntervalMS() / 1000.0;
	}

	static void DelayUS( ULONG us )
	{
		CTimeInterval tm;
		while( tm.GetIntervalUS() < us );
	}
	void Reset()
	{
		QueryPerformanceCounter( &liStart );
	}
};

#endif

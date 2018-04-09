#pragma once

const I32 DATA_SIZE = 2;

#include "EM9118Struct.h"

class CHistoricalDataReader
{
public:
	CHistoricalDataReader( const string& filePathName );
	~CHistoricalDataReader(void)
	{
		if( _dataF )
			fclose(_dataF);
	}

	I32 ReadHeader();
	F64 GetAllBC();//得到所有相关数据文件一共有字节
	I32 GetGroupBC();

	static I32 GetFileNo( string& fileName, string& str, I32& errNo );
	static string CombFileName( string& str, I32 fileNo );
	static string CsvCombFileName( string& str, I32 fileNo );
	static bool NextFileName( string& fileName, I32& errNo );
	static bool CsvNextFileName( string& fileName, I32& errNo );

	I32 GetErrorNo(){ return _errNo; };
	bool SetFilePos( F64 pos, U32& canReadBC );

public://下面的函数都是要开放给用户的
	U32 ReadCode( I8* codeBuf, U32 bufBC, F64 pos = -1 );//-1的时候表示从当前位置读
	U32 ReadCodeByCount( I8* codeBuf, U32 groupCodeCount, F64 codeBeginInx );

	U32 GetAdChValue( I32 chInx, I8* codeBuf, U32 groupCodeCount, F64* adValue );

	U32 CodeBufferNeedCount( U32 chCodeCount )
	{
		return chCodeCount * GetGroupBC();
	}
	F64 GetFreq()
	{
		return _property.groupFreq;
	}
	F64 GetGroupTotal()//得到每通道有多少数据
	{
		return _allDataBC / _chCnt / 2;
	};
	F64 GetTimeLengthS()//得到时间长度，以秒为单位
	{
		return GetGroupTotal() / _property.groupFreq;
	}

	void GetBeginEndCh( U32* beginCh, U32* endCh )
	{
		if( beginCh )
			*beginCh = _beginCh;
		if( endCh )
			*endCh = _chCnt;
	}

	void GetAdIsInFifo( I8* isInFifo )
	{
		if( isInFifo )
			for( int i = 0; i < EM9118_MAXADCHCNT; ++i )
				isInFifo[i] = _property.adPara.isInFifo[i];
	}

	void GetCtIsInFifo( I8* isInFifo )
	{
		if( isInFifo )
			for( int i = 0; i < EM9118_MAXCTCHCNT; ++i )
				isInFifo[i] = _property.ctInFifo[i];
	}

	void GetEcIsInFifo( I8* isInFifo )
	{
		if( isInFifo )
			for( int i = 0; i < EM9118_MAXECCHCNT; ++i )
				isInFifo[i] = _property.ecInFifo[i];
	}

	I32 GetClkSource()
	{
		return _property.clkSrc;
	}

	I32 GetEtrSource()
	{
		return _property.triSrc;
	}

public:
	string _filePathName;//当前正在读取的文件
	string _firstPathName;//第一个文件的名字
	string _lastPathName;//最后一个文件的名字
	FILE* _dataF;
	V_U32 _fileDataBC;
	F64 _allDataBC;
	U32 _beginCh;
	U32 _chCnt;
	U32 _nowFilePos;//当前文件里读取的位置，是文件的实际读取位置,是从0开始计数的
	F64 _nowAllPos;//是当前总数据读取的位置，需要考虑_dataBeginPos
	EM9118_DevPara _property;
	U32 _dataBeginPos;//数据起始位置
	I32 _errNo;
};

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
	F64 GetAllBC();//�õ�������������ļ�һ�����ֽ�
	I32 GetGroupBC();

	static I32 GetFileNo( string& fileName, string& str, I32& errNo );
	static string CombFileName( string& str, I32 fileNo );
	static string CsvCombFileName( string& str, I32 fileNo );
	static bool NextFileName( string& fileName, I32& errNo );
	static bool CsvNextFileName( string& fileName, I32& errNo );

	I32 GetErrorNo(){ return _errNo; };
	bool SetFilePos( F64 pos, U32& canReadBC );

public://����ĺ�������Ҫ���Ÿ��û���
	U32 ReadCode( I8* codeBuf, U32 bufBC, F64 pos = -1 );//-1��ʱ���ʾ�ӵ�ǰλ�ö�
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
	F64 GetGroupTotal()//�õ�ÿͨ���ж�������
	{
		return _allDataBC / _chCnt / 2;
	};
	F64 GetTimeLengthS()//�õ�ʱ�䳤�ȣ�����Ϊ��λ
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
	string _filePathName;//��ǰ���ڶ�ȡ���ļ�
	string _firstPathName;//��һ���ļ�������
	string _lastPathName;//���һ���ļ�������
	FILE* _dataF;
	V_U32 _fileDataBC;
	F64 _allDataBC;
	U32 _beginCh;
	U32 _chCnt;
	U32 _nowFilePos;//��ǰ�ļ����ȡ��λ�ã����ļ���ʵ�ʶ�ȡλ��,�Ǵ�0��ʼ������
	F64 _nowAllPos;//�ǵ�ǰ�����ݶ�ȡ��λ�ã���Ҫ����_dataBeginPos
	EM9118_DevPara _property;
	U32 _dataBeginPos;//������ʼλ��
	I32 _errNo;
};

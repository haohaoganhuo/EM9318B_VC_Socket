#include "StdAfx.h"
#include <assert.h>
#include <string>
#include "EM9118_Errors.h"
#include "EM9118.h"
#include "HistoricalDataReader.h"

CHistoricalDataReader::CHistoricalDataReader( const string& filePathName )
{
	DWORD dwAttr = GetFileAttributes( filePathName.c_str() );  //得到文件属性

	if (dwAttr == INVALID_FILE_ATTRIBUTES)    // 文件或目录不存在
	{
		_errNo = EM9118_FILEPATH_ERROR;
		return;
	}

	errno_t ret = fopen_s( &_dataF, filePathName.c_str(), "rb" );

	if( ret )
	{
		_errNo = EM9118_FILEPATH_ERROR;
		_dataF = 0;
		return;
	}

	if( ReadHeader() )
	{
		_errNo = EM9118_FILEREAD_ERROR;
		return;
	}
	_filePathName = filePathName;
	_firstPathName = filePathName;

	_errNo = 0;
	_allDataBC = GetAllBC();
}

I32 CHistoricalDataReader::ReadHeader()
{
	//先写设备名称
	char devName[5];
	string name9118( "9118" );
	fread( devName, 1, name9118.length(), _dataF );
	devName[4] = 0;
	if( name9118 != devName )
		return -1;
	//版本号
	U8 ver = 0;
	fread( &ver, 1, sizeof(ver), _dataF );
	if( ver != 0 )
		return -1;
	_chCnt = 0;
	//AD使能通道信息
	_beginCh = 0;
	_property.commonPara.adChCount = 0;
	for( U32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		fread( &_property.adPara.isInFifo[i], 1, 1, _dataF );
		if( _property.adPara.isInFifo[i] )
		{
			if( !_beginCh )
				_beginCh = i + 1;
			++_chCnt;
			++_property.commonPara.adChCount;
		}
		fread( &_property.adPara.fullCode[i], 1, sizeof(I16), _dataF );
		fread( &_property.adPara.zeroCode[i], 1, sizeof(I16), _dataF );
		fread( &_property.adPara.fullValue[i], 1, sizeof(F64), _dataF );
		fread( &_property.adPara.zeroValue[i], 1, sizeof(F64), _dataF );
	}
	fread( &_property.groupFreq, 1, sizeof(F64), _dataF );
	fread( &_property.clkSrc, 1, sizeof(I32), _dataF );
	fread( &_property.triSrc, 1, sizeof(I32), _dataF );

	_property.commonPara.ctChCount = 0;
	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		fread( &_property.ctInFifo[i], 1, 1, _dataF );
		if( _property.ctInFifo[i] )
			++_property.commonPara.ctChCount;
			
	}
	_property.commonPara.ecChCount = 0;
	for( I32 i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		fread( &_property.ecInFifo[i], 1, 1, _dataF );
		if( _property.ecInFifo[i] )
			++_property.commonPara.ecChCount;

	}
	fread( &_property.diInFifo, 1, 1, _dataF );
	_property.commonPara.ioInFifo = _property.diInFifo;

	_dataBeginPos = ftell( _dataF );
	_nowFilePos = _dataBeginPos;
	_nowAllPos = 0;

	return 0;
}

I32 CHistoricalDataReader::GetFileNo( string& fileName, string& str, I32& errNo )
{
	U32 st = 0;
	do 
	{
		st = fileName.find( "_", st + 1 );
		if( st == string::npos )
		{
			errNo = EM9118_FILEPATH_ERROR;
			return -1;
		}
	} while (st < fileName.length() - 10);
		
	str = fileName.substr( 0, st );
	string strNumber = fileName.substr( st + 1 );

	return atol( strNumber.c_str() );
}

string CHistoricalDataReader::CombFileName( string& str, I32 fileNo )
{
	char tailStr[256];
	sprintf_s( tailStr, sizeof(tailStr), "_%d.dat", fileNo );
	return str + tailStr;
}

string CHistoricalDataReader::CsvCombFileName( string& str, I32 fileNo )
{
	char tailStr[256];
	sprintf_s( tailStr, sizeof(tailStr), "_%d.csv", fileNo );
	return str + tailStr;
}

bool CHistoricalDataReader::NextFileName( string& fileName, I32& errNo )
{
	string str;
	I32 no = GetFileNo( fileName, str, errNo );
	if( no < 0 )
	{
		return false;
	}
	fileName = CombFileName( str, no + 1 );
	return true;
}

bool CHistoricalDataReader::CsvNextFileName( string& fileName, I32& errNo )
{
	string str;
	I32 no = GetFileNo( fileName, str, errNo );
	if( no < 0 )
	{
		return false;
	}
	fileName = CsvCombFileName( str, no + 1 );
	return true;
}


F64 CHistoricalDataReader::GetAllBC()
{
	if( _dataF )
	{
		int ret = fclose( _dataF );
		if( ret )
		{
			_errNo = EM9118_FILECLOSE_ERROR;
		}
		_dataF = 0;
	}
	errno_t err = 0;
	FILE* f;
	string nextFile = _filePathName;
	F64 allBC = 0;
	while ( !(err = fopen_s( &f, nextFile.c_str(), "rb" )) ) 
	{
		fseek(f, 0, SEEK_END);
		U32 dataSize = ftell(f) - _dataBeginPos;
		_fileDataBC.push_back( dataSize );
		allBC += dataSize;
		fclose( f );
		_lastPathName = nextFile;
		NextFileName( nextFile, _errNo );
	}

	err = fopen_s( &_dataF, _filePathName.c_str(), "rb" );
	if( err )
	{
		_errNo = EM9118_FILEPATH_ERROR;
		_dataF = 0;
		return 0;
	}

	fseek( _dataF, _dataBeginPos, SEEK_SET );
/*	fseek(_dataF, 0, SEEK_END);
	U32 dataSize = ftell(_dataF) - _dataBeginPos;
	_fileDataBC.push_back( dataSize );
	F64 allBC = dataSize;*/

	return allBC;
}

bool CHistoricalDataReader::SetFilePos( F64 pos, U32& canReadBC )
{
	F64 tempPos = 0;
	U32 nowPos = 0;
	U32 i = 0;
	for( i = 0; i < _fileDataBC.size(); ++i )
	{
		tempPos += _fileDataBC[i];
		if( tempPos > pos )
		{
			tempPos -= _fileDataBC[i];
			nowPos = (U32)(pos - tempPos) + _dataBeginPos;//文件实际位置
			canReadBC = _fileDataBC[i] - (I32)nowPos;
			break;
		}
	}

	assert( i < _fileDataBC.size() );

	string str;
	I32 no = GetFileNo( _firstPathName, str, _errNo );
	if( no < 0 )
		return false;

	no += i;
	string fileName = CombFileName( str, no );
	
	if( fileName != _filePathName )
	{
		fclose( _dataF );
		errno_t err = fopen_s( &_dataF, fileName.c_str(), "rb" );
		if( err )
		{
			_errNo = EM9118_FILEPATH_ERROR;
			_dataF = 0;
			return false;
		}
		_filePathName = fileName;
	}

	int ret = fseek( _dataF, nowPos, SEEK_SET );
	if( ret )
	{
		_errNo = EM9118_FILESEEK_ERROR;
		return false;
	}
	
	_nowFilePos = nowPos;

	return true;
}

U32 CHistoricalDataReader::ReadCode( I8* codeBuf, //注意其大小还需要是通道数目的整数倍
									U32 bufBC, 
									F64 pos )
{
	if( pos >= _allDataBC )
		return 0;
	
	assert( _fileDataBC.size() );

	U32 fileCanReadBC = 0;
	if( pos != -1 )
	{//设置文件位置
		if( !SetFilePos( pos, fileCanReadBC ) )
			return 0;
		_nowAllPos = pos;
	}

	U32 canReadBC = (U32)(_allDataBC - _nowAllPos);
	U32 wantReadBC = min( bufBC, canReadBC );

	//先读出本文件内容
	U32 nowReadBC = min( fileCanReadBC, wantReadBC );
	U32 retBC = fread( &codeBuf[0], 1, nowReadBC, _dataF );
	if( retBC < nowReadBC )
	{
		_errNo = EM9118_FILEREAD_ERROR;
		return retBC;
	}

	if( fileCanReadBC <= wantReadBC )
	{//需要打开下一个文件,如果不是最后一个文件的话
		if( _filePathName != _lastPathName )
		{
			fclose( _dataF );
			if( !NextFileName( _filePathName, _errNo ) )
			{//不是最后一个文件则必然可以生成下一个文件名，否则就是发生了错误
				_dataF = 0;
				return retBC;
			}

			errno_t err = fopen_s( &_dataF, _filePathName.c_str(), "rb" );
			if( err )
			{
				_errNo = EM9118_FILEPATH_ERROR;
				_dataF = 0;
				return retBC;
			}
			fseek( _dataF, _dataBeginPos, SEEK_SET );
			_nowFilePos = _dataBeginPos;
			U32 needBC = wantReadBC - fileCanReadBC;
			if( !needBC )
				return retBC;
			U32 remainBC = fread( &codeBuf[retBC], 1, needBC, _dataF );
			if( remainBC < needBC )
			{
				_errNo = EM9118_FILEREAD_ERROR;
				return remainBC + retBC;
			}
			retBC += remainBC;
			_nowAllPos += remainBC;
			_nowFilePos = ftell( _dataF );
		}
		else
		{
			assert( fileCanReadBC == wantReadBC );//最后一个文件只能是这种情况
			//最后一个文件又刚好读完的时候，不能让文件指针溢出。
			_nowAllPos = _nowAllPos;
			_nowFilePos = ftell( _dataF ) - retBC;
		}
	}
	else
	{
		_nowAllPos += retBC;
		_nowFilePos = ftell( _dataF );
	}
	return retBC;
}

U32 CHistoricalDataReader::ReadCodeByCount( I8* codeBuf, U32 groupCodeCount, F64 codeBeginInx )
{
	U32 groupBC = GetGroupBC();
	U32 bufBC = groupCodeCount * groupBC;

	if( codeBeginInx == -1 )
		return ReadCode( codeBuf, bufBC, -1 );

	
	F64 bcBeginInx = codeBeginInx * groupBC;
	if( bcBeginInx > _allDataBC )
	{
		_errNo = EM9118_FILEREAD_ERROR;
		return 0;
	}

	U32 ret = ReadCode( codeBuf, bufBC, bcBeginInx );

	return ret / groupBC;
}

I32 CHistoricalDataReader::GetGroupBC()
{
	I32 groupBC = 0;
	EM9118_GetFifoGroupByteCount( &_property, &groupBC );
	return groupBC;
}

U32 CHistoricalDataReader::GetAdChValue( I32 chInx, I8* codeBuf, U32 groupCodeCount, F64* adValue )
{
	if( codeBuf == 0 || adValue == 0 )
		return 0;
	//需要将chInx转换成真正的fifo中的通道索引，这是因为通道可以间隔使能。
	I32 fifoChInx = chInx;//？？？？？？？？？？？还没有实现

	U32 groupBC = GetGroupBC();
	U32 codeBC = groupCodeCount * groupBC;
	I32 valueInx = 0;
	for( U32 i = fifoChInx * ADBC; i < codeBC; i += groupBC )
	{
		I16 adCode = *(I16*)(codeBuf + i);
		adValue[valueInx] = AdCodeToValue( adCode, _property.adPara.zeroCode[chInx], _property.adPara.fullCode[chInx], 
			                                       _property.adPara.zeroValue[chInx], _property.adPara.fullValue[chInx] );
		valueInx++;
	}
	return groupCodeCount;
}
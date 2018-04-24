// EM9118.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <math.h>
#include <time.h>
#include "ZT_Type.h"
#include "EM9118.h"
#include "EM9118Struct.h"
#include "EM9118_Errors.h"
#include "TimeInterval.h"
#include "HistoricalDataReader.h"
#include "ZCD_CS.h"

typedef struct ZTLC_PARA 
{//设备共用参数
	ZTLC_PARA()
	{
		cmdFd = 0;
		dataFd = 0;
		InitializeCriticalSection( &sectCmd );
		isHcStart = 0;
		errorCode = 0;
	}
	SOCKET cmdFd;//命令端口
	SOCKET dataFd;//数据端口
	CRITICAL_SECTION sectCmd;
	I32 isHcStart;
	I32 errorCode;//错误码，如果产生超时错误，需要重新建立连接。
}ZTLC_Para;

ZTLC_Para* _stdcall ZTLC_CmdConnect( I8* strIP, U16 port, I32 timeoutMS )
{
	ZTLC_Para* pPara = new ZTLC_Para;
	if( pPara == 0 )
		return 0;
	if( strIP == 0 )
		return 0;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,1),&wsaData) != 0) {
		return 0;

	}
	//建立命令套接字，TCP协议
	SOCKET cmdFd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( cmdFd == INVALID_SOCKET ) {
		return 0;
	}

	//设置非阻塞方式连接   
	unsigned long ul = 1;
	int ret = ioctlsocket( cmdFd, FIONBIO, (unsigned long*)&ul );   
	if( ret == SOCKET_ERROR){
		return 0;
	}

	//连接   
	//设置地址信息
	struct sockaddr_in cmdAddress;
	memset( &cmdAddress, 0, sizeof(cmdAddress) );
	cmdAddress.sin_family = AF_INET;
	cmdAddress.sin_addr.S_un.S_addr = inet_addr(strIP);
	cmdAddress.sin_port = htons(port);
	ret = connect( cmdFd, (struct sockaddr*)&cmdAddress, sizeof(cmdAddress) );

	int err = WSAGetLastError();


	//设置超时
	struct timeval timeout;
	fd_set r;   

	FD_ZERO( &r );   
	FD_SET( cmdFd, &r );
	timeout.tv_sec = timeoutMS / 1000;
	timeout.tv_usec = (timeoutMS % 1000) * 1000000;
	ret = select( 0, 0, &r, 0, &timeout );
	err = WSAGetLastError();
	if( ret <= 0 )
	{   
		::closesocket( cmdFd );
		return 0;
	}
	//设回阻塞模式   
	unsigned long ul1 = 0;   
	ret = ioctlsocket( cmdFd, FIONBIO, (unsigned long*)&ul1 );
	if( ret == SOCKET_ERROR ){   
		closesocket( cmdFd );   
		return 0;
	}

	//	ZTLC_InitDev( pDev );
	pPara->cmdFd = cmdFd;

	return pPara;
}

void _stdcall ZTLC_CmdClose( HANDLE hDev )
{
	ZTLC_Para* p = (ZTLC_Para*)hDev;
	closesocket( p->cmdFd );
	delete p;
}

void MakeSendFrame( U16 addr, const I8 vData[256], U32 bytesTotal, I8 v[256], U32* vSize )
{
	memset( v, 0, 13 );
	v[5] = (U8)bytesTotal + 7;//后面的字符数
	v[6] = 1;
	v[7] = 16;
	v[8] = (addr >> 8) & 0xff;
	v[9] = addr & 0xff;
	v[10] = ((bytesTotal / 2) >> 8) & 0xff;
	v[11] = (bytesTotal / 2) & 0xff;
	v[12] = bytesTotal & 0xff;

	memcpy( v + 13, vData, bytesTotal );
	*vSize = 13 + bytesTotal;
}

void MakeRecvFrame( U16 addr, U32 bytesCount, I8 v[256], U32* vSize )
{
	memset( v, 0, 12 );
	U16 regCount = (U16)bytesCount / 2;
	v[5] = 6;//后面的字符数
	v[6] = 1;
	v[7] = 3;
	v[8] = (addr >> 8) & 0xff;
	v[9] = addr & 0xff;
	v[10] = (regCount >> 8) & 0xff;
	v[11] = regCount & 0xff;
	*vSize = 12;
}

I32 ClearRecvBuffer( ZTLC_Para* pDev )
{//每次读取失败后，都要清空一下缓冲区，否则就会发生错位
	//设置非阻塞方式
	//	EnterCriticalSection( &pDev->sectCmd );
	CZCD_CS cs( &pDev->sectCmd );
	U32 ul = 1;
	I32 ret = ioctlsocket( pDev->cmdFd, FIONBIO, (U32*)&ul );   
	if( ret == SOCKET_ERROR){
		return EM9118_FAILURE;

	}

	char buf[256];
	while( recv( pDev->cmdFd, buf, sizeof(buf), 0 ) > 0 );

	//设置阻塞方式连接
	ul = 0;
	ret = ioctlsocket( pDev->cmdFd, FIONBIO, (unsigned long*)&ul );   
	if( ret == SOCKET_ERROR){
		return EM9118_FAILURE;
	}
	//	LeaveCriticalSection( &pDev->sectCmd );
	return 0;
}

I32 ZTLC_SendCommand( ZTLC_Para* pDev, I8 cmd[256], U32 cmdSize, I8* retInfo, U32 retSize, I32 outtime )
{
	I32 retv;
	//	EnterCriticalSection( &pDev->sectCmd );
	CZCD_CS cs( &pDev->sectCmd );
	if( ( retv = send( pDev->cmdFd, cmd, cmdSize, 0 ) ) != cmdSize ){
		int err = WSAGetLastError();
		if( err == WSAECONNRESET  )
			return EM9118_CONNECT_ERROR;
		else
			return EM9118_SEND_ERROR;
	}

	//设定接收超时时间
	if( setsockopt( pDev->cmdFd, SOL_SOCKET, SO_RCVTIMEO, (char*)&outtime, sizeof(outtime) ) == SOCKET_ERROR )
	{
		int err = WSAGetLastError();
		if( err == WSAECONNRESET  )
			return EM9118_CONNECT_ERROR;
		else
			return EM9118_SEND_ERROR;
	}

	if( retSize == 0 )
		return retv;

	//	CTimeInterval tv;
	retv = recv( pDev->cmdFd, retInfo, retSize, 0 );
	if( retv < 0 )
	{
		int err = WSAGetLastError();
		//		ClearRecvBuffer();
		if( err == WSAETIMEDOUT  )
		{
			ClearRecvBuffer( pDev );
			pDev->errorCode = EM9118_TIMEOUT_ERROR;
			return EM9118_TIMEOUT_ERROR;
		}
		else
		{
			ClearRecvBuffer( pDev );
			if( err == WSAECONNRESET  )
			{
				pDev->errorCode = EM9118_CONNECT_ERROR;
				return EM9118_CONNECT_ERROR;
			}
			else
				return EM9118_SEND_ERROR;
		}
	}
	//	LeaveCriticalSection( &pDev->sectCmd );
	//	TRACE( "SendCommand:%f\n", tv.GetIntervalMS() );
	return retv;
}

I32 _stdcall ZTLC_SendAndVerify( HANDLE hDev, U16 addr, I8* vcData, U32 vcSize, int outtime )
{
	if( hDev == INVALID_HANDLE_VALUE || hDev == 0 )
		return EM9118_FAILURE;
	ZTLC_Para* pDev = (ZTLC_Para*)hDev;
	I8 cmd[256];
	U32 cmdSize;
	MakeSendFrame( addr, vcData, vcSize, cmd, &cmdSize );
	I8 retInfo[12];
	I32 ret = ZTLC_SendCommand( pDev, cmd, cmdSize, retInfo, sizeof(retInfo), outtime );
	if( ret )
		return ret;
	//	MyTrace( "%2x ", cmd );
	//	MyTrace( "%2x ", retInfo );
	U8 regCnt = 0;
	regCnt |= retInfo[10];
	regCnt = (regCnt<<8) | retInfo[11];

	if( regCnt * 2 != vcSize )
	{
		ClearRecvBuffer( pDev );
		return EM9118_CMDVERIFY_ERROR;
	}
	return 0;
}

I32 _stdcall ZTLC_RecvAndVerify( HANDLE hDev, U16 addr, I8* vcData, U32 vcSize, I32 outtime )
{
	if( hDev == INVALID_HANDLE_VALUE || hDev == 0 )
		return EM9118_FAILURE;

	ZTLC_Para* pDev = (ZTLC_Para*)hDev;
	I8 cmd[256];
	U32 cmdSize;
	MakeRecvFrame( addr, vcSize, cmd, &cmdSize );
	//	MyTrace( "%2x ", cmd );
	I8 retInfo[256];
	U32 retSize = vcSize + 9;
	I32 ret = ZTLC_SendCommand( pDev, cmd, cmdSize, retInfo, retSize, outtime );
	if( ret < 0 )
		return ret;

	char fnCode = retInfo[7];
	unsigned char byteCnt = (unsigned char)retInfo[8];
	if( (fnCode & 0x80) )
	{
		if( retInfo[8] == 4 )
			return EM9118_DATANOTREADY;
		else
			return EM9118_FAILURE;
	}else if( fnCode != 3 || byteCnt != vcSize )
	{
		//		MyTrace( "%2x ", retInfo );
		ClearRecvBuffer( pDev );
		return EM9118_RECVVERIFY_ERROR;
	}
	memcpy( vcData, retInfo + 9, vcSize );
	return 0;
}


inline I32 CtChBatchCodeToFreq( U16 mode, F64 freqBase, U32* pCode, I32 codeCount, F64* ctValue )
{
	switch( mode )
	{
	case EM9118_CT_MODE_COUNT:
		for( I32 i = 0; i < codeCount; ++i )
			ctValue[i] = pCode[i];
		break;
	case EM9118_CT_MODE_HFREQ:
		if( freqBase == 0 )
			return EM9118_FREQBASE_ERROR;
		else
			for( I32 i = 0; i < codeCount; ++i )
				ctValue[i] = pCode[i] * 1000 / freqBase;
		break;
	case EM9118_CT_MODE_LFREQ:
		if( freqBase == 0 )
			return EM9118_FREQBASE_ERROR;
		else
			for( I32 i = 0; i < codeCount; ++i )
				ctValue[i] = pCode[i] ? 1000 / (pCode[i] * freqBase) : 0;//测低频时最高频率就是基础时钟频率
		break;
	default:
		return EM9118_PARA_ERROR;
	}
	return EM9118_SUCCESS;
}

inline F64 AdCodeToValue( I16 iCode, I16 zeroCode, I16 fullCode, F64 zeroValue, F64 fullValue )
{
	return (iCode - zeroCode) * ( fullValue - zeroValue ) /	
		( fullCode - zeroCode ) + zeroValue;
}

inline I32 _stdcall EM9118_AdGetZeroFullValue( I32 rangeInx, F64* zeroValue, F64* fullValue )
{
	F64 fC,zC;
	switch( rangeInx )
	{
	case EM9118_AD_RANGE_N10_10V:
		fC = 10.0 * 0.9;
		zC = 0;
		break;
	case EM9118_AD_RANGE_N5_5V:
		fC = 5.0 * 0.9;
		zC = 0;
		break;
	case EM9118_AD_RANGE_0_20MA:
		fC = 20.0 * 0.9;
		zC = 0;
		break;
	default:
		return EM9118_FAILURE;
	}
	if( zeroValue )
		*zeroValue = zC;
	if( fullValue )
		*fullValue = fC;

	return 0;
}

I32 GetAdEnChCnt( I8* ad, EM9118_CommonPara& commonPara )
{
	I32 chCnt = 0;

	for( int i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		if( ad[i] )
			++chCnt;
	}
	commonPara.adChCount = chCnt;
	return chCnt;
}

I32 GetCtEnChCnt( I8* ct, EM9118_CommonPara& commonPara )
{
	I32 chCnt = 0;

	for( int i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		if( ct[i] )
			++chCnt;
	}
	commonPara.ctChCount = chCnt;
	return chCnt;
}

I32 GetEcEnChCnt( I8* ec, EM9118_CommonPara& commonPara )
{
	I32 chCnt = 0;

	for( int i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		if( ec[i] )
			++chCnt;
	}
	commonPara.ecChCount = chCnt;
	return chCnt;
}

I32 _stdcall EM9118_OlFileOpen( I8* filePathName )
{
	FILE* f;
	errno_t err = fopen_s( &f, filePathName, "rb" );
	if ( err ) {
		return -1;
	}
	OfflineSet os;
	fread( &os.version, 1, sizeof(os.version), f );
	fread( &os.version, 1, sizeof(os.version), f );
	fread( &os.intervalMS, 1, sizeof(os.intervalMS), f );
	fread( &os.intervalS, 1, sizeof(os.intervalS), f );
	fread( &os.intervalM, 1, sizeof(os.intervalM), f );
	fread( &os.intervalH, 1, sizeof(os.intervalH), f );
	fread( &os.daqStyle, 1, sizeof(os.daqStyle), f );
	fread( &os.startMode, 1, sizeof(os.startMode), f );
	fread( &os.groupCount, 1, sizeof(os.groupCount), f );
	fread( &os.isPSync, 1, sizeof(os.isPSync), f );
	fread( &os.chFreq, 1, sizeof(os.chFreq), f );
	fread( &os.groupFreq, 1, sizeof(os.groupFreq), f );
	fread( &os.adBeginCh, 1, sizeof(os.adBeginCh), f );
	fread( &os.adEndCh, 1, sizeof(os.adEndCh), f );
	fread( &os.adRange, 1, sizeof(os.adRange), f );
	fread( &os.adGain, 1, sizeof(os.adGain), f );
	fread( &os.adEndMode, 1, sizeof(os.adEndMode), f );
	fread( &os.ioInFifo, 1, sizeof(os.ioInFifo), f );
	fread( &os.ioDir, 1, sizeof(os.ioDir), f );
	fread( &os.ctMode, 1, sizeof(os.ctMode), f );
	fread( &os.ctGateFreq, 1, sizeof(os.ctGateFreq), f );
	fread( &os.ad, 1, sizeof(os.ad), f );
	fread( &os.io, 1, sizeof(os.io), f );
	fread( &os.ct, 1, sizeof(os.ct), f );
	fread( &os.ec, 1, sizeof(os.ec), f );

	I32 chInx = 0;
	for( I32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		//遍历18个通道的零点满度值，将其使能的通道按顺序填写到零点满度数组中，注意此处chInx和实际的通道号可能不一致
		fread( &os.zCode[chInx], 1, sizeof(I16), f );
		fread( &os.fCode[chInx], 1, sizeof(I16), f );
		EM9118_AdGetZeroFullValue( os.adRange, &os.zValue[chInx], &os.fValue[chInx] );
		if( os.ad[i] )
			++chInx;
	}

	size_t ss = sizeof( os );
	OfflineFile* of = new OfflineFile;
	if( !of )
		return -1;
	of->f = f;
	of->dataBeginPos = ftell( f );
/*	if( of->dataBeginPos )
	{
		return -1;
	}*/
	fseek(f, 0, SEEK_END);
	of->dataSize = ftell(f) - of->dataBeginPos;
	if( of->dataSize < 0 )
		return -1;
	fseek( f, of->dataBeginPos, SEEK_SET );

	memcpy( &of->os, &os, sizeof(os) );

	GetAdEnChCnt( of->os.ad, of->commonPara );
	GetCtEnChCnt( of->os.ct, of->commonPara );
	GetEcEnChCnt( of->os.ec, of->commonPara );

	//设定每一个元素对应的实际是第几个通道的数值。
	chInx = 0;
	for( I8 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		of->ctLogicToRealChInx[chInx] = i;
		if( os.ct[i] )
			++chInx;
	}
	
	return (I32)of;
}

I32 _stdcall EM9118_OlFileClose( I32 fileHandle )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;
	fclose( p->f );
	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_OlGetAdEnCh( I32 fileHandle, I8 enCh[EM9118_MAXADCHCNT] )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	for( int i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		if( enCh )
			enCh[i] = p->os.ad[i];
	}

	return GetAdEnChCnt( p->os.ad, p->commonPara );
}

I32 _stdcall EM9118_OlGetCtEnCh( I32 fileHandle, I8 enCh[EM9118_MAXCTCHCNT] )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	I32 chCnt = 0;

	for( int i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{

		if( enCh )
			enCh[i] = p->os.ct[i];
	}
	return GetCtEnChCnt( p->os.ct, p->commonPara );
}

I32 _stdcall EM9118_OlGetEcEnCh( I32 fileHandle, I8 enCh[EM9118_MAXECCHCNT] )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	I32 chCnt = 0;

	for( int i = 0; i < EM9118_MAXECCHCNT; ++i )
	{

		if( enCh )
			enCh[i] = p->os.ec[i];
	}
	return GetEcEnChCnt( p->os.ec, p->commonPara );
}

I32 _stdcall EM9118_OlGetGroupFreq( I32 fileHandle, F64* groupFreq )
{
	if( !fileHandle )
		return -1;
	OfflineFile* p = (OfflineFile*)fileHandle;

	if( groupFreq )
		*groupFreq = p->os.groupFreq;

	return 0;
}

I32 _stdcall EM9118_OlGetOriginalCode( I32 fileHandle, I32 readPos, U32 bytesCount, U8* originalCode, U32* realBC )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	if( readPos > p->dataSize )
		return -1;

	if( readPos >= 0 )
	{
		int ret = fseek( p->f, readPos + p->dataBeginPos, SEEK_SET );
		if( ret != 0 )
			return -1;
	}

	size_t siz = fread( originalCode, 1, bytesCount, p->f );
	
	if( realBC )
		*realBC = siz;
	return ftell( p->f );
}

inline I32 GetGroupBytesCount( const EM9118_CommonPara& p )
{
	return p.adChCount * ADBC + p.ctChCount * CTBC + p.ecChCount * ECBC + p.ioInFifo * IOBC;
}

I32 _stdcall EM9118_OlGetGroupBytesCount( I32 fileHandle )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;
	return GetGroupBytesCount( p->commonPara );
}

I32 _stdcall EM9118_OlGetDataBytesCount( I32 fileHandle )
{
	if( !fileHandle )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;
	return p->dataSize;
}

I32 _stdcall EM9118_OlGetAdChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, I16* adChCode, U32* adCodeCount )
{
	if( !fileHandle || !originalCode  || !adChCode )
		return -1;
	if( chInx >= EM9118_MAXADCHCNT )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	I16* adCode = (I16*)originalCode;
	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 codeCount = bytesCount / 2;//姑且将全部数据均看成是16位
	U32 stepCount = gBC / 2;
	U32 j = 0;
	for( U32 i = chInx; i < codeCount; i += stepCount )
	{
		adChCode[j++] = adCode[i];
	}

	if( adCodeCount )
		*adCodeCount = j;
	return 0;
}

I32 _stdcall EM9118_OlGetAdChValue( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, F64* adChValue, U32* adValueCount )
{
	if( !fileHandle || !originalCode  || !adChValue )
		return -1;
	if( chInx >= EM9118_MAXADCHCNT )
		return -1;
	OfflineFile *p = (OfflineFile*)fileHandle;

	I16* adCode = (I16*)originalCode;
	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 codeCount = bytesCount / 2;//姑且将全部数据均看成是16位
	U32 stepCount = gBC / 2;
	U32 j = 0;

	for( U32 i = chInx; i < codeCount; i += stepCount )
	{
		adChValue[j++] = AdCodeToValue( adCode[i], p->os.zCode[chInx], p->os.fCode[chInx], p->os.zValue[chInx], p->os.fValue[chInx] );
	}

	if( adValueCount )
		*adValueCount = j;
	return 0;
}

I32 _stdcall EM9118_OlGetCtChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, U32* ctChCode, U32* ctCodeCount )
{
	if( !fileHandle || !originalCode  || !ctChCode )
		return -1;
	if( chInx >= EM9118_MAXCTCHCNT )
		return -1;

	OfflineFile *p = (OfflineFile*)fileHandle;

	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 j = 0;
	for( U32 i = chInx * CTBC + p->commonPara.adChCount * ADBC; i < bytesCount; i += gBC )
	{
		U32* ctCode = (U32*)( originalCode + i );
		ctChCode[j++] = *ctCode;
	}

	if( ctCodeCount )
		*ctCodeCount = j;
	return 0;
}

I32 _stdcall EM9118_OlGetCtChValue( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, F64* ctChValue, U32* ctValueCount )
{
	if( fileHandle == -1 || fileHandle == 0 )
		return EM9118_FAILURE;
	OfflineFile *p = (OfflineFile*)fileHandle;

	if( !originalCode || !ctChValue )
	{
		return EM9118_FAILURE;
	}

	V_U32 ctCode(bytesCount);
	U32 ctCodeCount = 0;
	I32 ret = EM9118_OlGetCtChCode( fileHandle, chInx, bytesCount, originalCode, &ctCode[0], &ctCodeCount );
	if( !ret )
		return ret;
	
	if( ctValueCount )
		*ctValueCount = ctCodeCount;
	//将逻辑通道转换成实际通道
	I32 realChInx = p->ctLogicToRealChInx[chInx];
	return CtChBatchCodeToFreq( p->os.ctMode[realChInx], p->os.ctGateFreq[realChInx], &ctCode[0], ctCodeCount, ctChValue );
}

I32 _stdcall EM9118_OlGetEcChCode( I32 fileHandle, I32 chInx, U32 bytesCount, U8* originalCode, I32* ecChCode, U32* ecCodeCount )
{
	if( !fileHandle || !originalCode  || !ecChCode )
		return -1;
	if( chInx >= EM9118_MAXECCHCNT )
		return -1;

	OfflineFile *p = (OfflineFile*)fileHandle;

	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 j = 0;
	for( U32 i = chInx * ECBC + p->commonPara.adChCount * ADBC + p->commonPara.ctChCount * CTBC; i < bytesCount; i += gBC )
	{
		U32* ecCode = (U32*)( originalCode + i );
		ecChCode[j++] = *ecCode;
	}

	if( ecCodeCount )
		*ecCodeCount = j;
	return 0;
}

typedef struct EM9118_OLDIR
{
	FILE* f;
	I64 dataBeginPos;//数据开始位置
	I64 dataNowPos;//数据当前位置
	I64 dataSize;//数据总大小
	I32 fileCount;//数据文件个数
	V_STR fileName;//数据文件名数组，每个大小是1024
	OfflineSet os;
	I32 adChCnt;
}Em9118_OlDir;

BOOL IsOlDir( const string& str )
{
	if( str.length() != 6 )
		return FALSE;
	for( size_t i = 0; i < str.length(); ++i )
	{
		if( str[i] < '0' || str[i] > '9'  )
			return FALSE;
	}
	return TRUE;
}

I32 _stdcall EM9118_OlDirOpen( I8* path )
{
	size_t len = strlen( path );
	if( path[len-1] == '\\' )
	{
		path[len] = '*';
		path[len+1] = 0;
	}else
	{
		path[len] = '\\';
		path[len+1] = '*';
		path[len+2] = 0;
	}
	Em9118_OlDir* pDir = new Em9118_OlDir;
	WIN32_FIND_DATA wd;
	HANDLE h = FindFirstFile( path, &wd );

	if( h == INVALID_HANDLE_VALUE )
	{
		return -1;
	}

	BOOL b;
	V_STR strDir;
	do
	{
		b = FindNextFile( h, &wd );
		DWORD dir = GetFileAttributes( wd.cFileName );
		if( dir == FILE_ATTRIBUTE_DIRECTORY )
		{
			if( IsOlDir( wd.cFileName ) )
				strDir.push_back( wd.cFileName );
		}
	}while( b );
//	判断问题

	if( !strDir.size() )
		return -1;

	return (I32)pDir;
}

/*V_U8 chCounter(18);
std::vector<V_I16> chCode(18);

I32 _stdcall EM9118_Init()
{
	chCounter.assign( chCounter.size(), 0 );
	return 0;
}

I32 _stdcall EM9118_ExtractADCode( U8* enCh, U16* chDiv, I16* codeBuffer, U32 codeCount, U32* chCodeCount )
{
	for( size_t i = 0; i < chCounter.size(); ++i )
	{
		chCode[i].clear();
		if( chDiv[i] == 0 )
			return -1;
	}
	for( U32 i = 0; i < codeCount; )
	{
		for( size_t j = 0; j < chCounter.size(); ++j )
		{
			if( enCh[j] && (chDiv[j]==++chCounter[j]) )
			{
				if( i == codeCount )
					break;

				chCounter[j] = 0;
				chCode[j].push_back( codeBuffer[i] );
				++chCodeCount[j];

				++i;
			}
		}
	}
	return 0;
}

I32 _stdcall EM9118_GetADChCode( I32 chNo, I16* chCodeBuffer )
{
	if( chNo < 1 || chNo > 18 )
		return -1;
	if( chCode[chNo-1].size() && chCodeBuffer )
		memcpy( chCodeBuffer, &chCode[chNo-1][0], chCode[chNo-1].size() * sizeof(I16) );
	else
		return -1;
	size_t tt = chCode[chNo - 1].size();
	return 0;
}*/

EM9118_DevPara* _stdcall EM9118_DeviceCreate()
{
	EM9118_DevPara* pDev = new EM9118_DevPara;
	if( !pDev )
		return 0;

	memset( pDev->strIP, 0, sizeof(pDev->strIP) );

	memset( pDev->adPara.isInFifo, 1, EM9118_MAXADCHCNT );
	memset( pDev->ctInFifo, 0, EM9118_MAXCTCHCNT );
	memset( pDev->ecInFifo, 0, EM9118_MAXECCHCNT );
	memset( &pDev->commonPara, 0, sizeof(pDev->commonPara) );

	pDev->adPara.rangeInx = EM9118_AD_RANGE_N10_10V;

	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		pDev->ctPara.mode[i] = EM9118_CT_MODE_COUNT;
		pDev->ctPara.freqBase[i] = 1000;
	}

	pDev->ioPara.doMod[0] = EM9118_DO_MODE_DO;
	pDev->ioPara.doMod[1] = EM9118_DO_MODE_DO;

	pDev->groupFreq = 100000;
	pDev->triSrc = EM9118_CLKSRC_IN;
	pDev->edgeOrLevel = EM9118_TRI_EDGE;
	pDev->upOrDown = EM9118_TRI_UP;
	pDev->singleFileSize = 500 * 1024 * 1024;
	pDev->csvLinesCount = 100000;

	pDev->isPick = 1;
	pDev->realGroupFreq = 0;
	pDev->pickCodeCnt = 0;
	pDev->hCmd = 0;
	pDev->dataFd = 0;
	
	return pDev;
}

void _stdcall EM9118_DeviceClose( EM9118_DevPara* pDev )
{
	if(pDev)
		delete pDev;
}

I32 _stdcall EM9118_CmdConnect( EM9118_DevPara* pDev, char* strIP, I32 ipBC, I32 cmdPort, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	
	if( strIP == 0 )
		return EM9118_SOCKET_ERROR;

	if( !ipBC )
		ipBC = strlen( strIP );
	
	if( ipBC && ipBC < 256 )
	{
		memcpy( pDev->strIP, strIP, ipBC );
		pDev->strIP[ipBC] = 0;
	}

	HANDLE hCmd = ZTLC_CmdConnect( pDev->strIP, (U16)cmdPort, timeOutMS );
	if( hCmd == INVALID_HANDLE_VALUE || hCmd == 0 )
		return EM9118_FAILURE;

	pDev->hCmd = hCmd;
	pDev->TimeOutCnt = 0;
	InitializeCriticalSection( &pDev->sectCmd );
	return EM9118_SUCCESS;
}

I32 SetAutoSend( EM9118_DevPara* pDev, I8 isSutoSend, I32 timeOutMS )
{
	V_I8 vc(2,0);
	vc[1] = isSutoSend;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 23, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return 0;
}

I32 _stdcall EM9118_DataConnect( EM9118_DevPara* pDev, I32 dataPort, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( pDev->strIP[0] == 0 )
		return EM9118_SOCKET_ERROR;

	if( pDev->hCmd == 0 )
		return EM9118_SOCKET_ERROR;

	//先使能自动传输数据
	int ret = SetAutoSend( pDev, 1, timeOutMS );
	if( ret < 0 )
		return ret;

	
	//建立命令套接字，TCP协议
	SOCKET dataFd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( dataFd == INVALID_SOCKET ) {
		return EM9118_SOCKET_ERROR;
	}

	//设置非阻塞方式连接   
	unsigned long ul = 1;
	ret = ioctlsocket( dataFd, FIONBIO, (unsigned long*)&ul );   
	if( ret == SOCKET_ERROR){
		return EM9118_SOCKET_ERROR;
	}

	//连接   
	//设置地址信息
	struct sockaddr_in cmdAddress;
	memset( &cmdAddress, 0, sizeof(cmdAddress) );
	cmdAddress.sin_family = AF_INET;
	cmdAddress.sin_addr.S_un.S_addr = inet_addr( pDev->strIP);
	cmdAddress.sin_port = htons((U16)dataPort);
	ret = connect( dataFd, (struct sockaddr*)&cmdAddress, sizeof(cmdAddress) );

	//设置超时
	struct timeval timeout;
	fd_set r;   

	FD_ZERO( &r );   
	FD_SET( dataFd, &r );
	timeout.tv_sec = timeOutMS / 1000;
	timeout.tv_usec = (timeOutMS % 1000) * 1000000; 
	ret = select( 0, 0, &r, 0, &timeout );   
	if( ret <= 0 )
	{   
		::closesocket( dataFd );
		return EM9118_SOCKET_ERROR;
	}
	//设回阻塞模式   
	unsigned long ul1 = 0;   
	ret = ioctlsocket( dataFd, FIONBIO, (unsigned long*)&ul1 );
	if( ret == SOCKET_ERROR ){   
		closesocket( dataFd );   
		return EM9118_SOCKET_ERROR;
	}
	pDev->dataFd = dataFd;
	return EM9118_SUCCESS;
}

void _stdcall EM9118_CmdClose( EM9118_DevPara* pDev, I32 timeOutMS )
{
	if( !pDev )
		return;
	

	if( pDev->dataFd )
		EM9118_DataClose( pDev, timeOutMS );

	if( pDev->hCmd )
	{
		ZTLC_CmdClose( pDev->hCmd );
		DeleteCriticalSection( &pDev->sectCmd );
	}
	pDev->hCmd = 0;
}

void _stdcall EM9118_DataClose( EM9118_DevPara* pDev, I32 timeOutMS )
{
	if( !pDev )
		return;
	
	if( pDev->dataFd == 0 )
		return;
	closesocket( pDev->dataFd );
	//先禁止自动传输数据
	SetAutoSend( pDev, 0, timeOutMS );

	pDev->dataFd = 0;
}

I32 _stdcall EM9118_CmdSetInFifo( EM9118_DevPara* pDev, I32 timeOutMS )
{
	V_I8 vc( 4, 0 );
	for( I32 chInx = 0; chInx < EM9118_MAXADCHCNT; ++chInx )
	{
		I32 vcInx = chInx / 8;
		I32 offBit = chInx % 8;
		vc[vcInx] |= pDev->adPara.isInFifo[chInx] << offBit;
	}
	vc[2] |= (pDev->diInFifo&3)<<6;

	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		vc[3] |= pDev->ctInFifo[i] << (i+2);
	}

	if( pDev->ecInFifo[0] )
		vc[3] |= (1) | (1<<1);
	if( pDev->ecInFifo[1] )
		vc[3] |= (1<<6) | (1<<7);

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 14, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[18], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	for( I32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		pDev->adPara.isInFifo[i] = isInFifo[i] & 1;
	}

	//给commonPara.adChCount赋值
	GetAdEnChCnt( pDev->adPara.isInFifo, pDev->commonPara );

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_CtSetModeAll( EM9118_DevPara* pDev, I8 ctMode[EM9118_MAXCTCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	U16 ctModeAll = 0;
	I32 ctSize = sizeof(U16) * EM9118_MAXCTCHCNT;
	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		ctModeAll |= (((U16)ctMode[i]) & 3) << (i*2);
		pDev->ctPara.mode[i] = ctMode[i];
	}
	V_I8 vc(2, 0);
	vc[0] = (ctModeAll>>8) & 0xff;
	vc[1] = ctModeAll & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 52, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return 0;
}

bool IsCtChRight( I32 chNo )
{
	return 1 <= chNo && chNo <= EM9118_MAXCTCHCNT;
}


I32 _stdcall EM9118_CtClear( EM9118_DevPara* pDev, I32 chNo, I32 timeOutMS )
{
	if( !IsCtChRight(chNo) )
		return EM9118_FAILURE;
	if( !pDev )
		return EM9118_FAILURE;
	

	I32 chInx = chNo - 1;
	pDev->ctPara.mode[chInx] = 2;
	EM9118_CtSetModeAll( pDev, pDev->ctPara.mode );
	Sleep(2);
	pDev->ctPara.mode[chInx] = 0;
	EM9118_CtSetModeAll( pDev, pDev->ctPara.mode );

	return 0;
}

I32 _stdcall EM9118_CtReadCodeAll( EM9118_DevPara* pDev, U32 ctCode[EM9118_MAXCTCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(sizeof(U32) * EM9118_MAXCTCHCNT);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 274, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	//	MyTrace( "%2x", vc );
	memset( ctCode, 0, vc.size() );
	I32 vcInx = 0;
	for( I32 codeInx = 0; codeInx < EM9118_MAXCTCHCNT; ++codeInx )
	{
		ctCode[codeInx] |= vc[vcInx];
		for( I32 byteInx = 1; byteInx < 4; ++byteInx )
		{
			ctCode[codeInx] <<= 8;
			++vcInx;
			ctCode[codeInx] |= (U8)vc[vcInx];
		}
		++vcInx;
	}
	return 0;
}

I32 _stdcall EM9118_CtSetFreqBase( EM9118_DevPara* pDev, I32 chNo, F64 freqBase_ms, F64* real_ms, I32 timeOutMS )
{
	if( !IsCtChRight( chNo ) )
		return EM9118_FAILURE;
	if( !pDev )
		return EM9118_FAILURE;
	

	U32 div = (U32)(EM9118_BASE_FREQ / (1000.0 / freqBase_ms));
	V_I8 vc(4, 0);
	vc[0] |= ((div >> 24) & 0xff);
	vc[1] |= ((div >> 16) & 0xff);
	vc[2] |= ((div >> 8) & 0xff);
	vc[3] |= (div & 0xff);
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, (U16)(40 + (chNo - 1) * 2), &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	I32 chInx = chNo - 1;
	pDev->ctPara.freqBase[chInx] = 1000.0 / (EM9118_BASE_FREQ / div);
	if( real_ms )
		*real_ms = pDev->ctPara.freqBase[chInx];
	return 0;
}

I32 _stdcall EM9118_CtChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[EM9118_MAXCTCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		pDev->ctInFifo[i] = isInFifo[i] & 1;
	}
	//给commonPara.ctChCount赋值
	GetCtEnChCnt( pDev->ctInFifo, pDev->commonPara );

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_EcChIsInFifo( EM9118_DevPara* pDev, I32 isInFifo[EM9118_MAXECCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	for( I32 i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		pDev->ecInFifo[i] = isInFifo[i] & 1;
	}

	//给commonPara.ecChCount赋值
	GetEcEnChCnt( pDev->ecInFifo, pDev->commonPara );

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DiIsInFifo( EM9118_DevPara* pDev, I32 isInFifo, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	pDev->diInFifo = isInFifo & 1;
	pDev->commonPara.ioInFifo = isInFifo & 1;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdReadZeroFullCode( EM9118_DevPara* pDev, I16 zeroCode[18], I16 fullCode[18], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(72);

	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 200, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	for( I32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		I32 vcInx = i * 4;
		zeroCode[i] = ((U8)vc[vcInx] << 8) | (U8)vc[vcInx+1];
		vcInx += 2;
		fullCode[i] = ((U8)vc[vcInx] << 8) | (U8)vc[vcInx+1];
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdSetRange( EM9118_DevPara* pDev, I32 rangeInx, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc( 2, 0 );
	if( rangeInx == 2 )
		vc[1] = EM9118_AD_RANGE_N5_5V;
	else
		vc[1] = rangeInx & 0xff;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 10, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	//下位机设置完采集范围后，读回的就是当前范围的零点满度值
	I16 zeroCode[EM9118_MAXADCHCNT], fullCode[EM9118_MAXADCHCNT];
	ret = EM9118_AdReadZeroFullCode( pDev, zeroCode, fullCode, timeOutMS );
	if( ret < 0 )
		return ret;

	F64 zeroValue, fullValue;
	ret = EM9118_AdGetZeroFullValue( rangeInx, &zeroValue, &fullValue );
	if( ret < 0 )
		return ret;

	pDev->adPara.rangeInx = rangeInx;
	for( I32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		pDev->adPara.zeroCode[i] = zeroCode[i];
		pDev->adPara.fullCode[i] = fullCode[i];
		pDev->adPara.zeroValue[i] = zeroValue;
		pDev->adPara.fullValue[i] = fullValue;
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* originalCode, I16* adCode, U32* adCodeCount )
{
	if( !pDev )
		return EM9118_FAILURE;
	if( chInx >= EM9118_MAXADCHCNT )
		return EM9118_FAILURE;

	EM9118_DevPara* p = (EM9118_DevPara*)pDev;

	I16* tempCode = (I16*)originalCode;
	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 codeCount = bytesCount / 2;//姑且将全部数据均看成是16位
	U32 stepCount = gBC / 2;
	U32 j = 0;

	for( U32 i = chInx; i < codeCount; i += stepCount )
	{
		adCode[j++] = tempCode[i];
	}

	if( adCodeCount )
		*adCodeCount = j;
	return 0;
}

I32 _stdcall EM9118_CtChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* originalCode, U32* ctChCode, U32* ctCodeCount )
{
	if( !pDev )
		return EM9118_FAILURE;
	if( chInx >= EM9118_MAXCTCHCNT )
		return EM9118_FAILURE;

	EM9118_DevPara* p = (EM9118_DevPara*)pDev;

	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 j = 0;
	for( U32 i = chInx * CTBC + p->commonPara.adChCount * ADBC; i < bytesCount; i += gBC )
	{
		U32* ctCode = (U32*)( originalCode + i );
		ctChCode[j++] = *ctCode;
	}

	if( ctCodeCount )
		*ctCodeCount = j;
	return 0;
}

I32 _stdcall EM9118_CtChBatchCodeToValue( EM9118_DevPara* pDev, I32 chNo, U32* pCode, I32 codeCount, F64* ctValue )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !ctValue || !pCode )
	{
		return EM9118_FAILURE;
	}

	I32 chInx = chNo - 1;
	return CtChBatchCodeToFreq( pDev->ctPara.mode[chInx], pDev->ctPara.freqBase[chInx], pCode, codeCount, ctValue );
}

I32 _stdcall EM9118_EcChGetCode( EM9118_DevPara* pDev, I32 chInx, U32 bytesCount, I8* originalCode, I32* ecChCode, U32* ecCodeCount )
{
	if( !pDev )
		return EM9118_FAILURE;
	if( chInx >= EM9118_MAXECCHCNT )
		return EM9118_FAILURE;

	EM9118_DevPara* p = (EM9118_DevPara*)pDev;

	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 j = 0;
	for( U32 i = chInx * ECBC + p->commonPara.adChCount * ADBC + p->commonPara.ctChCount * CTBC; i < bytesCount; i += gBC )
	{
		U32* ecCode = (U32*)( originalCode + i );
		ecChCode[j++] = *ecCode;
	}

	if( ecCodeCount )
		*ecCodeCount = j;
	return 0;
}

I32 _stdcall EM9318_IoGetCode( EM9118_DevPara* pDev, U32 bytesCount, I8* originalCode, U16* diCode, U32* diCodeCount )
{
	if( !pDev )
		return EM9118_FAILURE;
	
	EM9118_DevPara* p = (EM9118_DevPara*)pDev;

	I32 gBC = GetGroupBytesCount( p->commonPara );
	U32 j = 0;
	U32 startInx = p->commonPara.ecChCount * ECBC + p->commonPara.adChCount * ADBC + p->commonPara.ctChCount * CTBC;
	for( U32 i = startInx; i < bytesCount; i += gBC )
	{
		U16* tempCode = (U16*)( originalCode + i );
		diCode[j++] = *tempCode;
	}

	if( diCodeCount )
		*diCodeCount = j;
	return 0;
}

I32 _stdcall EM9118_AdChCodeToValue( EM9118_DevPara* pDev, I32 chNo, I16 iCode, F64* adValue )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !adValue )
	{
		return EM9118_FAILURE;
	}

	I32 chInx = chNo - 1;
	if( adValue )
		*adValue = AdCodeToValue( iCode, pDev->adPara.zeroCode[chInx], 
										pDev->adPara.fullCode[chInx], 
										pDev->adPara.zeroValue[chInx], 
										pDev->adPara.fullValue[chInx] );

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdChBatchCodeToValue( EM9118_DevPara* pDev, I32 chNo, I16* pCode, I32 codeCount, F64* adValue )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !adValue || !pCode )
	{
		return EM9118_FAILURE;
	}

	I32 chInx = chNo - 1;
	for( I32 i = 0; i < codeCount; ++i )
		adValue[i] = AdCodeToValue( pCode[i], pDev->adPara.zeroCode[chInx], 
									pDev->adPara.fullCode[chInx], 
									pDev->adPara.zeroValue[chInx], 
									pDev->adPara.fullValue[chInx] );

	return EM9118_SUCCESS;
}

I32 _stdcall HcSetGroupFreq( EM9118_DevPara* pDev,  F64 groupFreq, F64* realFreq, I32 timeOutMS )
{
	if( groupFreq < 1 || groupFreq > 450000 )
		return EM9118_FAILURE;

	U32 freqDiv = (U32)(EM9118_BASE_FREQ / groupFreq);
	V_I8 vc(4);
	vc[0] = (freqDiv >> 8) & 0xff;
	vc[1] = freqDiv & 0xff;
	vc[2] = (freqDiv >> 24) & 0xff;
	vc[3] = (freqDiv >> 16) & 0xff;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 16, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	if( realFreq )
		*realFreq = EM9118_BASE_FREQ / freqDiv;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_HcSetGroupFreq( EM9118_DevPara* pDev,  F64 groupFreq, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	F64 realFreq;
	if( HcSetGroupFreq( pDev, groupFreq, &realFreq, timeOutMS ) < 0 )
		return EM9118_FAILURE;

	pDev->groupFreq = realFreq;

	return EM9118_SUCCESS;
}

I32 SendStartOrStop( EM9118_DevPara* pDev, I32 clkSrc, I32 triSrc, I32 edgeOrLevel, I32 upOrDown, I32 isStart, I32 timeOutMS )
{

	const I32 MIN_BC = 8192 * 2;//下位机fifo半满
	//计算0.8s所需要数据量
	I32 gBC = GetGroupBytesCount( pDev->commonPara );
	I32 gF = (I32)pDev->groupFreq;
	I32 bc08 = ((I32)(gBC * pDev->groupFreq * 0.8) / gF) * gF;
	if( bc08 == 0 )
		return EM9118_FAILURE;
	I32 needBC = bc08;
	pDev->pickCodeCnt = 0;
	while( needBC < MIN_BC )
	{
		pDev->pickCodeCnt += 10;
		needBC = pDev->pickCodeCnt * bc08;
	}
	pDev->realGroupFreq = pDev->groupFreq * pDev->pickCodeCnt;
	if( pDev->realGroupFreq > 0 )
	{
		F64 realFreq;
		I32 ret = HcSetGroupFreq( pDev, pDev->realGroupFreq, &realFreq, timeOutMS );
		if( ret < 0 )
			return ret;
		pDev->realGroupFreq = realFreq;
		pDev->groupFreq = realFreq / pDev->pickCodeCnt;
	}

	isStart &= 1;
	clkSrc &= 1;
	triSrc &= 1;
	edgeOrLevel &= 1;
	upOrDown &= 1;
	I32 ctrlCode = isStart | (clkSrc<<1) | (triSrc<<2) | (edgeOrLevel<<3) | (upOrDown<<4);
	V_I8 vc(2);
	vc[1] = ctrlCode & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 22, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	pDev->isHcStart = isStart;
	pDev->clkSrc = clkSrc;
	pDev->triSrc = triSrc;
	pDev->edgeOrLevel = edgeOrLevel;
	pDev->upOrDown = upOrDown;

	return 0;
}

I32 _stdcall EM9118_GetFifoAdChCount( EM9118_DevPara* pDev )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	I32 c = 0;
	for( I32 i = 0; i < EM9118_MAXADCHCNT; ++i )
		c += pDev->adPara.isInFifo[i];

	pDev->commonPara.adChCount = c;
	return c;
}

I32 _stdcall EM9118_GetFifoCtChCount( EM9118_DevPara* pDev )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	I32 c = 0;
	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
		c += pDev->ctInFifo[i];
	pDev->commonPara.ctChCount = c;
	return c;
}

I32 _stdcall EM9118_GetFifoEcChCount( EM9118_DevPara* pDev )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	I32 c = 0;
	for( I32 i = 0; i < EM9118_MAXECCHCNT; ++i )
		c += pDev->ecInFifo[i];
	pDev->commonPara.ecChCount = c;
	return c;
}

I32 _stdcall EM9118_GetFifoGroupByteCount( EM9118_DevPara* pDev, I32* byteCount )
{
	if( !pDev )
		return EM9118_FAILURE;
	
	
	if( byteCount )
		*byteCount = GetGroupBytesCount( pDev->commonPara );
	return EM9118_SUCCESS;
}



I32 _stdcall EM9118_HcStart( EM9118_DevPara* pDev, I32 clkSrc, I32 triSrc, I32 edgeOrLevel, I32 upOrDown, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	I32 ret = EM9118_CmdSetInFifo( pDev, timeOutMS );
	if( ret < 0 )
		return ret;
	
	return SendStartOrStop( pDev, clkSrc, triSrc, edgeOrLevel, upOrDown, 1, timeOutMS );
	//	return SendStartOrStop( pDev, 0, startMode & 3, timeOutMS );
}

I32 _stdcall EM9118_HcStop( EM9118_DevPara* pDev, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	return SendStartOrStop( pDev, 0, 0, 0, 0, 0, timeOutMS );
}

#include <stdarg.h>

void DebugPrint( char* fmt, ... )
{
	va_list list;
	char temp[1024];
	va_start( list, fmt );
	vsprintf_s( temp, sizeof(temp), fmt, list );
	OutputDebugString( temp );
	va_end(list);
}

I32 _stdcall EM9118_HcReadData( EM9118_DevPara* pDev, I32 byteCount, I8* CodeBuffer, I32* realByteCount, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	if( !CodeBuffer )
		return EM9118_FAILURE;
	

	if( !pDev->dataFd )
		return EM9118_CONNECT_ERROR;
	

	//设定接收超时时间
	if( setsockopt( pDev->dataFd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeOutMS, sizeof(timeOutMS) ) == SOCKET_ERROR )
	{
		int err = WSAGetLastError();
		if( err == WSAECONNRESET  )
			return EM9118_CONNECT_ERROR;
		else
			return EM9118_FAILURE;
	}

	//为了解决用户设置低频时可以快速读取数据的问题
	I8* userCodeBuffer = CodeBuffer;
	I32 userByteCount = byteCount;
	V_I8 tempBuffer( byteCount * pDev->pickCodeCnt );
	if( pDev->pickCodeCnt )
	{
		CodeBuffer = &tempBuffer[0];
		byteCount *= pDev->pickCodeCnt;
	}

	U32 dataBC = byteCount;
	if( dataBC < 0 )
		return EM9118_FAILURE;
	I32 leftBC = dataBC;
	I32 readBC = 0;
	CTimeInterval tv;
	F64 nowMS = 0;
	do{
		U32 canRead;
		if( ioctlsocket( pDev->dataFd, FIONREAD, &canRead ) )
			return EM9118_CONNECT_ERROR;
		if( canRead == 0 )
		{
			Sleep(1);
			nowMS = tv.GetIntervalMS();
			continue;
		}
//		int retv = recv( pDev->dataFd, CodeBuffer + readBC, leftBC, 0 );
		canRead = (I32)canRead < leftBC ? canRead : leftBC;
		int retv = recv( pDev->dataFd, CodeBuffer + readBC, canRead, 0 );
		if( retv < 0 )
		{
			int err = WSAGetLastError();
			if( err == WSAETIMEDOUT  )
			{
//				DebugPrint( "EM9118_HcReadData Out:readBC:%d\n", readBC );
				return EM9118_TIMEOUT_ERROR;
			}
			else
			{
				if( err == WSAECONNRESET  )
					return EM9118_CONNECT_ERROR;
				else
					return EM9118_FAILURE;
			}
		}
		readBC += retv;
		leftBC -= retv;
		nowMS = tv.GetIntervalMS();
//		DebugPrint( "EM9118_HcReadData:readBC:%d, leftBC:%d, byteCount:%d\n", readBC, leftBC, byteCount );
	}while( nowMS < timeOutMS && leftBC > 0);

	//为了解决用户设置低频时可以快速读取数据的问题
	//挑选出数据
	I32 userRealBc = readBC;
	if( pDev->pickCodeCnt )
	{
		I32 gBC = GetGroupBytesCount( pDev->commonPara );
		I32 jumpBC = pDev->pickCodeCnt * gBC;
		userRealBc = 0;
		for( I32 i = (pDev->pickCodeCnt - 1) * gBC; i <= readBC - gBC; i += jumpBC )
		{
			if( userRealBc <= byteCount - gBC )
				memcpy( userCodeBuffer + userRealBc, &CodeBuffer[i], gBC );
				userRealBc += gBC;
		}
	}
	readBC = userRealBc;


	if( realByteCount )
		*realByteCount = readBC;

/*	char str[256];
	sprintf_s( str, 256, "EM9118_HCReadData:%d,%d,%5.1f\n", CodeBuffer[0], readBC, nowMS );
	OutputDebugString( str );*/

	if( nowMS >= timeOutMS )
		return EM9118_TIMEOUT_ERROR;
	else
		return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DoSetMode( EM9118_DevPara* pDev, I8 doMode1, I8 doMode2, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2);
	vc[0] = doMode1;
	vc[1] = doMode2;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 34, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

bool IsEM9318IoChRight( I32 chNo )
{
	return 0 < chNo && chNo <= EM9318_MAXIOCHCNT;
}

I32 _stdcall EM9318_IoSetMode( EM9118_DevPara* pDev, I32 chNo, I8 ioMode, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	if( !IsEM9318IoChRight( chNo ) )
	{
		return EM9118_PARA_ERROR;
	}

	

	V_I8 vc(2);
	vc[0] = I8(chNo);
	vc[1] = ioMode;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 34, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DiAll( EM9118_DevPara* pDev, I8 iStatus[4], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 282, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	for( I32 j = 0; j < EM9118_MAXDICHCNT; ++j )
	{
		iStatus[j] = (vc[1] >> j) & 1;
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DoAll( EM9118_DevPara* pDev, I8 iStatus[2], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2, 0);
	vc[1] = (iStatus[1]&1) * 2 + iStatus[0];

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 36, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9318_IoSetAll( EM9118_DevPara* pDev, I8 oStatus[EM9318_MAXIOCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2, 0);
	for( I32 i = 0; i < EM9318_MAXIOCHCNT; ++i )
	{
		vc[1] |= oStatus[i] << i;
	}

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 36, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_EcReadCodeAll( EM9118_DevPara* pDev, I32 abCode[EM9118_MAXECCHCNT], I32 zCode[EM9118_MAXECCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(sizeof(I32) * 2 * EM9118_MAXECCHCNT);

	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 283, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	//	MyTrace( "%2x", vc );
	if( abCode )
		memset( abCode, 0, sizeof(I32) * EM9118_MAXECCHCNT );
	if( zCode )
		memset( zCode, 0, sizeof(I32) * EM9118_MAXECCHCNT );
	I32 vcInx = 0;
	for( I32 codeInx = 0; codeInx < EM9118_MAXECCHCNT; ++codeInx )
	{
		//得到AB值
		if( abCode )
		{
			abCode[codeInx] |= vc[vcInx];
			for( I32 byteInx = 1; byteInx < 4; ++byteInx )
			{
				abCode[codeInx] <<= 8;
				++vcInx;
				abCode[codeInx] |= (U8)vc[vcInx];
			}
		}
		//得到Z值
		vcInx += 3;
		if( zCode )
			zCode[codeInx] = (vc[vcInx] << 8) | vc[vcInx + 1];
		vcInx += 2;
	}
	return 0;
}

I32 _stdcall EM9118_EcClear( EM9118_DevPara* pDev, I32 chClear[EM9118_MAXECCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2, 0);
	vc[1] = (chClear[1]&1) * 2 + (chClear[0]&1);

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 193, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}


//PWM脉冲是否输出到指定个数
I32 _stdcall EM9118_PwmIsOver( EM9118_DevPara* pDev, I8 isOver[EM9118_MAXPWMCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 105, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	if( isOver )
		for( int i = 0; i < EM9118_MAXPWMCHCNT; ++i )
		{
			isOver[i] = (vc[1] >> i) & 1;
		}

	return 0;
}

bool IsPwmChRight( I32 chNo )
{
	return 0 < chNo && chNo <= EM9118_MAXPWMCHCNT;
}

//设置PWM脉冲的周期和占空比
I32 _stdcall EM9118_PwmSetPulse( EM9118_DevPara* pDev, I32 chNo, double freq, double dutyCycle, double* realFreq, double* realDutyCycle, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !IsPwmChRight( chNo ) )
		return EM9118_FAILURE;

	if( freq == 0 )
		return -1;
	U32 freqDiv = (U32)(EM9118_PWM_BASE_FREQ / freq);
	if( freqDiv > 65535 )
		return -1;
	U32 dutyDiv = (U32)(freqDiv * dutyCycle);
	if( dutyDiv > freqDiv )
		return -1;

	if( realFreq )
		*realFreq = EM9118_PWM_BASE_FREQ * 1.0 / freqDiv;
	if( realDutyCycle )
		*realDutyCycle = dutyDiv*1.0 / freqDiv;

	V_I8 vc(2, 0);
	vc[0] = (freqDiv>>8) & 0xff;
	vc[1] = freqDiv & 0xff;
	ZTLC_SendAndVerify( pDev->hCmd, U16(80 + chNo - 1), &vc[0], vc.size(), timeOutMS );
	vc[0] = (dutyDiv>>8) & 0xff;
	vc[1] = dutyDiv & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, U16(88 + chNo - 1), &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	return 0;
}

//设置指定通道的PWM脉冲输出到个数，如果设置为0，则表示启动PWM后一直输出。
I32 _stdcall EM9118_PwmSetCount( EM9118_DevPara* pDev, I32 chNo, U32 setCount, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !IsPwmChRight( chNo ) )
		return EM9118_FAILURE;
	V_I8 vc(2, 0);
	vc[0] = (setCount>>8) & 0xff;
	vc[1] = setCount & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, U16(96 + chNo - 1), &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	return 0;
}

//启动或者停止PWM输出，is90表示相应的通道是否延时90°
I32 _stdcall EM9118_PwmStartAll( EM9118_DevPara* pDev, I8 startOrStop[EM9118_MAXPWMCHCNT], I8 is90[EM9118_MAXPWMCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vcStart(2, 0), vcIs90(2, 0), vcDoFun(2,0);
	for( int i = 0; i < EM9118_MAXPWMCHCNT; ++i )
	{
		startOrStop[i] &= 1;
		vcStart[1] |= (startOrStop[i] << i);
		if( startOrStop[i] )
			vcDoFun[i] = 1;
		is90[i] &= 1;
		vcIs90[1] |= (is90[i] << i);
	}

	//设置DO管脚功能
	ZTLC_SendAndVerify( pDev->hCmd, 34, &vcDoFun[0], vcDoFun.size(), timeOutMS );
	ZTLC_SendAndVerify( pDev->hCmd, 104, &vcIs90[0], vcIs90.size(), timeOutMS );
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 105, &vcStart[0], vcStart.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return 0;
}

bool IsEM9318PwmChRight( I32 chNo )
{
	return 0 < chNo && chNo <= EM9318_MAXPWMCHCNT;
}

//PWM脉冲是否输出到指定个数
I32 _stdcall EM9318_PwmIsOver( EM9118_DevPara* pDev, I8 isOver[EM9318_MAXPWMCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 105, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	if( isOver )
		for( int i = 0; i < EM9318_MAXPWMCHCNT; ++i )
		{
			isOver[i] = ((vc[1] >> i) & 1) ? 0 : 1;
		}

	return 0;
}

//设置PWM脉冲的周期和占空比
I32 _stdcall EM9318_PwmSetPulse( EM9118_DevPara* pDev, I32 chNo, double freq, double dutyCycle, double* realFreq, double* realDutyCycle, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	
	if( !pDev )
		return EM9118_FAILURE;
	if( !IsEM9318PwmChRight( chNo ) )
		return EM9118_FAILURE;

	U32 freqDiv = U32(EM9318_PWM_BASE_FREQ / freq);//得到分频系数
	U32 dutyHight = U32(freqDiv * dutyCycle);//得到高电平系数
	V_I8 vc(10, 0);
	vc[0] = 0;
	vc[1] = chNo & 0xff;
	vc[2] = (freqDiv>>24)&0xff;
	vc[3] = (freqDiv>>16)&0xff;
	vc[4] = (freqDiv>>8)&0xff;
	vc[5] = freqDiv & 0xff;
	vc[6] = (dutyHight>>24)&0xff;
	vc[7] = (dutyHight>>16)&0xff;
	vc[8] = (dutyHight>>8)&0xff;
	vc[9] = dutyHight & 0xff;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, U16(80), &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	if( realFreq )
		*realFreq = EM9118_BASE_FREQ / freqDiv;
	if( realDutyCycle )
		*realDutyCycle = (F64)dutyHight / freqDiv;

	return 0;
}

//设置指定通道的PWM脉冲输出到个数，如果设置为0，则表示启动PWM后一直输出。
I32 _stdcall EM9318_PwmSetCount( EM9118_DevPara* pDev, I32 chNo, U32 pwmCount, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !IsEM9318PwmChRight( chNo ) )
		return EM9118_FAILURE;
	V_I8 vc(6, 0);
	vc[1] = chNo & 0xff;
	vc[2] = (pwmCount>>24) & 0xff;
	vc[3] = (pwmCount>>16) & 0xff;
	vc[4] = (pwmCount>>8) & 0xff;
	vc[5] = pwmCount & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, U16(96), &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	return 0;
}

//启动或者停止PWM输出，is90表示相应的通道是否延时90°
I32 _stdcall EM9318_PwmStartAll( EM9118_DevPara* pDev, I8 startOrStop[EM9318_MAXPWMCHCNT], I8 is90[EM9318_MAXPWMCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vcStart(2, 0), vcIs90(2, 0);
	for( int i = 0; i < EM9318_MAXPWMCHCNT; ++i )
	{
		startOrStop[i] &= 1;
		vcStart[1] |= (startOrStop[i] << i);
		is90[i] &= 1;
		vcIs90[1] |= (is90[i] << i);
	}

	ZTLC_SendAndVerify( pDev->hCmd, 104, &vcIs90[0], vcIs90.size(), timeOutMS );
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 105, &vcStart[0], vcStart.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return 0;
}

FILE* CSVFileOpen( const string& str )
{
	FILE* f;
	errno_t ret = fopen_s( &f, str.c_str(), "w" );
	if( ret )
	{
		return NULL;
	}
	return f;
}

I32 _stdcall EM9118_CSVOpen( EM9118_DevPara* pDev, char* pathName, I32 pnLen )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( pnLen >= MAX_PATH )
		return EM9118_FILEPATH_ERROR;
	string strPN;
	if( pnLen )
	{
		V_I8 pn(pnLen + 1);
		memcpy( &pn[0], pathName, pnLen );
		strPN = &pn[0];
	}else{
		strPN = pathName;
	}

	DWORD dwAttr = GetFileAttributes(strPN.c_str());  //得到文件属性

	if (dwAttr == INVALID_FILE_ATTRIBUTES)    // 文件或目录不存在
	{
		return EM9118_FILEPATH_ERROR;
	}
	if( !(dwAttr&FILE_ATTRIBUTE_DIRECTORY) )  // 不是目录
	{
		return EM9118_FILEPATH_ERROR;
	}
	pDev->dataPathName = strPN;
	if( pathName[pDev->dataPathName.length()] != '\\')//如果是根目录，会有\，比如c:
		pDev->dataPathName += "\\";
	char fileName[256];
	//生成文件名
	time_t t;
	time( &t );
	struct tm mtm;
	localtime_s( &mtm, &t );
	sprintf_s( fileName, sizeof(fileName), "%02d-%02d-%02d-%02d-%02d-%02d_1.csv", mtm.tm_year % 100, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_mon, mtm.tm_sec );
	pDev->dataPathName += fileName;
	pDev->csvF = CSVFileOpen( pDev->dataPathName );
	pDev->csvLineInx = 0;

	EM9118_GetFifoGroupByteCount( pDev, &pDev->groupBC );

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_CSVClose( EM9118_DevPara* pDev )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( pDev->csvF )
	{
		fclose( pDev->csvF );
		pDev->csvF = NULL;
	}

	return EM9118_SUCCESS;
}

void CSVWriteHeader( EM9118_DevPara* pDev )
{
	//先写标题行
	fprintf( pDev->csvF, "time(us),");
	
	for( U32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		if( pDev->adPara.isInFifo[i] )
			fprintf( pDev->csvF, "AD%d,", i + 1 );
	}

	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		if( pDev->ctInFifo[i] )
			fprintf( pDev->csvF, "CT%d,", i + 1 );
	}
	for( I32 i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		if( pDev->ecInFifo[i] )
			fprintf( pDev->csvF, "EC%d,", i + 1 );
	}
	if( pDev->diInFifo )
		fprintf( pDev->csvF, "DI" );

	fprintf( pDev->csvF, "\n" );
}

I32 _stdcall EM9118_CSVWriteValue( EM9118_DevPara* pDev, I8* pCode, U32 codeBC )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !pDev->csvF )
		return EM9118_FAILURE;
	if( !pDev->csvLineInx )
		CSVWriteHeader( pDev );
	
	int ret = 0;
	for( U32 j = 0; j < codeBC; j += pDev->groupBC )
	{
		fprintf( pDev->csvF, "%0.0f,", pDev->csvLineInx / pDev->groupFreq * 1000000.0 );//微秒
		I16* adCode = (I16*)&pCode[j];
		int i = 0;
		for( i = 0; i < pDev->commonPara.adChCount; ++i )
		{
			//AD原码值转化成物理值
			F64 adValue;
/*			EM9118_AdChCodeToValue( pDev, i + 1, adCode[i], &adValue );
			ret = fprintf( pDev->csvF, "%0.0f,", adValue * 1000.0 );*/
			adValue = adCode[i];
			ret = fprintf( pDev->csvF, "%0.0f,", adValue );
		}
		U32* ctCode = (U32*)&pCode[j + pDev->commonPara.adChCount*2];
		for( i = 0; i < pDev->commonPara.ctChCount; ++i )
		{
			ret = fprintf( pDev->csvF, "%u,", ctCode[i] );
		}
		I32* ecCode = (I32*)&pCode[j + pDev->commonPara.adChCount*2 + pDev->commonPara.ctChCount*2];
		for( i = 0; i < pDev->commonPara.ecChCount; ++i )
		{
			ret = fprintf( pDev->csvF, "%d,", ecCode[i] );
		}
		ret = fprintf( pDev->csvF, "\n", ecCode[i] );
		++pDev->csvLineInx;
	}

	if( ret < 0 )
	{
		fclose( pDev->csvF );
		pDev->csvF = NULL;
		pDev->csvLineInx = 0;
		return EM9118_FILEWRITE_ERROR;;
	}

	if( pDev->csvLineInx >= pDev->csvLinesCount )
	{
		fclose( pDev->csvF );
		pDev->csvLineInx = 0;
		I32 _errNo =0;
		if( !CHistoricalDataReader::CsvNextFileName( pDev->dataPathName, _errNo ) )
		{
			return EM9118_FILEPATH_ERROR;
		}
		//这里貌似有错，应该是csvF，15.11.12,ZCD
//		pDev->dfwF = CSVFileOpen( pDev->dataPathName );
		pDev->csvF = CSVFileOpen( pDev->dataPathName );
	}

	return EM9118_SUCCESS;
}

FILE* DFWFileOpen( const string& str )
{
	FILE* f;
	errno_t ret = fopen_s( &f, str.c_str(), "wb" );
	if( ret )
	{
		return NULL;
	}
	return f;
}

I32 _stdcall EM9118_DFWOpen( EM9118_DevPara* pDev, char* pathName, I32 pnLen )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( pnLen >= MAX_PATH )
		return EM9118_FILEPATH_ERROR;
	string strPN;
	if( pnLen )
	{
		V_I8 pn(pnLen + 1);
		memcpy( &pn[0], pathName, pnLen );
		strPN = &pn[0];
	}else{
		strPN = pathName;
	}

	DWORD dwAttr = GetFileAttributes(strPN.c_str());  //得到文件属性

	if (dwAttr == INVALID_FILE_ATTRIBUTES)    // 文件或目录不存在
	{
		return EM9118_FILEPATH_ERROR;
	}
	if( !(dwAttr&FILE_ATTRIBUTE_DIRECTORY) )  // 不是目录
	{
		return EM9118_FILEPATH_ERROR;
	}
	pDev->dataPathName = strPN;
	if( pathName[pDev->dataPathName.length()] != '\\')//如果是根目录，会有\，比如c:
		pDev->dataPathName += "\\";
	char fileName[256];
	//生成文件名
	time_t t;
	time( &t );
	struct tm mtm;
	localtime_s( &mtm, &t );
	sprintf_s( fileName, sizeof(fileName), "%02d-%02d-%02d-%02d-%02d-%02d_1.dat", mtm.tm_year % 100, mtm.tm_mon + 1, mtm.tm_mday, mtm.tm_hour, mtm.tm_min, mtm.tm_sec );
	pDev->dataPathName += fileName;
	pDev->dfwF = DFWFileOpen( pDev->dataPathName );
	pDev->dfwPos = 0;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DFWClose( EM9118_DevPara* pDev )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( pDev->dfwF )
	{
		fclose( pDev->dfwF );
		pDev->dfwF = NULL;
	}

	return EM9118_SUCCESS;
}

void DFWriteHeader( EM9118_DevPara* pDev )
{
	pDev->dfwPos = 0;
	//先写设备名称
	string devName( "9118" );
	fwrite( devName.c_str(), 1, devName.length(), pDev->dfwF );
	pDev->dfwPos += devName.length();
	//版本号
	U8 ver = 0;
	fwrite( &ver, 1, sizeof(ver), pDev->dfwF );
	pDev->dfwPos += sizeof(ver);
	//AD使能通道信息
	for( U32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		fwrite( &pDev->adPara.isInFifo[i], 1, 1, pDev->dfwF );
		++pDev->dfwPos;
		fwrite( &pDev->adPara.fullCode[i], 1, sizeof(I16), pDev->dfwF );
		pDev->dfwPos += sizeof(I16);
		fwrite( &pDev->adPara.zeroCode[i], 1, sizeof(I16), pDev->dfwF );
		pDev->dfwPos += sizeof(I16);
		fwrite( &pDev->adPara.fullValue[i], 1, sizeof(F64), pDev->dfwF );
		pDev->dfwPos += sizeof(F64);
		fwrite( &pDev->adPara.zeroValue[i], 1, sizeof(F64), pDev->dfwF );
		pDev->dfwPos += sizeof(F64);
	}
	fwrite( &pDev->groupFreq, 1, sizeof(F64), pDev->dfwF );
	pDev->dfwPos += sizeof(F64);
	fwrite( &pDev->clkSrc, 1, sizeof(I32), pDev->dfwF );
	pDev->dfwPos += sizeof(I32);
	fwrite( &pDev->triSrc, 1, sizeof(I32), pDev->dfwF );
	pDev->dfwPos += sizeof(I32);

	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		fwrite( &pDev->ctInFifo[i], 1, 1, pDev->dfwF );
		++pDev->dfwPos;
	}
	for( I32 i = 0; i < EM9118_MAXECCHCNT; ++i )
	{
		fwrite( &pDev->ecInFifo[i], 1, 1, pDev->dfwF );
		++pDev->dfwPos;
	}
	fwrite( &pDev->diInFifo, 1, 1, pDev->dfwF );
		++pDev->dfwPos;
}

I32 _stdcall EM9118_DFWrite( EM9118_DevPara* pDev, I8* pCode, U32 codeBC )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	if( !pDev->dfwF )
		return EM9118_FAILURE;
	if( !pDev->dfwPos )
		DFWriteHeader( pDev );
	size_t bc = fwrite( pCode, 1, codeBC, pDev->dfwF );
	if( bc != codeBC )
	{
		fclose( pDev->dfwF );
		pDev->dfwF = NULL;
		pDev->dfwPos = 0;
		return EM9118_FILEWRITE_ERROR;;
	}
	pDev->dfwPos += codeBC;

	if( pDev->dfwPos >= pDev->singleFileSize )
	{
		fclose( pDev->dfwF );
		pDev->dfwPos = 0;
		I32 _errNo =0;
		if( !CHistoricalDataReader::NextFileName( pDev->dataPathName, _errNo ) )
		{
			return EM9118_FILEPATH_ERROR;
		}
		pDev->dfwF = DFWFileOpen( pDev->dataPathName );
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DFROpen( char* filePathName, I32 pathLen )
{
	if( filePathName )
		if( pathLen )
			filePathName[pathLen] = 0;
	CHistoricalDataReader* hd = new CHistoricalDataReader( filePathName );
	return (I32)hd;
}

I32 _stdcall EM9118_DFRClose( I32 hDFR )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;
	delete hd;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_DFRCodeBufferSize( I32 hDFR, U32 groupCount, U32* allCodeCount )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;
	if( allCodeCount )
		*allCodeCount = hd->CodeBufferNeedCount( groupCount );
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRReadCodeByGroupInx( I32 hDFR, I8* codeBuf, U32 groupCodeCount, F64 codeBeginInx, U32* realReadCount )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	U32 ret = hd->ReadCodeByCount( codeBuf, groupCodeCount, codeBeginInx );
	if( realReadCount )
		*realReadCount = ret;
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetFreq( I32 hDFR, F64* daqFreq )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	if( daqFreq )
		*daqFreq = hd->GetFreq();
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetGroupTotal( I32 hDFR, F64* groupTotal )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	if( groupTotal )
		*groupTotal = hd->GetGroupTotal();
	return hd->GetErrorNo();
}


I32 _stdcall EM9118_DFRGetBeginEndCh( I32 hDFR, I32* beginChNo, I32* endChNo )
{//历史遗留，不再使用
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	hd->GetBeginEndCh( (U32*)beginChNo, (U32*)endChNo );

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetAdIsInfifo( I32 hDFR, I8* isInFifo )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	hd->GetAdIsInFifo( isInFifo );

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetCtIsInfifo( I32 hDFR, I8* isInFifo )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	hd->GetCtIsInFifo( isInFifo );

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetEcIsInfifo( I32 hDFR, I8* isInFifo )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	hd->GetEcIsInFifo( isInFifo );

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetClkSource( I32 hDFR, I32* clkSource )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	if( clkSource )
		*clkSource = hd->GetClkSource();

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRGetEtrSource( I32 hDFR, I32* etrSource )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	if( etrSource )
		*etrSource = hd->GetEtrSource();

	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRAdChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, I16* adCode, U32* adCodeCount )
{
	if( !hDFR )
		return EM9118_FAILURE;
	if( chInx >= EM9118_MAXADCHCNT )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	return EM9118_AdChGetCode( (&hd->_property), chInx, bytesCount, originalCode, adCode, adCodeCount );
}

I32 _stdcall EM9118_DFRAdCodeToValue( I32 hDFR, I32 chNo, I16 adCode, F64* adValue )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	EM9118_AdChCodeToValue( &hd->_property, chNo, adCode, adValue );
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRAdBatchCodeToValue( I32 hDFR, I32 chNo, I16* pCode, I32 codeCount, F64* adValue )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	EM9118_AdChBatchCodeToValue( &hd->_property, chNo, pCode, codeCount, adValue );
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFRCtChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, U32* ctCode, U32* ctCodeCount )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	return EM9118_CtChGetCode( (&hd->_property), chInx, bytesCount, originalCode, ctCode, ctCodeCount );
}

I32 _stdcall EM9118_DFRCtBatchCodeToValue( I32 hDFR, I32 chNo, U32* pCode, I32 codeCount, F64* ctValue )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	EM9118_CtChBatchCodeToValue( &hd->_property, chNo, pCode, codeCount, ctValue );
	return hd->GetErrorNo();
}

I32 _stdcall EM9118_DFREcChGetCode( I32 hDFR, I32 chInx, U32 bytesCount, I8* originalCode, I32* ecCode, U32* ecCodeCount )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	return EM9118_EcChGetCode( (&hd->_property), chInx, bytesCount, originalCode, ecCode, ecCodeCount );
}

I32 _stdcall EM9318_DFRIoGetCode( I32 hDFR, U32 bytesCount, I8* originalCode, U16* ioCode, U32* ioCodeCount )
{
	if( !hDFR )
		return EM9118_FAILURE;

	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	return EM9318_IoGetCode( (&hd->_property), bytesCount, originalCode, ioCode, ioCodeCount );
}

I32 _stdcall EM9118_DFRGetAdChValue( I32 hDFR, I32 chNo, I8* codeBuf, U32 groupCodeCount, F64* adValue )
{
	if( !hDFR )
		return EM9118_FAILURE;
	CHistoricalDataReader* hd = (CHistoricalDataReader*)hDFR;

	I32 chInx = chNo - 1;
	hd->GetAdChValue( chInx, codeBuf, groupCodeCount, adValue );
	
	return hd->GetErrorNo();
}

I32 _stdcall EM9106_IoSetDirAndMode( EM9118_DevPara* pDev, I8 ioDir[3], I8 doMode[4], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2,0);
	for( I32 i = 0; i < 4; ++i )
		vc[0] |= (doMode[i]&1)<<i;
	for( I32 i = 0; i < 3; ++i )
		vc[1] |= (ioDir[i]&1)<<i;

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 34, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9106_IoSetAll( EM9118_DevPara* pDev, I8* oStatus, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(4, 0);
	for( I32 i = 0; i < EM9106_MAXIOCHCNT; ++i )
	{
		I32 groupInx = i / 8;
		I32 bitInx = i % 8;
		vc[groupInx] |= oStatus[i] << bitInx;
	}

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 36, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9106_IoGetAll( EM9118_DevPara* pDev, I8* iStatus, I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(4);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 291, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	for( I32 j = 0; j < EM9118_MAXDICHCNT; ++j )
	{
		iStatus[j] = (vc[1] >> j) & 1;
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9318_IoGetAll( EM9118_DevPara* pDev, I8 iStatus[EM9318_MAXIOCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2);
	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 282, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	for( I32 j = 0; j < EM9318_MAXIOCHCNT; ++j )
	{
		iStatus[j] = (vc[1] >> j) & 1;
	}

	return EM9118_SUCCESS;
}

I32 _stdcall EM9318_EcClear( EM9118_DevPara* pDev, I32 chClear[EM9318_MAXECCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(2, 0);
	vc[1] = (chClear[3]&1) * 8 + (chClear[2]&1) * 4 + (chClear[1]&1) * 2 + (chClear[0]&1);

	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 193, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9318_EcReadCodeAll( EM9118_DevPara* pDev, I32 abCode[EM9318_MAXECCHCNT], I32 timeOutMS )
{
	if( !pDev )
		return EM9118_FAILURE;
	

	V_I8 vc(sizeof(I32) * EM9318_MAXECCHCNT);

	I32 ret = ZTLC_RecvAndVerify( pDev->hCmd, 283, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	//	MyTrace( "%2x", vc );
	if( abCode )
		memset( abCode, 0, sizeof(I32) * EM9318_MAXECCHCNT );
	I32 vcInx = 0;
	for( I32 codeInx = 0; codeInx < EM9318_MAXECCHCNT; ++codeInx )
	{
		//得到AB值
		if( abCode )
		{
			abCode[codeInx] |= vc[vcInx];
			for( I32 byteInx = 1; byteInx < 4; ++byteInx )
			{
				abCode[codeInx] <<= 8;
				++vcInx;
				abCode[codeInx] |= (U8)vc[vcInx];
			}
		}
		++vcInx;
	}
	return 0;
}

I32 _stdcall EM9118_HcSetTriCount( EM9118_DevPara* pDev, U32 triCount, I32 timeOutMS )
{
	
	if( !pDev )
		return EM9118_FAILURE;
	V_I8 vc(4, 0);
	vc[0] = (triCount>>24) & 0xff;
	vc[1] = (triCount>>16) & 0xff;
	vc[2] = (triCount>>8) & 0xff;
	vc[3] = triCount & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 26, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;
	pDev->triCount = triCount;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdWriteZero( EM9118_DevPara* pDev, I32 chNo, I16 zeroCode, I32 timeOutMS )
{
	
	if( !pDev )
		return EM9118_FAILURE;

	V_I8 vc(4, 0);
	vc[1] = chNo & 0xff;
	vc[2] = (zeroCode>>8) & 0xff;
	vc[3] = zeroCode & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 200, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdWriteFull( EM9118_DevPara* pDev, I32 chNo, I16 fullCode, I32 timeOutMS )
{
	
	if( !pDev )
		return EM9118_FAILURE;

	V_I8 vc(4, 0);
	vc[1] = chNo & 0xff;
	vc[2] = (fullCode>>8) & 0xff;
	vc[3] = fullCode & 0xff;
	I32 ret = ZTLC_SendAndVerify( pDev->hCmd, 202, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;


	return EM9118_SUCCESS;
}

I32 _stdcall EM9118_AdReadAllCodeOnce( EM9118_DevPara* pDev, I16 adCode[EM9118_MAXADCHCNT], I32 timeOutMS )
{
	if( !adCode )
		return EM9118_FAILURE;

	I32 ret = 0;
	V_I8 vc(sizeof(I16) * EM9118_MAXADCHCNT);
	if( pDev->isHcStart )
		ret = ZTLC_RecvAndVerify( pDev->hCmd, 512, &vc[0], vc.size(), timeOutMS );
	else
		ret = ZTLC_RecvAndVerify( pDev->hCmd, 256, &vc[0], vc.size(), timeOutMS );
	if( ret < 0 )
		return ret;

	//	MyTrace( "%2x", vc );
	memset( adCode, 0, vc.size() );
	I32 vcInx = 0;
	for( I32 codeInx = 0; codeInx < EM9118_MAXADCHCNT; ++codeInx )
	{
		adCode[codeInx] |= vc[vcInx++] & 0xff;
		adCode[codeInx] <<= 8;
		adCode[codeInx] |= (U8)vc[vcInx++] & 0xff;
	}
	return 0;
}
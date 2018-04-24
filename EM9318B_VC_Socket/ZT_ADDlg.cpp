// ZT_ADDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <float.h>
#include "EM9318B_VC_Socket.h"
#include "ZT_ADDlg.h"
#include "EM9318B_VC_SocketDlg.h"

// CZT_ADDlg 对话框

IMPLEMENT_DYNAMIC(CZT_ADDlg, CDialog)

CZT_ADDlg::CZT_ADDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZT_ADDlg::IDD, pParent)
	, _adRange(0)
	, _clkSrc(0)
	, _triSrc(0)
	, _edgeLevel(0)
	, _upDown(0)
	, _dispMode(0)
	, _isDaqStart(0)
	, _isSave(FALSE)
	, _isProcess(TRUE)
	, _adAverageNumber(1000)
	, _readTimes(0)
	, _triCount(0)
{
	_ctrlDaqFreq = "200KHz";

	_readThreadExit = CreateEvent( NULL, FALSE, FALSE, NULL );

	int i = 0;
	for( i = 0; i < EM9118_MAXADCHCNT; ++i )
		_isAdInFifo[i] = 1;

}

CZT_ADDlg::~CZT_ADDlg()
{
	CloseHandle( _readThreadExit );
}

void CZT_ADDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_ADFREQDIV, _ctrlDaqFreq);
	DDX_CBIndex(pDX, IDC_ADRANGE, _adRange);
	int i = 0;
	for( i = IDC_CHKAD1; i <= IDC_CHKAD18; ++i )
		DDX_Check(pDX, i, _isAdInFifo[i-IDC_CHKAD1]);
	DDX_CBIndex(pDX, IDC_CLKSRC, _clkSrc);
	DDX_CBIndex(pDX, IDC_TRISRC, _triSrc);
	DDX_CBIndex(pDX, IDC_EDGELEVEL, _edgeLevel);
	DDX_CBIndex(pDX, IDC_UPDOWN, _upDown);
	DDX_CBIndex(pDX, IDC_ADDISPLAY, _dispMode);
	DDX_Check(pDX, IDC_ISSAVE, _isSave);
	DDX_Check(pDX, IDC_ISPROCESS, _isProcess);
	DDX_Text(pDX, IDC_AVERAGENUM, _adAverageNumber);
	DDV_MinMaxInt(pDX, _adAverageNumber, 0, 10000);
	DDX_Text(pDX, IDC_TRICOUNT, _triCount);
}


BEGIN_MESSAGE_MAP(CZT_ADDlg, CDialog)
	ON_BN_CLICKED(IDSTARTDAQ, &CZT_ADDlg::OnBnClickedStartdaq)
	ON_BN_CLICKED(IDSTOPDAQ, &CZT_ADDlg::OnBnClickedStopdaq)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDREADADONCE, &CZT_ADDlg::OnBnClickedReadadonce)
END_MESSAGE_MAP()

F64 CZT_ADDlg::GetDaqFreq()
{
	F64 daqFreq = atol(_ctrlDaqFreq);
	_ctrlDaqFreq.MakeUpper();
	if( _ctrlDaqFreq.Find( 'K' ) != -1 )
		daqFreq *= 1000;
	else if( _ctrlDaqFreq.Find( 'M') != -1 )
		daqFreq *= 1000000;

	if( !( 0 < daqFreq || daqFreq <= 450000 ) )
	{
		daqFreq = 200000.0;
		_ctrlDaqFreq = "200KHz";
		GetDlgItem(IDC_ADFREQDIV)->SetWindowText( _ctrlDaqFreq );
	}
	return daqFreq;
}

I32 CZT_ADDlg::StopAD()
{
	I32 ret = EM9118_HcStop( _pF->_hDev );
	return ret;
}

void CZT_ADDlg::ShowSpeed( F64 tvS, I32 onceBC, I32 bcGroup )
{
	_allReadBC += onceBC;
	CString str;
	str.Format( "%7.3fMB", _allReadBC / 1024 / 1024 );
	GetDlgItem( IDC_ALLBCMB )->SetWindowText( str );
	F64 allTime = _tvAll.GetIntervalS();
	F64 averageSpeedMB = _allReadBC / 1024 / 1024 / allTime;
	str.Format( "%7.3fMB/s", averageSpeedMB );
	GetDlgItem( IDC_SPEEDEVERAGEMB )->SetWindowText( str );
	F64 everageSpeedGC = _allReadBC / bcGroup / allTime;
	str.Format( "%8.0f", everageSpeedGC );
	GetDlgItem( IDC_SPEEDEVERAGEGC )->SetWindowText( str );
	F64 rtSpeedGC = onceBC / bcGroup / tvS;
	str.Format( "%8.0f", rtSpeedGC );
	GetDlgItem( IDC_SPEEDRTGC )->SetWindowText( str );

	if( _triSrc == EM9118_CLKSRC_EX && _edgeLevel == EM9118_TRI_EDGE && _allReadBC / bcGroup >= _triCount )
	{
		_pF->ShowInfo( "已经获取指定采集组数，实际组数：%f\n", _allReadBC / bcGroup );
		_pF->ShowInfo( "正在等待外触发信号\n" );
		_allReadBC = 0;
	}
}

void CZT_ADDlg::ProcessData( V_I8& codeBuffer, I32 bcGroup )
{
	I32 i = 0, j = 0;
	//拆分出设定组数数据：
	U32 needBC = bcGroup * _processGroupCnt;
	for( i = 0; i < _adCnt; ++i )
	{
		V_I16 adCode(_processGroupCnt);
		EM9118_AdChGetCode( _pF->_hDev, i, needBC, &codeBuffer[0], &adCode[0], 0 );
		if( _dispMode )
		{
			EM9118_AdChBatchCodeToValue( _pF->_hDev, _adInx2No[i], &adCode[0], _processGroupCnt, &_adValue[i][0] );
		}else{
			for( j = 0; j < _processGroupCnt; ++j )
				_adValue[i][j] = adCode[j];
		}
	}
	for( i = 0; i < _ctCnt; ++i )
	{
		V_U32 ctCode( _processGroupCnt );
		EM9118_CtChGetCode( _pF->_hDev, i, needBC, &codeBuffer[0], &ctCode[0], 0 );
		//计数方式方式下原码值就是物理值，测频方式下原码值需要经过转换才能变成物理值（频率)
		EM9118_CtChBatchCodeToValue( _pF->_hDev, i + 1, &ctCode[0], _processGroupCnt, &_ctValue[i][0] );
	}
	for( i = 0; i < _ecCnt; ++i )
	{
		V_I32 ecCode( _processGroupCnt );
		EM9118_EcChGetCode( _pF->_hDev, i, needBC, &codeBuffer[0], &ecCode[0], 0 );
		//原码值直接就是物理值
		for( j = 0; j < _processGroupCnt; ++j )
			_ecValue[i][j] = ecCode[j];
	}
	if( _pF->_dlgIo._isIoInFifo )
	{
		V_U16 ioCode( _processGroupCnt );
		EM9318_IoGetCode( _pF->_hDev, needBC, &codeBuffer[0], &ioCode[0], 0 );
		for( j = 0; j < _processGroupCnt; ++j )
			_ioValue[j] = ioCode[j];
	}
}

void CZT_ADDlg::ReadThread()
{
	//计算出每次需要读取的数据量
	I32 bcGroup = 0; 
	I32 ret = EM9118_GetFifoGroupByteCount( _pF->_hDev, &bcGroup );
	if( ret < 0 )
	{
		_pF->ShowInfo( "EM9118_GetFifoGroupByteCount error: %d", ret );
		return;
	}
	I32 bcNeed = bcGroup * _groupCntNeed;
	
	//根据数据量建立缓冲区
	V_I8 codeBuffer(bcNeed);
	I32 isError = 0;

	_tvAll.Reset();
	_allReadBC = 0;
	while( _isDaqStart )
	{
		I32 realBC = 0;
		CTimeInterval tv;
		//读取数据
		I32 ret = EM9118_HcReadData( _pF->_hDev, bcNeed, &codeBuffer[0], &realBC );
		if( ret < 0 )
		{
			if( ret == EM9118_TIMEOUT_ERROR )
			{
				if( realBC > 0 )
				{
//					_pF->ShowInfo( "超时:%d,%d\n", ret, realBC );
				}else
				{
//					_allReadBC = 0;
					continue;
				}
			}else
			{
				isError = 1;
				break;
			}
		}
		ShowSpeed( tv.GetIntervalS(), realBC, bcGroup );

		//如果选中了存盘，则执行存储操作
		if( _isSave )
			EM9118_DFWrite( _pF->_hDev, &codeBuffer[0], realBC );


		if( _isProcess )
		{//处理数据
			_lockValue.Lock();
			ProcessData( codeBuffer, bcGroup );
			_lockValue.Unlock();
		}
		++_readTimes;
	}
//	_pF->ShowInfo( "thread end\n" );
	SetEvent( _readThreadExit );
	if( isError )
		OnBnClickedStopdaq();
}

UINT ReadThreadShell( void* para )
{
	CZT_ADDlg* p = (CZT_ADDlg*)para;
	//	while( p->_isDaqStart );
	p->ReadThread();
	return 0;
}

void CZT_ADDlg::OpenWriteFile()
{
	char dirPath[256];
	GetCurrentDirectory( sizeof(dirPath), dirPath );
	I32 ret = EM9118_DFWOpen( _pF->_hDev, dirPath, 0 );
	if( ret < 0 )
	{
		_pF->ShowInfo( "EM9118_DFWOpen error %d\n",  ret );
	}
}

void CZT_ADDlg::OnBnClickedStartdaq()
{
	if( !UpdateData(TRUE) )
	{
		return;
	}
	_pF->_dlgIo.UpdateData( TRUE );
	_pF->_dlgCt.UpdateData( TRUE );
	_pF->_dlgEc.UpdateData( TRUE );

	//初始化AD采集
	//设置AD采集频率
	F64 freq = GetDaqFreq();
	EM9118_HcSetGroupFreq( _pF->_hDev, freq );

	//设置需要AD高速采集的通道
	EM9118_AdChIsInFifo( _pF->_hDev, (I32*)_isAdInFifo );
	//设置需要编码器高速采集的通道
	EM9118_EcChIsInFifo( _pF->_hDev, (I32*)_pF->_dlgEc._isEcInFifo );
	//设置需要计数器高速采集的通道
	EM9118_CtChIsInFifo( _pF->_hDev, (I32*)_pF->_dlgCt._isCtInFifo );
	//设置需要开关量高速采集的通道
	EM9118_DiIsInFifo( _pF->_hDev, _pF->_dlgIo._isIoInFifo );
	//上面四个函数调用完毕后，可以调用下面的函数获得每组数据的字节数
	EM9118_GetFifoGroupByteCount( _pF->_hDev, &_groupBC );

	//设置AD采集范围
	EM9118_AdSetRange( _pF->_hDev, _adRange );

	//获取每种测量值的通道数
	_adCnt = EM9118_GetFifoAdChCount( _pF->_hDev );
	_ctCnt = EM9118_GetFifoCtChCount( _pF->_hDev );
	_ecCnt = EM9118_GetFifoEcChCount( _pF->_hDev );

	//给物理值分配空间
	_groupCntNeed = I32(GetDaqFreq() / 1000.0 * _pF->_readInterval);
	ASSERT( _groupCntNeed > 0 );

	//如果选中了处理数据复选框，则需要对处理数据相关的变量初始化
	if( _isProcess )
	{
		if( _adAverageNumber > 0 )
		{
			if( _adAverageNumber > _groupCntNeed )
				_processGroupCnt = _groupCntNeed;
			else
				_processGroupCnt = _adAverageNumber;
		}
		else
		{
			_processGroupCnt = _groupCntNeed;
		}
		I32 i;
		I32 chInx = 0;
		for( i = 0; i < EM9118_MAXADCHCNT; ++i )
		{
			if( _isAdInFifo[i] )
			{
				_adValue[chInx].resize( _processGroupCnt );
				_adInx2No[chInx++] = i + 1;
			}
		}
		for( i = 0; i < _ctCnt; ++i )
			_ctValue[i].resize( _processGroupCnt );
		for( i = 0; i < _ecCnt; ++i )
			_ecValue[i].resize( _processGroupCnt );
		if( _pF->_dlgIo._isIoInFifo )
			_ioValue.resize( _processGroupCnt );
	}
	
	//如果选择存盘，则需要建立文件
	if( _isSave )
	{
		OpenWriteFile();
	}

	//设置边沿外触发时外触发个数
	EM9118_HcSetTriCount( _pF->_hDev, _triCount );

	//启动硬件采集
	EM9118_HcStart( _pF->_hDev, _clkSrc, _triSrc, _edgeLevel, _upDown );

	if( _triSrc == EM9118_CLKSRC_EX )
		_pF->ShowInfo( "正在等待外触发信号\n" );


	_isDaqStart = 1;
	_readTimes = 0;
	AfxBeginThread( ReadThreadShell, this );

	SetTimer( 1, _pF->_readInterval, NULL );

	GetDlgItem(IDSTARTDAQ)->EnableWindow(FALSE);
	GetDlgItem(IDSTOPDAQ)->EnableWindow(TRUE);
}

void MyWaitForSingleObject( HANDLE a, I32 timeoutMS )
{
	DWORD dwRet = 0;
	MSG msg;
	while(TRUE)
	{
		dwRet = MsgWaitForMultipleObjects(1, &a, FALSE, timeoutMS, QS_ALLINPUT);
		switch(dwRet)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_OBJECT_0 + 1:
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
			continue;
		case WAIT_TIMEOUT:
			break;
		default:
			break;
		}
		break;
	}
}

void CZT_ADDlg::OnBnClickedStopdaq()
{
	_isDaqStart = 0;
	MyWaitForSingleObject( _readThreadExit, 2000 );

	//停止硬件控制采集
	StopAD();
	if( _isSave )
		EM9118_DFWClose( _pF->_hDev );
	KillTimer( 1 );

	GetDlgItem(IDSTARTDAQ)->EnableWindow(TRUE);
	GetDlgItem(IDSTOPDAQ)->EnableWindow(FALSE);
}

void CZT_ADDlg::ShowAdValue( I32 chInx, const V_F64& adValue )
{
	ASSERT( 0 <= chInx && chInx < EM9118_MAXADCHCNT );
	F64 sumValue = 0;
	F64 minValue = DBL_MAX, maxValue = DBL_MIN;
	for( V_F64::const_iterator it = adValue.begin(); it != adValue.end(); ++it )
	{
		sumValue += *it;
		minValue = min( minValue, *it );
		maxValue = max( maxValue, *it );
	}
	CString strMean,strDiff;
	if( _dispMode )
	{
		strMean.Format( "%5.3f", sumValue / adValue.size() );
		strDiff.Format( "%5.3f", (maxValue - minValue) );
	}else
	{
		strMean.Format( "%5.0f", sumValue / adValue.size() );
		strDiff.Format( "%5.0f", (maxValue - minValue) );
	}
	GetDlgItem( IDC_ADDATA1 + _adInx2No[chInx] - 1 )->SetWindowText( strMean );
	GetDlgItem( IDC_ADDIFF1 + _adInx2No[chInx] - 1 )->SetWindowText( strDiff );
}

void CZT_ADDlg::ShowCtValue( I32 chInx, F64 ctCode )
{
	ASSERT( 0 <= chInx && chInx < EM9118_MAXCTCHCNT );

	CString str;
	str.Format( "%0.0f", ctCode );
	_pF->_dlgCt.GetDlgItem( IDC_CNTVAL1 + chInx )->SetWindowText( str );
}

void CZT_ADDlg::ShowEcValue( I32 chInx, I32 ecCode )
{
	ASSERT( 0 <= chInx && chInx < EM9118_MAXECCHCNT );

	CString str;
	str.Format( "%8d", ecCode );
	_pF->_dlgEc.GetDlgItem( IDC_EC1 + chInx )->SetWindowText( str );
}

void CZT_ADDlg::ShowIoValue( U16 ioCode )
{
	for( I32 i = 0; i < EM9318_MAXIOCHCNT; ++i )
		((CButton*)_pF->_dlgIo.GetDlgItem( IDC_IO1 + i ))->SetCheck( (ioCode>>i)&1 );
}

void CZT_ADDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( !_isProcess )
		return;

	int i = 0;
	_lockValue.Lock();
	for( i = 0; i < _adCnt; ++i )
	{
		ShowAdValue( i, _adValue[i] );
	}
	for( i = 0; i < _ctCnt; ++i )
		ShowCtValue( i, _ctValue[i][0] );
	for( i = 0; i < _ecCnt; ++i )
		ShowEcValue( i, (I32)_ecValue[i][0] );
	if( _pF->_dlgIo._isIoInFifo )
		ShowIoValue( _ioValue[0] );
	_lockValue.Unlock();

	CDialog::OnTimer(nIDEvent);
}

void CZT_ADDlg::OnBnClickedReadadonce()
{
	UpdateData( TRUE );
	I32 ret = EM9118_AdSetRange( _pF->_hDev, _adRange );
	if( ret < 0 )
	{
		_pF->ShowInfo( "EM9118_AdSetRange error:%d", ret );
		return;
	}

	V_I16 adCode(EM9118_MAXADCHCNT);
	ret = EM9118_AdReadAllCodeOnce( _pF->_hDev, &adCode[0] );
	if( ret < 0 )
	{
		_pF->ShowInfo( "EM9118_AdReadAllCodeOnce error:%d", ret );
		return;
	}

	for( U32 i = 0; i < EM9118_MAXADCHCNT; ++i )
	{
		F64 adValue;
		CString str;
		if( _dispMode )
		{
			EM9118_AdChCodeToValue( _pF->_hDev, i + 1, adCode[i], &adValue );
			str.Format( "%5.3f", adValue );
		}
		else
		{
			adValue = adCode[i];
			str.Format( "%5.0f", adValue );
		}

		SetDlgItemText( IDC_ADDATA1 + i, str );
	}
}

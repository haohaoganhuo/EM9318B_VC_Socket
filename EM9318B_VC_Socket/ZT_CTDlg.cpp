// ZT_CTDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EM9318B_VC_Socket.h"
#include "ZT_CTDlg.h"
#include "EM9318B_VC_SocketDlg.h"


// CZT_CTDlg 对话框

IMPLEMENT_DYNAMIC(CZT_CTDlg, CDialog)

CZT_CTDlg::CZT_CTDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZT_CTDlg::IDD, pParent)
	, _isCt(FALSE)
{
	_cntMode = 0;
	I32 i = 0;
	for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		_isCtInFifo[i] = 0;
		_ctBaseMS[i] = 1000;
	}
}

CZT_CTDlg::~CZT_CTDlg()
{
}

void CZT_CTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_CBIndex(pDX, IDC_CNTSTYTLE1, _cntMode);
	I32 i;
	for( i = IDC_CNTENDVAL1; i <= IDC_CNTENDVAL4; ++i )
	{
		DDX_Text(pDX, i, _ctBaseMS[i-IDC_CNTENDVAL1]);
		DDV_MinMaxDouble(pDX, _ctBaseMS[i-IDC_CNTENDVAL1], 1.e-003, 1000.);
	}
	for( i = IDC_CHKCT1; i <= IDC_CHKCT4; ++i )
		DDX_Check(pDX, i, _isCtInFifo[i-IDC_CHKCT1]);
	DDX_Check(pDX, IDC_ISCT, _isCt);
}


BEGIN_MESSAGE_MAP(CZT_CTDlg, CDialog)
	ON_BN_CLICKED(IDC_SETCNT, &CZT_CTDlg::OnBnClickedSetcnt)
	ON_BN_CLICKED(IDC_ISCT, &CZT_CTDlg::OnBnClickedIsct)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CTCLEAR, &CZT_CTDlg::OnBnClickedCtclear)
END_MESSAGE_MAP()


// CZT_CTDlg 消息处理程序

void CZT_CTDlg::OnBnClickedSetcnt()
{
	UpdateData( TRUE );
	I8 ctMode[EM9118_MAXCTCHCNT];
	I32 i = 0;
	for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		ctMode[i] = _cntMode;
	}
	//设置计数器工作方式（计数还是测频）
	I32 ret = EM9118_CtSetModeAll( _pF->_hDev, ctMode );
	for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		//测频方式下需要设置基准时钟
		I32 ret = EM9118_CtSetFreqBase( _pF->_hDev, i + 1, _ctBaseMS[i], 0 );
		if( ret < 0 )
		{
			_pF->ShowInfo( "EM9118_CtReadCodeAll：%d\n", ret );
			break;
		}
	}
}

void CZT_CTDlg::OnBnClickedIsct()
{
	UpdateData( TRUE );
	if( _isCt )
	{
		SetTimer( 1, _pF->_readInterval, 0 );
	}else
	{
		KillTimer( 1 );
	}
}

void CZT_CTDlg::ShowCtValue( I32 chInx, F64 ctCode )
{
	ASSERT( 0 <= chInx && chInx < EM9118_MAXCTCHCNT );

	CString str;
	str.Format( "%0.0f", ctCode );
	GetDlgItem( IDC_CNTVAL1 + chInx )->SetWindowText( str );
}

void CZT_CTDlg::OnTimer(UINT_PTR nIDEvent)
{
	U32 ctCode[EM9118_MAXCTCHCNT];
	I32 ret = EM9118_CtReadCodeAll( _pF->_hDev, ctCode, 0 );
	if( ret < 0 )
		_pF->ShowInfo( "EM9118_CtReadCodeAll：%d\n", ret );
	F64 ctValue[EM9118_MAXCTCHCNT];
	I32 i = 0;
	if( _cntMode )
		for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
			EM9118_CtChBatchCodeToValue( _pF->_hDev, i+1, &ctCode[i], 1, &ctValue[i]);
	else
		for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
			ctValue[i] = ctCode[i];

	F64 cmpValue[4] = {154,6951,184,230};
	for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
		if( cmpValue[i] != ctValue[i] )
			break;

	for( i = 0; i < EM9118_MAXCTCHCNT; ++i )
	{
		ShowCtValue( i, ctValue[i] );
	}

	CDialog::OnTimer(nIDEvent);
}

void CZT_CTDlg::OnBnClickedCtclear()
{
	for( I32 i = 0; i < EM9118_MAXCTCHCNT; ++i )
		EM9118_CtClear( _pF->_hDev, i+1 );
}
